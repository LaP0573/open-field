#!/usr/bin/env python3
"""open-field helper for Linux: the file work of the install, on the OP-1 Field's disk.

  python3 -I open-field-helper.py --page <the page's mark>          # the unit over USB, the folder ~/open-field
  python3 -I open-field-helper.py --folder <folder> --page <mark>   # another shared folder
  python3 -I open-field-helper.py --source <folder>                 # a folder of this computer stands for the unit's disk
  python3 -I open-field-helper.py --list-only                       # the unit over USB: print what is seen, change nothing

A web page cannot reach the unit's files, so this does it. The unit shows its disk while its bootloader's drive mode
is open (hold COM while switching it on, press 3, then 2). The helper works with the install page
through one folder of this computer, the shared folder:

  1. it finds the unit, copies your own firmware file off (upgrade/firmware/blackfin/bin/os.ldr), takes the log files
     off into the shared folder, lists the disk, and waits;
  2. the page builds the new firmware file, works out whether the write lands, and writes a plan into the folder;
  3. this puts the new file on the unit as os.ldr (and a pad file and pack files when the plan says so), lists the
     disk again, holds the listing against the plan row for row, and reads every file it copied back against its
     SHA-256. Then it says READY TO WRITE.

It decides nothing about the write's placement: the page does, and this only compares. It is the same work as the
Windows helper, open-field-helper.ps1, with the same files in the shared folder and the same lines.

The unit over USB. The unit shows its disk over MTP (Media Transfer Protocol). The helper speaks MTP itself, through
the system's library libusb-1.0 (on Debian and Ubuntu the package libusb-1.0-0, which is usually there); nothing is
installed. It changes a unit only when the unit's USB product id is in the helper's list KNOWN_PRODUCTS (0x0102).
With any other id the helper only reads from the unit: a full run finds the unit and stops before it takes anything
off, with a line that says so. --list-only prints what the helper sees of the unit (its ids, its MTP interface, every
file and folder) and changes nothing, on the unit or on this computer: send what it prints to the open-field project.

What has run on a unit. One unit, on OS 1.7.3 with the bootloader 1000.1.1, from one computer with Linux Mint. Its USB
id was 2367:0102 in the normal disk mode and in the bootloader's drive mode. The helper listed its disk, read only, in
both modes. That computer needed no udev rule. One whole install then ran: the helper copied the original OS 1.7.3
file off, took the log files off, put os.ldr, a pad file, the folder user/pk, four feature packs and base.pack on the
unit, and read each back with the plan's SHA-256. The page agreed that the write could be made. The owner made the
write with the unit's keys, and the unit started with open-field.

Not tried on a unit: any other unit or distribution, a second install over an earlier one (os.ldr or a pack replaced,
old pad files taken off), the way back, a unit whose drive the desktop's file manager holds, a computer that needs the
udev rule. These paths of the helper have run on stand-ins only, in its tests.

Two things on Linux can stand in the way, and the helper says which:
  - The computer does not let you open the unit. A udev rule gives the device to the person at the screen. Put the
    line below into a file in /etc/udev/rules.d/ (for example 70-op1-field.rules), run sudo udevadm control --reload,
    then unplug the cable and plug it in again:
      SUBSYSTEM=="usb", ATTR{idVendor}=="2367", TAG+="uaccess"
  - The desktop's file manager holds the unit: it opens an MTP device by itself. Unmount "OP-1" there (the eject sign
    beside it in Files), or find its mtp:// address with gio mount -l and unmount it with gio mount -u mtp://...
    Leave the file manager away from the unit until the helper says READY TO WRITE: a program that adds a file to the
    unit's disk spoils the plan.

What it changes on the unit, and nothing else: it takes off the files directly in "logs", and the pad files of
earlier installs (open-field-pad.bin, open-field-pad-2.bin, ...) at the top level. It writes os.ldr at the top level,
the pad file the plan names, the folder user/pk when the plan says so, and the pack files the plan names into user/pk.
Every one of these changes passes one gate first (the class Gate), which refuses anything else. It never deletes a
folder, and it writes nothing before it has listed the disk again and found it as the plan was made from. It works
only on the bootloader's raw disk: the unit's normal disk mode has the same USB product id, so a disk with no folder
user or upgrade at its top stops the run before anything is read off it.

On this computer, each folder of a run in the shared folder (to-unit-, read-back-, incoming-, logs-, old-pads-, with
the run's time) is made new, and so is every file the helper puts in one: anything of that name already there, a link
included, stops the run.

--page, or the environment's OPEN_FIELD_PAGE, is the install page's mark: the page writes it into the folder it chose
(page.json), and this checks it before anything else, so that the two never work in two different folders. Without a
mark nothing is checked. --wait-minutes is how long it waits for the plan (60), --find-seconds how long for the unit
(180).

The exit status: 0 ready to write (with --list-only: listed), 1 stopped (the lines say why), 2 the mark was refused,
the command line is wrong, or the system is not Linux.
Standard library only; Python 3.8 or later. The page serves this as one file, so everything is in it. It runs as well
when Python reads it from standard input (python3 -I - --page <mark>): it never reads standard input itself.
"""

import argparse
import collections
import contextlib
import ctypes as ct
import ctypes.util
import datetime
import hashlib
import json
import math
import os
import re
import shutil
import stat
import struct
import subprocess
import sys
import time

HELPER_VERSION = 1
# The original OS 1.7.3 file, as the builder's manifest names it: the only file kept under the name "yours".
ORIGINAL_SHA = "d134eb1ac7ad88e8370d988329cd5d7419f016005b1d269034f9cf7166fad0ec"
NAMES = {"stock": "os-1.7.3-yours.ldr", "image": "open-field-os.ldr", "helper": "helper.json", "before": "listing-before.json",
         "after": "listing-after.json", "plan": "plan.json", "packs": "packs", "page": "page.json"}
STOCK_FROM = "upgrade/firmware/blackfin/bin/os.ldr"     # the owner's own firmware file on the unit: read, never written
PAD = re.compile(r"open-field-pad(-[0-9]+)?\.bin\Z", re.IGNORECASE)    # the name of a pad file the helper puts on
PACK_FOLDER = "user/pk"                                 # where the pack files go
PACK_NAME_BAD = re.compile(r'[\\/:*?"<>|\x00-\x1f]')    # a character a file name on the unit cannot have (install.mjs)
SECOND = 1.0            # the length of a second, in seconds, for every wait and pause: a test makes it shorter

MORE_THAN_ONE = "more than one OP-1 is connected: connect only the unit you are installing on"
NOT_SHOWN = ("the unit did not show on this computer. Hold COM while switching it on, press 3, then 2, with the cable in; "
             "then run the command again")
WAITING = "Waiting for the unit: on the unit, in the bootloader menu, press 3, then 2. The cable must be in."
NOT_BOOTLOADER = ("the drive that showed is not the bootloader's drive: it has no folder user or upgrade at its top. "
                  "If the unit is in its normal disk mode, switch it off, hold COM while switching it on, press 3, then 2; "
                  "then run the command again")
BOOTLOADER_TOP = ("user", "upgrade")    # the bootloader's raw disk has at least one of these folders at its top
NOT_LINUX = ("STOPPED: this helper is for Linux, and this system is not Linux. On Windows, use the PowerShell helper, "
             "open-field-helper.ps1. Nothing was done.")
NOTHING_MORE = "Nothing more is done. The unit is as this line finds it; read the lines above. Run the command again to start over."


# ---------------------------------------------------------------- the unit's disk

class DiskError(Exception):
    """A disk could not do what was asked. Its text is a plain sentence for the owner. `kind` says which failure it
    is: the transport over USB gives one of absent, several, permission, busy, gone, refused, reply, local, usage,
    library, unproven, broken, usb; anything else is "other"."""

    def __init__(self, message, kind="other"):
        super().__init__(message)
        self.kind = kind


class Disk:
    """The unit's one storage, as the helper uses it. A path is names joined with "/"; "" is the top.

    The stand-in (FolderDisk) and the unit over USB (UsbDisk) both have these methods, and nothing else is used.
    """

    name = ""       # what helper.json says in "unit"

    def children(self, folder_path):
        """The direct children of a folder: [(name, is_dir, size)]. A file's size is its exact byte count, a folder's 0.
        DiskError when folder_path is not a folder."""
        raise NotImplementedError

    def read(self, path, local_path):
        """Copies the file at path to local_path, a file made new (anything of that name there, a link included,
        is a DiskError), and gives its byte count."""
        raise NotImplementedError

    def write(self, local_path, folder_path, name):
        """Sends a local file into a folder under that name. A file of exactly that name there is removed first.
        Nothing else is touched."""
        raise NotImplementedError

    def make_folder(self, folder_path, name):
        raise NotImplementedError

    def remove_file(self, path):
        """Removes a file. A folder is refused."""
        raise NotImplementedError

    def close(self):
        pass


def join(folder, name):
    return folder + "/" + name if folder else name


def new_file(path):
    """A file of this computer made new for writing: it fails when anything of that name is there, a link included."""
    return open(os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o644), "wb")


class FolderDisk(Disk):
    """A folder of this computer that stands for the unit's disk (--source): the check of this helper with no unit.

    Its entries are the folder's own, dot files included, with their real sizes. A symbolic link is never followed:
    it is listed as a file of the link's own size, and reading through it is refused.
    """

    def __init__(self, root, shown_as=None):
        if not os.path.isdir(root) or os.path.islink(root):
            raise DiskError("no such folder: " + root)
        self.root = os.path.realpath(root)
        self.name = "a folder standing for the unit: " + (root if shown_as is None else shown_as)

    def _at(self, path):
        parts = [p for p in path.split("/") if p != ""] if path else []
        if any(p in (".", "..") for p in parts):
            raise DiskError("the unit has no such place: " + path)
        local = os.path.join(self.root, *parts)
        if os.path.realpath(local) != local:        # a link on the way: not followed
            raise DiskError("the unit has no such place: " + path)
        return local

    def children(self, folder_path):
        local = self._at(folder_path)
        if not os.path.isdir(local):
            raise DiskError("the unit has no folder " + folder_path)
        out = []
        with os.scandir(local) as entries:
            for e in entries:
                is_dir = e.is_dir(follow_symlinks=False)
                out.append((e.name, is_dir, 0 if is_dir else e.stat(follow_symlinks=False).st_size))
        out.sort(key=lambda c: c[0])
        return out

    def read(self, path, local_path):
        local = self._at(path)
        if os.path.islink(local) or not os.path.isfile(local):
            raise DiskError("the unit has no file " + path)
        try:
            dst = new_file(local_path)
        except OSError as e:
            raise DiskError("the copy of " + path + " could not be made new on this computer (" + str(e.strerror) + ")", "local")
        try:
            with open(local, "rb") as src, dst:
                shutil.copyfileobj(src, dst, 1 << 20)
        except BaseException:
            with contextlib.suppress(OSError):
                os.remove(local_path)
            raise
        return os.path.getsize(local_path)

    def write(self, local_path, folder_path, name):
        target = os.path.join(self._at(folder_path), name)
        if os.path.isdir(target) and not os.path.islink(target):
            raise DiskError("the unit has a folder where the file " + join(folder_path, name) + " goes")
        if os.path.lexists(target):
            os.unlink(target)
        with open(local_path, "rb") as src, open(target, "xb") as dst:
            shutil.copyfileobj(src, dst, 1 << 20)

    def make_folder(self, folder_path, name):
        try:
            os.mkdir(os.path.join(self._at(folder_path), name))
        except OSError as e:
            raise DiskError("the folder " + join(folder_path, name) + " could not be made (" + e.strerror + ")")

    def remove_file(self, path):
        local = self._at(path)
        if os.path.isdir(local) and not os.path.islink(local):
            raise DiskError(path + " is a folder, and a folder is never removed")
        os.unlink(local)


# ---------------------------------------------------------------- the unit over USB
# The transport to the unit in its bootloader's disk mode: MTP through libusb-1.0, with nothing but Python's own
# library. find_unit() below is how the helper asks for it.
#
# MTP (Media Transfer Protocol) is how the unit shows its disk in the bootloader's disk mode (hold COM while switching
# it on, press 3, then 2). libusb-1.0 is the system's library for talking to a USB device; it is loaded with ctypes,
# so no package is installed. Debian and Ubuntu have it as libusb-1.0-0, which is usually there.
#
# UsbDisk offers the helper's Disk interface and nothing more: list one folder, copy one file off, send one file,
# make one folder, remove one file. A path is names joined with "/"; "" is the top of the unit's first storage. Names
# are matched exactly: capitals count. A file is sent by removing a file of exactly that name first, never a folder.
# No other change is made to the unit, and usb_list_only sends no operation that changes anything.
#
# An object is removed by one function only, _delete. It asks the unit about the object again right before, and
# removes it only when it is a plain file of the expected name in the expected folder, and not lost+found. The handles
# 0 and 0xFFFFFFFF are never taken from the unit: to MTP they mean "no object" and "every object".
#
# The unit is found by the vendor id of teenage engineering and an interface that speaks MTP. When a unit's product id
# is not in KNOWN_PRODUCTS, the transport only reads from it: listing and copying off work, and sending, making a
# folder and removing a file are refused.
#
# Every failure is a DiskError with a plain message for the owner, and a `kind` the helper can act on:
# absent, several, permission, busy, gone, refused, reply, local, usage, library, unproven, broken, usb.
#
# The transport is ported from open-xy's tooling/usbmtp.py at commit f10f6bd.
# Copyright (c) 2026 Open-XY contributors. MIT licence: builder/open-xy.LICENSE.
# Changed here: the Disk interface by paths with a cache of handles; files streamed to and from the computer's disk;
# the 64-bit size property; the device found without opening it first, so a missing permission and a busy interface
# are told apart; an MTP interface needs its three endpoints; messages for the OP-1 Field on Linux; a read-only mode.

# The unit.
VENDOR = 0x2367          # teenage engineering
# The product ids the helper may change a unit with. 0x0102 is the id of the one unit this helper has run on (OS 1.7.3):
# it listed that unit's disk in both disk modes and made one whole install on it. Every other path has run on stand-ins
# only. The same id shows in the unit's normal disk mode, so the id alone never lets a run go on: bootloader_drive()
# decides that.
KNOWN_PRODUCTS = (0x0102,)
UNIT_NAME = 'OP-1'       # What helper.json says in "unit", as the Windows helper does.

# MTP containers and operations.
ROOT = 0xFFFFFFFF        # The parent handle of the top of a storage.
NO_OBJECT = 0            # A handle that names no object.
NOT_HANDLES = (NO_OBJECT, ROOT)   # Never the handle of a listed object; DeleteObject(ROOT) removes every object.
SPECIAL_NAMES = ('lost+found',)   # Shown by the unit, never removed by the helper.
COMMAND, DATA, RESPONSE = 1, 2, 3
RESPONSE_LIMIT = 5 * 4   # An answer carries at most five parameters.
OK, SESSION_ALREADY_OPEN = 0x2001, 0x201E
OPEN_SESSION, CLOSE_SESSION, STORAGE_IDS = 0x1002, 0x1003, 0x1004
OBJECT_HANDLES, OBJECT_INFO, GET_OBJECT = 0x1007, 0x1008, 0x1009
DELETE_OBJECT, SEND_OBJECT_INFO, SEND_OBJECT = 0x100B, 0x100C, 0x100D
GET_OBJECT_PROP_VALUE = 0x9803
OBJECT_SIZE_PROPERTY = 0xDC04   # The 64-bit size of an object.
READING = frozenset((OPEN_SESSION, CLOSE_SESSION, STORAGE_IDS, OBJECT_HANDLES, OBJECT_INFO, GET_OBJECT,
                     GET_OBJECT_PROP_VALUE))   # The operations that change nothing on the unit.
UNDEFINED_FORMAT, FOLDER_FORMAT = 0x3000, 0x3001   # Any file; a folder (an association).
LARGE = 0xFFFFFFFF       # A 32-bit size or container length that means "4 GiB or more".
CHUNK = 1 << 20          # One bulk transfer; a multiple of every packet size, so only a last transfer is short.
TIMEOUT_MS = 20000       # For one bulk transfer.
HEADER = struct.Struct('<IHHI')   # A container: length, kind, code, transaction.
ANSWERS = {0x2002: 'it gave a general error', 0x2005: 'it does not do that operation',
           0x2008: 'it does not know that storage', 0x2009: 'it no longer knows that object',
           0x200C: 'its disk is full', 0x200D: 'its disk is read-only', 0x200F: 'it refused access',
           0x2019: 'it is busy', 0x201A: 'the folder is not a folder to it', 0xA80A: 'it does not know that property'}

# libusb.
LIBUSB_IO, LIBUSB_ACCESS, LIBUSB_NO_DEVICE, LIBUSB_NOT_FOUND = -1, -3, -4, -5
LIBUSB_BUSY, LIBUSB_TIMEOUT, LIBUSB_PIPE = -6, -7, -9
LIBUSB_ERRORS = {-1: 'input/output error', -2: 'invalid parameter', -3: 'access denied', -4: 'no such device',
                 -5: 'not found', -6: 'busy', -7: 'timed out', -8: 'overflow', -9: 'stalled',
                 -10: 'interrupted', -11: 'out of memory', -12: 'not supported'}
LIBRARY_NAMES = ('libusb-1.0.so.0', 'libusb-1.0.so',
                 '/opt/homebrew/lib/libusb-1.0.0.dylib', '/usr/local/lib/libusb-1.0.0.dylib')   # the last two: macOS, Homebrew

# What the owner reads.
NOT_FOUND = NOT_SHOWN
SEVERAL = MORE_THAN_ONE
PERMISSION = ('the unit is connected (bus {bus:03d}, device {address:03d}, product id {product:04x}), but this user '
              'may not open it on this computer. Linux needs a udev rule that gives the device to the person at the '
              'screen: put the line\n'
              '  SUBSYSTEM=="usb", ATTR{{idVendor}}=="2367", TAG+="uaccess"\n'
              'into a file in /etc/udev/rules.d/ (for example 70-op1-field.rules), run sudo udevadm control --reload, '
              'unplug the cable and plug it in again; then run the command again')
BUSY = ('the desktop\'s file manager holds the unit: it opens an MTP device by itself, and then the helper cannot. '
        'Unmount "OP-1" there (the eject sign beside it in Files), or run gio mount -l to see its mtp:// address '
        'and unmount it with gio mount -u mtp://... (that address); then run the command again')
MAC = sys.platform == 'darwin'
if MAC:   # macOS: no udev; Image Capture's ptpcamerad, or an MTP app, takes the device instead.
    PERMISSION = BUSY = ('the unit is connected (bus {bus:03d}, device {address:03d}, product id {product:04x}), but another program '
                         'holds it. Quit Image Capture, Photos, OpenMTP and Android File Transfer, run killall ptpcamerad, '
                         'then run the command again')
GONE = ('the unit went away while the helper was working with it ({action}). Check the cable and that the unit '
        'still shows the bootloader\'s disk mode. The unit is as this moment left it')
SILENT = ('the unit stopped answering ({action}). Check the cable and that the unit still shows the bootloader\'s '
          'disk mode. The unit is as this moment left it')
UNPROVEN = ('this unit\'s USB product id is 0x{product:04x}. This helper has run on a unit with the id 0x0102 only, '
            'so it only reads from this one and changes nothing. Run the helper with --list-only and send what it '
            'prints to the open-field project')
BROKEN = ('the session with the unit broke at an earlier failure, and the helper sends nothing more in it. Run the '
          'command again')


def require(condition, message, kind='reply'):
    if not condition:
        raise DiskError(message, kind)


# MTP data.

def words(values):
    return struct.pack('<%dI' % len(values), *values)


def container(kind, code, transaction, payload=b''):
    return HEADER.pack(12 + len(payload), kind, code, transaction) + payload


def ptp_string(value):
    """A PTP string: a count of UTF-16 units with the ending zero, then UTF-16LE. The empty string is one zero byte."""
    text = (value + '\0').encode('utf-16-le') if value else b''
    require(len(text) // 2 <= 255, 'the name %s is too long for the unit (254 letters at most)' % value, 'usage')
    return bytes([len(text) // 2]) + text


def read_string(data, at):
    require(at < len(data), 'the unit sent an object description that is cut short')
    count = data[at]
    require(at + 1 + 2 * count <= len(data), 'the unit sent an object description that is cut short')
    try:
        return data[at + 1:at + 1 + 2 * count].decode('utf-16-le').rstrip('\0')
    except UnicodeDecodeError:
        raise DiskError('the unit sent a name that is not readable text')


def object_info(storage, name, size, parent, folder=False):
    """An ObjectInfo: 52 bytes of fixed fields, the file name, then three empty strings (dates and keywords).
    A folder is an association of type 1 (a generic folder) with no data."""
    fixed = bytearray(52)
    struct.pack_into('<IHxxI', fixed, 0, storage, FOLDER_FORMAT if folder else UNDEFINED_FORMAT, size)
    struct.pack_into('<I', fixed, 38, parent)
    if folder:
        struct.pack_into('<H', fixed, 42, 1)
    return bytes(fixed) + ptp_string(name) + ptp_string('') * 3


def read_words(data):
    """A list of 32-bit numbers: a count, then the numbers."""
    require(data is not None and len(data) >= 4, 'the unit sent a list that is cut short')
    count = struct.unpack_from('<I', data)[0]
    require(len(data) >= 4 + 4 * count, 'the unit sent a list that is cut short')
    return list(struct.unpack_from('<%dI' % count, data, 4))


def read_handles(data, what):
    """A list of handles or storage ids from the unit. 0 and 0xFFFFFFFF are refused: they name no single object."""
    values = read_words(data)
    for value in values:
        require(value not in NOT_HANDLES, 'the unit listed %s with the number %#010x, which names no single one; '
                'the helper stops here and changes nothing' % (what, value))
    require(len(set(values)) == len(values), 'the unit listed %s with one number twice' % what)
    return values


class ObjectInfo(collections.namedtuple('ObjectInfo', 'format size parent association name')):
    """What the unit says of one object: its format, 32-bit size, parent handle, association type and name."""

    @property
    def folder(self):
        """A folder is an association, or anything of association type 1 (a generic folder)."""
        return self.format == FOLDER_FORMAT or self.association == 1


def read_object_info(data):
    require(data is not None and len(data) >= 53, 'the unit sent an object description that is cut short')
    kind, = struct.unpack_from('<H', data, 4)
    size, = struct.unpack_from('<I', data, 8)
    parent, association = struct.unpack_from('<IH', data, 38)
    return ObjectInfo(kind, size, parent, association, read_string(data, 52))


def check_name(name, where='the helper'):
    """A name of one file or folder: not empty, no "/", no NUL, not "." or ".."."""
    require(isinstance(name, str) and name not in ('', '.', '..') and '/' not in name and '\0' not in name,
            '%s names %r, which is not a name of a file on the unit' % (where, name), 'usage')
    return name


def split_path(path):
    return [check_name(name) for name in path.split('/')] if path else []


def answer(code, action):
    """The words for an MTP answer that is not OK."""
    meaning = ANSWERS.get(code, 'it gave an answer the helper does not know')
    return 'the unit refused to %s: %s (MTP answer %#06x)' % (action, meaning, code)


# USB.

def mtp_interface(interfaces):
    """(number, bulk in, bulk out, packet size, (class, subclass, protocol)) of the first interface that speaks MTP,
    or None. It is still-image class 6/1/1, or vendor-specific 0xff, with three endpoints: bulk in, bulk out and
    interrupt in. `interfaces` holds (number, alternate, class, subclass, protocol, [(address, kind, packet)])."""
    for number, alternate, klass, subclass, protocol, endpoints in interfaces:
        if alternate != 0 or ((klass, subclass, protocol) != (6, 1, 1) and klass != 0xFF):
            continue
        bulk_in = [e for e in endpoints if e[1] == 2 and e[0] & 0x80]
        bulk_out = [e for e in endpoints if e[1] == 2 and not e[0] & 0x80]
        interrupt_in = [e for e in endpoints if e[1] == 3 and e[0] & 0x80]
        if len(endpoints) == 3 and bulk_in and bulk_out and interrupt_in:
            return number, bulk_in[0][0], bulk_out[0][0], bulk_out[0][2] or 512, (klass, subclass, protocol)
    return None


# One USB device as libusb lists it. `interfaces` is None when its descriptors could not be read.
UsbDevice = collections.namedtuple('UsbDevice', 'ref vendor product bus address interfaces configuration')


class EndpointDescriptor(ct.Structure):
    _fields_ = [('bLength', ct.c_uint8), ('bDescriptorType', ct.c_uint8), ('bEndpointAddress', ct.c_uint8),
                ('bmAttributes', ct.c_uint8), ('wMaxPacketSize', ct.c_uint16), ('bInterval', ct.c_uint8),
                ('bRefresh', ct.c_uint8), ('bSynchAddress', ct.c_uint8), ('extra', ct.c_void_p),
                ('extra_length', ct.c_int)]


class InterfaceDescriptor(ct.Structure):
    _fields_ = [('bLength', ct.c_uint8), ('bDescriptorType', ct.c_uint8), ('bInterfaceNumber', ct.c_uint8),
                ('bAlternateSetting', ct.c_uint8), ('bNumEndpoints', ct.c_uint8), ('bInterfaceClass', ct.c_uint8),
                ('bInterfaceSubClass', ct.c_uint8), ('bInterfaceProtocol', ct.c_uint8), ('iInterface', ct.c_uint8),
                ('endpoint', ct.POINTER(EndpointDescriptor)), ('extra', ct.c_void_p), ('extra_length', ct.c_int)]


class Interface(ct.Structure):
    _fields_ = [('altsetting', ct.POINTER(InterfaceDescriptor)), ('num_altsetting', ct.c_int)]


class ConfigDescriptor(ct.Structure):
    _fields_ = [('bLength', ct.c_uint8), ('bDescriptorType', ct.c_uint8), ('wTotalLength', ct.c_uint16),
                ('bNumInterfaces', ct.c_uint8), ('bConfigurationValue', ct.c_uint8), ('iConfiguration', ct.c_uint8),
                ('bmAttributes', ct.c_uint8), ('MaxPower', ct.c_uint8), ('interface', ct.POINTER(Interface)),
                ('extra', ct.c_void_p), ('extra_length', ct.c_int)]


def interfaces_of(config):
    """The interfaces of a libusb configuration descriptor, in the form mtp_interface takes."""
    result = []
    for i in range(config.bNumInterfaces):
        interface = config.interface[i]
        for a in range(interface.num_altsetting):
            d = interface.altsetting[a]
            endpoints = [(e.bEndpointAddress, e.bmAttributes & 3, e.wMaxPacketSize & 0x7FF)
                         for e in (d.endpoint[k] for k in range(d.bNumEndpoints))]
            result.append((d.bInterfaceNumber, d.bAlternateSetting, d.bInterfaceClass, d.bInterfaceSubClass,
                           d.bInterfaceProtocol, endpoints))
    return result


def load_libusb():
    for name in (ctypes.util.find_library('usb-1.0'),) + LIBRARY_NAMES:
        if name:
            with contextlib.suppress(OSError):
                return ct.CDLL(name)
    raise DiskError('the helper needs the library libusb-1.0, and this computer does not have it. '
                    'On Debian and Ubuntu: sudo apt install libusb-1.0-0', 'library')


class Libusb:
    """libusb-1.0 through ctypes, as plain calls that answer libusb's codes. Tests put a stand-in in its place."""

    def __init__(self):
        self.lib = lib = load_libusb()
        void, pointer = ct.c_void_p, ct.POINTER
        lib.libusb_init.argtypes = [pointer(void)]
        lib.libusb_exit.argtypes = [void]
        lib.libusb_exit.restype = None
        lib.libusb_get_device_list.argtypes = [void, pointer(pointer(void))]
        lib.libusb_get_device_list.restype = ct.c_ssize_t
        lib.libusb_free_device_list.argtypes = [pointer(void), ct.c_int]
        lib.libusb_free_device_list.restype = None
        lib.libusb_get_device_descriptor.argtypes = [void, void]
        lib.libusb_get_bus_number.argtypes = lib.libusb_get_device_address.argtypes = [void]
        lib.libusb_get_bus_number.restype = lib.libusb_get_device_address.restype = ct.c_uint8
        lib.libusb_get_active_config_descriptor.argtypes = [void, pointer(pointer(ConfigDescriptor))]
        lib.libusb_get_config_descriptor.argtypes = [void, ct.c_uint8, pointer(pointer(ConfigDescriptor))]
        lib.libusb_free_config_descriptor.argtypes = [pointer(ConfigDescriptor)]
        lib.libusb_free_config_descriptor.restype = None
        lib.libusb_open.argtypes = [void, pointer(void)]
        lib.libusb_close.argtypes = [void]
        lib.libusb_close.restype = None
        lib.libusb_get_configuration.argtypes = [void, pointer(ct.c_int)]
        lib.libusb_set_configuration.argtypes = [void, ct.c_int]
        lib.libusb_set_auto_detach_kernel_driver.argtypes = [void, ct.c_int]
        lib.libusb_claim_interface.argtypes = lib.libusb_release_interface.argtypes = [void, ct.c_int]
        lib.libusb_clear_halt.argtypes = [void, ct.c_ubyte]
        lib.libusb_bulk_transfer.argtypes = [void, ct.c_ubyte, void, ct.c_int, pointer(ct.c_int), ct.c_uint]
        lib.libusb_control_transfer.argtypes = [void, ct.c_ubyte, ct.c_ubyte, ct.c_ushort, ct.c_ushort,
                                                void, ct.c_ushort, ct.c_uint]
        self.buffer = (ct.c_ubyte * CHUNK)()
        self.listing = None
        context = ct.c_void_p()
        code = lib.libusb_init(ct.byref(context))
        require(code == 0, 'libusb did not start: %s' % LIBUSB_ERRORS.get(code, code), 'library')
        self.context = context

    def devices(self):
        """Every USB device, with the descriptors of those of teenage engineering. Kept until free_devices()."""
        listing = ct.POINTER(ct.c_void_p)()
        count = self.lib.libusb_get_device_list(self.context, ct.byref(listing))
        require(count >= 0, 'libusb could not list the USB devices: %s' % LIBUSB_ERRORS.get(count, count), 'library')
        self.listing = listing
        result = []
        for index in range(count):
            ref = listing[index]
            descriptor = ct.create_string_buffer(18)
            if self.lib.libusb_get_device_descriptor(ref, descriptor) != 0:
                continue
            vendor, product = struct.unpack_from('<HH', descriptor.raw, 8)
            interfaces, configuration = (self._interfaces(ref) if vendor == VENDOR else (None, 0))
            result.append(UsbDevice(ref, vendor, product, self.lib.libusb_get_bus_number(ref),
                                    self.lib.libusb_get_device_address(ref), interfaces, configuration))
        return result

    def _interfaces(self, ref):
        """Read from the system's copy of the descriptors: the device is not opened for this."""
        config = ct.POINTER(ConfigDescriptor)()
        code = self.lib.libusb_get_active_config_descriptor(ref, ct.byref(config))
        if code == LIBUSB_NOT_FOUND:   # Not configured yet: its first configuration.
            code = self.lib.libusb_get_config_descriptor(ref, 0, ct.byref(config))
        if code != 0:
            return None, 0
        try:
            return interfaces_of(config.contents), config.contents.bConfigurationValue
        finally:
            self.lib.libusb_free_config_descriptor(config)

    def free_devices(self):
        listing, self.listing = self.listing, None
        if listing is not None:
            self.lib.libusb_free_device_list(listing, 1)

    def open(self, ref):
        handle = ct.c_void_p()
        return self.lib.libusb_open(ref, ct.byref(handle)), handle

    def configuration(self, handle):
        value = ct.c_int()
        return self.lib.libusb_get_configuration(handle, ct.byref(value)), value.value

    def set_configuration(self, handle, value):
        return self.lib.libusb_set_configuration(handle, value)

    def auto_detach(self, handle):
        return self.lib.libusb_set_auto_detach_kernel_driver(handle, 1)

    def claim(self, handle, number):
        return self.lib.libusb_claim_interface(handle, number)

    def release(self, handle, number):
        return self.lib.libusb_release_interface(handle, number)

    def close(self, handle):
        self.lib.libusb_close(handle)

    def clear_halt(self, handle, endpoint):
        return self.lib.libusb_clear_halt(handle, endpoint)

    def bulk_out(self, handle, endpoint, data, timeout):
        ct.memmove(self.buffer, data, len(data))
        count = ct.c_int()
        code = self.lib.libusb_bulk_transfer(handle, endpoint, self.buffer, len(data), ct.byref(count), timeout)
        return code, count.value

    def bulk_in(self, handle, endpoint, length, timeout):
        """The bytes that came in are given back even with an error code, so a part of a transfer is seen."""
        count = ct.c_int()
        code = self.lib.libusb_bulk_transfer(handle, endpoint, self.buffer, length, ct.byref(count), timeout)
        return code, ct.string_at(self.buffer, max(0, min(count.value, length)))

    def control(self, handle, request_type, request, value, index, data=b'', length=0, timeout=2000):
        size = max(length, len(data))
        buffer = ct.create_string_buffer(data, size) if size else None
        code = self.lib.libusb_control_transfer(handle, request_type, request, value, index, buffer, size, timeout)
        return code, (buffer.raw[:code] if buffer is not None and code > 0 else b'')

    def exit(self):
        self.free_devices()
        context, self.context = self.context, None
        if context is not None:
            self.lib.libusb_exit(context)


class Usb:
    """The bulk transfers to the one unit's MTP interface, claimed through libusb.
    `libusb` is for tests: an object with the calls of Libusb."""

    def __init__(self, libusb=None):
        self.libusb = libusb if libusb is not None else Libusb()
        self.handle = self.number = None
        try:
            self._open()
        except BaseException:
            self.close()
            raise

    def _failure(self, code, action):
        """A libusb code as the DiskError the owner reads."""
        if code == LIBUSB_ACCESS:
            return DiskError(PERMISSION.format(bus=self.bus, address=self.address, product=self.product), 'permission')
        if code == LIBUSB_BUSY:
            return DiskError(BUSY.format(bus=self.bus, address=self.address, product=self.product), 'busy')
        if code in (LIBUSB_NO_DEVICE, LIBUSB_IO):
            return DiskError(GONE.format(action=action), 'gone')
        if code == LIBUSB_TIMEOUT:
            return DiskError(SILENT.format(action=action), 'gone')
        return DiskError('%s failed: libusb says %s' % (action, LIBUSB_ERRORS.get(code, 'error %d' % code)), 'usb')

    def _open(self):
        libusb = self.libusb
        try:
            found, unreadable = [], []
            for device in libusb.devices():
                if device.vendor != VENDOR:   # Any product id: one not in KNOWN_PRODUCTS is only read (UsbDisk).
                    continue
                if device.interfaces is None:
                    unreadable.append(device)
                    continue
                layout = mtp_interface(device.interfaces)
                if layout is not None:
                    found.append((device, layout))
            if not found:
                message = NOT_FOUND
                for device in unreadable:
                    message += ('. (A device of teenage engineering is on bus %03d, device %03d, but this computer '
                                'could not read what it offers.)' % (device.bus, device.address))
                raise DiskError(message, 'absent')
            require(len(found) == 1, SEVERAL, 'several')
            device, (self.number_wanted, self.inbound, self.outbound, self.packet, self.triple) = found[0]
            self.vendor, self.product, self.bus, self.address = device.vendor, device.product, device.bus, device.address
            code, handle = libusb.open(device.ref)
            if code != 0:
                raise self._failure(code, 'opening the unit')
            self.handle = handle
        finally:
            libusb.free_devices()
        code, active = libusb.configuration(self.handle)
        if code != 0:
            raise self._failure(code, 'reading the unit\'s configuration')
        if active == 0:
            code = libusb.set_configuration(self.handle, device.configuration)
            if code != 0:
                raise self._failure(code, 'choosing the unit\'s configuration')
        libusb.auto_detach(self.handle)   # Only Linux has kernel drivers to detach; none is expected on MTP.
        code = libusb.claim(self.handle, self.number_wanted)
        for _ in range(5 if MAC else 0):   # macOS's ptpcamerad grabs MTP devices and launchd restarts it: kill, retry.
            if code not in (LIBUSB_ACCESS, LIBUSB_BUSY):
                break
            subprocess.run(['killall', 'ptpcamerad'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(0.3)
            code = libusb.claim(self.handle, self.number_wanted)
        if code != 0:
            raise self._failure(code, 'taking the unit\'s MTP interface')
        self.number = self.number_wanted

    def _bulk(self, call, endpoint, payload, action):
        """One bulk transfer. `call` gives back a code and the byte count sent or the bytes received. A stalled
        endpoint is cleared and the transfer tried once more, but only when no byte of it went through: tried again
        after a part, that part would go twice or be lost. A healthy endpoint is never cleared, because that resets
        the data toggle and some devices then drop the next packet."""
        code, result = call(self.handle, endpoint, payload, TIMEOUT_MS)
        moved = result if isinstance(result, int) else len(result)
        if code == LIBUSB_PIPE and moved == 0:
            cleared = self.libusb.clear_halt(self.handle, endpoint)
            if cleared != 0:
                raise self._failure(cleared, action)
            code, result = call(self.handle, endpoint, payload, TIMEOUT_MS)
        if code != 0:
            raise self._failure(code, action)
        return result

    def write(self, data):
        count = self._bulk(self.libusb.bulk_out, self.outbound, data, 'sending to the unit')
        require(count == len(data), GONE.format(action='sending to the unit'), 'gone')

    def read(self):
        return self._bulk(self.libusb.bulk_in, self.inbound, CHUNK, 'receiving from the unit')

    def cancel(self, transaction):
        """Cancel Request (0x64) for the transaction, then Get Device Status (0x67) until the device says OK, as
        the still-image class asks before the next command. Failures are let pass: this runs after a failure."""
        if self.handle is None:
            return
        self.libusb.control(self.handle, 0x21, 0x64, 0, self.number_wanted,
                            data=struct.pack('<HI', 0x4001, transaction))
        for _ in range(20):
            code, status = self.libusb.control(self.handle, 0xA1, 0x67, 0, self.number_wanted, length=64)
            if code < 0 or len(status) < 4 or struct.unpack_from('<H', status, 2)[0] == OK:
                return
            time.sleep(0.1)

    def close(self):
        handle, self.handle = self.handle, None
        if handle is not None:
            if self.number is not None:
                self.libusb.release(handle, self.number)
                self.number = None
            self.libusb.close(handle)
        libusb, self.libusb = self.libusb, None
        if libusb is not None:
            libusb.exit()


# The disk.

# A cached object: its handle, whether it is a folder, its size (0 for a folder).
Entry = collections.namedtuple('Entry', 'handle dir size')


class UsbDisk(Disk):
    """The unit's first storage over MTP. `transport` is for tests: an object with write(bytes), read() -> bytes,
    cancel(transaction), close(), `packet` and `product`, as Usb has. `read_only` refuses, before sending, every
    operation that would change the unit; so does a product id that is not in KNOWN_PRODUCTS."""

    def __init__(self, transport=None, read_only=False):
        self.name = UNIT_NAME
        self.read_only = read_only
        self.transport = transport if transport is not None else Usb()
        self.transaction = 0
        self.storage = None
        self.storages = []
        self.broken = False
        self.session = False     # True once the unit has a session open: close() then ends it
        self._forget()
        try:
            opened = self._run(OPEN_SESSION, [1])
            if opened[0] != SESSION_ALREADY_OPEN:
                self._expect(opened, 'open a session')
            self.session = True
            self.storages = read_handles(self._expect(self._run(STORAGE_IDS), 'name its storages')[1], 'a storage')
            require(self.storages, 'the unit shows no storage')
            self.storage = self.storages[0]
        except BaseException:   # The session is closed and the interface given back before the error leaves.
            self.close()
            raise

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    # The session.

    def close(self):
        transport = self.transport
        if transport is None:
            return
        try:
            if self.session and not self.broken:   # After a failed transfer the session is left, not closed.
                with contextlib.suppress(Exception):
                    self._run(CLOSE_SESSION)
        finally:
            self.transport = None
            self.session = False
            transport.close()

    def _may_change(self):
        """Refuse a change, before anything is sent, on a unit whose product id is not in KNOWN_PRODUCTS."""
        product = getattr(self.transport, 'product', None)
        if product not in KNOWN_PRODUCTS:
            raise DiskError(UNPROVEN.format(product=product if isinstance(product, int) else 0), 'unproven')
        require(not self.read_only, 'the helper was opened to read only, and this would change the unit', 'usage')

    @staticmethod
    def _expect(result, action):
        if result[0] != OK:
            raise DiskError(answer(result[0], action), 'refused')
        return result

    def _send(self, pieces, total):
        """Bulk writes of at most CHUNK bytes; a container that ends on a packet's edge gets a zero-length packet."""
        for piece in pieces:
            self.transport.write(piece)
        if total % self.transport.packet == 0:
            self.transport.write(b'')

    def _file_pieces(self, code, transaction, source, size):
        """A data container streamed from an open file: exactly `size` bytes of it. A file that is shorter, or
        longer, than it was when its size was taken stops the send before its last piece goes out."""
        head = HEADER.pack(12 + size, DATA, code, transaction)
        left, first = size, True
        while first or left:
            want = min(CHUNK - 12 if first else CHUNK, left)
            piece = source.read(want) if want else b''
            require(len(piece) == want, 'the file on this computer got shorter while it was sent', 'local')
            left -= want
            if not left:
                require(not source.read(1), 'the file on this computer got longer while it was sent', 'local')
            yield head + piece if first else piece
            first = False

    def _receive(self, limit, sink=None):
        """(kind, code, transaction, payload) of one container from the unit. `limit` is the most data a data
        container may carry; an answer carries at most five parameters. With `sink`, a data container is written
        into it and the payload is its byte count; a data container of 4 GiB or more then ends with a short
        transfer."""
        first = b''
        while not first:
            first = self.transport.read()   # Passes over a zero-length packet left from the container before.
        require(len(first) >= 12, 'the unit sent a reply that is cut short')
        length, kind, code, transaction = HEADER.unpack_from(first)
        open_ended = kind == DATA and sink is not None and length == LARGE
        most = limit if kind == DATA else RESPONSE_LIMIT
        require(open_ended or (12 <= length <= most + 12 and len(first) <= length),
                'the unit sent a reply of a size the helper did not ask for')
        if kind == DATA and sink is not None:
            sink.write(first[12:])
            have, last = len(first), len(first)
            while (last == CHUNK) if open_ended else (have < length):
                more = self.transport.read()
                require(open_ended or (more and have + len(more) <= length), 'the unit sent more than it said')
                sink.write(more)
                have, last = have + len(more), len(more)
            return kind, code, transaction, have - 12
        parts, have = [first], len(first)
        while have < length:
            more = self.transport.read()
            require(more and have + len(more) <= length, 'the unit sent more than it said')
            parts.append(more)
            have += len(more)
        return kind, code, transaction, b''.join(parts)[12:]

    def _run(self, code, params=(), data=None, limit=16 << 20, sink=None):
        """One MTP transaction: (answer code, data phase or None, answer parameters). `data` is bytes or
        (open file, size). With `sink`, the data phase goes into it and its byte count is given back."""
        if code not in READING:
            self._may_change()
        require(self.transport is not None, 'the session with the unit is closed', 'usage')
        require(not self.broken, BROKEN, 'broken')
        transaction = self.transaction
        self.transaction += 1
        try:
            command = container(COMMAND, code, transaction, words(params))
            self._send([command], len(command))
            if isinstance(data, bytes):
                whole = container(DATA, code, transaction, data)
                self._send([whole[at:at + CHUNK] for at in range(0, len(whole), CHUNK)], len(whole))
            elif data is not None:
                source, size = data
                self._send(self._file_pieces(code, transaction, source, size), 12 + size)
            kind, reply, tid, payload = self._receive(limit, sink)
            phase = None
            if kind == DATA:
                require(tid == transaction and reply == code, 'the unit sent data the helper did not ask for')
                phase = payload
                kind, reply, tid, payload = self._receive(limit)
            require(kind == RESPONSE and tid == transaction, 'the unit sent a reply the helper did not ask for')
            return reply, phase, list(struct.unpack_from('<%dI' % (len(payload) // 4), payload))
        except BaseException:
            self.broken = True
            # Cancel the transaction so the unit is idle again, even after Ctrl+C.
            with contextlib.suppress(Exception):
                self.transport.cancel(transaction)
            raise

    # Objects and the cache.

    def _forget(self):
        """Drop the cache of handles: done after every change."""
        self._handles = {'': Entry(ROOT, True, 0)}

    def _info(self, handle, action):
        """The ObjectInfo of one object, asked of the unit now."""
        return read_object_info(self._expect(self._run(OBJECT_INFO, [handle]), action)[1])

    def _size64(self, handle, path):
        reply, data, _ = self._run(GET_OBJECT_PROP_VALUE, [handle, OBJECT_SIZE_PROPERTY])
        if reply != OK:
            raise DiskError('the unit shows %s as 4 GiB or more and does not tell its exact size (%s); the helper '
                            'cannot list the disk exactly' % (path, answer(reply, 'give its 64-bit size')), 'refused')
        require(data is not None and len(data) >= 8, 'the unit sent a size that is cut short')
        return struct.unpack_from('<Q', data)[0]

    def _list(self, folder_path, handle):
        """The direct children of a folder, read from the unit: [(name, Entry)]. The cache learns each of them."""
        action = 'list %s' % (folder_path or 'the top of its disk')
        handles = read_handles(self._expect(self._run(OBJECT_HANDLES, [self.storage, 0, handle]), action)[1],
                               'an object in %s' % (folder_path or 'the top of its disk'))
        result, seen = [], set()
        for child in handles:
            info = self._info(child, action)
            name = check_name(info.name, 'the unit, in %s,' % (folder_path or 'the top of its disk'))
            path = folder_path + '/' + name if folder_path else name
            require(name not in seen, 'the unit shows two objects named %s' % path)
            seen.add(name)
            size = info.size
            if info.folder:
                size = 0
            elif size == LARGE:
                size = self._size64(child, path)
            entry = Entry(child, info.folder, size)
            self._handles[path] = entry
            result.append((name, entry))
        return result

    def _find(self, path):
        """The cached entry of a path, listing the folders on its way that the cache does not know."""
        names = split_path(path)
        for depth in range(1, len(names) + 1):
            here = '/'.join(names[:depth])
            if here in self._handles:
                continue
            above = '/'.join(names[:depth - 1])
            parent = self._handles[above]
            require(parent.dir, 'on the unit, %s is a file, not a folder' % above, 'usage')
            self._list(above, parent.handle)
            require(here in self._handles, 'the unit has no %s' % here, 'usage')
        return self._handles[path]

    def _folder(self, folder_path):
        entry = self._find(folder_path)
        require(entry.dir, 'on the unit, %s is a file, not a folder' % folder_path, 'usage')
        return entry

    # The Disk interface.

    def children(self, folder_path):
        folder = self._folder(folder_path)
        return [(name, entry.dir, entry.size) for name, entry in self._list(folder_path, folder.handle)]

    def read(self, path, local_path):
        entry = self._find(path)
        require(path and not entry.dir, 'on the unit, %s is a folder, not a file' % (path or 'the top'), 'usage')
        try:
            sink = new_file(local_path)
        except OSError as error:
            raise DiskError('the copy of %s could not be made new on this computer: %s' % (path, error), 'local')
        try:
            try:
                with sink:
                    count = self._expect(self._run(GET_OBJECT, [entry.handle], limit=entry.size, sink=sink),
                                         'give %s' % path)[1]
            except OSError as error:
                raise DiskError('the copy of %s could not be written on this computer: %s' % (path, error), 'local')
            require(count == entry.size, '%s came off the unit with %s bytes; the unit lists %d'
                    % (path, count, entry.size))
        except BaseException:   # No part of a file is left behind as if it were the file; only the helper's own.
            with contextlib.suppress(OSError):
                os.remove(local_path)
            raise
        return count

    def _delete(self, handle, parent, name, path):
        """Remove one file: the only place that sends DeleteObject. `parent` is the handle of the folder it should
        be in. The unit is asked about the object again first, and anything but a plain file of exactly that name in
        exactly that folder is refused; so are lost+found and the handles 0 and 0xFFFFFFFF, whoever asks."""
        require(handle not in NOT_HANDLES, 'the helper never removes the object numbered %#010x (%s): to the unit '
                'that number names no single object' % (handle, path), 'usage')
        require(name not in SPECIAL_NAMES, 'the helper never removes %s' % path, 'usage')
        info = self._info(handle, 'describe %s' % path)
        require(not info.folder and info.format != FOLDER_FORMAT and info.association == 0,
                'the helper removes files only, and to the unit %s is a folder' % path, 'usage')
        require(info.name == name and info.parent == parent,
                'the unit changed: %s is no longer where it was listed, so nothing is removed' % path, 'usage')
        self._expect(self._run(DELETE_OBJECT, [handle, 0]), 'remove %s' % path)

    def write(self, local_path, folder_path, name):
        check_name(name)
        self._may_change()
        path = folder_path + '/' + name if folder_path else name
        try:
            size = os.path.getsize(local_path)
            source = open(local_path, 'rb')
        except OSError as error:
            raise DiskError('the file %s on this computer cannot be read: %s' % (local_path, error), 'local')
        with source:
            require(size <= LARGE - 12, '%s is too large to send over MTP (4 GiB or more)' % path, 'usage')
            folder = self._folder(folder_path)
            try:
                there = dict(self._list(folder_path, folder.handle))
                if name in there:
                    require(not there[name].dir, 'the unit has a folder named %s where the file goes' % path, 'usage')
                    self._delete(there[name].handle, folder.handle, name, path)
                    self._forget()
                info = object_info(self.storage, name, size, folder.handle)
                self._expect(self._run(SEND_OBJECT_INFO, [self.storage, folder.handle], info), 'take %s' % path)
                self._expect(self._run(SEND_OBJECT, [], (source, size)), 'store %s' % path)
            finally:
                self._forget()

    def make_folder(self, folder_path, name):
        check_name(name)
        self._may_change()
        path = folder_path + '/' + name if folder_path else name
        folder = self._folder(folder_path)
        try:
            there = dict(self._list(folder_path, folder.handle))
            require(name not in there, 'the unit already has a %s named %s'
                    % ('folder' if there.get(name, Entry(0, False, 0)).dir else 'file', path), 'usage')
            info = object_info(self.storage, name, 0, folder.handle, folder=True)
            self._expect(self._run(SEND_OBJECT_INFO, [self.storage, folder.handle], info), 'make the folder %s' % path)
        finally:
            self._forget()

    def remove_file(self, path):
        self._may_change()
        entry = self._find(path)
        require(path and not entry.dir, 'the helper removes files only, and %s is a folder' % (path or 'the top'),
                'usage')
        *above, name = path.split('/')
        parent = self._handles['/'.join(above)].handle   # Known: _find listed it on the way.
        try:
            self._delete(entry.handle, parent, name, path)
        finally:
            self._forget()


def usb_list_only(out, transport=None):
    """The helper's --list-only: open the unit, print what it is, everything on it and whether it looks like the
    bootloader's drive, close. The disk is opened to read only, so no operation that changes the unit can be sent."""
    disk = UsbDisk(transport, read_only=True)
    try:
        usb = disk.transport
        out.write('Vendor id %04x, product id %04x, bus %03d, device %03d.\n'
                  % (usb.vendor, usb.product, usb.bus, usb.address))
        out.write('MTP interface: class %02x, subclass %02x, protocol %02x.\n' % tuple(usb.triple))
        out.write('Storage ids: %s. The first is used.\n' % ', '.join('%08x' % s for s in disk.storages))
        rows = get_listing(disk)
        for row in rows:
            out.write(json.dumps(row, ensure_ascii=False, separators=(',', ':')) + '\n')
        folders = sum(1 for row in rows if row['dir'])
        out.write('%d files, %d folders.\n' % (len(rows) - folders, folders))
        top = [(row['path'], row['dir'], 0) for row in rows if '/' not in row['path']]
        if bootloader_drive(top):
            out.write('The drive looks like the bootloader\'s: it has a folder user or upgrade at its top.\n')
        else:
            out.write('The drive does not look like the bootloader\'s: it has no folder user or upgrade at its top. '
                      'A run of the helper stops on it.\n')
    finally:
        disk.close()


def find_unit():
    """The unit over USB as a UsbDisk, or None while no unit shows. DiskError when more than one shows (kind
    "several"), when this computer does not let the helper open it ("permission"), when another program holds it
    ("busy"), or when libusb is missing ("library")."""
    try:
        return UsbDisk()
    except DiskError as e:
        if e.kind == "absent":
            return None
        raise


def unit_read_only(disk):
    """The transport's plain text when it would refuse every change to this unit over USB, or None.
    A unit whose product id is not in KNOWN_PRODUCTS is only read. Nothing is sent to the unit to find this out."""
    if not isinstance(disk, UsbDisk):
        return None
    try:
        disk._may_change()
    except DiskError as e:
        return str(e)
    return None


# ---------------------------------------------------------------- end of the unit over USB


def bootloader_drive(top):
    """True when the rows of a disk's top, (name, is_dir, size), hold a folder named exactly user or upgrade: the
    bootloader's raw disk. The unit's normal disk mode has the same USB product id and shows other folders."""
    return any(is_dir and name in BOOTLOADER_TOP for name, is_dir, _ in top)


def wait_for_unit(seconds, note):
    """find_unit() asked every two seconds until a unit shows; `note` gets the waiting line once."""
    deadline = time.monotonic() + seconds * SECOND
    told = False
    while True:
        disk = find_unit()
        if disk is not None:
            return disk
        if time.monotonic() > deadline:
            raise DiskError(NOT_SHOWN)
        if not told:
            note(WAITING)
            told = True
        time.sleep(2 * SECOND)


class Gate:
    """The unit as the helper reaches it. Reading passes; every change (write, make_folder, remove_file) is held to
    the rules first, here and nowhere else, and anything else is refused before the disk is asked. A place written
    is named exactly, letter for letter: "user/PK" is not "user/pk".

    The rules, by phase:
      "taking off" (before the first listing): remove a file directly in the top-level folder logs, or a top-level
        file whose name is a pad file's;
      "putting" (only once the plan was checked and the disk listed again as the plan was made from): write os.ldr at
        the top, write the plan's pad file at the top, make user/pk when the plan says so, write a pack file the plan
        names into user/pk.
    In every other phase nothing changes.
    """

    def __init__(self, disk):
        self.disk = disk
        self.phase = "looking"
        self.logs = None                # the top-level logs folder's own name, once found
        self.pad = None                 # the plan's pad file's name
        self.make = False               # the plan makes user/pk
        self.packs = set()              # the names of the plan's pack files

    def children(self, folder_path):
        return self.disk.children(folder_path)

    def read(self, path, local_path):
        return self.disk.read(path, local_path)

    def allowed(self, action, folder, name):
        plain = name not in ("", ".", "..") and "/" not in name
        if not plain:
            return False
        if self.phase == "taking off" and action == "remove_file":
            if folder == "" and PAD.match(name):
                return True
            return self.logs is not None and folder == self.logs and folder.lower() == "logs"
        if self.phase == "putting":
            if action == "write" and folder == "" and (name == "os.ldr" or (self.pad is not None and name == self.pad and PAD.match(name))):
                return True
            if action == "make_folder" and self.make and folder == "user" and name == "pk":
                return True
            if action == "write" and folder == PACK_FOLDER and name in self.packs:
                return True
        return False

    def check(self, action, folder, name):
        if not self.allowed(action, folder, name):
            raise DiskError("the helper held back a change its rules do not allow: " + action + " " + join(folder, name))

    def write(self, local_path, folder_path, name):
        self.check("write", folder_path, name)
        self.disk.write(local_path, folder_path, name)

    def make_folder(self, folder_path, name):
        self.check("make_folder", folder_path, name)
        self.disk.make_folder(folder_path, name)

    def remove_file(self, path):
        folder, _, name = path.rpartition("/")
        self.check("remove_file", folder, name)
        self.disk.remove_file(path)


# ---------------------------------------------------------------- listings

def find_child(disk, folder, name):
    """A child of a folder by its name: (its own name, is_dir, size), or None.
    The exact name first; else the one child whose name is the same in small letters, when there is exactly one."""
    kids = disk.children(folder)
    for kid in kids:
        if kid[0] == name:
            return kid
    alike = [kid for kid in kids if kid[0].lower() == name.lower()]
    return alike[0] if len(alike) == 1 else None


def resolve_folder(disk, path):
    """A folder's path as the unit names it, found part by part, or None."""
    at = ""
    for part in [p for p in path.split("/") if p]:
        kid = find_child(disk, at, part)
        if kid is None or not kid[1]:
            return None
        at = join(at, kid[0])
    return at


def exact_child(disk, folder, name):
    """A child of a folder by its exact name: (name, is_dir, size), or None. Every place the helper writes is found so."""
    for kid in disk.children(folder):
        if kid[0] == name:
            return kid
    return None


def exact_folder(disk, path):
    """True when the folder at path is there, every part of it by its exact name."""
    at = ""
    for part in [p for p in path.split("/") if p]:
        kid = exact_child(disk, at, part)
        if kid is None or not kid[1]:
            return False
        at = join(at, part)
    return True


def get_listing(disk):
    """Every folder and file of the disk, depth first: a folder's row, then what is in it."""
    rows = []

    def walk(folder):
        for name, is_dir, size in disk.children(folder):
            path = join(folder, name)
            if is_dir:
                rows.append({"path": path, "dir": True})
                walk(path)
            else:
                rows.append({"path": path, "size": int(size), "dir": False})
    walk("")
    return rows


def row_key(row):
    """A row as a string: a folder "D path", a file "F path size"."""
    path = str(row.get("path", ""))
    if row.get("dir") is True:
        return "D " + path
    try:
        size = float(row.get("size"))
        size = int(size) if math.isfinite(size) else -1
    except (TypeError, ValueError):
        size = -1
    return "F " + path + " " + str(size)


def compare_rows(want, have):
    """Two listings as sets of rows: (what want has and have not, what have has and want not), each sorted."""
    a = {row_key(r) for r in want}
    b = {row_key(r) for r in have}
    return sorted(a - b), sorted(b - a)


# ---------------------------------------------------------------- small helpers

def sha_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def plain(text):
    """A text that can be printed and written as UTF-8, whatever bytes a file name had."""
    return text.encode("utf-8", "backslashreplace").decode("utf-8")


def save_text(path, text):
    """Written beside and renamed, so that the page, which reads the folder every second, never reads half a file.
    UTF-8, no byte order mark. A .part that is there is removed first, and the new one is made new: a link left
    there is never written through."""
    part = path + ".part"
    if os.path.lexists(part):
        os.remove(part)
    fd = os.open(part, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o644)
    with open(fd, "w", encoding="utf-8", newline="") as f:
        f.write(text)
    os.replace(part, path)


def stage_copy(local, staged):
    """A plan's file copied into the staging folder: the copy is made new (nothing of its name may be there, a link
    included), and the file is read only when it is a regular file and no link. False when it cannot be copied so."""
    try:
        src = os.open(local, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)     # a pipe would not wait
    except OSError:
        return False
    with open(src, "rb") as f:
        if not stat.S_ISREG(os.fstat(f.fileno()).st_mode):
            return False
        try:
            out = new_file(staged)
        except OSError:
            return False
        with out:
            shutil.copyfileobj(f, out, 1 << 20)
    return True


COLORS = {"Red": "31", "Green": "32", "Yellow": "33", "Cyan": "36"}


def terminal(line, color=""):
    if color and sys.stdout.isatty():
        line = "\033[" + COLORS[color] + "m" + line + "\033[0m"
    print(plain(line), flush=True)


def read_mark(folder):
    """The id in the page's page.json, or None."""
    try:
        with open(os.path.join(folder, NAMES["page"]), "r", encoding="utf-8-sig") as f:
            mark = json.load(f).get("id")
        return None if mark is None else str(mark)
    except (OSError, ValueError, AttributeError):
        return None


def local_file(folder, name):
    """A file the plan names in the shared folder, or None when the name is not a plain one inside it.
    Safer than the Windows helper, which takes the name as it stands: no absolute name, no "..", nothing outside,
    and no symbolic link."""
    if not isinstance(name, str) or not name or name.startswith("/") or "\\" in name or "\x00" in name:
        return None
    parts = name.split("/")
    if any(p in ("", ".", "..") for p in parts):
        return None
    path = os.path.join(folder, *parts)
    if os.path.islink(path):
        return None
    top = os.path.realpath(folder)
    if not os.path.realpath(path).startswith(top + os.sep):
        return None
    return path


def pack_leaf_problem(name):
    return (not isinstance(name, str) or name in ("", ".", "..") or not name.lower().endswith(".pack")
            or bool(PACK_NAME_BAD.search(name)) or len(name) > 120)


class Stopped(Exception):
    """The helper has stopped and said why."""


class LogLost(Exception):
    """The helper's log file cannot be written as a file the helper made: the run stops."""


# ---------------------------------------------------------------- the helper

class Helper:
    def __init__(self, folder, source, page, wait_minutes, find_seconds, echo, disk=None):
        self.folder = folder
        self.source = source
        self.page = page
        self.wait_minutes = wait_minutes
        self.find_seconds = find_seconds
        self.echo = echo
        self.given = disk
        self.gate = None
        self.log = None
        self.state = None

    # -- the lines, the state

    def say(self, text, color=""):
        line = time.strftime("%H:%M:%S") + "  " + text
        self.echo(line, color)
        self.state["lines"].append(text)
        try:
            fd = os.open(self.log, os.O_WRONLY | os.O_APPEND | os.O_CREAT | os.O_NOFOLLOW, 0o644)
        except OSError as e:
            raise LogLost("the helper's log file " + os.path.basename(self.log) + " cannot be written (" + str(e.strerror)
                          + "): something else stands in its place")
        with open(fd, "a", encoding="utf-8") as f:
            f.write(plain(line) + "\n")

    def save_state(self, now):
        self.state["state"] = now
        self.state["time"] = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
        save_text(os.path.join(self.folder, NAMES["helper"]), json.dumps(self.state, indent=1))

    def stop(self, why):
        self.say("STOPPED: " + why, "Red")
        self.say(NOTHING_MORE, "Red")
        self.state["message"] = why
        self.save_state("stopped")
        raise Stopped(why)

    def at(self, name):
        return os.path.join(self.folder, name)

    def new_folder(self, path):
        """A folder of this run, made new with a plain mkdir: when anything of its name is there, a link included,
        the run stops, and nothing is written into it."""
        try:
            os.mkdir(path)
        except FileExistsError:
            self.stop("the folder " + os.path.basename(path) + " is there already on this computer, and the helper "
                      "writes only into a folder it made itself. Remove it and run the command again")
        except OSError as e:
            self.stop("the folder " + os.path.basename(path) + " could not be made on this computer (" + str(e.strerror) + ")")
        return path

    # -- the run

    def run(self):
        os.makedirs(self.folder, exist_ok=True)
        self.folder = os.path.abspath(self.folder)
        self.run_id = time.strftime("%Y-%m-%d_%H-%M-%S")
        self.log = self.at("helper-" + self.run_id + ".log")
        self.state = {"helper": HELPER_VERSION, "run": self.run_id, "page": self.page or None, "state": "starting", "time": None,
                      "folder": self.folder, "unit": None, "stock": None, "logs": None, "oldPads": None, "listing": None,
                      "checks": [], "lines": [], "message": None}
        try:
            try:
                os.close(os.open(self.log, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o644))
            except OSError as e:
                raise LogLost("the helper's log file " + os.path.basename(self.log) + " could not be made new in the folder ("
                              + str(e.strerror) + "): something of that name is there already")
            return self.work()
        except LogLost as e:
            self.echo("STOPPED: " + str(e), "Red")
            self.state["message"] = str(e)
            with contextlib.suppress(Exception):
                self.save_state("stopped")
            return 1

    def work(self):
        try:
            self.say("open-field helper " + str(HELPER_VERSION) + ", run " + self.run_id + ". The folder: " + self.folder, "Cyan")
            for old in (NAMES["plan"], NAMES["after"]):
                if os.path.lexists(self.at(old)):
                    os.remove(self.at(old))
            self.save_state("starting")
            self.steps()
            return 0
        except Stopped:
            return 1
        except DiskError as e:
            return self.stop_quietly(str(e))
        except KeyboardInterrupt:
            return self.stop_quietly("the helper was interrupted at this computer")
        except LogLost:
            raise
        except Exception as e:
            try:
                self.say("STOPPED by an error: " + str(e), "Red")
                self.state["message"] = str(e)
                self.save_state("stopped")
            except LogLost:
                raise
            except Exception:
                self.echo("STOPPED by an error: " + str(e), "Red")
            return 1
        finally:
            if self.gate is not None:
                self.gate.phase = "closed"
                try:
                    self.gate.disk.close()
                except Exception:
                    pass

    def stop_quietly(self, why):
        try:
            self.stop(why)
        except Stopped:
            pass
        return 1

    def steps(self):
        # ---- 1. the unit ----
        if self.given is not None:
            disk = self.given
        elif self.source:
            disk = FolderDisk(self.source)
        else:
            disk = wait_for_unit(self.find_seconds, lambda text: self.say(text, "Yellow"))
        self.gate = gate = Gate(disk)
        self.state["unit"] = disk.name
        self.say("Found the unit's drive (" + disk.name + ").", "Green")
        # A unit over USB that the transport only reads stops here, before anything is taken off it.
        if self.given is None and not self.source:
            why = unit_read_only(disk)
            if why is not None:
                self.stop(why)
        # The same product id shows in the unit's normal disk mode: only the bootloader's raw disk is worked on.
        if not bootloader_drive(gate.children("")):
            self.stop(NOT_BOOTLOADER)

        self.stock(gate)
        gate.phase = "taking off"
        self.logs(gate)
        self.old_pads(gate)
        gate.phase = "looking"

        # ---- 4. the disk, listed ----
        rows = get_listing(gate)
        before = self.at(NAMES["before"])
        save_text(before, json.dumps(rows, separators=(",", ":")))
        files = sum(1 for r in rows if not r["dir"])
        folders = len(rows) - files
        self.state["listing"] = {"file": NAMES["before"], "files": files, "folders": folders, "sha256": sha_file(before)}
        self.say("The disk is listed: " + str(files) + " files, " + str(folders) + " folders.", "Green")
        self.save_state("listed")

        plan = self.wait_for_plan()
        image, pad, packs = self.check_plan(plan)

        # ---- 6. the disk as the plan was made from it? ----
        # The Windows helper finds the unit again here; this one keeps the disk it opened, and lists it again.
        now = get_listing(gate)
        missing, extra = compare_rows(rows, now)
        if missing or extra:
            for k in missing[:10]:
                self.say("  gone since the listing: " + k, "Red")
            for k in extra[:10]:
                self.say("  new since the listing: " + k, "Red")
            self.stop("the unit's disk changed since it was listed, so the plan no longer holds")
        self.say("The disk is as it was listed.", "Green")

        # ---- 6b. the plan's writes, held to its expected listing before any of them ----
        self.check_expected(plan, now, image, pad, packs)

        # ---- 7. the files onto the unit ----
        # Every file is copied into to-unit-<run> first, and the copy is held to the plan's SHA-256 and sent: a file
        # of the shared folder changed or swapped after the plan was checked never reaches the unit.
        stage = self.new_folder(self.at("to-unit-" + self.run_id))
        self.new_folder(os.path.join(stage, "packs"))

        def staged(step, name, why):
            copy = os.path.join(stage, name)
            if not stage_copy(step["local"], copy) or sha_file(copy) != step["sha256"]:
                self.stop(why)
            return copy

        image_copy = staged(image, "os.ldr", "the copy of the new firmware file on this computer is not the plan's")
        pad_copy = staged(pad, pad["name"], "the copy of the pad file " + pad["name"] + " on this computer is not the plan's") if pad else None
        pack_copies = [staged(pk, "packs/" + pk["name"], "the copy of the pack file " + str(pk.get("file")) + " on this computer is not the plan's")
                       for pk in packs]

        self.save_state("putting")
        gate.pad = pad["name"] if pad else None
        gate.make = plan.get("makeFolder") == PACK_FOLDER
        gate.packs = {pk["name"] for pk in packs}
        gate.phase = "putting"
        self.copy_on(image_copy, "", "os.ldr", image["bytes"])
        self.say("os.ldr is on the unit (" + str(image["bytes"]) + " bytes).", "Green")
        if pad:
            self.copy_on(pad_copy, "", pad["name"], pad["bytes"])
            self.say("The pad file " + pad["name"] + " is on the unit (" + str(pad["bytes"]) + " bytes).", "Green")
        if plan.get("makeFolder"):
            if not exact_folder(gate, "user"):
                self.stop("the unit has no folder user")
            gate.make_folder("user", "pk")
            if not exact_folder(gate, PACK_FOLDER):
                self.stop("the folder " + PACK_FOLDER + " did not appear on the unit")
            self.say("The folder " + PACK_FOLDER + " is made on the unit.", "Green")
        for pk, copy in zip(packs, pack_copies):
            folder, leaf = pk["path"].rsplit("/", 1)
            self.copy_on(copy, folder, leaf, pk["bytes"])
            self.say("The pack file " + pk["name"] + " is on the unit.", "Green")
        gate.phase = "looking"

        # ---- 8. the disk listed again, held against the plan ----
        after = get_listing(gate)
        save_text(self.at(NAMES["after"]), json.dumps(after, separators=(",", ":")))
        expected = plan.get("expected")
        missing, extra = compare_rows(expected, after)
        if missing or extra:
            for k in missing[:10]:
                self.say("  planned and not on the unit: " + k, "Red")
            for k in extra[:10]:
                self.say("  on the unit and not planned: " + k, "Red")
            self.stop("the disk is not as the plan says it must be for the write")
        self.state["checks"].append("the listing is the plan's, row for row (" + str(len(expected)) + " rows)")
        self.say("The disk's new listing is the plan's, row for row.", "Green")

        # ---- 9. every copied file read back ----
        back = self.new_folder(self.at("read-back-" + self.run_id))
        wanted = [("os.ldr", image["sha256"])]
        if pad:
            wanted.append((pad["name"], pad["sha256"]))
        wanted += [(pk["path"], pk["sha256"]) for pk in packs]
        for path, sha in wanted:
            if not self.read_back(path, sha, back):
                self.stop(path + " does not read back from the unit with the SHA-256 of the file sent")
            self.state["checks"].append(path + " read back: " + sha)
            self.say(path + " read back from the unit: the same bytes.", "Green")
        for d in (stage, back):
            shutil.rmtree(d, ignore_errors=True)

        self.state["message"] = "ready to write"
        self.save_state("ready")
        self.echo("", "")
        self.say("READY TO WRITE.", "Green")
        self.say("On the unit: press 2 to close the drive, then switch the unit off. Leave it off until the write;", "Green")
        self.say("a start in between writes to its disk, and the placement no longer holds. The install page has the keys of the write.", "Green")

    # ---- 2. your own firmware file, copied off ----
    def stock(self, gate):
        bin_folder = resolve_folder(gate, "upgrade/firmware/blackfin/bin")
        item = find_child(gate, bin_folder, "os.ldr") if bin_folder is not None else None
        if item is None or item[1]:
            self.say("The unit has no upgrade/firmware/blackfin/bin/os.ldr. The page will ask for the firmware file you kept.", "Yellow")
            return
        incoming = self.new_folder(self.at("incoming-" + self.run_id))
        copy = os.path.join(incoming, "os.ldr")
        try:
            got = gate.read(join(bin_folder, item[0]), copy)
        except DiskError:
            got = -1
        if got != item[2] or not os.path.isfile(copy) or os.path.getsize(copy) != got:
            shutil.rmtree(incoming, ignore_errors=True)
            self.stop("the firmware file did not come off the unit")
        sha = sha_file(copy)
        size = os.path.getsize(copy)
        original = sha == ORIGINAL_SHA
        # The original file is kept as os-1.7.3-yours.ldr, once; another file by a name that says what it is.
        name = NAMES["stock"] if original else "os-from-upgrade-folder-" + self.run_id + ".ldr"
        kept = self.at(name)
        if os.path.isfile(kept) and sha_file(kept) == sha:
            os.remove(copy)
        else:
            if os.path.lexists(kept):
                name = "os-1.7.3-yours-" + self.run_id + ".ldr"
                kept = self.at(name)
            os.replace(copy, kept)
        shutil.rmtree(incoming, ignore_errors=True)
        self.state["stock"] = {"file": name, "bytes": size, "sha256": sha, "original": original, "from": STOCK_FROM}
        if original:
            self.say("Your own firmware file is copied off: " + name + " (the original OS 1.7.3 file, " + str(size) + " bytes).", "Green")
        else:
            self.say("The unit's upgrade folder holds another file than the original OS 1.7.3 one (" + str(size) + " bytes); it is copied off as "
                     + name + ". The page takes the original from this folder (" + NAMES["stock"] + ") if it is there, or asks for it.", "Yellow")

    def move_off(self, gate, path, size, local_dir):
        """A file taken off the unit: copied into local_dir, and removed from the unit only when the copy is whole.
        True when it was taken off."""
        local = os.path.join(local_dir, path.rsplit("/", 1)[-1])
        try:
            got = gate.read(path, local)
        except DiskError:
            got = -1
        if got != size:
            if os.path.lexists(local):
                os.remove(local)
            return False
        try:
            gate.remove_file(path)
        except (DiskError, OSError):
            return False            # left on the unit, and counted so: never a stop
        return True

    # ---- 3. the log files, taken off ----
    def logs(self, gate):
        logs = find_child(gate, "", "logs")
        moved = left = 0
        log_dir = None
        if logs is not None and logs[1]:
            gate.logs = logs[0]
            files = [kid for kid in gate.children(logs[0]) if not kid[1]]
            if files:
                log_dir = self.new_folder(self.at("logs-" + self.run_id))
                for name, _, size in files:
                    self.move_off(gate, join(logs[0], name), size, log_dir)
                time.sleep(SECOND)
                now = resolve_folder(gate, logs[0])
                left = len([kid for kid in gate.children(now) if not kid[1]]) if now is not None else 0
                moved = len([n for n in os.listdir(log_dir) if os.path.isfile(os.path.join(log_dir, n))])
        self.state["logs"] = {"folder": os.path.basename(log_dir) if log_dir else None, "copied": moved, "left_on_unit": left}
        if moved == 0 and left == 0:
            self.say("The unit's logs folder holds no file.")
        elif left == 0:
            self.say("The " + str(moved) + " log file(s) are taken off the unit into " + os.path.basename(log_dir) + ".", "Green")
        else:
            self.say(str(left) + " log file(s) stay on the unit (" + str(moved) + " copied). That is no fault: the plan counts the disk as it is.", "Yellow")

    # ---- 3b. the pad files of earlier installs, taken off ----
    # A pad file serves one write only. The helper's own are taken off before the listing, as the logs are, so that
    # the plan counts the disk without them and at most one is ever on the unit. A file of any other name is left
    # where it is. Their bytes are of no use (only a pad file's size ever counted), so the copies are deleted.
    def old_pads(self, gate):
        def pads():
            return [kid for kid in gate.children("") if not kid[1] and PAD.match(kid[0])]
        found = pads()
        left = 0
        if found:
            pad_dir = self.new_folder(self.at("old-pads-" + self.run_id))
            for name, _, size in found:
                self.move_off(gate, name, size, pad_dir)
            time.sleep(SECOND)
            left = len(pads())
            shutil.rmtree(pad_dir, ignore_errors=True)
        self.state["oldPads"] = {"found": len(found), "taken_off": len(found) - left, "left_on_unit": left}
        if found:
            if left == 0:
                self.say("The " + str(len(found)) + " pad file(s) of earlier installs are taken off the unit.", "Green")
            else:
                self.say(str(left) + " pad file(s) of earlier installs stay on the unit. That is no fault: the plan counts the disk as it is.", "Yellow")

    # ---- 5. the page's plan ----
    def wait_for_plan(self):
        self.say("Now go back to the install page. This window waits for it; leave it open.", "Cyan")
        path = self.at(NAMES["plan"])
        deadline = time.monotonic() + self.wait_minutes * 60 * SECOND
        note = time.monotonic() + 120 * SECOND
        while True:
            try:
                with open(path, "r", encoding="utf-8-sig") as f:
                    p = json.load(f)
                # The run is compared as it stands; the Windows helper compares it in small letters or capitals alike.
                if isinstance(p, dict) and p.get("run") == self.run_id:
                    self.say("The page's plan is here.", "Green")
                    return p
            except (OSError, ValueError):
                pass
            if time.monotonic() > deadline:
                self.stop("no plan came from the page in " + str(self.wait_minutes) + " minutes")
            if time.monotonic() > note:
                self.say("  still waiting for the page...")
                note = time.monotonic() + 120 * SECOND
            time.sleep(SECOND)

    def check_plan(self, plan):
        """The plan held to this run and to the shared folder, in the Windows helper's order, with one more check
        between: every name it gives is one this helper takes. Gives the image, the pad (or None) and the packs, each
        with the local file."""
        kind = plan.get("plan")
        if kind != 1 or isinstance(kind, bool):
            self.stop("the plan is of a kind this helper does not know (" + ("" if kind is None else str(kind)) + ")")
        before = plan.get("before")
        if not isinstance(before, dict) or str(before.get("sha256", "")).lower() != self.state["listing"]["sha256"]:
            self.stop("the plan was not made from this run's listing")

        # Safer than the Windows helper, which takes the plan's names as they stand: a file in the shared folder is a
        # plain name inside it, and a place on the unit is one of the few this helper ever writes.
        def file_of(step):
            path = local_file(self.folder, step.get("file")) if isinstance(step, dict) else None
            if path is None:
                self.stop("the plan names a file that is not a plain name inside this folder: "
                          + str(step.get("file") if isinstance(step, dict) else step))
            return path

        def not_ours(where):
            self.stop("the plan names a place on the unit this helper does not write: " + str(where))

        image = plan.get("image")
        image_local = file_of(image)
        for key in ("name", "path"):
            if key in image and image[key] != "os.ldr":
                not_ours(image[key])
        pad = plan.get("pad")
        pad_local = None
        if pad:
            pad_local = file_of(pad)
            if not isinstance(pad.get("name"), str) or not PAD.match(pad["name"]):
                not_ours(pad.get("name"))
        make = plan.get("makeFolder")
        if make and make != PACK_FOLDER:
            not_ours(make)
        packs = []
        names = set()
        raw = plan.get("packs") or []
        if not isinstance(raw, list):
            not_ours(raw)
        for pk in raw:
            if not pk:
                continue
            local = file_of(pk)
            path = pk.get("path")
            leaf = path[len(PACK_FOLDER) + 1:] if isinstance(path, str) and path.startswith(PACK_FOLDER + "/") else None
            if leaf is None or pack_leaf_problem(leaf) or ("name" in pk and pk["name"] != leaf):
                not_ours(path)
            if leaf.lower() in names:
                self.stop("the plan names two pack files of one name: " + leaf)
            names.add(leaf.lower())
            packs.append(dict(pk, local=local))

        def as_planned(local, step):
            try:
                return (stat.S_ISREG(os.lstat(local).st_mode) and os.path.getsize(local) == int(step.get("bytes"))
                        and sha_file(local) == str(step.get("sha256", "")).lower())
            except (OSError, TypeError, ValueError):
                return False

        if not as_planned(image_local, image):
            self.stop("the new firmware file " + str(image.get("file")) + " is not in the folder as the plan says")
        if pad and not as_planned(pad_local, pad):
            self.stop("the pad file " + str(pad.get("file")) + " is not in the folder as the plan says")
        for pk in packs:
            if not as_planned(pk["local"], pk):
                self.stop("the pack file " + str(pk.get("file")) + " is not in the folder as the plan says")
        image = dict(image, local=image_local, bytes=int(image["bytes"]), sha256=str(image["sha256"]).lower())
        if pad:
            pad = dict(pad, local=pad_local, bytes=int(pad["bytes"]), sha256=str(pad["sha256"]).lower())
        packs = [dict(pk, bytes=int(pk["bytes"]), sha256=str(pk["sha256"]).lower(), name=pk["path"].rsplit("/", 1)[1]) for pk in packs]
        return image, (pad or None), packs

    def check_expected(self, plan, now, image, pad, packs):
        """Before the first write: the listing the disk must have afterwards, worked out from the second listing and
        the plan's own writes, held to the plan's expected listing as sets of rows. Every place written must be there
        by its exact name, or be made by the plan."""
        expected = plan.get("expected")
        if not isinstance(expected, list) or not all(isinstance(r, dict) and isinstance(r.get("path"), str) for r in expected):
            self.stop("the plan's expected listing is not a list of rows")
        have = {r["path"]: r for r in now}
        make = plan.get("makeFolder") == PACK_FOLDER
        if make and not (have.get("user") or {}).get("dir"):
            self.stop("the unit has no folder user")
        if packs and not make and not (have.get(PACK_FOLDER) or {}).get("dir"):
            self.stop("the unit has no folder " + PACK_FOLDER)
        if make and PACK_FOLDER in have:
            self.stop("the plan makes the folder " + PACK_FOLDER + ", and the unit has it already")
        made = dict(have)
        made["os.ldr"] = {"path": "os.ldr", "size": image["bytes"], "dir": False}
        if pad:
            made[pad["name"]] = {"path": pad["name"], "size": pad["bytes"], "dir": False}
        if make:
            made[PACK_FOLDER] = {"path": PACK_FOLDER, "dir": True}
        for pk in packs:
            made[pk["path"]] = {"path": pk["path"], "size": pk["bytes"], "dir": False}
        missing, extra = compare_rows(expected, list(made.values()))
        if missing or extra:
            for k in missing[:10]:
                self.say("  planned and not made by the plan's writes: " + k, "Red")
            for k in extra[:10]:
                self.say("  made by the plan's writes and not planned: " + k, "Red")
            self.stop("the plan's expected listing is not what its own writes make of the disk")

    def copy_on(self, local, folder_path, name, size):
        """A file of this computer onto the unit, then looked up there and held to its size.
        The Windows shell copies in the background and the Windows helper polls for the file; here the write is done
        when the call comes back, so the file is looked up once."""
        if not exact_folder(self.gate, folder_path):
            self.stop("the unit has no folder " + folder_path)
        start = time.monotonic()
        self.gate.write(local, folder_path, name)
        kid = exact_child(self.gate, folder_path, name)
        if kid is None or kid[1] or kid[2] != size:
            seconds = max(1, int(math.ceil(time.monotonic() - start)))
            self.stop("the unit did not show " + join(folder_path, name) + " with its " + str(size) + " bytes in " + str(seconds) + " seconds")

    # A file the helper put on the unit, read back and held to its SHA-256; read again a few times, since a unit may
    # list a file with its size before the last of it has arrived.
    def read_back(self, path, sha, back):
        parent, _, leaf = path.rpartition("/")
        for attempt in range(1, 6):
            kid = exact_child(self.gate, parent, leaf) if exact_folder(self.gate, parent) else None
            if kid is not None and not kid[1]:
                copy = os.path.join(back, kid[0])
                try:
                    got = self.gate.read(path, copy)
                except DiskError:
                    got = -1
                if got == kid[2]:
                    digest = sha_file(copy)
                    os.remove(copy)
                    if digest == sha:
                        return True
                    self.say("  " + path + " read back with another SHA-256 (" + digest[:12] + "...), try " + str(attempt) + " of 5", "Yellow")
            time.sleep(2 * SECOND)
        return False


# ---------------------------------------------------------------- the command line

class EchoLines:
    """A file to write text into, whose every whole line goes to echo(line, "")."""

    def __init__(self, echo):
        self.echo = echo
        self.held = ""

    def write(self, text):
        self.held += text
        *lines, self.held = self.held.split("\n")
        for line in lines:
            self.echo(line, "")


def list_only(find_seconds, echo):
    """--list-only: the transport's usb_list_only, once the unit shows (asked every two seconds, up to find_seconds).
    The unit is opened to read only. Nothing is made or written on this computer, and nothing changes on the unit."""
    deadline = time.monotonic() + find_seconds * SECOND
    told = False
    while True:
        try:
            usb_list_only(EchoLines(echo))
            return 0
        except DiskError as e:
            if e.kind != "absent":
                echo("STOPPED: " + str(e), "Red")
                return 1
        if time.monotonic() > deadline:
            echo("STOPPED: " + NOT_SHOWN, "Red")
            return 1
        if not told:
            echo(WAITING, "Yellow")
            told = True
        time.sleep(2 * SECOND)


def main(argv=None, disk=None, echo=terminal, system=None):
    """The helper's run. `disk`, from a test, takes the place of --source; `echo(line, color)` prints a line;
    `system`, from a test, takes the place of sys.platform."""
    # Before anything is read or made: this helper is for Linux only.
    if not (sys.platform if system is None else system).startswith(("linux", "darwin")):
        echo(NOT_LINUX, "Red")
        return 2
    ap = argparse.ArgumentParser(prog="open-field-helper.py", description="The open-field install's file work on the unit's disk.")
    ap.add_argument("--folder", default="", help="the shared folder (default ~/open-field)")
    ap.add_argument("--source", default="", help="a folder of this computer that stands for the unit's disk")
    ap.add_argument("--page", default="", help="the install page's mark (else the environment's OPEN_FIELD_PAGE)")
    ap.add_argument("--wait-minutes", type=int, default=60, help="how long to wait for the page's plan")
    ap.add_argument("--find-seconds", type=int, default=180, help="how long to wait for the unit")
    ap.add_argument("--list-only", action="store_true", help="the unit over USB: print what is found and stop; no change anywhere")
    try:
        args = ap.parse_args(argv)
    except SystemExit as e:
        return 0 if e.code in (0, None) else 2
    wait_minutes = args.wait_minutes if args.wait_minutes > 0 else 60
    find_seconds = args.find_seconds if args.find_seconds > 0 else 180
    if args.list_only:
        if args.source or disk is not None:
            echo("--list-only is for the unit over USB; it takes no --source", "Red")
            return 2
        return list_only(find_seconds, echo)
    folder = args.folder or os.path.join(os.path.expanduser("~"), "open-field")
    page = args.page or os.environ.get("OPEN_FIELD_PAGE", "")
    # The page's mark, before anything is made or written: the folder the page chose must be this one.
    if page:
        mark = read_mark(folder)
        if mark is None or mark.lower() != page.lower():
            echo("STOPPED: the install page did not choose this folder: " + folder, "Red")
            echo("In the page, step 3, choose the folder open-field in your home folder (make it there if it is not there yet),", "Red")
            echo("then copy the page's line again and run it. Nothing was done.", "Red")
            return 2
    return Helper(folder, args.source, page, wait_minutes, find_seconds, echo, disk).run()


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:     # Ctrl-C outside a run (while the unit is looked for, or with --list-only)
        terminal("STOPPED: the helper was interrupted at this computer.", "Red")
        sys.exit(1)

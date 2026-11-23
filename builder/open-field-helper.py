#!/usr/bin/env python3
"""open-field helper for Linux and macOS: the file work of the install, on the OP-1 Field's disk.

  python3 -I open-field-helper.py --page <mark> --folder-name <name>     # the unit over USB, the folder looked for
  python3 -I open-field-helper.py --page <mark> --folder <folder>        # the folder named outright
  python3 -I open-field-helper.py --folder <folder> --source <folder>    # a folder stands for the unit's disk
  python3 -I open-field-helper.py --list-only                            # print what is seen of the unit, change nothing

A web page cannot reach the unit's files, so this does it. The unit shows its disk while its bootloader's drive mode
is open (hold COM while switching it on, press 3, then 2). The helper works with the install page
through one folder of this computer, the working folder the owner picked on the page (any folder):

  1. it finds the unit, copies your own firmware file off (upgrade/firmware/blackfin/bin/os.ldr), takes the log files
     off into the working folder, lists the disk, and waits;
  2. the page builds the new firmware file, works out whether the write lands, and writes a plan into the folder;
  3. this puts the new file on the unit as os.ldr (and a pad file and pack files when the plan says so), lists the
     disk again, holds the listing against the plan row for row, and reads every file it copied back against its
     SHA-256. Then it says READY TO WRITE.

It decides nothing about the write's placement: the page does, and this only compares. It is the same work as the
Windows helper, open-field-helper.ps1, with the same files in the working folder and the same lines.

The unit over USB. The unit shows its disk over MTP (Media Transfer Protocol). The helper speaks MTP itself, through
the system's library libusb-1.0 (on Debian and Ubuntu the package libusb-1.0-0, which is usually there; on a Mac
Homebrew's: brew install libusb); the helper installs nothing. It changes a unit only when the unit's USB product id
is in the helper's list KNOWN_PRODUCTS (0x0102). With any other id the helper only reads from the unit: a full run
finds the unit and stops before it takes anything off, with a line that says so. --list-only prints what the helper
sees of the unit (its ids, its MTP interface, every file and folder) and changes no file, on the unit or on this
computer: send what it prints to the open-field project.

What has run on a unit. This version has run on stand-ins only, in its tests. What follows was reported of the helper
of open-field 0.3, which had a fixed working folder (open-field in the home folder) and left user/pkin as it was.

One unit, on OS 1.7.3 with the bootloader 1000.1.1, from one computer with Linux Mint. Its USB
id was 2367:0102 in the normal disk mode and in the bootloader's drive mode. The 0.3 helper listed its disk, read only,
in both modes. That computer needed no udev rule. One whole install then ran: the 0.3 helper copied the original OS
1.7.3 file off, took the log files off, put os.ldr, a pad file, the folder user/pk, four feature packs and base.pack on
the unit, and read each back with the plan's SHA-256. The page agreed that the write could be made. The owner made the
write with the unit's keys, and the unit started with open-field.

One unit with the bootloader 1000.2.0, its screen saying OS 1.7.3+1 and its firmware file the original 1.7.3, from
one computer with macOS and Homebrew's libusb (2026-11-20): the 0.3 helper, changed for the Mac, copied the original
file off, took the log files off, replaced the os.ldr at the top level, put a pad file, the folder user/pk and
base.pack on the unit, read each back with the plan's SHA-256 and said READY TO WRITE. The owner made the write, and
it landed the first time.

Not tried on a unit, by either: any other unit, distribution or Mac, a pack replaced or old pad files taken off, the
way back, a unit whose drive the desktop's file manager holds, a computer that needs the udev rule. And nothing that
came after 0.3 was tried on a unit: the search for the working folder and its question, the emptying of user/pkin, and
on a Mac the messages and the ending of ptpcamerad as they are now.

On a Mac one thing often stands in the way: ptpcamerad, the system's camera helper, takes an MTP device by itself, and
the system starts it again when it ends. When, and only when, the Mac refuses to open or claim the unit, the helper
says one line, ends it with /usr/bin/killall ptpcamerad (never as another user) and tries again, five times. Image
Capture, Photos, OpenMTP and Android File Transfer hold the unit too: quit them. A Mac may also ask whether Terminal
may read the Desktop, Documents and Downloads folders: a folder it refuses is passed over in the search.

Two things on Linux can stand in the way, and the helper says which:
  - The computer does not let you open the unit. A udev rule gives the device to the person at the screen. Put the
    line below into a file in /etc/udev/rules.d/ (for example 70-op1-field.rules), run sudo udevadm control --reload,
    then unplug the cable and plug it in again:
      SUBSYSTEM=="usb", ATTR{idVendor}=="2367", TAG+="uaccess"
  - The desktop's file manager holds the unit: it opens an MTP device by itself. Unmount "OP-1" there (the eject sign
    beside it in Files), or find its mtp:// address with gio mount -l and unmount it with gio mount -u mtp://...
    Leave the file manager away from the unit until the helper says READY TO WRITE: a program that adds a file to the
    unit's disk spoils the plan.

What it changes on the unit, and nothing else: it takes off the files directly in "logs", the pad files of
earlier installs (open-field-pad.bin, open-field-pad-2.bin, ...) at the top level, and the files directly in user/pkin,
the waiting folder where the install page leaves pack files over USB MIDI (its note .apply among them). It writes
os.ldr at the top level, the pad file the plan names, the folder user/pk when the plan says so, and the pack files the
plan names into user/pk. Every one of these changes passes one gate first (the class Gate), which refuses anything
else. It never deletes a folder, and it writes nothing before it has listed the disk again and found it as the plan
was made from. It works only on the bootloader's raw disk: the unit's normal disk mode has the same USB product id,
so a disk with no folder user or upgrade at its top stops the run before anything is read off it.

On this computer, each folder of a run in the working folder (to-unit-, read-back-, incoming-, logs-, old-pads-,
old-pkin-, with the run's time) is made new: anything of that name already there, a link included, stops the run.
Every file the helper puts in one is made new too. A file taken off the unit gets a number in its name when this
computer has that name already (a Mac's disk takes LOG.txt and log.txt for one), and it leaves the unit only when that
copy is whole. A backup of your firmware file is never written over: with both its names taken, the run stops.
What comes from the unit, a name, the plan or an error is shown and logged with its control characters written out.

--page, or the environment's OPEN_FIELD_PAGE, is the install page's mark: the page writes it into the folder it chose
(page.json), and this checks it before anything else, so that the two never work in two different folders. Without a
mark nothing is checked. --folder-name, or the environment's OPEN_FIELD_FOLDER, is that folder's name. A browser tells
a page a folder's name and never its path, so this looks for the folder: one of that name that holds the page's mark,
in the current folder, the home folder, Desktop, Documents and Downloads (on Linux also those three as
user-dirs.dirs names them), each and one level below. With exactly one such folder it works there. With none or
several it asks once for the folder's path, on /dev/tty, and checks the mark of that folder. --folder names the
folder outright (no search). --no-ask stops where the question would come, and so does a run with no terminal.
--wait-minutes is how long it waits for the plan (60), --find-seconds how long for the unit (180).

The exit status: 0 ready to write (with --list-only: listed), 1 stopped (the lines say why), 2 nothing was done: no
working folder was found or given, the mark was refused, the command line is wrong, or the system is neither Linux
nor macOS.
Standard library only; Python 3.8 or later. The page serves this as one file, so everything is in it. It runs as well
when Python reads it from standard input (python3 -I - --page <mark>): it never reads standard input itself, and asks its
one question on /dev/tty.
"""

import argparse
import collections
import contextlib
import ctypes as ct
import ctypes.util
import datetime
import errno
import hashlib
import json
import math
import os
import re
import shlex
import shutil
import stat
import string
import struct
import subprocess
import sys
import time
import unicodedata

HELPER_VERSION = 1
# The original OS 1.7.3 file, as the builder's manifest names it: the only file kept under the name "yours".
ORIGINAL_SHA = "d134eb1ac7ad88e8370d988329cd5d7419f016005b1d269034f9cf7166fad0ec"
NAMES = {"stock": "os-1.7.3-yours.ldr", "image": "open-field-os.ldr", "helper": "helper.json", "before": "listing-before.json",
         "after": "listing-after.json", "plan": "plan.json", "packs": "packs", "page": "page.json"}
STOCK_FROM = "upgrade/firmware/blackfin/bin/os.ldr"     # the owner's own firmware file on the unit: read, never written
# A name on the unit is compared in capitals or small letters of ASCII alone: Python's own small letters take a
# dotless i, a dotted I and a Kelvin sign for an i and a k, and then another file than the helper's would be taken.
PAD = re.compile(r"open-field-pad(-[0-9]+)?\.bin\Z", re.IGNORECASE | re.ASCII)    # a pad file the helper puts on
PACK_END = re.compile(r"\.pack\Z", re.IGNORECASE | re.ASCII)      # the end of a pack file's name
ASCII_SMALL = str.maketrans(string.ascii_uppercase, string.ascii_lowercase)
PACK_FOLDER = "user/pk"                                 # where the pack files go
WAITING_FOLDER = "user/pkin"                            # where pack files wait: emptied before the listing
PACK_NAME_BAD = re.compile(r'[\\/:*?"<>|\x00-\x1f]')    # a character a file name on the unit cannot have (install.mjs)
TTY = "/dev/tty"        # where the one question is asked: standard input is the pipe this file came through
KILLALL = "/usr/bin/killall"
MAC_TRIES = 5           # how often ptpcamerad is ended and the refused step tried again
# What is never shown as it came: the controls C0, DEL and C1, what breaks a line or turns the writing's direction,
# and half of a surrogate pair.
NOT_SHOWABLE = re.compile("[\x00-\x1f\x7f-\x9f\u2028\u2029\u202a-\u202e\u2066-\u2069\ud800-\udfff]")
# An error that is one name's, not the disk's: the next file can still work.
NAME_ERRORS = (errno.EEXIST, errno.ENAMETOOLONG, errno.EILSEQ, errno.EINVAL, errno.EISDIR, errno.ELOOP)
NEVER_AGAIN = (errno.ENOSPC, errno.EDQUOT, errno.EROFS) if hasattr(errno, "EDQUOT") else (errno.ENOSPC, errno.EROFS)
ENDS_RUN = ("local", "changed", "broken", "gone")       # a DiskError's kinds after which no next file can work
SECOND = 1.0            # the length of a second, in seconds, for every wait and pause: a test makes it shorter

MORE_THAN_ONE = "more than one OP-1 is connected: connect only the unit you are installing on"
NOT_SHOWN = ("the unit did not show on this computer. Hold COM while switching it on, press 3, then 2, with the cable in; "
             "then run the command again")
WAITING = "Waiting for the unit: on the unit, in the bootloader menu, press 3, then 2. The cable must be in."
NOT_BOOTLOADER = ("the drive that showed is not the bootloader's drive: it has no folder user or upgrade at its top. "
                  "If the unit is in its normal disk mode, switch it off, hold COM while switching it on, press 3, then 2; "
                  "then run the command again")
BOOTLOADER_TOP = ("user", "upgrade")    # the bootloader's raw disk has at least one of these folders at its top
NOT_HERE = ("STOPPED: this helper is for Linux and macOS, and this system is neither. On Windows, use the PowerShell "
            "helper, open-field-helper.ps1. Nothing was done.")
NOT_CHOSEN = ("STOPPED: the install page did not choose this folder: ",
              "On the install page, step 3, pick your working folder again,",
              "then copy the line of step 4 again and run it. Nothing was done.")
MAY_NOT_READ = ("STOPPED: this window may not read the folder: ",
                "Allow it to (on a Mac: System Settings, Privacy & Security, Files and Folders), or pick another folder on the install page.",
                "Then run the command again. Nothing was done.")
GIVE_FOLDER = " Drag the folder into this window, or type its path, and press Enter.\nThe working folder: "
RUN_WITH_FOLDER = 'Run the command again with --folder and the path of your working folder at its end: --folder "the folder\'s path"'
WAITING_LEFT = (" file(s) stay in user/pkin on the unit and would meet the new base.pack. "
                "Delete them in the unit's drive, then run the command again")
READY = ("READY TO WRITE.",
         "Now restart your OP-1 in bootloader mode: turn it off, then turn it on while holding COM.",
         "Do NOT restart it in normal mode until the install is finished, otherwise you will have to start again.",
         "Step 7 of the install page has the rest.")
NOTHING_MORE = "Nothing more is done. The unit is as this line finds it; read the lines above. Run the command again to start over."


# ---------------------------------------------------------------- the unit's disk

class DiskError(Exception):
    """A disk could not do what was asked. Its text is a plain sentence for the owner. `kind` says which failure it
    is: the transport over USB gives one of absent, no storage, several, permission, busy, gone, refused, reply, local,
    usage, library, unproven, broken, changed, name, usb; anything else is "other"."""

    def __init__(self, message, kind="other"):
        super().__init__(message)
        self.kind = kind


class OwnLines(str):
    """A text of the helper's own whose line breaks are meant. Nothing of the unit, a plan, a name or an error is in it."""


def shown(text):
    """A text as it may be printed and logged: every sign of NOT_SHOWABLE written out, as \\u001b is. Only an
    OwnLines keeps its line breaks, so no name makes a line of its own."""
    escape = lambda part: NOT_SHOWABLE.sub(lambda m: "\\u%04x" % ord(m.group()), part)
    if isinstance(text, OwnLines):
        return OwnLines("\n".join(escape(part) for part in text.split("\n")))
    return escape(str(text))


def after(prefix, text):
    """prefix + text, and an OwnLines stays one."""
    return (OwnLines if isinstance(text, OwnLines) else str)(prefix + text)


def words_of(error):
    """An error's words, an OwnLines as it was made."""
    if isinstance(error, DiskError):
        return error.args[0]
    return str(error) or type(error).__name__


def ascii_lower(name):
    return name.translate(ASCII_SMALL)


def local_kind(error):
    return "name" if error.errno in NAME_ERRORS else "local"


class Disk:
    """The unit's one storage, as the helper uses it. A path is names joined with "/"; "" is the top.

    The stand-in (FolderDisk) and the unit over USB (UsbDisk) both have these methods, and nothing else is used.
    """

    name = ""       # what helper.json says in "unit"
    broken = False  # True once nothing more can be asked of the disk

    def children(self, folder_path):
        """The direct children of a folder: [(name, is_dir, size)]. A file's size is its exact byte count, a folder's 0.
        DiskError when folder_path is not a folder."""
        raise NotImplementedError

    def read(self, path, local_path):
        """Copies the file at path to local_path, a file made new (anything of that name there, a link included,
        is a DiskError and is left as it is), and gives its byte count. A failed read leaves no file of its own."""
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
    it is listed as a file of the link's own size, and reading through it is refused. Where this computer's disk
    takes capitals and small letters as one (a Mac's usually does), the stand-in does too; the unit's disk does not.
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
            raise DiskError("the copy of " + path + " could not be made new on this computer (" + str(e.strerror) + ")", local_kind(e))
        try:
            try:
                with dst, open(local, "rb") as src:
                    shutil.copyfileobj(src, dst, 1 << 20)
            except OSError as e:
                raise DiskError("the copy of " + path + " could not be written on this computer (" + str(e.strerror) + ")", "local")
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
# so no package is installed. Debian and Ubuntu have it as libusb-1.0-0, which is usually there; a Mac gets it with
# Homebrew (brew install libusb).
#
# UsbDisk offers the helper's Disk interface and nothing more: list one folder, copy one file off, send one file,
# make one folder, remove one file. A path is names joined with "/"; "" is the top of the unit's first storage. Names
# are matched exactly: capitals count. A file is sent by removing a file of exactly that name first, never a folder.
# No other change is made to the unit, and usb_list_only sends no operation that changes anything.
#
# Before every change the unit is asked about the cached handle of each folder on the way, _confirm: is it that
# folder, by that name, in that place. An object is removed by one function only, _delete. It asks the unit about the
# object again right before, and
# removes it only when it is a plain file of the expected name in the expected folder, and not lost+found. The handles
# 0 and 0xFFFFFFFF are never taken from the unit: to MTP they mean "no object" and "every object".
#
# The unit is found by the vendor id of teenage engineering and an interface that speaks MTP; beside one with a
# product id of KNOWN_PRODUCTS, a device with another id is let be. A unit with no storage yet is waited for.
# When a unit's product id is not in KNOWN_PRODUCTS, the transport only reads from it: listing and copying off work, and sending, making a
# folder and removing a file are refused.
#
# Every failure is a DiskError with a plain message for the owner, and a `kind` the helper can act on:
# absent, no storage, several, permission, busy, gone, refused, reply, local, name, usage, changed, library, unproven,
# broken, usb.
#
# The transport is ported from open-xy's tooling/usbmtp.py at commit f10f6bd.
# Copyright (c) 2026 Open-XY contributors. MIT licence: builder/open-xy.LICENSE.
# Changed here: the Disk interface by paths with a cache of handles; files streamed to and from the computer's disk;
# the 64-bit size property; the device found without opening it first, so a missing permission and a busy interface
# are told apart; an MTP interface needs its three endpoints; messages for the OP-1 Field on Linux and
# on macOS; a read-only mode.

# The unit.
VENDOR = 0x2367          # teenage engineering
# The product ids the helper may change a unit with. 0x0102 is the id reported from the runs of open-field 0.3's
# helper (OS 1.7.3): on Linux it listed a unit's disk in both disk modes, and an install was made on it. This version
# has run on stand-ins only. The same id shows in the unit's normal disk mode, so the id alone never lets a run go on:
# bootloader_drive() decides that.
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
LINUX_LIBRARIES = ('libusb-1.0.so.0', 'libusb-1.0.so')
MAC_LIBRARIES = ('/opt/homebrew/lib/libusb-1.0.0.dylib', '/usr/local/lib/libusb-1.0.0.dylib',
                 '/opt/local/lib/libusb-1.0.0.dylib')   # Homebrew on Apple and on Intel processors; MacPorts

# What the owner reads. MAC is looked at when a text is needed, so that no text of the one system shows on the other.
MAC = sys.platform == 'darwin'
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
HELD_MAC = ('the unit is connected (bus {bus:03d}, device {address:03d}, product id {product:04x}), but another '
            'program holds it. Quit Image Capture, Photos, OpenMTP and Android File Transfer, run killall ptpcamerad, '
            'then run the command again')
ENDS_CAMERA_HELPER = ('The Mac refuses the unit: most often its camera helper, ptpcamerad, holds it. '
                      'The helper ends ptpcamerad now and tries again.')
NO_LIBRARY = ('the helper needs the library libusb-1.0, and this computer does not have it. '
              'On Debian and Ubuntu: sudo apt install libusb-1.0-0')
NO_LIBRARY_MAC = ('the helper needs the library libusb-1.0, and this Mac does not have it. With Homebrew '
                  '(https://brew.sh): brew install libusb (then run the command again)')
BAD_LIBRARY_MAC = ('the helper needs the library libusb-1.0, and the one at {path} does not load in this Python (it '
                   'may be made for the other type of processor, Apple or Intel). With a Homebrew for this Mac\'s own '
                   'processor: brew install libusb (then run the command again)')
GONE = ('the unit went away while the helper was working with it ({action}). Check the cable and that the unit '
        'still shows the bootloader\'s disk mode. The unit is as this moment left it')
SILENT = ('the unit stopped answering ({action}). Check the cable and that the unit still shows the bootloader\'s '
          'disk mode. The unit is as this moment left it')
UNPROVEN = ('this unit\'s USB product id is 0x{product:04x}. An earlier helper (open-field 0.3\'s) was '
            'tried with the id 0x0102 only, so this helper only reads from this unit and changes nothing. '
            'Run the helper with --list-only and send what it '
            'prints to the open-field project')
NO_STORAGE = ('the unit shows no storage, so this computer sees nothing of its disk. Hold COM while switching it on, '
              'press 3, then 2, with the cable in; then run the command again')
CHANGED = ('the unit changed: the folder %s is no longer where it was listed, so nothing in it is changed. '
           'Run the command again')
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
    names = MAC_LIBRARIES if MAC else LINUX_LIBRARIES
    with contextlib.suppress(OSError):
        found = ctypes.util.find_library('usb-1.0')
        if found:
            names = (found,) + names
    will_not_load = None
    for name in names:
        try:
            return ct.CDLL(name)
        except OSError:
            if will_not_load is None and os.path.isabs(name) and os.path.exists(name):
                will_not_load = name    # there, and not for this Python: the other processor type, most often
    if not MAC:
        raise DiskError(NO_LIBRARY, 'library')
    raise DiskError(BAD_LIBRARY_MAC.format(path=will_not_load) if will_not_load else NO_LIBRARY_MAC, 'library')


def end_ptpcamerad():
    """macOS's camera helper, ended: only the user's own, by the system's killall, whatever the PATH says. Its
    standard input is not the helper's, which may be the pipe this file came through."""
    with contextlib.suppress(OSError, subprocess.SubprocessError):
        subprocess.run([KILLALL, 'ptpcamerad'], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL, timeout=10)


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
    `libusb` is for tests: an object with the calls of Libusb. `note(text)` says a line to the owner."""

    def __init__(self, libusb=None, note=None):
        self.note = note if note is not None else (lambda text: terminal(text, 'Yellow'))
        self.told = False           # True once the line about ptpcamerad was said
        self.libusb = libusb if libusb is not None else Libusb()
        self.handle = self.number = None
        try:
            self._open()
        except BaseException:
            self.close()
            raise

    def _failure(self, code, action):
        """A libusb code as the DiskError the owner reads."""
        if code in (LIBUSB_ACCESS, LIBUSB_BUSY):
            if MAC:
                text = HELD_MAC
            else:
                text = PERMISSION if code == LIBUSB_ACCESS else BUSY
            return DiskError(OwnLines(text.format(bus=self.bus, address=self.address, product=self.product)),
                             'permission' if code == LIBUSB_ACCESS else 'busy')
        if code in (LIBUSB_NO_DEVICE, LIBUSB_IO):
            return DiskError(GONE.format(action=action), 'gone')
        if code == LIBUSB_TIMEOUT:
            return DiskError(SILENT.format(action=action), 'gone')
        return DiskError('%s failed: libusb says %s' % (action, LIBUSB_ERRORS.get(code, 'error %d' % code)), 'usb')

    def _retried(self, call):
        """`call` is the opening or the claim, and gives (code, what it got). On a Mac, and only when it is refused,
        the owner is told once, ptpcamerad is ended, and the call made again: launchd starts ptpcamerad again."""
        result = call()
        tries = MAC_TRIES if MAC else 0
        while tries and result[0] in (LIBUSB_ACCESS, LIBUSB_BUSY):
            if not self.told:
                self.told = True
                self.note(ENDS_CAMERA_HELPER)
            end_ptpcamerad()
            time.sleep(0.3 * SECOND)
            result = call()
            tries -= 1
        return result

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
            known = [each for each in found if each[0].product in KNOWN_PRODUCTS]
            if known:       # Another MTP device of the maker, beside the OP-1: not the helper's business.
                found = known
            require(len(found) == 1, SEVERAL, 'several')
            device, (self.number_wanted, self.inbound, self.outbound, self.packet, self.triple) = found[0]
            self.vendor, self.product, self.bus, self.address = device.vendor, device.product, device.bus, device.address
            code, handle = self._retried(lambda: libusb.open(device.ref))
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
        code, _ = self._retried(lambda: (libusb.claim(self.handle, self.number_wanted), None))
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
    operation that would change the unit; so does a product id that is not in KNOWN_PRODUCTS. `note(text)` says a
    line to the owner while the unit is opened."""

    def __init__(self, transport=None, read_only=False, note=None):
        self.name = UNIT_NAME
        self.read_only = read_only
        self.transport = transport if transport is not None else Usb(note=note)
        self.transaction = 0
        self.storage = None
        self.storages = []
        self.broken = False
        self.cause = None        # The first failure's words: what broke the session, for every later refusal.
        self.session = False     # True once the unit has a session open: close() then ends it
        self._forget()
        try:
            opened = self._run(OPEN_SESSION, [1])
            if opened[0] != SESSION_ALREADY_OPEN:
                self._expect(opened, 'open a session')
            self.session = True
            self.storages = read_handles(self._expect(self._run(STORAGE_IDS), 'name its storages')[1], 'a storage')
            require(self.storages, NO_STORAGE, 'no storage')    # Not there yet: waited for, as the unit itself is.
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
        if self.broken:
            raise DiskError(self.cause or BROKEN, 'broken')
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
        except BaseException as error:
            self.broken, self.cause = True, words_of(error)
            # Cancel the transaction so the unit is idle again, even after Ctrl+C.
            with contextlib.suppress(Exception):
                self.transport.cancel(transaction)
            raise

    # Objects and the cache.

    def _forget(self):
        """Drop the cache of handles: done after every file or folder sent. A removal drops its own entry alone."""
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

    @staticmethod
    def _inside(info, parent):
        """True when the ObjectInfo says the object is in that folder. An object at the top of the storage names the
        parent 0, as the MTP spec says and as the unit answered (2026-11-20, a unit with bootloader 1000.2.0). ROOT,
        0xFFFFFFFF, means "the top" only as a parameter of GetObjectHandles; it is taken here too, for a unit that
        answers with it."""
        return info.parent == parent or (parent == ROOT and info.parent in (NO_OBJECT, ROOT))

    def _chain(self, folder_path):
        """The cached handles of a folder and of every folder on its way: [(path, name, handle)]."""
        names = split_path(folder_path)
        paths = ['/'.join(names[:depth]) for depth in range(1, len(names) + 1)]
        return [(path, name, self._find(path).handle) for path, name in zip(paths, names)]

    def _confirm(self, chain):
        """Right before a change in a folder: the unit is asked whether each handle of its chain still is that
        folder, by that name, in that place. If not, nothing is changed in it."""
        parent = ROOT
        for path, name, handle in chain:
            reply, data, _ = self._run(OBJECT_INFO, [handle])
            require(reply == OK, CHANGED % path, 'changed')
            info = read_object_info(data)
            require(info.folder and info.name == name and self._inside(info, parent), CHANGED % path, 'changed')
            parent = handle

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
            raise DiskError('the copy of %s could not be made new on this computer: %s' % (path, error), local_kind(error))
        try:
            try:
                with sink:
                    count = self._expect(self._run(GET_OBJECT, [entry.handle], limit=entry.size, sink=sink),
                                         'give %s' % path)[1]
            except OSError as error:
                failed = DiskError('the copy of %s could not be written on this computer: %s' % (path, error), 'local')
                if self.broken:
                    self.cause = words_of(failed)
                raise failed
            require(count == entry.size, '%s came off the unit with %s bytes; the unit lists %d'
                    % (path, count, entry.size))
        except BaseException:   # No part of a file is left behind as if it were the file; only the helper's own.
            with contextlib.suppress(OSError):
                os.remove(local_path)
            raise
        return count

    def _delete(self, handle, parent, name, path, size=None):
        """Remove one file: the only place that sends DeleteObject. `parent` is the handle of the folder it should
        be in. The unit is asked about that folder, then about the object, first, and anything but a plain file of
        exactly that name in exactly that folder (and of `size`, when given: the bytes that were copied) is refused;
        so are lost+found and the handles 0 and 0xFFFFFFFF, whoever asks."""
        require(handle not in NOT_HANDLES, 'the helper never removes the object numbered %#010x (%s): to the unit '
                'that number names no single object' % (handle, path), 'usage')
        require(name not in SPECIAL_NAMES, 'the helper never removes %s' % path, 'usage')
        self._confirm(self._chain(path.rpartition('/')[0]))
        info = self._info(handle, 'describe %s' % path)
        require(not info.folder and info.format != FOLDER_FORMAT and info.association == 0,
                'the helper removes files only, and to the unit %s is a folder' % path, 'usage')
        require(info.name == name and self._inside(info, parent),
                'the unit changed: %s is no longer where it was listed, so nothing is removed' % path, 'changed')
        require(size is None or info.size == min(size, LARGE),
                'the unit changed: %s is not of the size it was copied with, so it is not removed' % path, 'changed')
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
            chain = self._chain(folder_path)
            try:
                there = dict(self._list(folder_path, folder.handle))
                if name in there:
                    require(not there[name].dir, 'the unit has a folder named %s where the file goes' % path, 'usage')
                    self._delete(there[name].handle, folder.handle, name, path)
                    self._forget()
                self._confirm(chain)
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
        chain = self._chain(folder_path)
        try:
            there = dict(self._list(folder_path, folder.handle))
            require(name not in there, 'the unit already has a %s named %s'
                    % ('folder' if there.get(name, Entry(0, False, 0)).dir else 'file', path), 'usage')
            self._confirm(chain)
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
            self._delete(entry.handle, parent, name, path, entry.size)
        finally:
            # This one object alone leaves the cache: forgetting all would list the folder again for every file of
            # it. _confirm and _delete ask the unit about the cached handles before each change.
            self._handles.pop(path, None)


def usb_list_only(out, transport=None, note=None):
    """The helper's --list-only: open the unit, print what it is, everything on it and whether it looks like the
    bootloader's drive, close. The disk is opened to read only, so no operation that changes the unit can be sent."""
    disk = UsbDisk(transport, read_only=True, note=note or (lambda text: out.write(text + '\n')))
    try:
        usb = disk.transport
        out.write('Vendor id %04x, product id %04x, bus %03d, device %03d.\n'
                  % (usb.vendor, usb.product, usb.bus, usb.address))
        out.write('MTP interface: class %02x, subclass %02x, protocol %02x.\n' % tuple(usb.triple))
        out.write('Storage ids: %s. The first is used.\n' % ', '.join('%08x' % s for s in disk.storages))
        rows = get_listing(disk)
        for row in rows:
            out.write(shown(json.dumps(row, ensure_ascii=False, separators=(',', ':'))) + '\n')
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


def find_unit(note=None):
    """The unit over USB as a UsbDisk, or None while no unit shows; a unit with no storage yet is a DiskError of
    kind "no storage", which is waited for. DiskError when more than one shows (kind
    "several"), when this computer does not let the helper open it ("permission"), when another program holds it
    ("busy"), or when libusb is missing ("library")."""
    try:
        return UsbDisk(note=note)
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


def once(note):
    """note(text), said the first time alone for each text: what is asked every two seconds says its line once."""
    said = set()

    def say(text):
        if text not in said:
            said.add(text)
            note(text)
    return say


def wait_for_unit(seconds, note):
    """find_unit() asked every two seconds until a unit shows; `note` gets the waiting line once, and once what the
    transport says while it opens the unit, however often that is."""
    deadline = time.monotonic() + seconds * SECOND
    note = once(note)
    missing = NOT_SHOWN
    while True:
        try:
            disk = find_unit(note)
        except DiskError as e:
            if e.kind != "no storage":
                raise
            disk, missing = None, words_of(e)
        if disk is not None:
            return disk
        note(WAITING)               # before the deadline is looked at: a first search may take longer than that
        if time.monotonic() > deadline:
            raise DiskError(missing)
        time.sleep(2 * SECOND)


def waiting_place(folder):
    """True for the waiting folder user/pkin alone, as the unit may spell it: capitals or small letters, ASCII only
    (a Kelvin sign is a "k" in small letters, and no k)."""
    return ascii_lower(folder) == WAITING_FOLDER


class Gate:
    """The unit as the helper reaches it. Reading passes; every change (write, make_folder, remove_file) is held to
    the rules first, here and nowhere else, and anything else is refused before the disk is asked. A place written
    is named exactly, letter for letter: "user/PK" is not "user/pk".

    The rules, by phase:
      "taking off" (before the first listing): remove a file directly in the top-level folder logs, a top-level
        file whose name is a pad file's, or a file directly in user/pkin, once that folder was found;
      "putting" (only once the plan was checked and the disk listed again as the plan was made from): write os.ldr at
        the top, write the plan's pad file at the top, make user/pk when the plan says so, write a pack file the plan
        names into user/pk.
    In every other phase nothing changes.
    """

    def __init__(self, disk):
        self.disk = disk
        self.phase = "looking"
        self.logs = None                # the top-level logs folder's own name, once found
        self.waiting = None             # user/pkin as the unit names it, while it is emptied
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
            if self.waiting is not None and folder == self.waiting and waiting_place(folder):
                return True
            return self.logs is not None and folder == self.logs and ascii_lower(folder) == "logs"
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
    The exact name first; else the one child whose name is the same in ASCII's small letters, when there is exactly
    one."""
    kids = disk.children(folder)
    for kid in kids:
        if kid[0] == name:
            return kid
    alike = [kid for kid in kids if ascii_lower(kid[0]) == ascii_lower(name)]
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


def free_name(folder, name):
    """A path in folder that nothing has: the name itself, else with -2, -3 ... before its end. This computer's disk
    may take two of the unit's names for one, and then the second gets a name of its own."""
    stem, end = os.path.splitext(name)
    path, number = os.path.join(folder, name), 1
    while os.path.lexists(path):
        number += 1
        path = os.path.join(folder, "%s-%d%s" % (stem, number, end))
    return path


def whole_copy(path, size):
    """True when path is a regular file of exactly that size."""
    try:
        st = os.lstat(path)
    except OSError:
        return False
    return stat.S_ISREG(st.st_mode) and st.st_size == size


def discard(folder, made):
    """The helper's own copies removed, and their folder when nothing else is in it."""
    for path in made:
        with contextlib.suppress(OSError):
            os.remove(path)
    with contextlib.suppress(OSError):
        os.rmdir(folder)


def keep_new(copy, kept):
    """The copy put under the name kept, and never in the place of anything there. False when the name is taken."""
    try:
        os.link(copy, kept)             # one step, and it refuses a name that is taken
    except OSError:
        if os.path.lexists(kept):
            return False
        os.rename(copy, kept)           # a disk with no hard links
        return True
    os.remove(copy)
    return True


def save_text(path, text):
    """Written beside and put in the old file's place in one step, so that the page, which reads the folder every
    second, never reads half a file and never finds none. Where the swap is refused while the page holds the file, it
    is tried again a few times. UTF-8, no byte order mark. A .part that is there is removed first, and the new one is
    made new: a link left there is never written through."""
    part = path + ".part"
    if os.path.lexists(part):
        os.remove(part)
    fd = os.open(part, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o644)
    with open(fd, "w", encoding="utf-8", newline="") as f:
        f.write(text)
    for left in range(19, -1, -1):
        try:
            os.replace(part, path)
            return
        except OSError as e:
            if not left or e.errno in NEVER_AGAIN:
                raise
            time.sleep(0.05 * SECOND)


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
    line = shown(line)          # before the colour, which is an escape of the helper's own
    if color and sys.stdout.isatty():
        line = "\033[" + COLORS[color] + "m" + line + "\033[0m"
    coding = getattr(sys.stdout, "encoding", None) or "utf-8"     # a folder may have any name
    print(line.encode(coding, "backslashreplace").decode(coding), flush=True)


def read_mark(folder):
    """The id in the page's page.json, or None."""
    path = os.path.join(folder, NAMES["page"])
    try:
        if not os.path.isfile(path):        # a pipe of that name is never waited on
            return None
        with open(path, "r", encoding="utf-8-sig") as f:
            mark = json.load(f).get("id")
        return None if mark is None else str(mark)
    except (OSError, ValueError, AttributeError):
        return None


def mark_is(folder, page):
    mark = read_mark(folder)
    return mark is not None and mark.lower() == page.lower()


def may_not_read(folder):
    """True when the system refuses this window the folder or its page.json (a Mac's privacy setting does)."""
    path = os.path.join(folder, NAMES["page"])
    try:
        with os.scandir(folder):
            pass
        if os.path.isfile(path):
            with open(path, "rb"):
                pass
    except PermissionError:
        return True
    except (OSError, ValueError):
        pass
    return False


# ---------------------------------------------------------------- the working folder

def folded(name):
    """A name to compare: a Mac gives names decomposed (an accent as a sign of its own), and letters' case is no matter."""
    return unicodedata.normalize("NFC", unicodedata.normalize("NFD", name).casefold())


def user_dirs(home):
    """Desktop, Documents and Downloads by the names this Linux gives them (user-dirs.dirs), where it names them."""
    config = os.environ.get("XDG_CONFIG_HOME", "")
    if not os.path.isabs(config):
        config = os.path.join(home, ".config")
    path = os.path.join(config, "user-dirs.dirs")
    out = []
    try:
        if not os.path.isfile(path):
            return out
        with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
            for line in f:
                m = re.match(r'\s*XDG_(DESKTOP|DOCUMENTS|DOWNLOAD)_DIR="((?:[^"\\]|\\.)*)"', line)
                if not m:
                    continue
                value = re.sub(r"\\(.)", r"\1", m.group(2))
                if value == "$HOME" or value.startswith("$HOME/"):
                    value = home + value[len("$HOME"):]
                if os.path.isabs(value):
                    out.append(value)
    except OSError:
        pass
    return out


def search_places(linux):
    """The places an owner makes a folder in: the current folder, the home folder, Desktop, Documents, Downloads."""
    places = []
    with contextlib.suppress(OSError):      # the current folder may be gone, or refused
        places.append(os.getcwd())
    home = os.path.expanduser("~")
    if os.path.isabs(home):
        places += [home] + [os.path.join(home, n) for n in ("Desktop", "Documents", "Downloads")]
        if linux:
            places += user_dirs(home)
    return places


def find_folders(name, page, linux):
    """The folders that may be the one the page picked: of the name given and holding the mark given, among the
    places searched, each and one level below. Each folder once, by what it really is, and by its real path. A folder
    that cannot be read is passed over."""
    wanted = folded(name) if name else None
    found = {}
    for root in search_places(linux):
        if not os.path.isdir(root):
            continue
        candidates = [(os.path.basename(os.path.normpath(root)), root)]
        with contextlib.suppress(OSError):
            with os.scandir(root) as entries:
                for e in entries:
                    with contextlib.suppress(OSError):
                        if e.is_dir():
                            candidates.append((e.name, e.path))
        for own, path in candidates:
            if wanted is not None and folded(own) != wanted:
                continue
            if page and not mark_is(path, page):
                continue
            with contextlib.suppress(OSError):
                real = os.path.realpath(path)
                st = os.stat(real)          # a Mac's disk takes two spellings for one folder
                found.setdefault((st.st_dev, st.st_ino) if st.st_ino else real, real)
    return list(found.values())


def ask_terminal(question):
    """The answer typed at the terminal (its line as it came, "" when none came), or None with no terminal.
    Standard input is never read: it is the pipe when the helper comes through one."""
    try:
        with open(TTY, "r+b", buffering=0) as tty:
            tty.write(question.encode("utf-8", "backslashreplace"))
            return os.fsdecode(tty.readline() or b"")
    except OSError:
        return None


def typed_path(text):
    """A path as typed, or as dragging a folder into a terminal pastes it: in quotes, or with a backslash before a
    space or another sign, and a space after it."""
    text = text.strip("\r\n").lstrip()
    path = text.rstrip()
    if path and not os.path.isdir(os.path.expanduser(path)):
        with contextlib.suppress(ValueError):       # a quote with no end: the text as it stands
            parts = shlex.split(text)
            if len(parts) == 1:
                path = parts[0]
    return os.path.expanduser(path)


def choose_folder(name, page, linux, no_ask, echo, ask):
    """The working folder, looked for, or asked once; None when there is none, and nothing is done."""
    hits = find_folders(name, page, linux) if name or page else []
    if len(hits) == 1:
        return hits[0]
    what = 'the working folder "' + name + '"' if name else "the working folder"
    if hits:
        why = "Found " + str(len(hits)) + " folders that could be " + what + "."
    else:
        why = "Could not find " + what + " by itself."
    answered = None if no_ask else ask(OwnLines(shown(why) + GIVE_FOLDER))
    if answered is None:
        echo("STOPPED: " + why + " Nothing was done.", "Red")
        echo(RUN_WITH_FOLDER, "Red")
        return None
    folder = typed_path(answered)
    if not folder.strip():
        echo("STOPPED: no folder was given. Nothing was done.", "Red")
        return None
    if not os.path.isdir(folder) and not may_not_read(folder):
        echo("STOPPED: there is no such folder: " + folder + ". Nothing was done.", "Red")
        return None
    return folder


# ---------------------------------------------------------------- the plan's names

def local_file(folder, name):
    """A file the plan names in the working folder, or None when the name is not a plain one inside it.
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
    return (not isinstance(name, str) or name in ("", ".", "..") or not PACK_END.search(name)
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
        text = shown(text)
        line = after(time.strftime("%H:%M:%S") + "  ", text)
        self.echo(line, color)
        self.state["lines"].append(text)
        lost = "the helper's log file " + os.path.basename(self.log) + " cannot be written ("
        try:
            fd = os.open(self.log, os.O_WRONLY | os.O_APPEND | os.O_CREAT | os.O_NOFOLLOW, 0o644)
        except OSError as e:
            raise LogLost(lost + str(e.strerror) + ")"
                          + (": something else stands in its place" if e.errno in NAME_ERRORS else ""))
        try:
            with open(fd, "a", encoding="utf-8") as f:
                f.write(line + "\n")
        except OSError as e:
            raise LogLost(lost + str(e.strerror) + ")")

    def save_state(self, now):
        self.state["state"] = now
        self.state["time"] = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
        save_text(os.path.join(self.folder, NAMES["helper"]), json.dumps(self.state, indent=1))

    def stop(self, why):
        self.last_words(why, [after("STOPPED: ", why), NOTHING_MORE])
        raise Stopped(why)

    def last_words(self, why, lines):
        """A stop's lines and state. Its cause is what the owner reads, also when this computer takes no more
        writing (a full disk): say() prints a line before it logs it."""
        self.state["message"] = shown(why)
        for line in lines:
            with contextlib.suppress(LogLost, OSError):
                self.say(line, "Red")
        with contextlib.suppress(OSError):
            self.save_state("stopped")

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
        try:
            os.makedirs(self.folder, exist_ok=True)
            self.folder = os.path.abspath(self.folder)
        except OSError as e:
            self.echo("STOPPED: the working folder " + self.folder + " cannot be used (" + str(e.strerror) + "). Nothing was done.", "Red")
            return 1
        self.run_id = time.strftime("%Y-%m-%d_%H-%M-%S")
        self.log = self.at("helper-" + self.run_id + ".log")
        self.state = {"helper": HELPER_VERSION, "run": self.run_id, "page": self.page or None, "state": "starting", "time": None,
                      "folder": self.folder, "unit": None, "stock": None, "logs": None, "oldPads": None,
                      "waiting": None, "listing": None,
                      "checks": [], "lines": [], "message": None}
        try:
            try:
                os.close(os.open(self.log, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o644))
            except OSError as e:
                raise LogLost("the helper's log file " + os.path.basename(self.log) + " could not be made new in the folder ("
                              + str(e.strerror) + ")"
                              + (": something of that name is there already" if e.errno in NAME_ERRORS else ""))
            return self.work()
        except LogLost as e:
            self.echo("STOPPED: " + str(e), "Red")
            self.state["message"] = shown(str(e))
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
            return self.stop_quietly(words_of(e))
        except KeyboardInterrupt:
            return self.stop_quietly("the helper was interrupted at this computer")
        except LogLost:
            raise
        except Exception as e:
            self.last_words(words_of(e), ["STOPPED by an error: " + words_of(e)])
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
        self.state["unit"] = shown(disk.name)
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
        self.waiting_files(gate)
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
        # of the working folder changed or swapped after the plan was checked never reaches the unit.
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
            same, cause = self.read_back(path, sha, back)
            if not same:
                self.stop(path + " does not read back from the unit with the SHA-256 of the file sent"
                          + (": " + cause if cause else ""))
            self.state["checks"].append(shown(path + " read back: " + sha))
            self.say(path + " read back from the unit: the same bytes.", "Green")
        for d in (stage, back):
            shutil.rmtree(d, ignore_errors=True)

        self.state["message"] = "ready to write"
        self.save_state("ready")
        self.echo("", "")
        for line in READY:
            self.say(line, "Green")

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
        except DiskError as e:
            discard(incoming, [])
            self.stop("the firmware file did not come off the unit: " + words_of(e))
        if got != item[2] or not whole_copy(copy, got):
            discard(incoming, [copy])
            self.stop("the firmware file did not come off the unit: its copy is not of the " + str(item[2]) + " bytes the unit lists")
        sha = sha_file(copy)
        size = os.path.getsize(copy)
        original = sha == ORIGINAL_SHA
        # The original file is kept as os-1.7.3-yours.ldr, once; another file by a name that says what it is.
        name = NAMES["stock"] if original else "os-from-upgrade-folder-" + self.run_id + ".ldr"
        kept = self.at(name)
        if os.path.isfile(kept) and sha_file(kept) == sha:
            os.remove(copy)
        elif not keep_new(copy, kept):
            first, name = name, "os-1.7.3-yours-" + self.run_id + ".ldr"
            if not keep_new(copy, self.at(name)):
                discard(incoming, [copy])
                self.stop("the copy of your firmware file has no name left in the folder: " + first + " and " + name
                          + " are there already, and neither is written over. Move them away, then run the command again")
        discard(incoming, [])
        self.state["stock"] = {"file": name, "bytes": size, "sha256": sha, "original": original, "from": STOCK_FROM}
        if original:
            self.say("Your own firmware file is copied off: " + name + " (the original OS 1.7.3 file, " + str(size) + " bytes).", "Green")
        else:
            self.say("The unit's upgrade folder holds another file than the original OS 1.7.3 one (" + str(size) + " bytes); it is copied off as "
                     + name + ". The page takes the original from this folder (" + NAMES["stock"] + ") if it is there, or asks for it.", "Yellow")

    def ends_run(self, error):
        """True after a failure with which no next file, and no other try, can work."""
        return self.gate.disk.broken or error.kind in ENDS_RUN

    def move_off(self, gate, path, size, local_dir, local_name=None):
        """A file taken off the unit: copied into local_dir under a name of its own (local_name or the file's, with a
        number when that is taken), and removed from the unit only when that copy is whole. Gives (the whole copy or
        None, why the file stayed or None). A failure after which nothing can work is raised. A local file that this
        call did not make is never removed: a disk that cannot make or fill a copy takes its own part file away."""
        local = free_name(local_dir, local_name if local_name is not None else path.rsplit("/", 1)[-1])
        try:
            got = gate.read(path, local)
        except DiskError as e:
            if self.ends_run(e):
                raise
            return None, words_of(e)
        if got != size:             # read() gives a count only for a file it made new
            with contextlib.suppress(OSError):
                os.remove(local)
            return None, "its copy has " + str(got) + " bytes, and the unit lists " + str(size)
        if not whole_copy(local, size):
            return None, "its copy is not whole on this computer"
        try:
            gate.remove_file(path)
        except DiskError as e:
            if self.ends_run(e):
                raise
            return local, words_of(e)       # left on the unit, and counted so
        except OSError as e:
            return local, words_of(e)
        return local, None

    def take_off(self, gate, files, local_dir, numbered=False):
        """Files taken off, [(path, size)]: (the copies made, the first cause of one that stayed or None)."""
        copies, cause = [], None
        for number, (path, size) in enumerate(files, 1):
            local, stayed = self.move_off(gate, path, size, local_dir, str(number) if numbered else None)
            if local is not None:
                copies.append(local)
            if stayed is not None and cause is None:
                cause = path + ": " + stayed
        return copies, cause

    # ---- 3. the log files, taken off ----
    def logs(self, gate):
        logs = find_child(gate, "", "logs")
        moved = left = 0
        log_dir = cause = None
        if logs is not None and logs[1]:
            gate.logs = logs[0]
            files = [kid for kid in gate.children(logs[0]) if not kid[1]]
            if files:
                log_dir = self.new_folder(self.at("logs-" + self.run_id))
                copies, cause = self.take_off(gate, [(join(logs[0], name), size) for name, _, size in files], log_dir)
                time.sleep(SECOND)
                if exact_folder(gate, logs[0]):
                    left = len([kid for kid in gate.children(logs[0]) if not kid[1]])
                moved = len(copies)
        self.state["logs"] = {"folder": os.path.basename(log_dir) if log_dir else None, "copied": moved, "left_on_unit": left}
        if moved == 0 and left == 0:
            self.say("The unit's logs folder holds no file.")
        elif left == 0:
            self.say("The " + str(moved) + " log file(s) are taken off the unit into " + os.path.basename(log_dir) + ".", "Green")
        else:
            self.say(str(left) + " log file(s) stay on the unit (" + str(moved) + " copied). That is no fault: the plan counts the disk as it is.", "Yellow")
        if left and cause:
            self.say("  the first that stayed: " + cause, "Yellow")

    # ---- 3b. the pad files of earlier installs, taken off ----
    # A pad file serves one write only. The helper's own are taken off before the listing, as the logs are, so that
    # the plan counts the disk without them and at most one is ever on the unit. A file of any other name is left
    # where it is. Their bytes are of no use (only a pad file's size ever counted), so the copies are deleted.
    def old_pads(self, gate):
        def pads():
            return [kid for kid in gate.children("") if not kid[1] and PAD.match(kid[0])]
        found = pads()
        left = 0
        cause = None
        if found:
            pad_dir = self.new_folder(self.at("old-pads-" + self.run_id))
            copies, cause = self.take_off(gate, [(name, size) for name, _, size in found], pad_dir)
            time.sleep(SECOND)
            left = len(pads())
            discard(pad_dir, copies)
        self.state["oldPads"] = {"found": len(found), "taken_off": len(found) - left, "left_on_unit": left}
        if found:
            if left == 0:
                self.say("The " + str(len(found)) + " pad file(s) of earlier installs are taken off the unit.", "Green")
            else:
                self.say(str(left) + " pad file(s) of earlier installs stay on the unit. That is no fault: the plan counts the disk as it is.", "Yellow")
                if cause:
                    self.say("  the first that stayed: " + cause, "Yellow")

    # ---- 3c. the waiting folder of the packs, emptied ----
    # The install page sends pack files over USB MIDI into user/pkin, where they wait for the owner's T1 on the unit,
    # and the unit keeps a note there (.apply) while it changes its packs. An install puts a base.pack on the unit,
    # and a note left behind must never meet it: the very same base.pack file cannot be told from the one that wrote
    # the note. The plan is made after the listing, so whether this run writes base.pack is not known yet; every run
    # takes the files of user/pkin off, the note among them, before the listing, as it does the logs, so that the
    # plan counts the disk without them. The folder itself stays. The copies are numbered, since this computer's disk
    # may take two of the unit's names for one, and deleted, since the files are lost at the unit's next start anyway.
    def waiting_files(self, gate):
        place = resolve_folder(gate, WAITING_FOLDER)
        if place is not None and not waiting_place(place):
            place = None

        def files():
            if place is None or not exact_folder(gate, place):
                return []
            return [kid for kid in gate.children(place) if not kid[1]]
        found = files()
        left = 0
        if found:
            wait_dir = self.new_folder(self.at("old-pkin-" + self.run_id))
            gate.waiting = place
            copies, cause = self.take_off(gate, [(join(place, name), size) for name, _, size in found], wait_dir, True)
            gate.waiting = None
            time.sleep(SECOND)
            left = len(files())
            discard(wait_dir, copies)
        self.state["waiting"] = {"found": len(found), "taken_off": len(found) - left, "left_on_unit": left}
        if found:
            if left:
                self.stop(str(left) + WAITING_LEFT + (". The cause: " + cause if cause else ""))
            self.say("The " + str(len(found)) + " file(s) waiting in user/pkin are taken off the unit.", "Green")

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
        """The plan held to this run and to the working folder, in the Windows helper's order, with one more check
        between: every name it gives is one this helper takes. Gives the image, the pad (or None) and the packs, each
        with the local file."""
        kind = plan.get("plan")
        if kind != 1 or isinstance(kind, bool):
            self.stop("the plan is of a kind this helper does not know (" + ("" if kind is None else str(kind)) + ")")
        before = plan.get("before")
        if not isinstance(before, dict) or str(before.get("sha256", "")).lower() != self.state["listing"]["sha256"]:
            self.stop("the plan was not made from this run's listing")

        # Safer than the Windows helper, which takes the plan's names as they stand: a file in the working folder is a
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
        """(True when the same bytes came back, the first cause when none came). A failure with which no other try
        can work is raised, with its own words."""
        parent, _, leaf = path.rpartition("/")
        cause = None
        for attempt in range(1, 6):
            kid = exact_child(self.gate, parent, leaf) if exact_folder(self.gate, parent) else None
            if kid is not None and not kid[1]:
                copy = os.path.join(back, kid[0])
                try:
                    got = self.gate.read(path, copy)
                except DiskError as e:
                    if self.ends_run(e) or e.kind == "name":
                        raise
                    cause = cause or words_of(e)
                else:
                    if got == kid[2]:
                        digest = sha_file(copy)
                        os.remove(copy)
                        if digest == sha:
                            return True, None
                        self.say("  " + path + " read back with another SHA-256 (" + digest[:12] + "...), try " + str(attempt) + " of 5", "Yellow")
                    else:
                        os.remove(copy)     # read() made it: the next try makes it new again
            time.sleep(2 * SECOND)
        return False, cause


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
    note = once(lambda text: echo(text, "Yellow"))
    missing = NOT_SHOWN
    while True:
        try:
            usb_list_only(EchoLines(echo), note=note)
            return 0
        except DiskError as e:
            if e.kind == "no storage":
                missing = words_of(e)
            elif e.kind != "absent":
                echo(after("STOPPED: ", words_of(e)), "Red")
                return 1
        except Exception as e:
            echo("STOPPED by an error: " + str(e), "Red")
            return 1
        note(WAITING)
        if time.monotonic() > deadline:
            echo("STOPPED: " + missing, "Red")
            return 1
        time.sleep(2 * SECOND)


def main(argv=None, disk=None, echo=terminal, system=None, ask=None):
    """The helper's run. `disk`, from a test, takes the place of --source; `echo(line, color)` prints a line;
    `system`, from a test, takes the place of sys.platform, and `ask(question)` that of the question on /dev/tty."""
    system = sys.platform if system is None else system
    write_line = echo
    echo = lambda line, color: write_line(shown(line), color)       # every line passes here, whoever wrote it
    # Before anything is read or made: this helper is for Linux and macOS only.
    if not system.startswith(("linux", "darwin")):
        echo(NOT_HERE, "Red")
        return 2
    ap = argparse.ArgumentParser(prog="open-field-helper.py", description="The open-field install's file work on the unit's disk.")
    ap.add_argument("--folder", default="", help="the working folder, named outright (no search)")
    ap.add_argument("--folder-name", default="", help="the working folder's name, to look for it (else the environment's OPEN_FIELD_FOLDER)")
    ap.add_argument("--no-ask", action="store_true", help="stop where the folder's path would be asked for")
    ap.add_argument("--source", default="", help="a folder of this computer that stands for the unit's disk")
    ap.add_argument("--page", default="", help="the install page's mark (else the environment's OPEN_FIELD_PAGE)")
    ap.add_argument("--wait-minutes", type=int, default=60, help="how long to wait for the page's plan")
    ap.add_argument("--find-seconds", type=int, default=180, help="how long to wait for the unit")
    ap.add_argument("--list-only", action="store_true", help="the unit over USB: print what is found and stop; no file is changed anywhere")
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
    page = args.page or os.environ.get("OPEN_FIELD_PAGE", "")
    folder = args.folder
    # The folder the page picked: named, or looked for by its name and the page's mark, or asked for.
    if not folder:
        name = args.folder_name or os.environ.get("OPEN_FIELD_FOLDER", "")
        folder = choose_folder(name, page, system.startswith("linux"), args.no_ask, echo, ask or ask_terminal)
        if folder is None:
            return 2
    # The page's mark, before anything is made or written: the folder the page chose must be this one.
    if page and not mark_is(folder, page):
        lines = MAY_NOT_READ if may_not_read(folder) else NOT_CHOSEN
        echo(lines[0] + folder, "Red")
        for line in lines[1:]:
            echo(line, "Red")
        return 2
    return Helper(folder, args.source, page, wait_minutes, find_seconds, echo, disk).run()


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:     # Ctrl-C outside a run (while the unit is looked for, or with --list-only)
        terminal("STOPPED: the helper was interrupted at this computer.", "Red")
        sys.exit(1)

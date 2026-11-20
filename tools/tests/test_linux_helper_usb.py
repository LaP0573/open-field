"""The Linux helper's transport over USB, in builder/open-field-helper.py: its MTP disk against a stand-in unit, its
USB layer against a stand-in libusb, and the whole helper run over that stand-in unit.

  python -I -m unittest discover -s tools/tests -p "test_linux_helper_usb.py"

No USB device is used, and the system's libusb is never loaded: the module's Libusb is replaced by a guard that fails
the test. The stand-in unit, Responder, holds a tree of objects made of made-up bytes and answers MTP containers as a
device would: replies cut into transfers of one TRANSFER, with a zero-length packet after a container that ends on a
packet's edge. It has its own copy of every MTP number it uses and takes none from the helper, so a wrong number in
the helper is caught. It checks every container the helper sends: its length field, its type, its transaction id
(one more for each operation, the data phase with the same id and code as its command), and the zero-length packet
after a container that ends on a packet's edge. A fault raises AssertionError and is kept in `faults`; every test
ends by checking that there is none. FakeLibusb answers libusb's calls with the codes a test gives it.

Responder is ported from open-xy's tests/test_usbmtp.py at commit f10f6bd.
Copyright (c) 2026 Open-XY contributors. MIT licence: builder/open-xy.LICENSE.
"""

import ctypes as ct
import hashlib
import importlib.util
import inspect
import io
import json
import os
import shutil
import struct
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

if not sys.platform.startswith(("linux", "darwin")):
    # Before the helper is loaded: it is for Linux, and these tests use os.O_NOFOLLOW, named pipes and symbolic links.
    raise unittest.SkipTest("the Linux helper is tried on Linux only")

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HELPER = os.path.join(ROOT, "builder", "open-field-helper.py")


class NoRealLibusb:
    """Stands in the helper's Libusb in every test: the system's libusb is never loaded, so no real device is reached.
    A test that needs USB behaviour puts FakeLibusb in its place itself."""

    def __init__(self):
        raise AssertionError("a test reached for the system's libusb")


def load():
    spec = importlib.util.spec_from_file_location("linux_helper_usb", HELPER)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


H = load()
REAL_LIBUSB = H.Libusb      # the class itself, for the one test of its bulk_in; never made with its __init__
H.Libusb = NoRealLibusb     # the guard, set as the module is loaded: no test can reach the system's libusb
DiskError, Usb, UsbDevice, UsbDisk = H.DiskError, H.Usb, H.UsbDevice, H.UsbDisk


def setUpModule():
    H.SECOND = 0.002        # every wait of the helper, 500 times shorter

# The stand-in unit's own copy of MTP. Nothing here comes from the helper.
TE_VENDOR = 0x2367
PRODUCT = 0x0102         # The id in KNOWN_PRODUCTS: the one unit the helper has run on showed it in both disk modes.
OTHER = 0x0199           # Made up: an id not in KNOWN_PRODUCTS, for the tests of the unproven stop.
STORAGE, SECOND_STORAGE = 0x10001, 0x20001
TOP = 0xFFFFFFFF         # The parent handle of the top of a storage.
PACKET = 512             # The bulk endpoints' packet size.
TRANSFER = 1 << 20       # The most the helper asks for in one bulk read.
BOX = struct.Struct('<IHHI')   # A container's header: length, kind, code, transaction.
COMMAND_KIND, DATA_KIND, RESPONSE_KIND = 1, 2, 3
OK, GENERAL_ERROR, NOT_SUPPORTED, NO_STORAGE, NO_OBJECT = 0x2001, 0x2002, 0x2005, 0x2008, 0x2009
NOT_A_FOLDER, ALREADY_OPEN = 0x201A, 0x201E
OPEN_SESSION, CLOSE_SESSION, STORAGE_IDS, OBJECT_HANDLES = 0x1002, 0x1003, 0x1004, 0x1007
OBJECT_INFO, GET_OBJECT, DELETE_OBJECT, SEND_OBJECT_INFO, SEND_OBJECT = 0x1008, 0x1009, 0x100B, 0x100C, 0x100D
GET_PROP_VALUE, SIZE_PROPERTY = 0x9803, 0xDC04
FILE_FORMAT, FOLDER_FORMAT = 0x3000, 0x3001
# The parameters each operation the helper uses takes.
PARAMETERS = {OPEN_SESSION: 1, CLOSE_SESSION: 0, STORAGE_IDS: 0, OBJECT_HANDLES: 3, OBJECT_INFO: 1, GET_OBJECT: 1,
              DELETE_OBJECT: 2, SEND_OBJECT_INFO: 2, SEND_OBJECT: 0, GET_PROP_VALUE: 2}
CHANGING = frozenset((DELETE_OBJECT, SEND_OBJECT_INFO, SEND_OBJECT))   # The operations that change the unit.

MTP_INTERFACES = [(0, 0, 6, 1, 1, [(0x81, 2, 512), (0x02, 2, 512), (0x83, 3, 8)])]
AUDIO_INTERFACES = [(0, 0, 1, 1, 0, []), (1, 0, 1, 3, 0, [(0x81, 2, 512), (0x02, 2, 512)])]


def made_up(size, seed=1):
    """Bytes that stand for a file: a counting pattern, nothing of any device."""
    return bytes((seed + i * 7) & 0xFF for i in range(size))


def box(kind, code, transaction, payload=b''):
    return BOX.pack(12 + len(payload), kind, code, transaction) + payload


def numbers(values):
    return struct.pack('<%dI' % len(values), *values)


def text_field(value):
    """A PTP string: a count of UTF-16 units with the ending zero, then UTF-16LE; the empty string is one zero."""
    data = (value + '\0').encode('utf-16-le') if value else b''
    return bytes([len(data) // 2]) + data


def text_at(data, at):
    count = data[at]
    return data[at + 1:at + 1 + 2 * count].decode('utf-16-le').rstrip('\0')


def described(storage, name, size, parent, kind, association):
    """An ObjectInfo as the unit gives it."""
    fixed = bytearray(52)
    struct.pack_into('<IHxxI', fixed, 0, storage, kind, size)
    struct.pack_into('<IH', fixed, 38, parent, association)
    return bytes(fixed) + text_field(name) + b'\0' * 3


class Obj:
    """One object of the stand-in unit. `data` is None for a folder; `size` stands in for a size too large to hold.
    `kind` and `association` are what its ObjectInfo says."""

    def __init__(self, parent, name, data, size=None):
        self.parent, self.name, self.data = parent, name, data
        self.size = size if size is not None else len(data or b'')
        self.kind = FOLDER_FORMAT if data is None else FILE_FORMAT
        self.association = 1 if data is None else 0


class Responder:
    """A transport that answers MTP containers from a tree of objects and checks every container it is sent."""
    packet = PACKET
    vendor, product, triple, bus, address = TE_VENDOR, PRODUCT, (6, 1, 1), 7, 42   # made up

    def __init__(self, files=(), folders=()):
        self.objects, self.next, self.reply = {}, 0, []
        self.pending = None        # The bytes of a container that has not all come yet.
        self.waiting = None        # A whole container on a packet's edge, waiting for its zero-length packet.
        self.command = None        # (code, transaction, params) of a command whose data phase is due.
        self.next_transaction = 0
        self.ops = []              # Every operation code received, in order.
        self.faults = []           # Every fault in what the helper sent.
        self.refuse = {}           # operation code: a list of answers, one per call; None answers normally.
        self.answer_params = {}    # operation code: extra parameters for its answer.
        self.storages = [STORAGE, SECOND_STORAGE]
        self.no_size_property = False
        self.open_ended = set()    # Handles whose GetObject container says its length is 0xFFFFFFFF.
        self.session_already_open = False
        self.cancelled, self.closed, self.deleted_folders, self.deleted_everything = [], False, [], False
        for path in folders:
            self.make(path)
        for path, data in dict(files).items():
            *above, name = path.split('/')
            parent = self.make('/'.join(above)) if above else TOP
            if isinstance(data, int):
                self.add(parent, name, None, data)   # Only a size: never read.
            else:
                self.add(parent, name, data)

    def make(self, path):
        parent = TOP
        for name in path.split('/'):
            parent = self.find(parent, name) or self.add(parent, name, None)
        return parent

    def add(self, parent, name, data, size=None):
        """A file has bytes, or only a size; a folder has neither."""
        self.next += 1
        self.objects[self.next] = Obj(parent, name, b'' if data is None and size is not None else data, size)
        return self.next

    def find(self, parent, name):
        return next((h for h, o in self.objects.items() if o.parent == parent and o.name == name), None)

    def at(self, path):
        handle = TOP
        for name in path.split('/'):
            handle = self.find(handle, name)
            if handle is None:
                return None
        return self.objects[handle]

    def paths(self):
        def path(handle):
            o = self.objects[handle]
            return (path(o.parent) + '/' if o.parent != TOP and o.parent in self.objects else '') + o.name
        return sorted(path(h) + ('/' if o.data is None else '') for h, o in self.objects.items())

    def folder(self, handle):
        return handle == TOP or (handle in self.objects and self.objects[handle].data is None)

    def fault(self, text):
        self.faults.append(text)
        raise AssertionError('the stand-in unit was sent something wrong: ' + text)

    # The transport.

    def write(self, data):
        """One bulk transfer from the helper. A transfer whose length is not a whole number of packets ends with a
        short packet, which ends the container to the unit; one of whole packets does not, so a container that ends
        on a packet's edge is taken only when a zero-length transfer follows it."""
        data = bytes(data)
        if self.waiting is not None:
            if data:
                self.fault('a container ended on a packet\'s edge with no zero-length packet after it')
            whole, self.waiting = self.waiting, None
            self.receive(whole)
            return
        if not data:
            self.fault('a zero-length packet where no container ended on a packet\'s edge')
        if self.pending is None:
            if len(data) < 12:
                self.fault('a container shorter than its header')
            self.pending = [b'', BOX.unpack_from(data)[0]]
        self.pending[0] += data
        have, length = len(self.pending[0]), self.pending[1]
        if have > length:
            self.fault('a container longer than its length field says (%d against %d)' % (have, length))
        if have < length and len(data) % PACKET:
            self.fault('a short packet ended a container before its length (%d against %d)' % (have, length))
        if have == length:
            whole, self.pending = self.pending[0], None
            if length % PACKET == 0:
                self.waiting = whole
            else:
                self.receive(whole)

    def receive(self, data):
        """One whole container from the helper."""
        length, kind, code, transaction = BOX.unpack_from(data)
        payload = data[12:length]
        if self.command is not None:
            command, self.command = self.command, None
            if (kind, code, transaction) != (DATA_KIND, command[0], command[1]):
                self.fault('a data phase of kind %d, code %#06x, transaction %d after the command %#06x of '
                           'transaction %d' % (kind, code, transaction, command[0], command[1]))
            self.answer(command[0], command[1], command[2], payload)
            return
        if kind != COMMAND_KIND:
            self.fault('a container of kind %d where a command was due' % kind)
        if transaction != self.next_transaction:
            self.fault('the command %#06x with transaction %d where %d was due'
                       % (code, transaction, self.next_transaction))
        if not self.ops and code != OPEN_SESSION:
            self.fault('the operation %#06x before OpenSession' % code)
        if code not in PARAMETERS or len(payload) != 4 * PARAMETERS[code]:
            self.fault('the operation %#06x with %d bytes of parameters' % (code, len(payload)))
        self.next_transaction = transaction + 1
        params = list(struct.unpack('<%dI' % (len(payload) // 4), payload))
        self.ops.append(code)
        if code in (SEND_OBJECT_INFO, SEND_OBJECT):
            self.command = (code, transaction, params)
        else:
            self.answer(code, transaction, params, None)

    def send(self, whole):
        for at in range(0, len(whole), TRANSFER):
            self.reply.append(whole[at:at + TRANSFER])
        if len(whole) % self.packet == 0:
            self.reply.append(b'')

    def answer(self, code, transaction, params, payload):
        reply, data, out, header = OK, None, [], None
        planned = self.refuse.get(code)
        if planned:
            reply = planned.pop(0) or OK
        if reply != OK:
            pass
        elif code == OPEN_SESSION:
            reply = ALREADY_OPEN if self.session_already_open else OK
        elif code == CLOSE_SESSION:
            pass
        elif code == STORAGE_IDS:
            data = numbers([len(self.storages)] + self.storages)
        elif code == OBJECT_HANDLES:
            if params[0] not in (STORAGE, 0xFFFFFFFF):
                reply = NO_STORAGE
            elif params[2] == 0:   # Every object of the storage, at any depth.
                data = numbers([len(self.objects)] + list(self.objects))
            elif not self.folder(params[2]):
                reply = NOT_A_FOLDER
            else:
                handles = [h for h, o in self.objects.items() if o.parent == params[2]]
                data = numbers([len(handles)] + handles)
        elif code == OBJECT_INFO:
            o = self.objects.get(params[0])
            if o is None:
                reply = NO_OBJECT
            else:
                # An object at the top: the unit's ObjectInfo names the parent 0, as the MTP spec says (seen on a unit
                # with bootloader 1000.2.0). TOP is only GetObjectHandles' word for the top.
                data = described(STORAGE, o.name, min(o.size, 0xFFFFFFFF), 0 if o.parent == TOP else o.parent,
                                 o.kind, o.association)
        elif code == GET_PROP_VALUE:
            o = self.objects.get(params[0])
            if self.no_size_property or params[1] != SIZE_PROPERTY:
                reply = NOT_SUPPORTED
            elif o is None:
                reply = NO_OBJECT
            else:
                data = struct.pack('<Q', o.size)
        elif code == GET_OBJECT:
            o = self.objects.get(params[0])
            if o is None or o.data is None:
                reply = NO_OBJECT
            else:
                data = o.data
                if params[0] in self.open_ended:
                    header = BOX.pack(0xFFFFFFFF, DATA_KIND, code, transaction)
        elif code == DELETE_OBJECT:
            if params[0] == TOP:   # To MTP: every object.
                self.deleted_everything = True
                self.objects.clear()
            elif params[0] not in self.objects:
                reply = NO_OBJECT
            else:
                if self.objects[params[0]].data is None:
                    self.deleted_folders.append(self.objects[params[0]].name)
                del self.objects[params[0]]
        elif code == SEND_OBJECT_INFO:
            storage, kind, size = struct.unpack_from('<IHxxI', payload)
            parent, = struct.unpack_from('<I', payload, 38)
            if (storage, parent) != (params[0], params[1]):
                self.fault('an ObjectInfo for storage %#x in %#x sent with the parameters %r' % (storage, parent,
                                                                                                 params))
            if params[0] != STORAGE:
                reply = NO_STORAGE
            elif not self.folder(params[1]):
                reply = NOT_A_FOLDER
            else:
                folder = kind == FOLDER_FORMAT
                self.declared = size
                self.created = self.add(params[1], text_at(payload, 52), None if folder else b'')
                if folder:
                    self.objects[self.created].data = None
                out = [STORAGE, params[1], self.created]
        elif code == SEND_OBJECT:
            o = self.objects[self.created]
            o.data, o.size = bytes(payload), len(payload)
            if len(payload) != self.declared:
                reply = GENERAL_ERROR
        out += self.answer_params.get(code, [])
        if data is not None:
            self.send((header + data) if header else box(DATA_KIND, code, transaction, data))
        self.send(box(RESPONSE_KIND, reply, transaction, numbers(out)))

    def read(self):
        if not self.reply:
            self.fault('the helper waited for a reply the unit had no reason to send')
        return self.reply.pop(0)

    def cancel(self, transaction):
        self.cancelled.append(transaction)
        self.pending = self.waiting = self.command = None

    def close(self):
        self.closed = True


class FakeLibusb:
    """libusb's calls as Usb makes them, answering with the codes a test sets."""

    def __init__(self, devices, responder=None, open_code=0, claim_code=0, active=1):
        self.list, self.responder = devices, responder
        self.open_code, self.claim_code, self.active = open_code, claim_code, active
        # The next bulk transfers: 0 is a normal one, a code fails with nothing moved, (code, part) fails after part.
        self.in_codes, self.out_codes = [], []
        self.calls = []

    def devices(self):
        self.calls.append('devices')
        return self.list

    def free_devices(self):
        self.calls.append('free_devices')

    def open(self, ref):
        self.calls.append(('open', ref))
        return self.open_code, ('handle', ref)

    def configuration(self, handle):
        return 0, self.active

    def set_configuration(self, handle, value):
        self.calls.append(('set_configuration', value))
        return 0

    def auto_detach(self, handle):
        return 0

    def claim(self, handle, number):
        self.calls.append(('claim', number))
        return self.claim_code

    def release(self, handle, number):
        self.calls.append(('release', number))

    def close(self, handle):
        self.calls.append('close')

    def exit(self):
        self.calls.append('exit')

    def clear_halt(self, handle, endpoint):
        self.calls.append(('clear_halt', endpoint))
        return 0

    def bulk_out(self, handle, endpoint, data, timeout):
        code = self.out_codes.pop(0) if self.out_codes else 0
        if isinstance(code, tuple):
            return code
        if code:
            return code, 0
        self.responder.write(bytes(data))
        return 0, len(data)

    def bulk_in(self, handle, endpoint, length, timeout):
        code = self.in_codes.pop(0) if self.in_codes else 0
        if isinstance(code, tuple):
            return code
        if code:
            return code, b''
        piece = self.responder.read()
        assert len(piece) <= length
        return 0, piece

    def control(self, handle, request_type, request, value, index, data=b'', length=0, timeout=2000):
        self.calls.append(('control', request))
        return 4, struct.pack('<HH', 4, OK)


def unit(vendor=TE_VENDOR, product=PRODUCT, bus=7, address=42, interfaces=MTP_INTERFACES, ref='unit'):
    """A made-up device: its bus and address are of no real computer."""
    return UsbDevice(ref, vendor, product, bus, address, interfaces, 1)


class Base(unittest.TestCase):
    """The stand-in shows the known id, PRODUCT, unless a test gives it OTHER. KNOWN_PRODUCTS is never patched."""

    def setUp(self):
        self.temp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.temp, ignore_errors=True)

    def local(self, name, data=None):
        path = os.path.join(self.temp, name)
        if data is not None:
            with open(path, 'wb') as f:
                f.write(data)
        return path

    def bytes_of(self, name):
        with open(self.local(name), 'rb') as f:
            return f.read()

    def watch(self, responder):
        """The stand-in, with a check at the end of the test that it saw no fault."""
        self.unit = responder
        self.addCleanup(lambda: self.assertEqual(responder.faults, []))
        return responder

    def disk(self, files=(), folders=(), **options):
        self.watch(Responder(files, folders))
        for key, value in options.items():
            setattr(self.unit, key, value)
        disk = UsbDisk(self.unit)
        self.addCleanup(disk.close)
        return disk

    def over_usb(self, files=(), **options):
        self.watch(Responder(files))
        self.libusb = FakeLibusb([unit()], self.unit, **options)
        disk = UsbDisk(Usb(self.libusb))
        self.addCleanup(disk.close)
        return disk


class DataTests(unittest.TestCase):
    def test_strings_and_object_info_are_laid_out_as_ptp_asks(self):
        self.assertEqual(H.ptp_string('a'), b'\x02a\x00\x00\x00')
        self.assertEqual(H.ptp_string(''), b'\x00')
        self.assertEqual(H.read_string(H.ptp_string('Bänd ünd ß.aif'), 0), 'Bänd ünd ß.aif')
        info = H.object_info(7, 'x', 5, TOP)
        self.assertEqual(struct.unpack_from('<IHxxI', info), (7, FILE_FORMAT, 5))
        self.assertEqual(struct.unpack_from('<IH', info, 38), (TOP, 0))
        self.assertEqual(text_at(info, 52), 'x')
        folder = H.object_info(7, 'x', 0, 3, folder=True)
        self.assertEqual(struct.unpack_from('<H', folder, 4)[0], FOLDER_FORMAT)
        self.assertEqual(struct.unpack_from('<IH', folder, 38), (3, 1))
        self.assertEqual(H.read_words(numbers([2, 7, 8])), [7, 8])
        self.assertEqual(H.container(COMMAND_KIND, OPEN_SESSION, 0, numbers([1])),
                         box(COMMAND_KIND, OPEN_SESSION, 0, numbers([1])))
        with self.assertRaisesRegex(DiskError, 'too long'):
            H.ptp_string('x' * 255)

    def test_the_module_numbers_are_those_of_mtp(self):
        self.assertEqual((H.ROOT, H.VENDOR, H.OK), (TOP, TE_VENDOR, OK))
        self.assertEqual((H.COMMAND, H.DATA, H.RESPONSE), (COMMAND_KIND, DATA_KIND, RESPONSE_KIND))
        self.assertEqual(H.READING & CHANGING, frozenset())
        self.assertEqual(H.READING | CHANGING, frozenset(PARAMETERS))

    def test_an_mtp_interface_needs_its_class_and_its_three_endpoints(self):
        three = [(0x81, 2, 512), (0x02, 2, 512), (0x83, 3, 8)]
        self.assertEqual(H.mtp_interface(MTP_INTERFACES), (0, 0x81, 0x02, 512, (6, 1, 1)))
        self.assertEqual(H.mtp_interface(AUDIO_INTERFACES + [(2, 0, 0xFF, 0, 0, three)]),
                         (2, 0x81, 0x02, 512, (0xFF, 0, 0)))
        self.assertIsNone(H.mtp_interface([(0, 0, 0xFF, 0, 0, three[:2])]))      # No interrupt endpoint.
        self.assertIsNone(H.mtp_interface([(0, 0, 8, 6, 0x50, three)]))          # Mass storage.
        self.assertIsNone(H.mtp_interface([(0, 0, 6, 1, 2, three)]))             # Still image, not MTP.
        self.assertIsNone(H.mtp_interface([(0, 1, 6, 1, 1, three)]))             # Not the first alternate.
        self.assertIsNone(H.mtp_interface(AUDIO_INTERFACES))

    def test_libusb_descriptors_are_read_into_interfaces(self):
        endpoints = (H.EndpointDescriptor * 3)(
            H.EndpointDescriptor(7, 5, 0x81, 2, 512), H.EndpointDescriptor(7, 5, 0x02, 2, 512),
            H.EndpointDescriptor(7, 5, 0x83, 3, 8))
        alternates = (H.InterfaceDescriptor * 1)(
            H.InterfaceDescriptor(9, 4, 1, 0, 3, 6, 1, 1, 0, ct.cast(endpoints, ct.POINTER(H.EndpointDescriptor))))
        interfaces = (H.Interface * 1)(H.Interface(ct.cast(alternates, ct.POINTER(H.InterfaceDescriptor)), 1))
        config = H.ConfigDescriptor(9, 2, 39, 1, 1, 0, 0x80, 50, ct.cast(interfaces, ct.POINTER(H.Interface)))
        found = H.interfaces_of(config)
        self.assertEqual(found, [(1, 0, 6, 1, 1, [(0x81, 2, 512), (0x02, 2, 512), (0x83, 3, 8)])])
        self.assertEqual(H.mtp_interface(found), (1, 0x81, 0x02, 512, (6, 1, 1)))

    def test_delete_object_is_sent_from_one_function_only(self):
        source = inspect.getsource(H)
        self.assertEqual(source.count('DELETE_OBJECT'), 2)   # Its number, and its one use, in _delete.
        self.assertEqual(source.count('_run(DELETE_OBJECT'), 1)
        self.assertIn('_run(DELETE_OBJECT', inspect.getsource(H.UsbDisk._delete))


class ListingTests(Base):
    FILES = {'content/tapes/track 1.aif': made_up(3000), 'content/Bänd ünd ß.aif': made_up(10, 2),
             '.hidden': made_up(4, 3), 'user/.keep': b'', 'lost+found': made_up(40, 4), 'os.ldr': made_up(300, 5)}

    def test_the_tree_is_listed_with_exact_names_sizes_and_folders(self):
        disk = self.disk(self.FILES, folders=['user/pk', 'empty'])
        top = sorted(disk.children(''))
        self.assertEqual(top, [('.hidden', False, 4), ('content', True, 0), ('empty', True, 0),
                               ('lost+found', False, 40), ('os.ldr', False, 300), ('user', True, 0)])
        self.assertEqual(disk.children('empty'), [])
        self.assertEqual(sorted(disk.children('user')), [('.keep', False, 0), ('pk', True, 0)])
        self.assertEqual(disk.children('user/pk'), [])
        self.assertEqual(sorted(disk.children('content')), [('Bänd ünd ß.aif', False, 10), ('tapes', True, 0)])
        self.assertEqual(disk.children('content/tapes'), [('track 1.aif', False, 3000)])
        rows = H.get_listing(disk)
        self.assertEqual(len(rows), 11)
        self.assertLess(rows.index({'path': 'content', 'dir': True}),
                        rows.index({'path': 'content/tapes/track 1.aif', 'size': 3000, 'dir': False}))
        self.assertIn({'path': 'lost+found', 'size': 40, 'dir': False}, rows)
        self.assertNotIn(GET_PROP_VALUE, self.unit.ops)

    def test_a_folder_that_is_a_file_or_missing_is_refused(self):
        disk = self.disk(self.FILES)
        with self.assertRaisesRegex(DiskError, 'os.ldr is a file, not a folder'):
            disk.children('os.ldr')
        with self.assertRaisesRegex(DiskError, 'the unit has no nothing'):
            disk.children('nothing')
        with self.assertRaisesRegex(DiskError, 'not a name'):
            disk.children('content/../user')

    def test_sizes_at_the_32_bit_limit_come_from_the_64_bit_property(self):
        disk = self.disk({'tapes/exactly.aif': 0xFFFFFFFF, 'tapes/beyond.aif': 0x100000005,
                          'tapes/below.aif': 0xFFFFFFFE})
        self.assertEqual(sorted(disk.children('tapes')), [('below.aif', False, 0xFFFFFFFE),
                                                          ('beyond.aif', False, 0x100000005),
                                                          ('exactly.aif', False, 0xFFFFFFFF)])
        self.assertEqual(self.unit.ops.count(GET_PROP_VALUE), 2)

    def test_a_unit_without_the_64_bit_property_stops_the_listing_in_plain_words(self):
        disk = self.disk({'tapes/beyond.aif': 0x100000005}, no_size_property=True)
        with self.assertRaises(DiskError) as caught:
            disk.children('tapes')
        self.assertEqual(caught.exception.kind, 'refused')
        self.assertIn('tapes/beyond.aif as 4 GiB or more', str(caught.exception))
        self.assertIn('0x2005', str(caught.exception))

    def test_two_objects_of_one_name_are_refused(self):
        disk = self.disk({'a': b'1'})
        self.unit.add(TOP, 'a', b'2')
        with self.assertRaisesRegex(DiskError, 'two objects named a'):
            disk.children('')

    def test_handles_0_and_ffffffff_from_the_unit_are_refused_and_nothing_is_removed(self):
        for bad in (0, TOP):
            disk = self.disk({'a.log': b'1', 'b.log': b'2'})
            self.unit.objects[bad] = self.unit.objects.pop(self.unit.find(TOP, 'a.log'))
            for action in (lambda: disk.children(''), lambda: disk.remove_file('a.log'),
                           lambda: disk.write(self.local('n', b'n'), '', 'a.log')):
                with self.assertRaisesRegex(DiskError, 'names no single one'):
                    action()
            self.assertNotIn(DELETE_OBJECT, self.unit.ops)
            self.assertEqual(self.unit.paths(), ['a.log', 'b.log'])

    def test_storage_ids_0_and_ffffffff_are_refused(self):
        for bad in (0, TOP):
            self.watch(Responder({'a': b'1'}))
            self.unit.storages = [bad, STORAGE]
            with self.assertRaisesRegex(DiskError, 'names no single one'):
                UsbDisk(self.unit)
            self.assertEqual(self.unit.ops, [OPEN_SESSION, STORAGE_IDS, CLOSE_SESSION])   # The session is closed.

    def test_a_number_listed_twice_is_refused(self):
        self.watch(Responder({'a': b'1'}))
        self.unit.storages = [STORAGE, STORAGE]
        with self.assertRaisesRegex(DiskError, 'a storage with one number twice'):
            UsbDisk(self.unit)
        self.assertEqual(self.unit.ops, [OPEN_SESSION, STORAGE_IDS, CLOSE_SESSION])

    def test_an_object_of_association_type_1_is_a_folder(self):
        disk = self.disk({'odd': b'', 'user/a': b'1'})
        self.unit.at('odd').association = 1   # A file's format, but a generic folder's association.
        self.assertEqual(sorted(disk.children('')), [('odd', True, 0), ('user', True, 0)])
        with self.assertRaisesRegex(DiskError, 'removes files only'):
            disk.remove_file('odd')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)


class ReadTests(Base):
    def test_files_come_off_whole_including_an_empty_one_and_one_larger_than_a_chunk(self):
        big = made_up(2 * TRANSFER + 777)
        disk = self.disk({'empty.bin': b'', 'logs/big.log': big, 'logs/Ünïcode name.log': made_up(9),
                          'logs/edge.log': made_up(PACKET - 12)})
        self.assertEqual(disk.read('empty.bin', self.local('e')), 0)
        self.assertEqual(self.bytes_of('e'), b'')
        self.assertEqual(disk.read('logs/big.log', self.local('b')), len(big))
        self.assertEqual(self.bytes_of('b'), big)
        self.assertEqual(disk.read('logs/edge.log', self.local('g')), PACKET - 12)   # Ends with a zero-length one.
        self.assertEqual(disk.read('logs/Ünïcode name.log', self.local('u')), 9)
        # The copy is made new: a file or a link of its name there already is refused before the unit is asked.
        asked = self.unit.ops.count(GET_OBJECT)
        self.local('old', b'something there already')
        victim = self.local('victim', b'untouched')
        os.symlink(victim, self.local('link'))
        for name in ('old', 'link'):
            with self.assertRaisesRegex(DiskError, 'could not be made new on this computer') as caught:
                disk.read('logs/Ünïcode name.log', self.local(name))
            self.assertEqual(caught.exception.kind, 'local')
        self.assertEqual((self.bytes_of('old'), self.bytes_of('victim')), (b'something there already', b'untouched'))
        self.assertTrue(os.path.islink(self.local('link')))
        self.assertEqual(self.unit.ops.count(GET_OBJECT), asked)

    def test_a_container_of_unknown_length_ends_with_a_short_transfer(self):
        for size in (TRANSFER + 100, 2 * TRANSFER - 12):   # The second ends on a chunk: a zero-length packet ends it.
            disk = self.disk({'x.bin': made_up(size)})
            self.unit.open_ended.add(self.unit.find(TOP, 'x.bin'))
            self.assertEqual(disk.read('x.bin', self.local('x%d' % size)), size)
            self.assertEqual(self.bytes_of('x%d' % size), made_up(size))
            self.assertEqual(disk.children(''), [('x.bin', False, size)])   # The session goes on.

    def test_a_folder_is_not_read(self):
        disk = self.disk(folders=['logs'])
        with self.assertRaisesRegex(DiskError, 'logs is a folder'):
            disk.read('logs', self.local('l'))
        self.assertNotIn(GET_OBJECT, self.unit.ops)

    def test_a_file_that_comes_off_shorter_than_listed_is_refused_and_not_kept(self):
        disk = self.disk({'a.log': made_up(100)})
        disk.children('')
        self.unit.at('a.log').data = made_up(60)
        with self.assertRaisesRegex(DiskError, 'a.log came off the unit with 60 bytes; the unit lists 100'):
            disk.read('a.log', self.local('a'))
        self.assertFalse(os.path.exists(self.local('a')))

    def test_an_empty_file_whose_answer_carries_parameters_is_read(self):
        disk = self.disk({'e': b'', 'f': made_up(3)})
        self.unit.answer_params[GET_OBJECT] = [0]
        self.assertEqual(disk.read('e', self.local('e')), 0)
        self.assertEqual(disk.read('f', self.local('f')), 3)
        self.unit.answer_params[GET_OBJECT] = [0] * 6   # More than an answer can carry.
        with self.assertRaisesRegex(DiskError, 'a size the helper did not ask for'):
            disk.read('e', self.local('e2'))


class ChangeTests(Base):
    def test_write_replaces_a_file_of_exactly_that_name_and_touches_no_neighbour(self):
        disk = self.disk({'user/pk/a.pack': b'old', 'user/pk/A.pack': b'capital', 'user/pk/a.pack.bak': b'bak',
                          'user/pk/b.pack': b'other'})
        before = {n: (self.unit.find(self.unit.make('user/pk'), n), self.unit.at('user/pk/' + n).data)
                  for n in ('A.pack', 'a.pack.bak', 'b.pack')}
        disk.write(self.local('new', b'new bytes'), 'user/pk', 'a.pack')
        self.assertEqual(self.unit.at('user/pk/a.pack').data, b'new bytes')
        after = {n: (self.unit.find(self.unit.make('user/pk'), n), self.unit.at('user/pk/' + n).data)
                 for n in ('A.pack', 'a.pack.bak', 'b.pack')}
        self.assertEqual(after, before)
        self.assertEqual(self.unit.ops.count(DELETE_OBJECT), 1)
        self.assertEqual(sorted(disk.children('user/pk')), [('A.pack', False, 7), ('a.pack', False, 9),
                                                            ('a.pack.bak', False, 3), ('b.pack', False, 5)])

    def test_write_sends_new_files_of_every_size(self):
        disk = self.disk(folders=['user'])
        sizes = {'zero.bin': 0, 'edge.bin': PACKET - 12, 'two edges.bin': 2 * PACKET - 12, 'small.bin': 3,
                 'big.bin': TRANSFER * 2 + 5, 'chunk.bin': TRANSFER - 12, 'chunk and edge.bin': TRANSFER + PACKET - 12}
        for name, size in sizes.items():
            disk.write(self.local(name, made_up(size, len(name))), 'user', name)
        for name, size in sizes.items():
            self.assertEqual(self.unit.at('user/' + name).data, made_up(size, len(name)))
        disk.write(self.local('top', b'top'), '', 'Top file.bin')
        self.assertEqual(self.unit.at('Top file.bin').data, b'top')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)

    def test_write_never_replaces_a_folder(self):
        disk = self.disk(folders=['user/pk/a.pack'])
        with self.assertRaisesRegex(DiskError, 'a folder named user/pk/a.pack'):
            disk.write(self.local('new', b'x'), 'user/pk', 'a.pack')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)
        self.assertNotIn(SEND_OBJECT_INFO, self.unit.ops)

    def test_write_refuses_a_missing_local_file_before_touching_the_unit(self):
        disk = self.disk(folders=['user'])
        with self.assertRaises(DiskError) as caught:
            disk.write(self.local('absent'), 'user', 'x.bin')
        self.assertEqual(caught.exception.kind, 'local')
        self.assertEqual(self.unit.ops, [OPEN_SESSION, STORAGE_IDS])

    def test_a_file_that_changes_size_while_it_is_sent_is_stopped_before_its_last_piece(self):
        for real, said in ((5000, 1000), (1000, 3000), (2 * TRANSFER, TRANSFER + 5), (0, 4), (4, 0)):
            disk = self.disk(folders=['user'])
            path = self.local('changing', made_up(real))
            with mock.patch.object(H.os.path, 'getsize', lambda p: said):
                with self.assertRaises(DiskError) as caught:
                    disk.write(path, 'user', 'g.bin')
            self.assertEqual(caught.exception.kind, 'local')
            self.assertIn('while it was sent', str(caught.exception))
            self.assertEqual(self.unit.at('user/g.bin').data, b'')   # Created, but no data phase was taken.
            self.assertEqual(self.unit.ops[-1], SEND_OBJECT)
            self.assertEqual(self.unit.cancelled, [len(self.unit.ops) - 1])

    def test_remove_file_refuses_a_folder(self):
        disk = self.disk({'user/pk/a.pack': b'1'}, folders=['empty'])
        for path in ('empty', 'user/pk', ''):
            with self.assertRaisesRegex(DiskError, 'removes files only'):
                disk.remove_file(path)
        disk.children('')
        handle = self.unit.find(TOP, 'empty')
        disk._handles['empty'] = H.Entry(handle, False, 0)   # The cache thinks: a file; the unit says: a folder.
        with self.assertRaisesRegex(DiskError, 'removes files only'):
            disk.remove_file('empty')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)
        disk.remove_file('user/pk/a.pack')
        self.assertEqual(self.unit.paths(), ['empty/', 'user/', 'user/pk/'])
        self.assertEqual(self.unit.deleted_folders, [])

    def test_remove_file_never_removes_lost_found(self):
        disk = self.disk({'lost+found': made_up(40), 'a': b'1'})
        with self.assertRaisesRegex(DiskError, 'never removes lost\\+found'):
            disk.remove_file('lost+found')
        with self.assertRaisesRegex(DiskError, 'never removes lost\\+found'):
            disk.write(self.local('n', b'n'), '', 'lost+found')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)
        self.assertEqual(self.unit.paths(), ['a', 'lost+found'])

    def test_remove_file_asks_the_unit_again_and_removes_only_the_object_it_listed(self):
        disk = self.disk({'logs/x.log': b'1', 'other/x.log': b'2', 'logs/y.log': b'3'})
        disk.children('logs')
        other = self.unit.find(self.unit.make('other'), 'x.log')
        disk._handles['logs/x.log'] = H.Entry(other, False, 1)   # The cache is old: same name, other folder.
        with self.assertRaisesRegex(DiskError, 'no longer where it was listed'):
            disk.remove_file('logs/x.log')
        disk.children('logs')
        self.unit.at('logs/y.log').name = 'z.log'                  # Renamed on the unit since it was listed.
        with self.assertRaisesRegex(DiskError, 'no longer where it was listed'):
            disk.remove_file('logs/y.log')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)
        self.assertEqual(self.unit.paths(), ['logs/', 'logs/x.log', 'logs/z.log', 'other/', 'other/x.log'])

    def test_a_top_level_file_whose_object_info_says_parent_0_is_removed(self):
        disk = self.disk({'os.ldr': b'old', 'logs/x.log': b'1'})
        disk.children('')
        self.assertEqual(disk._info(disk._handles['os.ldr'].handle, 'describe os.ldr').parent, 0)
        disk.remove_file('os.ldr')
        self.assertEqual(self.unit.ops.count(DELETE_OBJECT), 1)
        self.assertEqual(self.unit.paths(), ['logs/', 'logs/x.log'])

    def test_a_file_listed_at_the_top_whose_object_info_names_a_folder_is_refused(self):
        disk = self.disk({'logs/x.log': b'1'})
        inside = self.unit.find(self.unit.make('logs'), 'x.log')
        with self.assertRaisesRegex(DiskError, 'no longer where it was listed'):
            disk._delete(inside, TOP, 'x.log', 'x.log')
        self.assertNotIn(DELETE_OBJECT, self.unit.ops)
        self.assertEqual(self.unit.paths(), ['logs/', 'logs/x.log'])

    def test_the_one_delete_refuses_0_and_ffffffff_whoever_asks(self):
        disk = self.disk({'a': b'1'})
        for bad in (0, TOP):
            with self.assertRaisesRegex(DiskError, 'never removes the object numbered'):
                disk._delete(bad, TOP, 'a', 'a')
        self.assertEqual(self.unit.ops, [OPEN_SESSION, STORAGE_IDS])

    def test_make_folder(self):
        disk = self.disk({'user/x': b'1'})
        disk.make_folder('user', 'pk')
        self.assertEqual(sorted(disk.children('user')), [('pk', True, 0), ('x', False, 1)])
        self.assertEqual(disk.children('user/pk'), [])
        disk.write(self.local('p', b'pack'), 'user/pk', 'base.pack')
        self.assertEqual(self.unit.at('user/pk/base.pack').data, b'pack')
        for name in ('pk', 'x'):
            with self.assertRaisesRegex(DiskError, 'already has a'):
                disk.make_folder('user', name)
        with self.assertRaisesRegex(DiskError, 'the unit has no nowhere'):
            disk.make_folder('nowhere', 'pk')

    def test_the_cache_of_handles_is_dropped_after_every_change(self):
        disk = self.disk({'user/pk/a.pack': b'old', 'logs/one.log': b'1'})
        disk.read('user/pk/a.pack', self.local('r1'))
        self.assertIn('user/pk/a.pack', disk._handles)
        listings = self.unit.ops.count(OBJECT_HANDLES)
        disk.read('user/pk/a.pack', self.local('r2'))
        self.assertEqual(self.unit.ops.count(OBJECT_HANDLES), listings)   # Known: not listed again.
        for change in (lambda: disk.write(self.local('n', b'newer'), 'user/pk', 'a.pack'),
                       lambda: disk.make_folder('user', 'more'),
                       lambda: disk.remove_file('logs/one.log')):
            disk.children('user')
            change()
            self.assertEqual(disk._handles, {'': H.Entry(TOP, True, 0)})
        self.assertEqual(disk.read('user/pk/a.pack', self.local('r3')), 5)   # The new handle, not the old one.
        self.assertEqual(self.bytes_of('r3'), b'newer')


class AnswerTests(Base):
    def test_an_mtp_error_in_the_middle_of_a_listing_is_said_and_the_session_goes_on(self):
        disk = self.disk({'a': b'1', 'b': b'2', 'c': b'3'})
        self.unit.refuse[OBJECT_INFO] = [None, GENERAL_ERROR]
        with self.assertRaises(DiskError) as caught:
            disk.children('')
        self.assertEqual(caught.exception.kind, 'refused')
        self.assertIn('the unit refused to list the top of its disk', str(caught.exception))
        self.assertIn('MTP answer 0x2002', str(caught.exception))
        self.assertEqual(len(disk.children('')), 3)

    def test_a_refused_send_names_the_problem(self):
        disk = self.disk(folders=['user'])
        self.unit.refuse[SEND_OBJECT] = [0x200C]
        with self.assertRaisesRegex(DiskError, r'refused to store user/x.bin: its disk is full \(MTP answer 0x200c\)'):
            disk.write(self.local('x', made_up(2000)), 'user', 'x.bin')
        self.unit.refuse[SEND_OBJECT_INFO] = [0x2019]
        with self.assertRaisesRegex(DiskError, 'it is busy'):
            disk.write(self.local('x', made_up(2000)), 'user', 'y.bin')

    def test_a_refused_read_leaves_no_file_behind(self):
        disk = self.disk({'a.log': made_up(100)})
        disk.children('')
        self.unit.refuse[GET_OBJECT] = [0x200F]
        with self.assertRaisesRegex(DiskError, 'refused access'):
            disk.read('a.log', self.local('a'))
        self.assertFalse(os.path.exists(self.local('a')))

    def test_a_reply_out_of_order_cancels_and_leaves_the_session(self):
        disk = self.disk({'a': b'1'})
        self.unit.reply.append(box(RESPONSE_KIND, OK, 99))
        with self.assertRaisesRegex(DiskError, 'did not ask for'):
            disk.children('')
        self.assertEqual(self.unit.cancelled, [2])
        disk.close()
        self.assertTrue(self.unit.closed)
        self.assertNotIn(CLOSE_SESSION, self.unit.ops)

    def test_after_a_broken_transaction_nothing_more_is_sent(self):
        disk = self.disk({'a': b'1'}, folders=['user'])
        self.unit.reply.append(box(RESPONSE_KIND, OK, 99))
        with self.assertRaisesRegex(DiskError, 'did not ask for'):
            disk.children('')
        self.unit.reply.clear()
        sent = len(self.unit.ops)
        for action in (lambda: disk.children(''), lambda: disk.read('a', self.local('a')),
                       lambda: disk.write(self.local('x', b'x'), 'user', 'x'), lambda: disk.remove_file('a')):
            with self.assertRaises(DiskError) as caught:
                action()
            self.assertEqual(caught.exception.kind, 'broken')
        self.assertEqual(len(self.unit.ops), sent)

    def test_a_session_already_open_is_taken_and_close_ends_it(self):
        disk = self.disk({'a': b'1'}, session_already_open=True)
        self.assertEqual(disk.storage, STORAGE)
        self.assertEqual(disk.storages, [STORAGE, SECOND_STORAGE])
        disk.close()
        self.assertEqual(self.unit.ops[-1], CLOSE_SESSION)
        self.assertTrue(self.unit.closed)
        disk.close()

    def test_every_operation_carries_the_next_transaction_id(self):
        disk = self.disk({'user/a': made_up(PACKET - 12)})
        disk.write(self.local('b', made_up(TRANSFER)), 'user', 'a')
        disk.make_folder('user', 'pk')
        disk.remove_file('user/a')
        disk.close()
        self.assertEqual(self.unit.next_transaction, len(self.unit.ops))
        self.assertEqual(self.unit.faults, [])


class GuardTests(unittest.TestCase):
    def test_the_system_libusb_is_never_reached(self):
        self.assertIs(H.Libusb, NoRealLibusb)
        with self.assertRaises(AssertionError):
            H.Libusb()

    def test_the_proven_product_ids_are_exactly_these(self):
        # A change of the list is a deliberate edit of this test too: 0x0102 rests on one unit listed read-only.
        self.assertEqual(H.KNOWN_PRODUCTS, (0x0102,))
        self.assertEqual(PRODUCT, 0x0102)
        self.assertNotIn(OTHER, H.KNOWN_PRODUCTS)
        for product in H.KNOWN_PRODUCTS:
            self.assertIn('0x%04x' % product, H.UNPROVEN)


class DeleteTests(Base):
    def test_the_delete_itself_sends_nothing_that_changes_an_unproven_or_read_only_unit(self):
        for product, read_only, kind in ((OTHER, False, 'unproven'), (PRODUCT, True, 'usage')):
            with self.subTest(kind):
                self.watch(Responder({'a': b'1', 'user/b': b'2'}))
                self.unit.product = product
                disk = UsbDisk(self.unit, read_only=read_only)
                self.addCleanup(disk.close)
                disk.children('')
                disk.children('user')
                for handle, parent, name, path in ((disk._handles['a'].handle, H.ROOT, 'a', 'a'),
                                                   (disk._handles['user/b'].handle, disk._handles['user'].handle,
                                                    'b', 'user/b')):
                    with self.assertRaises(DiskError) as caught:
                        disk._delete(handle, parent, name, path)
                    self.assertEqual(caught.exception.kind, kind)
                self.assertNotIn(DELETE_OBJECT, self.unit.ops)
                self.assertEqual(self.unit.paths(), ['a', 'user/', 'user/b'])
                disk.close()


class OpenFailureTests(Base):
    def test_a_failure_after_the_session_opened_closes_it_and_gives_the_interface_back(self):
        def refused(u):
            u.refuse[STORAGE_IDS] = [GENERAL_ERROR]

        def none(u):
            u.storages = []

        def zero(u):
            u.storages = [0]
        for what, spoil, words_ in (('storage ids refused', refused, 'name its storages'),
                                    ('no storage', none, 'shows no storage'), ('storage id 0', zero, '0x00000000')):
            with self.subTest(what):
                responder = self.watch(Responder({'a': b'1'}))
                spoil(responder)
                libusb = FakeLibusb([unit()], responder)
                with self.assertRaises(DiskError) as caught:
                    UsbDisk(Usb(libusb))
                self.assertIn(words_, str(caught.exception))
                self.assertEqual(responder.ops, [OPEN_SESSION, STORAGE_IDS, CLOSE_SESSION])
                self.assertEqual(libusb.calls[-3:], [('release', 0), 'close', 'exit'])

    def test_a_session_that_did_not_open_is_not_closed(self):
        responder = self.watch(Responder({'a': b'1'}))
        responder.refuse[OPEN_SESSION] = [GENERAL_ERROR]
        libusb = FakeLibusb([unit()], responder)
        with self.assertRaises(DiskError):
            UsbDisk(Usb(libusb))
        self.assertEqual(responder.ops, [OPEN_SESSION])
        self.assertEqual(libusb.calls[-3:], [('release', 0), 'close', 'exit'])


class UsbTests(Base):
    def open_with(self, devices, **options):
        self.libusb = FakeLibusb(devices, Responder({'a': b'1'}), **options)
        with self.assertRaises(DiskError) as caught:
            Usb(self.libusb)
        return caught.exception

    def test_no_unit(self):
        for devices in ([], [unit(vendor=0x1234)], [unit(interfaces=AUDIO_INTERFACES)]):
            error = self.open_with(devices)
            self.assertEqual(error.kind, 'absent')
            self.assertTrue(str(error).startswith(H.NOT_FOUND))
        error = self.open_with([unit(interfaces=None)])
        self.assertIn('bus 007, device 042', str(error))

    def test_a_unit_not_proven_is_opened_but_only_read(self):
        for product in (OTHER, 0x0001):
            self.watch(Responder({'a': b'1', 'logs/b.log': made_up(10)}))
            self.libusb = FakeLibusb([unit(product=product)], self.unit)
            disk = UsbDisk(Usb(self.libusb))
            self.addCleanup(disk.close)
            self.assertEqual(sorted(disk.children('')), [('a', False, 1), ('logs', True, 0)])
            self.assertEqual(disk.read('logs/b.log', self.local('b%d' % product)), 10)
            for change in (lambda: disk.write(self.local('x', b'x'), 'logs', 'x'),
                           lambda: disk.make_folder('', 'n'), lambda: disk.remove_file('a'),
                           lambda: disk.remove_file('logs/b.log')):
                with self.assertRaises(DiskError) as caught:
                    change()
                self.assertEqual(caught.exception.kind, 'unproven')
                for words_ in ('product id is 0x%04x' % product, 'has run on a unit with the id 0x0102 only', '--list-only',
                               'send what it prints'):
                    self.assertIn(words_, str(caught.exception))
            self.assertFalse(set(self.unit.ops) & CHANGING, self.unit.ops)
            self.assertEqual(self.unit.paths(), ['a', 'logs/', 'logs/b.log'])
            disk.close()

    def test_more_than_one_unit(self):
        error = self.open_with([unit(ref='one'), unit(ref='two', address=43)])
        self.assertEqual((error.kind, str(error)), ('several', H.SEVERAL))
        self.assertNotIn(('open', 'one'), self.libusb.calls)

    @unittest.skipIf(H.MAC, "asserts the Linux texts; the macOS ones are held in test_mac_claim_retry")
    def test_no_permission(self):
        for options in ({'open_code': -3}, {'claim_code': -3}):
            error = self.open_with([unit()], **options)
            self.assertEqual(error.kind, 'permission')
            for words_ in ('bus 007, device 042', 'product id 0102', 'this user may not open it',
                           '\n  SUBSYSTEM=="usb", ATTR{idVendor}=="2367", TAG+="uaccess"\n', '/etc/udev/rules.d/',
                           'sudo udevadm control --reload', 'unplug the cable and plug it in again'):
                self.assertIn(words_, str(error))
        self.assertEqual(self.libusb.calls[-2:], ['close', 'exit'])
        self.assertNotIn(('release', 0), self.libusb.calls)

    @unittest.skipUnless(H.MAC, "macOS only")
    def test_mac_claim_retry(self):
        # ptpcamerad holds the unit for good: every retry is refused, and the owner reads the macOS text.
        error = self.open_with([unit()], claim_code=-3)
        self.assertEqual(error.kind, 'permission')
        for words_ in ('product id 0102', 'another program holds it', 'killall ptpcamerad'):
            self.assertIn(words_, str(error))
        self.assertEqual(self.libusb.calls.count(('claim', 0)), 6)   # the first claim and five after a kill

    @unittest.skipIf(H.MAC, "asserts the Linux texts; the macOS ones are held in test_mac_claim_retry")
    def test_the_interface_is_busy(self):
        error = self.open_with([unit()], claim_code=-6)
        self.assertEqual(error.kind, 'busy')
        for words_ in ('the desktop\'s file manager holds the unit', 'Unmount "OP-1" there', 'gio mount -u mtp://',
                       'then run the command again'):
            self.assertIn(words_, str(error))
        self.assertNotIn(('release', 0), self.libusb.calls)
        self.assertEqual(self.libusb.calls[-2:], ['close', 'exit'])

    def test_the_unit_goes_away(self):
        error = self.open_with([unit()], open_code=-4)
        self.assertEqual(error.kind, 'gone')
        disk = self.over_usb({'logs/a.log': made_up(3 * TRANSFER)})
        disk.children('logs')
        self.libusb.in_codes = [0, -4]
        with self.assertRaises(DiskError) as caught:
            disk.read('logs/a.log', self.local('a'))
        self.assertEqual(caught.exception.kind, 'gone')
        self.assertIn('went away', str(caught.exception))
        self.assertFalse(os.path.exists(self.local('a')))
        self.assertIn(('control', 0x64), self.libusb.calls)   # The transaction was cancelled.

    def test_the_unit_stops_answering(self):
        disk = self.over_usb({'a': b'1'})
        self.libusb.in_codes = [-7]
        with self.assertRaisesRegex(DiskError, 'stopped answering') as caught:
            disk.children('')
        self.assertEqual(caught.exception.kind, 'gone')

    def test_a_stall_is_cleared_once_and_the_transfer_tried_again(self):
        disk = self.over_usb({'a': b'1'})
        self.libusb.in_codes = [-9]
        self.assertEqual(disk.children(''), [('a', False, 1)])
        self.assertEqual([c for c in self.libusb.calls if c[0] == 'clear_halt'], [('clear_halt', 0x81)])
        self.libusb.out_codes = [-9, -9]
        with self.assertRaises(DiskError):
            disk.children('')

    def test_a_stall_after_part_of_a_transfer_is_not_tried_again(self):
        for codes in ({'out_codes': [(-9, 5)]}, {'in_codes': [(-9, b'\x10\x00\x00')]}):
            disk = self.over_usb({'a': b'1'})
            for key, value in codes.items():
                setattr(self.libusb, key, value)
            with self.assertRaisesRegex(DiskError, 'stalled'):
                disk.children('')
            self.assertNotIn(('clear_halt', 0x81), self.libusb.calls)
            self.assertNotIn(('clear_halt', 0x02), self.libusb.calls)

    def test_libusb_bulk_in_gives_the_bytes_of_a_failed_transfer(self):
        class Lib:
            def libusb_bulk_transfer(self, handle, endpoint, buffer, length, count, timeout):
                ct.memmove(buffer, b'abc', 3)
                count._obj.value = 3
                return -9
        libusb = REAL_LIBUSB.__new__(REAL_LIBUSB)   # Made without __init__: the system's libusb is not loaded.
        libusb.lib, libusb.buffer = Lib(), (ct.c_ubyte * TRANSFER)()
        self.assertEqual(libusb.bulk_in(None, 0x81, TRANSFER, 10), (-9, b'abc'))

    def test_a_whole_session_over_usb_and_close_lets_go(self):
        disk = self.over_usb({'logs/a.log': made_up(TRANSFER + 3)}, active=0)
        self.assertIn(('set_configuration', 1), self.libusb.calls)
        self.assertIn(('claim', 0), self.libusb.calls)
        self.assertEqual(disk.read('logs/a.log', self.local('a')), TRANSFER + 3)
        disk.write(self.local('b', made_up(TRANSFER * 2)), 'logs', 'b.log')
        self.assertEqual(self.unit.at('logs/b.log').data, made_up(TRANSFER * 2))
        disk.close()
        self.assertEqual(self.unit.ops[-1], CLOSE_SESSION)
        self.assertEqual(self.libusb.calls[-3:], [('release', 0), 'close', 'exit'])


class ListOnlyTests(Base):
    def test_list_only_prints_the_unit_and_every_object_and_sends_only_reading_operations(self):
        for product in (PRODUCT, OTHER):
            self.watch(Responder({'os.ldr': made_up(30), 'content/a b.aif': made_up(5), 'lost+found': made_up(4),
                                  'tapes/long.aif': 0x100000005}, folders=['empty']))
            self.unit.product = product
            out = io.StringIO()
            H.usb_list_only(out, self.unit)
            lines = out.getvalue().splitlines()
            self.assertEqual(lines[0], 'Vendor id 2367, product id %04x, bus 007, device 042.' % product)
            self.assertEqual(lines[1], 'MTP interface: class 06, subclass 01, protocol 01.')
            self.assertEqual(lines[2], 'Storage ids: 00010001, 00020001. The first is used.')
            rows = [json.loads(line) for line in lines[3:-2]]
            self.assertIn({'path': 'content/a b.aif', 'size': 5, 'dir': False}, rows)
            self.assertIn({'path': 'tapes/long.aif', 'size': 0x100000005, 'dir': False}, rows)
            self.assertIn({'path': 'empty', 'dir': True}, rows)
            self.assertEqual(lines[-2], '4 files, 3 folders.')
            self.assertEqual(lines[-1], 'The drive does not look like the bootloader\'s: it has no folder user or '
                                        'upgrade at its top. A run of the helper stops on it.')
            self.assertFalse(set(self.unit.ops) & CHANGING, self.unit.ops)
            self.assertEqual(self.unit.ops[-1], CLOSE_SESSION)
            self.assertTrue(self.unit.closed)

    def test_list_only_opens_the_disk_to_read_only_even_on_a_proven_unit(self):
        made = []

        class Seen(UsbDisk):
            def __init__(self, *args, **kwargs):
                super().__init__(*args, **kwargs)
                made.append(self)
        self.assertIn(PRODUCT, H.KNOWN_PRODUCTS)
        self.watch(Responder({'a': b'1'}, folders=['user']))
        with mock.patch.object(H, 'UsbDisk', Seen):
            H.usb_list_only(io.StringIO(), self.unit)
        self.assertEqual(len(made), 1)
        self.assertIs(made[0].read_only, True)
        self.assertFalse(set(self.unit.ops) & CHANGING, self.unit.ops)

    def test_a_disk_opened_to_read_only_sends_no_change(self):
        self.watch(Responder({'a': b'1'}, folders=['user']))
        disk = UsbDisk(self.unit, read_only=True)
        self.addCleanup(disk.close)
        for change in (lambda: disk.write(self.local('x', b'x'), 'user', 'x'), lambda: disk.make_folder('', 'n'),
                       lambda: disk.remove_file('a')):
            with self.assertRaisesRegex(DiskError, 'read only'):
                change()
        self.assertFalse(set(self.unit.ops) & CHANGING, self.unit.ops)
        self.assertEqual(self.unit.paths(), ['a', 'user/'])


# The whole helper over the stand-in unit.

STOCK = bytes(range(256)) * 4      # Made up: stands for the owner's firmware file.
IMAGE = b'\x11\x22\x33' * 1000     # Made up: stands for the page's new firmware file.
UNIT_FILES = {'upgrade/firmware/blackfin/bin/os.ldr': STOCK, 'content/a.aif': made_up(100), '.hidden': b'h',
              'logs/one.txt': b'first log', 'logs/two.txt': b'second', 'logs/old/three.txt': b'kept',
              'open-field-pad.bin': made_up(300, 3), 'os.ldr': b'old top' * 10, 'lost+found': made_up(40, 4),
              'user/pk/lfo.pack': b'o' * 70}
CHANGING_OR_READING_FILES = CHANGING | {GET_OBJECT}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def put(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(data)


def snapshot(responder):
    """Every object of the stand-in unit: its path, and None for a folder or its bytes for a file."""
    def path(handle):
        o = responder.objects[handle]
        return (path(o.parent) + '/' if o.parent != TOP else '') + o.name
    return {path(h): (None if o.data is None else bytes(o.data)) for h, o in responder.objects.items()}


def page_plan(shared, pad=None, packs=()):
    """The install page's part: a plan made from the helper's listing, its files written into the shared folder.
    packs: [(name, bytes)]. user/pk is made when the listing has none."""
    with open(os.path.join(shared, 'listing-before.json'), 'rb') as f:
        before = f.read()
    with open(os.path.join(shared, 'helper.json'), 'rb') as f:
        run = json.loads(f.read())['run']
    by = {r['path']: r for r in json.loads(before)}
    put(os.path.join(shared, 'open-field-os.ldr'), IMAGE)
    plan = {'plan': 1, 'run': run, 'image': {'file': 'open-field-os.ldr', 'name': 'os.ldr', 'path': 'os.ldr',
                                             'bytes': len(IMAGE), 'sha256': sha(IMAGE)},
            'pad': None, 'makeFolder': None, 'packs': [], 'before': {'sha256': sha(before)}}
    by['os.ldr'] = {'path': 'os.ldr', 'size': len(IMAGE), 'dir': False}
    if pad is not None:
        put(os.path.join(shared, 'open-field-pad.bin'), pad)
        plan['pad'] = {'file': 'open-field-pad.bin', 'name': 'open-field-pad.bin', 'bytes': len(pad),
                       'sha256': sha(pad)}
        by['open-field-pad.bin'] = {'path': 'open-field-pad.bin', 'size': len(pad), 'dir': False}
    if packs and 'user/pk' not in by:
        plan['makeFolder'] = 'user/pk'
        by['user/pk'] = {'path': 'user/pk', 'dir': True}
    for name, data in packs:
        put(os.path.join(shared, 'packs', name), data)
        path = 'user/pk/' + name
        plan['packs'].append({'file': 'packs/' + name, 'name': name, 'path': path, 'bytes': len(data),
                              'sha256': sha(data)})
        by[path] = {'path': path, 'size': len(data), 'dir': False}
    plan['expected'] = sorted(by.values(), key=lambda r: r['path'])
    with open(os.path.join(shared, 'plan.json.part'), 'w', encoding='utf-8') as f:
        f.write(json.dumps(plan))
    os.replace(os.path.join(shared, 'plan.json.part'), os.path.join(shared, 'plan.json'))


class OverUsb(Base):
    """The helper's command line with no --source: the unit is found through a stand-in libusb that leads to a
    Responder. Each search for the unit makes a new FakeLibusb, as each makes a new Libusb."""

    def setUp(self):
        super().setUp()
        self.shared = os.path.join(self.temp, 'open-field')
        self.home = os.path.join(self.temp, 'home')       # Never made: nothing may be written into it.
        for patch in (mock.patch.object(H, 'Libusb', self.libusb_of),
                      mock.patch.dict(os.environ, {'HOME': self.home})):
            patch.start()
            self.addCleanup(patch.stop)
        self.found = []             # The FakeLibusb of every search, in order.
        self.devices = [[unit()]]   # The devices each search finds; the last stays.
        self.options = {}

    def libusb_of(self):
        devices = self.devices.pop(0) if len(self.devices) > 1 else self.devices[0]
        libusb = FakeLibusb(devices, self.unit, **self.options)
        self.found.append(libusb)
        return libusb

    def plug(self, files=UNIT_FILES, folders=('tape01',), **options):
        self.watch(Responder(files, folders))
        self.options = options
        return snapshot(self.unit)

    def run_helper(self, page=None, args=()):
        """The helper run in a thread; `page()` is called once it has listed the disk. (code, helper.json, lines)."""
        out, res = [], {}
        argv = ['--folder', self.shared, '--wait-minutes', '1', '--find-seconds', '1'] + list(args)
        thread = threading.Thread(target=lambda: res.setdefault(
            'code', H.main(argv, echo=lambda line, color: out.append(line))))
        thread.start()
        if page is not None:
            until = time.monotonic() + 5
            while time.monotonic() < until and self.state() not in ('listed', 'stopped'):
                time.sleep(0.001)
            self.assertEqual(self.state(), 'listed', out)
            page()
        thread.join(20)
        self.assertFalse(thread.is_alive())
        try:
            with open(os.path.join(self.shared, 'helper.json'), 'rb') as f:
                h = json.loads(f.read())
        except OSError:
            h = None
        return res.get('code'), h, out

    def state(self):
        try:
            with open(os.path.join(self.shared, 'helper.json'), 'rb') as f:
                return json.loads(f.read())['state']
        except (OSError, ValueError):
            return None

    def assertStopped(self, code, h, why):
        self.assertEqual(code, 1)
        self.assertEqual((h['state'], h['message']), ('stopped', why))
        self.assertEqual(h['lines'][-2:], ['STOPPED: ' + why, H.NOTHING_MORE])

    def assertLetGo(self):
        """The session was closed and the unit's interface given back."""
        self.assertEqual(self.unit.ops[-1], CLOSE_SESSION)
        self.assertEqual(self.found[-1].calls[-3:], [('release', 0), 'close', 'exit'])


class FullRunOverUsb(OverUsb):
    def test_the_helper_and_the_transport_say_the_same_texts(self):
        self.assertEqual(H.SEVERAL, H.MORE_THAN_ONE)
        self.assertEqual(H.NOT_FOUND, H.NOT_SHOWN)

    def test_a_unit_not_proven_stops_the_run_before_anything_is_taken_off(self):
        before = self.plug()
        self.devices = [[unit(product=OTHER)]]
        code, h, out = self.run_helper()
        why = H.UNPROVEN.format(product=OTHER)
        self.assertStopped(code, h, why)
        for words_ in ('0x0199', '--list-only', 'changes nothing'):
            self.assertIn(words_, why)
        self.assertIn("Found the unit's drive (OP-1).", h['lines'])
        self.assertEqual(h['unit'], 'OP-1')
        self.assertEqual((h['stock'], h['logs'], h['oldPads'], h['listing']), (None, None, None, None))
        self.assertEqual(snapshot(self.unit), before)
        self.assertEqual(self.unit.ops, [OPEN_SESSION, STORAGE_IDS, CLOSE_SESSION])
        self.assertLetGo()
        self.assertEqual(sorted(n for n in os.listdir(self.shared) if not n.startswith('helper-')), ['helper.json'])

    def test_a_proven_unit_is_brought_to_ready_and_changed_only_where_the_helper_may(self):
        before = self.plug()
        pad, a_pack, lfo = made_up(500, 9), b'A' * 33, b'L' * 90
        code, h, out = self.run_helper(lambda: page_plan(self.shared, pad=pad, packs=[('a.pack', a_pack),
                                                                                      ('lfo.pack', lfo)]))
        self.assertEqual(code, 0, h['lines'])
        self.assertEqual((h['state'], h['message'], h['unit']), ('ready', 'ready to write', 'OP-1'))
        self.assertTrue(out[-3].endswith('READY TO WRITE.'), out[-3:])
        want = dict(before)
        for gone in ('logs/one.txt', 'logs/two.txt', 'open-field-pad.bin'):
            del want[gone]
        want.update({'os.ldr': IMAGE, 'open-field-pad.bin': pad, 'user/pk/a.pack': a_pack, 'user/pk/lfo.pack': lfo})
        self.assertEqual(snapshot(self.unit), want)
        self.assertEqual((self.unit.deleted_folders, self.unit.deleted_everything), ([], False))
        self.assertEqual(h['logs'], {'folder': 'logs-' + h['run'], 'copied': 2, 'left_on_unit': 0})
        self.assertEqual(h['oldPads'], {'found': 1, 'taken_off': 1, 'left_on_unit': 0})
        self.assertEqual(h['stock']['sha256'], sha(STOCK))
        # Two logs, one old pad, the old os.ldr and the old lfo.pack.
        self.assertEqual(self.unit.ops.count(DELETE_OBJECT), 5)
        self.assertLetGo()

    def test_a_proven_unit_gets_the_pack_folder_the_plan_makes(self):
        files = {k: v for k, v in UNIT_FILES.items() if not k.startswith('user/')}
        before = self.plug(files, folders=('tape01', 'user'))
        code, h, out = self.run_helper(lambda: page_plan(self.shared, packs=[('base.pack', b'B' * 55)]))
        self.assertEqual(code, 0, h['lines'])
        want = dict(before)
        for gone in ('logs/one.txt', 'logs/two.txt', 'open-field-pad.bin'):
            del want[gone]
        want.update({'os.ldr': IMAGE, 'user/pk': None, 'user/pk/base.pack': b'B' * 55})
        self.assertEqual(snapshot(self.unit), want)

    def test_a_drive_of_the_normal_disk_mode_stops_before_anything_is_read_or_changed(self):
        normal = {'synth/a.aif': made_up(10), 'drum/b.aif': made_up(10, 2), 'tapes/track_1.aif': made_up(10, 3),
                  'logs/a.txt': b'log', 'open-field-pad.bin': made_up(30, 4), 'os.ldr': b'o',
                  'drum/upgrade/firmware/blackfin/bin/os.ldr': STOCK}
        for what, files, folders in (('normal mode', normal, ('mixdown',)),
                                     ('in capitals', normal, ('mixdown', 'USER', 'Upgrade')),
                                     ('files of those names', dict(normal, user=b'u', upgrade=b'g'), ('mixdown',))):
            with self.subTest(what):
                shutil.rmtree(self.shared, ignore_errors=True)
                before = self.plug(files, folders)
                code, h, out = self.run_helper()
                self.assertStopped(code, h, H.NOT_BOOTLOADER)
                self.assertEqual(snapshot(self.unit), before)
                self.assertFalse(set(self.unit.ops) & CHANGING_OR_READING_FILES, self.unit.ops)
                self.assertEqual((h['stock'], h['logs'], h['oldPads'], h['listing']), (None, None, None, None))
                self.assertLetGo()
                self.assertEqual(sorted(n for n in os.listdir(self.shared) if not n.startswith('helper-')),
                                 ['helper.json'])

    def test_the_proven_id_in_the_normal_disk_mode_stops_at_the_bootloader_check(self):
        # A unit shows 0x0102 in its normal disk mode too: the id alone must not let a run change it.
        self.assertIn(PRODUCT, H.KNOWN_PRODUCTS)
        files = {'synth/a.aif': made_up(10), 'drum/b.aif': made_up(10, 2), 'tapes/track_1.aif': made_up(10, 3),
                 'mixdown/c.aif': made_up(10, 5)}
        before = self.plug(files, folders=())
        self.devices = [[unit(product=PRODUCT)]]
        code, h, out = self.run_helper()
        self.assertStopped(code, h, H.NOT_BOOTLOADER)
        self.assertEqual(sorted({path.split('/')[0] for path in before}), ['drum', 'mixdown', 'synth', 'tapes'])
        self.assertEqual((h['stock'], h['logs'], h['oldPads'], h['listing']), (None, None, None, None))
        self.assertLetGo()
        self.assertFalse(set(self.unit.ops) & CHANGING, self.unit.ops)
        self.assertEqual(snapshot(self.unit), before)

    def test_the_gate_stands_before_the_unit_over_usb(self):
        before = self.plug()
        disk = UsbDisk(self.unit)
        self.addCleanup(disk.close)
        gate = H.Gate(disk)
        sent = list(self.unit.ops)
        for call in (lambda: gate.write(self.local('x', b'x'), '', 'os.ldr'), lambda: gate.make_folder('user', 'pk'),
                     lambda: gate.remove_file('logs/one.txt'), lambda: gate.remove_file('lost+found')):
            with self.assertRaisesRegex(DiskError, 'rules do not allow'):
                call()
        self.assertEqual(self.unit.ops, sent)
        self.assertEqual(snapshot(self.unit), before)

    def test_r2b_no_unit_in_find_seconds(self):
        self.plug()
        self.devices = [[]]
        code, h, out = self.run_helper()
        self.assertStopped(code, h, H.NOT_SHOWN)
        self.assertIn(H.WAITING, h['lines'])
        self.assertEqual(self.unit.ops, [])

    def test_a_unit_that_shows_after_a_wait_is_taken(self):
        self.plug()
        self.devices = [[], [], [unit(product=OTHER)]]
        code, h, out = self.run_helper(args=['--find-seconds', '60'])
        self.assertIn(H.WAITING, h['lines'])
        self.assertEqual(h['message'], H.UNPROVEN.format(product=OTHER))
        self.assertEqual(len(self.found), 3)

    def test_r2a_two_units(self):
        self.plug()
        self.devices = [[unit(ref='one'), unit(ref='two', address=43)]]
        code, h, out = self.run_helper()
        self.assertStopped(code, h, H.MORE_THAN_ONE)
        self.assertFalse([c for c in self.found[-1].calls if c[0] == 'open'])
        self.assertEqual(self.unit.ops, [])

    @unittest.skipIf(H.MAC, "asserts the Linux texts; the macOS ones are held in test_mac_claim_retry")
    def test_no_permission_and_busy_show_the_transports_texts_and_stop(self):
        for options, kind, words_ in (({'open_code': -3}, 'permission', 'uaccess'),
                                      ({'claim_code': -3}, 'permission', '70-op1-field.rules'),
                                      ({'claim_code': -6}, 'busy', 'gio mount -u')):
            self.plug(**options)
            shutil.rmtree(self.shared, ignore_errors=True)
            code, h, out = self.run_helper()
            self.assertEqual(code, 1)
            self.assertIn(words_, h['message'])
            self.assertTrue(h['message'].startswith(H.BUSY if kind == 'busy' else 'the unit is connected'))
            self.assertEqual(self.unit.ops, [])
            self.assertEqual(self.found[-1].calls[-1], 'exit')
            self.assertNotIn(('release', 0), self.found[-1].calls)


class ListOnlyOverUsb(OverUsb):
    def list_only(self, args=()):
        out = []
        code = H.main(['--list-only', '--find-seconds', '1'] + list(args), echo=lambda line, color: out.append(line))
        return code, out

    def assertNothingMade(self):
        self.assertFalse(os.path.exists(self.shared))
        self.assertFalse(os.path.exists(self.home))
        self.assertEqual(os.listdir(self.temp), [])

    def test_list_only_prints_the_unit_and_changes_nothing_anywhere(self):
        for product in (PRODUCT, OTHER):
            before = self.plug()
            self.devices = [[unit(product=product)]]
            code, out = self.list_only(['--folder', self.shared])
            self.assertEqual(code, 0, out)
            self.assertEqual(out[0], 'Vendor id 2367, product id %04x, bus 007, device 042.' % product)
            self.assertIn(json.dumps({'path': 'logs/one.txt', 'size': 9, 'dir': False}, separators=(',', ':')), out)
            self.assertEqual(out[-2], '10 files, 10 folders.')
            self.assertEqual(out[-1], 'The drive looks like the bootloader\'s: it has a folder user or upgrade at its top.')
            self.assertEqual(snapshot(self.unit), before)
            self.assertFalse(set(self.unit.ops) & CHANGING_OR_READING_FILES, self.unit.ops)
            self.assertLetGo()
            self.assertNothingMade()

    def test_list_only_waits_for_the_unit(self):
        self.plug()
        self.devices = [[], [], [unit()]]
        code, out = self.list_only(['--find-seconds', '60'])
        self.assertEqual(code, 0, out)
        self.assertEqual(out[0], H.WAITING)
        self.assertNothingMade()

    @unittest.skipIf(H.MAC, "asserts the Linux texts; the macOS ones are held in test_mac_claim_retry")
    def test_list_only_stops_with_the_texts_of_the_transport(self):
        cases = (([[]], {}, H.NOT_SHOWN), ([[unit(ref='one'), unit(ref='two', address=43)]], {}, H.MORE_THAN_ONE),
                 ([[unit()]], {'open_code': -3}, None), ([[unit()]], {'claim_code': -6}, H.BUSY))
        for devices, options, why in cases:
            self.plug(**options)
            self.devices = devices
            code, out = self.list_only()
            self.assertEqual(code, 1)
            if why is None:
                self.assertTrue(out[-1].startswith('STOPPED: the unit is connected (bus 007, device 042, product id 0102), '
                                                   'but this user may not open it'), out)
                self.assertIn('TAG+="uaccess"', out[-1])
            else:
                self.assertEqual(out[-1], 'STOPPED: ' + why)
            self.assertEqual(self.unit.ops, [])
            self.assertNothingMade()

    def test_list_only_takes_no_source(self):
        self.assertEqual(self.list_only(['--source', self.temp])[0], 2)
        self.assertEqual(self.found, [])


if __name__ == '__main__':
    unittest.main()

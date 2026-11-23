"""The helper for Linux and macOS, builder/open-field-helper.py, run whole on a folder that stands for the unit's disk.

  python -I -m unittest discover -s tools/tests -p "test_linux_helper.py"

Each test makes a stand-in disk and a shared folder in a temporary folder, runs the helper in a thread, and plays
the install page's part in Python: it waits for the listing, writes the files and a plan made by hand from that
listing, and lets the helper finish. Then it holds helper.json, the listings, the shared folder and the whole
stand-in disk (every name, size and SHA-256) to what the helper may do. The bytes of every file are made up here.
No unit and no USB are used: the transport over USB is not tested here. Where a test needs a disk that tells capitals
from small letters, as the unit's does, it uses MemoryDisk, so that it runs the same on a Mac's usual disk. The home
folder of every test is made up: the owner's own is never searched.
"""

import ast
import contextlib
import errno
import hashlib
import importlib.util
import io
import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unicodedata
import unittest
from unittest import mock

if not sys.platform.startswith(("linux", "darwin")):
    # Before the helper is loaded: it is for Linux and macOS, and these tests use os.O_NOFOLLOW, named pipes and links.
    raise unittest.SkipTest("the helper for Linux and macOS is tried on those two only")

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HELPER = os.path.join(ROOT, "builder", "open-field-helper.py")
MARK = "1a2b3c4d"


class NoRealLibusb:
    """Stands in the helper's Libusb: the system's libusb is never loaded here. The transport over USB is tested in
    test_linux_helper_usb.py."""

    def __init__(self):
        raise AssertionError("a test reached for the system's libusb")


def load():
    spec = importlib.util.spec_from_file_location("linux_helper", HELPER)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def no_real_killall():
    raise AssertionError("a test would have ended ptpcamerad")


H = load()
H.Libusb = NoRealLibusb     # the guard, set as the module is loaded: no test can reach the system's libusb
H.end_ptpcamerad = no_real_killall


def setUpModule():
    H.SECOND = 0.002        # every wait of the helper, 500 times shorter: a minute is 0.12 seconds


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read(path):
    with open(path, "rb") as f:
        return f.read()


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def entry(data):
    return ("F", len(data), sha(data))


def tree(root):
    """Every folder, file and link under root: path -> ("D",), ("F", size, sha256) or ("L", target)."""
    out = {}

    def failed(error):
        raise error                 # a folder that cannot be listed is never taken for an empty one
    for here, dirs, files in os.walk(root, onerror=failed):
        rel = os.path.relpath(here, root)
        for name in dirs + files:
            path = name if rel == "." else rel.replace(os.sep, "/") + "/" + name
            full = os.path.join(here, name)
            if os.path.islink(full):
                out[path] = ("L", os.readlink(full))
            elif os.path.isdir(full):
                out[path] = ("D",)
            else:
                out[path] = entry(read(full))
    return out


def make_unit(root, spec):
    """A stand-in disk: path -> bytes for a file, None for a folder."""
    os.makedirs(root, exist_ok=True)
    for path, data in spec.items():
        full = os.path.join(root, *path.split("/"))
        if data is None:
            os.makedirs(full, exist_ok=True)
        else:
            write(full, data)
    return root


STOCK = bytes(range(256)) * 4                   # made up: stands for the owner's firmware file
IMAGE = b"\x11\x22\x33" * 1000                  # made up: stands for the page's new firmware file
PLAIN = {
    "content/a.aif": b"a" * 100, "content/b.aif": b"b" * 250, ".hidden": b"h", "firmware.version": b"x.y.z",
    "upgrade/firmware/blackfin/bin/os.ldr": STOCK, "user": None, "tape01": None,
}


def case_sensitive(folder):
    probe = os.path.join(folder, "case-probe")
    write(probe, b"")
    try:
        return not os.path.exists(os.path.join(folder, "CASE-PROBE"))
    finally:
        os.remove(probe)


def make_pipe(path):
    """A named pipe; False where this disk takes none (a Windows disk seen from Linux)."""
    try:
        os.mkfifo(path)
        return True
    except OSError:
        return False


class MemoryDisk(H.Disk):
    """A disk held in memory. Names count letter for letter, as on the unit, whatever this computer's disk does."""

    name = "a disk held in memory"

    def __init__(self, spec):
        self.at = {}
        for path, data in spec.items():
            self.parents(path)
            self.at[path] = data

    def parents(self, path):
        parts = path.split("/")[:-1]
        for n in range(1, len(parts) + 1):
            self.at["/".join(parts[:n])] = None

    def children(self, folder):
        if folder and self.at.get(folder, b"") is not None:
            raise H.DiskError("the unit has no folder " + folder)
        out = [(p.rpartition("/")[2], d is None, 0 if d is None else len(d))
               for p, d in self.at.items() if p.rpartition("/")[0] == folder]
        return sorted(out)

    def read(self, path, local_path):
        data = self.at.get(path)
        if data is None:
            raise H.DiskError("the unit has no file " + path)
        with H.new_file(local_path) as f:
            f.write(data)
        return len(data)

    def write(self, local_path, folder_path, name):
        path = H.join(folder_path, name)
        if path in self.at and self.at[path] is None:
            raise H.DiskError("a folder is where the file goes")
        self.at[path] = read(local_path)

    def make_folder(self, folder_path, name):
        path = H.join(folder_path, name)
        if path in self.at:
            raise H.DiskError("it is there already")
        self.at[path] = None

    def remove_file(self, path):
        if self.at[path] is None:
            raise H.DiskError("a folder is never removed")
        del self.at[path]

    def tree(self):
        return {p: ("D",) if d is None else entry(d) for p, d in self.at.items()}


def page_plan(shared, image=IMAGE, pad=None, packs=()):
    """The page's part: the plan made from the helper's listing, with its files written into the shared folder.
    packs: [(name, bytes)] in the order the plan gives."""
    before = read(os.path.join(shared, "listing-before.json"))
    helper = json.loads(read(os.path.join(shared, "helper.json")))
    rows = json.loads(before)
    by = {r["path"]: r for r in rows}
    write(os.path.join(shared, "open-field-os.ldr"), image)
    plan = {"plan": 1, "run": helper["run"], "made": "2026-11-19T10:00:00.000Z",
            "image": {"file": "open-field-os.ldr", "name": "os.ldr", "path": "os.ldr", "bytes": len(image), "sha256": sha(image), "replaces": "os.ldr" in by},
            "pad": None, "makeFolder": None, "packs": [], "wayBack": False, "before": {"file": "listing-before.json", "sha256": sha(before)}}
    by["os.ldr"] = {"path": "os.ldr", "size": len(image), "dir": False}
    if pad is not None:
        name = next(n for n in ["open-field-pad.bin"] + ["open-field-pad-%d.bin" % i for i in range(2, 9)]
                    if n.lower() not in {p.lower() for p in by if "/" not in p})
        write(os.path.join(shared, name), pad)
        plan["pad"] = {"file": name, "name": name, "bytes": len(pad), "sha256": sha(pad), "marginNodes": 1, "marginRecords": 1}
        by[name] = {"path": name, "size": len(pad), "dir": False}
    if packs:
        if "user/pk" not in by:
            plan["makeFolder"] = "user/pk"
            by["user/pk"] = {"path": "user/pk", "dir": True}
        for name, data in packs:
            write(os.path.join(shared, "packs", name), data)
            path = "user/pk/" + name
            plan["packs"].append({"file": "packs/" + name, "name": name, "path": path, "bytes": len(data), "sha256": sha(data), "replaces": path in by})
            by[path] = {"path": path, "size": len(data), "dir": False}
    plan["expected"] = sorted(by.values(), key=lambda r: r["path"])
    return plan


def put_plan(shared, plan):
    path = os.path.join(shared, "plan.json")
    with open(path + ".part", "w", encoding="utf-8") as f:
        f.write(json.dumps(plan, indent=1))
    os.replace(path + ".part", path)


def helper_json(shared):
    try:
        return json.loads(read(os.path.join(shared, "helper.json")))
    except (OSError, ValueError):
        return None


class Recording(H.Disk):
    """A disk that passes every call to another and notes it, with the state helper.json says at that moment.
    `hooks` may change a call: name -> function(disk, *args) run before it. A read hook may give "spoil" (one byte
    changed) or "short" (the last byte missing); a write or make_folder hook "skip"; any hook may raise."""

    def __init__(self, inner, shared, hooks=None):
        self.inner, self.shared, self.hooks, self.calls = inner, shared, hooks or {}, []
        self.name = inner.name

    def note(self, what, *args):
        h = helper_json(self.shared)
        self.calls.append((what, args, h and h["state"]))
        if what in self.hooks:
            return self.hooks[what](self, *args)

    def children(self, folder_path):
        self.note("children", folder_path)
        return self.inner.children(folder_path)

    def read(self, path, local_path):
        how = self.note("read", path, local_path)
        if how == "spoil":
            data = read(os.path.join(self.inner.root, *path.split("/")))
            write(local_path, bytes([data[0] ^ 1]) + data[1:])
            return len(data)
        if how == "short":
            data = read(os.path.join(self.inner.root, *path.split("/")))
            write(local_path, data[:-1])
            return len(data) - 1
        return self.inner.read(path, local_path)

    def write(self, local_path, folder_path, name):
        if self.note("write", local_path, folder_path, name) == "skip":
            return
        self.inner.write(local_path, folder_path, name)

    def make_folder(self, folder_path, name):
        if self.note("make_folder", folder_path, name) == "skip":
            return
        self.inner.make_folder(folder_path, name)

    def remove_file(self, path):
        self.note("remove_file", path)
        self.inner.remove_file(path)

    def close(self):
        self.note("close")


class ForwardClock:
    """The helper's time module, whose monotonic clock a test moves on once the helper has looked often enough."""

    def __init__(self):
        self.ahead, self.looks = 0, 0

    def monotonic(self):
        self.looks += 1
        return time.monotonic() + self.ahead

    def run_out(self):
        """Every deadline over, once the helper went round its loop twice more (it looks twice in each round)."""
        then, until = self.looks, time.monotonic() + 20
        while self.looks < then + 5 and time.monotonic() < until:
            time.sleep(0.001)
        self.ahead = 1000000

    def __getattr__(self, name):
        return getattr(time, name)


class HelperCase(unittest.TestCase):
    def setUp(self):
        self.tmp = os.path.realpath(tempfile.mkdtemp(prefix="of-helper-"))    # a Mac's /var is a link to /private/var
        self.shared = os.path.join(self.tmp, "Any Folder")
        self.home = os.path.join(self.tmp, "home")
        self.was_in = os.getcwd()
        env = mock.patch.dict(os.environ, {"HOME": self.home})
        env.start()
        self.addCleanup(env.stop)
        for name in ("OPEN_FIELD_PAGE", "OPEN_FIELD_FOLDER", "XDG_CONFIG_HOME"):
            os.environ.pop(name, None)
        self.unit, self.units = os.path.join(self.tmp, "unit"), 0
        os.makedirs(self.shared)
        self.mark()
        self.saved = (H.ORIGINAL_SHA, H.find_unit, H.Gate)

    def tearDown(self):
        H.ORIGINAL_SHA, H.find_unit, H.Gate = self.saved
        os.chdir(self.was_in)
        shutil.rmtree(self.tmp, ignore_errors=True)

    def mark(self, mark=MARK, bom=False):
        write(os.path.join(self.shared, "page.json"), (b"\xef\xbb\xbf" if bom else b"") +
              json.dumps({"page": 1, "id": mark, "made": "2026-11-19T10:00:00.000Z"}).encode())

    def go(self, page=None, args=(), disk=None, wait="60", mark=MARK, source=True, named=True, ask=None):
        """The helper run in a thread; `page(helper)` is called once it has listed the disk. (code, helper.json, lines)."""
        out, res = [], {}
        argv = (["--folder", self.shared] if named else []) + ["--wait-minutes", wait] + (["--page", mark] if mark else [])
        if source and disk is None:
            argv += ["--source", self.unit]
        argv += list(args)
        th = threading.Thread(target=lambda: res.setdefault("code", H.main(argv, disk=disk, echo=lambda line, color: out.append(line),
                                                                              ask=ask or self.not_asked)))
        th.start()
        if page is not None:
            until = time.monotonic() + 20
            while time.monotonic() < until:
                h = helper_json(self.shared)
                if h and h["state"] in ("listed", "stopped"):
                    break
                time.sleep(0.001)
            self.assertEqual(h and h["state"], "listed", h and h["lines"])
            page(h)
        th.join(30)
        self.assertFalse(th.is_alive())
        return res.get("code"), helper_json(self.shared), out

    def not_asked(self, question):
        raise AssertionError("the helper asked: " + question)

    def assertStopped(self, code, h, why):
        self.assertEqual(code, 1)
        self.assertEqual(h["state"], "stopped")
        self.assertEqual(h["message"], why)
        self.assertEqual(h["lines"][-2:], ["STOPPED: " + why, H.NOTHING_MORE])

    def new_unit(self):
        """Another stand-in disk, at a path of its own: a disk that removes late cannot mix two subtests."""
        self.units += 1
        self.unit = os.path.join(self.tmp, "unit-%d" % self.units)

    def clear_shared(self):
        """The shared folder back to page.json alone, between two runs: two runs in one second have one run id."""
        for n in os.listdir(self.shared):
            if n != "page.json":
                q = os.path.join(self.shared, n)
                shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)

    def plain_unit(self, more=None):
        spec = dict(PLAIN)
        spec.update(more or {})
        return make_unit(self.unit, spec)

    def refused(self, mutate, why, pad=None, packs=(("x.pack", b"x" * 40),), more=None):
        """A plan the helper must refuse before it writes anything: the stand-in disk is left as it was."""
        self.plain_unit(more)
        before = tree(self.unit)

        def page(h):
            plan = page_plan(self.shared, pad=pad, packs=packs)
            mutate(plan)
            put_plan(self.shared, plan)
        code, h, _ = self.go(page)
        self.assertStopped(code, h, why)
        self.assertEqual(tree(self.unit), before)
        return h


class Whole(HelperCase):
    def test_ready_with_logs_pads_os_ldr_and_packs(self):
        old_lfo, new_lfo, a_pack, base = b"o" * 70, b"L" * 90, b"A" * 33, b"B" * 55
        self.plain_unit({
            "logs/one.txt": b"first log", "logs/two.txt": b"second", "logs/old/three.txt": b"kept",
            "open-field-pad.bin": b"p" * 300, "OPEN-FIELD-PAD-2.BIN": b"q" * 10, "Open-Field-Pad-7.bin": b"r",
            "open-field-pad-x.bin": b"s", "open-field-pad.bin.txt": b"t", "user/open-field-pad.bin": b"u",
            "open-field-pad-3.bin/inside": b"v", "os.ldr": b"old top" * 10, "user/pk/lfo.pack": old_lfo,
            "user/pkin/.apply": b"note", "user/pkin/next.pack": b"waits", "user/pkin/sub/kept": b"kept",
            "user/pk/.apply": b"not the waiting folder's", "user/other.txt": b"kept",
        })
        before = tree(self.unit)
        self.mark()
        write(os.path.join(self.shared, "plan.json"), b"{}")                     # stale, from an earlier run
        write(os.path.join(self.shared, "listing-after.json"), b"[]")
        disk = Recording(H.FolderDisk(self.unit), self.shared)
        seen = {}

        def page(h):
            seen["lines"] = list(h["lines"])
            seen["stale"] = [n for n in ("plan.json", "listing-after.json") if os.path.exists(os.path.join(self.shared, n))]
            seen["before"] = read(os.path.join(self.shared, "listing-before.json"))
            put_plan(self.shared, page_plan(self.shared, pad=b"P" * 500, packs=[("a.pack", a_pack), ("lfo.pack", new_lfo), ("base.pack", base)]))
        code, h, out = self.go(page, disk=disk)
        self.assertEqual(code, 0, h["lines"])
        run = h["run"]
        self.assertRegex(run, r"^\d{4}-\d\d-\d\d_\d\d-\d\d-\d\d$")
        self.assertRegex(h["time"], r"^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d$")
        self.assertEqual((h["helper"], h["page"], h["state"], h["message"]), (1, MARK, "ready", "ready to write"))
        self.assertEqual(h["folder"], os.path.abspath(self.shared))
        self.assertEqual(h["unit"], "a folder standing for the unit: " + self.unit)
        self.assertEqual(seen["stale"], [])

        # the stock file, the logs, the old pads
        self.assertEqual(h["stock"], {"file": "os-from-upgrade-folder-" + run + ".ldr", "bytes": len(STOCK), "sha256": sha(STOCK),
                                      "original": False, "from": "upgrade/firmware/blackfin/bin/os.ldr"})
        self.assertEqual(read(os.path.join(self.shared, h["stock"]["file"])), STOCK)
        self.assertEqual(h["logs"], {"folder": "logs-" + run, "copied": 2, "left_on_unit": 0})
        self.assertEqual(tree(os.path.join(self.shared, "logs-" + run)), {"one.txt": entry(b"first log"), "two.txt": entry(b"second")})
        self.assertEqual(h["oldPads"], {"found": 3, "taken_off": 3, "left_on_unit": 0})
        self.assertEqual(h["waiting"], {"found": 2, "taken_off": 2, "left_on_unit": 0})
        self.assertNotIn("user/pkin/.apply", [r["path"] for r in json.loads(seen["before"])])

        # the listing: one line, no byte order mark, rows of their form, its SHA-256 the file's own bytes
        text = seen["before"]
        self.assertFalse(text.startswith(b"\xef\xbb\xbf") or b"\n" in text or b" " in text.replace(b"open-field-pad", b""))
        rows = json.loads(text)
        self.assertEqual(h["listing"], {"file": "listing-before.json", "files": sum(not r["dir"] for r in rows),
                                        "folders": sum(r["dir"] for r in rows), "sha256": sha(text)})
        self.assertIn({"path": "logs/old", "dir": True}, rows)
        self.assertIn({"path": ".hidden", "size": 1, "dir": False}, rows)
        self.assertTrue(all(list(r) == (["path", "dir"] if r["dir"] else ["path", "size", "dir"]) for r in rows))
        index = {r["path"]: i for i, r in enumerate(rows)}
        self.assertTrue(all(index[p.rsplit("/", 1)[0]] < i for p, i in index.items() if "/" in p), "a folder's row before its contents")
        self.assertNotIn("open-field-pad.bin", index)
        self.assertNotIn("logs/one.txt", index)

        # the checks, the lines, the shared folder
        plan = json.loads(read(os.path.join(self.shared, "plan.json")))
        self.assertEqual(h["checks"], ["the listing is the plan's, row for row (" + str(len(plan["expected"])) + " rows)",
                                       "os.ldr read back: " + sha(IMAGE), "open-field-pad.bin read back: " + sha(b"P" * 500),
                                       "user/pk/a.pack read back: " + sha(a_pack), "user/pk/lfo.pack read back: " + sha(new_lfo),
                                       "user/pk/base.pack read back: " + sha(base)])
        self.assertEqual(h["lines"][:len(seen["lines"])], seen["lines"], "lines are only added to")
        self.assertEqual(h["lines"][0], "open-field helper 1, run " + run + ". The folder: " + os.path.abspath(self.shared))
        for line in ["Found the unit's drive (a folder standing for the unit: " + self.unit + ").",
                     "The 2 log file(s) are taken off the unit into logs-" + run + ".",
                     "The 3 pad file(s) of earlier installs are taken off the unit.",
                     "The 2 file(s) waiting in user/pkin are taken off the unit.",
                     "The disk is listed: %d files, %d folders." % (h["listing"]["files"], h["listing"]["folders"]),
                     "The page's plan is here.", "The disk is as it was listed.", "os.ldr is on the unit (3000 bytes).",
                     "The pad file open-field-pad.bin is on the unit (500 bytes).", "The pack file base.pack is on the unit.",
                     "The disk's new listing is the plan's, row for row.", "user/pk/lfo.pack read back from the unit: the same bytes."]:
            self.assertIn(line, h["lines"])
        # as the Windows helper does, the four last lines are said after the state "ready" is saved
        said = [l[10:] for l in out if l]
        self.assertEqual(said[:len(h["lines"])], h["lines"])
        self.assertEqual(len(said), len(h["lines"]) + 4)
        self.assertEqual(said[-4:], [
            "READY TO WRITE.",
            "Now restart your OP-1 in bootloader mode: turn it off, then turn it on while holding COM.",
            "Do NOT restart it in normal mode until the install is finished, otherwise you will have to start again.",
            "Step 7 of the install page has the rest."])
        log = read(os.path.join(self.shared, "helper-" + run + ".log")).decode("utf-8").splitlines()
        self.assertEqual([l[10:] for l in log], said)
        self.assertEqual(H.compare_rows(plan["expected"], json.loads(read(os.path.join(self.shared, "listing-after.json")))), ([], []))
        left = sorted(os.listdir(self.shared))
        self.assertEqual(left, sorted(["page.json", "helper.json", "helper-" + run + ".log", "listing-before.json", "listing-after.json",
                                       "plan.json", "open-field-os.ldr", "open-field-pad.bin", "packs", "logs-" + run, h["stock"]["file"]]))
        self.assertFalse(read(os.path.join(self.shared, "helper.json")).startswith(b"\xef\xbb\xbf"))

        # the whole stand-in disk: only what the helper may change has changed
        want = dict(before)
        for gone in ("logs/one.txt", "logs/two.txt", "open-field-pad.bin", "OPEN-FIELD-PAD-2.BIN", "Open-Field-Pad-7.bin",
                     "user/pkin/.apply", "user/pkin/next.pack"):
            del want[gone]
        want.update({"os.ldr": entry(IMAGE), "open-field-pad.bin": entry(b"P" * 500), "user/pk/a.pack": entry(a_pack),
                     "user/pk/lfo.pack": entry(new_lfo), "user/pk/base.pack": entry(base)})
        self.assertEqual(tree(self.unit), want)

        # the order of the changes, and nothing written before "putting" nor after the end
        changes = [(c[0], c[1][-1] if c[0] != "remove_file" else c[1][0], c[2]) for c in disk.calls if c[0] in ("write", "make_folder", "remove_file")]
        self.assertEqual([c[1] for c in changes if c[0] == "write"], ["os.ldr", "open-field-pad.bin", "a.pack", "lfo.pack", "base.pack"])
        self.assertTrue(all(c[2] == "putting" for c in changes if c[0] == "write"))
        self.assertTrue(all(c[2] == "starting" for c in changes if c[0] == "remove_file"))
        self.assertEqual(disk.calls[-1][0], "close")

    def test_folder_made_and_lookups_in_capitals(self):
        unit = MemoryDisk({"UPGRADE/Firmware/blackfin/bin/os.ldr": STOCK, "UPGRADE/Firmware/blackfin/bin/OS.LDR": b"other",
                           "Logs/a.txt": b"log", "user/keep.txt": b"k", "content/x.aif": b"x" * 9})
        before = unit.tree()
        packs = [("zeta.pack", b"z" * 20), ("Alpha.pack", b"a" * 21), ("base.pack", b"b" * 22)]   # the plan's order, not sorted
        disk = Recording(unit, self.shared)
        self.mark("1A2B3C4D", bom=True)                  # the mark in capitals, page.json with a byte order mark
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=packs)), disk=disk)
        self.assertEqual(code, 0, h["lines"])
        self.assertEqual(h["stock"]["sha256"], sha(STOCK), "the exact name first")
        self.assertEqual(h["logs"]["copied"], 1)
        self.assertIn("The folder user/pk is made on the unit.", h["lines"])
        self.assertEqual([c[1][-1] for c in disk.calls if c[0] == "write"], ["os.ldr", "zeta.pack", "Alpha.pack", "base.pack"])
        want = dict(before)
        del want["Logs/a.txt"]
        want.update({"os.ldr": entry(IMAGE), "user/pk": ("D",)})
        want.update({"user/pk/" + n: entry(d) for n, d in packs})
        self.assertEqual(unit.tree(), want)

    def test_only_the_exact_pack_name_is_replaced(self):
        unit = MemoryDisk(dict(PLAIN, **{"user/pk/LFO.pack": b"old", "logs": None}))
        before = unit.tree()
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=[("lfo.pack", b"new!")])), disk=unit)
        self.assertEqual(code, 0, h["lines"])
        self.assertIn("The unit's logs folder holds no file.", h["lines"])
        want = dict(before, **{"os.ldr": entry(IMAGE), "user/pk/lfo.pack": entry(b"new!")})
        self.assertEqual(unit.tree(), want)

    def test_no_mark_no_check(self):
        self.plain_unit()
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), mark=None)
        self.assertEqual((code, h["page"]), (0, None))

    def test_mark_from_the_environment(self):
        self.plain_unit()
        self.mark()
        os.environ["OPEN_FIELD_PAGE"] = MARK
        try:
            code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), mark=None)
        finally:
            del os.environ["OPEN_FIELD_PAGE"]
        self.assertEqual((code, h["page"]), (0, MARK))

    def test_plan_fields_the_helper_does_not_know_are_taken(self):
        self.plain_unit()

        def page(h):
            plan = page_plan(self.shared, pad=b"P" * 32, packs=[("x.pack", b"x" * 40)])
            plan.update(bootloader="1000.2.0", base={"from": "the page", "kept": None}, chosen=[{"name": "x.pack"}], later={"a": [1]})
            for step in (plan["image"], plan["pad"], plan["packs"][0], plan["before"]):
                step["a field to come"] = True
            put_plan(self.shared, plan)
        code, h, _ = self.go(page)
        self.assertEqual((code, h["state"]), (0, "ready"), h["lines"])


class Stock(HelperCase):
    def stock_run(self, kept=None):
        self.plain_unit()
        if kept is not None:
            write(os.path.join(self.shared, "os-1.7.3-yours.ldr"), kept)
        code, h, _ = self.go(wait="1")
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        self.assertFalse(any(n.startswith("incoming-") for n in os.listdir(self.shared)))
        return h

    def test_the_original_file(self):
        H.ORIGINAL_SHA = sha(STOCK)                     # made-up bytes stand for the original here
        h = self.stock_run()
        self.assertEqual((h["stock"]["file"], h["stock"]["original"]), ("os-1.7.3-yours.ldr", True))
        self.assertIn("Your own firmware file is copied off: os-1.7.3-yours.ldr (the original OS 1.7.3 file, 1024 bytes).", h["lines"])
        self.assertEqual(read(os.path.join(self.shared, "os-1.7.3-yours.ldr")), STOCK)

    def test_the_original_file_kept_once(self):
        H.ORIGINAL_SHA = sha(STOCK)
        h = self.stock_run(kept=STOCK)
        self.assertEqual(h["stock"]["file"], "os-1.7.3-yours.ldr")
        self.assertEqual(sorted(n for n in os.listdir(self.shared) if n.endswith(".ldr")), ["os-1.7.3-yours.ldr"])

    def test_a_kept_file_of_other_bytes_is_not_written_over(self):
        H.ORIGINAL_SHA = sha(STOCK)
        h = self.stock_run(kept=b"mine")
        self.assertEqual(h["stock"]["file"], "os-1.7.3-yours-" + h["run"] + ".ldr")
        self.assertEqual(read(os.path.join(self.shared, "os-1.7.3-yours.ldr")), b"mine")

    def test_no_firmware_file_on_the_unit(self):
        self.plain_unit()
        os.remove(os.path.join(self.unit, "upgrade/firmware/blackfin/bin/os.ldr"))
        code, h, _ = self.go(wait="1")
        self.assertIsNone(h["stock"])
        self.assertIn("The unit has no upgrade/firmware/blackfin/bin/os.ldr. The page will ask for the firmware file you kept.", h["lines"])
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")

    def test_r5_a_link_is_not_read_through(self):
        self.plain_unit()
        bin_ = os.path.join(self.unit, "upgrade/firmware/blackfin/bin")
        os.remove(os.path.join(bin_, "os.ldr"))
        write(os.path.join(self.tmp, "elsewhere.ldr"), STOCK)
        os.symlink(os.path.join(self.tmp, "elsewhere.ldr"), os.path.join(bin_, "os.ldr"))
        before = tree(self.unit)
        code, h, _ = self.go()
        self.assertStopped(code, h, "the firmware file did not come off the unit: "
                                    "the unit has no such place: upgrade/firmware/blackfin/bin/os.ldr")
        self.assertEqual(tree(self.unit), before)


class Refusals(HelperCase):
    def test_r0_no_page_json_nothing_made(self):
        shutil.rmtree(self.shared)
        code, h, out = self.go()
        self.assertEqual((code, h), (2, None))
        self.assertFalse(os.path.exists(self.shared))
        self.assertEqual(out, ["STOPPED: the install page did not choose this folder: " + self.shared,
                               "On the install page, step 3, pick your working folder again,",
                               "then copy the line of step 4 again and run it. Nothing was done."])

    def test_r0_another_mark(self):
        self.plain_unit()
        self.mark("ffffffff")
        code, h, out = self.go()
        self.assertEqual((code, h, os.listdir(self.shared)), (2, None, ["page.json"]))

    def test_r1_no_such_source(self):
        self.mark()
        code, h, _ = self.go(args=["--source", os.path.join(self.tmp, "nowhere")], source=False)
        self.assertStopped(code, h, "no such folder: " + os.path.join(self.tmp, "nowhere"))

    def test_r2_usb(self):
        self.mark()
        H.find_unit = lambda note=None: None
        code, h, _ = self.go(args=["--find-seconds", "1"], source=False)
        self.assertStopped(code, h, H.NOT_SHOWN)
        self.assertIn(H.WAITING, h["lines"])

        def two(note=None):
            raise H.DiskError(H.MORE_THAN_ONE)
        H.find_unit = two
        self.clear_shared()
        code, h, _ = self.go(source=False)
        self.assertStopped(code, h, H.MORE_THAN_ONE)

    def test_r7_a_plan_of_another_run_is_not_taken(self):
        self.plain_unit()
        self.mark()
        before = tree(self.unit)
        clock = ForwardClock()

        def page(h):
            plan = page_plan(self.shared)
            plan["run"] = "2000-01-01_00-00-00"
            put_plan(self.shared, plan)
            clock.run_out()
        with mock.patch.object(H, "time", clock):
            code, h, _ = self.go(page)
        self.assertStopped(code, h, "no plan came from the page in 60 minutes")
        self.assertEqual(tree(self.unit), before)

    def test_r8_to_r12_and_unsafe_plans(self):
        self.mark()
        outside = os.path.join(self.tmp, "outside.ldr")
        write(outside, IMAGE)

        def link_out(p):
            os.symlink(outside, os.path.join(self.shared, "out.ldr"))
            p["image"]["file"] = "out.ldr"

        def two_of_one_name(p):
            p["packs"].append(dict(p["packs"][0], name="X.pack", path="user/pk/X.pack"))

        def gone(name):
            return lambda p: os.remove(os.path.join(self.shared, name))

        def spoil(name):
            return lambda p: write(os.path.join(self.shared, name), b"other")

        unsafe = "the plan names a file that is not a plain name inside this folder: "
        place = "the plan names a place on the unit this helper does not write: "
        cases = [
            ("R8", lambda p: p.update(plan=2), "the plan is of a kind this helper does not know (2)"),
            ("R9", lambda p: p["before"].update(sha256="0" * 64), "the plan was not made from this run's listing"),
            ("R9 none", lambda p: p.pop("before"), "the plan was not made from this run's listing"),
            ("R10 gone", gone("open-field-os.ldr"), "the new firmware file open-field-os.ldr is not in the folder as the plan says"),
            ("R10 bytes", spoil("open-field-os.ldr"), "the new firmware file open-field-os.ldr is not in the folder as the plan says"),
            ("R10 size", lambda p: p["image"].update(bytes=5), "the new firmware file open-field-os.ldr is not in the folder as the plan says"),
            ("R11", spoil("open-field-pad.bin"), "the pad file open-field-pad.bin is not in the folder as the plan says"),
            ("R12", gone("packs/x.pack"), "the pack file packs/x.pack is not in the folder as the plan says"),
            ("absolute", lambda p: p["image"].update(file=outside), unsafe + outside),
            ("dot dot", lambda p: p["image"].update(file="../outside.ldr"), unsafe + "../outside.ldr"),
            ("dot dot inside", lambda p: p["image"].update(file="packs/../open-field-os.ldr"), unsafe + "packs/../open-field-os.ldr"),
            ("a link out", link_out, unsafe + "out.ldr"),
            ("pad file", lambda p: p["pad"].update(file="../../x.bin"), unsafe + "../../x.bin"),
            ("pack file", lambda p: p["packs"][0].update(file="packs/../../x.pack"), unsafe + "packs/../../x.pack"),
            ("image path", lambda p: p["image"].update(path="upgrade/firmware/blackfin/bin/os.ldr"), place + "upgrade/firmware/blackfin/bin/os.ldr"),
            ("pad name", lambda p: p["pad"].update(name="content/a.aif"), place + "content/a.aif"),
            ("pad name 2", lambda p: p["pad"].update(name="pad.bin"), place + "pad.bin"),
            ("make folder", lambda p: p.update(makeFolder="upgrade/pk"), place + "upgrade/pk"),
            ("pack path", lambda p: p["packs"][0].update(path="user/pk/../../os.ldr"), place + "user/pk/../../os.ldr"),
            ("pack elsewhere", lambda p: p["packs"][0].update(path="upgrade/x.pack", name="x.pack"), place + "upgrade/x.pack"),
            ("pack not .pack", lambda p: p["packs"][0].update(path="user/pk/x.bin", name="x.bin"), place + "user/pk/x.bin"),
            ("pack name", lambda p: p["packs"][0].update(name="y.pack"), place + "user/pk/x.pack"),
            ("two of one name", two_of_one_name, "the plan names two pack files of one name: X.pack"),
        ]
        for what, mutate, why in cases:
            with self.subTest(what):
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        p = os.path.join(self.shared, n)
                        shutil.rmtree(p) if os.path.isdir(p) and not os.path.islink(p) else os.remove(p)
                self.new_unit()
                self.refused(mutate, why, pad=b"P" * 64)

    def test_r13_the_disk_changed(self):
        self.plain_unit()
        self.mark()

        def page(h):
            write(os.path.join(self.unit, "content", "new.aif"), b"abc")
            os.remove(os.path.join(self.unit, "content", "a.aif"))
            put_plan(self.shared, page_plan(self.shared))
        code, h, _ = self.go(page)
        self.assertStopped(code, h, "the unit's disk changed since it was listed, so the plan no longer holds")
        self.assertIn("  gone since the listing: F content/a.aif 100", h["lines"])
        self.assertIn("  new since the listing: F content/new.aif 3", h["lines"])
        self.assertFalse(os.path.exists(os.path.join(self.unit, "os.ldr")))

    def test_r14_the_staged_copy(self):
        self.plain_unit()
        self.mark()
        before = tree(self.unit)

        def change_image(disk, folder):
            if folder == "" and os.path.exists(os.path.join(self.shared, "plan.json")):
                write(os.path.join(self.shared, "open-field-os.ldr"), b"changed")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"children": change_image})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertStopped(code, h, "the copy of the new firmware file on this computer is not the plan's")
        self.assertEqual(tree(self.unit), before)

    def after_os_ldr(self, why, plan_more=None, disk_hooks=None, unit_more=None):
        """A stop after os.ldr is written: the disk has os.ldr, and nothing else of the plan's steps after it."""
        self.plain_unit(unit_more)
        self.mark()
        before = tree(self.unit)

        def page(h):
            plan = page_plan(self.shared, packs=[("x.pack", b"x" * 40)])
            if plan_more:
                plan_more(plan)
            put_plan(self.shared, plan)
        disk = Recording(H.FolderDisk(self.unit), self.shared, disk_hooks)
        code, h, _ = self.go(page, disk=disk)
        self.assertStopped(code, h, why)
        self.assertEqual(disk.calls[-1][0], "close")
        return before, h

    def test_r3_no_pack_folder_before_any_write(self):
        def no_folder(p):
            p["makeFolder"] = None
            p["expected"] = [r for r in p["expected"] if r["path"] != "user/pk"]
        self.refused(no_folder, "the unit has no folder user/pk")

    def test_r15_no_user_folder_before_any_write(self):
        self.plain_unit()
        shutil.rmtree(os.path.join(self.unit, "user"))
        before = tree(self.unit)

        def page(h):
            plan = page_plan(self.shared, packs=[("x.pack", b"x" * 40)])
            plan["expected"] = [r for r in plan["expected"] if not r["path"].startswith("user")]
            put_plan(self.shared, plan)
        code, h, _ = self.go(page)
        self.assertStopped(code, h, "the unit has no folder user")
        self.assertEqual(tree(self.unit), before)

    def test_r16_the_folder_does_not_appear(self):
        before, h = self.after_os_ldr("the folder user/pk did not appear on the unit", disk_hooks={"make_folder": lambda d, f, n: "skip"})
        self.assertEqual(tree(self.unit), dict(before, **{"os.ldr": entry(IMAGE)}))

    def test_r4_the_file_does_not_show(self):
        self.plain_unit()
        self.mark()
        before = tree(self.unit)
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"write": lambda d, l, f, n: "skip"})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertEqual(code, 1)
        self.assertRegex(h["message"], r"^the unit did not show os\.ldr with its 3000 bytes in \d+ seconds$")
        self.assertEqual(tree(self.unit), before)

    def test_r17_not_the_plan(self):
        def a_file_comes(d, local, folder, name):
            if name == "os.ldr":
                write(os.path.join(self.unit, "content", "new.aif"), b"abc")
        before, h = self.after_os_ldr("the disk is not as the plan says it must be for the write", disk_hooks={"write": a_file_comes})
        self.assertIn("  on the unit and not planned: F content/new.aif 3", h["lines"])
        self.assertTrue(os.path.exists(os.path.join(self.shared, "listing-after.json")))
        self.assertEqual(h["checks"], [])

    def test_r18_read_back(self):
        hooks = {"read": lambda d, path, local: "spoil" if path == "os.ldr" else None}
        before, h = self.after_os_ldr("os.ldr does not read back from the unit with the SHA-256 of the file sent", disk_hooks=hooks)
        self.assertEqual(sum(l.startswith("  os.ldr read back with another SHA-256 (") for l in h["lines"]), 5)
        self.assertIn("try 5 of 5", h["lines"][-3])

    def test_r19_an_error(self):
        self.plain_unit()
        self.mark()

        def fault(d, local, folder, name):
            raise RuntimeError("a made-up fault")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"write": fault})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertEqual((code, h["state"], h["message"]), (1, "stopped", "a made-up fault"))
        self.assertEqual(h["lines"][-1], "STOPPED by an error: a made-up fault")


class Parts(HelperCase):
    def test_the_stand_in_disk(self):
        make_unit(self.unit, {".dot": b"d", "a/b": b"bb", "dir": None})
        os.symlink(os.path.join(self.unit, "a"), os.path.join(self.unit, "link"))
        disk = H.FolderDisk(self.unit)
        kids = disk.children("")
        self.assertEqual(kids, [(".dot", False, 1), ("a", True, 0), ("dir", True, 0), ("link", False, os.lstat(os.path.join(self.unit, "link")).st_size)])
        with self.assertRaises(H.DiskError):
            disk.children("link")
        with self.assertRaises(H.DiskError):
            disk.read("link", os.path.join(self.tmp, "x"))
        with self.assertRaises(H.DiskError):
            disk.remove_file("dir")
        with self.assertRaises(H.DiskError):
            disk.children("a/b")
        write(os.path.join(self.tmp, "new"), b"new")
        neighbour = "B" if case_sensitive(self.tmp) else "b2"     # on a Mac's usual disk "B" is the same file
        write(os.path.join(self.unit, "a", neighbour), b"next")
        disk.write(os.path.join(self.tmp, "new"), "a", "b")
        self.assertEqual((read(os.path.join(self.unit, "a", "b")), read(os.path.join(self.unit, "a", neighbour))), (b"new", b"next"))
        self.assertEqual(disk.read("a/b", os.path.join(self.tmp, "back")), 3)

    def test_the_gate(self):
        make_unit(self.unit, {"logs/a": b"a", "content/a.aif": b"a", "open-field-pad.bin": b"p", "user": None,
                              "user/open-field-pad.bin": b"u", "content/open-field-pad-2.bin": b"c"})
        gate = H.Gate(H.FolderDisk(self.unit))
        local = os.path.join(self.tmp, "f")
        write(local, b"f")
        for phase, what, call in [
                ("looking", "write", lambda: gate.write(local, "", "os.ldr")), ("looking", "remove", lambda: gate.remove_file("logs/a")),
                ("taking off", "remove content", lambda: gate.remove_file("content/a.aif")),
                ("taking off", "write early", lambda: gate.write(local, "", "os.ldr")),
                ("taking off", "remove the logs folder", lambda: gate.remove_file("logs")),
                ("taking off", "remove in a folder ending in logs", lambda: gate.remove_file("mylogs/a")),
                ("taking off", "remove in a logs folder further down", lambda: gate.remove_file("user/logs/a")),
                ("taking off", "remove further down in logs", lambda: gate.remove_file("logs/old/a")),
                ("taking off", "a pad file in user", lambda: gate.remove_file("user/open-field-pad.bin")),
                ("taking off", "a pad file in content", lambda: gate.remove_file("content/open-field-pad-2.bin"))]:
            gate.phase, gate.logs = phase, "logs"
            with self.subTest(what):
                with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                    call()
        self.assertTrue(os.path.exists(os.path.join(self.unit, "logs", "a")))
        self.assertEqual((read(os.path.join(self.unit, "user", "open-field-pad.bin")),
                          read(os.path.join(self.unit, "content", "open-field-pad-2.bin"))), (b"u", b"c"))
        gate.phase = "putting"
        gate.pad, gate.make, gate.packs = "open-field-pad-2.bin", True, {"x.pack"}
        for what, call in [("upgrade", lambda: gate.write(local, "upgrade/firmware/blackfin/bin", "os.ldr")),
                           ("another top file", lambda: gate.write(local, "", "content")),
                           ("another pad", lambda: gate.write(local, "", "open-field-pad.bin")),
                           ("another pack", lambda: gate.write(local, "user/pk", "y.pack")),
                           ("another folder", lambda: gate.make_folder("", "pk")),
                           ("USER", lambda: gate.make_folder("USER", "pk")),
                           ("PK made", lambda: gate.make_folder("user", "PK")),
                           ("into user/PK", lambda: gate.write(local, "user/PK", "x.pack")),
                           ("into User/pk", lambda: gate.write(local, "User/pk", "x.pack")),
                           ("into a Kelvin sign", lambda: gate.write(local, "user/p\u212a", "x.pack")),
                           ("pad in capitals", lambda: gate.write(local, "", "OPEN-FIELD-PAD-2.BIN")),
                           ("OS.LDR", lambda: gate.write(local, "", "OS.LDR")),
                           ("Os.ldr", lambda: gate.write(local, "", "Os.ldr")),
                           ("into USER/PK", lambda: gate.write(local, "USER/PK", "x.pack")),
                           ("remove", lambda: gate.remove_file("open-field-pad.bin"))]:
            with self.subTest(what):
                with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                    call()
        self.assertEqual(tree(self.unit)["open-field-pad.bin"], entry(b"p"))
        self.assertNotIn("OS.LDR", tree(self.unit))

    def test_the_gate_lets_files_out_of_user_pkin_and_no_more(self):
        disk = MemoryDisk({"user/pkin/.apply": b"n", "user/pkin/a.pack": b"a", "user/pkin/sub/deep": b"d", "user/pk/b.pack": b"b",
                           "user/pk/.apply": b"p", "user/pkin2/c": b"c", "user/file": b"f", "user/PKIN/e": b"e",
                           "content/pkin/g": b"g", "user/p\u212ain/h": b"h"})
        before = disk.tree()
        gate = H.Gate(disk)
        local = os.path.join(self.tmp, "f")
        write(local, b"f")
        gate.phase = "taking off"
        with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
            gate.remove_file("user/pkin/.apply")                        # not before the folder was found
        gate.waiting = "user/pkin"
        for path in ("user/pk/b.pack", "user/pk/.apply", "user/pkin2/c", "user/file", "user/pkin", "user/pkin/sub/deep",
                     "user/PKIN/e", "content/pkin/g", "user/p\u212ain/h", "user/pkin/..", "user/pkin/", "user/pk"):
            with self.subTest(path):
                with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                    gate.remove_file(path)
        for wrong in ("user/pk", "user", "", "content/pkin", "user/p\u212ain"):
            with self.subTest("waiting set to " + wrong):
                gate.waiting = wrong
                inside = next(p for p in sorted(before) if p.rpartition("/")[0] == wrong)
                with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                    gate.remove_file(inside)
        gate.waiting = "user/pkin"
        for call in (lambda: gate.write(local, "user/pkin", "x.pack"), lambda: gate.make_folder("user", "pkin")):
            for phase in ("looking", "taking off", "putting"):
                gate.phase, gate.packs, gate.make = phase, {"x.pack"}, True
                with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                    call()
        for phase in ("looking", "putting", "closed"):
            gate.phase = phase
            with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                gate.remove_file("user/pkin/.apply")
        self.assertEqual(disk.tree(), before)
        gate.phase = "taking off"
        gate.remove_file("user/pkin/.apply")
        self.assertEqual(sorted(set(before) - set(disk.tree())), ["user/pkin/.apply"])

    def test_exact_folder_counts_capitals(self):
        make_unit(self.unit, {"user/pk": None, "upgrade/x": b"x"})
        disk = H.FolderDisk(self.unit)
        self.assertTrue(H.exact_folder(disk, "user/pk"))
        self.assertTrue(H.exact_folder(disk, ""))
        for other in ("USER", "User/pk", "user/PK", "USER/PK", "upgrade/x", "Upgrade"):
            with self.subTest(other):
                self.assertFalse(H.exact_folder(disk, other))

    def test_stage_copy_makes_a_new_file_from_a_plain_file(self):
        victim = os.path.join(self.tmp, "victim")
        write(victim, b"untouched")
        plain_file = os.path.join(self.tmp, "plain")
        write(plain_file, b"plain")
        link = os.path.join(self.tmp, "link")
        os.symlink(victim, link)
        pipe = os.path.join(self.tmp, "pipe")
        pipe_made = make_pipe(pipe)
        staged = os.path.join(self.tmp, "staged")
        self.assertFalse(H.stage_copy(link, staged), "a link is not read through")
        if pipe_made:
            self.assertFalse(H.stage_copy(pipe, staged), "a pipe is not a file")
        self.assertFalse(os.path.lexists(staged))
        os.symlink(victim, staged)
        self.assertFalse(H.stage_copy(plain_file, staged), "a link in the copy's place is not written through")
        self.assertEqual(read(victim), b"untouched")
        os.remove(staged)
        self.assertTrue(H.stage_copy(plain_file, staged))
        self.assertEqual(read(staged), b"plain")
        self.assertFalse(H.stage_copy(plain_file, staged), "a file there already is not written over")

    def test_command_line(self):
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(H.main(["--wait-minutes", "soon"], echo=lambda l, c: None), 2)
        self.assertEqual(H.main(["--list-only", "--source", self.unit], echo=lambda l, c: None), 2)

    def test_a_system_that_is_neither_is_refused_before_anything_is_made(self):
        folder = os.path.join(self.tmp, "not-made")
        for system in ("win32", "cygwin", "freebsd14"):
            with self.subTest(system):
                lines = []
                code = H.main(["--folder", folder, "--source", self.unit], echo=lambda l, c: lines.append(l), system=system)
                self.assertEqual((code, lines), (2, [H.NOT_HERE]))
                self.assertIn("Linux and macOS", lines[0])
                self.assertIn("open-field-helper.ps1", lines[0])
                self.assertFalse(os.path.lexists(folder))

    def test_both_systems_are_taken(self):
        for system in ("linux", "darwin"):
            with self.subTest(system):
                self.clear_shared()
                self.new_unit()
                self.plain_unit()
                lines = []
                code = H.main(["--folder", self.shared, "--source", self.unit, "--wait-minutes", "1"],
                              echo=lambda l, c: lines.append(l), system=system)
                self.assertEqual(code, 1)
                self.assertTrue(any("no plan came" in l for l in lines), lines)

    def test_the_file_is_python_3_9s(self):
        # The python3 of Apple's command line tools is 3.9.
        ast.parse(read(HELPER).decode("utf-8"), feature_version=(3, 9))

    def test_the_old_folder_and_wording_are_gone(self):
        source = read(HELPER).decode("utf-8")
        for old in ("~/open-field", "in your home folder", "shared folder", "press 2 to close the drive", "copy the page's line again"):
            self.assertNotIn(old, source)

    def test_the_usage_names_the_helper_when_it_is_read_from_standard_input(self):
        with open(HELPER, "rb") as f:
            source = f.read()
        for argv in (["--help"], ["--wait-minutes", "soon"]):
            with self.subTest(argv):
                done = subprocess.run([sys.executable, "-I", "-"] + argv, input=source, stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE, timeout=60, cwd=self.tmp)
                self.assertEqual(done.returncode, 0 if argv == ["--help"] else 2, done.stderr)
                text = (done.stdout + done.stderr).decode("utf-8")
                self.assertIn("usage: open-field-helper.py ", text)
                self.assertNotIn("usage: - ", text)


class Edges(HelperCase):
    """Files swapped after the check, places of other letters, expected listings, links, and logs that stay."""

    def fresh(self):
        """Between two subtests: no stand-in disk, and the shared folder holds only page.json."""
        self.new_unit()
        for n in os.listdir(self.shared):
            if n != "page.json":
                q = os.path.join(self.shared, n)
                shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)

    def writes(self, disk):
        return [c for c in disk.calls if c[0] in ("write", "make_folder", "remove_file") and c[2] != "starting"]

    # ---- 1. every file to the unit is staged, re-hashed and sent from the staged copy
    def test_every_file_is_sent_from_its_staged_copy(self):
        self.plain_unit()
        disk = Recording(H.FolderDisk(self.unit), self.shared)
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, pad=b"P" * 64,
                                                                      packs=[("a.pack", b"a" * 9), ("b.pack", b"b" * 8)])), disk=disk)
        self.assertEqual(code, 0, h["lines"])
        stage = os.path.join(os.path.abspath(self.shared), "to-unit-" + h["run"]) + os.sep
        sent = [c[1][0] for c in disk.calls if c[0] == "write"]
        self.assertEqual(len(sent), 4)
        self.assertTrue(all(p.startswith(stage) for p in sent), sent)
        self.assertFalse(os.path.exists(stage))

    def swapped(self, name, swap, why, pad=None):
        """A file of the shared folder changed after the plan was checked: nothing is written."""
        self.plain_unit()
        before = tree(self.unit)

        def hook(d, folder):
            path = os.path.join(self.shared, name)
            if folder == "" and os.path.exists(os.path.join(self.shared, "plan.json")) and not os.path.islink(path):
                swap(path)
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"children": hook})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, pad=pad, packs=[("x.pack", b"x" * 40)])), disk=disk)
        self.assertStopped(code, h, why)
        self.assertEqual(self.writes(disk), [])
        self.assertEqual(tree(self.unit), before)

    def test_a_pack_swapped_for_a_link_after_the_check(self):
        secret = os.path.join(self.tmp, "secret-outside")
        write(secret, b"S" * 40)

        def to_link(path):
            os.remove(path)
            os.symlink(secret, path)
        self.swapped("packs/x.pack", to_link, "the copy of the pack file packs/x.pack on this computer is not the plan's")

    def test_a_pack_changed_after_the_check(self):
        self.swapped("packs/x.pack", lambda path: write(path, b"y" * 40),
                     "the copy of the pack file packs/x.pack on this computer is not the plan's")

    def test_a_pad_changed_after_the_check(self):
        self.swapped("open-field-pad.bin", lambda path: write(path, b"Q" * 64),
                     "the copy of the pad file open-field-pad.bin on this computer is not the plan's", pad=b"P" * 64)

    def test_a_plan_file_that_is_a_link_or_not_a_file(self):
        def link_inside(p):
            os.rename(os.path.join(self.shared, "packs", "x.pack"), os.path.join(self.shared, "real.pack"))
            os.symlink(os.path.join(self.shared, "real.pack"), os.path.join(self.shared, "packs", "x.pack"))

        def pipe(p):
            os.remove(os.path.join(self.shared, "packs", "x.pack"))
            if not make_pipe(os.path.join(self.shared, "packs", "x.pack")):
                os.mkdir(os.path.join(self.shared, "packs", "x.pack"))      # no pipes on this disk: no file either

        def folder(p):
            os.remove(os.path.join(self.shared, "packs", "x.pack"))
            os.mkdir(os.path.join(self.shared, "packs", "x.pack"))
        def folder_a_link(p):
            os.rename(os.path.join(self.shared, "packs"), os.path.join(self.tmp, "packs-outside"))
            os.symlink(os.path.join(self.tmp, "packs-outside"), os.path.join(self.shared, "packs"))
        for what, mutate, why in [
                ("its folder a link out", folder_a_link, "the plan names a file that is not a plain name inside this folder: packs/x.pack"),
                ("a link inside the folder", link_inside, "the plan names a file that is not a plain name inside this folder: packs/x.pack"),
                ("a pipe", pipe, "the pack file packs/x.pack is not in the folder as the plan says"),
                ("a folder", folder, "the pack file packs/x.pack is not in the folder as the plan says")]:
            with self.subTest(what):
                self.fresh()
                shutil.rmtree(os.path.join(self.tmp, "packs-outside"), ignore_errors=True)
                self.refused(mutate, why)

    # ---- 2. a place written is named exactly
    def test_no_pack_into_a_folder_of_other_letters(self):
        def no_make(p):
            p["makeFolder"] = None
            p["expected"] = [r for r in p["expected"] if r["path"] != "user/pk"]
        for other in ("PK", "pK", "pK"):          # the last with a Kelvin sign, which is "k" in small letters
            with self.subTest(other):
                self.fresh()
                h = self.refused(no_make, "the unit has no folder user/pk", more={"user/" + other + "/keep": b"k"})
                self.assertNotIn("user/pk", tree(self.unit))

    def test_a_folder_of_other_letters_and_a_plan_that_makes_none(self):
        # the expected listing still has the row of user/pk
        self.refused(lambda p: p.update(makeFolder=None), "the unit has no folder user/pk", more={"user/PK/keep": b"k"})

    def test_the_folder_made_beside_one_of_other_letters(self):
        unit = MemoryDisk(dict(PLAIN, **{"user/PK/keep": b"k"}))
        before = unit.tree()
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=[("x.pack", b"x" * 40)])), disk=unit)
        self.assertEqual(code, 0, h["lines"])
        self.assertEqual(unit.tree(), dict(before, **{"os.ldr": entry(IMAGE), "user/pk": ("D",), "user/pk/x.pack": entry(b"x" * 40)}))

    def test_a_plan_that_makes_a_folder_the_unit_has(self):
        def make_again(p):
            p["makeFolder"] = "user/pk"
        self.refused(make_again, "the plan makes the folder user/pk, and the unit has it already", more={"user/pk": None})

    # ---- 3. the plan's expected listing, before any write
    def test_an_expected_listing_of_no_rows(self):
        not_rows = "the plan's expected listing is not a list of rows"
        for what, value in [("none", None), ("a number", 5), ("a text", "rows"), ("a row that is a number", [1]),
                            ("a row with no path", [{"size": 1, "dir": False}]), ("a dict", {"path": "os.ldr"})]:
            with self.subTest(what):
                self.fresh()
                self.refused(lambda p: p.update(expected=value), not_rows)

    def test_an_expected_listing_the_writes_do_not_make(self):
        wrong = "the plan's expected listing is not what its own writes make of the disk"

        def drop(path):
            return lambda p: p.update(expected=[r for r in p["expected"] if r["path"] != path])

        def size(path, n):
            return lambda p: p.update(expected=[dict(r, size=n) if r["path"] == path else r for r in p["expected"]])
        for what, mutate, line in [
                ("a row of the disk left out", drop("content/a.aif"), "  made by the plan's writes and not planned: F content/a.aif 100"),
                ("os.ldr left out", drop("os.ldr"), "  made by the plan's writes and not planned: F os.ldr 3000"),
                ("a row more", lambda p: p["expected"].append({"path": "content/z.aif", "size": 1, "dir": False}),
                 "  planned and not made by the plan's writes: F content/z.aif 1"),
                ("os.ldr of another size", size("os.ldr", 3001), "  planned and not made by the plan's writes: F os.ldr 3001"),
                ("a pack of another size", size("user/pk/x.pack", 41), "  planned and not made by the plan's writes: F user/pk/x.pack 41"),
                ("the pad left out", drop("open-field-pad.bin"), "  made by the plan's writes and not planned: F open-field-pad.bin 64")]:
            with self.subTest(what):
                self.fresh()
                h = self.refused(mutate, wrong, pad=b"P" * 64)
                self.assertIn(line, h["lines"])
                self.assertFalse(os.path.exists(os.path.join(self.shared, "listing-after.json")))

    # ---- 4. a pad file's name, with nothing after it
    def test_a_name_with_a_newline_after_a_pad_name_is_left(self):
        self.assertIsNone(H.PAD.match("open-field-pad.bin\n"))
        self.assertIsNone(H.PAD.match("open-field-pad-2.bin\n"))
        self.plain_unit({"open-field-pad.bin\n": b"the owner's own file"})
        before = tree(self.unit)
        code, h, _ = self.go(wait="1")
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        self.assertEqual(h["oldPads"]["found"], 0)
        self.assertEqual(tree(self.unit), before)

    # ---- 5. a link never written through
    def test_a_part_file_that_is_a_link(self):
        self.plain_unit()
        victim = os.path.join(self.tmp, "victim.txt")
        write(victim, b"untouched")
        os.symlink(victim, os.path.join(self.shared, "helper.json.part"))
        code, h, _ = self.go(wait="1")
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        self.assertEqual(read(victim), b"untouched")
        self.assertFalse(os.path.lexists(os.path.join(self.shared, "helper.json.part")))

    def test_save_text_makes_a_new_file(self):
        target = os.path.join(self.tmp, "t.json")
        victim = os.path.join(self.tmp, "victim.txt")
        write(victim, b"untouched")
        os.symlink(victim, target + ".part")
        H.save_text(target, "new")
        self.assertEqual((read(target), read(victim), os.path.islink(target)), (b"new", b"untouched", False))

    # ---- 6. a log file that cannot be removed stays on the unit, and the run goes on
    def test_a_log_that_cannot_be_removed(self):
        self.plain_unit({"logs/a.txt": b"log a", "logs/b.txt": b"log b"})

        def refuse(d, path):
            if path == "logs/a.txt":
                raise PermissionError(13, "made-up refusal")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"remove_file": refuse})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertEqual(code, 0, h["lines"])
        self.assertEqual(h["logs"]["left_on_unit"], 1)
        self.assertIn("1 log file(s) stay on the unit (2 copied). That is no fault: the plan counts the disk as it is.", h["lines"])
        self.assertEqual(read(os.path.join(self.unit, "logs", "a.txt")), b"log a")
        self.assertFalse(os.path.exists(os.path.join(self.unit, "logs", "b.txt")))

    # ---- 7. a short copy, the top logs folder only, and the gate while the disk is listed again
    def test_a_log_whose_copy_is_short_stays(self):
        self.plain_unit({"logs/a.txt": b"log a", "logs/b.txt": b"log b"})
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"read": lambda d, path, local: "short" if path == "logs/a.txt" else None})
        code, h, _ = self.go(wait="1", disk=disk)
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        self.assertEqual(h["logs"], {"folder": "logs-" + h["run"], "copied": 1, "left_on_unit": 1})
        self.assertEqual(read(os.path.join(self.unit, "logs", "a.txt")), b"log a")
        self.assertEqual(os.listdir(os.path.join(self.shared, "logs-" + h["run"])), ["b.txt"])
        self.assertNotIn("logs/a.txt", [c[1][0] for c in disk.calls if c[0] == "remove_file"])

    def test_only_the_top_logs_folder_is_emptied(self):
        self.plain_unit({"logs/a.txt": b"log", "mylogs/b.txt": b"b", "user/logs/c.txt": b"c", "oldlogs/d.txt": b"d"})
        before = tree(self.unit)
        code, h, _ = self.go(wait="1")
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        want = dict(before)
        del want["logs/a.txt"]
        self.assertEqual(tree(self.unit), want)

    def test_the_gate_is_shut_while_the_disk_is_listed_again(self):
        seen, Gate = [], H.Gate

        class Watched(Gate):
            def children(self, folder_path):
                h = helper_json(shared)
                seen.append((h and h["state"], self.phase, self.make, frozenset(self.packs), self.pad))
                return Gate.children(self, folder_path)
        shared = self.shared
        H.Gate = Watched
        self.plain_unit()
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, pad=b"P" * 64, packs=[("x.pack", b"x" * 40)])))
        self.assertEqual(code, 0, h["lines"])
        second = [s for s in seen if s[0] == "listed"]
        self.assertTrue(second)
        self.assertEqual(set(s[1:] for s in second), {("looking", False, frozenset(), None)})
        self.assertIn("putting", {s[1] for s in seen})


class Guard(unittest.TestCase):
    def test_the_system_libusb_is_never_reached(self):
        self.assertIs(H.Libusb, NoRealLibusb)
        with self.assertRaises(AssertionError):
            H.Libusb()


RUN = "2001-02-03_04-05-06"     # a run id fixed by a test, so that a folder of the run can be planted before it


class Clock:
    """The helper's time module, with the run's id fixed to RUN."""

    def strftime(self, form):
        return RUN if form == "%Y-%m-%d_%H-%M-%S" else time.strftime(form)

    def __getattr__(self, name):
        return getattr(time, name)


class LocalFolders(HelperCase):
    """Every folder of a run is made new, and every file in it: nothing planted is written through."""

    def setUp(self):
        super().setUp()
        self.outside = os.path.join(self.tmp, "outside")
        make_unit(self.outside, {"os.ldr": b"the owner's", "a.txt": b"the owner's", "open-field-pad.bin": b"the owner's",
                                 "x.pack": b"the owner's"})
        self.victim = os.path.join(self.outside, "os.ldr")
        self.kept = tree(self.outside)

    def tearDown(self):
        self.assertEqual(tree(self.outside), self.kept, "nothing outside the shared folder is written")
        super().tearDown()

    def planted_after_mkdir(self, prefix, name, target):
        """os.mkdir, which plants a link `name` -> target in the folder of the run whose name starts with prefix,
        right after the helper made it: a race won by another program."""
        real = os.mkdir

        def mkdir(path, *args, **kwargs):
            real(path, *args, **kwargs)
            if os.path.basename(path).startswith(prefix):
                os.symlink(target, os.path.join(path, name))
        return mock.patch.object(os, "mkdir", mkdir)

    def already(self, name):
        return ("the folder " + name + " is there already on this computer, and the helper writes only into a folder "
                "it made itself. Remove it and run the command again")

    def test_a_folder_taking_off_planted_before_the_run(self):
        for prefix in ("incoming-", "logs-", "old-pads-", "old-pkin-"):
            for kind in ("a link", "a folder with a link in it"):
                with self.subTest(prefix + " " + kind):
                    self.new_unit()
                    for n in os.listdir(self.shared):
                        if n != "page.json":
                            q = os.path.join(self.shared, n)
                            shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                    self.plain_unit({"logs/a.txt": b"log", "open-field-pad.bin": b"p", "user/pkin/.apply": b"note"})
                    before = tree(self.unit)
                    planted = os.path.join(self.shared, prefix + RUN)
                    if kind == "a link":
                        os.symlink(self.outside, planted)
                    else:
                        os.mkdir(planted)
                        for leaf in ("os.ldr", "a.txt", "open-field-pad.bin"):
                            os.symlink(os.path.join(self.outside, leaf), os.path.join(planted, leaf))
                    with mock.patch.object(H, "time", Clock()):
                        code, h, _ = self.go(wait="1")
                    self.assertStopped(code, h, self.already(prefix + RUN))
                    if prefix in ("old-pads-", "old-pkin-"):    # the logs first, then the old pads, then user/pkin
                        del before["logs/a.txt"]
                    if prefix == "old-pkin-":
                        del before["open-field-pad.bin"]
                    self.assertEqual(tree(self.unit), before, "nothing more is taken off the unit")

    def test_a_file_planted_in_a_folder_taking_off_right_after_it_is_made(self):
        for prefix, leaf in (("incoming-", "os.ldr"), ("logs-", "a.txt"), ("old-pads-", "open-field-pad.bin"), ("old-pkin-", "1")):
            with self.subTest(prefix):
                self.new_unit()
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        q = os.path.join(self.shared, n)
                        shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                self.plain_unit({"logs/a.txt": b"log", "open-field-pad.bin": b"p", "user/pkin/.apply": b"note"})
                with self.planted_after_mkdir(prefix, leaf, os.path.join(self.outside, leaf)):
                    code, h, _ = self.go(wait="1")
                self.assertEqual(code, 1)
                planted = os.path.join(self.shared, prefix + h["run"], leaf)
                self.assertTrue(os.path.islink(planted), "what the helper did not make is never removed")
                if prefix == "incoming-":
                    self.assertTrue(h["message"].startswith("the firmware file did not come off the unit: the copy of "
                                                            "upgrade/firmware/blackfin/bin/os.ldr could not be made new"), h["message"])
                elif prefix == "logs-":
                    self.assertEqual(h["logs"], {"folder": "logs-" + h["run"], "copied": 1, "left_on_unit": 0})
                    self.assertEqual(read(os.path.join(self.shared, "logs-" + h["run"], "a-2.txt")), b"log")
                elif prefix == "old-pkin-":
                    self.assertEqual(h["waiting"], {"found": 1, "taken_off": 1, "left_on_unit": 0})
                    self.assertFalse(os.path.exists(os.path.join(self.unit, "user", "pkin", ".apply")))
                else:
                    self.assertEqual(h["oldPads"]["left_on_unit"], 0)
                    self.assertFalse(os.path.exists(os.path.join(self.unit, "open-field-pad.bin")))
                    self.assertEqual(os.listdir(os.path.dirname(planted)), [leaf])

    def test_the_staging_folder_planted(self):
        def as_link(h):
            os.symlink(self.outside, os.path.join(self.shared, "to-unit-" + h["run"]))

        def packs_inside(h):
            stage = os.path.join(self.shared, "to-unit-" + h["run"])
            os.mkdir(stage)
            os.symlink(self.outside, os.path.join(stage, "packs"))
        for what, plant in (("a link", as_link), ("its packs a link", packs_inside)):
            with self.subTest(what):
                self.new_unit()
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        q = os.path.join(self.shared, n)
                        shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                self.plain_unit()
                before = tree(self.unit)

                def page(h):
                    plant(h)
                    put_plan(self.shared, page_plan(self.shared, pad=b"P" * 64, packs=[("x.pack", b"x" * 40)]))
                code, h, _ = self.go(page)
                self.assertStopped(code, h, self.already("to-unit-" + h["run"]))
                self.assertEqual(tree(self.unit), before, "nothing is written on the unit")

    def test_the_packs_folder_planted_right_after_the_staging_folder_is_made(self):
        self.plain_unit()
        before = tree(self.unit)
        with self.planted_after_mkdir("to-unit-", "packs", self.outside):
            code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=[("x.pack", b"x" * 40)])))
        self.assertStopped(code, h, self.already("packs"))
        self.assertEqual(tree(self.unit), before, "nothing is written on the unit")

    def test_the_read_back_folder_planted(self):
        def as_link(h):
            os.symlink(self.outside, os.path.join(self.shared, "read-back-" + h["run"]))

        def link_inside(h):
            back = os.path.join(self.shared, "read-back-" + h["run"])
            os.mkdir(back)
            os.symlink(self.victim, os.path.join(back, "os.ldr"))
        for what, plant in (("a link", as_link), ("os.ldr in it a link", link_inside)):
            with self.subTest(what):
                self.new_unit()
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        q = os.path.join(self.shared, n)
                        shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                self.plain_unit()

                def page(h):
                    plant(h)
                    put_plan(self.shared, page_plan(self.shared))
                code, h, _ = self.go(page)
                self.assertStopped(code, h, self.already("read-back-" + h["run"]))

    def test_a_file_planted_in_the_read_back_folder_right_after_it_is_made(self):
        self.plain_unit()
        with self.planted_after_mkdir("read-back-", "os.ldr", self.victim):
            code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)))
        self.assertEqual(code, 1)
        self.assertTrue(h["message"].startswith("the copy of os.ldr could not be made new on this computer"), h["message"])

    def test_the_log_file_planted_before_the_run(self):
        self.plain_unit()
        before = tree(self.unit)
        log = os.path.join(self.shared, "helper-" + RUN + ".log")
        os.symlink(self.victim, log)
        with mock.patch.object(H, "time", Clock()):
            code, h, out = self.go(wait="1")
        self.assertEqual(code, 1)
        self.assertEqual(h["state"], "stopped")
        self.assertTrue(h["message"].startswith("the helper's log file helper-" + RUN + ".log could not be made new"), h["message"])
        self.assertEqual(out, ["STOPPED: " + h["message"]])
        self.assertTrue(os.path.islink(log))
        self.assertEqual(tree(self.unit), before)

    def test_the_log_file_swapped_for_a_link_during_the_run(self):
        self.plain_unit()
        before = tree(self.unit)

        def page(h):
            log = os.path.join(self.shared, "helper-" + h["run"] + ".log")
            os.remove(log)
            os.symlink(self.victim, log)
            put_plan(self.shared, page_plan(self.shared))
        code, h, out = self.go(page)
        self.assertEqual((code, h["state"]), (1, "stopped"))
        self.assertTrue(h["message"].startswith("the helper's log file helper-" + h["run"] + ".log cannot be written"), h["message"])
        self.assertEqual(tree(self.unit), before, "nothing is written on the unit")


class BootloaderDrive(HelperCase):
    """Only the bootloader's raw disk is worked on: its top holds a folder user or upgrade, by that exact name."""

    def test_a_drive_of_the_normal_disk_mode_stops_before_anything_is_read_or_changed(self):
        normal = {"synth/a.aif": b"s" * 10, "drum/b.aif": b"d" * 10, "tapes/track_1.aif": b"t" * 10, "mixdown": None,
                  "logs/a.txt": b"log", "open-field-pad.bin": b"p", "os.ldr": b"o"}
        for what, more in (("normal mode", {}), ("in capitals", {"USER": None, "Upgrade": None}),
                           ("files of those names", {"user": b"u", "upgrade": b"g"}),
                           ("further down", {"synth/user": None, "drum/upgrade/firmware/blackfin/bin/os.ldr": STOCK})):
            with self.subTest(what):
                self.new_unit()
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        q = os.path.join(self.shared, n)
                        shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                make_unit(self.unit, dict(normal, **more))
                before = tree(self.unit)
                disk = Recording(H.FolderDisk(self.unit), self.shared)
                code, h, _ = self.go(wait="1", disk=disk)
                self.assertStopped(code, h, H.NOT_BOOTLOADER)
                self.assertEqual([c[0] for c in disk.calls], ["children", "close"])
                self.assertEqual((h["stock"], h["logs"], h["oldPads"], h["listing"]), (None, None, None, None))
                self.assertEqual(tree(self.unit), before)
                self.assertEqual(sorted(os.listdir(self.shared)), sorted(["page.json", "helper.json", "helper-" + h["run"] + ".log"]))

    def test_the_text(self):
        self.assertEqual(H.NOT_BOOTLOADER, "the drive that showed is not the bootloader's drive: it has no folder user or "
                         "upgrade at its top. If the unit is in its normal disk mode, switch it off, hold COM while switching "
                         "it on, press 3, then 2; then run the command again")

    def test_one_of_the_two_folders_is_enough(self):
        for top in ("user", "upgrade"):
            with self.subTest(top):
                self.new_unit()
                self.clear_shared()
                make_unit(self.unit, {top: None, "content/a.aif": b"a"})
                code, h, _ = self.go(wait="1")
                self.assertStopped(code, h, "no plan came from the page in 1 minutes")


class CopyOn(HelperCase):
    def test_a_file_that_shows_with_another_size_stops_the_run(self):
        self.plain_unit()

        def short(d, local, folder, name):
            write(os.path.join(self.unit, *(folder.split("/") if folder else []), name), read(local)[:-1])
            return "skip"
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"write": short})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertEqual(code, 1)
        self.assertRegex(h["message"], r"^the unit did not show os\.ldr with its 3000 bytes in \d+ seconds$")
        self.assertEqual(h["checks"], [])


class WaitingFolder(HelperCase):
    """user/pkin emptied before the listing, as the Windows helper's 3c does."""

    def test_every_file_goes_and_the_folder_stays(self):
        unit = MemoryDisk(dict(PLAIN, **{
            "user/pkin/.apply": b"", "user/pkin/A.pack": b"one", "user/pkin/a.pack": b"two", "user/pkin/any thing.bin": b"3",
            "user/pkin/café.pack": b"four", "user/pkin/sub/kept": b"k", "user/pk/a.pack": b"kept", "user/pk/.apply": b"kept"}))
        before = unit.tree()
        disk = Recording(unit, self.shared)
        code, h, _ = self.go(wait="1", disk=disk)
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
        self.assertEqual(h["waiting"], {"found": 5, "taken_off": 5, "left_on_unit": 0})
        self.assertIn("The 5 file(s) waiting in user/pkin are taken off the unit.", h["lines"])
        want = {p: v for p, v in before.items() if p.rpartition("/")[0] != "user/pkin" or v == ("D",)}
        self.assertEqual(unit.tree(), want)
        self.assertEqual(unit.tree()["user/pkin"], ("D",))
        rows = [r["path"] for r in json.loads(read(os.path.join(self.shared, "listing-before.json")))]
        self.assertEqual(sorted(rows), sorted(want), "the plan counts the disk without them")
        self.assertFalse([n for n in os.listdir(self.shared) if n.startswith("old-pkin-")])
        self.assertTrue(all(c[2] == "starting" for c in disk.calls if c[0] == "remove_file"))

    def test_the_folder_in_other_letters_and_the_exact_one_first(self):
        unit = MemoryDisk(dict(PLAIN, **{"user/PKIN/x": b"x", "user/pk/y": b"y"}))
        code, h, _ = self.go(wait="1", disk=unit)
        self.assertEqual(h["waiting"]["taken_off"], 1)
        self.assertNotIn("user/PKIN/x", unit.at)
        self.assertIn("user/pk/y", unit.at)
        self.clear_shared()
        unit = MemoryDisk(dict(PLAIN, **{"user/PKIN/x": b"x", "user/pkin/.apply": b"n"}))
        code, h, _ = self.go(wait="1", disk=unit)
        self.assertEqual(h["waiting"]["taken_off"], 1)
        self.assertIn("user/PKIN/x", unit.at)
        self.assertNotIn("user/pkin/.apply", unit.at)

    def test_nothing_waits(self):
        for what, more in (("no folder", {}), ("empty", {"user/pkin": None}), ("a folder alone", {"user/pkin/sub": None}),
                           ("a file of that name", {"user/pkin": b"file"}), ("a Kelvin sign", {"user/pKin/x": b"x"})):
            with self.subTest(what):
                self.clear_shared()
                unit = MemoryDisk(dict(PLAIN, **more))
                before = unit.tree()
                code, h, _ = self.go(wait="1", disk=unit)
                self.assertStopped(code, h, "no plan came from the page in 1 minutes")
                self.assertEqual(h["waiting"], {"found": 0, "taken_off": 0, "left_on_unit": 0})
                self.assertFalse([l for l in h["lines"] if "pkin" in l])
                self.assertEqual(unit.tree(), before)

    def test_a_file_left_means_stop(self):
        def refuse(d, path):
            if path.endswith("/.apply"):
                raise H.DiskError("made-up refusal")

        def short(d, path, local):
            if path.endswith("/.apply"):
                raise H.DiskError("made-up failure")
        for what, hooks in (("not removed", {"remove_file": refuse}), ("not copied", {"read": short})):
            with self.subTest(what):
                self.clear_shared()
                unit = MemoryDisk(dict(PLAIN, **{"user/pkin/.apply": b"note", "user/pkin/a.pack": b"a"}))
                code, h, _ = self.go(disk=Recording(unit, self.shared, hooks))
                self.assertStopped(code, h, "1 file(s) stay in user/pkin on the unit and would meet the new base.pack. "
                                            "Delete them in the unit's drive, then run the command again. The cause: "
                                            "user/pkin/.apply: " + ("made-up refusal" if what == "not removed" else "made-up failure"))
                self.assertEqual(h["waiting"], {"found": 2, "taken_off": 1, "left_on_unit": 1})
                self.assertEqual(h["listing"], None)
                self.assertIn("user/pkin/.apply", unit.at)
                self.assertNotIn("user/pkin/a.pack", unit.at)
                self.assertFalse([n for n in os.listdir(self.shared) if n.startswith(("old-pkin-", "listing-"))])

    def test_the_state_has_the_fields_in_the_windows_helpers_order(self):
        self.plain_unit()
        code, h, _ = self.go(wait="1")
        self.assertEqual(list(h), ["helper", "run", "page", "state", "time", "folder", "unit", "stock", "logs", "oldPads",
                                   "waiting", "listing", "checks", "lines", "message"])


class StateInOneStep(HelperCase):
    def test_a_refused_swap_is_tried_again_and_the_old_file_stays_until_then(self):
        target = os.path.join(self.tmp, "helper.json")
        write(target, b"old")
        real, seen = os.replace, []

        def busy(a, b):
            seen.append(read(target))
            if len(seen) < 4:
                raise PermissionError(13, "made up: the page reads it")
            real(a, b)
        with mock.patch.object(H.os, "replace", busy):
            H.save_text(target, "new")
        self.assertEqual((seen, read(target)), ([b"old"] * 4, b"new"))
        self.assertFalse(os.path.exists(target + ".part"))

    def test_a_swap_refused_for_good_is_an_error_and_the_old_file_stays(self):
        target = os.path.join(self.tmp, "helper.json")
        write(target, b"old")
        with mock.patch.object(H.os, "replace", side_effect=PermissionError(13, "made up")) as swap:
            with self.assertRaises(PermissionError):
                H.save_text(target, "new")
        self.assertEqual((swap.call_count, read(target)), (20, b"old"))


class WorkingFolder(HelperCase):
    """The folder looked for by its name and the page's mark, or asked for once."""

    def setUp(self):
        super().setUp()
        self.here = os.path.join(self.tmp, "here")
        for p in [self.here] + [os.path.join(self.home, n) for n in ("Desktop", "Documents", "Downloads")]:
            os.makedirs(p)
        os.chdir(self.here)

    def folder(self, *parts, mark=MARK):
        path = os.path.join(self.tmp, *parts)
        os.makedirs(path, exist_ok=True)
        if mark is not None:
            write(os.path.join(path, "page.json"), json.dumps({"id": mark}).encode())
        return path

    def find(self, name="Work", mark=MARK, linux=True):
        return sorted(H.find_folders(name, mark, linux))

    def start(self, args=(), answers=(), system=None):
        """The command line with no --folder: (code, the lines, the questions asked). Helper.run is not run: the
        folder it would get is the code."""
        out, asked, left = [], [], list(answers)

        def ask(question):
            asked.append(question)
            return left.pop(0) if left else None
        with mock.patch.object(H.Helper, "run", lambda helper: helper.folder):
            code = H.main(list(args), echo=lambda l, c: out.append(l), ask=ask, system=system)
        return code, out, asked

    def test_each_place_and_one_level_below(self):
        for parts in (("here", "Work"), ("home", "Work"), ("home", "Desktop", "Work"), ("home", "Documents", "Work"),
                      ("home", "Downloads", "Work")):
            with self.subTest(parts):
                path = self.folder(*parts)
                self.assertEqual(self.find(), [path])
                shutil.rmtree(path)

    def test_the_places_themselves(self):
        for parts in (("here",), ("home",), ("home", "Desktop"), ("home", "Documents"), ("home", "Downloads")):
            with self.subTest(parts):
                path = self.folder(*parts)
                self.assertEqual(self.find(parts[-1]), [path])
                os.remove(os.path.join(path, "page.json"))

    def test_not_two_levels_below_and_not_elsewhere(self):
        self.folder("here", "sub", "Work")
        self.folder("home", "Desktop", "sub", "Work")
        self.folder("elsewhere", "Work")
        self.assertEqual(self.find(), [])

    def test_the_name_and_the_mark_both_count(self):
        self.folder("home", "Work", mark="ffffffff")
        self.folder("home", "Desktop", "Work", mark=None)
        self.folder("home", "Documents", "Other")
        write(os.path.join(self.home, "Downloads", "Work"), b"a file, no folder")
        broken = self.folder("here", "Work", mark=None)
        write(os.path.join(broken, "page.json"), b"{not json")
        self.assertEqual(self.find(), [])
        self.assertEqual(self.find(name=""), [os.path.join(self.home, "Documents", "Other")], "by the mark alone")
        self.assertEqual(len(self.find(mark="")), 3, "by the name alone")

    def test_a_pipe_named_page_json_is_not_waited_on(self):
        path = self.folder("home", "Work", mark=None)
        if make_pipe(os.path.join(path, "page.json")):
            self.assertEqual(self.find(), [])
        self.assertIsNone(H.read_mark(path))

    def test_letters_case_and_decomposed_accents(self):
        composed = "Café Årger"
        decomposed = unicodedata.normalize("NFD", composed)
        self.assertNotEqual(composed, decomposed)
        path = self.folder("home", decomposed)
        for name in (composed, decomposed, composed.upper(), composed.lower()):
            with self.subTest(name):
                found = self.find(name, MARK.upper())
                self.assertEqual(len(found), 1)
                self.assertTrue(os.path.samefile(found[0], path))

    def test_one_folder_reached_twice_is_one(self):
        path = self.folder("here", "Work")
        os.symlink(path, os.path.join(self.home, "Desktop", "Work"))
        os.chdir(path)                              # the current folder, and one below nothing
        os.symlink(path, os.path.join(self.home, "Work"))
        self.assertEqual(self.find(), [path])

    def test_the_home_folder_as_the_current_one(self):
        path = self.folder("home", "Work")
        os.chdir(self.home)
        self.assertEqual(self.find(), [path])

    def test_linuxs_own_names_of_desktop_documents_downloads(self):
        bureau = self.folder("home", "Bureau \"deux\"", "Work")
        papers = self.folder("papers", "Work")
        write(os.path.join(self.home, ".config", "user-dirs.dirs"), (
            '# a comment\nXDG_DESKTOP_DIR="$HOME/Bureau \\"deux\\""\nXDG_DOCUMENTS_DIR="%s"\n'
            'XDG_DOWNLOAD_DIR="not/absolute"\nXDG_MUSIC_DIR="$HOME/Music"\n' % os.path.dirname(papers)).encode())
        self.folder("home", "Music", "Work")
        self.assertEqual(self.find(linux=True), sorted([bureau, papers]))
        self.assertEqual(self.find(linux=False), [], "a Mac has no user-dirs.dirs")
        moved = os.path.join(self.tmp, "config")
        os.makedirs(moved)
        with mock.patch.dict(os.environ, {"XDG_CONFIG_HOME": moved}):
            self.assertEqual(self.find(linux=True), [], "another place for the settings")

    def test_a_folder_that_cannot_be_read_is_passed_over(self):
        path = self.folder("home", "Downloads", "Work")
        self.folder("home", "Desktop", "Work")
        real, desktop = os.scandir, os.path.join(self.home, "Desktop")

        def scandir(where):
            if str(where) == desktop:
                raise PermissionError(1, "Operation not permitted")      # as a Mac's privacy setting refuses
            return real(where)
        with mock.patch.object(H.os, "scandir", scandir):
            self.assertEqual(self.find(), [path])

    def test_a_current_folder_that_is_gone(self):
        path = self.folder("home", "Work")
        with mock.patch.object(H.os, "getcwd", side_effect=FileNotFoundError(2, "gone")):
            self.assertEqual(self.find(), [path])

    def test_one_hit_is_taken_and_nothing_is_asked(self):
        path = self.folder("home", "Desktop", "Work")
        for system in ("linux", "darwin"):
            code, out, asked = self.start(["--page", MARK, "--folder-name", "work"], system=system)
            self.assertEqual((code, out, asked), (path, [], []))

    def test_name_and_mark_from_the_environment(self):
        path = self.folder("here", "Work")
        with mock.patch.dict(os.environ, {"OPEN_FIELD_PAGE": MARK, "OPEN_FIELD_FOLDER": "Work"}):
            self.assertEqual(self.start(), (path, [], []))
            code, out, asked = self.start(["--folder-name", "Other"])      # the command line's counts first
            self.assertEqual(code, 2)
            self.assertIn('"Other"', asked[0])

    def test_a_folder_named_outright_is_not_looked_for(self):
        self.folder("home", "Work")
        other = self.folder("elsewhere", "Any")
        self.assertEqual(self.start(["--page", MARK, "--folder-name", "Work", "--folder", other]), (other, [], []))

    def test_no_default_folder(self):
        self.folder("home", "open-field")
        code, out, asked = self.start([])
        self.assertEqual((code, len(asked)), (2, 1))
        self.assertEqual(out[0], "STOPPED: Could not find the working folder by itself. Nothing was done.")
        shutil.rmtree(os.path.join(self.home, "open-field"))
        self.assertEqual(self.start(["--no-ask"])[0], 2)
        self.assertFalse(os.path.exists(os.path.join(self.home, "open-field")))

    def test_none_and_several_are_asked_once(self):
        code, out, asked = self.start(["--page", MARK, "--folder-name", "Work"], answers=["\n"])
        self.assertEqual((code, out, len(asked)), (2, ["STOPPED: no folder was given. Nothing was done."], 1))
        self.assertTrue(asked[0].startswith('Could not find the working folder "Work" by itself.'))
        first, second = self.folder("home", "Work"), self.folder("home", "Desktop", "Work")
        code, out, asked = self.start(["--page", MARK, "--folder-name", "Work"], answers=[second + "\n"])
        self.assertEqual((code, out, len(asked)), (second, [], 1))
        self.assertTrue(asked[0].startswith('Found 2 folders that could be the working folder "Work".'))

    def test_no_ask_and_no_terminal_stop_with_nothing_done(self):
        self.folder("home", "Work")
        self.folder("home", "Desktop", "Work")
        before = tree(self.tmp)
        for args, asks in ((["--no-ask"], 0), ([], 1)):
            with self.subTest(args):
                code, out, asked = self.start(["--page", MARK, "--folder-name", "Work"] + args)
                self.assertEqual((code, len(asked)), (2, asks))
                self.assertEqual(out, ['STOPPED: Found 2 folders that could be the working folder "Work". Nothing was done.',
                                       H.RUN_WITH_FOLDER])
                self.assertIn("--folder", out[1])
                self.assertEqual(tree(self.tmp), before)
        with mock.patch.object(H, "TTY", os.path.join(self.tmp, "no", "tty")):
            self.assertIsNone(H.ask_terminal("a question"))

    def test_what_dragging_a_folder_into_a_terminal_pastes(self):
        path = self.folder("away", "My Folder's (new) #1 & café")
        escaped = "".join("\\" + c if c in " '()#&" else c for c in path)
        for what, typed in (("plain", path), ("plain, a space after", path + " "), ("backslashes", escaped),
                            ("backslashes, a space after", escaped + " "), ("quotes", shlex.quote(path)),
                            ("quotes, a space after", shlex.quote(path) + " "), ("double quotes", '"' + path + '"')):
            for end in ("\n", "\r\n", ""):
                with self.subTest(what + repr(end)):
                    self.assertEqual(self.start(["--page", MARK], answers=[typed + end]), (path, [], mock.ANY))
        ends_in_space = self.folder("away", "ends ")
        self.assertEqual(H.typed_path(ends_in_space.replace(" ", "\\ ") + " \n"), ends_in_space)
        self.assertEqual(H.typed_path("~/Desktop\n"), os.path.join(self.home, "Desktop"))
        self.assertEqual(H.typed_path("it's here\n"), "it's here")

    def test_the_folder_given_is_held_to_the_mark(self):
        path = self.folder("away", "Work", mark="ffffffff")
        code, out, asked = self.start(["--page", MARK, "--folder-name", "Work"], answers=[path + "\n"])
        self.assertEqual((code, out), (2, [H.NOT_CHOSEN[0] + path] + list(H.NOT_CHOSEN[1:])))
        self.assertEqual(os.listdir(path), ["page.json"])

    def test_a_folder_given_that_is_not_there_is_not_made(self):
        path = os.path.join(self.tmp, "nowhere")
        for args in ([], ["--page", MARK]):
            code, out, asked = self.start(args, answers=[path + "\n"])
            self.assertEqual((code, out), (2, ["STOPPED: there is no such folder: " + path + ". Nothing was done."]))
            self.assertFalse(os.path.exists(path))

    def test_a_folder_given_that_may_not_be_read_says_so(self):
        path = self.folder("away", "Work")
        with mock.patch.object(H, "read_mark", lambda folder: None):
            with mock.patch.object(H.os, "scandir", side_effect=PermissionError(1, "Operation not permitted")):
                code, out, asked = self.start(["--page", MARK, "--folder", path])
        self.assertEqual((code, out), (2, [H.MAY_NOT_READ[0] + path] + list(H.MAY_NOT_READ[1:])))
        self.assertIn("Privacy & Security", out[1])

    def test_a_folder_that_cannot_be_made_is_no_traceback(self):
        write(os.path.join(self.tmp, "a file"), b"x")
        out = []
        code = H.main(["--folder", os.path.join(self.tmp, "a file", "under")], echo=lambda l, c: out.append(l))
        self.assertEqual((code, len(out)), (1, 1))
        self.assertIn("cannot be used", out[0])

    def test_a_whole_run_in_the_folder_found(self):
        self.shared = self.folder("home", "Documents", "Samples and such")
        self.plain_unit()
        code, h, out = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), named=False,
                               args=["--folder-name", "samples AND such"])
        self.assertEqual((code, h["state"], h["folder"]), (0, "ready", self.shared), out)

    def piped(self, args, terminal=None):
        """The helper as the page's line starts it: the file through standard input, in a session of its own, and so
        with no terminal but the one given. (code, what it printed)."""
        command = [sys.executable, "-I", "-"] + args
        if terminal is not None:
            take = ("import fcntl,os,sys,termios\n"
                    "fd=os.open(sys.argv[1],os.O_RDWR)\n"
                    "try:\n fcntl.ioctl(fd,termios.TIOCSCTTY,0)\n"
                    "except OSError:\n pass\n"
                    "os.execv(sys.executable,[sys.executable,'-I','-']+sys.argv[2:])\n")
            command = [sys.executable, "-I", "-c", take, os.ttyname(terminal[1])] + args
        with open(HELPER, "rb") as script, tempfile.TemporaryFile() as printed:
            child = subprocess.Popen(command, stdin=script, stdout=printed, stderr=subprocess.STDOUT, cwd=self.here,
                                     start_new_session=True)
            try:
                self.answer(child, terminal)
                code = child.wait(30)
            finally:
                if child.poll() is None:
                    child.kill()
                    child.wait()
            printed.seek(0)
            return code, printed.read().decode("utf-8")

    def answer(self, child, terminal):
        if terminal is None:
            return
        import select
        master, _, answer = terminal
        seen, answered, until = b"", False, time.monotonic() + 30
        while child.poll() is None and time.monotonic() < until:    # read to the end: a Mac's tty waits for that
            if select.select([master], [], [], 0.05)[0]:
                try:
                    seen += os.read(master, 4096)
                except OSError:
                    break
                if not answered and b"The working folder: " in seen:
                    os.write(master, answer)
                    answered = True
        self.assertTrue(answered, seen)

    def test_through_a_pipe_with_no_terminal_it_stops(self):
        code, text = self.piped(["--page", MARK, "--folder-name", "Work", "--source", self.unit])
        self.assertEqual(code, 2, text)
        self.assertIn('STOPPED: Could not find the working folder "Work" by itself. Nothing was done.', text)
        self.assertIn(H.RUN_WITH_FOLDER, text)
        self.assertNotIn("Traceback", text)

    def test_through_a_pipe_it_asks_on_the_terminal(self):
        path = self.folder("away", "My Folder", mark="ffffffff")     # another mark, so that the run ends at the answer
        import pty
        master, slave = pty.openpty()
        self.addCleanup(os.close, slave)
        self.addCleanup(os.close, master)       # the master first: a Mac's slave would wait until it is read empty
        typed = (path.replace(" ", "\\ ") + " \n").encode("utf-8")
        code, text = self.piped(["--page", MARK, "--folder-name", "My Folder", "--source", self.unit],
                                (master, slave, typed))
        self.assertEqual(code, 2, text)
        self.assertIn(H.NOT_CHOSEN[0] + path, text)


EVIL = "x\x1b]52;c;cm0gLXJm\x07\r\x1b[2J\x9bH\x7f ‮\nREADY TO WRITE."      # a name that would act on a terminal


class OwnCopy(HelperCase):
    """Nothing leaves the unit before its own copy is whole."""

    def test_a_copy_that_is_not_there_whole_keeps_the_file_on_the_unit(self):
        class NotWritten(MemoryDisk):
            def read(self, path, local_path):
                if path in ("logs/a.txt", "open-field-pad.bin", "user/pkin/.apply"):
                    return len(self.at[path])       # says it copied, and the copy is not there
                return super().read(path, local_path)
        unit = NotWritten(dict(PLAIN, **{"logs/a.txt": b"the log", "open-field-pad.bin": b"p", "user/pkin/.apply": b"n"}))
        code, h, _ = self.go(disk=unit)
        self.assertEqual(code, 1)
        self.assertEqual((unit.at.get("logs/a.txt"), unit.at.get("open-field-pad.bin"), unit.at.get("user/pkin/.apply")),
                         (b"the log", b"p", b"n"))
        self.assertEqual((h["logs"]["copied"], h["logs"]["left_on_unit"], h["oldPads"]["left_on_unit"]), (0, 1, 1))

    def test_a_local_file_the_call_did_not_make_is_never_removed(self):
        theirs = []

        def other_program(d, path, local):
            if path == "logs/a.txt":
                write(local, b"another file")
                theirs.append(local)
                raise H.DiskError("made up: the name was taken meanwhile")
        self.plain_unit({"logs/a.txt": b"the log"})
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"read": other_program})
        code, h, _ = self.go(wait="1", disk=disk)
        self.assertTrue(os.path.exists(theirs[0]), "a file of this computer the helper did not make was removed")
        self.assertEqual(read(theirs[0]), b"another file")
        self.assertEqual(read(os.path.join(self.unit, "logs", "a.txt")), b"the log")


class UnitNames(HelperCase):
    """On the unit's side a name is never compared loosely."""

    def test_a_pad_files_look_alike_at_the_top_stays(self):
        names = ["open-fıeld-pad.bin", "open-fİeld-pad.bin", "open-field-pad.bın", "open-field-pad-٣.bin"]
        unit = MemoryDisk(dict(PLAIN, **{n: b"the owner's" for n in names}))
        before = unit.tree()
        code, h, _ = self.go(wait="1", disk=unit)
        self.assertEqual(h["oldPads"]["found"], 0)
        self.assertEqual(unit.tree(), before)
        gate = H.Gate(unit)
        for phase, change in (("taking off", lambda n: gate.remove_file(n)),
                              ("putting", lambda n: gate.write("unused", "", n))):
            gate.phase = phase
            for name in names:
                gate.pad = name
                with self.subTest(phase + " " + ascii(name)):
                    with self.assertRaisesRegex(H.DiskError, "rules do not allow"):
                        change(name)

    def test_a_place_of_look_alike_letters_is_not_found(self):
        unit = MemoryDisk({"upgrade/firmware/blacKfin/bin/os.ldr": STOCK, "user/pK/a": b"a"})
        self.assertIsNone(H.resolve_folder(unit, "upgrade/firmware/blackfin/bin"))
        self.assertIsNone(H.find_child(unit, "user", "pk"))

    def test_a_pack_files_end_is_ascii(self):
        self.assertTrue(H.pack_leaf_problem("x.pacK"))
        self.assertFalse(H.pack_leaf_problem("x.PACK"))

        def alike(plan):
            plan["packs"][0].update(name="x.pacK", path="user/pk/x.pacK")
        self.refused(alike, "the plan names a place on the unit this helper does not write: user/pk/x.pacK")


class Shown(HelperCase):
    """Nothing of the unit, a name, the plan or an error acts on the terminal or makes a line of its own."""

    def assertClean(self, lines):
        for line in lines:
            self.assertFalse([c for c in line if ord(c) < 32 or 127 <= ord(c) < 160 or c in "  ‮"], ascii(line))
            self.assertNotEqual(line.strip(), "READY TO WRITE.")

    def assertCleanRun(self, h, out):
        self.assertClean([l for l in out if l])
        self.assertClean(h["lines"] + [h["message"]])
        with open(os.path.join(self.shared, "helper-" + h["run"] + ".log"), "rb") as f:
            logged = f.read().decode("utf-8").split("\n")[:-1]
        self.assertEqual(len(logged), len([l for l in out if l]), "one line logged for each line said")
        self.assertClean(logged)

    def test_a_name_of_the_unit(self):
        unit = MemoryDisk(PLAIN)

        def page(h):
            unit.at["content/" + EVIL] = b"1"
            put_plan(self.shared, page_plan(self.shared))
        code, h, out = self.go(page, disk=unit)
        self.assertEqual(code, 1)
        self.assertTrue(any("new since the listing" in l for l in out), out)
        self.assertCleanRun(h, out)

    def test_a_value_of_the_plan(self):
        self.plain_unit()

        def page(h):
            plan = page_plan(self.shared)
            plan["image"]["path"] = EVIL
            put_plan(self.shared, plan)
        code, h, out = self.go(page)
        self.assertEqual(code, 1)
        self.assertIn("\\u001b", h["message"])
        self.assertCleanRun(h, out)

    def test_an_errors_text(self):
        for error in (RuntimeError(EVIL), H.DiskError(EVIL)):
            with self.subTest(type(error).__name__):
                self.clear_shared()
                self.new_unit()
                self.plain_unit()

                def fault(d, local, folder, name):
                    raise error
                disk = Recording(H.FolderDisk(self.unit), self.shared, {"write": fault})
                code, h, out = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
                self.assertEqual(code, 1)
                self.assertCleanRun(h, out)

    def test_the_folders_name_and_path(self):
        out, asked = [], []
        code = H.main(["--folder-name", EVIL], echo=lambda l, c: out.append(l), ask=lambda q: asked.append(q))
        self.assertEqual(code, 2)
        self.assertClean(out)
        self.assertClean(asked[0].split("\n"))
        self.assertEqual(len(asked[0].split("\n")), 2, "the question's own line break alone")
        out = []
        self.assertEqual(H.main(["--folder", os.path.join(self.tmp, EVIL), "--page", MARK], echo=lambda l, c: out.append(l)), 2)
        self.assertClean(out)

    def test_the_terminal(self):
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            H.terminal("a" + EVIL)
        self.assertEqual(printed.getvalue().count("\n"), 1)
        self.assertClean([printed.getvalue()[:-1]])


class FirstCause(HelperCase):
    """What failed first is what the owner reads, and nothing is tried that cannot work."""

    def test_a_disk_of_this_computer_that_is_full(self):
        self.plain_unit({"logs/a.txt": b"log a", "logs/b.txt": b"log b"})
        real, tried = shutil.copyfileobj, []

        def full(src, dst, *args):
            if os.sep + "logs-" in str(getattr(src, "name", "")) or os.path.basename(str(getattr(src, "name", ""))) in ("a.txt", "b.txt"):
                tried.append(src.name)
                raise OSError(errno.ENOSPC, "No space left on device")
            return real(src, dst, *args)
        with mock.patch.object(H.shutil, "copyfileobj", full):
            code, h, _ = self.go(wait="1")
        self.assertEqual((code, len(tried)), (1, 1), h["lines"])
        self.assertIn("No space left on device", h["message"])
        self.assertIn("logs/a.txt could not be written on this computer", h["message"])
        self.assertEqual(sorted(os.listdir(os.path.join(self.unit, "logs"))), ["a.txt", "b.txt"])

    def test_the_cause_is_printed_when_the_log_takes_no_more(self):
        self.plain_unit()
        real, full = os.open, []

        def no_room(path, flags, *args, **kwargs):
            if full and str(path).endswith((".log", ".part")):
                raise OSError(errno.ENOSPC, "No space left on device")
            return real(path, flags, *args, **kwargs)

        def fault(d, path, local):
            full.append(1)
            raise H.DiskError("the cause: made up, no room")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"read": fault})
        with mock.patch.object(H.os, "open", no_room):
            code, h, out = self.go(disk=disk)
        self.assertEqual(code, 1)
        self.assertTrue(any("STOPPED" in l and "the cause: made up, no room" in l for l in out), out)
        self.assertFalse(any("something else stands in its place" in l for l in out), out)

    def test_a_log_that_stays_says_why(self):
        self.plain_unit({"logs/a.txt": b"log a"})

        def refuse(d, path):
            raise H.DiskError("made-up refusal")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"remove_file": refuse})
        code, h, _ = self.go(wait="1", disk=disk)
        self.assertIn("  the first that stayed: logs/a.txt: made-up refusal", h["lines"])

    def test_a_read_back_that_is_refused_says_why(self):
        self.plain_unit()

        def refuse(d, path, local):
            if path == "os.ldr":
                raise H.DiskError("made-up refusal", "refused")
        disk = Recording(H.FolderDisk(self.unit), self.shared, {"read": refuse})
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared)), disk=disk)
        self.assertStopped(code, h, "os.ldr does not read back from the unit with the SHA-256 of the file sent: made-up refusal")

    def test_a_swap_on_a_full_disk_is_not_tried_again(self):
        target = os.path.join(self.tmp, "t.json")
        with mock.patch.object(H.os, "replace", side_effect=OSError(errno.ENOSPC, "No space left on device")) as swap:
            with self.assertRaises(OSError):
                H.save_text(target, "new")
        self.assertEqual(swap.call_count, 1)


class Backups(HelperCase):
    """A backup of the owner's firmware file is never replaced."""

    def taken(self, names, original=False, links=True):
        unit = MemoryDisk(dict(PLAIN, **{"logs/a.txt": b"log"}))
        for number, name in enumerate(names):
            write(os.path.join(self.shared, name), b"kept %d" % number)
        if original:
            H.ORIGINAL_SHA = sha(STOCK)
        with mock.patch.object(H, "time", Clock()):
            if links:
                code, h, _ = self.go(wait="1", disk=unit)
            else:
                with mock.patch.object(H.os, "link", side_effect=OSError(errno.EPERM, "no links on this disk")):
                    code, h, _ = self.go(wait="1", disk=unit)
        for number, name in enumerate(names):
            self.assertEqual(read(os.path.join(self.shared, name)), b"kept %d" % number, name)
        return code, h, unit

    def test_both_names_taken_means_stop(self):
        for original in (False, True):
            for links in (True, False):
                with self.subTest(original=original, links=links):
                    self.clear_shared()
                    H.ORIGINAL_SHA = self.saved[0]
                    first = "os-1.7.3-yours.ldr" if original else "os-from-upgrade-folder-" + RUN + ".ldr"
                    second = "os-1.7.3-yours-" + RUN + ".ldr"
                    code, h, unit = self.taken([first, second], original, links)
                    self.assertEqual((code, h["state"]), (1, "stopped"))
                    self.assertIn(second + " are there already", h["message"])
                    self.assertIn("logs/a.txt", unit.at, "nothing was taken off")

    def test_a_disk_with_no_hard_links_still_keeps_the_copy(self):
        code, h, unit = self.taken([], links=False)
        self.assertEqual(read(os.path.join(self.shared, h["stock"]["file"])), STOCK)


if __name__ == "__main__":
    unittest.main()

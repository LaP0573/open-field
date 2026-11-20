"""The Linux helper, builder/open-field-helper.py, run whole on a folder that stands for the unit's disk.

  python -I -m unittest discover -s tools/tests -p "test_linux_helper.py"

Each test makes a stand-in disk and a shared folder in a temporary folder, runs the helper in a thread, and plays
the install page's part in Python: it waits for the listing, writes the files and a plan made by hand from that
listing, and lets the helper finish. Then it holds helper.json, the listings, the shared folder and the whole
stand-in disk (every name, size and SHA-256) to what the helper may do. The bytes of every file are made up here.
No unit and no USB are used: the transport over USB is not tested here.
"""

import contextlib
import hashlib
import importlib.util
import io
import json
import os
import shutil
import subprocess
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


H = load()
H.Libusb = NoRealLibusb     # the guard, set as the module is loaded: no test can reach the system's libusb


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
    for here, dirs, files in os.walk(root):
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


class HelperCase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="of-helper-")
        self.shared = os.path.join(self.tmp, "open-field")
        self.unit = os.path.join(self.tmp, "unit")
        os.makedirs(self.shared)
        self.mark()
        self.saved = (H.ORIGINAL_SHA, H.find_unit, H.Gate)

    def tearDown(self):
        H.ORIGINAL_SHA, H.find_unit, H.Gate = self.saved
        shutil.rmtree(self.tmp, ignore_errors=True)

    def mark(self, mark=MARK, bom=False):
        write(os.path.join(self.shared, "page.json"), (b"\xef\xbb\xbf" if bom else b"") +
              json.dumps({"page": 1, "id": mark, "made": "2026-11-19T10:00:00.000Z"}).encode())

    def go(self, page=None, args=(), disk=None, wait="60", mark=MARK, source=True):
        """The helper run in a thread; `page(helper)` is called once it has listed the disk. (code, helper.json, lines)."""
        out, res = [], {}
        argv = ["--folder", self.shared, "--wait-minutes", wait] + (["--page", mark] if mark else [])
        if source and disk is None:
            argv += ["--source", self.unit]
        argv += list(args)
        th = threading.Thread(target=lambda: res.setdefault("code", H.main(argv, disk=disk, echo=lambda line, color: out.append(line))))
        th.start()
        if page is not None:
            until = time.monotonic() + 5
            while time.monotonic() < until:
                h = helper_json(self.shared)
                if h and h["state"] in ("listed", "stopped"):
                    break
                time.sleep(0.001)
            self.assertEqual(h and h["state"], "listed", h and h["lines"])
            page(h)
        th.join(10)
        self.assertFalse(th.is_alive())
        return res.get("code"), helper_json(self.shared), out

    def assertStopped(self, code, h, why):
        self.assertEqual(code, 1)
        self.assertEqual(h["state"], "stopped")
        self.assertEqual(h["message"], why)
        self.assertEqual(h["lines"][-2:], ["STOPPED: " + why, H.NOTHING_MORE])

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
                     "The disk is listed: %d files, %d folders." % (h["listing"]["files"], h["listing"]["folders"]),
                     "The page's plan is here.", "The disk is as it was listed.", "os.ldr is on the unit (3000 bytes).",
                     "The pad file open-field-pad.bin is on the unit (500 bytes).", "The pack file base.pack is on the unit.",
                     "The disk's new listing is the plan's, row for row.", "user/pk/lfo.pack read back from the unit: the same bytes."]:
            self.assertIn(line, h["lines"])
        # as the Windows helper does, the three last lines are said after the state "ready" is saved
        said = [l[10:] for l in out if l]
        self.assertEqual(said[:len(h["lines"])], h["lines"])
        self.assertEqual(len(said), len(h["lines"]) + 3)
        self.assertEqual(said[-3], "READY TO WRITE.")
        log = read(os.path.join(self.shared, "helper-" + run + ".log")).decode("utf-8").splitlines()
        self.assertEqual([l[10:] for l in log], said)
        self.assertEqual(H.compare_rows(plan["expected"], json.loads(read(os.path.join(self.shared, "listing-after.json")))), ([], []))
        left = sorted(os.listdir(self.shared))
        self.assertEqual(left, sorted(["page.json", "helper.json", "helper-" + run + ".log", "listing-before.json", "listing-after.json",
                                       "plan.json", "open-field-os.ldr", "open-field-pad.bin", "packs", "logs-" + run, h["stock"]["file"]]))
        self.assertFalse(read(os.path.join(self.shared, "helper.json")).startswith(b"\xef\xbb\xbf"))

        # the whole stand-in disk: only what the helper may change has changed
        want = dict(before)
        for gone in ("logs/one.txt", "logs/two.txt", "open-field-pad.bin", "OPEN-FIELD-PAD-2.BIN", "Open-Field-Pad-7.bin"):
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
        make_unit(self.unit, {"UPGRADE/Firmware/blackfin/bin/os.ldr": STOCK, "UPGRADE/Firmware/blackfin/bin/OS.LDR": b"other",
                              "Logs/a.txt": b"log", "user/keep.txt": b"k", "content/x.aif": b"x" * 9})
        before = tree(self.unit)
        packs = [("zeta.pack", b"z" * 20), ("Alpha.pack", b"a" * 21), ("base.pack", b"b" * 22)]   # the plan's order, not sorted
        disk = Recording(H.FolderDisk(self.unit), self.shared)
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
        self.assertEqual(tree(self.unit), want)

    def test_only_the_exact_pack_name_is_replaced(self):
        self.plain_unit({"user/pk/LFO.pack": b"old", "logs": None})
        before = tree(self.unit)
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=[("lfo.pack", b"new!")])))
        self.assertEqual(code, 0, h["lines"])
        self.assertIn("The unit's logs folder holds no file.", h["lines"])
        want = dict(before, **{"os.ldr": entry(IMAGE), "user/pk/lfo.pack": entry(b"new!")})
        self.assertEqual(tree(self.unit), want)

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
        self.assertStopped(code, h, "the firmware file did not come off the unit")
        self.assertEqual(tree(self.unit), before)


class Refusals(HelperCase):
    def test_r0_no_page_json_nothing_made(self):
        shutil.rmtree(self.shared)
        code, h, out = self.go()
        self.assertEqual((code, h), (2, None))
        self.assertFalse(os.path.exists(self.shared))
        self.assertEqual(out, ["STOPPED: the install page did not choose this folder: " + self.shared,
                               "In the page, step 3, choose the folder open-field in your home folder (make it there if it is not there yet),",
                               "then copy the page's line again and run it. Nothing was done."])

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
        H.find_unit = lambda: None
        code, h, _ = self.go(args=["--find-seconds", "1"], source=False)
        self.assertStopped(code, h, H.NOT_SHOWN)
        self.assertIn(H.WAITING, h["lines"])

        def two():
            raise H.DiskError(H.MORE_THAN_ONE)
        H.find_unit = two
        self.clear_shared()
        code, h, _ = self.go(source=False)
        self.assertStopped(code, h, H.MORE_THAN_ONE)

    def test_r7_a_plan_of_another_run_is_not_taken(self):
        self.plain_unit()
        self.mark()
        before = tree(self.unit)

        def page(h):
            plan = page_plan(self.shared)
            plan["run"] = "2000-01-01_00-00-00"
            put_plan(self.shared, plan)
        code, h, _ = self.go(page, wait="1")
        self.assertStopped(code, h, "no plan came from the page in 1 minutes")
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
                shutil.rmtree(self.unit, ignore_errors=True)
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
        write(os.path.join(self.unit, "a", "B"), b"capital")
        disk.write(os.path.join(self.tmp, "new"), "a", "b")
        self.assertEqual((read(os.path.join(self.unit, "a", "b")), read(os.path.join(self.unit, "a", "B"))), (b"new", b"capital"))
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
        os.mkfifo(pipe)
        staged = os.path.join(self.tmp, "staged")
        self.assertFalse(H.stage_copy(link, staged), "a link is not read through")
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

    def test_a_system_that_is_not_linux_is_refused_before_anything_is_made(self):
        folder = os.path.join(self.tmp, "not-made")
        for system in ("win32", "cygwin"):
            with self.subTest(system):
                lines = []
                code = H.main(["--folder", folder, "--source", self.unit], echo=lambda l, c: lines.append(l), system=system)
                self.assertEqual((code, lines), (2, [H.NOT_LINUX]))
                self.assertIn("open-field-helper.ps1", lines[0])
                self.assertFalse(os.path.lexists(folder))

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
        shutil.rmtree(self.unit, ignore_errors=True)
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
            os.mkfifo(os.path.join(self.shared, "packs", "x.pack"))

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
        self.plain_unit({"user/PK/keep": b"k"})
        before = tree(self.unit)
        code, h, _ = self.go(lambda h: put_plan(self.shared, page_plan(self.shared, packs=[("x.pack", b"x" * 40)])))
        self.assertEqual(code, 0, h["lines"])
        self.assertEqual(tree(self.unit), dict(before, **{"os.ldr": entry(IMAGE), "user/pk": ("D",), "user/pk/x.pack": entry(b"x" * 40)}))

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
        for prefix in ("incoming-", "logs-", "old-pads-"):
            for kind in ("a link", "a folder with a link in it"):
                with self.subTest(prefix + " " + kind):
                    shutil.rmtree(self.unit, ignore_errors=True)
                    for n in os.listdir(self.shared):
                        if n != "page.json":
                            q = os.path.join(self.shared, n)
                            shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                    self.plain_unit({"logs/a.txt": b"log", "open-field-pad.bin": b"p"})
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
                    if prefix == "old-pads-":       # the logs are taken off before the old pads
                        del before["logs/a.txt"]
                    self.assertEqual(tree(self.unit), before, "nothing more is taken off the unit")

    def test_a_file_planted_in_a_folder_taking_off_right_after_it_is_made(self):
        for prefix, leaf in (("incoming-", "os.ldr"), ("logs-", "a.txt"), ("old-pads-", "open-field-pad.bin")):
            with self.subTest(prefix):
                shutil.rmtree(self.unit, ignore_errors=True)
                for n in os.listdir(self.shared):
                    if n != "page.json":
                        q = os.path.join(self.shared, n)
                        shutil.rmtree(q) if os.path.isdir(q) and not os.path.islink(q) else os.remove(q)
                self.plain_unit({"logs/a.txt": b"log", "open-field-pad.bin": b"p"})
                with self.planted_after_mkdir(prefix, leaf, os.path.join(self.outside, leaf)):
                    code, h, _ = self.go(wait="1")
                self.assertEqual(code, 1)
                if prefix == "incoming-":
                    self.assertStopped(code, h, "the firmware file did not come off the unit")
                elif prefix == "logs-":
                    self.assertEqual(h["logs"]["left_on_unit"], 1)
                    self.assertEqual(read(os.path.join(self.unit, "logs", "a.txt")), b"log")
                else:
                    self.assertEqual(h["oldPads"]["left_on_unit"], 1)
                    self.assertEqual(read(os.path.join(self.unit, "open-field-pad.bin")), b"p")

    def test_the_staging_folder_planted(self):
        def as_link(h):
            os.symlink(self.outside, os.path.join(self.shared, "to-unit-" + h["run"]))

        def packs_inside(h):
            stage = os.path.join(self.shared, "to-unit-" + h["run"])
            os.mkdir(stage)
            os.symlink(self.outside, os.path.join(stage, "packs"))
        for what, plant in (("a link", as_link), ("its packs a link", packs_inside)):
            with self.subTest(what):
                shutil.rmtree(self.unit, ignore_errors=True)
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
                shutil.rmtree(self.unit, ignore_errors=True)
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
        self.assertStopped(code, h, "os.ldr does not read back from the unit with the SHA-256 of the file sent")

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
                shutil.rmtree(self.unit, ignore_errors=True)
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
                shutil.rmtree(self.unit, ignore_errors=True)
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


if __name__ == "__main__":
    unittest.main()

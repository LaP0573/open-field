"""The listing of packs, catalog/packs.json, and its check, tools/catalog_check.py (docs/catalog.md).

  python -I -m unittest discover -s tools/tests -p "test_catalog.py"

Everything here needs nothing but this tree: the pack files the check reads are made by tools/pack_build.py's own
maker, with no compiler, and the downloads are answered from memory. One test needs more and is skipped without it:
the listing held to the pack files a build gives (tools/build_all.py, which needs the toolchain: docs/build.md), in
out/packs/ and out/examples/, or in the folders named by the environment's OPEN_FIELD_PACKS, parted by the system's
path separator.
"""

import copy
import hashlib
import importlib.util
import io
import json
import os
import tempfile
import unittest
from contextlib import redirect_stdout

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def tool(name):
    spec = importlib.util.spec_from_file_location("test_catalog_" + name, os.path.join(ROOT, "tools", name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


check = tool("catalog_check")
pb = check.pack_build
FIRMWARES = check.firmwares()
OS_173 = next(word for word, version in FIRMWARES.items() if version == "1.7.3")
COMMIT = "0123456789abcdef0123456789abcdef01234567"
REPOSITORY = "https://github.com/someone/some-pack"


def pack(name="tester", version=2, target_os=OS_173, works=(), imports=(), exports=()):
    """A pack file as the loader reads one: a body of zeros that changes no place of the firmware."""
    link = pb.ROOM_START
    return pb.make3(bytes(64), link, 64, 0, name, target_os, (link, link), version=version, works=works, imports=imports,
                    exports=exports, no_table=True)


def entry_for(data, **change):
    """A listed entry that says what the file says."""
    fields, _body = pb.parse(data)
    entry = {"id": fields["name"], "author": "someone", "status": "listed", "categories": ["tools"],
             "description": "A pack made for the tests.", "repository": REPOSITORY, "commit": COMMIT, "path": "build/%s.pack" % fields["name"],
             "sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data), "version": fields["version"], "firmware": "1.7.3",
             "places": 0, "needs": [], "folders": []}
    entry.update(change)
    return entry


def base_entry(level=3):
    """A base held, with no file: what the other entries need, and the level they may ask for."""
    return {"id": "base", "author": "open-field", "status": "reserved", "categories": ["library"], "description": "The base.",
            "offers_level": level}


def listing(*entries):
    return {"about": "A listing for the tests.", "categories": {"tools": "tools", "library": "library", "examples": "examples"},
            "packs": [base_entry()] + list(entries)}


def run(the_listing, **options):
    lines, in_order = check.check(the_listing, FIRMWARES, **options)
    return "\n".join(lines), in_order


class TheTreesListing(unittest.TestCase):
    def test_the_listing_of_this_tree_is_in_order(self):
        with open(check.CATALOG, encoding="utf-8") as f:
            said, in_order = run(json.load(f))
        self.assertTrue(in_order, said)

    def test_every_entry_of_this_tree_names_one_of_its_categories_and_a_firmware_it_has_a_profile_for(self):
        with open(check.CATALOG, encoding="utf-8") as f:
            the_listing = json.load(f)
        for entry in the_listing["packs"]:
            self.assertTrue(set(entry["categories"]) <= set(the_listing["categories"]), entry["id"])
            self.assertIn(entry.get("firmware", "1.7.3"), FIRMWARES.values(), entry["id"])

    def test_the_examples_are_held_and_never_offered_to_owners(self):
        """The example packs are for people who write packs: their sources lie in examples/, the build makes their
        files, and the install page offers only listed entries. Their entries stay reserved, so that their names and
        their folders stay held."""
        with open(check.CATALOG, encoding="utf-8") as f:
            the_listing = json.load(f)
        examples = [entry for entry in the_listing["packs"] if "examples" in entry["categories"]]
        self.assertTrue(examples)
        for entry in examples:
            self.assertEqual(entry["status"], "reserved", entry["id"])
            self.assertTrue(os.path.isdir(os.path.join(ROOT, "examples", entry["id"])), entry["id"])

    def test_the_firmware_word_is_read_from_the_profile(self):
        self.assertEqual(FIRMWARES[OS_173], "1.7.3")
        with open(os.path.join(ROOT, "profiles", "op1f-1.7.3.json"), encoding="utf-8") as f:
            self.assertEqual(pb.os_id(json.load(f)["stock_sha256"]), OS_173)


class AnEntryHeldToItsFile(unittest.TestCase):
    def held(self, data, entry):
        with tempfile.TemporaryDirectory() as folder:
            with open(os.path.join(folder, os.path.basename(entry["path"])), "wb") as f:
                f.write(data)
            return run(listing(entry), folders=[folder])

    def test_a_file_that_says_what_its_entry_says_is_in_order(self):
        data = pack(works=[("base", 2, True)], imports=[("op1f_level_2", True)])
        said, in_order = self.held(data, entry_for(data, needs=["base"], level=2))
        self.assertTrue(in_order, said)
        self.assertIn("ok: tester (listed, its file held)", said)

    def test_each_thing_the_file_says_is_held_to_the_entry(self):
        data = pack(works=[("base", 2, True), ("other", 1, False)], imports=[("op1f_level_2", True)])
        right = entry_for(data, needs=["base"], level=2)
        for change, words in (({"version": 3}, "the file's version is 2"),
                              ({"places": 1}, "the file's places is 0"),
                              ({"needs": []}, 'the file\'s needs is ["base"]'),
                              ({"level": 1}, "the file's level is 2"),
                              ({"offers_level": 1}, "the file's offers level is null"),
                              ({"folders": [{"name": "mine", "place": "mine"}]}, "the file's folders is []"),
                              ({"id": "testers"}, 'the file\'s id is "tester"')):
            with self.subTest(change=change):
                entry = dict(right, **change)
                if "id" in change:
                    entry["path"] = right["path"]
                said, in_order = self.held(data, entry)
                self.assertFalse(in_order)
                self.assertIn(words, said)

    def test_a_pack_for_another_firmware_is_named_as_such(self):
        data = pack(target_os=0x12345678)
        said, in_order = self.held(data, entry_for(data))
        self.assertFalse(in_order)
        self.assertIn("one this repository has no profile for (0x12345678)", said)

    def test_the_base_says_the_highest_level_it_offers(self):
        data = pack(name="mybase", exports=[("op1f_level_1", 0), ("op1f_level_2", 4), ("op1f_level_3", 8)])
        said, in_order = self.held(data, entry_for(data, offers_level=3))
        self.assertTrue(in_order, said)

    def test_another_file_is_refused_by_its_size_and_its_sha256(self):
        data, other = pack(), pack(version=5)
        entry = entry_for(data)
        said, in_order = self.held(other, entry)
        self.assertFalse(in_order)
        self.assertIn("the file's SHA-256 is %s, not the entry's" % hashlib.sha256(other).hexdigest(), said)
        self.assertNotIn("version", said.split("SHA-256", 1)[1])         # another file: what it says of itself is not read

    def test_a_file_that_is_no_pack_is_refused(self):
        data = b"no pack at all, " * 8
        said, in_order = self.held(data, entry_for(pack(), sha256=hashlib.sha256(data).hexdigest(), bytes=len(data)))
        self.assertFalse(in_order)
        self.assertIn("the file is no pack the loader takes", said)

    def test_a_file_that_is_not_in_the_folders_is_refused(self):
        said, in_order = run(listing(entry_for(pack())), folders=[tempfile.gettempdir() + os.sep + "no-such-folder"])
        self.assertFalse(in_order)
        self.assertIn("its file is not in", said)


class Downloads(unittest.TestCase):
    def test_a_listed_entry_is_downloaded_from_its_repository_at_its_commit(self):
        data = pack(name="two words")
        entry = entry_for(data, path="packs/two words.pack")
        asked = []

        def get(url):
            asked.append(url)
            return data
        said, in_order = run(listing(entry), download=True, get=get)
        self.assertTrue(in_order, said)
        self.assertEqual(asked, ["https://raw.githubusercontent.com/someone/some-pack/%s/packs/two%%20words.pack" % COMMIT])

    def test_with_the_listing_before_only_what_changed_is_downloaded(self):
        data, newer = pack(), pack(version=3)
        old = entry_for(data)
        new = entry_for(newer, commit="f" * 40)
        asked = []

        def get(url):
            asked.append(url)
            return newer
        said, in_order = run(listing(old), before=listing(old), download=True, get=get)
        self.assertTrue(in_order, said)
        self.assertEqual(asked, [])
        said, in_order = run(listing(new), before=listing(old), download=True, get=get)
        self.assertTrue(in_order, said)
        self.assertEqual(len(asked), 1)
        self.assertIn("f" * 40, asked[0])

    def test_a_link_that_does_not_answer_is_the_entry_s_fault(self):
        def get(url):
            raise OSError("nobody home")
        said, in_order = run(listing(entry_for(pack())), download=True, get=get)
        self.assertFalse(in_order)
        self.assertIn("could not be downloaded", said)
        self.assertIn("nobody home", said)

    def test_a_reserved_entry_is_not_downloaded(self):
        def get(url):
            raise AssertionError("asked for %s" % url)
        said, in_order = run(listing(), download=True, get=get)
        self.assertTrue(in_order, said)


class TheFormOfAnEntry(unittest.TestCase):
    def refused(self, entry, words):
        said, in_order = run(listing(entry))
        self.assertFalse(in_order, said)
        self.assertIn(words, said)

    def test_ids_that_cannot_name_a_file_or_fill_a_header_are_refused(self):
        for bad in ("a/b", "a:b", "ends ", " starts", ".dot", "dot.", "sixteen letters!", "", "tab\there", "é"):
            with self.subTest(id=bad):
                self.refused(entry_for(pack(), id=bad), "its id is not")
        for good in ("8 tracks", "fx train", "a", "x-1_y.z"):
            with self.subTest(id=good):
                self.assertEqual(check.entry_faults(entry_for(pack(), id=good), {"tools": "tools"}, FIRMWARES), [])

    def test_a_key_the_listing_does_not_know_is_refused(self):
        self.refused(entry_for(pack(), homepage="https://example.org"), "keys the listing does not know: homepage")

    def test_a_listed_entry_says_where_its_file_lies(self):
        entry = entry_for(pack())
        del entry["commit"]
        self.refused(entry, "it lacks commit")

    def test_a_reserved_entry_names_no_repository_or_commit(self):
        self.refused(entry_for(pack(), status="reserved"), "a reserved entry names no repository or commit")

    def test_a_reserved_entry_with_a_file_says_all_of_it(self):
        entry = {"id": "later", "author": "someone", "status": "reserved", "categories": ["tools"], "description": "Held for later.",
                 "sha256": "0" * 64}
        self.refused(entry, "it lacks bytes, version, firmware, places, needs, folders")
        del entry["sha256"]
        said, in_order = run(listing(entry))
        self.assertTrue(in_order, said)

    def test_the_fields_forms(self):
        for change, words in (({"categories": []}, "its categories are not"),
                              ({"categories": ["tools", "tools"]}, "its categories are not"),
                              ({"categories": ["tools", "library", "examples", "tools"]}, "its categories are not"),
                              ({"categories": ["sounds"]}, "its categories are not"),
                              ({"categories": [{"a": 1}]}, "its categories are not"),
                              ({"description": "two\nlines"}, "its description is not"),
                              ({"description": "x" * 141}, "its description is not"),
                              ({"author": "-someone"}, "its author is not"),
                              ({"repository": "https://gitlab.com/someone/pack"}, "its repository is not"),
                              ({"commit": "0123456"}, "its commit is not a full commit id"),
                              ({"path": "../out/x.pack"}, "its path is not a plain path"),
                              ({"path": "/x.pack"}, "its path is not a plain path"),
                              ({"path": "x.bin"}, "its path does not end in .pack"),
                              ({"path": "a\\x.pack"}, "its path is not a plain path"),
                              ({"sha256": "A" * 64}, "its sha256 is not"),
                              ({"bytes": True}, "its size is not"),
                              ({"bytes": 50}, "its size is not"),
                              ({"version": 0}, "its version is not"),
                              ({"firmware": "9.9.9"}, "its firmware is not one this repository has a profile for"),
                              ({"places": -1}, "its count of places is not"),
                              ({"needs": ["base", "base"]}, "what it needs is not"),
                              ({"level": 0}, "its level is not"),
                              ({"folders": [{"name": "a"}]}, "a folder is not a name and a place"),
                              ({"folders": [{"name": "a/b", "place": "a"}]}, "is not named as the base takes"),
                              ({"folders": [{"name": "f%d" % n, "place": "f%d" % n} for n in range(9)]}, "its folders are not a list"),
                              ({"status": "gone"}, "its status is not one of")):
            with self.subTest(change=change):
                self.refused(entry_for(pack(), **change), words)


class TheListingAsAWhole(unittest.TestCase):
    def test_two_entries_never_share_an_id_in_small_letters_or_capitals(self):
        said, in_order = run(listing(entry_for(pack()), entry_for(pack(), id="Tester")))
        self.assertFalse(in_order)
        self.assertIn("its id is the id of 'tester'", said)

    def test_two_entries_never_share_a_folder(self):
        one = entry_for(pack(), folders=[{"name": "notes", "place": "notes"}])
        two = entry_for(pack(name="other"), folders=[{"name": "Notes", "place": "elsewhere"}])
        said, in_order = run(listing(one, two))
        self.assertFalse(in_order)
        self.assertIn("its folder's name 'Notes' is held by 'tester'", said)

    def test_a_folder_never_takes_a_name_the_disk_mode_shows_already(self):
        for taken in ("synth", "Packs", "fx"):
            with self.subTest(name=taken):
                said, in_order = run(listing(entry_for(pack(), folders=[{"name": taken, "place": "mine"}])))
                self.assertFalse(in_order)
                self.assertIn("its folder's name %r is" % taken, said)

    def test_a_retired_entry_keeps_its_id_and_its_folders_held(self):
        retired = {"id": "old", "author": "someone", "status": "retired", "categories": ["tools"], "description": "No longer offered.",
                   "sha256": "0" * 64, "bytes": 1000, "version": 1, "firmware": "1.7.3", "places": 0, "needs": [],
                   "folders": [{"name": "old", "place": "old"}]}
        said, in_order = run(listing(retired, entry_for(pack(), folders=[{"name": "old", "place": "new"}])))
        self.assertFalse(in_order)
        self.assertIn("is held by 'old'", said)

    def test_a_pack_it_needs_is_in_the_listing_and_offered(self):
        said, in_order = run(listing(entry_for(pack(), needs=["nowhere"])))
        self.assertFalse(in_order)
        self.assertIn("it needs 'nowhere', which the listing does not offer", said)
        other = dict(entry_for(pack(name="other")), status="retired")
        said, in_order = run(listing(other, entry_for(pack(), needs=["other"])))
        self.assertFalse(in_order)
        self.assertIn("it needs 'other', which the listing does not offer", said)

    def test_no_pack_needs_a_level_the_base_does_not_offer(self):
        said, in_order = run(listing(entry_for(pack(), level=4)))
        self.assertFalse(in_order)
        self.assertIn("it needs level 4 of the base, and the listing's base offers 3", said)

    def test_the_listing_s_own_keys(self):
        for change in ({"extra": 1}, {"categories": {"Tools": "tools"}}, {"packs": {}}):
            with self.subTest(change=change):
                said, in_order = run(dict(listing(), **change))
                self.assertFalse(in_order, said)


class OurOwnPacks(unittest.TestCase):
    """The listing's `ours`: the repositories of open-field's own packs, on which the install page shows no warning."""

    OURS = "https://github.com/open-field-test/packs"

    def with_ours(self, *entries, ours=None):
        return dict(listing(*entries), ours=[self.OURS] if ours is None else ours)

    def test_an_entry_of_ours_in_one_of_ours_is_in_order(self):
        said, in_order = run(self.with_ours(entry_for(pack(), author="open-field", repository=self.OURS)))
        self.assertTrue(in_order, said)

    def test_an_entry_of_ours_lies_in_one_of_ours(self):
        said, in_order = run(self.with_ours(entry_for(pack(), author="open-field")))
        self.assertFalse(in_order)
        self.assertIn("its author is open-field, and its repository is not one of the listing's ours", said)

    def test_an_entry_in_one_of_ours_is_ours(self):
        said, in_order = run(self.with_ours(entry_for(pack(), repository=self.OURS)))
        self.assertFalse(in_order)
        self.assertIn("its repository is one of the listing's ours, and its author is not open-field", said)

    def test_the_form_of_ours(self):
        for bad in ("https://github.com/a/b", ["https://gitlab.com/a/b"], [self.OURS, self.OURS], [1]):
            with self.subTest(ours=bad):
                said, in_order = run(self.with_ours(ours=bad))
                self.assertFalse(in_order)
                self.assertIn("its ours is not a list of repositories", said)

    def test_ours_never_changes_in_a_change(self):
        said, in_order = run(self.with_ours(), before=listing())
        self.assertFalse(in_order)
        self.assertIn("its ours changed", said)
        said, in_order = run(self.with_ours(), before=self.with_ours())
        self.assertTrue(in_order, said)

    def test_the_tree_s_own_packs_lie_in_its_ours(self):
        with open(check.CATALOG, encoding="utf-8") as f:
            the_listing = json.load(f)
        self.assertTrue(the_listing["ours"])
        for entry in the_listing["packs"]:
            if "repository" in entry:
                self.assertEqual(entry["author"] == "open-field", entry["repository"] in the_listing["ours"], entry["id"])


class AChangeHeldToTheListingBefore(unittest.TestCase):
    def test_an_entry_is_never_taken_out(self):
        old = listing(entry_for(pack()))
        said, in_order = run(listing(), before=old)
        self.assertFalse(in_order)
        self.assertIn("REFUSED: tester: it was taken out", said)

    def test_a_status_never_moves_back(self):
        listed = entry_for(pack())
        reserved = {key: listed[key] for key in check.ALWAYS}
        reserved["status"] = "reserved"
        said, in_order = run(listing(reserved), before=listing(listed))
        self.assertFalse(in_order)
        self.assertIn("its status moved back, from listed to reserved", said)
        said, in_order = run(listing(dict(listed, status="retired")), before=listing(listed))
        self.assertTrue(in_order, said)

    def test_an_id_never_changes(self):
        said, in_order = run(listing(entry_for(pack(), id="Tester")), before=listing(entry_for(pack())))
        self.assertFalse(in_order)
        self.assertIn("its id was 'tester'", said)

    def test_a_new_file_comes_with_a_higher_version(self):
        old_data = pack(version=2)
        old = entry_for(old_data)
        same_version = dict(old, sha256="1" * 64, commit="f" * 40)
        said, in_order = run(listing(same_version), before=listing(old))
        self.assertFalse(in_order)
        self.assertIn("its file changed and its version did not grow past 2", said)
        said, in_order = run(listing(dict(same_version, version=3)), before=listing(old))
        self.assertTrue(in_order, said)

    def test_the_same_version_for_another_firmware_this_repository_has_a_profile_for(self):
        """A move of the project to another firmware: the same pack, built again for it, keeps its version; its
        entry names the new firmware, and the old file stays at its old commit."""
        old = entry_for(pack(version=2), firmware="1.7.0")
        moved = dict(entry_for(pack(version=2, works=[("base", 1, False)])), commit="f" * 40)
        self.assertNotEqual(moved["sha256"], old["sha256"])
        known = dict(FIRMWARES)
        known[0x12345678] = "1.7.0"                                     # the old firmware need not have a profile here

        def held(new, before, firmwares=known):
            lines, in_order = check.check(listing(new), firmwares, before=listing(before))
            return "\n".join(lines), in_order
        said, in_order = held(moved, old)
        self.assertTrue(in_order, said)
        said, in_order = held(moved, old, FIRMWARES)                    # 1.7.0 has no profile: the old entry is still the old one
        self.assertTrue(in_order, said)
        said, in_order = held(dict(moved, version=1), old)              # the version never goes down
        self.assertFalse(in_order)
        self.assertIn("its version did not grow past 2", said)
        within = dict(entry_for(pack(version=2), firmware="1.7.3"))
        said, in_order = held(dict(moved), within)                      # within one firmware the version must grow
        self.assertFalse(in_order)
        self.assertIn("its file changed and its version did not grow past 2", said)
        said, in_order = held(dict(moved, version=3), within)
        self.assertTrue(in_order, said)

    def test_a_move_to_a_firmware_with_no_profile_here_keeps_no_version(self):
        old = entry_for(pack(version=2))
        moved = dict(old, sha256="1" * 64, commit="f" * 40, firmware="9.9.9")
        self.assertEqual(check.before_faults(listing(moved), listing(old), FIRMWARES)["tester"][0][:52],
                         "its file changed and its version did not grow past 2")
        self.assertIn("tester", check.before_faults(listing(dict(moved, firmware="1.7.3")), listing(dict(old, firmware="1.7.0"))))   # no firmwares known
        self.assertNotIn("tester", check.before_faults(listing(dict(moved, firmware="1.7.3")), listing(dict(old, firmware="1.7.0")), FIRMWARES))


class TheCommand(unittest.TestCase):
    def main(self, *argv):
        out = io.StringIO()
        with redirect_stdout(out):
            answer = check.main(list(argv))
        return answer, out.getvalue()

    def test_it_answers_0_for_a_listing_in_order_and_1_otherwise(self):
        with tempfile.TemporaryDirectory() as folder:
            good, bad, odd = (os.path.join(folder, name) for name in ("good.json", "bad.json", "odd.json"))
            for path, value in ((good, listing()), (bad, listing(entry_for(pack(), id="a/b"))), (odd, ["not a listing"])):
                with open(path, "w", encoding="utf-8") as f:
                    json.dump(value, f)
            self.assertEqual(self.main("--catalog", good)[0], 0)
            answer, said = self.main("--catalog", bad)
            self.assertEqual(answer, 1)
            self.assertIn("REFUSED", said)
            self.assertEqual(self.main("--catalog", good, "--before", odd)[0], 1)
            self.assertEqual(self.main("--catalog", os.path.join(folder, "missing.json"))[0], 1)


def built_folders():
    named = os.environ.get("OPEN_FIELD_PACKS", "")
    folders = [f for f in named.split(os.pathsep) if f] if named else [os.path.join(ROOT, "out", "packs"), os.path.join(ROOT, "out", "examples")]
    return [f for f in folders if os.path.isdir(f)]


class ABundledEntry(unittest.TestCase):
    """base.pack lies in this repository beside the install page, which puts it on every unit it installs: its entry
    is `bundled`, and the check holds it to that file every time, with nothing downloaded."""

    @classmethod
    def setUpClass(cls):
        with open(check.CATALOG, encoding="utf-8") as f:
            cls.listing = json.load(f)
        cls.base = next(entry for entry in cls.listing["packs"] if entry["id"] == "base")

    def test_the_tree_s_base_is_bundled_and_held_to_the_file_beside_the_install_page(self):
        self.assertEqual((self.base["status"], self.base["path"]), ("bundled", "builder/base.pack"))
        said, in_order = run(copy.deepcopy(self.listing))
        self.assertTrue(in_order, said)
        self.assertIn("ok: base (bundled, its file held)", said)
        with open(os.path.join(ROOT, "builder", "base.pack"), "rb") as f:
            data = f.read()
        with open(os.path.join(ROOT, "builder", "base-pack.json"), encoding="utf-8") as f:
            named = json.load(f)
        with open(os.path.join(ROOT, "packs", "built.json"), encoding="utf-8") as f:
            built = json.load(f)["files"]["base.pack"]
        for said_of_it in ({"bytes": named["bytes"], "sha256": named["sha256"]}, built):
            self.assertEqual(said_of_it, {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()})
        self.assertEqual((self.base["bytes"], self.base["sha256"], self.base["version"]), (len(data), hashlib.sha256(data).hexdigest(), named["version"]))
        self.assertEqual([entry["id"] for entry in self.listing["packs"] if entry["status"] == "bundled"], ["base"])

    def test_it_lies_beside_the_install_page_and_names_no_repository(self):
        said, in_order = run(listing(dict(self.base, path="packs/base.pack")))
        self.assertFalse(in_order)
        self.assertIn("its path begins with builder/", said)
        said, in_order = run(listing(dict(self.base, repository=REPOSITORY, commit=COMMIT)))
        self.assertFalse(in_order)
        self.assertIn("a bundled entry names no repository or commit", said)

    def test_its_file_is_read_from_this_repository_and_never_downloaded(self):
        def get(url):
            raise AssertionError("asked for %s" % url)
        said, in_order = run(listing(dict(self.base, id="base2", path="builder/no such file.pack")), download=True, get=get)
        self.assertFalse(in_order)
        self.assertIn("its file is not in this repository at builder/no such file.pack", said)

    def test_a_reserved_entry_may_become_bundled_and_a_listed_one_may_not(self):
        def alone(entry):
            return {"about": "A listing for the tests.", "categories": {"library": "library"}, "packs": [entry]}
        reserved = dict({key: self.base[key] for key in check.ALWAYS}, status="reserved", offers_level=3)
        said, in_order = run(alone(copy.deepcopy(self.base)), before=alone(reserved))       # the step of 2026-11-18
        self.assertTrue(in_order, said)
        listed = dict(copy.deepcopy(self.base), status="listed", repository=REPOSITORY, commit=COMMIT)
        said, in_order = run(alone(copy.deepcopy(self.base)), before=alone(listed))
        self.assertFalse(in_order)
        self.assertIn("its status moved back, from listed to bundled", said)


class TheListingHeldToBuiltFiles(unittest.TestCase):
    @unittest.skipUnless(built_folders(), "no built pack files: tools/build_all.py, or OPEN_FIELD_PACKS")
    def test_every_entry_with_a_file_is_the_file_a_build_gives(self):
        with open(check.CATALOG, encoding="utf-8") as f:
            the_listing = json.load(f)
        # Only open-field's own entries come out of this tree's build: an entry of another author's repository is held
        # to its file by the listing job, which downloads it.
        ours = the_listing.get("ours", [])
        the_listing["packs"] = [e for e in the_listing["packs"] if e["status"] == "bundled" or e.get("repository") in ours]
        said, in_order = run(copy.deepcopy(the_listing), folders=built_folders())
        self.assertTrue(in_order, said)
        self.assertEqual(said.count("its file held"), sum(1 for e in the_listing["packs"] if e.get("sha256") and e["status"] != "retired"))


if __name__ == "__main__":
    unittest.main()

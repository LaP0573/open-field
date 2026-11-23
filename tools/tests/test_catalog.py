"""The listing of packs, catalog/packs.json, and its check, tools/catalog_check.py (docs/catalog.md).

  python -I -m unittest discover -s tools/tests -p "test_catalog.py"

Everything here needs nothing but this tree: the pack files the check reads are made by tools/pack_build.py's own
maker, with no compiler, and the downloads are answered from memory. One test needs more and is skipped without it:
the entries whose files this repository builds (the base and the examples; a listed pack's file is built in its own
repository) held to those files (tools/build_all.py, which needs the toolchain: docs/build.md), in
out/packs/ and out/examples/, or in the folders named by the environment's OPEN_FIELD_PACKS, parted by the system's
path separator.
"""

import copy
import hashlib
import importlib.util
import io
import json
import os
import struct
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


def card_pack(name="tester", version=2, folders=(("notes", "notes"),), card_version=None, folders_at=None):
    """A pack file with a card of level 3 that lists these folders, each (name, place), as the base reads one; the
    list where the pack's body puts it, or at the linked address `folders_at`."""
    link, at = pb.ROOM_START, 4
    listed = at + pb.CARD_3_BYTES
    address = link + listed if folders_at is None else folders_at
    table = pb.folder_table([(folder_name, place, True) for folder_name, place in folders])
    body = bytearray(listed + len(table))
    pb.CARD_3.pack_into(body, at, pb.CARD_MARK, pb.CARD_3_BYTES, 3, version if card_version is None else card_version,
                        name.encode("ascii"), 0, 0, 0, 0, address if folders else 0, len(folders))
    body[listed:] = table
    places = [(at + pb.CARD_2_BYTES, pb.KIND_WORD, None, address)] if folders else []
    return pb.make3(bytes(body), link, len(body), 0, name, OS_173, (link, link), version=version,
                    imports=[("op1f_level_3", True)], places=places, card_place=at, no_table=True)


def followed_for(data, **change):
    """A listed entry that follows the branch main of its repository, for a file that stands at its head."""
    entry = {key: value for key, value in entry_for(data).items() if key not in ("commit", "sha256", "bytes", "places")}
    entry["follow"] = "main"
    entry.update(change)
    return entry


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

    def test_folders_that_hold_no_entry_s_file_are_refused(self):
        folder = tempfile.gettempdir() + os.sep + "no-such-folder"
        said, in_order = run(listing(entry_for(pack())), folders=[folder])
        self.assertFalse(in_order)
        self.assertIn("REFUSED: the folders given (%s) hold no entry's file" % folder, said)


class FoldersOfSomeAuthorsFiles(unittest.TestCase):
    """With --files, an author with a file in the folders is held whole, and one with none there is held to the form:
    the maintainer gives the folders of a build, with every file of open-field's own packs and none of another
    author's, and an author gives a folder with their own file alone (docs/catalog.md)."""

    def setUp(self):
        self.ours = [pack(name="mine"), pack(name="also mine")]
        self.theirs = [pack(name="theirs"), pack(name="theirs too")]
        self.followed = card_pack(name="followed")
        self.listing = listing(*[entry_for(data, author="open-field", status="reserved", categories=["examples"]) for data in self.ours]
                               + [entry_for(data) for data in self.theirs]
                               + [followed_for(self.followed, folders=[{"name": "notes", "place": "notes"}])])
        for entry in self.listing["packs"][1:3]:                # held as the examples are: a file, no repository
            del entry["repository"], entry["commit"]

    def held(self, *files):
        with tempfile.TemporaryDirectory() as folder:
            for data in files:
                with open(os.path.join(folder, pb.parse(data)[0]["name"] + ".pack"), "wb") as f:
                    f.write(data)
            return run(self.listing, folders=[folder])

    def test_the_folders_of_a_build_hold_ours_and_pass_another_author_s_entries_on_their_form(self):
        said, in_order = self.held(*self.ours)
        self.assertTrue(in_order, said)
        for name in ("mine", "also mine"):
            self.assertIn("ok: %s (reserved, its file held)" % name, said)
        for name in ("theirs", "theirs too"):
            self.assertIn("ok: %s (listed, its file not held: no file of someone's in the folders given)" % name, said)
        self.assertIn("ok: followed (listed, follows main, its file not held: no file of someone's in the folders given)", said)

    def test_one_of_ours_missing_is_refused(self):
        said, in_order = self.held(self.ours[0])
        self.assertFalse(in_order)
        self.assertIn("REFUSED: also mine: its file is not in", said)
        self.assertIn("ok: mine (reserved, its file held)", said)

    def test_an_author_s_own_folder_holds_their_file_and_passes_ours_on_their_form(self):
        said, in_order = self.held(*self.theirs, self.followed)
        self.assertTrue(in_order, said)
        self.assertIn("ok: theirs (listed, its file held)", said)
        self.assertIn("ok: followed (listed, follows main, its file held)", said)
        self.assertIn("ok: mine (reserved, its file not held: no file of open-field's in the folders given)", said)

    def test_an_author_with_one_file_there_and_another_missing_is_refused(self):
        said, in_order = self.held(self.theirs[0])
        self.assertFalse(in_order)
        self.assertIn("REFUSED: theirs too: its file is not in", said)
        self.assertIn("REFUSED: followed: its file is not in", said)
        self.assertIn("ok: theirs (listed, its file held)", said)

    def test_a_file_held_is_still_held_to_its_entry(self):
        said, in_order = self.held(self.theirs[0], pack(name="theirs too", version=5), self.followed)
        self.assertFalse(in_order)
        self.assertIn("REFUSED: theirs too: the file's SHA-256 is", said)


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

    def test_a_picture_is_downloaded_from_the_same_repository_at_the_same_commit_and_held(self):
        import struct, zlib
        def png(width, height, colour=6):
            ihdr = struct.pack(">IIBBBBB", width, height, 8, colour, 0, 0, 0)
            return check.PNG_SIGNATURE + struct.pack(">I", 13) + b"IHDR" + ihdr + struct.pack(">I", zlib.crc32(b"IHDR" + ihdr)) + bytes(12)
        data = pack()

        def entry_with(picture_bytes):
            return entry_for(data, picture={"path": "thumbs/tester.png", "sha256": hashlib.sha256(picture_bytes).hexdigest(), "bytes": len(picture_bytes)})
        good = png(300, 240)
        asked = []

        def get(url):
            asked.append(url)
            return good if url.endswith(".png") else data
        said, in_order = run(listing(entry_with(good)), download=True, get=get)
        self.assertTrue(in_order, said)
        self.assertIn("its picture held", said)
        self.assertEqual(asked[1], "https://raw.githubusercontent.com/someone/some-pack/%s/thumbs/tester.png" % COMMIT)
        for picture, words in ((png(320, 240), "not exactly 5:4"), (png(700, 560), "at most 600x480"), (png(300, 240, 2), "no transparency"),
                               (b"\xff\xd8\xff" + bytes(40), "not a PNG")):
            said, in_order = run(listing(entry_with(picture)), download=True, get=lambda url, p=picture: p if url.endswith(".png") else data)
            self.assertFalse(in_order, words)
            self.assertIn(words, said)
        other = entry_with(good)
        said, in_order = run(listing(other), download=True, get=lambda url: png(5, 4) if url.endswith(".png") else data)
        self.assertIn("picture's SHA-256", said)

    def test_a_picture_s_form(self):
        data = pack()
        for picture in ({"path": "a.jpg", "sha256": "0" * 64, "bytes": 5}, {"path": "../a.png", "sha256": "0" * 64, "bytes": 5},
                        {"path": "a.png", "sha256": "0" * 64, "bytes": 100 * 1024 + 1}, {"path": "a.png", "sha256": "0" * 64}):
            said, in_order = run(listing(entry_for(data, picture=picture)))
            self.assertFalse(in_order, picture)
        reserved = {"id": "later", "author": "someone", "status": "reserved", "categories": ["tools"], "description": "Later.",
                    "picture": {"path": "a.png", "sha256": "0" * 64, "bytes": 5}}
        said, in_order = run(listing(reserved))
        self.assertIn("only a listed or a bundled entry has a picture", said)
        followed = followed_for(data, picture={"path": "a.png", "sha256": "0" * 64, "bytes": 5})
        said, in_order = run(listing(followed))
        self.assertFalse(in_order)
        self.assertIn("a followed entry has no picture", said)

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

    def test_with_the_listing_before_a_followed_entry_is_downloaded_only_when_it_changed(self):
        data = pack(version=3)
        old = followed_for(data)
        asked = []

        def get(url):
            asked.append(url)
            return data
        for new in (old, dict(old, version=3, description="Said again.")):    # its file is read again when the page opens, not here
            with self.subTest(new=new):
                said, in_order = run(listing(new), before=listing(old), download=True, get=get)
                self.assertTrue(in_order, said)
                self.assertEqual(asked, [])
        for new, link in ((dict(old, follow="release"), "/refs/heads/release/build/tester.pack"),
                          (dict(old, path="packs/tester.pack"), "/refs/heads/main/packs/tester.pack"),
                          (dict(old, needs=["base"]), "/refs/heads/main/build/tester.pack"),         # what the file is held to changed: held again
                          (dict(old, folders=[{"name": "notes", "place": "notes"}]), "/refs/heads/main/build/tester.pack"),
                          (dict(old, version=2), None),
                          (dict(old, repository="https://github.com/someone/other-pack"), "/other-pack/refs/heads/main/"),
                          (followed_for(data, id="new one"), None)):
            with self.subTest(new=new):
                del asked[:]
                said, in_order = run(listing(new), before=listing(old), download=True, get=get)
                self.assertEqual(len(asked), 1, said)
                if link:
                    self.assertIn(link, asked[0])
                    self.assertTrue(in_order, said)
        del asked[:]
        pinned = entry_for(pack(version=2))
        said, in_order = run(listing(old), before=listing(pinned), download=True, get=get)       # a pinned entry comes to follow
        self.assertTrue(in_order, said)
        self.assertEqual(asked, ["https://raw.githubusercontent.com/someone/some-pack/refs/heads/main/build/tester.pack"])


class AFollowedEntry(unittest.TestCase):
    """An entry that follows a branch of its author's repository: the install page reads its file at the head of the
    branch each time it opens, so a new version needs no pull request."""

    def refused(self, entry, words):
        said, in_order = run(listing(entry))
        self.assertFalse(in_order, said)
        self.assertIn(words, said)

    def test_it_names_its_branch_and_what_the_file_may_ask_for(self):
        entry = followed_for(pack())
        self.assertEqual(sorted(entry), sorted(check.ALWAYS + ("repository", "path", "follow", "version", "firmware", "needs", "folders")))
        said, in_order = run(listing(entry))
        self.assertTrue(in_order, said)
        self.assertIn("ok: tester (listed, follows main)", said)
        for key in ("repository", "path", "version", "firmware", "needs", "folders"):
            with self.subTest(lacks=key):
                self.refused({k: v for k, v in entry.items() if k != key}, "it lacks %s" % key)
        said, in_order = run(listing(dict(entry, status="retired")))
        self.assertTrue(in_order, said)

    def test_it_names_nothing_of_a_pinned_file(self):
        for key, value in (("commit", COMMIT), ("sha256", "0" * 64), ("bytes", 1000), ("places", 0), ("level", 3), ("offers_level", 3)):
            with self.subTest(names=key):
                self.refused(dict(followed_for(pack()), **{key: value}), "a followed entry names no %s" % key)

    def test_only_a_listed_or_a_retired_entry_follows_a_branch(self):
        reserved = {"id": "later", "author": "someone", "status": "reserved", "categories": ["tools"], "description": "Held.", "follow": "main"}
        self.refused(reserved, "only a listed or a retired entry follows a branch")
        bundled = dict(followed_for(pack(), path="builder/tester.pack"), status="bundled")
        del bundled["repository"]
        self.refused(bundled, "only a listed or a retired entry follows a branch")

    def test_the_form_of_a_branch_s_name(self):
        for good in ("main", "release-1.x", "v2_0", "_x", "9", "a" * 100, "lock"):
            with self.subTest(follow=good):
                self.assertEqual(check.entry_faults(followed_for(pack(), follow=good), {"tools": "tools"}, FIRMWARES), [])
        for bad in ("", ".hidden", "-x", "a..b", "ends.", "x.lock", "a/b", "refs/heads/main", "a" * 101, "two words", "é", 5, None, ["main"]):
            with self.subTest(follow=bad):
                self.refused(followed_for(pack(), follow=bad), "its follow is not a branch's name")

    def test_its_link_is_the_head_of_its_branch(self):
        entry = followed_for(pack(), path="packs/two words.pack", follow="v2.x")
        self.assertEqual(check.raw_link(entry), "https://raw.githubusercontent.com/someone/some-pack/refs/heads/v2.x/packs/two%20words.pack")
        self.assertEqual(check.raw_link(entry_for(pack())), "https://raw.githubusercontent.com/someone/some-pack/%s/build/tester.pack" % COMMIT)


class AFollowedFile(unittest.TestCase):
    """The file at the head of a followed entry's branch, held as the install page holds it before it offers it."""

    def fetched(self, data, entry, *others):
        asked = []

        def get(url):
            asked.append(url)
            return data
        said, in_order = run(listing(entry, *others), download=True, get=get)
        self.assertEqual(asked, [check.raw_link(entry)])
        return said, in_order

    def refused(self, data, entry, words, *others):
        said, in_order = self.fetched(data, entry, *others)
        self.assertFalse(in_order, said)
        self.assertIn(words, said)

    def test_a_file_that_keeps_to_its_entry_is_offered(self):
        data = card_pack(version=7)
        said, in_order = self.fetched(data, followed_for(data, version=5, folders=[{"name": "notes", "place": "notes"}]))
        self.assertTrue(in_order, said)
        self.assertIn("ok: tester (listed, follows main, its file held)", said)

    def test_1_at_most_2_mb(self):
        data = pack()
        large = data + bytes(check.LARGEST + 1 - len(data))
        self.refused(large, followed_for(data), "the file has 2097153 bytes, and a pack has at most 2097152")

    def test_2_a_pack_of_format_3_whose_header_is_its_file_s(self):
        data = pack()
        entry = followed_for(data)
        broken = bytearray(data)
        broken[-1] ^= 1
        for other, words in ((b"no pack at all, " * 8, "the file is no pack the loader takes: Not a pack file"),
                             (data + bytes(4), "the file is no pack the loader takes: The header's sizes do not fit the file"),
                             (bytes(broken), "the file is no pack the loader takes: The checksum"),
                             (pb.make(bytes(64), pb.ROOM_START, 64, 0, "tester", OS_173), "the file is a pack of format 1, which the loader no longer takes")):
            with self.subTest(words=words):
                self.refused(other, entry, words)

    def test_3_its_name_is_the_entry_s_id(self):
        self.refused(pack(name="other"), followed_for(pack()), 'the file\'s name is "other", the entry\'s id is "tester"')

    def test_4_it_is_built_for_the_entry_s_firmware(self):
        self.refused(pack(target_os=0x12345678), followed_for(pack()),
                     "the file's firmware is one this repository has no profile for (0x12345678), the entry says 1.7.3")

    def test_5_its_version_is_the_entry_s_at_the_least(self):
        self.refused(pack(version=4), followed_for(pack(), version=5), "the file's version is 4, the listing takes 5 or higher")
        for version in (5, 6):
            with self.subTest(version=version):
                said, in_order = self.fetched(pack(version=version), followed_for(pack(), version=5))
                self.assertTrue(in_order, said)

    def test_6_it_needs_no_pack_its_entry_does_not_name(self):
        data = pack(works=[("base", 2, True), ("other", 1, False)])
        self.refused(data, followed_for(data), 'the file needs "base", which the entry\'s needs do not name')
        said, in_order = self.fetched(data, followed_for(data, needs=["base"]))
        self.assertTrue(in_order, said)

    def test_7_it_needs_no_level_the_listed_base_does_not_offer(self):
        data = pack(imports=[("op1f_level_4", True)])
        self.refused(data, followed_for(data), "the file needs level 4 of the base, and the listing's base offers 3")
        for imports in ([("op1f_level_3", True)], [("op1f_level_9", False)]):     # an optional name is no need
            with self.subTest(imports=imports):
                data = pack(imports=imports)
                said, in_order = self.fetched(data, followed_for(data))
                self.assertTrue(in_order, said)

    def test_2_a_name_longer_than_the_page_takes_is_refused_before_the_file_is_read(self):
        data = pack(imports=[("a" * (check.NAME_MOST + 1), True)] * 50)        # one long name mentioned many times
        self.refused(data, followed_for(data), "a name in its lists is longer than %d characters" % check.NAME_MOST)
        data = pack(imports=[("a" * check.NAME_MOST, False)])
        said, in_order = self.fetched(data, followed_for(data))
        self.assertTrue(in_order, said)

    def test_8_a_list_of_folders_outside_the_body_is_refused(self):
        data = card_pack(folders_at=pb.ROOM_START - pb.FOLDER.size)
        self.refused(data, followed_for(data, folders=[{"name": "notes", "place": "notes"}]),
                     "the file's card lists folders that cannot be read: its list does not lie in its body")

    def test_8_its_card_asks_for_no_folder_its_entry_does_not_name(self):
        data = card_pack(folders=[("notes", "notes"), ("more", "kept")])
        entry = followed_for(data, folders=[{"name": "notes", "place": "notes"}])
        self.refused(data, entry, 'the file\'s card asks for the folder {"name": "more", "place": "kept"}, which the entry\'s folders do not name')
        self.refused(data, dict(entry, folders=[{"name": "notes", "place": "notes"}, {"name": "more", "place": "more"}]),
                     '{"name": "more", "place": "kept"}')
        said, in_order = self.fetched(data, dict(entry, folders=[{"name": "more", "place": "kept"}, {"name": "notes", "place": "notes"},
                                                                 {"name": "spare", "place": "spare"}]))
        self.assertTrue(in_order, said)

    def test_its_card_is_held_as_a_pinned_file_s(self):
        data = card_pack(card_version=1, folders=())
        self.refused(data, followed_for(data), "its card: its version is not the pack's, 2")

    def test_what_cannot_be_read_in_it_is_a_fault_never_a_stop(self):
        data = card_pack(folders_at=pb.ROOM_START + 4096)                 # a list of folders outside the body
        entry = followed_for(data, folders=[{"name": "notes", "place": "notes"}])
        self.refused(data, entry, "the file's card lists folders that cannot be read")
        self.refused(data, entry, "its card cannot be read")

    def test_a_file_in_the_folders_given_is_held_the_same_way(self):
        data = pack(version=4)
        with tempfile.TemporaryDirectory() as folder:
            with open(os.path.join(folder, "tester.pack"), "wb") as f:
                f.write(data)
            said, in_order = run(listing(followed_for(data, version=5)), folders=[folder])
            self.assertFalse(in_order)
            self.assertIn("the file's version is 4, the listing takes 5 or higher", said)
            said, in_order = run(listing(followed_for(data, version=4)), folders=[folder])
            self.assertTrue(in_order, said)
            self.assertIn("ok: tester (listed, follows main, its file held)", said)

    def test_a_retired_followed_entry_is_not_downloaded(self):
        def get(url):
            raise AssertionError("asked for %s" % url)
        said, in_order = run(listing(dict(followed_for(pack()), status="retired")), download=True, get=get)
        self.assertTrue(in_order, said)


def places_pack(lists=((("tape.svg",), False), (("signalflow.svg",), True)), level=6, name="tester", version=2):
    """A pack file with a card of `level` whose asks are asks of places, each (its pictures' names, whether it gives
    way), as the base reads them: by default a firm one and then one that gives way."""
    link, at = pb.ROOM_START, 4
    asks = at + pb.CARD_4_BYTES
    size = pb.TABLE_BYTES_6[5]
    tables = asks + pb.ASK.size * len(lists)
    end = tables + size * len(lists)                            # where the next list of names goes
    body = bytearray(end + sum(pb.PLACE_NAME * len(names) for names, _gives in lists))
    pb.CARD_4.pack_into(body, at, pb.CARD_MARK, pb.CARD_4_BYTES, level, version, name.encode("ascii"), 0, 0, 0, 0, 0, 0,
                        link + asks, len(lists))
    places = [(at + pb.CARD_3_BYTES, pb.KIND_WORD, None, link + asks)]
    for number, (names, gives) in enumerate(lists):
        table = tables + size * number
        pb.ASK.pack_into(body, asks + pb.ASK.size * number, 5, size, link + table)
        pb.KIND_HEAD.pack_into(body, table, size, b"list%d" % number, pb.PLACES_GIVES_WAY if gives else 0, 0)
        struct.pack_into("<2I", body, table + pb.KIND_HEAD.size, len(names), link + end)
        for n, picture in enumerate(names):
            body[end + pb.PLACE_NAME * n:end + pb.PLACE_NAME * n + len(picture)] = picture.encode("ascii")
        places += [(asks + pb.ASK.size * number + 8, pb.KIND_WORD, None, link + table), (table + 32, pb.KIND_WORD, None, link + end)]
        end += pb.PLACE_NAME * len(names)
    return pb.make3(bytes(body), link, len(body), 0, name, OS_173, (link, link), version=version,
                    imports=[("op1f_level_%d" % level, True)], places=places, card_place=at, no_table=True)


def versioned_base(version, level=6, firmware="1.7.3"):
    """A base of that version held to its form, whose file is not at hand: its entry says what its file would."""
    return dict(base_entry(level), sha256="0" * 64, bytes=1000, version=version, firmware=firmware, places=0, needs=[], folders=[])


class ABaseOfTheVersionACardNeeds(unittest.TestCase):
    """What a card asks that a base serves only from one of its versions on (BASE_SINCE): places that give way, which
    base 31 serves and bases 24 to 30 leave aside."""

    SAID = ("REFUSED: tester: the file's card asks for places that give way, which the base serves from its version 31, "
            "and the listing's base for 1.7.3 is version 30")

    def held(self, data, entry, *bases):
        with tempfile.TemporaryDirectory() as folder:
            with open(os.path.join(folder, os.path.basename(entry["path"])), "wb") as f:
                f.write(data)
            return run(dict(listing(), packs=list(bases) + [entry]), folders=[folder])

    def test_the_file_is_read_as_its_card_asks(self):
        self.assertTrue(check.gives_way(places_pack()))
        self.assertEqual(check.base_needed(places_pack()), (31, "places that give way"))
        self.assertTrue(check.gives_way(places_pack(lists=((("signalflow.svg",), True),))))
        for data in (places_pack(lists=((("tape.svg",), False),)), card_pack(), pack()):
            self.assertFalse(check.gives_way(data))
            self.assertIsNone(check.base_needed(data))

    def test_a_pack_that_gives_way_is_listed_beside_base_31_or_later(self):
        data = places_pack()
        entry = entry_for(data, level=6)
        said, in_order = self.held(data, entry, versioned_base(30))
        self.assertFalse(in_order, said)
        self.assertIn(self.SAID, said)
        for version in (31, 32):
            with self.subTest(version=version):
                said, in_order = self.held(data, entry, versioned_base(version))
                self.assertTrue(in_order, said)
                self.assertIn("ok: tester (listed, its file held)", said)

    def test_a_pack_with_firm_places_only_is_listed_beside_any_base(self):
        data = places_pack(lists=((("tape.svg",), False),))
        said, in_order = self.held(data, entry_for(data, level=6), versioned_base(24))
        self.assertTrue(in_order, said)

    def test_a_listing_whose_base_has_no_file_has_no_version(self):
        data = places_pack()
        said, in_order = self.held(data, entry_for(data, level=6), base_entry(6))
        self.assertFalse(in_order, said)
        self.assertIn("and the listing's base for 1.7.3 is none with a file", said)

    def test_the_base_of_the_pack_s_own_firmware_counts_and_no_retired_one(self):
        bases = [versioned_base(31, firmware="1.7.0"), versioned_base(30), dict(versioned_base(32), status="retired")]
        self.assertEqual(check.base_version(bases, (), "1.7.3"), 30)
        self.assertEqual(check.base_version(bases, (), "1.7.0"), 31)
        self.assertEqual(check.base_version(bases, (1,), "1.7.3"), 0)          # an entry out of form is no base

    def test_a_followed_file_that_gives_way_is_held_so_too(self):
        data = places_pack()
        entry = followed_for(data)
        for version, in_order_then in ((30, False), (31, True)):
            with self.subTest(version=version):
                said, in_order = run(dict(listing(), packs=[versioned_base(version), entry]), download=True, get=lambda url: data)
                self.assertEqual(in_order, in_order_then, said)
                if not in_order_then:
                    self.assertIn(self.SAID, said)


def options_pack(level=6, name="tester", version=2, with_places=False):
    """A pack file with a card of `level` whose ask is an ask of options (the pack's settings, base 32) with one switch
    shown in KEYBOARD, as the base reads it; with `with_places`, a firm ask of places and one that gives way before it."""
    link, at = pb.ROOM_START, 4
    lists = ((("tape.svg",), False), (("signalflow.svg",), True)) if with_places else ()
    count = len(lists) + 1
    asks = at + pb.CARD_4_BYTES
    places_size, size = pb.TABLE_BYTES_6[5], pb.TABLE_BYTES_6[pb.OPTIONS]
    tables = asks + pb.ASK.size * count
    table = tables + places_size * len(lists)                   # the options' table, behind those of places
    end = table + size                                          # where the lists of names go
    body = bytearray(end + sum(pb.PLACE_NAME * len(names) for names, _gives in lists))
    pb.CARD_4.pack_into(body, at, pb.CARD_MARK, pb.CARD_4_BYTES, level, version, name.encode("ascii"), 0, 0, 0, 0, 0, 0,
                        link + asks, count)
    places = [(at + pb.CARD_3_BYTES, pb.KIND_WORD, None, link + asks)]
    for number, (names, gives) in enumerate(lists):
        place = tables + places_size * number
        pb.ASK.pack_into(body, asks + pb.ASK.size * number, 5, places_size, link + place)
        pb.KIND_HEAD.pack_into(body, place, places_size, b"list%d" % number, pb.PLACES_GIVES_WAY if gives else 0, 0)
        struct.pack_into("<2I", body, place + pb.KIND_HEAD.size, len(names), link + end)
        for n, picture in enumerate(names):
            body[end + pb.PLACE_NAME * n:end + pb.PLACE_NAME * n + len(picture)] = picture.encode("ascii")
        places += [(asks + pb.ASK.size * number + 8, pb.KIND_WORD, None, link + place), (place + 32, pb.KIND_WORD, None, link + end)]
        end += pb.PLACE_NAME * len(names)
    ask = asks + pb.ASK.size * len(lists)
    pb.ASK.pack_into(body, ask, pb.OPTIONS, size, link + table)
    pb.KIND_HEAD.pack_into(body, table, size, b"mine", 0, 0)
    struct.pack_into("<2I", body, table + pb.KIND_HEAD.size, 0, 1)             # no `changed`, one option
    option = table + pb.KIND_HEAD.size + 8
    body[option:option + 3] = b"dot"
    body[option + 16:option + 20] = bytes((pb.OPTION_SWITCH, 2, 0, pb.OPTION_CATEGORY["keyboard"] << 4))   # a switch, off at first, in KEYBOARD
    places.append((ask + 8, pb.KIND_WORD, None, link + table))
    return pb.make3(bytes(body), link, len(body), 0, name, OS_173, (link, link), version=version,
                    imports=[("op1f_level_%d" % level, True)], places=places, card_place=at, no_table=True)


class ABaseOfVersion32ForOptions(unittest.TestCase):
    """The second row of BASE_SINCE: a card that asks for options, the pack's settings in the unit's settings, which
    base 32 serves and bases before it leave aside (with their line for a kind they do not know)."""

    SAID = ("REFUSED: tester: the file's card asks for options, which the base serves from its version 32, "
            "and the listing's base for 1.7.3 is version %d")

    held = ABaseOfTheVersionACardNeeds.held

    def test_the_file_is_read_as_its_card_asks(self):
        data = options_pack()
        tables = pb.ask_tables(data)
        self.assertEqual([(kind, wrong) for kind, _size, _what, _ask, wrong in tables], [(pb.OPTIONS, None)])   # an ask in order
        self.assertTrue(check.asks_options(data))
        self.assertEqual(check.base_needed(data), (32, "options"))
        self.assertEqual(check.base_needed(options_pack(with_places=True)), (32, "options"))      # the later of the two bases asked for
        for data in (places_pack(), places_pack(lists=((("tape.svg",), False),)), card_pack(), pack()):
            self.assertFalse(check.asks_options(data))

    def test_a_pack_that_asks_for_options_is_listed_beside_base_32_or_later(self):
        data = options_pack()
        entry = entry_for(data, level=6)
        for version in (30, 31):
            with self.subTest(version=version):
                said, in_order = self.held(data, entry, versioned_base(version))
                self.assertFalse(in_order, said)
                self.assertIn(self.SAID % version, said)
        for version in (32, 33):
            with self.subTest(version=version):
                said, in_order = self.held(data, entry, versioned_base(version))
                self.assertTrue(in_order, said)
                self.assertIn("ok: tester (listed, its file held)", said)

    def test_a_followed_file_that_asks_for_options_is_held_so_too(self):
        data = options_pack()
        entry = followed_for(data)
        for version, in_order_then in ((31, False), (32, True)):
            with self.subTest(version=version):
                said, in_order = run(dict(listing(), packs=[versioned_base(version), entry]), download=True, get=lambda url: data)
                self.assertEqual(in_order, in_order_then, said)
                if not in_order_then:
                    self.assertIn(self.SAID % version, said)


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

    def test_fx_and_lfo_are_kept_for_open_field_s_own_packs(self):
        for kept in ("lfo", "LFO"):
            with self.subTest(name=kept):
                said, in_order = run(listing(entry_for(pack(), folders=[{"name": kept, "place": "mine"}])))
                self.assertFalse(in_order)
                self.assertIn("its folder's name %r is kept for open-field's own packs" % kept, said)
        ours = dict(listing(), ours=[REPOSITORY])
        ours["packs"].append(entry_for(pack(name="fx train"), author="open-field", folders=[{"name": "fx", "place": "fx"}]))
        said, in_order = run(ours)
        self.assertTrue(in_order, said)
        ours["packs"].append(entry_for(pack(name="other"), author="open-field", folders=[{"name": "fx", "place": "other"}]))
        said, in_order = run(ours)
        self.assertFalse(in_order)
        self.assertIn("its folder's name 'fx' is held by 'fx train'", said)

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

    def test_a_version_never_goes_down_a_followed_entry_s_lowest_included(self):
        followed = followed_for(pack(), version=5)
        pinned = entry_for(pack(version=5))
        for new, old, words in ((dict(followed, version=4), followed, "its version went down, from 5 to 4"),
                                (followed_for(pack(), version=4), pinned, "its version went down, from 5 to 4"),      # pinned, then followed
                                (entry_for(pack(version=4)), followed, "its version went down, from 5 to 4"),        # followed, then pinned
                                (dict(pinned, version=4), pinned, "its version went down, from 5 to 4")):
            with self.subTest(new=new, old=old):
                said, in_order = run(listing(new), before=listing(old))
                self.assertFalse(in_order)
                self.assertIn(words, said)
        for new, old in ((followed, followed), (dict(followed, version=6), followed), (followed, pinned),
                         (entry_for(pack(version=5)), followed), (entry_for(pack(version=7)), followed)):
            with self.subTest(new=new, old=old):
                said, in_order = run(listing(new), before=listing(old))
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
    def test_every_entry_this_repository_builds_is_the_file_a_build_gives(self):
        with open(check.CATALOG, encoding="utf-8") as f:
            the_listing = json.load(f)
        # a listed pack's file is built in its own repository: .github/workflows/listing.yml downloads and holds it
        the_listing["packs"] = [entry for entry in the_listing["packs"] if entry["status"] != "listed"]
        said, in_order = run(copy.deepcopy(the_listing), folders=built_folders())
        self.assertTrue(in_order, said)
        self.assertEqual(said.count("its file held"), sum(1 for e in the_listing["packs"] if e.get("sha256") and e["status"] != "retired"))


if __name__ == "__main__":
    unittest.main()

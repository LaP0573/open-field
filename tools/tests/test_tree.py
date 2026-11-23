"""This repository against itself: the profile, the tables, the recipes, the example packs, the pages, the builder's
manifest and the build.

  python -I -m unittest discover -s tools/tests -p "test_tree.py"

What needs nothing but the tree runs anywhere. Two things need more and are skipped without it:

- your own firmware file, named by the environment's OPEN_FIELD_STOCK: every place's checksum in the profile is then
  held against the bytes of that file. No byte of the firmware is kept in this repository, so the expected values
  are read from your file and never written down here;
- the toolchain in open-xy/ (docs/build.md): everything is then built and held to the kept sizes and hashes.
"""

import hashlib
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOOLS = os.path.join(ROOT, "tools")
STOCK = os.environ.get("OPEN_FIELD_STOCK", "")
TOOLCHAIN = os.path.join(ROOT, "open-xy", "local", "toolchain", "bin")
PACKS = ("base",)                                                            # the base, base.pack: a feature lives in a repository of its own
EXAMPLES = ("crush", "dub", "echo", "fold", "list", "ping", "pluck", "sketch", "wink", "yoyo")      # the examples with a card and no place of the firmware
LEVELS = {"wink": 1, "yoyo": 2, "list": 3, "sketch": 4, "echo": 5, "dub": 7, "crush": 10, "ping": 10, "pluck": 10, "fold": 10}      # the level each card asks for


def tool(name):
    spec = importlib.util.spec_from_file_location("tree_" + name, os.path.join(TOOLS, name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8") as f:
        return json.load(f)


PROFILE = load("build.json")["profile"]                                         # the tree's one profile: profiles/op1f-<the firmware>.json


def text(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8", newline="") as f:
        return f.read()


class TheProfile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profile = load(*PROFILE.split("/"))
        cls.places = {place["address"]: place for place in cls.profile["patches"]}

    def test_it_holds_no_byte_of_the_firmware(self):
        """No field for the bytes a place holds, and no long run of hex but the firmware file's SHA-256."""
        written = text(*PROFILE.split("/"))
        self.assertNotIn('"before"', written)
        self.assertNotIn('"after"', written)
        runs = [run for run in re.findall(r"[0-9a-fA-F]{12,}", written) if run != self.profile["stock_sha256"]]
        self.assertEqual(runs, [])

    def test_each_place_stands_once_with_an_address_a_length_a_sum_and_words(self):
        self.assertEqual(len(self.places), len(self.profile["patches"]))
        for place in self.profile["patches"]:
            self.assertRegex(place["address"], r"^0x[0-9a-f]{8}$")
            self.assertIn(place["length"], (4, 8))
            self.assertRegex(place["sum"], r"^0x[0-9a-f]{8}$")
            self.assertGreater(len(place["name"]), 20, place["address"])
            self.assertTrue(place["group"])

    def test_every_table_names_places_of_the_profile_and_every_place_is_used(self):
        used = {item["address"] for item in self.profile["image"]}
        for table in self.profile["tables"]:
            for entry in table["entries"]:
                self.assertIn(entry["address"], self.places, table["file"])
                self.assertTrue(entry["does"], entry["address"])
                used.add(entry["address"])
        self.assertEqual(used, set(self.places))

    def test_the_table_sources_are_what_the_profile_gives(self):
        builder = tool("table_build")
        for table in self.profile["tables"]:
            self.assertEqual(text(*table["file"].split("/")), builder.render(self.profile, table, PROFILE), table["file"])

    def test_a_table_holds_a_checksum_in_every_entry_and_no_stock_word(self):
        """Every entry's kind has the bit that says "a checksum stands here"; its last word is a room's name or 0."""
        line = re.compile(r"^\s*\.long\s+(0x[0-9a-f]{8}),\s*(0x[0-9a-f]{8}),\s*(\w+),\s*(0x[0-9a-f]{8}),\s*(\w+)\s*/\*")
        for table in self.profile["tables"]:
            entries = [line.match(row) for row in text(*table["file"].split("/")).split("\n") if row.startswith("    .long 0x") and "never read" not in row]
            self.assertEqual(len(entries), len(table["entries"]), table["file"])
            for found, entry in zip(entries, table["entries"]):
                self.assertIsNotNone(found)
                kind = int(found.group(1), 16)
                self.assertTrue(kind & 0x10, entry["address"])
                self.assertEqual(found.group(2), entry["address"])
                self.assertEqual(found.group(3), entry["target"])
                self.assertEqual(found.group(5), entry.get("room", "0"))
                self.assertEqual(kind >> 16, entry.get("copies", 0))

    def test_it_says_what_the_builder_changes_in_the_file(self):
        """`image`: the changes the builder makes in the firmware file itself, each a new number for a load or a call
        of a function of the loader. With `entry` and the record of zeros the room for our code lies in, it is what
        builder/write-manifest.mjs makes a manifest from."""
        for field in ("entry", "heap_fill_base", "module_base"):
            self.assertRegex(self.profile[field], r"^0x[0-9a-f]{8}$", field)
        low, room = int(self.profile["module_base"], 16), self.profile["reserved_bytes"]
        fill, filled = int(self.profile["heap_fill_base"], 16), self.profile["heap_fill_bytes"]
        self.assertTrue(fill <= low and low + room <= fill + filled)
        kinds = []
        for item in self.profile["image"]:
            self.assertIn(item["address"], self.places)
            self.assertEqual(self.places[item["address"]]["group"], "image")
            self.assertTrue(item["does"])
            kinds.append(item["kind"])
            if item["kind"] == "imm32":
                self.assertRegex(item["new_imm"], r"^0x[0-9a-f]{8}$")
            else:
                self.assertEqual(item["kind"], "call")
                self.assertEqual(item["length"], self.places[item["address"]]["length"])
                self.assertRegex(item["target"], r"^[A-Za-z_]\w*$")
        self.assertEqual(sorted(kinds), ["call", "imm32"])

    def test_the_change_list_is_what_the_profile_gives(self):
        self.assertEqual(text("docs", "changes.md"), tool("change_list").render(self.profile))

    def test_the_change_list_names_every_place(self):
        page = text("docs", "changes.md")
        for address in self.places:
            self.assertIn("`%s`" % address, page)


class TheRecipes(unittest.TestCase):
    def test_each_recipe_names_files_that_are_there(self):
        for name in PACKS:
            recipe = load("packs", name + ".json")
            self.assertEqual(recipe["name"], name)
            for source in recipe["sources"]:
                self.assertTrue(os.path.isfile(os.path.join(ROOT, *source.split("/"))), source)
            for picture in recipe["pictures"]:
                self.assertTrue(os.path.isfile(os.path.join(ROOT, *picture["file"].split("/"))), picture["file"])

    def test_every_source_is_built_into_something(self):
        """Every source in module/ is one a recipe or the loader names, or one such a source includes (a unit compiled
        twice, once as it is and once inside another with a switch defined, as the train's page is for lfo)."""
        plan = load("build.json")
        used = set(plan["loader"]["sources"])
        for name in plan["packs"]:
            used.update(load("packs", name + ".json")["sources"])
        for path in list(used):
            if path.endswith(".cpp") and os.path.isfile(os.path.join(ROOT, *path.split("/"))):
                used.update("module/" + name for name in re.findall(r'^\s*#\s*include\s+"([^"]+\.cpp)"', text(*path.split("/")), re.M))
        there = {"module/" + name for name in os.listdir(os.path.join(ROOT, "module")) if not name.endswith(".h")}
        self.assertEqual(there, used)

    def test_the_tree_builds_the_base_and_no_feature(self):
        """A feature is a pack in a repository of its own, the project's four in open-field-packs: no recipe, table,
        place or kept file of one is here, and the base works with none of them."""
        self.assertEqual(load("build.json")["packs"], list(PACKS))
        self.assertEqual(sorted(os.listdir(os.path.join(ROOT, "packs"))), ["base.json", "built.json"])
        self.assertEqual({table["part"] for table in load(*PROFILE.split("/"))["tables"]}, {"loader", "base"})
        self.assertEqual({place["group"] for place in load(*PROFILE.split("/"))["patches"]}, {"image", "loader", "base"})
        self.assertEqual([name for name in load("packs", "built.json")["files"] if name.endswith(".pack") and "/" not in name], ["base.pack"])
        base = load("packs", "base.json")
        self.assertEqual((base["works_with"], base["serves"], base["marks"]), ([], [], []))

    def test_the_pages_for_pack_authors_point_at_the_packs_repository(self):
        for page in ("README.md", "AGENTS.md", "docs/first-pack.md", "docs/base.md"):
            self.assertIn("(https://github.com/", text(*page.split("/")), page)
            self.assertIn("/open-field-packs", text(*page.split("/")), page)

    def test_the_base_offers_its_release_and_the_marks_of_its_levels(self):
        base = load("packs", "base.json")
        self.assertRegex(base["release"], r"^[0-9A-Za-z._-]{1,8}$")
        self.assertIn("module/build_r6d_base_release.s", base["sources"])
        for mark in ["op1f_cards"] + ["op1f_level_%d" % level for level in range(1, 11)]:
            self.assertIn(mark, base["exports"])
        self.assertFalse(base["named"])                         # the start screen shows its release, not its name

    def test_every_picture_is_a_set_of_instructions_and_not_a_picture(self):
        """A pack carries how the unit is to put a picture together, never the picture."""
        for folder, _folders, names in os.walk(os.path.join(ROOT, "pictures")):
            for name in names:
                self.assertTrue(name.endswith(".make"), name)
                with open(os.path.join(folder, name), "rb") as f:
                    self.assertTrue(f.read(16).startswith(b"op1f picture "), name)


class TheExamplesWithACard(unittest.TestCase):
    """examples/wink/ to examples/fold/: each a pack in a folder of its own with a card of the level it shows (1 to 10),
    which changes no place of the firmware and needs the base; an engine's sounds beside its recipe."""

    @classmethod
    def setUpClass(cls):
        cls.plan = {example["name"]: example for example in load("build.json")["examples"]}
        cls.built = load("packs", "built.json")["files"]

    def test_the_build_names_each_and_its_files_are_there(self):
        for name in EXAMPLES:
            self.assertEqual(self.plan[name], {"name": name, "recipe": "examples/%s/recipe.json" % name})       # no profile: no place of the firmware
            recipe = load("examples", name, "recipe.json")
            self.assertEqual(recipe["name"], name)
            pictures = ["examples/%s/%s_pictures.s" % (name, name)] if recipe["pictures"] else []         # an example with no picture of its own has no directory
            self.assertEqual(recipe["sources"], ["examples/%s/%s.cpp" % (name, name), "examples/%s/%s_card.s" % (name, name)] + pictures)
            sounds = ["examples/%s/%s" % (name, sound["file"]) for ask in recipe["card"].get("asks") or [] for sound in ask.get("sounds") or []]
            for path in recipe["sources"] + [picture["file"] for picture in recipe["pictures"]] + sounds + ["examples/%s/README.md" % name]:
                self.assertTrue(os.path.isfile(os.path.join(ROOT, *path.split("/"))), path)
            self.assertIn("examples/%s.pack" % name, self.built)

    def test_each_has_a_card_of_its_level_and_needs_nothing_but_the_base(self):
        for name in EXAMPLES:
            recipe = load("examples", name, "recipe.json")
            self.assertEqual(recipe["card"]["level"], LEVELS[name], name)
            self.assertEqual((recipe["works_with"], recipe["serves"], recipe["exports"], recipe["marks"]), ([], [], [], []), name)
            card = text("examples", name, name + "_card.s")
            self.assertIn("op1f_level_%d" % LEVELS[name], card, name)       # the card makes its level's mark a name the pack needs
        self.assertEqual(load("examples", "list", "recipe.json")["card"]["folders"], [{}])      # one folder, its name and place the pack's own name
        for name in ("wink", "yoyo"):
            self.assertNotIn("folders", load("examples", name, "recipe.json")["card"])       # and none

    def test_each_picture_is_a_set_of_instructions(self):
        for name in EXAMPLES:
            for picture in load("examples", name, "recipe.json")["pictures"]:
                self.assertTrue(picture["file"].endswith(".make"), picture["file"])
                with open(os.path.join(ROOT, *picture["file"].split("/")), "rb") as f:
                    self.assertTrue(f.read(16).startswith(b"op1f picture "), picture["file"])


class TheExamplePack(unittest.TestCase):
    """examples/trampolines/: a pack in a folder of its own, with its own recipe and its own profile: the example of
    a pack that changes places of the firmware."""

    @classmethod
    def setUpClass(cls):
        cls.plan = [example for example in load("build.json")["examples"] if example["name"] == "trampolines"]
        cls.recipe = load("examples", "trampolines", "recipe.json")
        cls.profile = load("examples", "trampolines", "profile.json")

    def test_the_build_names_it_and_its_files_are_there(self):
        self.assertEqual(self.plan, [{"name": "trampolines", "recipe": "examples/trampolines/recipe.json", "profile": "examples/trampolines/profile.json"}])
        self.assertEqual(self.recipe["name"], "trampolines")
        for source in self.recipe["sources"]:
            self.assertTrue(source.startswith("examples/trampolines/"), source)
            self.assertTrue(os.path.isfile(os.path.join(ROOT, *source.split("/"))), source)
        self.assertIn("examples/trampolines.pack", load("packs", "built.json")["files"])

    def test_it_needs_no_other_pack_and_offers_nothing(self):
        self.assertEqual((self.recipe["works_with"], self.recipe["serves"], self.recipe["exports"], self.recipe["marks"]), ([], [], [], []))

    def test_its_profile_is_for_the_same_firmware_file_and_holds_no_byte_of_it(self):
        main = load(*PROFILE.split("/"))
        self.assertEqual(self.profile["stock_sha256"], main["stock_sha256"])
        written = text("examples", "trampolines", "profile.json")
        self.assertNotIn('"before"', written)
        self.assertEqual([run for run in re.findall(r"[0-9a-fA-F]{12,}", written) if run != main["stock_sha256"]], [])
        taken = {place["address"] for place in main["patches"]}
        for place in self.profile["patches"]:
            self.assertNotIn(place["address"], taken)           # the example stands where no feature does
            self.assertGreater(len(place["name"]), 20)

    def test_its_table_is_what_its_profile_gives_and_is_one_of_its_sources(self):
        builder = tool("table_build")
        tables = self.profile["tables"]
        self.assertEqual(len(tables), 1)
        self.assertIn(tables[0]["file"], self.recipe["sources"])
        self.assertEqual(text(*tables[0]["file"].split("/")), builder.render(self.profile, tables[0], "examples/trampolines/profile.json"))
        rooms = [entry["room"] for entry in tables[0]["entries"]]
        source = text("examples", "trampolines", "trampolines.s")
        for entry in tables[0]["entries"]:
            self.assertIn("%s:" % entry["target"], source)
            self.assertIn("%s:" % entry["room"], source)
        self.assertEqual(len(set(rooms)), len(rooms))

    def test_each_room_is_the_bytes_its_trampoline_copies_and_eight_more(self):
        source = text("examples", "trampolines", "trampolines.s")
        for entry in self.profile["tables"][0]["entries"]:
            found = re.search(r"^%s:\s*\n\s*\.space\s+(\d+)" % re.escape(entry["room"]), source, re.M)
            self.assertIsNotNone(found, entry["room"])
            self.assertEqual(int(found.group(1)), entry["copies"] + 8)


PAGES = ("README.md", "CONTRIBUTING.md", "AGENTS.md", "NOTICE.md", "THIRD-PARTY.md")
LINK = re.compile(r"\[[^\]\n]*\]\(([^)\s]+)\)")
NAMED = re.compile(r"`((?:tools|docs|module|packs|examples|profiles|builder|linker|hooks|pictures)/[^`\s]*)[^`]*`")


def pages():
    """Every page of the tree written for people: the ones at the top, docs/, and a README in a folder."""
    out = [name for name in PAGES if os.path.isfile(os.path.join(ROOT, name))]
    for folder, folders, names in os.walk(ROOT):
        folders[:] = sorted(name for name in folders if name not in (".git", "open-xy", "build", "out") and not name.startswith("out-"))
        if folder == ROOT:
            continue
        for name in sorted(names):
            if name.endswith(".md"):
                out.append(os.path.relpath(os.path.join(folder, name), ROOT).replace("\\", "/"))
    return out


def anchors(path):
    """The anchors of a page's headings, as a hosting site makes them: small letters, the signs taken out, a hyphen
    for each space."""
    out = set()
    for line in text(*path.split("/")).split("\n"):
        if line.startswith("#"):
            title = line.lstrip("#").strip().lower()
            out.add(re.sub(r"\s", "-", re.sub(r"[^\w\s-]", "", title)))
    return out


class ThePages(unittest.TestCase):
    def test_the_pages_for_those_who_write_a_pack_are_there(self):
        there = pages()
        for page in ("docs/first-pack.md", "docs/packs.md", "docs/base.md", "docs/methods.md", "docs/firmware-interfaces.md", "AGENTS.md",
                     "docs/guide.md", "docs/build.md", "docs/changes.md", "docs/checking.md", "examples/trampolines/README.md") \
                + tuple("docs/level-%d.md" % level for level in range(4, 11)) + tuple("examples/%s/README.md" % name for name in EXAMPLES):
            self.assertIn(page, there)
        start = text("README.md")
        for page in ("docs/first-pack.md", "docs/packs.md", "docs/base.md", "docs/methods.md", "docs/firmware-interfaces.md", "CONTRIBUTING.md", "AGENTS.md"):
            self.assertIn("(%s)" % page, start, page)

    def test_every_link_leads_to_a_file_of_the_tree_and_every_anchor_to_a_heading(self):
        seen = 0
        for page in pages():
            for target in LINK.findall(text(*page.split("/"))):
                if target.startswith(("http://", "https://", "mailto:")):
                    continue
                seen += 1
                path, _hash, anchor = target.partition("#")
                where = os.path.normpath(os.path.join(os.path.dirname(page), path)).replace("\\", "/") if path else page
                self.assertFalse(where.startswith(".."), "%s links out of the tree: %s" % (page, target))
                self.assertTrue(os.path.exists(os.path.join(ROOT, *where.split("/"))), "%s links to %s, which is not there" % (page, target))
                if anchor:
                    self.assertIn(anchor, anchors(where), "%s links to %s, and %s has no such heading" % (page, target, where))
        self.assertGreater(seen, 60)

    def test_every_file_a_page_names_is_there(self):
        """A tool, a source or a page named in a page by its path from the top of the tree."""
        seen = 0
        for page in pages():
            for named in NAMED.findall(text(*page.split("/"))):
                if any(sign in named for sign in "*<>") or named.endswith(".pack"):      # a pack file is built, never kept in the tree
                    continue
                seen += 1
                self.assertTrue(os.path.exists(os.path.join(ROOT, *named.rstrip("/.,:;").split("/"))), "%s names %s, which is not there" % (page, named))
        self.assertGreater(seen, 100)

    def test_the_rules_for_coding_assistants_say_what_is_never_done(self):
        rules = text("AGENTS.md")
        for said in ("## The three rules", "## Writing about the firmware", "## Never", "Never add a firmware file", "Never push", "Never touch a bootloader",
                     "Never change a unit without its owner", "python -I tools/build_all.py", "python -I -m unittest discover -s tools/tests",
                     "docs/methods.md", "outside the repository"):
            self.assertIn(said, rules, said)
        self.assertEqual(text("CLAUDE.md").strip(), "@AGENTS.md")

    def test_the_pages_say_which_firmware_can_use_this_and_what_an_update_does(self):
        for page in ("README.md", "docs/guide.md"):
            words = " ".join(text(*page.split("/")).split())
            self.assertIn("later OS version", words, page)
            self.assertIn("official update", words.lower(), page)

    def test_the_page_of_the_firmware_s_functions_is_what_the_sources_and_the_words_give(self):
        finder = tool("interfaces")
        found = finder.bindings()
        self.assertEqual(text("docs", "firmware-interfaces.md"), finder.render(found, finder.read_words()))
        words = finder.read_words()["functions"]
        self.assertGreater(len(words), 35)                       # the words for what only a feature calls are in that feature's repository
        for address, held in words.items():
            self.assertRegex(address, r"^0x[0-9a-f]{8}$")
            self.assertIn(int(address, 16), found, "%s is described and no source binds it" % address)
            self.assertGreater(len(held["what"]), 15, address)
            self.assertTrue(held["thread"], address)

    def test_the_example_s_function_of_the_firmware_is_on_that_page(self):
        """The example calls one function of the firmware, the logger: the page describes it."""
        self.assertIn("| `0x8c053a20` | `void log(", text("docs", "firmware-interfaces.md"))
        self.assertIn("0x8c053a20u", text("examples", "trampolines", "trampolines.cpp"))

    def test_the_words_for_the_functions_are_the_profile_s_own(self):
        """The words beside the profile build.json names, for the same firmware."""
        self.assertEqual(os.path.relpath(tool("interfaces").WORDS, ROOT).replace("\\", "/"), PROFILE[:-len(".json")] + "-interfaces.json")


MANIFEST = re.compile(r"^loader-[0-9]+(\.[0-9]+)*-v[0-9]+\.json$")


class TheInstaller(unittest.TestCase):
    """builder/: the manifests and the loader's files they name. The loader is our small piece of code inside the
    firmware file. `loader.json` is the version the builder's page offers by itself. A manifest
    `loader-<os>-<version>.json` is a version kept beside it, for someone who asks for it by name. Each is made by
    builder/write-manifest.mjs."""

    @classmethod
    def setUpClass(cls):
        cls.manifests = {name: load("builder", name) for name in sorted(os.listdir(os.path.join(ROOT, "builder")))
                         if name == "loader.json" or MANIFEST.match(name)}
        cls.profile = load(*PROFILE.split("/"))

    def test_the_page_s_own_manifest_is_there(self):
        self.assertIn("loader.json", self.manifests)

    def test_each_manifest_names_a_loader_s_file_that_is_there(self):
        for name, manifest in self.manifests.items():
            loader = manifest["loader"]
            self.assertEqual(loader["file"], "loader-%s-%s.bin" % (manifest["os"], loader["version"]), name)
            if name != "loader.json":
                self.assertEqual(name, "loader-%s-%s.json" % (manifest["os"], loader["version"]))
            with open(os.path.join(ROOT, "builder", loader["file"]), "rb") as f:
                data = f.read()
            self.assertEqual(len(data), loader["bytes"], name)
            self.assertEqual(hashlib.sha256(data).hexdigest(), loader["sha256"], name)
            self.assertLessEqual(len(data), 0x10000, name)      # the loader alone, in its 64 KiB: the packs are files on the unit
            self.assertTrue(loader["status"], name)             # the line the page shows: whether its firmware file has run on a unit

    def test_no_version_has_two_manifests_and_no_loader_s_file_is_without_one(self):
        versions = [(manifest["os"], manifest["loader"]["version"]) for manifest in self.manifests.values()]
        self.assertEqual(len(versions), len(set(versions)))
        there = {name for name in os.listdir(os.path.join(ROOT, "builder")) if name.endswith(".bin")}
        self.assertEqual(there, {manifest["loader"]["file"] for manifest in self.manifests.values()})

    def test_the_loader_s_file_this_tree_builds_has_a_manifest(self):
        """build.json names the loader's file the sources give today. One manifest names it, with the size and the
        SHA-256 packs/built.json keeps for it, and with the size of the firmware file the builder gives: the firmware
        file it starts from, one record's header, and the loader's file."""
        blob = load("build.json")["blob"]
        naming = [name for name, manifest in self.manifests.items() if manifest["loader"]["file"] == blob]
        self.assertEqual(len(naming), 1, "the manifests that name %s" % blob)
        manifest = self.manifests[naming[0]]
        kept = load("packs", "built.json")["files"][blob]
        self.assertEqual((kept["bytes"], kept["sha256"]), (manifest["loader"]["bytes"], manifest["loader"]["sha256"]))
        self.assertEqual((kept["bytes"], kept["sha256"]), (load("packs", "built.json")["files"]["loader.bin"]["bytes"], load("packs", "built.json")["files"]["loader.bin"]["sha256"]))
        self.assertEqual(manifest["output"]["bytes"], self.profile["stock_bytes"] + 16 + manifest["loader"]["bytes"])

    def test_each_manifest_is_for_the_firmware_file_the_profile_is_for(self):
        for name, manifest in self.manifests.items():
            self.assertEqual(manifest["stock"]["sha256"], self.profile["stock_sha256"], name)
            self.assertEqual(manifest["stock"]["bytes"], self.profile["stock_bytes"], name)
            self.assertEqual([(edit["address"], edit["kind"]) for edit in manifest["edits"]],
                             [(item["address"], item["kind"]) for item in self.profile["image"]], name)

    def test_the_two_tools_say_the_same_of_a_version_nobody_has_tried(self):
        """A version whose manifest says anything else has run on a unit, and neither tool writes over it."""
        line = '"not run on a unit yet"'
        self.assertIn("const NOT_RUN = %s;" % line, text("builder", "write-manifest.mjs"))
        self.assertIn("NOT_RUN = %s" % line, text("tools", "build_all.py"))

    def test_a_manifest_holds_no_byte_of_the_firmware_and_has_plain_line_ends(self):
        for name in self.manifests:
            written = text("builder", name)
            self.assertNotIn('"before"', written, name)
            self.assertNotIn('"after"', written, name)
            self.assertEqual([run for run in re.findall(r"[0-9a-f]{16,}", written) if len(run) != 64], [], name)
            self.assertNotIn("\r", written, name)
            self.assertEqual(written, json.dumps(json.loads(written), indent=2) + "\n", name)       # as the tool writes it

    def test_no_firmware_file_is_in_the_tree(self):
        for folder, folders, names in os.walk(ROOT):
            folders[:] = [name for name in folders if name not in (".git", "open-xy", "build", "out")]
            for name in names:
                self.assertFalse(name.lower().endswith((".ldr", ".tfw")), os.path.join(folder, name))


class ThePageSBase(unittest.TestCase):
    """builder/base.pack: the base the install page puts on every unit it installs, and builder/base-pack.json, the
    size and SHA-256 the page holds it to. The one finished pack file in the tree; the build holds it to
    packs/built.json and to what it builds."""

    @classmethod
    def setUpClass(cls):
        with open(os.path.join(ROOT, "builder", "base.pack"), "rb") as f:
            cls.data = f.read()
        cls.named_text = text("builder", "base-pack.json")
        cls.named = json.loads(cls.named_text)

    def test_it_is_the_file_base_pack_json_names_and_packs_built_json_keeps(self):
        held = {"bytes": len(self.data), "sha256": hashlib.sha256(self.data).hexdigest()}
        self.assertEqual({key: self.named[key] for key in held}, held)
        self.assertEqual(load("packs", "built.json")["files"]["base.pack"], held)
        self.assertEqual(self.named["file"], "base.pack")

    def test_what_base_pack_json_says_is_what_the_file_says_of_itself(self):
        pack_build = tool("pack_build")
        fields, _body = pack_build.parse(self.data)
        self.assertEqual(fields["name"], "base")
        self.assertEqual(fields["version"], self.named["version"])
        self.assertEqual(pack_build.release_in(self.data), self.named["release"])
        recipe = load("packs", "base.json")
        self.assertEqual((recipe["version"], recipe["release"]), (self.named["version"], self.named["release"]))

    def test_base_pack_json_is_what_the_build_writes_and_holds_no_byte_of_the_firmware(self):
        build_all = tool("build_all")
        record = {"about": self.named["about"], "file": "base.pack"}
        record.update(build_all.base_facts(self.data))
        self.assertEqual(self.named_text, json.dumps(record, indent=2) + "\n")
        self.assertEqual([run for run in re.findall(r"[0-9a-f]{16,}", self.named_text) if len(run) != 64], [])

    def test_it_is_the_one_pack_file_in_the_tree_and_git_keeps_it(self):
        found = []
        for folder, folders, names in os.walk(ROOT):
            folders[:] = [name for name in folders if name not in (".git", "open-xy", "build", "out") and not name.startswith("out-")]
            found += [os.path.relpath(os.path.join(folder, name), ROOT).replace("\\", "/") for name in names if name.endswith(".pack")]
        self.assertEqual(found, ["builder/base.pack"])
        ignored = text(".gitignore").split("\n")
        self.assertIn("*.pack", ignored)
        self.assertIn("!/builder/base.pack", ignored)
        self.assertGreater(ignored.index("!/builder/base.pack"), ignored.index("*.pack"))


class ThePagesOnAHost(unittest.TestCase):
    """What serving builder/ from a host's pages needs of the files: the page loads only files beside it, by relative
    paths, and nothing from another site. A link the owner may follow (an <a> to another site) is no load. The folder
    served is the one the pages workflow uploads."""

    SCRIPT = re.compile(r"<script\b[^>]*>(.*?)</script>", re.S | re.I)
    TAG = re.compile(r"<(\w+)\b([^>]*)>", re.S)
    ATTRIBUTE = re.compile(r"\b(src|href|action|data|poster|srcset)\s*=\s*([\"'])(.*?)\2", re.I | re.S)
    IMPORT = re.compile(r"\bimport\s*(?:[\w*{}\s,]+\s*from\s*)?\(?\s*([\"'`])(.*?)\1")
    LOAD = re.compile(r"\b(fetch|importScripts|new\s+URL|new\s+Worker|new\s+EventSource|new\s+WebSocket)\s*\(\s*([\"'`])(.*?)\2")
    CSS_URL = re.compile(r"url\(\s*([\"']?)(.*?)\1\s*\)")

    @classmethod
    def setUpClass(cls):
        workflow = text(".github", "workflows", "pages.yml")
        cls.served = re.search(r"^\s*path:\s*(\S+)\s*$", workflow, re.M).group(1).strip("'\"/")
        cls.folder = os.path.join(ROOT, *cls.served.split("/"))

    def page_files(self):
        """index.html and every module it imports, as the browser reaches them: {name in the served folder: text}."""
        seen, todo = {}, ["index.html"]
        while todo:
            name = todo.pop()
            if name in seen:
                continue
            seen[name] = text(self.served, *name.split("/"))
            code = "\n".join(self.SCRIPT.findall(seen[name])) if name.endswith(".html") else seen[name]
            for _quote, target in self.IMPORT.findall(code):
                self.assertTrue(target.startswith("./"), "%s imports %s" % (name, target))
                todo.append(os.path.normpath(os.path.join(os.path.dirname(name), target)).replace("\\", "/"))
        return seen

    def test_the_served_folder_is_builder_and_holds_the_page(self):
        self.assertEqual(self.served, "builder")
        self.assertTrue(os.path.isfile(os.path.join(self.folder, "index.html")))

    def test_every_module_the_page_imports_is_beside_it(self):
        files = self.page_files()
        self.assertEqual(sorted(files), ["catalog.mjs", "door.mjs", "ferret.mjs", "index.html", "install.mjs", "ldr.mjs", "packs-live.mjs", "patch.mjs", "placement.mjs"])

    def test_the_page_names_no_absolute_path_and_loads_nothing_from_another_site(self):
        for name, content in self.page_files().items():
            code = "\n".join(self.SCRIPT.findall(content)) if name.endswith(".html") else content
            code = re.sub(r"(?m)^\s*//.*$", "", code)                                   # a comment loads nothing
            for _kind, _quote, target in self.LOAD.findall(code):
                self.assertRegex(target, r"^\./", "%s loads %s" % (name, target))
            if not name.endswith(".html"):
                continue
            for tag, attributes in self.TAG.findall(content):
                for attribute, _quote, value in self.ATTRIBUTE.findall(attributes):
                    if tag.lower() == "a" and attribute.lower() == "href" and value.startswith("https://"):
                        continue                                                          # a link to follow, not a load
                    self.assertNotRegex(value, r"^(/|[a-z][a-z0-9+.-]*:)", "%s: <%s %s=%s>" % (name, tag, attribute, value))
            for _quote, value in self.CSS_URL.findall(content):
                self.assertNotRegex(value, r"^(/|[a-z][a-z0-9+.-]*:)", "%s: url(%s)" % (name, value))
            self.assertNotRegex(content, r"<link\b[^>]*\brel\s*=\s*[\"']?(stylesheet|preload|modulepreload|import)", name)

    def test_the_one_load_from_another_site_is_a_listed_pack_at_its_commit(self):
        """The page downloads a pack the owner ticks from its author's repository, at the commit the listing names
        (raw.githubusercontent.com, which answers any page), and holds it to the entry's size and SHA-256 before it is
        used (docs/catalog.md); a followed entry's file comes from the head of its branch on the same site, once, when
        the page opens, and is held to its entry there. That is its only load from another site: one fetch of a built
        address, in catalog.mjs's download, of rawLink's address, which begins with the one absolute address the
        page's code holds."""
        built, addresses = [], []
        for name, content in self.page_files().items():
            code = "\n".join(self.SCRIPT.findall(content)) if name.endswith(".html") else content
            code = re.sub(r"(?m)^\s*//.*$", "", code)
            for found in re.finditer(r"\bfetch\s*\(\s*(\S)", code):
                if found.group(1) not in "\"'`":                                      # a literal is held to ./ above
                    built.append((name, code[found.start():found.start() + 19]))
            addresses += [(name, value) for value in re.findall(r"[\"'`]((?:https?:)?//[^\"'`]*)", code)]
        self.assertEqual(built, [("catalog.mjs", "fetch(url, options)")])
        self.assertEqual(addresses, [("catalog.mjs", "https://raw.githubusercontent.com/")])
        catalog = text(self.served, "catalog.mjs")
        for said in ('export const RAW = "https://raw.githubusercontent.com/"',
                     'return RAW + found[1] + "/" + found[2] + "/" + entry.commit + "/"',
                     'return RAW + found[1] + "/" + found[2] + "/refs/heads/" + branchOf(entry) + "/"',
                     "const url = rawLink(entry);",
                     'credentials: "omit", redirect: "error"',
                     "if (bytes.length !== entry.bytes) throw",
                     "if (digest !== entry.sha256) throw"):
            self.assertIn(said, catalog)

    def test_every_file_the_page_fetches_by_name_is_beside_it(self):
        manifest = load(self.served, "loader.json")
        named = load(self.served, "base-pack.json")
        for name in ("loader.json", "base-pack.json", manifest["loader"]["file"], named["file"], "open-field-helper.ps1", "open-field-helper.py"):
            self.assertTrue(os.path.isfile(os.path.join(self.folder, name)), name)
        page = text(self.served, "index.html")
        for said in ('get("loader.json")', 'get("base-pack.json")', 'new URL("./open-field-helper.ps1", location.href)',
                     'new URL("./open-field-helper.py", location.href)', 'fetch("./" + LISTING_FILE)'):
            self.assertIn(said, page)
        # the listing of packs, packs.json, is put beside the page by the pages workflow, from catalog/packs.json
        self.assertIn('export const LISTING_FILE = "packs.json"', text(self.served, "catalog.mjs"))
        self.assertRegex(text(".github", "workflows", "pages.yml"), r"(?m)^\s*run: cp catalog/packs\.json %s/packs\.json\s*$" % re.escape(self.served))
        self.assertTrue(os.path.isfile(os.path.join(ROOT, "catalog", "packs.json")))
        self.assertFalse(os.path.isfile(os.path.join(self.folder, "packs.json")), "a copy kept beside the page would go stale: the workflow makes it")

    def test_the_page_waits_for_the_windows_checks_and_not_for_the_helper_s(self):
        """The pages workflow publishes on the success of the workflow checks. The Linux and Mac jobs are a workflow of
        their own, so that one of them, red or hung, never holds the page back."""
        self.assertRegex(text(".github", "workflows", "pages.yml"), r"(?m)^\s*workflows: \[checks\]\s*$")
        checks, helper = text(".github", "workflows", "checks.yml"), text(".github", "workflows", "helper.yml")
        self.assertEqual(re.findall(r"(?m)^    runs-on:\s*(\S+)", checks), ["windows-latest"])
        self.assertEqual(re.findall(r"(?m)^    runs-on:\s*(\S+)", helper), ["ubuntu-latest", "macos-latest"])
        for said in ("name: helper\n", "  contents: read\n", "  group: helper-"):
            self.assertIn(said, helper)

    def test_no_page_for_os_1_7_0_is_served(self):
        """The site serves the page for OS 1.7.3 alone: the pages workflow makes no page for OS 1.7.0 (the last commit
        for it was b6501e4), nothing of one is kept in the tree, and the page links to none."""
        workflow = text(".github", "workflows", "pages.yml")
        self.assertNotIn("b6501e4", workflow)
        self.assertNotIn("1.7.0", workflow)
        self.assertNotRegex(workflow, r"\bgit (archive|show)\b")
        self.assertFalse(os.path.exists(os.path.join(self.folder, "1.7.0")), "no page for 1.7.0 is kept beside the page")
        self.assertNotRegex(text(self.served, "index.html"), r"1\.7\.0/")       # no path into a 1.7.0 folder, whatever its form

    def test_the_helper_s_line_is_made_from_the_page_s_own_address(self):
        page = text(self.served, "index.html")
        self.assertIn("\"$openFieldPage = '\" + PAGE_ID + \"'; $openFieldFolder = '\" + psQuoted(folderHandle.name) + \"'; irm '\" + helperUrl + \"' | iex\"", page)   # the page's mark and the folder's name
        self.assertIn('await chosen.writeText(NAMES.page, JSON.stringify({ page: 1, id: PAGE_ID', page)                 # which it writes into the folder chosen
        helper = text(self.served, "open-field-helper.ps1")
        self.assertIn("$Page = [string] $openFieldPage", helper)                                                         # and which the helper checks
        self.assertIn("$FolderName = [string] $openFieldFolder", helper)                                                 # and the name it looks for the folder by
        self.assertIn("STOPPED: the install page did not choose this folder", helper)
        self.assertNotRegex(page, r"irm\s+'https?://")
        # the line for a Mac and Linux: the same mark and name, the helper read from the page's own address and run by python3
        self.assertIn('const pythonLine = (url, mark, name) => "curl -fsSL " + shQuoted(url) + " | python3 -I - --page " + shQuoted(mark) + " --folder-name=" + shQuoted(name);', page)
        self.assertIn("pythonLine(pythonHelperUrl, PAGE_ID, folderHandle.name)", page)                                   # the page's mark and the folder's name
        helper = text(self.served, "open-field-helper.py")
        self.assertIn('ap.add_argument("--page"', helper)                                                                # which the helper takes
        self.assertIn('ap.add_argument("--folder-name"', helper)
        self.assertIn("STOPPED: the install page did not choose this folder", helper)
        self.assertNotRegex(page, r"curl\s+-fsSL\s+'https?://")


class ThePageSStorage(unittest.TestCase):
    """The site served a page for OS 1.7.0 before this one, and what that page stored may still be in a browser's
    storage. So the keys of the browser's storage the page writes carry its firmware, and a check of the unit it finds
    stored (step 1, the gate) is taken again only when it was made for this firmware and its answer is still a supported
    one. Without that, a gate the other page let through could be taken by this one."""

    @classmethod
    def setUpClass(cls):
        cls.page = text("builder", "index.html")
        cls.firmware = load(*PROFILE.split("/"))["firmware_version"].split("+")[0]

    def test_the_page_is_for_the_tree_s_firmware(self):
        self.assertIn('const FIRMWARE = "%s";' % self.firmware, self.page)
        self.assertIn("const SUPPORTED_OS = [FIRMWARE, FIRMWARE + \"+0\", FIRMWARE + \"+1\"];", self.page)
        self.assertEqual(load("builder", "loader.json")["os"], self.firmware)

    def test_every_key_of_the_session_s_storage_carries_the_firmware(self):
        self.assertIn('const GATE_KEY = "open-field-gate-" + FIRMWARE, PAGE_KEY = "open-field-page-" + FIRMWARE;', self.page)
        keys = re.findall(r"sessionStorage\.(?:getItem|setItem|removeItem)\(\s*([^,)]+)", self.page)
        self.assertGreater(len(keys), 4)
        self.assertEqual(sorted(set(key.strip() for key in keys)), ["GATE_KEY", "PAGE_KEY"])

    def test_a_stored_gate_is_held_to_the_firmware_and_the_supported_versions_again(self):
        body = re.search(r"\n  function restoreGate\(\) \{\n(.*?)\n  \}\n", self.page, re.S).group(1)
        check = body.index("const fits")
        self.assertIn("g.firmware === FIRMWARE", body)
        self.assertIn("SUPPORTED_BL.includes(g.bl", body)
        self.assertIn("SUPPORTED_OS.includes(g.os)", body)
        self.assertLess(check, body.index("if (gate && !fits(gate)) { gate = null;"))
        self.assertLess(body.index("if (gate && !fits(gate)) { gate = null;"), body.index("if (gate) {"))
        self.assertIn("g.mark === PAGE_ID", body)                                     # no gate without the mark its find made
        self.assertIn("gate = { firmware: FIRMWARE, bl: id.bl_version, os: id.os_version || null, unit: id.serial, mark: PAGE_ID, used: null };", self.page)

    def test_a_unit_on_os_1_7_0_is_told_to_update_first(self):
        self.assertIn('const OLDER_OS = ["1.7.0", "1.7.0+0"];', self.page)
        self.assertIn("} else if (blOk && OLDER_OS.includes(id.os_version)) {", self.page)
        # one refusal for every bootloader, built of nodes as the version in it comes from the device: the words of its
        # one warn span, a sentence written over two literals read as one, ask for the update and say its risk
        branch = re.search(r"\n      \} else if \(blOk && OLDER_OS\.includes\(id\.os_version\)\) \{\n(.*?)\n      \} else \{\n", self.page, re.S).group(1)
        branch = re.sub(r'"\s*\+\s*"', "", branch)
        refusal = re.findall(r'\$\("gate"\)\.replaceChildren\(spanOf\("warn", (.*?)\)\);\n', branch, re.S)
        self.assertEqual(len(refusal), 1)
        self.assertIn("Update it to OS 1.7.3 or 1.7.3+1 with Teenage Engineering's own update first, then come back to this page.", refusal[0])
        self.assertIn("1.7.3+1 comes with a new bootloader, 1000.2.0, that is more locked down and doesn't let your OP-1 go back to an "
                      "older OS version: updating to it is at your own risk.", refusal[0])
        self.assertNotIn("innerHTML", branch)


class TheCheckByForm(unittest.TestCase):
    def test_the_part_of_the_check_that_needs_no_firmware_finds_nothing(self):
        """The whole check needs the maintainer's own firmware files and runs before every commit (docs/checking.md).
        This part reads names and the form of text; it cannot judge a binary, and says so for each."""
        folders = [name for name in ("builder", "docs", "examples", "hooks", "linker", "module", "packs", "pictures", "profiles", "tools") if os.path.isdir(os.path.join(ROOT, name))]
        done = subprocess.run([sys.executable, "-I", os.path.join(TOOLS, "publish_check.py"), "--forms-only"] + folders,
                              cwd=ROOT, capture_output=True, text=True)
        self.assertEqual(done.returncode, 0, done.stdout[-3000:])
        self.assertIn(" 0 refused", done.stdout)


@unittest.skipUnless(STOCK and os.path.isfile(STOCK), "your own firmware file is needed: set OPEN_FIELD_STOCK to its path")
class TheProfileAgainstYourOwnFirmwareFile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profile = load(*PROFILE.split("/"))
        with open(STOCK, "rb") as f:
            cls.image = f.read()
        cls.records = [record for record in tool("ldr_dump_records").parse(cls.image) if not record["fill"]]

    def read(self, address, count):
        for record in self.records:
            if record["target"] <= address and address + count <= record["target"] + record["count"]:
                start = record["offset"] + 16 + address - record["target"]
                return self.image[start:start + count]
        self.fail("0x%08x is in no stored record of the file" % address)

    def test_it_is_the_file_the_profile_is_for(self):
        self.assertEqual(len(self.image), self.profile["stock_bytes"])
        self.assertEqual(hashlib.sha256(self.image).hexdigest(), self.profile["stock_sha256"])

    def test_every_place_s_sum_is_that_of_the_file_s_bytes_there(self):
        for place in self.profile["patches"]:
            held = self.read(int(place["address"], 16), place["length"])
            self.assertEqual("0x%08x" % (zlib.crc32(held) & 0xffffffff), place["sum"], place["address"])

    def test_a_trampoline_s_own_sum_is_over_all_the_bytes_it_copies(self):
        seen = 0
        for table in self.profile["tables"]:
            for entry in table["entries"]:
                if "sum" in entry:
                    seen += 1
                    held = self.read(int(entry["address"], 16), entry["copies"])
                    self.assertEqual("0x%08x" % (zlib.crc32(held) & 0xffffffff), entry["sum"], entry["address"])
        self.assertGreater(seen, 0)


@unittest.skipUnless(os.path.isdir(TOOLCHAIN), "the toolchain is needed: docs/build.md says how to get it")
class TheBuild(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder = tempfile.mkdtemp(prefix="open-field-build-")
        cls.out = os.path.join(cls.folder, "out")
        cls.done = subprocess.run([sys.executable, "-I", os.path.join("tools", "build_all.py"), "--out", cls.out], cwd=ROOT, capture_output=True, text=True)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.folder, ignore_errors=True)

    def test_everything_builds_to_the_kept_files(self):
        self.assertEqual(self.done.returncode, 0, self.done.stdout[-3000:] + self.done.stderr[-2000:])
        self.assertIn("each is the kept one", self.done.stdout)

    def test_the_build_holds_the_builder_s_loader_file_and_its_manifest(self):
        blob = load("build.json")["blob"]
        self.assertIn("the same  builder/%s, byte for byte" % blob, self.done.stdout)
        self.assertRegex(self.done.stdout, r"the same  builder/loader\S*\.json names it: its size, its SHA-256, and the loader's entry at 0x[0-9a-f]{8}")

    def test_the_build_holds_the_page_s_base_pack_and_the_file_that_names_it(self):
        self.assertIn("the same  builder/base.pack, byte for byte, and builder/base-pack.json names it", self.done.stdout)
        with open(os.path.join(self.out, "packs", "base.pack"), "rb") as f, open(os.path.join(ROOT, "builder", "base.pack"), "rb") as g:
            self.assertEqual(f.read(), g.read())

    def test_every_pack_and_example_is_built(self):
        for name in PACKS:
            self.assertRegex(self.done.stdout, r"the same  %s\.pack " % re.escape(name))
        for name in EXAMPLES + ("trampolines",):
            self.assertRegex(self.done.stdout, r"the same  examples/%s\.pack " % name)

    @unittest.skipUnless(shutil.which("node"), "node is needed")
    def test_the_manifest_is_the_one_the_tool_gives_for_this_build(self):
        """builder/write-manifest.mjs --check, on the loader's file this build made. With your own firmware file at hand
        it holds the whole manifest, byte for byte. Without one, everything but the two things that need one."""
        plan = load("build.json")
        version = re.match(r"^loader-.*-(v[0-9]+)\.bin$", plan["blob"]).group(1)
        words = ["node", os.path.join("builder", "write-manifest.mjs"), "--check", "--loader", os.path.join(self.out, plan["blob"]),
                 "--elf", os.path.join("build", plan["loader"]["name"] + ".elf"), "--version", version]
        have = bool(STOCK) and os.path.isfile(STOCK)
        done = subprocess.run(words + (["--stock", STOCK] if have else []), cwd=ROOT, capture_output=True, text=True, env=dict(os.environ, OPEN_FIELD_STOCK=""),
                              stdin=subprocess.DEVNULL, timeout=300)
        self.assertEqual(done.returncode, 0, done.stdout + done.stderr)
        self.assertNotIn("DIFFERS", done.stdout)
        self.assertIn("the manifest, byte for byte" if have else "the manifest, but for what needs a firmware file", done.stdout)


if __name__ == "__main__":
    unittest.main()

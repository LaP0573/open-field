"""tools/build_all.py's three ways of holding the base: the build on main, the check of a pull request, and the
release of the base. Main is the original of the base's sources, and its sources may be ahead of the released base,
the one the install page serves from builder/; the release's own files are the maintainer's.

  python -I -m unittest discover -s tools/tests -p "test_build_modes.py"

Needs nothing but the tree and git: no toolchain, no build. The files are made up in a temporary folder, or read
from the tree without being written.
"""

import contextlib
import hashlib
import importlib.util
import io
import json
import os
import re
import shutil
import subprocess
import tempfile
import unittest
import unittest.mock
import urllib.parse

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOOLS = os.path.join(ROOT, "tools")


def tool(name):
    spec = importlib.util.spec_from_file_location("modes_" + name, os.path.join(TOOLS, name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


build_all = tool("build_all")


def load(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8") as f:
        return json.load(f)


def raw(*parts):
    with open(os.path.join(ROOT, *parts), "rb") as f:
        return f.read()


def put(root, path, data):
    full = os.path.join(root, *path.split("/"))
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "wb") as f:
        f.write(data if isinstance(data, bytes) else data.encode("utf-8"))


class TheInputs(unittest.TestCase):
    """base_inputs(): the SHA-256 over the files the build of the base reads, kept beside the released base."""

    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="open-field-modes-")
        self.addCleanup(shutil.rmtree, self.root, True)
        put(self.root, "packs/base.json", json.dumps({"name": "base", "version": 1, "release": "0.1", "sources": ["module/a.s", "module/b.cpp"],
                                                      "pictures": [{"name": "door.svg", "file": "pictures/base/door.svg.make"}]}))
        put(self.root, "module/a.s", ".long 1\n")
        put(self.root, "module/b.cpp", '#include "x.h"\n#include "mock_not_here.h"\n#include <stddef.h>\nint b;\n')
        put(self.root, "module/x.h", '  #  include "y.h"\n')
        put(self.root, "module/y.h", "enum { Y = 1 };\n")
        put(self.root, "module/z.h", "enum { Z = 1 };\n")                   # taken in by nobody
        put(self.root, "pictures/base/door.svg.make", "box\n")
        put(self.root, "profiles/p.json", json.dumps({"id": "p", "firmware_version": "9.9.9+0", "module_base": "0x8d000000", "reserved_bytes": 65536 * 4}))
        put(self.root, "linker/p-pack.ld", "MEMORY\n")
        put(self.root, "linker/op1f-9.9.9-pack.ld", "MEMORY\n")             # the firmware's: the profile's own comes first
        for name in build_all.BASE_TOOLS:
            put(self.root, name, "# %s\n" % name)

    def inputs(self):
        return build_all.base_inputs("profiles/p.json", root=self.root)

    def test_the_files_are_the_recipe_its_sources_what_they_take_in_the_profile_the_script_and_the_tools(self):
        self.assertEqual(build_all.base_input_files("profiles/p.json", root=self.root),
                         sorted(["linker/p-pack.ld", "module/a.s", "module/b.cpp", "module/x.h", "module/y.h", "packs/base.json",
                                 "pictures/base/door.svg.make", "profiles/p.json"] + list(build_all.BASE_TOOLS)))

    def test_the_sum_is_each_file_s_path_size_and_bytes_in_sorted_order(self):
        digest = hashlib.sha256()
        for path in build_all.base_input_files("profiles/p.json", root=self.root):
            with open(os.path.join(self.root, *path.split("/")), "rb") as f:
                data = f.read()
            digest.update(path.encode("utf-8") + b"\0" + str(len(data)).encode("ascii") + b"\0" + data)
        self.assertEqual(self.inputs(), digest.hexdigest())

    def test_a_byte_of_a_file_taken_in_changes_it_and_a_file_nobody_takes_in_does_not(self):
        before = self.inputs()
        put(self.root, "module/z.h", "enum { Z = 2 };\n")
        self.assertEqual(self.inputs(), before)
        put(self.root, "module/y.h", "enum { Y = 2 };\n")
        self.assertNotEqual(self.inputs(), before)

    def test_a_line_ending_is_never_made_the_same(self):
        before = self.inputs()
        put(self.root, "module/a.s", ".long 1\r\n")
        self.assertNotEqual(self.inputs(), before)

    def test_a_tool_that_makes_the_bytes_and_the_profile_change_it(self):
        for path in build_all.BASE_TOOLS + ("linker/p-pack.ld", "pictures/base/door.svg.make"):
            before = self.inputs()
            put(self.root, path, "changed\n")
            self.assertNotEqual(self.inputs(), before, path)
        before = self.inputs()
        put(self.root, "profiles/p.json", json.dumps({"id": "p", "firmware_version": "9.9.9+0", "module_base": "0x8d000000", "reserved_bytes": 65536 * 8}))
        self.assertNotEqual(self.inputs(), before)

    def test_the_tree_s_files_and_its_kept_value(self):
        files = build_all.base_input_files(load("build.json")["profile"])
        recipe = load("packs", "base.json")
        for path in ["packs/base.json", load("build.json")["profile"], "linker/op1f-1.7.3-pack.ld"] + recipe["sources"] + list(build_all.BASE_TOOLS):
            self.assertIn(path, files)
        self.assertIn("module/build_r6d_base.h", files)
        self.assertNotIn("tools/build_all.py", files)
        self.assertRegex(load("packs", "built.json")["files"]["base.pack"]["inputs"], r"^[0-9a-f]{64}$")


class TheDecision(unittest.TestCase):
    """base_state(): the base a build made, held to the released one."""

    BUILT = b"a built base"
    RELEASED = {"bytes": len(BUILT), "sha256": hashlib.sha256(BUILT).hexdigest(), "inputs": "1" * 64}

    def test_the_released_bytes_are_the_same(self):
        self.assertEqual(build_all.base_state(self.BUILT, self.RELEASED, "1" * 64), "same")
        self.assertEqual(build_all.base_state(self.BUILT, self.RELEASED, "2" * 64), "same")

    def test_other_bytes_from_other_files_are_sources_ahead_of_the_release(self):
        self.assertEqual(build_all.base_state(b"another base", self.RELEASED, "2" * 64), "ahead")

    def test_other_bytes_from_the_same_files_are_a_fault(self):
        self.assertEqual(build_all.base_state(b"another base", self.RELEASED, "1" * 64), "fault")

    def test_other_bytes_with_no_inputs_recorded_are_a_fault_and_never_ahead(self):
        for record in ({"bytes": 1, "sha256": "0" * 64}, {"bytes": 1, "sha256": "0" * 64, "inputs": ""}, {"bytes": 1, "sha256": "0" * 64, "inputs": None}):
            self.assertEqual(build_all.base_state(b"another base", record, "2" * 64), "fault", record)
        self.assertEqual(build_all.base_state(self.BUILT, {"bytes": len(self.BUILT), "sha256": hashlib.sha256(self.BUILT).hexdigest()}, "2" * 64), "same")
        self.assertIn('"no inputs are recorded for the released base in packs/built.json, so the build cannot tell sources ahead "',
                      raw("tools", "build_all.py").decode("utf-8"))

    def test_no_released_base_is_a_fault(self):
        self.assertEqual(build_all.base_state(self.BUILT, None, "1" * 64), "fault")


DOOR = "/* Base 1 */\n.global op1f_door_base_version\nop1f_door_base_version:\n    .long %s\n"
RELEASE = '/* Generated by tools/pack_build.py release from packs/base.json */\nop1f_release:\n    .ascii "%s"\n    .fill 12, 1, 0\n'
ART = "/* Generated by tools/pack_build.py pictures from packs/base.json */\nop1f_art_base:\n    .long 0x31545241, 3, %d, %d\n"
SOURCES = ["module/door.s", "module/release.s", "module/art.s", "module/code.cpp"]


def recipe(version=43, release="0.25", sources=SOURCES):
    return json.dumps({"name": "base", "version": version, "release": release, "sources": sources}).encode()


def built(base_sha="a" * 64, example_sha="b" * 64):
    return json.dumps({"files": {"base.pack": {"bytes": 1, "sha256": base_sha, "inputs": "c" * 64}, "examples/wink.pack": {"bytes": 2, "sha256": example_sha}}}).encode()


class TheRefusals(unittest.TestCase):
    """release_changes(): what a pull request may not change, the release's own files and numbers."""

    OLD = {"packs/base.json": recipe(), "packs/built.json": built(), "module/door.s": (DOOR % 43).encode(),
           "module/release.s": (RELEASE % "0.25").encode(), "module/art.s": (ART % (6081, 43)).encode(), "module/code.cpp": b"int a = 1;\n",
           "builder/base.pack": b"base", "builder/base-pack.json": b"{}", "catalog/packs/base.json": b"{}"}

    def refused(self, path, data):
        files = dict(self.OLD, **{path: data})
        changed = {name for name in files if files[name] != self.OLD.get(name)}
        return build_all.release_changes(changed, self.OLD.get, files.get)

    def test_a_change_to_the_base_s_code_is_let_through(self):
        self.assertEqual(self.refused("module/code.cpp", b"int a = 2;\n"), [])
        self.assertEqual(self.refused("packs/base.json", recipe(sources=SOURCES + ["module/more.s"])), [])

    def test_each_of_the_release_s_own_files_is_refused(self):
        for path in build_all.RELEASE_OWN:
            said = self.refused(path, b"other")
            self.assertEqual(len(said), 1, said)
            self.assertIn(path, said[0])
            self.assertIn("a release is the maintainer's own pull request", said[0])

    def test_packs_built_json_s_base_is_refused_and_its_other_files_are_not(self):
        self.assertEqual(self.refused("packs/built.json", built(example_sha="d" * 64)), [])
        said = self.refused("packs/built.json", built(base_sha="d" * 64))
        self.assertEqual(len(said), 1, said)
        self.assertIn("packs/built.json's entry of base.pack", said[0])

    def test_the_version_and_the_release_in_the_recipe_are_refused(self):
        said = self.refused("packs/base.json", recipe(version=44, release="0.26"))
        self.assertEqual(len(said), 2, said)
        self.assertIn("version from 43 to 44", said[0])
        self.assertIn('release from "0.25" to "0.26"', said[1])

    def test_the_numbers_in_the_sources_are_refused_and_the_rest_of_them_is_not(self):
        self.assertIn("(op1f_door_base_version) from 43 to 44", self.refused("module/door.s", (DOOR % 44).encode())[0])
        self.assertEqual(self.refused("module/door.s", (DOOR % 43).replace("Base 1", "Base one").encode()), [])
        self.assertIn('(op1f_release) from "0.25" to "0.26"', self.refused("module/release.s", (RELEASE % "0.26").encode())[0])
        self.assertIn("(op1f_art_base) from 43 to 44", self.refused("module/art.s", (ART % (6081, 44)).encode())[0])
        self.assertEqual(self.refused("module/art.s", (ART % (6200, 43)).encode()), [], "a picture changed: its directory's sizes move, not its version")

    def test_a_whole_release_moves_the_version_up_and_names_a_new_release(self):
        self.assertEqual(build_all.release_moves(recipe(), recipe(version=44, release="0.26")), [])
        for version, release, said in ((43, "0.26", ["version is 43"]), (42, "0.26", ["version is 42"]), (44, "0.25", ['release is "0.25"']),
                                       (43, "0.25", ["version is 43", 'release is "0.25"'])):
            found = build_all.release_moves(recipe(), recipe(version=version, release=release))
            self.assertEqual(len(found), len(said), found)
            for sentence, part in zip(found, said):
                self.assertIn(part, sentence)
        self.assertIn("above the one of the branch it merges into, 43", build_all.release_moves(recipe(), recipe(version="44", release="0.26"))[0])

    def test_a_number_taken_out_of_its_source_is_refused(self):
        said = self.refused("module/door.s", b"/* gone */\n")
        self.assertEqual(len(said), 1, said)
        self.assertIn("from 43 to None", said[0])


class TheCommitAPullRequestStartsFrom(unittest.TestCase):
    """pull_request_files(): the paths from the commit in common to the files as they stand, in a git repository of
    its own."""

    @classmethod
    def setUpClass(cls):
        if not shutil.which("git"):
            raise unittest.SkipTest("git is needed")
        cls.root = tempfile.mkdtemp(prefix="open-field-modes-git-")

        def git(*words):
            subprocess.run(["git", "-C", cls.root, "-c", "user.name=t", "-c", "user.email=t@invalid", "-c", "commit.gpgsign=false"] + list(words),
                           check=True, capture_output=True)

        git("init", "-q")
        put(cls.root, "builder/base.pack", b"released")
        put(cls.root, "module/a.s", b".long 1\n")
        git("add", "-A")
        git("commit", "-q", "-m", "main")
        cls.base = subprocess.run(["git", "-C", cls.root, "rev-parse", "HEAD"], check=True, capture_output=True, text=True).stdout.strip()
        put(cls.root, "module/a.s", b".long 2\n")
        git("commit", "-q", "-am", "a change")
        put(cls.root, "builder/base.pack", b"touched")                      # not committed: it counts too

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.root, ignore_errors=True)

    def test_the_changed_paths_and_both_sides_of_each(self):
        start, changed, old, new = build_all.pull_request_files(self.base, self.root)
        self.assertEqual(start, self.base)
        self.assertEqual(changed, {"module/a.s", "builder/base.pack"})
        self.assertEqual((old("module/a.s"), new("module/a.s")), (b".long 1\n", b".long 2\n"))
        self.assertEqual((old("builder/base.pack"), new("builder/base.pack")), (b"released", b"touched"))
        self.assertIsNone(old("not/there"))
        self.assertIn('"builder/base.pack" changed', build_all.release_changes(changed, old, new)[0])

    def test_a_commit_git_does_not_have_stops_the_check(self):
        with self.assertRaises(RuntimeError):
            build_all.pull_request_files("0" * 40, self.root)


class TheAPISAnswer(unittest.TestCase):
    """gh_api(): a file that is not there is the HTTP status 404 of the answer's status line, and nothing else."""

    def answer(self, returncode, stdout, stderr=b""):
        done = subprocess.CompletedProcess([], returncode, stdout, stderr)
        with unittest.mock.patch.object(build_all.subprocess, "run", return_value=done) as called:
            try:
                return build_all.gh_api("repos/a/b/contents/x?ref=c", raw=True)
            finally:
                self.assertEqual(called.call_args[0][0][:3], ["gh", "api", "--include"])

    def test_the_status_line_decides(self):
        self.assertIsNone(self.answer(1, b"HTTP/2.0 404 Not Found\nX-A: 1\r\n\r\n{}", b"gh: Not Found (HTTP 404)"))
        self.assertEqual(self.answer(0, b"HTTP/2.0 200 OK\nX-A: 1\r\nX-B: 404\r\n\r\nline 404\n"), b"line 404\n")
        self.assertEqual(build_all.api_answer(b"HTTP/1.1 200 OK\n\nbody"), (200, b"body"))

    def test_a_404_in_any_other_text_is_an_error(self):
        for returncode, stdout, stderr in ((1, b"HTTP/2.0 500 Server Error\r\n\r\n{\"message\": \"404\"}", b"gh: 404 (HTTP 500)"),
                                           (1, b"", b"error connecting to api.github.com: 404"), (0, b"HTTP/2.0 4040 Odd\r\n\r\n", b"")):
            with self.assertRaises(RuntimeError):
                self.answer(returncode, stdout, stderr)


class TheReleaseSWayBack(unittest.TestCase):
    """main(): a release of the base that does not end in 0 puts back every file it wrote, as it was."""

    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="open-field-modes-back-")
        self.addCleanup(shutil.rmtree, self.root, True)
        self.kept, self.new = os.path.join(self.root, "kept.s"), os.path.join(self.root, "new.json")
        put(self.root, "kept.s", ".long 43\n")

    def release(self, answer):
        def hold(argv, taken):
            taken.update({self.kept: b".long 43\n", self.new: None})
            put(self.root, "kept.s", ".long 44\n")
            put(self.root, "new.json", "{}")
            if isinstance(answer, BaseException):
                raise answer
            return answer
        with unittest.mock.patch.object(build_all, "hold", hold), contextlib.redirect_stdout(io.StringIO()) as printed:
            try:
                return build_all.main([]), printed.getvalue()
            except KeyboardInterrupt:
                return "stopped", printed.getvalue()

    def test_a_later_step_that_fails_puts_every_file_back(self):
        for answer in (1, 2, KeyboardInterrupt()):
            said, printed = self.release(answer)
            with open(self.kept, "rb") as f:
                self.assertEqual(f.read(), b".long 43\n", answer)
            self.assertFalse(os.path.exists(self.new), answer)
            self.assertIn("every file it wrote is put back as it was", printed)

    def test_a_release_that_ends_in_0_keeps_what_it_wrote(self):
        said, printed = self.release(0)
        self.assertEqual(said, 0)
        with open(self.kept, "rb") as f:
            self.assertEqual(f.read(), b".long 44\n")
        self.assertTrue(os.path.exists(self.new))
        self.assertNotIn("put back", printed)


class TheReleaseSOwnFiles(unittest.TestCase):
    """release_faults(): builder/base.pack, builder/base-pack.json and catalog/packs/base.json, held to packs/built.json's
    base.pack, on a copy of the tree's."""

    def setUp(self):
        self.root = tempfile.mkdtemp(prefix="open-field-modes-release-")
        self.addCleanup(shutil.rmtree, self.root, True)
        for path in ("builder/base.pack", "builder/base-pack.json", "catalog/packs/base.json"):
            put(self.root, path, raw(*path.split("/")))
        self.released = load("packs", "built.json")["files"]["base.pack"]

    def test_the_tree_s_are_in_order(self):
        self.assertEqual(build_all.release_faults(self.released, self.root), [])

    def test_a_byte_of_the_page_s_base_is_a_fault(self):
        data = bytearray(raw("builder", "base.pack"))
        data[100] ^= 1
        put(self.root, "builder/base.pack", bytes(data))
        self.assertEqual(build_all.release_faults(self.released, self.root), ["builder/base.pack is not the released base, packs/built.json's base.pack"])

    def test_the_listing_s_entry_is_held_to_its_size_sha_and_version(self):
        entry = load("catalog", "packs", "base.json")
        put(self.root, "catalog/packs/base.json", json.dumps(dict(entry, version=entry["version"] + 1)))
        said = build_all.release_faults(self.released, self.root)
        self.assertEqual(len(said), 1, said)
        self.assertIn("catalog/packs/base.json says version", said[0])

    def test_the_listing_s_places_and_level_are_what_the_release_step_writes(self):
        data = raw("builder", "base.pack")
        fields, _body = build_all.tool_module("pack_build").parse(data)
        level = build_all.tool_module("catalog_check").level_of([(name, True) for name, _offset in fields["lists"]["exports"]], False)
        entry = load("catalog", "packs", "base.json")
        self.assertEqual((entry["places"], entry["offers_level"]), (fields["entries"], level))


class TheReleaseStep(unittest.TestCase):
    """release_numbers(): what the release step writes before it builds, read from the tree and never written here."""

    def test_the_released_numbers_give_the_tree_s_files_byte_for_byte(self):
        named = load("builder", "base-pack.json")
        out = build_all.release_numbers(named["version"], named["release"])
        self.assertEqual(sorted(out), ["module/build_r6d_base_pictures.s", "module/build_r6d_base_release.s", "module/build_r6d_door_base.s", "packs/base.json"])
        for path, data in out.items():
            self.assertEqual(data, raw(*path.split("/")), path)

    def test_a_new_release_changes_its_numbers_and_nothing_else(self):
        named = load("builder", "base-pack.json")
        version, release = named["version"] + 1, "9.99"
        for path, data in build_all.release_numbers(version, release).items():
            before, after = raw(*path.split("/")).decode("utf-8").split("\n"), data.decode("utf-8").split("\n")
            self.assertEqual(len(before), len(after), path)
            changed = [(a, b) for a, b in zip(before, after) if a != b]
            self.assertTrue(changed, path)
            for line_before, line_after in changed:
                self.assertTrue(str(version) in line_after or release in line_after, (path, line_after))
                self.assertEqual(re.sub(r"[0-9.]+", "#", line_before), re.sub(r"[0-9.]+", "#", line_after), path)

    def test_a_release_out_of_form_is_refused(self):
        with self.assertRaises(ValueError):
            build_all.release_numbers(99, "a release!")


class TheOptions(unittest.TestCase):
    """The options that go together, refused before anything is built."""

    def said(self, *words):
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            answer = build_all.main(list(words))
        return answer, printed.getvalue()

    @unittest.skipUnless(os.path.samefile(os.getcwd(), ROOT), "run from the repository's root")
    def test_each_wrong_set_is_refused(self):
        for words, said in ((["--release", "0.26"], "the release of the base takes all three"),
                            (["--base-version", "44", "--release", "0.26"], "the release of the base takes all three"),
                            (["--release-allowed"], "--release-allowed goes with --pull-request"),
                            (["--pull-request", "HEAD", "--update"], "--pull-request checks and writes nothing")):
            answer, printed = self.said(*words)
            self.assertEqual(answer, 2, words)
            self.assertIn(said, printed)


class TheWorkflows(unittest.TestCase):
    """The host's checks run the build in its mode: a pull request held to the commit it merges into, with the
    release's own files let through only from a branch of this repository; and builder/ is the released base."""

    def test_the_checks_run_the_three_modes(self):
        with open(os.path.join(ROOT, ".github", "workflows", "checks.yml"), encoding="utf-8") as f:
            checks = f.read()
        self.assertRegex(checks, r"- uses: actions/checkout@v4\n        with:\n          fetch-depth: 0\n")
        steps = re.findall(r"(?m)^        if: (.*)\n        run: (python -I tools/build_all\.py.*)$", checks)
        mine = "github.event.pull_request.head.repo.full_name == github.repository"
        self.assertEqual(steps, [
            ("github.event_name != 'pull_request'", "python -I tools/build_all.py"),
            ("github.event_name == 'pull_request' && %s" % mine.replace("==", "!="), 'python -I tools/build_all.py --pull-request "$(git rev-parse HEAD^1)"'),
            ("github.event_name == 'pull_request' && %s" % mine, 'python -I tools/build_all.py --pull-request "$(git rev-parse HEAD^1)" --release-allowed'),
        ])

    def test_the_pages_say_builder_holds_the_released_base_and_publish_a_push_to_this_repository_s_main(self):
        with open(os.path.join(ROOT, ".github", "workflows", "pages.yml"), encoding="utf-8") as f:
            pages = f.read()
        self.assertIn("builder/ always holds the released base", pages)
        condition = re.search(r"(?m)^    if: >-\n((?:      .*\n)+)", pages).group(1).split()
        self.assertEqual(" ".join(condition), "github.event_name == 'workflow_dispatch' || (github.event.workflow_run.conclusion == 'success' && "
                         "github.event.workflow_run.event == 'push' && github.event.workflow_run.head_repository.full_name == github.repository)")

    def test_the_guard_runs_main_s_own_code_and_reads_the_pull_request_as_data(self):
        with open(os.path.join(ROOT, ".github", "workflows", "guard.yml"), encoding="utf-8") as f:
            guard = f.read()
        self.assertRegex(guard, r"(?m)^on:\n  pull_request_target:\n")
        self.assertRegex(guard, r"(?m)^permissions:\n  contents: read\n  pull-requests: read\n  statuses: write\n\n")
        self.assertEqual(re.findall(r"(?m)^    runs-on:\s*(\S+)", guard), ["ubuntu-latest"])
        self.assertIn("ref: ${{ github.event.repository.default_branch }}\n          persist-credentials: false\n", guard)
        self.assertEqual(re.findall(r"(?m)^[ \t]*ref:.*$", guard), ["          ref: ${{ github.event.repository.default_branch }}"])
        self.assertNotRegex(guard, r"ref:\s*\$\{\{\s*github\.event\.pull_request|github\.head_ref|merge_commit_sha|toolchain|open-xy|pull_request\.(title|body)")
        self.assertEqual([line.strip() for line in re.findall(r"(?m)^.*github\.event\.pull_request\.head.*$", guard)],
                         ["HEAD: ${{ github.event.pull_request.head.sha }}"] * 2)        # the event's head, as data of two steps
        self.assertEqual(re.findall(r"(?m)^[ \t]*run: (.*)$", guard), ['python -I tools/build_all.py --guard "$PULL_REQUEST" --head "$HEAD"', "|"])
        self.assertIn("GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}", guard)

    def test_the_guard_s_answer_is_a_status_of_its_own_on_the_event_s_head(self):
        with open(os.path.join(ROOT, ".github", "workflows", "guard.yml"), encoding="utf-8") as f:
            guard = f.read()
        step = guard[guard.index("      - name: The status open-field/guard on the head commit"):]
        self.assertIn("        if: ${{ !cancelled() }}\n", step)
        self.assertIn("          ANSWER: ${{ steps.guard.outcome }}\n", step)
        self.assertIn('gh api --method POST "repos/$GITHUB_REPOSITORY/statuses/$HEAD" -f state="$state" -f context=open-field/guard', step)
        self.assertEqual(re.findall(r"state=(\w+);", step), ["success", "failure"])
        self.assertIn('if [ "$ANSWER" = success ]; then', step)
        self.assertIn("        id: guard\n", guard)
        for page in ("docs/build.md", "CONTRIBUTING.md"):
            with open(os.path.join(ROOT, *page.split("/")), encoding="utf-8") as f:
                self.assertIn("the status `open-field/guard`", f.read(), page)


class AnAPI:
    """The host's API as the guard asks it, from made-up answers: {path: bytes}, None (404) for any other."""

    def __init__(self, head_repository="someone/open-field", files=(), head=None, start=None, pages=1, changed_files=None, now="h" * 40):
        self.asked, self.answers = [], {}
        pull = {"base": {"sha": "b" * 40}, "head": {"sha": now, "repo": {"full_name": head_repository} if head_repository else None},
                "changed_files": len(files) if changed_files is None else changed_files}
        self.answers["repos/LaP0573/open-field/pulls/7"] = json.dumps(pull).encode()
        files = list(files)
        for page in range(1, pages + 1):
            part = files[(page - 1) * 100:page * 100] if page < pages else files[(page - 1) * 100:]
            self.answers["repos/LaP0573/open-field/pulls/7/files?per_page=100&page=%d" % page] = json.dumps(part).encode()
        self.answers["repos/LaP0573/open-field/compare/%s...%s" % ("b" * 40, "h" * 40)] = json.dumps({"merge_base_commit": {"sha": "m" * 40}}).encode()
        for commit, contents in (("m" * 40, start or {}), ("h" * 40, head or {})):
            for path, data in contents.items():
                self.answers["repos/%s/contents/%s?ref=%s" % (head_repository, urllib.parse.quote(path, safe="/"), commit)] = data

    def __call__(self, path, raw=False):
        self.asked.append((path, raw))
        return self.answers.get(path)


class TheGuard(unittest.TestCase):
    """guard(): the refusal of a fork's pull request, with main's own code, from the host's API alone."""

    START = {"packs/base.json": recipe(), "packs/built.json": built(), "module/door.s": (DOOR % 43).encode()}

    def run_guard(self, api):
        said = []
        return build_all.guard(7, "LaP0573/open-field", api, said.append), said

    def test_a_change_to_the_base_s_sources_passes(self):
        api = AnAPI(files=[{"filename": "module/code.cpp", "status": "modified"}], start=self.START, head=self.START)
        refusals, said = self.run_guard(api)
        self.assertEqual(refusals, [])
        self.assertTrue(all(path.startswith("repos/") for path, _raw in api.asked))
        self.assertIn(("repos/someone/open-field/contents/packs/base.json?ref=%s" % ("h" * 40), True), api.asked)

    def test_the_release_s_own_files_and_numbers_are_refused(self):
        head = dict(self.START, **{"packs/base.json": recipe(version=44), "packs/built.json": built(base_sha="d" * 64), "module/door.s": (DOOR % 44).encode()})
        files = [{"filename": name, "status": "modified"} for name in ("builder/base.pack", "packs/base.json", "packs/built.json", "module/door.s")]
        refusals, _said = self.run_guard(AnAPI(files=files, start=self.START, head=head))
        self.assertEqual(len(refusals), 4, refusals)
        for part in ('"builder/base.pack" changed', "packs/built.json's entry of base.pack", "version from 43 to 44", "(op1f_door_base_version) from 43 to 44"):
            self.assertTrue(any(part in refusal for refusal in refusals), part)

    def test_a_renamed_or_removed_file_of_the_release_is_refused_under_its_old_name(self):
        files = [{"filename": "builder/old-base.pack", "status": "renamed", "previous_filename": "builder/base.pack"},
                 {"filename": "catalog/packs/base.json", "status": "removed"}]
        refusals, _said = self.run_guard(AnAPI(files=files, start=self.START, head=self.START))
        self.assertEqual(len(refusals), 2, refusals)

    def test_every_page_of_files_is_read(self):
        files = [{"filename": "module/f%03d.cpp" % n, "status": "added"} for n in range(150)] + [{"filename": "builder/base-pack.json", "status": "modified"}]
        api = AnAPI(files=files, start=self.START, head=self.START, pages=2)
        refusals, _said = self.run_guard(api)
        self.assertEqual(len(refusals), 1, refusals)
        self.assertIn(("repos/LaP0573/open-field/pulls/7/files?per_page=100&page=2", False), api.asked)

    def test_a_branch_of_this_repository_passes_and_its_code_the_checks_run_is_named(self):
        files = [{"filename": name, "status": "modified"} for name in ("builder/base.pack", ".github/workflows/checks.yml", "tools/pack_build.py", "builder/install.mjs", "hooks/pre-commit", "builder/index.html")]
        refusals, said = self.run_guard(AnAPI(head_repository="LaP0573/open-field", files=files))
        self.assertEqual(refusals, [])
        named = [json.loads(line.rsplit(": ", 1)[1]) for line in said if line.startswith("code the checks run")]
        self.assertEqual(named, [".github/workflows/checks.yml", "builder/install.mjs", "hooks/pre-commit", "tools/pack_build.py"])

    def test_fewer_files_listed_than_the_pull_request_has_are_refused(self):
        files = [{"filename": "module/code.cpp", "status": "modified"}]
        refusals, _said = self.run_guard(AnAPI(files=files, start=self.START, head=self.START, changed_files=3001))
        self.assertEqual(len(refusals), 1, refusals)
        self.assertIn("gives 1 of the pull request's 3001 changed files", refusals[0])
        refusals, _said = self.run_guard(AnAPI(files=files, start=self.START, head=self.START, changed_files=1))
        self.assertEqual(refusals, [])

    def test_a_path_with_a_line_break_is_written_as_json_and_starts_no_line(self):
        odd = "tools/x\n::warning::y.s"
        start = dict(self.START, **{"packs/base.json": recipe(sources=SOURCES + [odd]), odd: (DOOR % 43).encode()})
        head = dict(start, **{odd: (DOOR % 44).encode()})
        refusals, said = self.run_guard(AnAPI(files=[{"filename": odd, "status": "modified"}], start=start, head=head))
        self.assertEqual(len(refusals), 1, refusals)
        for line in refusals + said:
            self.assertNotIn("\n", line)
        self.assertIn(json.dumps(odd), refusals[0])
        self.assertIn("code the checks run, changed by this pull request: %s" % json.dumps(odd), said)

    def test_the_event_s_head_is_judged_and_a_head_moved_on_stops_the_guard(self):
        api = AnAPI(files=[{"filename": "module/code.cpp", "status": "modified"}], start=self.START, head=self.START)
        refusals = build_all.guard(7, "LaP0573/open-field", api, [].append, head="h" * 40)
        self.assertEqual(refusals, [])
        self.assertIn(("repos/LaP0573/open-field/compare/%s...%s" % ("b" * 40, "h" * 40), False), api.asked)
        moved = AnAPI(files=[{"filename": "module/code.cpp", "status": "modified"}], start=self.START, head=self.START, now="n" * 40)
        with self.assertRaises(RuntimeError) as stopped:
            build_all.guard(7, "LaP0573/open-field", moved, [].append, head="h" * 40)
        self.assertIn("this run judges", str(stopped.exception))
        self.assertFalse([path for path, _raw in moved.asked if "/contents/" in path or "/compare/" in path])

    def test_a_head_repository_that_is_gone_stops_the_guard(self):
        with self.assertRaises(RuntimeError):
            self.run_guard(AnAPI(head_repository=None, files=[{"filename": "module/code.cpp"}]))

    @unittest.skipUnless(os.path.samefile(os.getcwd(), ROOT), "run from the repository's root")
    def test_the_guard_goes_with_the_repository_alone(self):
        printed = io.StringIO()
        with contextlib.redirect_stdout(printed):
            answer = build_all.main(["--guard", "7", "--repository", "LaP0573/open-field", "--update"])
        self.assertEqual(answer, 2)
        self.assertIn("--guard reads a pull request and builds nothing", printed.getvalue())


if __name__ == "__main__":
    unittest.main()

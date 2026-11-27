#!/usr/bin/env python3
"""Build everything this repository builds, and hold the results to the kept sizes and hashes.

  python -I tools/build_all.py [--out out] [--update [--loader-version v4] [--base-version N --release X.Y]]
  python -I tools/build_all.py [--out out] --pull-request BASE_SHA [--release-allowed]
  python -I tools/build_all.py --guard NUMBER [--repository OWNER/NAME] [--head SHA]

Run from the repository's root. It needs the Blackfin toolchain and the file `tooling/formats.py` of the project
open-xy, both in a folder `open-xy/` beside `tools/` (docs/build.md says how to get them). It needs no firmware
file and reads none: what it builds is our own code.

What it does, from `build.json`:

  1. holds the files that are made from others to what they are made from: the table sources to the profile
     (tools/table_build.py), the change list to the profile of the loader the install page offers by itself, which
     `build.json` names under `change_list` (tools/change_list.py), the page of the firmware's functions to the sources
     (tools/interfaces.py), and each example's table to the example's own profile;
  2. builds the base from its recipe (tools/pack_build.py build3) into <out>/packs: base.pack, which every unit with
     open-field needs. A feature is built in the repository it lives in;
  3. builds each example pack from its own recipe into <out>/examples;
  4. builds the loader, our small piece inside the firmware file (tools/module_build.py), into build/; and the
     builder's second loader, which `build.json` names under `loader_2` with a profile and a linker script of its own;
  5. copies each loader's binary into <out> under the name `build.json` gives it under `blob`,
     loader-<os>-<version>.bin: the file the installer's builder adds to an owner's firmware file;
  6. holds each result to packs/built.json; each file of step 5, byte for byte, to the file of that name in builder/;
     and the manifest in builder/ that names that file to this build: the file's size and SHA-256, the address at
     which the loader is entered, and the size of the firmware file the builder gives. The one thing of the manifest
     it cannot hold is the SHA-256 of that firmware file, which takes a firmware file:
     `node builder/validate-builder.mjs --stock <your own os.ldr>` holds it;
  7. holds the release's own files to the released base, packs/built.json's entry of base.pack: builder/base.pack,
     the base the install page puts on every unit it installs, byte for byte; builder/base-pack.json, which the page
     holds that file to, to the same file (its size, its SHA-256, and the version and the release the file says of
     itself); and the listing's entry catalog/packs/base.json to its size, SHA-256 and version.

The base's sources on main may be ahead of the released base: a change to the base is merged first and released
later, by the maintainer. So base.pack is held in three ways. When the build's bytes are the released ones, it is the
kept file. When they differ and so do the files its build reads (base_inputs(): the SHA-256 packs/built.json keeps
beside the released base as `inputs`), the sources are ahead of the release: one line says so, and that is no fault.
When they differ from the same files, the build is not the one the base was released with: a fault. So is a build
that differs when packs/built.json keeps no inputs for the released base: it cannot tell the two apart. The install page
keeps serving the released base in between, from builder/.

Answers 0 when every result is the kept one, 1 when one is not, 2 when it could not build. Writes only under the
folder --out names (which must be new or empty) and under build/, and with --update the files named below.

`--update` is for a change that is meant to change what is built. After the build it writes the new size and SHA-256
of every pack file that differs into packs/built.json, and says which, base.pack aside. Give the pack a new version
number in its recipe in the same change.

`--update --base-version N --release X.Y` is the release of the base, the maintainer's step. It writes the version N
and the release X.Y (what the start screen shows behind the words open-field) into packs/base.json and into the
sources that hold them (the one that gives op1f_door_base_version its number; the release's and the directory of
pictures, which `tools/pack_build.py release` and `pictures` write from the recipe), builds, and writes the released
base: packs/built.json's entry of base.pack (its size, its SHA-256 and its inputs), builder/base.pack,
builder/base-pack.json, and the size, SHA-256, version, places and level offered of catalog/packs/base.json. N comes
after the released base's version: a version keeps its bytes for good. When the build or a later step fails, every
file the step wrote is put back as it was, the numbers in the sources among them. Once the release is merged, the
install page
puts that base.pack on every unit it installs, so say in the change whether it ran on a unit.

`--pull-request BASE_SHA` is the check of a pull request into the commit BASE_SHA. Besides the build, it refuses a
change, from the commit the two have in common to the files as they stand, to the release's own files:
builder/base.pack, builder/base-pack.json, catalog/packs/base.json, packs/built.json's entry of base.pack, and the
base's version and release, in packs/base.json and in the sources that hold them, the directory of pictures' version
among them. A release is the maintainer's own pull request. `--release-allowed` lifts that refusal, for a branch of
this repository, which only the maintainer can make; when those files changed, the release must then be whole: the
build's base.pack is packs/built.json's, the release's own files are that file's, the inputs packs/built.json keeps
are the sources', and, when builder/base.pack is a new file, packs/base.json gives a version above BASE_SHA's and a
release other than BASE_SHA's. On the host, BASE_SHA is the first parent of the merge commit the checks run on.

`--guard NUMBER` is the check a fork cannot change: a pull request runs its own copy of the checks and of this tool,
so it could switch --pull-request off for itself. The workflow guard.yml runs on pull_request_target with main's own
code, and this mode reads the pull request NUMBER as data through the host's API (`gh api`, with GH_TOKEN), never
checks it out and never runs its code, and refuses what --pull-request refuses from a fork. It builds nothing and
needs no compiler. A branch of this repository passes. It also names the pull request's changed files that are code
the checks run (.github/, hooks/, tools/, builder/*.mjs), for the reader, each as JSON. --head SHA is the commit to

`--update --loader-version <name>` is for a change to the loader, which is the builder's file. A change to the base
(base.pack) is no change to the loader: the base is a file on the unit, never inside the firmware file, and its
release is the step above. A
version of the loader that anyone may have installed keeps its bytes for good, so a changed loader gets a new version
name: v and a number. The option names the builder's file loader-<os>-<name>.bin in build.json, builds it under that
name, and writes the new sizes and SHA-256 of the loader into packs/built.json. It does not write the builder's own
two files, the loader's file and its manifest in builder/: that takes a firmware file, and
`node builder/write-manifest.mjs` does it (docs/build.md, "A change to the loader"). Until they are written the
answer stays 1, and this tool says which command is next. Without --loader-version, --update leaves the entries of the
loader as they are and says so. A name whose file is in builder/ with other bytes is refused when that file's
manifest says it has run on a unit; a version of your own that nobody has tried may be built again under its name.
With two loaders in build.json, the new name goes to the one whose build is not its kept file. When both changed it
stops: a version's name is for one loader. The name of the other loader's file is refused too.

Standard library only.
"""

import argparse
import hashlib
import importlib.util
import json
import os
import posixpath
import re
import struct
import subprocess
import sys
import urllib.parse

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEPT = os.path.join(ROOT, "packs", "built.json")
PLAN = os.path.join(ROOT, "build.json")
PAGE_BASE = os.path.join(ROOT, "builder", "base.pack")                   # the base the install page puts on every unit
PAGE_BASE_NAMED = os.path.join(ROOT, "builder", "base-pack.json")        # what the page holds that file to
CATALOG_BASE = os.path.join(ROOT, "catalog", "packs", "base.json")       # the listing's entry of the base
BLOB = re.compile(r"^(loader-([0-9]+(?:\.[0-9]+)*))-(v[0-9]+)\.bin$")     # the builder's file: loader-<os>-<version>.bin
RECORD_HEADER = 16                                                        # what the builder puts before the loader in the firmware file
NOT_RUN = "not run on a unit yet"                                         # what builder/write-manifest.mjs says of a version nobody has tried
BASE = "base.pack"                                                        # the base's name in packs/built.json
RECIPE = "packs/base.json"                                                # the base's recipe, from the root
RELEASE_OWN = ("builder/base.pack", "builder/base-pack.json", "catalog/packs/base.json")   # the release's own files, the maintainer's
BASE_TOOLS = ("tools/module_build.py", "tools/pack_build.py", "tools/table_build.py")      # the tools whose code makes the base's bytes
VERSION_LABEL = "op1f_door_base_version"                                  # the name a source of the base gives its version
RELEASE_LABEL = "op1f_release"                                            # and the name of its release, the start screen's text
INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+)"', re.M)       # a file a C++ source takes in, by a name in quotes
RELEASE_STEP = "python -I tools/build_all.py --out <a new folder> --update --base-version <N> --release <X.Y>"
REFUSED = "a release is the maintainer's own pull request, so leave it as the branch it merges into has it"
CHECKS_RUN = re.compile(r"^(\.github/|hooks/|tools/|builder/[^/]*\.mjs$)")      # code the host's checks run, which a pull request runs as its own
HTTP_STATUS = re.compile(rb"^HTTP/[0-9.]+ ([0-9]{3})\b")                   # the first line `gh api --include` prints


def from_root(path):
    """A path as a command run from the repository's root names it: from the root when it can be, else whole. A folder
    on another drive than the repository has no path from the root (a host's temporary folder can be one)."""
    try:
        return os.path.relpath(path, ROOT).replace("\\", "/")
    except ValueError:
        return os.path.abspath(path).replace("\\", "/")


def run(words, log):
    log.write("\n$ " + " ".join(words) + "\n")
    log.flush()
    done = subprocess.run(words, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    if done.returncode:
        raise RuntimeError("%s answered %d (the build's log has the rest)" % (" ".join(words[:4]), done.returncode))


def read(path):
    with open(path, "rb") as f:
        return f.read()


def manifests_naming(blob_name):
    """[(a manifest's file name in builder/, the manifest)] for the manifests that name a loader's file: the page's own,
    loader.json, or the one kept beside it under the version's name."""
    out = []
    for name in ("loader.json", blob_name[:-len(".bin")] + ".json"):
        path = os.path.join(ROOT, "builder", name)
        if os.path.isfile(path):
            with open(path, encoding="utf-8") as f:
                manifest = json.load(f)
            if manifest.get("loader", {}).get("file") == blob_name:
                out.append((name, manifest))
    return out


def loaders_of(plan):
    """[(the loader's part of build.json, its builder's file's name, its profile, its name in packs/built.json as
    linked)]: the loader under `loader`, whose file build.json names under `blob`, and the builder's second loader
    under `loader_2`, which names its own `blob` and profile."""
    out = [(plan["loader"], plan["blob"], plan["profile"], "loader.bin")]
    if plan.get("loader_2"):
        second = plan["loader_2"]
        out.append((second, second.get("blob", ""), second.get("profile", plan["profile"]), "loader_2.bin"))
    return out


def manifest_faults(manifest, name, blob, entry, profile):
    """What a manifest of the builder says of the loader's file that this build does not: [a sentence]. `blob` is the
    built file's bytes, `entry` the address at which the linked loader is entered."""
    found = BLOB.match(name)
    loader, stock, output = manifest.get("loader", {}), manifest.get("stock", {}), manifest.get("output", {})
    wanted = (
        ("the OS version", manifest.get("os"), found.group(2)),
        ("the version's name", loader.get("version"), found.group(3)),
        ("the loader's file's size", loader.get("bytes"), len(blob)),
        ("the loader's file's SHA-256", loader.get("sha256"), hashlib.sha256(blob).hexdigest()),
        ("the address at which the loader is entered", loader.get("stub"), "0x%08x" % entry),
        ("the firmware file's SHA-256 it starts from", stock.get("sha256"), profile["stock_sha256"]),
        ("the firmware file's size it starts from", stock.get("bytes"), profile["stock_bytes"]),
        ("the size of the firmware file the builder gives", output.get("bytes"), profile["stock_bytes"] + RECORD_HEADER + len(blob)),
    )
    return ["%s: it says %s, and this build gives %s" % (what, said, built) for what, said, built in wanted if said != built]


_TOOLS = {}


def tool_module(name):
    """A tool of tools/ as a module, loaded once."""
    if name not in _TOOLS:
        spec = importlib.util.spec_from_file_location("build_all_" + name, os.path.join(ROOT, "tools", name + ".py"))
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        _TOOLS[name] = module
    return _TOOLS[name]


def base_facts(data):
    """What base-pack.json says of a base.pack file: its size, its SHA-256, and the version and the release the file
    says of itself, read by tools/pack_build.py as `check` reads them."""
    pack_build = tool_module("pack_build")
    fields, _body = pack_build.parse(data)
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest(), "version": fields["version"], "release": pack_build.release_in(data)}


def recipe_sources(recipe, root=ROOT):
    """(the file that lists a recipe's sources, or None where the recipe lists them itself; the sources), as paths
    from the root, as tools/pack_build.py read_recipe takes them."""
    sources = recipe.get("sources", [])
    if isinstance(sources, str):
        with open(os.path.join(root, *sources.split("/")), encoding="utf-8") as f:
            return sources, f.read().split()
    return None, list(sources)


def pack_script(profile, root=ROOT):
    """The linker script tools/pack_build.py links a pack with for a profile, as its use_profile() picks it: for a
    profile that names a reservation linker/<the profile's id>-pack.ld, else its firmware's,
    linker/op1f-<firmware_version>-pack.ld; for one that names none the tool's own (PACK_SCRIPT). "" when none is there."""
    names = []
    if "module_base" in profile and "reserved_bytes" in profile:
        names = ["%s-pack.ld" % profile.get("id"), "op1f-%s-pack.ld" % profile.get("firmware_version", "").split("+")[0]]
    names.append(os.path.basename(tool_module("pack_build").PACK_SCRIPT))
    for name in names:
        if os.path.isfile(os.path.join(root, "linker", name)):
            return "linker/" + name
    return ""


def base_input_files(profile_path, recipe_path=RECIPE, root=ROOT):
    """The files the build of the base reads, as paths from the root with forward slashes, sorted: the recipe (and the
    file that lists its sources, where it names one), each source, every file a C++ source takes in by #include "..."
    (from the source's own folder, then from module/, the folder tools/module_build.py is given), the pictures and the
    sounds the recipe names, the profile, the linker script tools/pack_build.py links a pack with for that profile,
    and the tools that make the bytes (BASE_TOOLS). A file named that is not there is left out: the build stops on one
    it needs."""
    def where(path):
        return os.path.join(root, *path.split("/"))

    with open(where(recipe_path), encoding="utf-8") as f:
        recipe = json.load(f)
    with open(where(profile_path), encoding="utf-8") as f:
        profile = json.load(f)
    listing, sources = recipe_sources(recipe, root)
    files = {recipe_path, profile_path, pack_script(profile, root), listing or ""} | set(BASE_TOOLS)
    files.update(item["file"] for item in recipe.get("pictures", []) if isinstance(item, dict) and isinstance(item.get("file"), str))
    card = recipe.get("card")
    for ask in (card.get("asks") or []) if isinstance(card, dict) else []:          # an engine's sounds, beside the recipe
        if isinstance(ask, dict) and ask.get("kind") == "engine" and isinstance(ask.get("sounds"), list):
            files.update(posixpath.normpath(posixpath.join(posixpath.dirname(recipe_path), sound["file"])) for sound in ask["sounds"]
                         if isinstance(sound, dict) and isinstance(sound.get("file"), str) and "text" not in sound)
    todo, seen = [source.replace("\\", "/") for source in sources], set()
    while todo:
        path = posixpath.normpath(todo.pop())
        if path in seen:
            continue
        seen.add(path)
        if path.endswith((".cpp", ".cc", ".h", ".hpp")) and os.path.isfile(where(path)):
            for name in INCLUDE.findall(read(where(path)).decode("utf-8", "replace")):
                for folder in (posixpath.dirname(path), "module"):
                    taken = posixpath.normpath(posixpath.join(folder, name))
                    if os.path.isfile(where(taken)):
                        todo.append(taken)
                        break
    files |= seen
    return sorted(path for path in files if path and os.path.isfile(where(path)))


def base_inputs(profile_path, recipe_path=RECIPE, root=ROOT):
    """The SHA-256 over the files the build of the base reads (base_input_files()), which packs/built.json keeps beside
    the released base as `inputs`: for each file in sorted order, its path from the root, a zero byte, its size in
    decimal digits, a zero byte, and its bytes as they are, no line ending changed."""
    digest = hashlib.sha256()
    for path in base_input_files(profile_path, recipe_path, root):
        data = read(os.path.join(root, *path.split("/")))
        digest.update(b"%s\0%d\0" % (path.encode("utf-8"), len(data)))
        digest.update(data)
    return digest.hexdigest()


def base_state(built, released, inputs):
    """What the base.pack a build made is to the released base, `released` (packs/built.json's entry: bytes, sha256,
    inputs): "same" when its bytes are the released ones; "ahead" when they differ and so do the files its build read
    (`inputs`, base_inputs()), which is the sources moved on since the release; "fault" when they differ from the same
    files, when the released base has no inputs recorded (then the build cannot tell sources ahead from a fault), or
    when there is no released base."""
    if not released:
        return "fault"
    if (released.get("bytes"), released.get("sha256")) == (len(built), hashlib.sha256(built).hexdigest()):
        return "same"
    if not released.get("inputs"):
        return "fault"
    return "ahead" if released["inputs"] != inputs else "fault"


def release_moves(old, new, recipe_path=RECIPE):
    """What a release changes in the base's recipe that is not in order, from the commit a pull request starts from
    (`old`, the recipe's bytes there) to the recipe as it stands (`new`): [a sentence]. A release gives the base a
    version above the one it had and a release other than the one it had."""
    before, after = json_of(old) or {}, json_of(new) or {}
    said = []
    if not (isinstance(before.get("version"), int) and isinstance(after.get("version"), int) and after["version"] > before["version"]):
        said.append("%s's version is %s, and a release gives the base a version above the one of the branch it merges into, %s" % (
            recipe_path, json.dumps(after.get("version")), json.dumps(before.get("version"))))
    if after.get("release") == before.get("release"):
        said.append("%s's release is %s, as in the one it merges into: a release names a release of its own" % (recipe_path, json.dumps(after.get("release"))))
    return said


def release_faults(released, root=ROOT):
    """What is not in order with the release's own files, each held to the released base, `released` (packs/built.json's
    entry of base.pack): builder/base.pack is that file, builder/base-pack.json says what the file says of itself, and
    the listing's entry, catalog/packs/base.json, its size, its SHA-256 and its version. [a sentence]"""
    page = os.path.join(root, "builder", "base.pack")
    if not released:
        return ["packs/built.json keeps no base.pack: there is no released base"]
    if not os.path.isfile(page):
        return ["builder/base.pack is not there"]
    data = read(page)
    if (len(data), hashlib.sha256(data).hexdigest()) != (released.get("bytes"), released.get("sha256")):
        return ["builder/base.pack is not the released base, packs/built.json's base.pack"]
    facts, faults = dict(base_facts(data), file="base.pack"), []
    for name, wanted in (("builder/base-pack.json", facts), ("catalog/packs/base.json", {key: facts[key] for key in ("sha256", "bytes", "version")})):
        try:
            with open(os.path.join(root, *name.split("/")), encoding="utf-8") as f:
                said = json.load(f)
        except (OSError, ValueError) as problem:
            faults.append("%s cannot be read: %s" % (name, problem))
            continue
        faults += ["%s says %s %s, and the released base is %s" % (name, key, json.dumps(said.get(key)), json.dumps(value))
                   for key, value in wanted.items() if said.get(key) != value]
    return faults


def labelled(text, label):
    """The value a source gives a name of its data: the number of the `.long`, or the text of the `.ascii` or
    `.asciz`, on the line behind `label:`, as written; None where the source does not give it one."""
    found = re.search(r"(?m)^%s:[ \t]*\r?\n[ \t]*\.(?:long|ascii|asciz)[ \t]+(\S[^\r\n]*?)[ \t]*\r?$" % re.escape(label), text)
    return found.group(1) if found else None


def json_of(data):
    try:
        return json.loads(data.decode("utf-8")) if data is not None else None
    except ValueError:
        return None


def fourth(value):
    """The fourth word of a `.long` line's list: the pack's version in a directory of pictures."""
    words = [word.strip() for word in value.split(",")]
    return words[3] if len(words) > 3 else value


def release_labels(name):
    """[(a name of the data the base's sources hold its version or its release in, what it holds, the part of the
    value that holds it)]: the door's version, the release, and the version in the directory of pictures (the pack's
    name `name`), which tools/pack_build.py pictures writes from the recipe beside the pictures' sizes."""
    return [(VERSION_LABEL, "version", None), (RELEASE_LABEL, "release", None),
            (tool_module("pack_build").art_symbol(name), "version, in its directory of pictures,", fourth)]


def release_changes(changed, old, new, recipe_path=RECIPE):
    """What a pull request changes of the release's own files: [a sentence]. `changed` holds the paths it changes;
    old(path) and new(path) answer a file's bytes in the commit it starts from and as it stands, or None where it is
    not there. Its own are: the files of RELEASE_OWN, packs/built.json's entry of base.pack, and the base's version and
    release, in its recipe and in the recipe's sources that hold them (release_labels())."""
    said = ["%s changed: it is the release's own file (the released base the install page serves, or what names it); %s" % (json.dumps(path), REFUSED)
            for path in RELEASE_OWN if path in changed]
    if "packs/built.json" in changed:
        before, after = [((json_of(read_it("packs/built.json")) or {}).get("files") or {}).get(BASE) for read_it in (old, new)]
        if before != after:
            said.append("packs/built.json's entry of base.pack changed: it is the released base's size, SHA-256 and inputs; %s" % REFUSED)
    recipes = [json_of(read_it(recipe_path)) or {} for read_it in (old, new)]
    if recipe_path in changed:
        for key in ("version", "release"):
            if recipes[0].get(key) != recipes[1].get(key):
                said.append("%s changes the base's %s from %s to %s; %s" % (json.dumps(recipe_path), key, json.dumps(recipes[0].get(key)), json.dumps(recipes[1].get(key)), REFUSED))
    sources = set()
    for recipe in recipes:
        listed = recipe.get("sources")
        sources.update(listed if isinstance(listed, list) else [])
    labels = release_labels(recipes[1].get("name") or recipes[0].get("name") or "base")
    for path in sorted(path for path in changed & sources if path.endswith(".s")):           # the numbers are assembly data
        before, after = [(read_it(path) or b"").decode("utf-8", "replace") for read_it in (old, new)]
        for label, what, pick in labels:
            values = [labelled(text, label) for text in (before, after)]
            values = [pick(value) if pick and value is not None else value for value in values]
            if values[0] != values[1]:
                said.append("%s changes the base's %s (%s) from %s to %s; %s" % (json.dumps(path), what, label, values[0], values[1], REFUSED))
    return said


def git(words, root=ROOT):
    return subprocess.run(["git", "-C", root] + words, capture_output=True)


def pull_request_files(base_sha, root=ROOT):
    """(the commit a pull request into `base_sha` starts from, the paths it changes from there to the files as they
    stand, old(path), new(path)) for release_changes(). The commit is the one the two have in common, as `git diff
    BASE_SHA...HEAD` takes it; the files as they stand are HEAD's on a host, and a change not committed yet counts too."""
    done = git(["merge-base", base_sha, "HEAD"], root)
    if done.returncode:
        raise RuntimeError("git finds no commit in common with %s: it takes the whole history (actions/checkout with fetch-depth: 0)" % base_sha)
    start = done.stdout.decode("ascii").strip()
    done = git(["diff", "--name-only", "--no-renames", "-z", start], root)
    if done.returncode:
        raise RuntimeError("git diff %s answered %d: %s" % (start, done.returncode, done.stderr.decode("utf-8", "replace").strip()))
    changed = {path for path in done.stdout.decode("utf-8").split("\0") if path}

    def old(path):
        shown = git(["show", "%s:%s" % (start, path)], root)
        return shown.stdout if not shown.returncode else None

    def new(path):
        full = os.path.join(root, *path.split("/"))
        return read(full) if os.path.isfile(full) else None

    return start, changed, old, new


def api_answer(output):
    """(the HTTP status, the body) of what `gh api --include` printed: its first line is the status line, and the body
    stands behind the first empty line. (None, b"") for output with no status line."""
    found = HTTP_STATUS.match(output)
    if not found:
        return None, b""
    ends = [(at, len(mark)) for mark in (b"\r\n\r\n", b"\n\n") for at in [output.find(mark)] if at >= 0]
    at, size = min(ends) if ends else (len(output), 0)
    return int(found.group(1)), output[at + size:]


def gh_api(path, raw=False):
    """The host's API through `gh api`, with the environment's GH_TOKEN: the answer's bytes (a file's own bytes with
    `raw`), or None where the API's HTTP status is 404, a file that is not there."""
    done = subprocess.run(["gh", "api", "--include"] + (["-H", "Accept: application/vnd.github.raw"] if raw else []) + [path], capture_output=True)
    status, body = api_answer(done.stdout)
    if status == 404:
        return None
    if done.returncode or status != 200:
        problem = done.stderr.decode("utf-8", "replace").strip().splitlines()
        raise RuntimeError("gh api %s answered %d, HTTP status %s: %s" % (json.dumps(path), done.returncode, status, json.dumps(problem[-1] if problem else "")))
    return body


def guard(number, repository, api=gh_api, log=print, head=None):
    """--guard: the refusal of --pull-request from a fork, with main's own code, for the pull request `number` of
    `repository` at the commit `head` (the event's head; the pull request's head as the API gives it when None), read
    through the host's API as data and never checked out: its files (pulls/<n>/files, a renamed file under both
    names, all of them or a refusal), the commit it starts from (compare/<base>...<head>, the merge base), and the
    few files release_changes() reads, at the head and at the merge base, as raw contents from the head's repository.
    A branch of this repository passes: its own checks hold a release whole. Says which changed files are code the
    checks run. Every path of the pull request is written as JSON, so that no name of a file starts a line of the log.
    Answers [a refusal]."""
    def get(path):
        answer = api(path)
        if answer is None:
            raise RuntimeError("the host's API has no %s" % json.dumps(path))
        return json.loads(answer.decode("utf-8"))

    pull = get("repos/%s/pulls/%d" % (repository, number))
    head_repository = ((pull.get("head") or {}).get("repo") or {}).get("full_name")
    now = (pull.get("head") or {}).get("sha")
    if head and now != head:
        raise RuntimeError("the pull request's head is %s now, and this run judges %s: the run of the new head judges it" % (json.dumps(now), json.dumps(head)))
    head = head or now
    changed, read_in, page = set(), 0, 1
    while True:
        files = get("repos/%s/pulls/%d/files?per_page=100&page=%d" % (repository, number, page))
        read_in += len(files)
        for item in files:
            changed.add(item["filename"])
            if item.get("previous_filename"):
                changed.add(item["previous_filename"])
        if len(files) < 100:
            break
        page += 1
    for path in sorted(path for path in changed if CHECKS_RUN.match(path)):
        log("code the checks run, changed by this pull request: %s" % json.dumps(path))
    if head_repository == repository:
        log("a branch of this repository: the guard lets it through, and its own checks hold a release whole")
        return []
    if not head_repository:
        raise RuntimeError("the pull request's head repository is not there any more: its files cannot be read")
    refusals = []
    stated = pull.get("changed_files")
    if not isinstance(stated, int) or read_in < stated:
        refusals.append("the host's API gives %d of the pull request's %s changed files (its list stops at 3000): the guard cannot read them all, "
                        "so it refuses" % (read_in, json.dumps(stated)))
    start = get("repos/%s/compare/%s...%s" % (repository, pull["base"]["sha"], head))["merge_base_commit"]["sha"]

    def at(commit):
        return lambda path: api("repos/%s/contents/%s?ref=%s" % (head_repository, urllib.parse.quote(path, safe="/"), commit), raw=True)

    log("the pull request #%d from %s at %s, counted from %s" % (number, json.dumps(head_repository), head[:7], start[:7]))
    return refusals + release_changes(changed, at(start), at(head))


def release_numbers(version, release, recipe_path=RECIPE, root=ROOT):
    """The release step's first part, read and checked before anything is written: {path from the root: its new
    bytes} for the base's recipe with `version` and `release`, the source that gives VERSION_LABEL its number, and the
    two sources tools/pack_build.py writes from the recipe that hold one of them: the release's (`release`) and the
    directory of pictures (`pictures`), which holds the version. Raises ValueError."""
    pack_build = tool_module("pack_build")
    if not pack_build.RELEASE_FORM.fullmatch(release):
        raise ValueError("a release is one to eight characters of 0-9, A-Z, a-z, '.', '_' and '-': %s" % release)

    def where(path):
        return os.path.join(root, *path.split("/"))

    recipe = json.loads(read(where(recipe_path)).decode("utf-8"))
    if "version" not in recipe or "release" not in recipe:
        raise ValueError("%s names no version or no release" % recipe_path)
    if recipe.get("card") is not None:
        raise ValueError("%s names a card, which holds the version too: the release step writes no card" % recipe_path)
    _listing, sources = recipe_sources(recipe, root)
    texts = {path: read(where(path)).decode("utf-8", "replace") for path in sources if path.endswith(".s")}
    art = pack_build.art_symbol(recipe["name"])
    holders = {}
    for label in (VERSION_LABEL, RELEASE_LABEL) + ((art,) if recipe.get("pictures") else ()):
        holders[label] = [path for path, text in texts.items() if labelled(text, label) is not None]
        if len(holders[label]) != 1:
            raise ValueError("%d of the base's sources give %s a value: the release step writes it into exactly one" % (len(holders[label]), label))
    recipe["version"], recipe["release"] = int(version), release
    out = {recipe_path: (json.dumps(recipe, indent=2, ensure_ascii=False) + "\n").encode("utf-8")}
    door = holders[VERSION_LABEL][0]
    text, count = re.subn(r"(?m)^(%s:[ \t]*\r?\n[ \t]*\.long[ \t]+)[^\s;/]+" % re.escape(VERSION_LABEL),
                          lambda found: found.group(1) + str(int(version)), read(where(door)).decode("utf-8"))
    if count != 1:
        raise ValueError("%s gives %s no number on a .long line behind it" % (door, VERSION_LABEL))
    out[door] = text.encode("utf-8")
    made = [(RELEASE_LABEL, "release", pack_build.render_release)] + ([(art, "pictures", pack_build.render_pictures)] if recipe.get("pictures") else [])
    for label, command, render in made:
        source = holders[label][0]
        written = render(recipe, recipe_path)
        if not texts[source].startswith(written.split(" from ", 1)[0] + " from "):
            raise ValueError("%s is no source `tools/pack_build.py %s` wrote: the release step writes only such a one" % (source, command))
        out[source] = written.encode("utf-8")
    return out


def write_files(files):
    """{path from the root: bytes}, each written as it is."""
    for path, data in files.items():
        with open(os.path.join(ROOT, *path.split("/")), "wb") as f:
            f.write(data)


def write_page_base(built):
    """The release step: the released base copied into builder/, and builder/base-pack.json written anew."""
    named = {}
    if os.path.isfile(PAGE_BASE_NAMED):
        with open(PAGE_BASE_NAMED, encoding="utf-8") as f:
            named = json.load(f)
    record = {"about": named.get("about", "base.pack, the released base, which the install page puts on the unit with the new firmware file."), "file": "base.pack"}
    record.update(base_facts(built))
    with open(PAGE_BASE, "wb") as f:
        f.write(built)
    with open(PAGE_BASE_NAMED, "w", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(record, indent=2) + "\n")


def write_catalog_base(built):
    """The release step: the listing's entry of the base, catalog/packs/base.json, given the released base's size,
    SHA-256 and version, and the places and the level it offers as tools/catalog_check.py reads them from the file.
    Answers what could not be written, or None."""
    with open(CATALOG_BASE, encoding="utf-8") as f:
        entry = json.load(f)
    facts = base_facts(built)
    entry.update(sha256=facts["sha256"], bytes=facts["bytes"], version=facts["version"])
    left = None
    try:
        fields, _body = tool_module("pack_build").parse(built)
        level = tool_module("catalog_check").level_of([(name, True) for name, _offset in fields["lists"]["exports"]], False)
        entry.update(places=fields["entries"], offers_level=level)
    except Exception as problem:                                  # the listing's check says what is wrong with them
        left = "its places and offers_level are left as they were (%s)" % problem
    with open(CATALOG_BASE, "w", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(entry, indent=2, ensure_ascii=False) + "\n")
    return left


RELEASE_WRITES = (KEPT, PLAN, PAGE_BASE, PAGE_BASE_NAMED, CATALOG_BASE)   # what the release of the base writes besides the numbers


def main(argv=None):
    """The tool. A release of the base that does not end in 0 (its build failed, or a later step did, or it was
    stopped) puts back every file it wrote, the numbers in the sources among them, as they were."""
    taken = {}                                                    # {a file's whole path: its bytes before the release, or None}
    try:
        answer = hold(argv, taken)
    except BaseException:
        if taken:
            put_back(taken)
        raise
    if answer and taken:
        put_back(taken)
    return answer


def put_back(taken):
    for path, data in taken.items():
        if data is None:
            if os.path.isfile(path):
                os.remove(path)
            continue
        with open(path, "wb") as f:
            f.write(data)
    print("the release of the base did not finish, so every file it wrote is put back as it was: %s" % ", ".join(sorted(from_root(path) for path in taken)))


def hold(argv, taken):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", default="out", help="the folder for the finished files (new or empty)")
    ap.add_argument("--update", action="store_true", help="write the size and SHA-256 of every pack file that differs into packs/built.json")
    ap.add_argument("--loader-version", metavar="NAME", help="with --update, for a changed loader: the new version's name (v and a number) for the builder's file")
    ap.add_argument("--base-version", metavar="N", type=int, help="with --update and --release, the release of the base: its new version")
    ap.add_argument("--release", metavar="X.Y", help="with --update and --base-version, the release of the base: what the start screen shows behind open-field")
    ap.add_argument("--pull-request", metavar="BASE_SHA", help="the check of a pull request into the commit BASE_SHA: also refuses a change to the release's own files")
    ap.add_argument("--release-allowed", action="store_true", help="with --pull-request, for a branch of this repository: a change to the release's own files is let through when the release is whole")
    ap.add_argument("--guard", metavar="NUMBER", type=int, help="the guard of the release's own files for the pull request NUMBER, read through the host's API: builds nothing")
    ap.add_argument("--repository", metavar="OWNER/NAME", help="with --guard: the repository (the environment's GITHUB_REPOSITORY by default)")
    ap.add_argument("--head", metavar="SHA", help="with --guard: the commit to judge, the event's head; the guard stops when the pull request has moved on")
    args = ap.parse_args(argv)
    if not os.path.samefile(os.getcwd(), ROOT):
        print("error: run this from the repository's root: %s" % ROOT)
        return 2
    if args.head and args.guard is None:
        print("error: --head goes with --guard")
        return 2
    if args.guard is not None:
        repository = args.repository or os.environ.get("GITHUB_REPOSITORY", "")
        if args.update or args.pull_request or args.release_allowed or args.base_version is not None or args.release is not None:
            print("error: --guard reads a pull request and builds nothing: it goes with --repository alone")
            return 2
        if not repository:
            print("error: --guard needs the repository: --repository OWNER/NAME, or the environment's GITHUB_REPOSITORY")
            return 2
        try:
            refusals = guard(args.guard, repository, head=args.head)
        except (RuntimeError, ValueError, KeyError, TypeError) as problem:
            print("error: %s" % problem)
            return 2
        for refusal in refusals:
            print("REFUSED   %s" % refusal)
        print("the guard: %s" % ("%d change(s) of the release's own files refused" % len(refusals) if refusals else "the release's own files are left as they are"))
        return 1 if refusals else 0
    if args.loader_version and not args.update:
        print("error: --loader-version goes with --update")
        return 2
    if args.loader_version and not re.match(r"^v[0-9]+$", args.loader_version):
        print("error: a version's name is v and a number, such as v4: %s" % args.loader_version)
        return 2
    releasing = args.base_version is not None or args.release is not None
    if releasing and (args.base_version is None or args.release is None or not args.update):
        print("error: the release of the base takes all three: --update --base-version <N> --release <X.Y>")
        return 2
    if args.release_allowed and not args.pull_request:
        print("error: --release-allowed goes with --pull-request")
        return 2
    if args.pull_request and args.update:
        print("error: --pull-request checks and writes nothing: it does not go with --update")
        return 2
    out = os.path.abspath(args.out)
    if os.path.isdir(out) and os.listdir(out):
        print("error: %s is not empty: this tool overwrites nothing" % out)
        return 2
    with open(PLAN, encoding="utf-8", newline="") as f:
        plan_text = f.read()
    plan = json.loads(plan_text)
    with open(KEPT, encoding="utf-8") as f:
        record = json.load(f)
    kept = record["files"]
    loaders = loaders_of(plan)
    for _loader, name, _profile, _linked in loaders:
        if not BLOB.match(name):
            print("error: build.json names the builder's file %s: its name has the form loader-<os>-<version>.bin" % name)
            return 2
    for needed in (os.path.join("open-xy", "tooling", "formats.py"), os.path.join("open-xy", "local", "toolchain", "bin")):
        if not os.path.exists(os.path.join(ROOT, needed)):
            print("error: %s is not there: docs/build.md says how to get the toolchain" % needed)
            return 2
    changes, start = [], None
    if args.pull_request:
        try:
            start, changed, old, new = pull_request_files(args.pull_request)
        except RuntimeError as problem:
            print("error: %s" % problem)
            return 2
        changes = release_changes(changed, old, new)
    profile_path = plan["profile"]
    if releasing:
        released_version = base_facts(read(PAGE_BASE))["version"] if os.path.isfile(PAGE_BASE) else 0
        if args.base_version <= released_version:
            print("error: base %d does not come after the released base, version %d: a version keeps its bytes for good" % (args.base_version, released_version))
            return 2
        try:
            numbers = release_numbers(args.base_version, args.release)
        except (OSError, ValueError) as problem:
            print("error: %s" % problem)
            print("       nothing was written")
            return 2
        for path in [os.path.join(ROOT, *name.split("/")) for name in numbers] + list(RELEASE_WRITES):    # main() puts them back on a failure
            taken[path] = read(path) if os.path.isfile(path) else None
        write_files(numbers)
        print("the base's version %d and release %s are written into %s" % (args.base_version, args.release, ", ".join(sorted(numbers))))
    packs, examples, build = os.path.join(out, "packs"), os.path.join(out, "examples"), os.path.join(ROOT, "build")
    os.makedirs(packs, exist_ok=True)
    os.makedirs(build, exist_ok=True)
    python = [sys.executable, "-I"]
    log_path = os.path.join(out, "build.log")
    made = {}
    try:
        with open(log_path, "w", encoding="utf-8") as log:
            run(python + ["tools/table_build.py", "--profile", profile_path, "--check"], log)
            run(python + ["tools/change_list.py", "--profile", plan.get("change_list", profile_path), "--check"], log)     # the profile of the loader the page offers by itself
            run(python + ["tools/interfaces.py", "--check"], log)
            for example in plan.get("examples", ()):
                if example.get("profile"):                               # an example that changes places of the firmware has a profile of its own
                    run(python + ["tools/table_build.py", "--profile", example["profile"], "--check"], log)
            recipes = []
            for name in plan["packs"]:
                recipes += ["--recipe", "packs/%s.json" % name]
            run(python + ["tools/pack_build.py", "build3"] + recipes + ["--profile", profile_path, "--out-dir", from_root(packs)], log)
            for example in plan.get("examples", ()):
                target = os.path.join(examples, example["name"] + ".pack")
                run(python + ["tools/pack_build.py", "build3", "--recipe", example["recipe"], "--profile", example.get("profile", profile_path),
                              "--out", from_root(target)], log)
                made["examples/%s.pack" % example["name"]] = target
            for loader, name, _profile, _linked in loaders:
                run(python + ["tools/module_build.py", "--name", loader["name"], "--out", "build", "--linker", loader["linker"], "--include", "module"] + loader["sources"], log)
                with open(os.path.join(out, name), "xb") as f:            # the builder's file is the loader's binary itself
                    f.write(read(os.path.join(build, loader["name"] + ".bin")))
    except RuntimeError as problem:
        print("error: %s" % problem)
        print("the build's log: %s" % log_path)
        return 2

    def is_kept(name):
        data = read(os.path.join(out, name))
        return name in kept and (kept[name]["bytes"], kept[name]["sha256"]) == (len(data), hashlib.sha256(data).hexdigest())

    # the loader a new version's name goes to: the one whose build is not its kept file
    changed = [number for number, (_loader, name, _profile, _linked) in enumerate(loaders) if not is_kept(name)]
    which = 0
    if args.loader_version and not changed:
        # asked for a new version of a loader that did not change: nothing is renamed
        print("the loader is the kept one: no new version is needed, and --loader-version is passed over")
    elif args.loader_version and len(changed) > 1:
        print("error: both loaders of build.json changed (%s): a version's name is for one loader" % ", ".join(loaders[number][1] for number in changed))
        print("       nothing was written into packs/built.json or build.json")
        return 2
    elif changed:
        which = changed[0]
    loader, kept_name, which_profile, linked = loaders[which]            # the builder's file as build.json names it now
    blob_name = kept_name
    if args.loader_version and changed:
        blob_name = "%s-%s.bin" % (BLOB.match(kept_name).group(1), args.loader_version)
        if blob_name != kept_name and blob_name in [name for _loader, name, _profile, _linked in loaders]:
            print("error: build.json names %s for its other loader: give the changed loader a new name with --loader-version" % blob_name)
            print("       nothing was written into packs/built.json or build.json")
            return 2
        os.replace(os.path.join(out, kept_name), os.path.join(out, blob_name))
        loaders[which] = (loader, blob_name, which_profile, linked)
    blob = os.path.join(out, blob_name)
    blob_data = read(blob)
    renamed = blob_name != kept_name
    in_builder = os.path.join(ROOT, "builder", blob_name)
    if args.loader_version and os.path.isfile(in_builder) and read(in_builder) != blob_data:
        # a version that has run on a unit keeps its bytes for good: its name is not given to another loader
        for name, manifest in manifests_naming(blob_name):
            said = manifest["loader"].get("status")
            if said and said != NOT_RUN:
                print("error: builder/%s is there with other bytes, and builder/%s says of it: %s" % (blob_name, name, said))
                print("       a version that has run on a unit keeps its bytes for good: give the changed loader a new name with --loader-version")
                print("       nothing was written into packs/built.json or build.json")
                return 2
    made.update({name + ".pack": os.path.join(packs, name + ".pack") for name in plan["packs"]})
    for each, name, _profile, as_linked in loaders:
        made[as_linked] = os.path.join(build, each["name"] + ".bin")
        made[name] = os.path.join(out, name)
    of_the_loader = (linked, blob_name)                                   # the loader a new version's name goes to
    of_a_loader = {name for _loader, blob_file, _profile, as_linked in loaders for name in (as_linked, blob_file)}
    wrong, astray, renewed, loader_differs = 0, 0, [], False     # built files that differ; faults of the builder's own files
    differing = set()                                             # the names of the built files that differ
    inputs = base_inputs(profile_path)                            # the files the base's build read
    base_said = None                                              # what the built base.pack is to the released one
    for name in sorted(made):
        data = read(made[name])
        digest = hashlib.sha256(data).hexdigest()
        if name == BASE:                                          # the base: held to the released base, packs/built.json's
            released = kept.get(BASE)
            base_said = base_state(data, released, inputs)
            if releasing:
                kept[BASE] = {"bytes": len(data), "sha256": digest, "inputs": inputs}
                renewed.append(name)
                print("%-9s %-26s %8d bytes  %s" % ("released", name, len(data), digest))
            elif base_said == "ahead":
                print("%-9s %-26s %8d bytes  %s, and the released base is %d bytes  %s: the sources are ahead of it, and a release makes it the page's"
                      % ("ahead", name, len(data), digest, released["bytes"], released["sha256"]))
            elif base_said == "fault":
                wrong += 1
                differing.add(name)
                print("%-9s %-26s %8d bytes  %s" % ("DIFFERS", name, len(data), digest))
                print("          %s" % ("packs/built.json keeps no base.pack" if not released else
                                        "no inputs are recorded for the released base in packs/built.json, so the build cannot tell sources ahead "
                                        "of the release from a fault: the release step writes them, %s" % RELEASE_STEP if not released.get("inputs") else
                                        "the files its build read are the released base's (packs/built.json's inputs), and they build other bytes: "
                                        "the build is not the one the base was released with"))
            else:
                print("%-9s %-26s %8d bytes  %s" % ("the same", name, len(data), digest))
            continue
        same = name in kept and kept[name]["bytes"] == len(data) and kept[name]["sha256"] == digest
        word = "the same"
        if not same and args.update and (name.endswith(".pack") or (args.loader_version and name in of_the_loader)):
            if name == blob_name and renamed:                             # the new name takes the place of the old one
                record["files"] = kept = {(name if key == kept_name else key): value for key, value in kept.items()}
            kept[name] = {"bytes": len(data), "sha256": digest}
            renewed.append(name)
            word = "updated"
        elif not same:
            wrong += 1
            word = "DIFFERS"
            loader_differs = loader_differs or name in of_a_loader
            differing.add(name)
        print("%-9s %-26s %8d bytes  %s" % (word, name, len(data), digest))
    if renewed:
        with open(KEPT, "w", encoding="utf-8", newline="\n") as f:
            f.write(json.dumps(record, indent=1) + "\n")
        print("packs/built.json has the new size and SHA-256 of: %s" % ", ".join(renewed))
        if [name for name in renewed if name.endswith(".pack") and name != BASE]:
            print("give each of these packs a new version number in its recipe, if you have not yet")
    if args.update and not releasing and base_said == "ahead":
        print("base.pack is left as it was released: the release of the base renews it, %s" % RELEASE_STEP)
    if renamed:
        old, new = '"blob": "%s"' % kept_name, '"blob": "%s"' % blob_name
        if plan_text.count(old) != 1:
            print("error: build.json does not hold %s exactly once: nothing was renamed in it" % old)
            return 2
        with open(PLAN, "w", encoding="utf-8", newline="") as f:
            f.write(plan_text.replace(old, new))
        print("build.json names the builder's file %s now (%s before)" % (blob_name, kept_name))

    # the builder's own two files of each loader: the loader's file, and the manifest that names it
    for each, blob_name, each_profile, as_linked in loaders:
        blob, in_builder = os.path.join(out, blob_name), os.path.join(ROOT, "builder", blob_name)
        blob_data = read(blob)
        version = BLOB.match(blob_name).group(3)
        own_profile = "" if each_profile == profile_path else " --profile %s" % each_profile     # a loader with a profile of its own names it
        next_step = "node builder/write-manifest.mjs --loader %s --elf build/%s.elf --version %s%s --stock <your own os.ldr>" % (
            from_root(blob), each["name"], version, own_profile)
        if not os.path.isfile(in_builder):
            astray += 1
            print("MISSING   builder/%s and its manifest: write them with your own firmware file:" % blob_name)
            print("          %s" % next_step)
        elif read(in_builder) != blob_data:
            astray += 1
            print("DIFFERS   builder/%s is not the file that was built" % blob_name)
            if not (differing & {blob_name, as_linked}):
                print("          if %s is a version of your own that nobody else has, write it again:" % version)
                print("          %s --replace" % next_step)
        else:
            print("the same  builder/%s, byte for byte" % blob_name)
            with open(os.path.join(ROOT, *each_profile.split("/")), encoding="utf-8") as f:
                profile = json.load(f)
            entry = struct.unpack_from("<I", read(os.path.join(build, each["name"] + ".elf")), 24)[0]     # where the linked loader says it is entered
            naming = manifests_naming(blob_name)
            if not naming:
                astray += 1
                print("MISSING   a manifest in builder/ that names %s: write it with your own firmware file:" % blob_name)
                print("          %s" % next_step)
            for name, manifest in naming:
                faults = manifest_faults(manifest, blob_name, blob_data, entry, profile)
                if faults:
                    astray += 1
                    print("DIFFERS   builder/%s is not this build's manifest" % name)
                    for fault in faults:
                        print("          %s" % fault)
                else:
                    print("the same  builder/%s names it: its size, its SHA-256, and the loader's entry at 0x%08x" % (name, entry))
    # the release's own files: builder/base.pack, the base the install page puts on every unit, builder/base-pack.json
    # that names it, and the listing's entry, each held to the released base, packs/built.json's base.pack
    if releasing:
        write_page_base(read(made[BASE]))
        left = write_catalog_base(read(made[BASE]))
        print("builder/base.pack, builder/base-pack.json and catalog/packs/base.json are base %d, release %s, now: once this change is merged, "
              "the install page puts it on every unit it installs" % (args.base_version, args.release))
        if left:
            print("catalog/packs/base.json: %s" % left)
    base_faults = release_faults(kept.get(BASE))
    if base_faults:
        astray += 1
        for fault in base_faults:
            print("DIFFERS   %s" % fault)
        print("          the release's own files are the released base's, and the release of the base writes them all: %s" % RELEASE_STEP)
    else:
        print("the same  builder/base.pack, byte for byte, and builder/base-pack.json names it: the released base, packs/built.json's, which catalog/packs/base.json names too")
    refused = 0
    if args.pull_request:
        whole = []
        if changes and args.release_allowed:
            print("the release's own files changed, and --release-allowed lets them: the release must be whole")
            if base_said != "same":
                whole.append("the build's base.pack is not packs/built.json's: a release keeps the two together, %s" % RELEASE_STEP)
            if (kept.get(BASE) or {}).get("inputs") != inputs:
                whole.append("packs/built.json's inputs of base.pack are not the sources' (%s): the release was not made from these sources, %s" % (inputs, RELEASE_STEP))
            if "builder/base.pack" in changed:                    # a new base released: a new version and release
                whole += release_moves(old(RECIPE), new(RECIPE))
        else:
            whole = changes
        for fault in whole:
            refused += 1
            print("REFUSED   %s" % fault)
        if not whole:
            print("the same  the release's own files%s" % (" are a whole release" if changes else ", as %s has them" % start[:7]))
    if loader_differs:
        print("the loader changed. Such a change gets a new version name for the builder's file:")
        print("          python -I tools/build_all.py --out <a new folder> --update --loader-version <name>")
        print("          (docs/build.md, \"A change to the loader\")")
    ahead = base_said == "ahead" and not releasing
    print("%d file(s) built: %s%s%s%s" % (len(made), "each is the kept one" if not wrong and not renewed else
                                          "%d renewed in packs/built.json, the others are the kept ones" % len(renewed) if not wrong else
                                          "%d differ from the kept ones" % wrong,
                                          " but base.pack, whose sources are ahead of the released base" if ahead and not wrong else "",
                                          "; the builder's own files are not this build's yet" if astray else "",
                                          "; %d change(s) of the release's own files refused" % refused if refused else ""))
    return 1 if wrong or astray or refused else 0


if __name__ == "__main__":
    sys.exit(main())

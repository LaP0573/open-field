#!/usr/bin/env python3
"""Build everything this repository builds, and hold the results to the kept sizes and hashes.

  python -I tools/build_all.py [--out out] [--update [--loader-version v4]]

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
  7. holds builder/base.pack, the base the install page puts on every unit it installs, byte for byte to the
     base.pack of step 2, and builder/base-pack.json, which the page holds that file to, to the same file: its size,
     its SHA-256, and the version and the release the file says of itself.

Answers 0 when every result is the kept one, 1 when one is not, 2 when it could not build. Writes only under the
folder --out names (which must be new or empty) and under build/, and with --update the files named below.

`--update` is for a change that is meant to change what is built. After the build it writes the new size and SHA-256
of every pack file that differs into packs/built.json, and says which. Give the pack a new version number in its
recipe in the same change. When base.pack is one of them, it also copies the new base.pack into builder/ and writes
builder/base-pack.json anew: once the change is merged, the install page puts that base.pack on every unit it
installs, so say in the change whether it ran on a unit.

`--update --loader-version <name>` is for a change to the loader, which is the builder's file. A change to the base
(base.pack) is a change to a pack like any other: the base is a file on the unit, never inside the firmware file. A
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
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEPT = os.path.join(ROOT, "packs", "built.json")
PLAN = os.path.join(ROOT, "build.json")
PAGE_BASE = os.path.join(ROOT, "builder", "base.pack")                   # the base the install page puts on every unit
PAGE_BASE_NAMED = os.path.join(ROOT, "builder", "base-pack.json")        # what the page holds that file to
BLOB = re.compile(r"^(loader-([0-9]+(?:\.[0-9]+)*))-(v[0-9]+)\.bin$")     # the builder's file: loader-<os>-<version>.bin
RECORD_HEADER = 16                                                        # what the builder puts before the loader in the firmware file
NOT_RUN = "not run on a unit yet"                                         # what builder/write-manifest.mjs says of a version nobody has tried


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


def base_facts(data):
    """What base-pack.json says of a base.pack file: its size, its SHA-256, and the version and the release the file
    says of itself, read by tools/pack_build.py as `check` reads them."""
    spec = importlib.util.spec_from_file_location("build_all_pack_build", os.path.join(ROOT, "tools", "pack_build.py"))
    pack_build = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pack_build)
    fields, _body = pack_build.parse(data)
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest(), "version": fields["version"], "release": pack_build.release_in(data)}


def page_base_faults(built):
    """What is not in order with the install page's own base.pack and the file that names it, held to the base.pack
    this build made: [a sentence]."""
    if not os.path.isfile(PAGE_BASE):
        return ["builder/base.pack is not there"]
    if read(PAGE_BASE) != built:
        return ["builder/base.pack is not the base.pack that was built"]
    try:
        with open(PAGE_BASE_NAMED, encoding="utf-8") as f:
            named = json.load(f)
    except (OSError, ValueError) as problem:
        return ["builder/base-pack.json cannot be read: %s" % problem]
    facts = dict(base_facts(built), file="base.pack")
    return ["builder/base-pack.json says %s %s, and the file is %s" % (key, json.dumps(named.get(key)), json.dumps(value))
            for key, value in facts.items() if named.get(key) != value]


def write_page_base(built):
    """--update with a new base.pack: the file copied into builder/, and builder/base-pack.json written anew."""
    named = {}
    if os.path.isfile(PAGE_BASE_NAMED):
        with open(PAGE_BASE_NAMED, encoding="utf-8") as f:
            named = json.load(f)
    record = {"about": named.get("about", "base.pack, which the install page puts on the unit with the new firmware file."), "file": "base.pack"}
    record.update(base_facts(built))
    with open(PAGE_BASE, "wb") as f:
        f.write(built)
    with open(PAGE_BASE_NAMED, "w", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(record, indent=2) + "\n")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", default="out", help="the folder for the finished files (new or empty)")
    ap.add_argument("--update", action="store_true", help="write the size and SHA-256 of every pack file that differs into packs/built.json")
    ap.add_argument("--loader-version", metavar="NAME", help="with --update, for a changed loader: the new version's name (v and a number) for the builder's file")
    args = ap.parse_args(argv)
    if not os.path.samefile(os.getcwd(), ROOT):
        print("error: run this from the repository's root: %s" % ROOT)
        return 2
    if args.loader_version and not args.update:
        print("error: --loader-version goes with --update")
        return 2
    if args.loader_version and not re.match(r"^v[0-9]+$", args.loader_version):
        print("error: a version's name is v and a number, such as v4: %s" % args.loader_version)
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
    packs, examples, build = os.path.join(out, "packs"), os.path.join(out, "examples"), os.path.join(ROOT, "build")
    os.makedirs(packs, exist_ok=True)
    os.makedirs(build, exist_ok=True)
    python = [sys.executable, "-I"]
    profile_path = plan["profile"]
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
    for name in sorted(made):
        data = read(made[name])
        digest = hashlib.sha256(data).hexdigest()
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
        if [name for name in renewed if name.endswith(".pack")]:
            print("give each of these packs a new version number in its recipe, if you have not yet")
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
    # the base the install page puts on every unit: builder/base.pack, and builder/base-pack.json that names it
    built_base = read(made["base.pack"])
    if "base.pack" in renewed:
        write_page_base(built_base)
        print("builder/base.pack and builder/base-pack.json are the new base.pack now: once this change is merged, the install page puts it on every unit it installs")
    base_faults = page_base_faults(built_base)
    if base_faults:
        astray += 1
        for fault in base_faults:
            print("DIFFERS   %s" % fault)
        print("          the install page's base.pack is the one this build makes: python -I tools/build_all.py --out <a new folder> --update writes both")
    else:
        print("the same  builder/base.pack, byte for byte, and builder/base-pack.json names it")
    if loader_differs:
        print("the loader changed. Such a change gets a new version name for the builder's file:")
        print("          python -I tools/build_all.py --out <a new folder> --update --loader-version <name>")
        print("          (docs/build.md, \"A change to the loader\")")
    print("%d file(s) built: %s%s" % (len(made), "each is the kept one" if not wrong and not renewed else
                                      "%d renewed in packs/built.json, the others are the kept ones" % len(renewed) if not wrong else
                                      "%d differ from the kept ones" % wrong,
                                      "; the builder's own files are not this build's yet" if astray else ""))
    return 1 if wrong or astray else 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Write the change list, docs/changes.md, from the profile.

  python -I tools/change_list.py --profile profiles/op1f-1.7.3.json [--out docs/changes.md]
  python -I tools/change_list.py --profile profiles/op1f-1.7.3.json --check

The change list says, for every place where open-field changes the firmware, where it is, what stands there and
what our code does there: in our own words, with no instruction of the firmware quoted and none of its bytes given.
It follows from the profile, so it is made and never written by hand. `--check` writes nothing and answers 1 when
the page in the tree is not what the profile gives. Standard library only.
"""

import argparse
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FORMS = {"word": "a word of data", "call": "a call", "jump": "a jump", "load": "the load of an address"}
PARTS = ("loader", "base", "8 tracks", "engines", "fx train", "lfo", "sidechain")
ABOUT = {
    "loader": "The loader's own three changes: the loader is our small piece inside the firmware file. It makes them at every start that runs at least one pack.",
    "base": "The pack `base`, the file base.pack: the shared pack every other pack relies on. Every unit with open-field needs it. Its last five changes hand keys, knobs and a screen to a pack's page (level 2).",
    "8 tracks": "The pack `8 tracks`: tape tracks 5 to 8, and tapes the original firmware still reads.",
    "engines": "The pack `engines`: new synth engines as packs of their own (module/op1f_engine.h). Its last three changes are made only when `cherd` is away, which hands notes and the screen on itself.",
    "fx train": "The pack `fx train`: up to four effects on an instrument, their presets, and an instrument that keeps its effects.",
    "lfo": "The pack `lfo`: four LFOs on a synth sound, their page and their presets.",
    "sidechain": "The pack `sidechain`: one tape track turned down by another, or by the beat.",
}


def cell(text):
    return text.replace("|", "\\|").replace("\n", " ")


def render(profile, image_uses=None):
    """The page's text. `image_uses` may give what the builder does at a place of the file ({address: text}); the
    profile's own `image` list is used without it."""
    places = {place["address"]: place for place in profile["patches"]}
    image = dict(image_uses or {})
    for item in profile.get("image", ()):
        image.setdefault(item["address"], item["does"])
    in_tables = sum(len(table["entries"]) for table in profile["tables"])
    lines = [
        "# The change list",
        "",
        "Every place where open-field changes OS %s of the %s, and nothing else. This page is made from `profiles/%s.json` by `tools/change_list.py`; change the profile, not the page."
        % (profile["firmware_version"], profile["device"], profile["id"]),
        "",
        "It holds addresses and our own words. It quotes no instruction of the firmware and gives none of its bytes. For each place the profile keeps a checksum of the firmware's own bytes there, so that the loader can tell the place before it changes it; a unit with another firmware is left as it is.",
        "",
        "There are %d places. %d of them are changed in the firmware file, by the builder, on the owner's own computer. The others are changed in the running firmware, in memory, at every start: %d changes in %d tables. A place that two packs can use is listed under each."
        % (len(places), len(image), in_tables, len(profile["tables"])),
        "",
        "## In the firmware file, by the builder",
        "",
        "| Address | Bytes | What stands there | What is changed |",
        "| --- | --- | --- | --- |",
    ]
    for address in sorted(image):
        place = places[address]
        lines.append("| `%s` | %d | %s | %s |" % (address, place["length"], cell(place["name"]), cell(image[address])))
    lines += ["", "The builder also adds one record to the file: the loader, our own code, loaded into the memory the first change sets aside. The packs, the base among them, are files on the unit.", "",
              "## In the running firmware, at every start", ""]
    for part in PARTS:
        tables = [table for table in profile["tables"] if table["part"] == part]
        if not tables:
            continue
        count = sum(len(table["entries"]) for table in tables)
        lines += ["### %s (%d)" % ("The loader" if part == "loader" else "The base, `base.pack`" if part == "base" else "`%s`" % part, count), "",
                  ABOUT[part], "", "| Address | Change | Function of ours | What stands there | What ours does |", "| --- | --- | --- | --- | --- |"]
        for table in tables:
            for entry in table["entries"]:
                place = places[entry["address"]]
                form = FORMS[entry["form"]]
                if entry.get("room"):
                    form += ", with a trampoline of %d bytes" % entry["copies"]
                lines.append("| `%s` | %s | `%s` | %s | %s |" % (entry["address"], form, entry["target"], cell(place["name"]), cell(entry.get("does", ""))))
        lines.append("")
    lines += [
        "## Words",
        "",
        "- **A word of data**: four bytes that hold the address of a function. Ours is put there.",
        "- **A call**, **a jump**: eight bytes of code become a call of, or a jump to, a function of ours.",
        "- **The load of an address**: eight bytes of code that load a function's address load ours.",
        "- **A trampoline**: where a jump stands at the entry of a function of the firmware, the instructions it displaces are needed still. The loader copies them, from the running firmware, into a room of the pack and puts a jump back behind them. Our function ends its way into the firmware there. So no pack holds a copy of those instructions.",
        "- **L1 code** and **DDR**: the two kinds of memory the firmware's code runs from.",
        "",
    ]
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--profile", required=True)
    ap.add_argument("--out", help="the page to write (default docs/changes.md)")
    ap.add_argument("--check", action="store_true", help="write nothing: is the page in the tree what the profile gives?")
    args = ap.parse_args(argv)
    try:
        with open(args.profile, encoding="utf-8") as f:
            profile = json.load(f)
        text = render(profile)
        path = args.out or os.path.join(ROOT, "docs", "changes.md")
        if args.check:
            held = None
            if os.path.isfile(path):
                with open(path, encoding="utf-8", newline="") as f:
                    held = f.read()
            print("the change list is %s" % ("what the profile gives" if held == text else "not what the profile gives: write it anew"))
            return 0 if held == text else 1
        os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        print("wrote %s: %d places" % (path, len(profile["patches"])))
    except (OSError, ValueError, KeyError) as problem:
        print("error: %s" % problem)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())

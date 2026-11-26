# Instructions for coding assistants

You are working in the repository of open-field: extra features for the OP-1 Field, added to the firmware that is on an owner's own unit. Read [README.md](README.md) and [CONTRIBUTING.md](CONTRIBUTING.md) first. This file is the short form of what they ask, plus what an assistant in particular must know. What the person you work for tells you comes before this file, except for the rules marked **never**.

## First, which task is this?

- **Using open-field on a unit**: [docs/guide.md](docs/guide.md). Nothing in this repository has to change.
- **Writing a pack** (a feature): start with [docs/first-pack.md](docs/first-pack.md), then [docs/base.md](docs/base.md), [docs/packs.md](docs/packs.md) and [docs/methods.md](docs/methods.md). Prefer a pack that asks the base for what it needs through a card and names no address of the firmware: the examples `examples/wink/`, `examples/yoyo/`, `examples/list/`, `examples/sketch/`, `examples/echo/`, `examples/dub/`, `examples/crush/`, `examples/ping/`, `examples/pluck/` and `examples/fold/` are the models, one or two for each level of the base. A pack that has to stand in the firmware's own code carries a table of changes: `examples/trampolines/` is that model. Do not change the loader or the base to make a pack work: that is a change of its own (below). For more to learn the interface from, read [open-field-packs](https://github.com/LaP0573/open-field-packs): `8 tracks`, `fx train`, `lfo` and `sidechain`, real packs on cards and much larger than the examples, with their sources and a build that uses this repository's tools and headers. A pack is offered to owners through [the listing](docs/catalog.md): a pull request that adds the pack's own file, `catalog/packs/<id>.json` (or, for a new version, changes it), and nothing else, checked by `tools/catalog_check.py`.
- **Changing `8 tracks`, `fx train`, `lfo` or `sidechain`**: they are not here. Their sources, recipes and profiles are in [open-field-packs](https://github.com/LaP0573/open-field-packs), whose BUILDING.md says what to run, and the change is a pull request there. Their entries, each a file of its own under `catalog/packs/`, follow that repository's `main`: once the change is merged there, the install page offers the new file by itself.
- **Changing the base**, the shared pack `base.pack`: it is a pack like any other. Its recipe is `packs/base.json`: give it a new `version`, build, and the new file is copied onto a unit with no install. The install page puts the base on every unit it installs, from `builder/base.pack`: `python -I tools/build_all.py --out <a new folder> --update` writes that file and `builder/base-pack.json` with the rest, and the change says whether it ran on a unit. Keep every field and every name the levels of the card have, with its meaning: packs by other people rely on them. [docs/base.md](docs/base.md), "A change to the base", has the rest.
- **Changing the loader**, our small piece inside the firmware file: the change needs a new file for the builder and a new install on every unit. Say so before you start, and follow "A change to the loader" in [docs/build.md](docs/build.md). Its steps end in a new loader's file under a new version name and its manifest, both in `builder/`. Never write over a version that is there. And never make a version the one the builder's page offers by itself (the option `--current` of `builder/write-manifest.mjs`): that is the maintainer's step, after a try on a unit.
- **Changing a page or a tool**: go ahead; run the checks below.

## The three rules

1. **Never put anything of the firmware into this repository.** No firmware file or part of one, no copy of a unit's storage, no log of a unit, no listing of the firmware's code, no bytes of it written as hex or as data, no picture, font or sound of the device. Not in a source, not in a comment, not in a test, not in a commit message.
2. **Everything here is for OS 1.7.3**, the original (`1.7.3+0`). An address, a length and a checksum belong to that one firmware file. The release for OS 1.7.0 is in the history (commit `b6501e4`); do not mix the two.
3. **Say what was tried.** A build that passes says nothing about a unit. When you report, say what was built, what was run on a unit, and what was not. Never present a build or a test on a computer as proof that a unit works.

## Writing about the firmware

This rule decides what a comment, a page or a commit message may say.

- **Say what our code needs where it meets the firmware**: where a function stands, what it takes and gives back, what it expects of code put in its place, the form of a file our code reads or writes, the list of places that are changed.
- **Say how a thing is found**: the method. [docs/methods.md](docs/methods.md) is the page for that, and the place to add a method you worked out.
- **Do not write up how a part of the firmware works inside** when no code here calls it, hooks it or replaces it. If you read the firmware to understand something, keep to what the change needs and leave the rest out.
- **Use your own words.** Never quote an instruction of the firmware beside its address. "The load of the selected track's number" is right; the instruction itself is not.
- Quoting what the device shows on its screen or writes into its log is fine.

When you read the firmware with the tools in `tools/`, what they print is the firmware's own code. Keep it in your working memory or in a file **outside the repository**. Do not paste it into a file here, into a comment, or into a description of a change.

## Where the firmware lives

The person's own firmware file, the records made from it, their unit's pictures and logs live **outside this checkout**. They tell you where: the environment's `OPEN_FIELD_STOCK` names the firmware file, and the clone's settings `openfield.stock` and `openfield.display` name the records and the pictures ([docs/checking.md](docs/checking.md)). If you need one and do not have it, ask. Never copy such a file into the checkout, not even into an ignored folder, and never add one to a commit.

## Run these before you say a change is done

```bash
python -I tools/build_all.py
```

```bash
python -I -m unittest discover -s tools/tests
```

```bash
node builder/validate-devmode.mjs
```

The first builds everything into `out/` and compares each file with the size and SHA-256 kept in `packs/built.json`. The folder `out/` has to be new or empty: delete it before you build again, or name another folder with `--out`. For a change that is not meant to change what a pack does, every line must read `the same`. For one that is, give the pack a new `version` in its recipe and renew the kept values with `python -I tools/build_all.py --out <a new folder> --update`. Do not edit `packs/built.json` by hand, and never change a kept value to make a failing build pass.

The build needs the toolchain ([docs/build.md](docs/build.md)). If it is not there, say that the change was not built; do not guess at the result.

The check for anything of the firmware runs by itself before every commit, when the person has set it up ([docs/checking.md](docs/checking.md)). It refuses the commit when it finds something. **Never** get around it: do not pass `--no-verify`, do not change the hook, and do not reword a finding away. Remove what it found. Without the person's firmware at hand, the part that needs none still works:

```bash
python -I tools/publish_check.py --forms-only docs examples module packs profiles tools
```

## Files that are made, not written

Change the source, then run the tool. Never edit these by hand.

| File | Made from | By |
| --- | --- | --- |
| The tables, `module/*_table.s` and `examples/*/*_table.s` | The profile beside them | `python -I tools/table_build.py --profile <profile> --all` |
| `docs/changes.md` | `profiles/op1f-1.7.3-n2.json`, the profile of the loader the install page offers | `python -I tools/change_list.py --profile profiles/op1f-1.7.3-n2.json` |
| `docs/firmware-interfaces.md` | The sources and `profiles/op1f-1.7.3-interfaces.json` | `python -I tools/interfaces.py` |
| A pack's directory of pictures, `module/*_pictures.s` and `examples/*/*_pictures.s` | The pack's recipe | `python -I tools/pack_build.py pictures --recipe <recipe> --out <file>` |
| A pack's card, `examples/*/*_card.s` | The pack's recipe | `tools/pack_build.py card`, as [docs/base.md](docs/base.md) says under "The card" |
| The base's release, `module/build_r6d_base_release.s` | `packs/base.json` | `tools/pack_build.py release` |
| `packs/built.json` | A build | `python -I tools/build_all.py --out <a new folder> --update` |
| A loader's file in `builder/` and the manifest that names it (`builder/loader.json`, or one kept beside it under its version's name) | A build, and the person's own firmware file for one number | `builder/write-manifest.mjs`, as [docs/build.md](docs/build.md) says under "A change to the loader" |

## A new place in the firmware

1. Find it and read around it with the tools of [docs/methods.md](docs/methods.md), in the person's own firmware file.
2. Work out its checksum with `tools/place_sum.py`. The tool prints the lines for the profile. The firmware's bytes never go into the profile.
3. Write, in your own words, what stands there and what our function does there.
4. In the description of the change, say how you know what stands there.

A wrong address does no harm on a unit as long as its checksum is right for OS 1.7.3: the loader leaves a pack out when a place is not what the checksum says. A wrong *type* for a function you call is a crash. Read a caller before you bind a function.

## Code that runs on the unit

- No library: no `malloc`, `memcpy` or `printf`, no division by a variable, no object with a constructor that runs at the start.
- On the audio thread: no log line, no file, no memory asked for, nothing that can wait. A card's input function may run there: it counts, and its frame function says it.
- A frame function comes back quickly, every time: the display waits for it. Files and text are the frame function's alone, and a file is never written while the disk mode is open (`disk` in the frame).
- Hand-written assembly: every call gets twelve free bytes on the stack and keeps `RETS`. `tools/stub_calls.py` checks it.
- A pack never writes into the firmware's code. Every change is an entry of its table.
- A picture is never copied from the device. A pack carries instructions, and the unit puts the picture together from its own files ([docs/base.md](docs/base.md)).

## Never

- **Never add a firmware file**, a record of one, a log or a picture of the device to the repository, and never write the firmware's bytes into a file.
- **Never push, publish or open a pull request** unless the person asked for exactly that.
- **Never touch a bootloader**: do not build, change or write one, and do not suggest a step that does.
- **Never change a unit without its owner.** You do not copy a file to a unit, start an install or press a key on one. The owner does, after you have said what the step does and what it risks. In the bootloader's developer menu only the keys the guide names are pressed: one of the others erases the unit's recordings and asks nothing.
- **Never use or ask for the maker's keys**, and never get around a check of the device.
- **Never weaken a check to make something pass**: not the check before a commit, not a kept SHA-256, not a test.
- **Never put a person's name, address or the identifiers of their unit** (a serial number, a Bluetooth address, a path of their computer) into a file or a commit.

## How to write here

Plain words, short sentences, one idea in each. Explain a term the first time it is used. Say what a thing is and what to do, not how the work went: a page describes the present state, and the history is in git. Give a command as a block that can be run as it stands.

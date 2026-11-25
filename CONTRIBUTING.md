# Contributing

open-field is looked after by one person, in spare time. Help is welcome: a fix, a page made clearer, a report of what happened on your unit. This page says what a change needs before it can be taken, and what happens to it after you send it.

## Three rules

1. **No firmware in this repository, ever.** No firmware file or part of one, no copy of a unit's storage, no log of a unit, no listing of the firmware's code, no picture, font or sound of the device. Where you need to say what stands at a place in the firmware, say it in your own words. Code copied from the firmware, or written by following a listing of it line by line, cannot be taken. Neither can anything that needs the maker's keys or gets around a check of the device.
2. **Everything here is for OS 1.7.3**, the original (`1.7.3+0`). An address, a length and a checksum belong to that one firmware file. The release for OS 1.7.0 is in the history (commit `b6501e4`).
3. **Say what you tried.** A change that has only been built is welcome, as long as it says so. Nobody can see from the code alone whether a unit still starts.

## Writing about the firmware

A comment or a page may say what our code needs to know at the place where it meets the firmware: where a function stands, what it takes and gives back, what it expects of code that is put in its place. That is what makes our code work with the device.

A comment or a page may also say **how** something was found: how to copy the firmware off your own unit, how to read it, how to find a function by a message it logs, how to check that a place is the right one. Methods are what the next person needs most, and the tools people use to write code need them too.

What does not go in is a write-up of how a part of the firmware works inside when our code does not need it. If nothing here calls it, hooks it or replaces it, leave its inner workings out.

Quoting what the device shows on its screen or writes into its log is fine ([docs/checking.md](docs/checking.md)).

## Small things need none of the rest

A fix to a page, a clearer sentence in the guide, a report of what you saw: send it. You need no unit and no tools for that.

## What you need for a change to the code

- Your own OP-1 Field on OS 1.7.3, to try the change on.
- Python 3, Node, and the compiler. [docs/build.md](docs/build.md) has the two commands that fetch the compiler.
- For the check before a commit: the firmware file, the bootloader file and the pictures folder of your own unit, kept **outside** this repository. [docs/checking.md](docs/checking.md) says how to set that up, once, in a few commands.

## The check: is my work clean?

`tools/publish_check.py` looks through files for anything of the firmware: its bytes, its pictures, a listing of its code, a list of its texts. It is the same tool the maintainer uses, and you can run it yourself.

- **With your own firmware at hand** (set up once, as above), it runs by itself before every commit you make. It refuses the commit when it finds something, and names the file and the place. A commit that goes through is clean by the same measure the maintainer uses.
- **Without a firmware file**, a part of it still works:

```bash
python -I tools/publish_check.py --forms-only <the files you changed>
```

  That part finds a firmware file by its name, and a log or a listing by its form. It cannot find bytes or pictures, so the maintainer runs the whole check before your change goes in.

The check answers one question only: is something of the firmware in these files. Whether a change works, and whether a unit still starts with it, is what the build, the tests and a try on a unit are for.

## Before you send a change

```bash
python -I -m unittest discover -s tools/tests
```

```bash
python -I tools/build_all.py
```

```bash
node builder/validate-devmode.mjs
```

The second command builds everything into `out/` and compares each result with the size and SHA-256 kept in `packs/built.json`. It overwrites nothing: delete `out/` before you build again, or name another folder with `--out`. If your change is not meant to change what a pack does, it must say "the same" for every file. If it is meant to, give the pack a new version number in its recipe and let the build write the new size and SHA-256 into `packs/built.json`, in the same change:

```bash
python -I tools/build_all.py --out out-new --update
```

**A change to the base**, the shared pack `base.pack`, is a change to a pack like any other: a new version in `packs/base.json`, a new file, and no install. The install page puts the base on every unit it installs, from `builder/base.pack`, which the same `--update` writes anew; the maintainer tries a new base on a unit before it goes in. Packs by other people rely on what its card offers, so a field or a name of a level keeps its place and its meaning; something new is a new level ([docs/base.md](docs/base.md), "A change to the base").

**A change to the loader** is a change of another weight. The loader is the file the builder adds to an owner's firmware, so the change reaches a unit only with a new firmware file and a new install, where a pack is a file copied. You can finish such a change on your own computer: it ends in a new loader's file for the builder, under a new version name, and the small file that describes it, which a tool writes with your own firmware file. [docs/build.md](docs/build.md), "A change to the loader", has the steps. Your version then stands beside the one the builder's page offers, and owners get nothing new by themselves. The maintainer tries it on a unit, and only then makes it the version the page offers: that choice stays with the maintainer. Open an issue before you start on one.

## Where things are

| What | Where |
| --- | --- |
| A place where the loader or the base changes the firmware | `profiles/op1f-1.7.3.json`. The tables in `module/` and the page `docs/changes.md` are made from it, never written by hand. A pack has a profile of its own |
| The loader's and the base's code | `module/`, built by `build.json` and `packs/base.json` |
| The code of `8 tracks`, `fx train`, `lfo` and `sidechain` | Not here: in [open-field-packs](https://github.com/LaP0573/open-field-packs), a folder each. A change to one of them is a pull request there |
| A small pack to learn from | `examples/wink/`, `examples/yoyo/`, `examples/list/`, `examples/sketch/`, `examples/echo/`, `examples/dub/`, `examples/crush/`, `examples/ping/`, `examples/pluck/` and `examples/fold/`, one or two for each level of the base, and `examples/trampolines/` for a pack that changes places of the firmware |
| What the base offers a pack | `module/build_r6e_card.h`, and [docs/base.md](docs/base.md) with the pages of levels 4 to 10 |
| A function of the firmware our code calls | Bound in the source that calls it; listed in `docs/firmware-interfaces.md`, which is made from the sources and `profiles/op1f-1.7.3-interfaces.json` |
| A screen of the base | The instructions in `pictures/`. A pack carries instructions of its own, never a picture made from the device's own |
| The tools for reading your own firmware file | `tools/`, used as [docs/methods.md](docs/methods.md) says |
| The install pages | `builder/` |
| The packs the install page offers | `catalog/listing.json` and one file a pack, `catalog/packs/<id>.json`, checked by `tools/catalog_check.py` ([docs/catalog.md](docs/catalog.md)) |
| The pages for people | `docs/`, and the files at the top |

## The pages for those who write code

- [A first pack](docs/first-pack.md): an example, step by step.
- [The base](docs/base.md): what it offers a pack, level by level: the card, a call every frame, pictures, a page with keys and knobs, folders, files and text, then [level 4](docs/level-4.md) (painting, pages that stack), [level 5](docs/level-5.md) (a sequencer, the notes heard), [level 6](docs/level-6.md) (a place in the sound, working memory), [level 7](docs/level-7.md) (values kept in tapes and sounds), [level 8](docs/level-8.md) (an effect), [level 9](docs/level-9.md) (a synth engine) and [level 10](docs/level-10.md) (the value as the sound plays it); beyond the levels, [a page that holds its picture](docs/held-page.md) and [layers](docs/layers.md) (a pack's effect or LFO in the unit's own lists).
- [Packs](docs/packs.md): the file, and what the loader does with it. Every line the loader writes into the log is there.
- [Methods](docs/methods.md): how to find a place in your own firmware, check it and use it, how to read a log, how to try a pack safely.
- [The firmware's functions our code uses](docs/firmware-interfaces.md).
- [The listing](docs/catalog.md): how a pack of yours is offered on the install page, by a pull request that adds one file of its own, `catalog/packs/<your pack's id>.json`.
- [AGENTS.md](AGENTS.md): the same rules, for a coding assistant.

## Sending a change

Send one change at a time, as a pull request. In its description, say:

- what it changes, and why;
- how to try it;
- what you tried on a unit, with which feature files on it, and what you did not try;
- for a new place in the firmware: how you know what stands there, in your own words;
- whose work it builds on, if it builds on someone's: a project, a post, a person. They are named in [THIRD-PARTY.md](THIRD-PARTY.md).

## What happens then

1. The change is read, and the whole check is run on it against the maintainer's own copy of the firmware.
2. It is built and tried on the maintainer's unit, so a change that cannot be tried there may wait.
3. It goes in, comes back with questions, or is declined with the reason.

This is one person's spare time, so an answer can take a while.

## A feature of your own

[A first pack](docs/first-pack.md) shows how to build one and try it on your own unit; nothing has to be sent in for that. A new feature file is welcome here as an idea first: open an issue and say what it would do. A pack of yours lives in your own repository; to offer it to every owner on the install page, add it to [the listing](docs/catalog.md). A pack that needs something new of the base comes with a change to the base ([docs/base.md](docs/base.md)), so say that in the issue too.

## Licence

open-field is under the MIT licence ([LICENSE](LICENSE)). What you send in is offered under the same licence, and by sending it you say that it is yours to offer.

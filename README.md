# open-field

Extra features for the OP-1 Field, added to the firmware that is already on your own unit.

## Install

**[Open the install page: lap0573.github.io/open-field](https://lap0573.github.io/open-field/)**

The page runs in your browser and takes you through the install step by step. Afterwards, the same page adds and removes packs. Plug your OP-1 Field in and press **connect**. Use Chrome, or Edge on Windows ([what you need](#what-you-need)). Read [the guide](docs/guide.md) before your first install: it says what can go wrong and how to go back.

Want to write a pack of your own? Start with [A first pack](docs/first-pack.md).

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing it changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone. Read [NOTICE.md](NOTICE.md) first.

**Status: open-field 0.20, for OS 1.7.3.** Read "What is proven and what is not" below before you rely on anything here.

## What it is

- **A loader.** A small piece of our own code that is added, once, to the firmware file from your own unit. A page that runs in your browser does that: you give it your file, it gives you a new one, and nothing is uploaded anywhere. At every start the loader reads the pack files on the unit.
- **The base** (`base.pack`). The shared pack every feature relies on: one file that every unit with open-field has. It is also what any pack can build on: a call every frame, pictures, a page with keys and knobs, folders and files.
- **Packs.** Each feature is one file that you copy onto the unit like a sample. With the file there, the feature is there at the next start. Delete the file and the feature is gone.

With open-field on the unit, the start screen reads `1.7.3 - open-field 0.20`: the main version, then open-field and the release of the base. On a unit on `1.7.3+1` it reads `1.7.3+1 - open-field 0.20`.

The install puts two things on the unit: the new firmware file and `base.pack`, from `builder/base.pack` beside the page. Packs come after. They are not kept in this repository: their authors publish them, and once open-field is on your unit, the install page's packs section adds and removes the packs of [the listing](docs/catalog.md), and with its **+** any pack file you have on your computer. You can also copy a pack file onto the unit yourself, in its disk mode ([the guide](docs/guide.md#adding-a-feature)). **A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs.

## What is not here, and never will be

- **No firmware.** Not the original file, not a changed one, not an update package. Every owner builds from the file on their own unit.
- **Nothing of Teenage Engineering's.** No code, picture, font or key of theirs. Where a screen of ours is a changed screen of the device, the feature file holds instructions only, and the unit puts the picture together from its own files each time it starts.
- **No copy of a unit's storage, and no logs.**

A check that runs before every commit refuses a file that holds bytes of the firmware: [docs/checking.md](docs/checking.md).

## Where to start

**To use it**

- Install it: [the install page](https://lap0573.github.io/open-field/). Use it: [docs/guide.md](docs/guide.md).
- See exactly what the loader and the base change in the firmware: [docs/changes.md](docs/changes.md). What a pack changes in the firmware is in that pack's own profile, beside its sources.
- Build the loader, the base and the examples yourself from the sources: [docs/build.md](docs/build.md).

**To write a feature of your own**

1. [A first pack](docs/first-pack.md): build a small example, put it on your unit, read its lines in the log, change it.
2. [The base](docs/base.md): what it offers a pack, level by level. A call every frame and pictures, a page with keys and knobs, folders and files and text, painting and pages that stack, a sequencer of your own, a place in the sound and working memory, values kept in tapes and sounds, an effect of your own, a synth engine of your own, the value as the sound plays it, a page that holds its picture, and rows of your own in the unit's lists of effects and LFOs. And what it does not offer yet.
3. [Packs](docs/packs.md): what a pack file is, and what the loader does with it at every start.
4. [Methods](docs/methods.md): how to find, check and use a place in your own firmware, for a feature that has to stand in it.
5. [The firmware's functions our code uses](docs/firmware-interfaces.md): what each takes and gives back.
6. [The listing](docs/catalog.md): how a pack of yours is offered to every owner on the install page.

**To send a change in**: [CONTRIBUTING.md](CONTRIBUTING.md). **If you are a coding assistant**: [AGENTS.md](AGENTS.md).

## What you need

- An OP-1 Field with the original **OS 1.7.3** and bootloader **1000.1.1**, or **1000.2.0** (one install reported, see below). Other versions are refused: every address in this project belongs to that one firmware file. The start screen shows `1.7.3`, or `1.7.3+1` on a unit that is already on that version, and such a unit can use the page. That label is the unit's version record, not the firmware file's, so it does not decide: the install page holds your file to the original's SHA-256 and refuses any other.
- A USB cable and a computer: Windows with Chrome or Edge, a Mac with Chrome, or Linux with a Chromium browser that has folder access. A Mac and Linux need a few small things more ([the guide](docs/guide.md#what-you-need)).

**A unit still on OS 1.7.0** is updated first, with Teenage Engineering's own official update, to OS 1.7.3 or `1.7.3+1`, and then uses the install page. `1.7.3+1` comes with a new bootloader, 1000.2.0, that is more locked down and doesn't let the unit go back to an older OS version: updating to it is at your own risk ([the guide](docs/guide.md), "A unit on OS 1.7.0").

**A unit on a later OS version than 1.7.3 cannot use this yet** (`1.7.3+1` does not count as one: such a unit can use the install page, see above). The builder takes only the OS 1.7.3 file, which such a unit does not hold, and whether such a unit can be brought back to OS 1.7.3 is not known. Do not update a unit in the hope of going back. Support for a later version is work that has not been done ([methods](docs/methods.md), "Moving the work to another firmware version").

**An official update of the device replaces open-field.** It writes the maker's firmware over the one with our loader in it. The unit is then on the original firmware of the new version, and open-field is gone from it. Pack files that are still on the disk do nothing without the loader. If you want to keep open-field, do not update.

## What is proven and what is not

- The install without any key has been made on several units, by their owners, from Windows computers, with bootloader 1000.1.1.
- One install from Linux, with bootloader 1000.1.1 (2026-11-19), and one from a Mac, with 1000.2.0 (2026-11-20), were reported, both on open-field 0.3 with earlier forms of the helper. The base and packs of 0.20 and the helper as it is now have not been tried on 1000.2.0, a Mac or Linux, and the way back has not been tried there either. Windows with 1000.2.0 has not been tried.
- For 1000.2.0, three numbers of the page's placement are the contributor's reading of that bootloader. The rest of the calculation is 1000.1.1's, assumed for 1000.2.0. The contributor reported that the one write landed. The maintainer has not read that bootloader or tried a unit with it.
- A write that is interrupted can leave the unit unable to start. The guide says how to keep that risk small and what the way back is. The way back from a unit that does not start has **not** been rehearsed.
- The loader and the base have been used on several units, on the maintainer's for weeks. The example packs have run on the maintainer's unit, `crush`, `ping`, `pluck` and `fold` in their first version, which drew their values as set rather than as the sound plays them. The base of this release, base 38, has run on the maintainer's unit beside other packs and a test pack of the maintainer's that paints over its own page: the install question stayed whole over that page, and an effect switched off for its cost played again after a pick, on a sound and on a wagon of `fx train`. Base 37 before it ran with `lfo` 12 riding its LFO layers on a synth sound and a drum sound. What the bases added after base 32 (a page that holds its picture, its changed parts drawn again, the logs in the disk mode, layers) was each tried on that unit, with test packs of the maintainer's where a pack was needed. Base 32's settings were tried with two test packs that add settings: they showed in KEYBOARD, SYSTEM, MIDI and PACKS, changed with the knobs, and came back after a restart.

## What is in this repository

| Folder | What |
| --- | --- |
| `builder/` | The page that guides an install, the builder, and `base.pack`, which the page puts on the unit. They run in the browser from these files. |
| `catalog/` | The listing of packs: what the install page will offer. |
| `.github/` | The checks a host runs on every change, the templates for a report and for a change, and the publishing of the install page on the host's pages. |
| `module/` | The sources of the loader and of the base, and the headers a pack includes: C++ and assembly for the Blackfin processor. |
| `packs/` | The recipe of `base.pack`: what it is built from. And the kept size and SHA-256 of what this repository builds. |
| `examples/` | Small packs that build, as models for one of your own: `wink`, `yoyo`, `list`, `sketch`, `echo`, `dub`, `crush`, `ping`, `pluck` and `fold`, one or two for each level of the base, and `trampolines`, which changes places of the firmware. |
| `pictures/` | The base's instructions by which the unit puts its screens together. |
| `profiles/` | Where the firmware is changed: addresses, lengths and checksums. No bytes of the firmware. |
| `linker/` | The linker scripts. |
| `tools/` | The build tools, the tools for reading your own firmware file, and the check before a commit. |
| `hooks/` | The hook that runs that check. |
| `docs/` | The guide, the change list, how to build, how the check works, and the pages for those who write a pack. |

## Licence

MIT: [LICENSE](LICENSE).

## Credits

Parts of two other projects are reused here under their own licences, which are MIT too, and this project learnt its way of working from them. [THIRD-PARTY.md](THIRD-PARTY.md) names what was taken and what was learnt.

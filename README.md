# open-field

Extra features for the OP-1 Field, added to the firmware that is already on your own unit.

**Unofficial.** This project is not made, endorsed or supported by Teenage Engineering, and it is not affiliated with them.

**At your own risk.** Installing it changes the firmware of your unit. That is a risky process: a unit can end up unable to start for good ("bricked"), and installing it will void the warranty of your device. Whether to do it is your decision alone. Read [NOTICE.md](NOTICE.md) first.

**Status: open-field 0.3, for OS 1.7.3.** Read "What is proven and what is not" below before you rely on anything here.

## What it is

- **A loader.** A small piece of our own code that is added, once, to the firmware file from your own unit. A page that runs in your browser does that: you give it your file, it gives you a new one, and nothing is uploaded anywhere. At every start the loader reads the pack files on the unit.
- **The base** (`base.pack`). The shared pack every feature relies on: one file that every unit with open-field has. It is also what any pack can build on: a call every frame, pictures, a page with keys and knobs, folders and files.
- **Feature files** ("packs"). Each feature is one file that you copy onto the unit like a sample. With the file there, the feature is there at the next start. Delete the file and the feature is gone.

| Feature file | What it adds |
| --- | --- |
| `8 tracks.pack` | Tape tracks 5 to 8, with everything tracks 1 to 4 do. The original firmware can still read your tapes. |
| `fx train.pack` | Up to four effects on an instrument, their presets, and instruments that keep their effects. |
| `lfo.pack` | Four LFOs on a synth sound, with presets. |
| `sidechain.pack` | One tape track turned down by another, or by the beat. |

Each works by itself or together with the others, beside `base.pack`. With them on the unit, the start screen reads `1.7.3 - open-field 0.3`: the main version, then open-field and the release of the base.

`base.pack` comes with the install: the install page puts it on the unit with the new firmware file, from `builder/base.pack` beside the page. The feature files are not kept in this repository. Their authors share them, and you copy them onto the unit in its disk mode ([the guide](docs/guide.md#adding-a-feature)), or you build them from the sources here with one command ([docs/build.md](docs/build.md)). **A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs.

## What is not here, and never will be

- **No firmware.** Not the original file, not a changed one, not an update package. Every owner builds from the file on their own unit.
- **Nothing of Teenage Engineering's.** No code, picture, font or key of theirs. Where a screen of ours is a changed screen of the device, the feature file holds instructions only, and the unit puts the picture together from its own files each time it starts.
- **No copy of a unit's storage, and no logs.**

A check that runs before every commit refuses a file that holds bytes of the firmware: [docs/checking.md](docs/checking.md).

## Where to start

**To use it**

- Install it and use it: [docs/guide.md](docs/guide.md).
- See exactly what is changed in the firmware: [docs/changes.md](docs/changes.md).
- Build it yourself from the sources: [docs/build.md](docs/build.md).

**To write a feature of your own, or to change one**

1. [A first pack](docs/first-pack.md): build a small example, put it on your unit, read its lines in the log, change it.
2. [The base](docs/base.md): what it offers a pack, in three levels: a call every frame and pictures; a page with keys and knobs; folders, files and text. And what it does not offer yet.
3. [Packs](docs/packs.md): what a pack file is, and what the loader does with it at every start.
4. [Methods](docs/methods.md): how to find, check and use a place in your own firmware, for a feature that has to stand in it.
5. [The firmware's functions our code uses](docs/firmware-interfaces.md): what each takes and gives back.
6. [The listing](docs/catalog.md): how a pack of yours is offered to every owner on the install page.

**To send a change in**: [CONTRIBUTING.md](CONTRIBUTING.md). **If you are a coding assistant**: [AGENTS.md](AGENTS.md).

## What you need

- An OP-1 Field with the original **OS 1.7.3** and bootloader **1000.1.1** or **1000.2.0**. Other versions are refused: every address in this project belongs to that one firmware file. With 1000.2.0 the install has been made on one unit. The start screen of the original firmware shows `1.7.3`, and on one unit it showed `1.7.3+1` with exactly the original file on the unit, so the install page takes both. The screen decides nothing: the page holds the unit's firmware file to the original's SHA-256 and refuses any other file.
- A desktop computer with Chrome or Edge, or a Linux computer with a browser that has the folder access the page needs (tried with Brave), and a USB cable. A Mac cannot be used yet.

**A unit still on OS 1.7.0** has two ways. Update it to OS 1.7.3 with Teenage Engineering's own official update, one that gives the original OS 1.7.3 file, and then use the install page: it refuses a unit whose firmware file is not that one. Or use the page for OS 1.7.0, which the install page links to: the release of before, kept as it was, only for a unit that was never updated to 1.7.3 ([the guide](docs/guide.md), "A unit on OS 1.7.0").

**A unit on a later OS version than 1.7.3 cannot use this yet.** The builder takes only the OS 1.7.3 file, which such a unit does not hold, and whether such a unit can be brought back to OS 1.7.3 is not known. Do not update a unit in the hope of going back. Support for a later version is work that has not been done ([methods](docs/methods.md), "Moving the work to another firmware version").

**An official update of the device replaces open-field.** It writes the maker's firmware over the one with our loader in it. The unit is then on the original firmware of the new version, and open-field is gone from it. Pack files that are still on the disk do nothing without the loader. If you want to keep open-field, do not update.

## What is proven and what is not

- The install without any key has been made on several units, by their owners, from Windows computers. It has also been made once, on one unit, from a Linux computer. It has not been tried on another bootloader version, or from a Mac.
- A write that is interrupted can leave the unit unable to start. The guide says how to keep that risk small and what the way back is. The way back from a unit that does not start has **not** been rehearsed.
- The features have been used on several units, on the maintainer's for weeks. The three example packs `wink`, `yoyo` and `list` have run on the maintainer's unit.

## What is in this repository

| Folder | What |
| --- | --- |
| `builder/` | The page that guides an install, the builder, and `base.pack`, which the page puts on the unit. They run in the browser from these files. |
| `catalog/` | The listing of packs: what the install page will offer. |
| `.github/` | The checks a host runs on every change, the templates for a report and for a change, and the publishing of the install page on the host's pages. |
| `module/` | The sources of the loader, of the base and of the features: C++ and assembly for the Blackfin processor. |
| `packs/` | One recipe for `base.pack` and for each feature file: what it is built from. |
| `examples/` | Small packs that build, as models for one of your own: `wink`, `yoyo` and `list`, one for each level of the base, and `trampolines`, which changes places of the firmware. |
| `pictures/` | Instructions by which the unit puts our screens together from its own pictures. |
| `profiles/` | Where the firmware is changed: addresses, lengths and checksums. No bytes of the firmware. |
| `linker/` | The linker scripts. |
| `tools/` | The build tools, the tools for reading your own firmware file, and the check before a commit. |
| `hooks/` | The hook that runs that check. |
| `docs/` | The guide, the change list, how to build, how the check works, and the pages for those who write a pack. |

## Licence

MIT: [LICENSE](LICENSE).

## Credits

Parts of two other projects are reused here under their own licences, which are MIT too, and this project learnt its way of working from them. [THIRD-PARTY.md](THIRD-PARTY.md) names what was taken and what was learnt.

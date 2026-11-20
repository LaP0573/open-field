# The install page and the builder

One page that leads an owner through the install, the helpers that do the file work on the unit's disk, and the builder they use. The page runs in the browser, on the owner's own files: nothing is uploaded, and nothing here is firmware.

| File | What it is |
| --- | --- |
| `index.html` | The install page: the unit checked over USB MIDI, the new firmware file built, the write planned, and the keys of the write |
| `open-field-helper.ps1`, `open-field-helper.py` | Two helpers that speak the same folder protocol: the first for Windows, the second for Linux (Python 3, standard library only; it reaches the unit over USB with libusb). A web page cannot reach the unit's files, so a helper does: it copies the owner's firmware file and the log files off, lists the disk, waits for the page's plan, puts the new file (and a pad file and pack files when the plan says so) on the unit, and reads every copy back |
| `install.mjs` | The plan the page makes from the helper's listing: what goes on the unit, and the whole listing the disk must show afterwards |
| `catalog.mjs` | The listing of packs as the page reads it, and the download of a ticked pack from its author's repository, held to the listing's size and SHA-256 ([docs/catalog.md](../docs/catalog.md)) |
| `ferret.mjs` | The unit's check over USB MIDI: reads the bootloader's version and switches the developer menu on |
| `patch.mjs`, `ldr.mjs` | The builder: the open-field firmware file from the owner's own file |
| `placement.mjs` | Where the bootloader's write lands, worked out from a listing of the unit's disk, and the size of the one extra file (the pad file) that makes it land |
| `loader.json` | The manifest: what the builder does. It names the two places the builder changes, the loader's file it adds, and the SHA-256 the result must have. This one is the version the page offers by itself |
| `loader-1.7.3-v1.bin` | The loader's file that manifest names: the loader, our small piece of code inside the firmware file, built from this repository. The packs, `base.pack` among them, are files on the unit and are not in it |
| `base.pack` | The base, the shared pack every feature needs, built from this repository. The page puts it on every unit it installs, with the new firmware file. The build holds it to `packs/built.json`, byte for byte |
| `base-pack.json` | Its size and SHA-256, and its version and release: the page refuses a `base.pack` that is not this one |
| `build-image.mjs` | The builder as a command, for those who prefer one |
| `write-manifest.mjs` | Writes a new version of the loader's file and its manifest, from a build and your own firmware file. For a change to the loader |
| `validate-builder.mjs`, `validate-devmode.mjs`, `validate-install.mjs` | Checks of the builder, of the unit's check over USB MIDI, and of the install's plan |
| `ferret.LICENSE`, `open-xy.LICENSE` | The licences of the two projects whose code is reused here |

## Opening the page

The page loads its code from files beside it and uses USB MIDI, so the browser wants it from a web address, not from a file. On your own computer:

```bash
python -m http.server 8000 --directory builder
```

Then open `http://localhost:8000/` in Chrome or Edge on a desktop computer. The line the page gives for the helper fetches the helper from the same address. Served this way, the page has no listing of packs beside it (the host's pages put one there), so it offers no pack to tick; pack files you put into the folder's `packs` still go on. To have the listing, copy `catalog/packs.json` into `builder/` as `packs.json` first, and do not commit that copy.

## How the page and the helper share the work

They share one folder on the computer, `open-field` in the user's folder. The owner chooses it on the page once, before the helper runs; the page keeps it in the browser's own storage and offers it again on a later visit. A browser never tells a page a folder's path, so the page writes its mark into the folder (`page.json`, a random id kept for the browser tab) and puts the same mark into the helper's line (`$openFieldPage = '<id>'; irm '…' | iex`, or `-Page <id>` for the file). On Linux the line is `curl -fsSL <the page's address>/open-field-helper.py | python3 -I - --page <the page's mark>`. The helper checks the mark before it makes, writes or takes anything, and stops when the folder holds another mark or none; it writes the mark into its state, and the page follows only a helper run that carries its own. The helper writes into the folder what it took off the unit and its listing of the disk, and waits. The page reads them, builds the new firmware file, puts its own `base.pack` into the folder's `packs`, decides with `placement.mjs` whether the write lands or needs a pad file, and writes the new file, the pad file, `base.pack` and the plan into the folder, the plan last. A `base.pack` of other bytes already in `packs` is first kept in the folder's `replaced`, under the first twelve digits of its SHA-256; one of the same bytes is left as it is. The packs the owner ticks in the listing go into `packs` the same way, before `base.pack`, once each has been downloaded and held to its entry (`catalog.mjs`). The way back, the page's second choice, puts no pack on. The helper then does what the plan says and holds its new listing of the disk against the plan's, row for row: it decides nothing about the placement itself. The page runs the placement once more on the helper's last listing.

What the helper changes on the unit, and nothing else: it takes the files in `logs` off, and the pad files earlier installs left under its own names (`open-field-pad.bin`, `open-field-pad-2.bin`, ...; a file of any other name stays), and it writes `os.ldr` at the top of the drive, the pad file the plan names, and the pack files of the folder's `packs` sub folder into `user/pk` (the folder made if it is not there; `base.pack` last). Before it writes, it lists the disk again and stops if anything changed since the plan was made. It never deletes a folder.

## The checks

```bash
node builder/validate-devmode.mjs
```

```bash
node builder/validate-builder.mjs --stock <your own os.ldr>
```

```bash
node builder/validate-install.mjs
```

The first runs the unit's check against a made-up bootloader; no MIDI port of the computer is touched. The second builds the image from your own firmware file and holds its SHA-256 to the manifest's; without `--stock` it makes only the checks that need no firmware file. The third holds the install's plan to the placement on made-up disks; it needs no firmware file.

A helper can be run whole without a unit: `-Source` (the `.ps1`) or `--source` (the `.py`) names a folder of the computer that stands for the unit's disk. The `.py` also has `--list-only`, which reads the unit and changes nothing. `node builder/validate-install.mjs --play <folder> --bootloader <version> --stock <your own os.ldr>` plays the page's part on the helper's folder.

## More than one version of the loader

The loader's file has a version: `v1` today, the first for OS 1.7.3. A version that anyone may have installed keeps its bytes for good, so a changed loader is a new file under a new name, with a manifest of its own. A change to the base, `base.pack`, needs none of this: the base is a file on the unit, and a new one is copied there.

| In this folder | What it is | Who gets it |
| --- | --- | --- |
| `loader.json` and the loader's file it names | The version the page offers by itself | Everyone who opens the page |
| A manifest named after its version, such as `loader-1.7.3-v2.json`, and its loader's file | A version kept beside that one: a new one that waits for a try on a unit, or an older one | Someone who asks for it by name, with `?loader=1.7.3-v2` behind the page's address |

The page shows a line beside the version it has loaded: `ran on a unit` with a date, or `not run on a unit yet`. That line comes from the manifest.

`write-manifest.mjs` makes both files of a version. Its first lines are its manual, and [building](../docs/build.md) has the steps under "A change to the loader". A new version is written beside `loader.json`. Making it the one the page offers by itself is the maintainer's step, after a try on a unit.

The two commands that take a manifest take a version's name too:

```bash
node builder/validate-builder.mjs --loader 1.7.3-v1 --stock <your own os.ldr>
```

```bash
node builder/build-image.mjs --loader 1.7.3-v1 <your own os.ldr> <the file to write>
```

A version's name works wherever its manifest lies: beside `loader.json`, or as `loader.json` itself, and so does `?loader=` on the page.

## On a host's pages

The page can be served from any web address over HTTPS: the unit's check uses USB MIDI, and the folder the page and the helper share uses the browser's folder access, and a browser offers both only to a page served over HTTPS or from the computer itself. The workflow `.github/workflows/pages.yml` publishes this folder, `builder/`, on the host's pages, once the checks have passed on `main`. What that needs:

- **This folder alone is served**, as it stands on `main`, with the listing of packs beside it: the workflow copies `catalog/packs.json` into it as `packs.json` before it publishes. The page loads only files beside it, by relative paths, and one thing from another site: the file of a pack the owner ticks, from its author's repository at the commit the listing names (`raw.githubusercontent.com`), held to the listing's size and SHA-256 before it is used. `tools/tests/test_tree.py` holds that (`ThePagesOnAHost`). Its one link to another site is a link the owner may follow, not a load.
- **The page for OS 1.7.0 is served beside it, as it was.** The workflow takes the folder `builder/` and the listing of the last commit for OS 1.7.0, `b6501e4`, from the history, and puts them in a folder `1.7.0` beside the page before it publishes; nothing of it is kept on `main`. The page links to it in one line, for a unit that was never updated to 1.7.3. Its keys in the browser's storage carry the firmware, so that the two pages, on one site, never take each other's.
- **Script files ending in `.mjs` are served as scripts.** A browser runs a module only when the server says it is JavaScript.
- **`open-field-helper.ps1` is served as text or as a download.** PowerShell reads it either way. `open-field-helper.py` is read by `curl` and handed to `python3` the same way. The line the page shows for it is made from the page's own address, so it names the host's address there.
- **`base.pack` and the loader's file are served as they are**, byte for byte: the page holds each to its SHA-256 and refuses one that is not.
- **On the host**, its pages are switched on once, with the workflow as their source. The repository's settings do that; nothing in this folder can.

## What the page cannot do

- It does not press the unit's keys. The write itself is always keys on the unit.
- It has a helper for Windows and one for Linux. On a Mac the page could do the file work itself over USB, as open-xy's builder does for the OP-XY, but that is not built.
- The placement model is that of two bootloader versions, 1000.1.1 and 1000.2.0. 1000.2.0's heap was read from the bootloader itself; the rest of the model is 1000.1.1's, and one write on one unit holds it for 1000.2.0. The page's first step reads the version and refuses any other, and every plan names the version it was made for.

[The owner's guide](../docs/guide.md) has the whole install in words.

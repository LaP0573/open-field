# The install page and the builder

One page that leads an owner through the install, the helpers that do the file work on the unit's disk (one for Windows, one for Mac and Linux), and the builder they use. The page runs in the browser, on the owner's own files: nothing is uploaded, and nothing here is firmware.

| File | What it is |
| --- | --- |
| `index.html` | The install page: the unit checked over USB MIDI, the new firmware file built, the write planned, and the keys of the write |
| `open-field-helper.ps1` | The Windows helper. A web page cannot reach the unit's files on Windows, so the helper does: it copies the owner's firmware file and the log files off, lists the disk, waits for the page's plan, puts the new file (and a pad file and pack files when the plan says so) on the unit, and reads every copy back. Before it lists the disk it takes every file in `user/pkin` off, the waiting folder of the base's door, so that a change left waiting there is never finished over a new install |
| `open-field-helper.py` | The helper for Mac and Linux: the same work through the same files in the folder, `user/pkin` emptied too. It needs Python 3 with nothing added, and libusb, through which it reaches the unit over USB |
| `door.mjs` | The page's side of the door: the messages the base answers over USB MIDI to list the unit's packs, check each by its SHA-256, send files into the waiting folder, mark files for removal and ask the unit, which shows its own question and changes nothing before T1 |
| `packs-live.mjs` | What the packs section shows: each file on the unit held to the listing (on your OP-1, an update out, a different file, not listed, coming off), and the ticks turned into the change the page sends, `base.pack` last |
| `room.mjs` | The room for packs, as the packs section counts it: what each pack file takes of the loader's room, where the loader would place the unit's files now and after the change waiting, and the pack files, engines and effects against what the unit takes |
| `after.html` | The page the browser waits on while the unit restarts: it uses no MIDI, then goes back and checks every file |
| `install.mjs` | The plan the page makes from the helper's listing: what goes on the unit, and the whole listing the disk must show afterwards |
| `catalog.mjs` | The listing of packs as the page reads it, and the download of a ticked pack from its author's repository, held to the listing's size and SHA-256 ([docs/catalog.md](../docs/catalog.md)) |
| `ferret.mjs` | The unit's check over USB MIDI: reads the bootloader's version and switches the developer menu on |
| `patch.mjs`, `ldr.mjs` | The builder: the open-field firmware file from the owner's own file |
| `placement.mjs` | Where the bootloader's write lands, worked out from a listing of the unit's disk, and the size of the one extra file (the pad file) that makes it land |
| `loader.json` | The manifest: what the builder does. It names the two places the builder changes, the loader's file it adds, and the SHA-256 the result must have. This one is the version the page offers by itself |
| `loader-1.7.3-v2.bin` | The loader's file that manifest names: the loader N2, our small piece of code inside the firmware file, built from this repository. The packs, `base.pack` among them, are files on the unit and are not in it |
| `loader-1.7.3-v1.json`, `loader-1.7.3-v1.bin` | The first version of the loader, N1, and its manifest, kept beside the second for the way back. It is built from the same sources and loads fewer pack files. The page offers it only to someone who asks for it by name |
| `base.pack` | The base, the shared pack every feature needs, built from this repository. The page puts it on every unit it installs, with the new firmware file. The build holds it to `packs/built.json`, byte for byte |
| `base-pack.json` | Its size and SHA-256, and its version and release: the page refuses a `base.pack` that is not this one |
| `build-image.mjs` | The builder as a command, for those who prefer one |
| `write-manifest.mjs` | Writes a new version of the loader's file and its manifest, from a build and your own firmware file. For a change to the loader |
| `validate-builder.mjs`, `validate-devmode.mjs`, `validate-install.mjs` | Checks of the builder, of the unit's check over USB MIDI, and of the install's plan |
| `validate-door.mjs`, `validate-packs-live.mjs` | Checks of the door's messages against a model of the unit, and of what the packs section shows and sends |
| `validate-room.mjs` | Checks of the count of the room for packs, on made-up pack files and the page's own `base.pack` |
| `ferret.LICENSE`, `open-xy.LICENSE` | The licences of the two projects whose code is reused here |

## Opening the page

The page loads its code from files beside it and uses USB MIDI, so the browser wants it from a web address, not from a file. On your own computer:

```bash
python -m http.server 8000 --directory builder
```

On a Mac or Linux the command's first word is `python3`. Then open `http://localhost:8000/` in a browser [the guide](../docs/guide.md#what-you-need) names for your computer. The line the page gives for the helper fetches the helper from the same address. Served this way, the page has no listing of packs beside it (the host's pages put one there), so its packs section shows no pack. To have the listing, join the files of `catalog/` into one file beside the page first, with `python -I tools/catalog_check.py --join builder/packs.json`, and do not commit that file.

## What the page shows

At first, the top of the page and the list of packs. After **connect**, the top says what it found: an OP-1 with no open-field gets one **install open-field** button, which shows the install's eight steps there, in place; an OP-1 with open-field gets a small table (its OS, its open-field, its packs, its last start), and the packs section adds and takes off packs over USB MIDI. Above the packs, the room window shows the loader's room for packs as a bar, each pack where the loader would place it, now and after the change waiting, with the pack files, engines and effects against what the unit takes. It shows only while the page talks to the unit. Under the table, **reinstall or go back to the original firmware** shows the same steps; in step 5, **my backup of the original firmware** is the way back. The address keeps `#install` while the steps are open, so a reload comes back to them. The page holds the OP-1's MIDI ports only while it talks to the unit, and gives every port back before step 1 talks to the bootloader.

## How the page and the helper share the work

They share one folder on the computer, any folder the owner picks on the page, in step 3, before the helper runs; the page keeps it in the browser's own storage and offers it again on a later visit. A browser never tells a page a folder's path, only its name, so the page writes its mark into the folder (`page.json`, a random id kept for the browser tab and made new each time step 1 finds the unit, so that a helper started before that is not followed) and puts the mark and the folder's name into the helper's line (`$openFieldPage = '<id>'; $openFieldFolder = '<name>'; irm '…' | iex`, or `-Page <id> -FolderName <name>` for the file; `-Folder <path>` names the folder outright). On a Mac or Linux the line is `curl -fsSL '<url>' | python3 -I - --page '<id>' --folder-name='<name>'` (`--folder <path>` names the folder outright). The page shows the one line for the computer it detects. The helper looks for a folder of that name that holds the mark, in its current folder, the user folder, the Desktop, Documents and Downloads, each and one level below. With exactly one, it works there; with none or several, it asks once: the Windows helper in a folder dialog, the other for a path in the terminal. It checks the mark before it makes, writes or takes anything, and stops when the folder holds another mark or none; it writes the mark into its state, and the page follows only a helper run that carries its own. The helper writes into the folder what it took off the unit and its listing of the disk, and waits. The page reads them, builds the new firmware file, puts its own `base.pack` into the folder's `packs`, decides with `placement.mjs` whether the write lands or needs a pad file, and writes the new file, the pad file, `base.pack` and the plan into the folder, the plan last. A `base.pack` of other bytes already in `packs` is first kept in the folder's `replaced`, under the first twelve digits of its SHA-256; one of the same bytes is left as it is. The install puts `base.pack` alone on the unit: other pack files in the folder's `packs` are left there, not copied, and the page names them in its log. Packs are added from the packs section once open-field is on the unit. The way back, the page's second choice, puts no pack on. The helper then does what the plan says and holds its new listing of the disk against the plan's, row for row: it decides nothing about the placement itself. The page runs the placement once more on the helper's last listing. Its lines and the plan's numbers go into the page's log, at the bottom of the page; the steps show only where the install stands.

What the helper changes on the unit, and nothing else: it takes the files in `logs` and those waiting in `user/pkin` off, and the pad files earlier installs left under its own names (`open-field-pad.bin`, `open-field-pad-2.bin`, ...; a file of any other name stays), and it writes `os.ldr` at the top of the drive, the pad file the plan names, and the pack file the plan names, `base.pack`, into `user/pk` (the folder made if it is not there). Before it writes, it lists the disk again and stops if anything changed since the plan was made. It never deletes a folder.

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

```bash
node builder/validate-room.mjs
```

The first runs the unit's check against a made-up bootloader; no MIDI port of the computer is touched. The second builds the image from your own firmware file and holds its SHA-256 to the manifest's; without `--stock` it makes only the checks that need no firmware file. The third holds the install's plan to the placement on made-up disks; it needs no firmware file. The fourth holds the count of the room for packs to made-up pack files, and the numbers it keeps to the files that give them; it needs no firmware file either.

A helper can be run whole without a unit: `-Source` (the `.ps1`) or `--source` (the `.py`) names a folder of the computer that stands for the unit's disk, and `node builder/validate-install.mjs --play <folder> --bootloader <version> --stock <your own os.ldr>` plays the page's part on the helper's folder.

## More than one version of the loader

The loader's file has a version. There are two for OS 1.7.3 today. `v2` is the one the page offers by itself. `v1` stands beside it as the way back: you get it with `?loader=1.7.3-v1` behind the page's address. A version that anyone may have installed keeps its bytes for good, so a changed loader is a new file under a new name, with a manifest of its own. A change to the base, `base.pack`, needs none of this: the base is a file on the unit, and a new one is copied there.

| In this folder | What it is | Who gets it |
| --- | --- | --- |
| `loader.json` and the loader's file it names | The version the page offers by itself | Everyone who opens the page |
| A manifest named after its version, such as `loader-1.7.3-v1.json`, and its loader's file | A version kept beside that one: a new one that waits for a try on a unit, or an older one | Someone who asks for it by name, with `?loader=1.7.3-v1` behind the page's address |

The page shows a line beside the version it has loaded: `ran on a unit` with a date, or `not run on a unit yet`. That line comes from the manifest.

`write-manifest.mjs` makes both files of a version. For `v2`, the page's own, give it `--profile profiles/op1f-1.7.3-n2.json`, the profile of that loader's room. Without it, the tool takes the profile of `v1`. Its first lines are its manual, and [building](../docs/build.md) has the steps under "A change to the loader". A new version is written beside `loader.json`. Making it the one the page offers by itself is the maintainer's step, after a try on a unit.

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

- **This folder alone is served**, as it stands on `main`, with the listing of packs beside it: the workflow joins the files of `catalog/` into it as `packs.json` before it publishes. The page loads only files beside it, by relative paths, and one thing from another site: the file of a listed pack, from its author's repository at the commit the listing names (`raw.githubusercontent.com`), held to the listing's size and SHA-256 before it is used. It downloads a file when the owner ticks it, and, to count the room for packs, a listed file that is on the unit. `tools/tests/test_tree.py` holds that (`ThePagesOnAHost`). Its one link to another site is a link the owner may follow, not a load.
- **Script files ending in `.mjs` are served as scripts.** A browser runs a module only when the server says it is JavaScript.
- **`open-field-helper.ps1` is served as text or as a download.** PowerShell reads it either way. So is `open-field-helper.py`, which `curl` fetches and hands to `python3`. The line the page shows for either is made from the page's own address, so it names the host's address there.
- **`base.pack` and the loader's file are served as they are**, byte for byte: the page holds each to its SHA-256 and refuses one that is not.
- **On the host**, its pages are switched on once, with the workflow as their source. The repository's settings do that; nothing in this folder can.

## What the page cannot do

- It does not press the unit's keys. The write itself is always keys on the unit.
- It does not do the file work itself: a helper does, on Windows, a Mac and Linux. On a Mac or on Linux the page could do it itself over USB, as open-xy's builder does for the OP-XY, but that is not built.
- The placement model knows two bootloader versions, 1000.1.1 and 1000.2.0, and no other. For 1000.2.0 three numbers are the contributor's reading of that bootloader, and the rest of the calculation is 1000.1.1's, assumed for 1000.2.0. The contributor reported that one write on one unit landed with them. The maintainer has not read that bootloader or tried a unit with it. The page's first step reads the version and refuses any other, and the plan names the one it was made for.

[The owner's guide](../docs/guide.md) has the whole install in words.

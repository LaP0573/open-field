# The check before a commit

Nothing of the device's firmware may go into this repository: no byte of its code, no part of its pictures, no list of its texts. A reading of every file by a person cannot promise that, so a tool does it: `tools/publish_check.py`. A hook runs it before every commit and refuses the commit unless the tool answers 0.

## What the tool does

```bash
python -I tools/publish_check.py --strict --stock <records of your own OS 1.7.3 os.ldr> --stock <records of bootloader.ldr> --display <stock pictures> --profile profiles/op1f-1.7.3.json <files or folders>
```

It answers **0** when it found nothing of the firmware in the files, **1** when it found something, **2** when it could not make the check. Each finding names its rule, the file, the line or the byte, and where the same bytes stand in the firmware.

| Rule | What it finds |
| --- | --- |
| `name` | A file that never goes, whatever is in it: a firmware file, an update package, a copy of a unit's storage, a log read from a unit |
| `table` | Bytes of the firmware kept in a table of changes |
| `site` | The firmware's bytes of a place this project changes, anywhere |
| `bytes` | Sixteen bytes or more in a row that stand in the firmware |
| `text` | The firmware's texts in bulk: a row of its strings laid out as its memory has them, or one file that quotes 64 different phrases of it or more, or 4,096 bytes of them. One phrase quoted is fine: see "Quoting what the device shows" below |
| `picture` | What a picture has of the firmware's pictures: shapes, letters, long stretches |
| `hex` | Bytes of the firmware written out as hex |
| `listing` | Lines of a listing of the firmware's code |
| `quote` | Two or more instructions of the firmware written beside one of its addresses |
| `log` | A file that is mostly a log of the unit |

With `--strict`, a file the tool cannot judge counts as a finding. That is so for a picture of pixels (a photo, a screenshot): no rule reads pixels, so a person has to look at it, and it stays out until somebody has.

## What it is held against, and why that is never here

The tool compares files with the maintainer's **own copy** of the firmware: the firmware file and the bootloader file as folders of records, and the unit's own pictures. None of that is in this repository and none of it ever will be. So the check runs on the maintainer's computer, before a commit, and cannot run on a public host: a host has no firmware and must never get one.

Make the records once, outside the repository, from the files of your own unit:

```bash
python -I tools/ldr_dump_records.py <your os.ldr> --out <a folder outside the repository>
```

Then tell your clone where they are. These settings stay on your computer; a clone does not carry them:

```bash
git config core.hooksPath hooks
```

```bash
git config --add openfield.stock <the folder of records of your own OS 1.7.3 os.ldr>
```

```bash
git config --add openfield.stock <the folder of records of bootloader.ldr>
```

```bash
git config openfield.display <a copy of your unit's content/display folder>
```

## The hook

`hooks/pre-commit` runs `tools/commit_check.py`. It takes the tree as it would be committed, runs the tool with `--strict` on every file of it, and lets the commit through only on a 0. It hands the tool the list of places in the profile `build.json` names, `profiles/op1f-1.7.3.json`, and the list of every example pack that changes places of the firmware (`examples/<name>/profile.json`), so the firmware's bytes of an example's places are searched for too. Without the settings above it refuses every commit: a commit never goes through unchecked. Each run leaves a line in `.git/open-field-check.log`.

A change to the check itself is checked by the changed check. Read such a change with care.

## On a host

A host can run the part of the check that needs no firmware:

```bash
python -I tools/publish_check.py --forms-only <files or folders>
```

It looks at names and at the form of a log, of a listing and of quoted instructions. **Its 0 is not the whole check's 0**: it cannot find bytes, texts or pictures of the firmware, because it has nothing to compare with, and it does not judge a binary file at all (it says so for each; with `--strict` that alone makes the answer 1). Use it as a second net, never as the first.

### The checks on every change

On every pull request and every change to `main`, the host runs `.github/workflows/checks.yml` on its standard Windows and Linux machines. It needs no firmware, and it runs:

- the build, `python -I tools/build_all.py`, held to `packs/built.json`, with the compiler fetched as [docs/build.md](build.md) says and kept between runs;
- the tree's own tests, `python -I -m unittest discover -s tools/tests`;
- the install page's three checks, `node builder/validate-builder.mjs` (without a firmware file), `node builder/validate-devmode.mjs` and `node builder/validate-install.mjs`;
- the part of this check that needs no firmware, `--forms-only`, on every file of the tree.

A pull request that changes the listing of packs also runs `.github/workflows/listing.yml`: `tools/catalog_check.py`, held to the listing before the change and to every pack file that changed ([the listing](catalog.md)).

**A green run is not the whole check.** It cannot see bytes, texts or pictures of the firmware in a file, because a host never has the firmware to compare with. The whole check runs on the maintainer's computer, with the maintainer's own copy of the firmware, before a change goes in. A green run says nothing about a unit either: only a try on one does.

## Quoting what the device shows

A page or a comment may quote the device: the words of a menu entry, a line on a screen, a message in its log. What a screen or a log says is no secret, and a guide that quotes it is clearer than one that talks round it. The tool finds such quotes, counts them and lets them through. It reports them as `quoted`.

A copy of the firmware's texts is another matter. One file that quotes 64 different phrases of the firmware, or 4,096 bytes of them, is refused, and so is a row of its strings laid out as its memory has them. Those are lists of the firmware's strings, not quotes. The heaviest page of the working notes this project grew from quotes 32 phrases; a list of one program's strings holds hundreds.

The tool still takes a file of texts with `--allow`, one text a line, for a text a person has read and accepted. The hook hands it a file `publish-allow.txt` from the top of the tree when there is one. There is none, and since quotes pass there is little left that would need one. There is no way to allow bytes.

## What the check cannot see

A 0 means "nothing of the firmware was found by these rules". It does not mean more than that.

- Code of ours that does what a piece of the firmware does, written in another language or with other registers. The bytes differ, so no rule finds it.
- An instruction quoted with no address beside it, and a single instruction beside an address, which the tool lets through as a description.
- A screen of ours laid out like one of the device's.
- What a file is for, and what it says in its own words about the firmware.
- Anything of another firmware version than the one it was given.

These are for people to judge.

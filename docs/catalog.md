# The listing: offering your pack to every owner

The install page offers owners the packs of one listing, [`catalog/packs.json`](../catalog/packs.json), sorted by category, each with a tick. A pack gets into it by a pull request that changes only that file. Nothing of a pack is copied into this repository, but `base.pack`, which comes with the install page: your pack file stays in your own repository, and the listing names it there, at one commit, with the file's SHA-256. The page downloads the file from there and holds it to that SHA-256 before it goes anywhere near a unit ([on the install page](#on-the-install-page)).

An entry may instead follow a branch of your repository. The page then takes your file as it stands on that branch, and you put out a new version with a commit, with no pull request here ([following a branch](#following-a-branch)).

If you have not built a pack yet, start with [a first pack](first-pack.md).

## An entry

```json
{
  "id": "notes",
  "author": "your-github-name",
  "status": "listed",
  "categories": ["tools"],
  "description": "Shift and the help key open a page of notes kept in its own folder.",
  "repository": "https://github.com/your-github-name/notes-pack",
  "commit": "the full commit id, forty digits of 0-9 and a-f",
  "path": "notes.pack",
  "sha256": "the file's SHA-256, sixty-four digits of 0-9 and a-f",
  "bytes": 5120,
  "version": 1,
  "firmware": "1.7.3",
  "places": 0,
  "needs": [],
  "level": 3,
  "folders": [{"name": "notes", "place": "notes"}]
}
```

| Field | What |
| --- | --- |
| `id` | Your pack's name, as its header holds it (its recipe's `name`). The install page names the file after it on the unit, `<id>.pack`, so it is 1 to 15 plain characters, none of `/ \ : * ? " < > |`, with no space or dot at either end. It is held for good: no other pack ever gets it |
| `author` | Your GitHub name, or your organisation's |
| `status` | `reserved`, `bundled`, `listed` or `retired` (below) |
| `categories` | One to three of the listing's categories (below) |
| `description` | One line for the install page, up to 140 plain characters |
| `repository`, `commit`, `path` | Where the file lies: your public repository on GitHub, the full id of a commit in it, and the file's path in the repository at that commit |
| `follow` | Only in a followed entry, in place of `commit`: the name of the branch the page reads the file from. 1 to 100 of `A-Z a-z 0-9 _ . -`, not beginning with `.` or `-`, no `..`, not ending in `.` or `.lock`, no slash |
| `sha256`, `bytes` | The file's SHA-256 and its size |
| `version` | The version in the file's header. It only grows: every new file has a higher one, but for a move to another firmware (below). In a followed entry: the lowest version the page takes from the branch |
| `firmware` | The firmware version the file is built for: a version this repository has a profile for (`profiles/op1f-<version>.json`). Today that is 1.7.3 |
| `places` | How many places of the firmware the pack changes: 0 for a pack that only uses the base's card |
| `needs` | The packs it cannot run without, by their ids: `base` for a pack that wants the base's names and has no card, none for a pack with a card. In a followed entry: the packs the file may need |
| `level` | For a pack with a card: the level of the base it needs, 1 to 10 ([the base](base.md)) |
| `folders` | The folders its card asks for under `user/pf/`: each its name in the disk mode and its place. None to eight. In a followed entry: the folders its card may ask for |
| `picture` | Optional, on a `listed` or `bundled` entry: `{"path": ..., "sha256": ..., "bytes": ...}`, a thumbnail the install page shows in place of the category's pictogram ([a picture](#a-picture)) |

The base's own entry says `offers_level` instead of `level`: the highest level it offers. A pack that needs a higher level than the listed base offers is refused.

Some things a card can ask need a newer base, not a higher level. Today there are two: a `places` ask that gives way ([level 6](level-6.md#giving-way-base-31)) needs base 31 or later, and an `options` ask, settings of the pack's own ([level 6](level-6.md#settings-base-32)), needs base 32 or later. An older base of the same level leaves that ask aside and runs the rest of the pack. While the listed base is older than what your card asks for, the check refuses your entry and names the version it needs.

Everything from `version` on is what the file says of itself. `python -I tools/pack_build.py check --pack <your file>` prints it: `name`, `version`, `entries` (the places), `works with` (what it needs, marked `needed`), `wants` (the level, as `op1f_level_<n>`) and the card's folders. The listing's check reads the file the same way and refuses an entry that says otherwise.

The same pack, following the branch `main` instead of one commit:

```json
{
  "id": "notes",
  "author": "your-github-name",
  "status": "listed",
  "categories": ["tools"],
  "description": "Shift and the help key open a page of notes kept in its own folder.",
  "repository": "https://github.com/your-github-name/notes-pack",
  "follow": "main",
  "path": "notes.pack",
  "version": 1,
  "firmware": "1.7.3",
  "needs": [],
  "folders": [{"name": "notes", "place": "notes"}]
}
```

A followed entry names no `commit`, `sha256`, `bytes`, `places` or `level`: the page reads them from the file each time.

## A picture

A `listed` entry may name a picture, and a `bundled` one may too; no other may, and none needs one. The install page shows it on the pack's tile in the slot of the category's line pictogram (150×120 on a wide screen, 130×104 on a phone), so draw it to sit there: ink lines on a transparent ground rather than a screenshot.

```json
"picture": {"path": "thumbs/notes.png", "sha256": "the picture's SHA-256", "bytes": 3012}
```

- It lies in the same `repository` at the same `commit` as the pack (a bundled entry's, in this repository); `path` is a plain path in it, ending in `.png`.
- A PNG with transparency: an alpha channel, or a `tRNS` chunk.
- Its width to its height exactly 5:4, at most 600×480 (300×240 is plenty), at most 100 KB (102400 bytes).

The listing's check downloads it from raw.githubusercontent.com at the entry's commit, as it does the pack, and holds it to its size, its SHA-256, the PNG signature, and the width, height and transparency its IHDR and chunks say. The install page downloads it when the tile comes into view, holds it to its size, SHA-256 and signature, and only then shows it; a picture that is refused or does not load leaves the pictogram.

## Categories

| Word | On the install page |
| --- | --- |
| `tape` | tape |
| `effects` | effects |
| `modulation` | modulation |
| `sequencing` | sequencing |
| `mixer` | mixer |
| `looks` | screens and looks |
| `tools` | tools |
| `examples` | examples |
| `library` | library |

The list is the maintainer's to change. To ask for a word it lacks, say so in your pull request.

## Status

- **`reserved`**: the id and the folders are held for a pack being written, and nothing is offered yet. A reserved entry names no repository and no commit. Holding a name early is how you make sure nobody else takes it.
- **`bundled`**: the file lies in this repository beside the install page, under `builder/`, and the page puts it on every unit it installs. `base.pack` alone is bundled. Its entry names its path in this repository and no repository or commit, and the check holds it to that file every time, with nothing downloaded.
- **`listed`**: offered on the install page. Every field above is there, or, in a followed entry, every field of the second example.
- **`retired`**: no longer offered. Its id and its folders stay held for good, so that no other pack takes them from owners who still have the file.

An entry only moves that way, and is never taken out.

## This project's own packs

The listing's `ours` names the repositories that hold this project's own packs: the four features lie in a repository of their own beside this one, [open-field-packs](https://github.com/LaP0573/open-field-packs), sources and finished files, and are listed like anyone's.

The examples under `examples/` are for people who write packs, not for owners: their sources lie in this repository, and the build makes their files. Ten of them have an entry, `wink`, `yoyo`, `list`, `sketch`, `echo`, `dub`, `crush`, `ping`, `pluck` and `fold`, and every one of those stays `reserved`. That keeps their names held, and with them the folders `list`, `sketch`, `echo` and `dub`, and the install page never offers them. `trampolines`, the example that changes places of the firmware, has no entry. An entry whose `author` is `open-field` lies in one of those repositories, and an entry in one of them has `open-field` as its author. The check refuses anything else. A pull request never changes `ours`: the check refuses that too, so that no entry can take the place of this project's own.

## On the install page

The install page reads the listing beside it, as `packs.json`: the workflow that publishes the page (`.github/workflows/pages.yml`) copies `catalog/packs.json` there from `main`. In its step 5 it shows:

- `base.pack`, which goes on every unit and has no tick;
- every `listed` entry for the unit's firmware, under its first category, with its description, author, version, what it needs and its size, and a tick;
- on every entry that is not this project's own, the warning: **a pack is a program that can read and write the files on your unit: take packs only from people you trust**;
- on a followed entry, beside the warning, the mark `latest from <author>'s <branch> · not reviewed`, and `latest from open-field's <branch>` on one of this project's own ([following a branch](#following-a-branch)).

When an owner ticks a pack, the page downloads its file from `https://raw.githubusercontent.com/<owner>/<name>/<commit>/<path>`, the entry's repository at the entry's commit, and holds it to the entry's size and SHA-256. A file that differs is refused, named, and its tick taken off. A pack the ticked one needs is ticked with it.

A followed entry's file is downloaded once, when the page has read the listing: from `https://raw.githubusercontent.com/<owner>/<name>/refs/heads/<branch>/<path>`, the head of the entry's branch. The page holds it to the entry as [following a branch](#following-a-branch) says, and offers it only if it passes. Its size, SHA-256, version, places and level are then the file's own. When the owner ticks it, the page puts on the unit the very bytes it checked, with no second download. A file that cannot be downloaded or fails a check is not offered, and the page's log names the pack and the reason.

These downloads are the only things the page loads from another site, and the tree's own test holds that (`tools/tests/test_tree.py`, `ThePagesOnAHost`).

A ticked file goes into the folder `packs` that the page and the helper share, under the name `<id>.pack`, and the plan of the install counts it; the helper copies it onto the unit with the rest, `base.pack` last, and reads every copy back. A file of the same name with other bytes already in `packs` is kept in the folder `replaced` first. One with the same bytes is shown ticked and goes on as it is. The way back to the original firmware puts no pack on.

## Getting your pack listed

1. **Build it and try it on your unit.** The listing's check cannot run a pack; the review asks what you tried and what the unit did.
2. **Commit the pack file into your own public repository on GitHub**, at the path the entry names. A file in the repository, not only a release's download: the install page runs in the owner's browser, and a browser may read a file of a repository at a commit, but not a release's download from another site.
3. **Write the entry** into `catalog/packs.json`, behind the others, with the full id of the commit that holds the file, or with the branch it follows.
4. **Run the check**, with your file in a folder of its own: `python -I tools/catalog_check.py --files <that folder>`. It holds every entry's form, and yours to your file. Where the file's size or SHA-256 differs from the entry, it prints the file's own. The other authors' entries, whose files are not in your folder, pass on their form, and their lines say `its file not held`. If you list more than one pack, put all their files in the folder: once one of your files is there, each of your entries is held to its file, and a missing one is refused. The next section shows it on this repository's own packs.
5. **Open a pull request that changes only `catalog/packs.json`.** Say what the pack does, how you tried it, and where its source is. The checks on the pull request (`.github/workflows/listing.yml`) hold the listing to the one before your change, download your file from the commit, or from the head of the branch, and hold your entry to it, as step 4 did.
6. **The review.** The maintainer reads your source at that commit, or on the branch as it stands then, tries the file on a unit, and merges or says what stands in the way. Once merged, the pack is on the install page: the next publication of the page carries the listing with your entry.

For a pinned entry, a new version is the same pull request again: a new commit, the file's new SHA-256 and size, and a higher `version`. The check refuses a new file whose version did not grow. For a followed entry, a new version is a commit on its branch and no pull request ([following a branch](#following-a-branch)). A pinned entry may come to follow a branch, and a followed one may go back to one commit: each is a pull request, reviewed like any other.

## Following a branch

A followed entry names a branch of your repository with `follow`, in place of a commit. Each time an owner opens the install page, it reads your file at the head of that branch. The entry's `version` is the lowest version it takes from there: the version reviewed when the entry was listed. That number never goes down in a change of the listing.

**To put out a new version**, commit the new file at the entry's path on that branch, with a higher `version` in its recipe. Nothing in this repository changes. The link may give the old file for up to five minutes after your push. Some changes are still a pull request, because the entry names them: a new folder, a pack your file needs that the entry does not name, another firmware, and another repository, path or branch.

**What the page holds**, in the owner's browser, before it offers your file:

1. It is at most 2 MB (2,097,152 bytes).
2. It is a pack the loader takes: format 3, its header's sizes add up to the file's size, its checksum is the header's, its lists can be read, and its card stands where the header says.
3. Its name is the entry's `id`.
4. It is built for the entry's `firmware`.
5. Its version is the entry's `version` or higher.
6. Every pack it needs is in the entry's `needs`.
7. The level of the base it needs is no higher than the listed base offers.
8. Every folder its card asks for, name and place, is in the entry's `folders`.

A file that fails one of them is not offered, and the page's log says which. The listing's check holds the same, and holds the card, the release and the base's version (above) as it does for a pinned file: run `python -I tools/catalog_check.py --fetch` after you push, or `--files` with your file in a folder.

**What the owner sees** on the row of a followed pack, beside the warning: the mark `latest from <author>'s <branch> · not reviewed`. With the pointer on it, it says: "This file comes straight from <author>'s branch <branch> as it stands today: nobody at open-field has looked at this version." On one of this project's own packs, from [open-field-packs](https://github.com/LaP0573/open-field-packs), the mark is `latest from open-field's <branch>`, with no "not reviewed": a new version there is the project's own work.

**What is given up.** Know this before you choose it:

- Nobody looks at a new version before owners get it. Each push on the branch goes straight to the next owner who opens the page.
- The file changes whenever you push. Two owners who install on different days may get different files, and nothing here keeps the older one.
- The check of the pull request ran on the version listed then, and on no later one.

A pinned entry gives up none of this: every new file is a pull request, checked and reviewed before an owner sees it.

## One firmware for each entry

An entry is for one firmware, and there is one entry for each id. A pack file is built for one firmware and carries its mark: the loader leaves the pack out on a unit that runs any other firmware, and the start screen counts it as left out.

When this project moves to another firmware, every entry moves with it:

1. Build the same pack again for the new firmware, with the profile of the new firmware.
2. Commit the new file into your repository. This makes a new commit; the old file stays at its old commit.
3. Change the entry: the new commit, the new SHA-256 and size, and the new `firmware`. The `version` may stay the same.

The check lets a file change at the same version only in this case: the entry's `firmware` changed too, and this repository has a profile for the new firmware. For anything else, the version has to grow, as above. The version never goes down.

Do not delete or rewrite the commit that holds the old file. A page for the old firmware may still name it, and owners who stay on the old firmware download it from there.

A followed entry moves by a pull request too, with its new `firmware`. Put the file for the new firmware on a branch of its own and name that branch in the entry. The old branch keeps the old file for a page of the old firmware: a page offers a followed file only when it is built for the entry's firmware.

## The check on this repository's own packs

The listing's form, and every name held once:

```bash
python -I tools/catalog_check.py
```

After a build ([docs/build.md](build.md)), every entry of this project's own packs held to its file: the base's and the examples' in `out/packs/` and `out/examples/`, the four features' in a checkout of open-field-packs at the commit their entries name:

```bash
python -I tools/catalog_check.py --files out/packs --files out/examples --files <your checkout of open-field-packs>
```

The rule for `--files`: an author with a file in one of the folders is held whole, each of their entries that names a file held to it and a missing one refused. So every entry whose `author` is `open-field` is held here, and a missing file of ours is refused. An entry by another author, whose file is not in these folders, passes on its form, and its line says `its file not held`; `--fetch` holds it to its file. Folders that hold no entry's file at all are refused: a file is looked for by the last part of its entry's `path`, or as `<id>.pack`.

Every listed entry held to its file downloaded from its repository at its commit, or at the head of the branch it follows, as the install page downloads it:

```bash
python -I tools/catalog_check.py --fetch
```

Each line says `ok` or `REFUSED` with the reason, and the last line sums it up. It answers 0 when every entry is in order.

## What is held, and what is not

The listing holds the names two packs may not share: the id, and each folder's name and place, in small letters or capitals. A folder never takes a name the disk mode already shows: `synth`, `drum`, `tapes`, `mixdown` (the firmware's), `packs` (the loader's), `fx` and `lfo` (the base's).

It does not hold keys. When two packs pick the same keys for their pages, their authors sort it out between them.

## A pack is code

A pack runs inside the firmware with nothing walled off from it. The install page says so on every pack that is not this project's own, and an owner chooses. Review is a person reading your source and trying your file; it is no promise that a pack is safe. A followed pack's later versions get no review at all, and the page marks them as not reviewed.

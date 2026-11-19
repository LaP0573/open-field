# The listing: offering your pack to every owner

The install page offers owners the packs of one listing, [`catalog/packs.json`](../catalog/packs.json), sorted by category, each with a tick. A pack gets into it by a pull request that changes only that file. Nothing of a pack is copied into this repository, but `base.pack`, which comes with the install page: your pack file stays in your own repository, and the listing names it there, at one commit, with the file's SHA-256. The page downloads the file from there and holds it to that SHA-256 before it goes anywhere near a unit ([on the install page](#on-the-install-page)).

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
| `sha256`, `bytes` | The file's SHA-256 and its size |
| `version` | The version in the file's header. It only grows: every new file has a higher one, but for a move to another firmware (below) |
| `firmware` | The firmware version the file is built for: a version this repository has a profile for (`profiles/op1f-<version>.json`). Today that is 1.7.3 |
| `places` | How many places of the firmware the pack changes: 0 for a pack that only uses the base's card |
| `needs` | The packs it cannot run without, by their ids: `base` for a pack built as the four features are, none for a pack with a card |
| `level` | For a pack with a card: the level of the base it needs, 1 to 3 ([the base](base.md)) |
| `folders` | The folders its card asks for under `user/pf/`: each its name in the disk mode and its place. None to eight |

The base's own entry says `offers_level` instead of `level`: the highest level it offers. A pack that needs a higher level than the listed base offers is refused.

Everything from `version` on is what the file says of itself. `python -I tools/pack_build.py check --pack <your file>` prints it: `name`, `version`, `entries` (the places), `works with` (what it needs, marked `needed`), `wants` (the level, as `op1f_level_<n>`) and the card's folders. The listing's check reads the file the same way and refuses an entry that says otherwise.

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
- **`listed`**: offered on the install page. Every field above is there.
- **`retired`**: no longer offered. Its id and its folders stay held for good, so that no other pack takes them from owners who still have the file.

An entry only moves that way, and is never taken out.

## This project's own packs

The listing's `ours` names the repositories that hold this project's own packs: the four features lie in a repository of their own, beside this one, and are listed like anyone's.

The three examples, `wink`, `yoyo` and `list`, are for people who write packs, not for owners: their sources lie in this repository under `examples/`, and the build makes their files. Their entries stay `reserved`, so that their names and the folder `list` stay held, and the install page never offers them. An entry whose `author` is `open-field` lies in one of those repositories, and an entry in one of them has `open-field` as its author; the check refuses anything else. A pull request never changes `ours`: the check refuses that too, so that no entry can take the place of this project's own.

## On the install page

The install page reads the listing beside it, as `packs.json`: the workflow that publishes the page (`.github/workflows/pages.yml`) copies `catalog/packs.json` there from `main`. In its step 5 it shows:

- `base.pack`, which goes on every unit and has no tick;
- every `listed` entry for the unit's firmware, under its first category, with its description, author, version, what it needs and its size, and a tick;
- on every entry that is not this project's own, the warning: **a pack is a program that can read and write the files on your unit: take packs only from people you trust**.

When an owner ticks a pack, the page downloads its file from `https://raw.githubusercontent.com/<owner>/<name>/<commit>/<path>`, the entry's repository at the entry's commit, and holds it to the entry's size and SHA-256. A file that differs is refused, named, and its tick taken off. This download is the only thing the page loads from another site, and the tree's own test holds that (`tools/tests/test_tree.py`, `ThePagesOnAHost`). A pack the ticked one needs is ticked with it.

A ticked file goes into the folder `packs` that the page and the helper share, under the name `<id>.pack`, and the plan of the install counts it; the helper copies it onto the unit with the rest, `base.pack` last, and reads every copy back. A file of the same name with other bytes already in `packs` is kept in the folder `replaced` first. One with the same bytes is shown ticked and goes on as it is. The way back to the original firmware puts no pack on.

## Getting your pack listed

1. **Build it and try it on your unit.** The listing's check cannot run a pack; the review asks what you tried and what the unit did.
2. **Commit the pack file into your own public repository on GitHub**, at the path the entry names. A file in the repository, not only a release's download: the install page runs in the owner's browser, and a browser may read a file of a repository at a commit, but not a release's download from another site.
3. **Write the entry** into `catalog/packs.json`, behind the others, with the full id of the commit that holds the file.
4. **Run the check**, with your file in a folder of its own: `python -I tools/catalog_check.py --files <that folder>`. It holds every entry's form, and yours to your file. Where the file's size or SHA-256 differs from the entry, it prints the file's own. The next section shows it on this repository's own packs.
5. **Open a pull request that changes only `catalog/packs.json`.** Say what the pack does, how you tried it, and where its source is. The checks on the pull request (`.github/workflows/listing.yml`) hold the listing to the one before your change, download your file from the commit and hold your entry to it, as step 4 did.
6. **The review.** The maintainer reads your source at that commit, tries the file on a unit, and merges or says what stands in the way. Once merged, the pack is on the install page: the next publication of the page carries the listing with your entry.

A new version is the same pull request again: a new commit, the file's new SHA-256 and size, and a higher `version`. The check refuses a new file whose version did not grow.

## One firmware for each entry

An entry is for one firmware, and there is one entry for each id. A pack file is built for one firmware and carries its mark: the loader leaves the pack out on a unit that runs any other firmware, and the start screen counts it as left out.

When this project moves to another firmware, every entry moves with it:

1. Build the same pack again for the new firmware, with the profile of the new firmware.
2. Commit the new file into your repository. This makes a new commit; the old file stays at its old commit.
3. Change the entry: the new commit, the new SHA-256 and size, and the new `firmware`. The `version` may stay the same.

The check lets a file change at the same version only in this case: the entry's `firmware` changed too, and this repository has a profile for the new firmware. For anything else, the version has to grow, as above. The version never goes down.

Do not delete or rewrite the commit that holds the old file. A page for the old firmware may still name it, and owners who stay on the old firmware download it from there.

## The check on this repository's own packs

The listing's form, and every name held once:

```bash
python -I tools/catalog_check.py
```

After a build ([docs/build.md](build.md)), every entry of this project's own packs held to the file the build gave, in `out/packs/` and `out/examples/`:

```bash
python -I tools/catalog_check.py --files out/packs --files out/examples
```

Every listed entry held to its file downloaded from its repository at its commit, as the install page downloads it:

```bash
python -I tools/catalog_check.py --fetch
```

Each line says `ok` or `REFUSED` with the reason, and the last line sums it up. It answers 0 when every entry is in order.

## What is held, and what is not

The listing holds the names two packs may not share: the id, and each folder's name and place, in small letters or capitals. A folder never takes a name the disk mode already shows: `synth`, `drum`, `tapes`, `mixdown` (the firmware's), `packs` (the loader's), `fx` and `lfo` (the base's).

It does not hold keys. When two packs pick the same keys for their pages, their authors sort it out between them.

## A pack is code

A pack runs inside the firmware with nothing walled off from it. The install page says so on every pack that is not this project's own, and an owner chooses. Review is a person reading your source and trying your file; it is no promise that a pack is safe.

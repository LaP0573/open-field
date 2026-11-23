#!/usr/bin/env python3
"""Check the listing of packs, catalog/packs.json: what the install page offers an owner (docs/catalog.md).

  python -I tools/catalog_check.py                                  # the listing's form: every entry, every name held once
  python -I tools/catalog_check.py --files out/packs --files out/examples   # and the entries of each author with a file there held to their files
  python -I tools/catalog_check.py --fetch                          # and every listed entry held to its file, downloaded at its commit or from its branch
  python -I tools/catalog_check.py --before before.json --fetch     # a pull request: against the listing before it, fetching what changed

An entry names a pack file that lies in its author's own repository, at one commit, with the file's SHA-256; nothing
of a pack is copied into this repository, but base.pack. An entry is `reserved` (its id and its folders held, no file
to offer yet), `bundled` (its file lies in this repository beside the install page, under builder/, and the page puts
it on every unit it installs: base.pack alone), `listed` (offered on the install page) or `retired` (no longer
offered; its id and its folders stay held for good). It only moves that way.

A listed or retired entry may follow a branch instead (`follow`, the branch's name): its file is the one at the head
of that branch, as it stands when the install page opens, so its author puts out a new version with a commit and no
pull request. A followed entry names no commit, SHA-256, size, places or level: the page reads them from the file.
Its `version` is the lowest the page takes, its `needs` and its `folders` the most the file may ask for.

What is held, with nothing but this repository:

- the form of every entry, and that it holds the fields its status asks for and no other;
- that no two entries share an id or a folder, in small letters or capitals, and that no folder takes a name the
  firmware, the loader or the base already shows in the disk mode; the names fx and lfo are kept for open-field's
  own packs (an entry by open-field may take them, once);
- that every pack an entry needs is in the listing, and that no pack needs a level the listed base does not offer.

A bundled entry is always held to its file in this repository. With a pack file at hand (`--files`, or `--fetch`,
which downloads it from its repository at the entry's commit):
its size and its SHA-256, and that what the file says of itself is what the entry says: its name, its version, the
firmware it is for, how many places of the firmware it changes, the packs it needs, the level of the base it needs
or offers, the folders its card asks for. The file is read by tools/pack_build.py, as `check` reads it. And that its
card asks for nothing the listing's base for its firmware is too old to serve (BASE_SINCE: an ask of places that gives
way needs base 31 or later, an ask of options base 32 or later), which is no level: an older base of the same level
leaves that ask aside and runs the rest of the pack.
An entry's picture (a transparent PNG thumbnail, 5:4, at most 600x480 and 100 KB) is held the same way, from the same
repository at the same commit: its size, its SHA-256, and what its IHDR and chunks say.

With `--files`, an author whose file lies in one of the folders is held whole: every entry of theirs that names a file
is held to it, and one whose file is not there is refused. So open-field's own packs are all held when the folders of
a build are given, and an author's own are when their file is. An entry of an author with no file there is held to its
form only, and its line says so. Folders that hold no entry's file at all are refused.

A followed entry's file (`--files`, or `--fetch`, which downloads it from the head of its branch) is held as the
install page holds it in the owner's browser before it offers it: at most 2 MB; a pack of format 3 whose header's
sizes and checksum are the file's, its lists readable, its card where the header says; its name the entry's id; built
for the entry's firmware; its version at least the entry's; no pack needed that the entry's `needs` do not name; no
level of the base above the one the listed base offers; no folder that the entry's `folders` do not name. Its card,
its release and the base's version it needs are held as a pinned file's are (the page does not hold the version: it
puts the listed base on the unit with every pack).

The listing's `ours` names the repositories of open-field's own packs: an entry whose author is `open-field` and
that names a repository names one of them, and an entry in one of them has `open-field` as its author. The install
page shows the warning on a pack that is not ours on every other entry.

With `--before`, the listing before the change: no entry is taken out, no status moves back, an id is never changed,
a new file comes with a higher version, no version goes down (a followed entry's lowest included), and `ours` stays
as it was. One new file may keep its version: the same pack built for another firmware, when the entry's `firmware`
changed too and this repository has a profile for the new one. A pinned entry may come to follow a branch, and back:
that is a change like any other. With `--fetch`, only the entries that are new or name another file or another
branch are downloaded then, and a followed entry whose version, firmware, needs or folders changed.

What it does not hold, and the review does: that the source at the commit is what the author says, and that the
pack does what its description says on a unit. Nobody holds a followed entry's later versions so: each new commit on
its branch reaches owners unread. It answers 0 when every entry is in order, 1 otherwise.
"""

import argparse
import hashlib
import importlib.util
import json
import os
import re
import struct
import sys
import urllib.parse
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CATALOG = os.path.join(ROOT, "catalog", "packs.json")
PROFILES = os.path.join(ROOT, "profiles")


def tool(name):
    spec = importlib.util.spec_from_file_location("catalog_" + name, os.path.join(ROOT, "tools", name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


pack_build = tool("pack_build")

STATUSES = ("reserved", "bundled", "listed", "retired")          # the way an entry moves, never back
ALWAYS = ("id", "author", "status", "categories", "description")
LOCATION = ("repository", "commit", "path")                     # where a listed entry's file lies
BUNDLED_IN = "builder/"                                         # where a bundled entry's file lies in this repository: beside the install page
FILE_FACTS = ("sha256", "bytes", "version", "firmware", "places", "needs", "folders")   # what the file says, held to it
OPTIONAL = ("level", "offers_level")                            # a pack with a card needs a level; the base offers them
FOLLOW = "follow"                                               # the branch a followed entry's file is read from, instead of a commit
FOLLOWED = ("repository", "path", FOLLOW, "version", "firmware", "needs", "folders")   # what a followed entry says, with ALWAYS
PINNED = ("commit", "sha256", "bytes", "places") + OPTIONAL     # what a followed entry never says: the page reads it from the file
FOLLOWING = ("listed", "retired")                               # the statuses that may follow a branch
PICTURE = "picture"                                             # a listed entry's thumbnail, beside its pack
KEYS = set(ALWAYS + LOCATION + FILE_FACTS + OPTIONAL + (FOLLOW, PICTURE))
TOP_KEYS = {"about", "categories", "packs"}
TOP_OPTIONAL = {"ours"}                                         # the repositories of open-field's own packs
OURS = "open-field"                                             # the author of open-field's own packs
KEPT_FOR_OURS = {name: "kept for %s's own packs: the base showed it itself up to version 20, and from 21 it is the folder of the pack %r"
                 % (OURS, pack) for name, pack in (("fx", "fx train"), ("lfo", "lfo"))}   # only an entry by open-field takes these

NOT_IN_A_NAME = set('/\\:*?"<>|')                              # what no file name on the unit or on a computer holds
AUTHOR_FORM = re.compile(r"[A-Za-z0-9](?:[A-Za-z0-9-]{0,37}[A-Za-z0-9])?")             # a GitHub user or organisation
REPOSITORY_FORM = re.compile(r"https://github\.com/([A-Za-z0-9](?:[A-Za-z0-9-]{0,37}[A-Za-z0-9])?)/([A-Za-z0-9._-]{1,100})")
COMMIT_FORM = re.compile(r"[0-9a-f]{40}")
BRANCH_FORM = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.-]{0,99}")  # a branch's name as the listing takes one: no slash
SHA256_FORM = re.compile(r"[0-9a-f]{64}")
CATEGORY_FORM = re.compile(r"[a-z]{1,16}")
PATH_PART = re.compile(r"[A-Za-z0-9 _.()+-]+")
VERSION_FORM = re.compile(r"op1f-(\d+\.\d+\.\d+)\.json")       # a profile, profiles/op1f-<firmware>.json
LEVEL_NAME = "op1f_level_"
CATEGORIES_MOST = 3
DESCRIPTION_MOST = 140
FOLDERS_MOST = pack_build.FOLDERS_MOST
LARGEST = 2 * 1024 * 1024                                       # a pack file larger than this is not downloaded
PICTURE_LARGEST = 100 * 1024                                    # a picture larger than this is refused
PICTURE_SIDE_MOST = (600, 480)                                  # its width and its height at most, in pixels
PICTURE_ASPECT = (5, 4)                                         # its width to its height, exactly: the tile's pictogram slot
PICTURE_KINDS = (".png",)
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
RAW = "https://raw.githubusercontent.com/%s/%s/%s/%s"           # a file of a repository at a commit: a web page can read it too
BRANCH_HEAD = "refs/heads/"                                     # before a branch's name in that link: the head of the branch
UNREADABLE = (ValueError, IndexError, KeyError, TypeError, OverflowError, struct.error)   # what reading a file nobody built here may raise
NAME_MOST = 255                                                 # the longest name a followed file's lists may hold, as the install page takes it
PLACES_KIND = 6                                                 # the level from which a card may ask for places (and, from base 32, options)
GIVES_WAY = getattr(pack_build, "PLACES_GIVES_WAY", 1)          # the places ask's flag that it gives way


def gives_way(data):
    """Whether the file's card asks for places that give way (its ask head's flag OP1F_PLACES_GIVES_WAY)."""
    card = pack_build.card_in(data)
    if card is None or card["level"] < PLACES_KIND:
        return False
    places = pack_build.ASK_KINDS_OF_LEVEL[PLACES_KIND]["places"]
    return any(kind == places and ask is not None and ask[2] & GIVES_WAY for kind, _size, _what, ask, _wrong in pack_build.ask_tables(data))


OPTIONS_KIND = getattr(pack_build, "OPTIONS", 10)               # the kind of an ask of options, the pack's settings (base 32)


def asks_options(data):
    """Whether the file's card asks for options (an ask of kind OP1F_ASK_OPTIONS), on a card of level 6 or later."""
    card = pack_build.card_in(data)
    if card is None or card["level"] < PLACES_KIND:
        return False
    return any(kind == OPTIONS_KIND for kind, _size, _what, _ask, _wrong in pack_build.ask_tables(data))


# What a card may ask that a base serves only from one of its versions on, with no new level: (what, that version, how
# it is read in a file). A base of the same level before that version leaves the ask aside, says so in its log and
# runs the rest of the pack. A pack that asks it is listed only beside a base of that version or later for its
# firmware: the base the install page puts on the unit with every pack.
BASE_SINCE = (
    ("places that give way", 31, gives_way),
    ("options", 32, asks_options),
)


def base_needed(data):
    """(the base's lowest version that serves what the file's card asks, what it asks) for the highest of BASE_SINCE
    the file asks, or None."""
    asked = [(version, what) for what, version, asks in BASE_SINCE if asks(data)]
    return max(asked) if asked else None


def base_faults(entry, data, base):
    """What the file's card asks that the listing's base, of version `base` for the entry's firmware, is too old to
    serve, as texts."""
    needed = base_needed(data)
    if needed is None or needed[0] <= base:
        return []
    return ["the file's card asks for %s, which the base serves from its version %d, and the listing's base for %s is %s"
            % (needed[1], needed[0], entry["firmware"], "version %d" % base if base else "none with a file")]


def is_int(value):
    return type(value) is int                                   # JSON's true and false are no numbers here


def plain(text, most):
    return isinstance(text, str) and 0 < len(text) <= most and all(0x20 <= ord(c) < 0x7f for c in text)


def id_in_form(text):
    """A pack's id: its name as its header holds it, one to fifteen plain characters, which the install page also
    names its file after on the unit (`<id>.pack`): none of / \\ : * ? " < > |, no space or dot at either end."""
    return plain(text, 15) and not NOT_IN_A_NAME & set(text) and text[0] not in " ." and text[-1] not in " ."


def branch_in_form(text):
    """A followed entry's branch: one to a hundred of A-Z a-z 0-9 _ . -, not beginning with a dot or a dash, no two
    dots together, not ending in a dot or in .lock. No slash, so that its link names the branch and nothing else."""
    return (isinstance(text, str) and BRANCH_FORM.fullmatch(text) is not None and ".." not in text
            and not text.endswith(".") and not text.endswith(".lock"))


def firmwares(folder=PROFILES):
    """The firmware versions this tree has a profile for, by the word a pack's header names its firmware with."""
    out = {}
    for name in sorted(os.listdir(folder)):
        found = VERSION_FORM.fullmatch(name)
        if found:
            with open(os.path.join(folder, name), encoding="utf-8") as f:
                out[pack_build.os_id(json.load(f)["stock_sha256"])] = found.group(1)
    return out


def path_faults(path):
    if not plain(path, 200):
        return ["its path is not one to two hundred plain characters"]
    parts = path.split("/")
    if not path.lower().endswith(".pack"):
        return ["its path does not end in .pack"]
    if any(part in ("", ".", "..") or not PATH_PART.fullmatch(part) for part in parts):
        return ["its path is not a plain path inside the repository: parts of letters, digits, spaces and . _ ( ) + -, parted by /"]
    return []


def picture_faults(picture):
    """The form of an entry's picture: {path, sha256, bytes}, a transparent PNG in the pack's repository."""
    if not isinstance(picture, dict) or set(picture) != {"path", "sha256", "bytes"}:
        return ["its picture is not a path, a sha256 and a size in bytes"]
    faults = []
    path = picture["path"]
    if not plain(path, 200) or not path.lower().endswith(PICTURE_KINDS):
        faults.append("its picture's path is not one to two hundred plain characters ending in .png")
    elif any(part in ("", ".", "..") or not PATH_PART.fullmatch(part) for part in path.split("/")):
        faults.append("its picture's path is not a plain path inside the repository: parts of letters, digits, spaces and . _ ( ) + -, parted by /")
    if not (isinstance(picture["sha256"], str) and SHA256_FORM.fullmatch(picture["sha256"])):
        faults.append("its picture's sha256 is not sixty-four digits of 0-9 and a-f")
    if not (is_int(picture["bytes"]) and 0 < picture["bytes"] <= PICTURE_LARGEST):
        faults.append("its picture's size is not a number of bytes from 1 to %d" % PICTURE_LARGEST)
    return faults


def png_facts(data):
    """A PNG's (width, height, whether it has transparency), read from its IHDR and its chunks: or a text that says why not."""
    if data[:8] != PNG_SIGNATURE:
        return "it is not a PNG"
    if len(data) < 33 or data[12:16] != b"IHDR":
        return "it is a PNG with no IHDR first"
    width, height, colour = int.from_bytes(data[16:20], "big"), int.from_bytes(data[20:24], "big"), data[25]
    alpha = colour in (4, 6)                                    # grey or colour with an alpha channel
    at = 8
    while not alpha and at + 8 <= len(data):                     # or a palette or a colour made transparent by tRNS
        length, kind = int.from_bytes(data[at:at + 4], "big"), data[at + 4:at + 8]
        if kind == b"tRNS":
            alpha = True
        if kind in (b"IDAT", b"IEND"):
            break
        at += 12 + length
    return width, height, alpha


def picture_file_faults(picture, data):
    """What a picture's file is that the entry does not say, or that a picture may not be: none for one in order."""
    faults = []
    if len(data) != picture["bytes"]:
        faults.append("the picture has %d bytes, the entry says %d" % (len(data), picture["bytes"]))
    if hashlib.sha256(data).hexdigest() != picture["sha256"]:
        return faults + ["the picture's SHA-256 is %s, not the entry's" % hashlib.sha256(data).hexdigest()]
    facts = png_facts(data)
    if isinstance(facts, str):
        return faults + ["the picture: %s" % facts]
    width, height, alpha = facts
    if not (0 < width <= PICTURE_SIDE_MOST[0] and 0 < height <= PICTURE_SIDE_MOST[1]):
        faults.append("the picture is %dx%d: it is at most %dx%d" % ((width, height) + PICTURE_SIDE_MOST))
    elif width * PICTURE_ASPECT[1] != height * PICTURE_ASPECT[0]:
        faults.append("the picture is %dx%d: its width to its height is not exactly %d:%d" % ((width, height) + PICTURE_ASPECT))
    if not alpha:
        faults.append("the picture has no transparency: a PNG with an alpha channel or a tRNS chunk")
    return faults


def folder_faults(folders):
    if not isinstance(folders, list) or len(folders) > FOLDERS_MOST:
        return ["its folders are not a list of none to %d" % FOLDERS_MOST]
    faults = []
    for folder in folders:
        if not isinstance(folder, dict) or set(folder) != {"name", "place"}:
            faults.append("a folder is not a name and a place")
        elif not all(isinstance(folder[key], str) and pack_build.FOLDER_FORM.fullmatch(folder[key]) for key in ("name", "place")):
            faults.append("the folder %r is not named as the base takes a folder's name" % (folder,))
    return faults


def entry_faults(entry, categories, known_firmwares):
    """What is not in order with one entry's form, as texts."""
    if not isinstance(entry, dict):
        return ["it is not an entry"]
    faults = []
    status = entry.get("status")
    if status not in STATUSES:
        return ["its status is not one of %s" % ", ".join(STATUSES)]
    unknown = sorted(set(entry) - KEYS)
    if unknown:
        faults.append("it has keys the listing does not know: %s" % ", ".join(unknown))
    missing = [key for key in ALWAYS if key not in entry]
    followed = FOLLOW in entry
    if followed and status in FOLLOWING:                        # its file is read from its branch: the entry says what may come
        missing += [key for key in FOLLOWED if key not in entry]
    elif status == "listed":
        missing += [key for key in LOCATION + FILE_FACTS if key not in entry]
    elif status == "bundled":
        missing += [key for key in ("path",) + FILE_FACTS if key not in entry]
    elif any(key in entry for key in FILE_FACTS):               # a reserved or retired entry with a file says all of it
        missing += [key for key in FILE_FACTS if key not in entry]
    if missing:
        faults.append("it lacks %s" % ", ".join(missing))
    if followed and status not in FOLLOWING:
        faults.append("only a listed or a retired entry follows a branch")
    named = [key for key in PINNED if key in entry]
    if followed and named:
        faults.append("a followed entry names no %s: the install page reads them from its file" % ", ".join(named))
    if followed and not branch_in_form(entry[FOLLOW]):
        faults.append("its follow is not a branch's name: one to a hundred of A-Z a-z 0-9 _ . -, not beginning with . or -, "
                      "no .., not ending in . or .lock, no slash")
    if status == "reserved" and any(key in entry for key in ("repository", "commit")):
        faults.append("a reserved entry names no repository or commit: it has no file to offer yet")
    if status == "bundled" and any(key in entry for key in ("repository", "commit")):
        faults.append("a bundled entry names no repository or commit: its file lies in this repository")
    if status == "bundled" and isinstance(entry.get("path"), str) and not entry["path"].startswith(BUNDLED_IN):
        faults.append("a bundled entry's file lies beside the install page: its path begins with %s" % BUNDLED_IN)
    if "id" in entry and not id_in_form(entry["id"]):
        faults.append("its id is not one to fifteen plain characters that can name a file: none of / \\ : * ? \" < > |, no space or dot at either end")
    if "author" in entry and not (isinstance(entry["author"], str) and AUTHOR_FORM.fullmatch(entry["author"])):
        faults.append("its author is not a GitHub user's or organisation's name")
    if "categories" in entry:
        chosen = entry["categories"]
        if (not isinstance(chosen, list) or not 0 < len(chosen) <= CATEGORIES_MOST or len(set(map(str, chosen))) != len(chosen)
                or any(not isinstance(c, str) or c not in categories for c in chosen)):
            faults.append("its categories are not one to %d of the listing's: %s" % (CATEGORIES_MOST, ", ".join(categories)))
    if "description" in entry and not plain(entry["description"], DESCRIPTION_MOST):
        faults.append("its description is not one line of one to %d plain characters" % DESCRIPTION_MOST)
    if "repository" in entry and not (isinstance(entry["repository"], str) and REPOSITORY_FORM.fullmatch(entry["repository"])):
        faults.append("its repository is not https://github.com/<owner>/<name>")
    if "commit" in entry and not (isinstance(entry["commit"], str) and COMMIT_FORM.fullmatch(entry["commit"])):
        faults.append("its commit is not a full commit id: forty digits of 0-9 and a-f")
    if "path" in entry:
        faults += path_faults(entry["path"])
    if "sha256" in entry and not (isinstance(entry["sha256"], str) and SHA256_FORM.fullmatch(entry["sha256"])):
        faults.append("its sha256 is not sixty-four digits of 0-9 and a-f")
    if "bytes" in entry and not (is_int(entry["bytes"]) and pack_build.HEADER3_BYTES < entry["bytes"] <= LARGEST):
        faults.append("its size is not a number of bytes from %d to %d" % (pack_build.HEADER3_BYTES + 1, LARGEST))
    if "version" in entry and not (is_int(entry["version"]) and entry["version"] >= 1):
        faults.append("its version is not a whole number from 1")
    if "firmware" in entry and entry["firmware"] not in known_firmwares.values():
        faults.append("its firmware is not one this repository has a profile for: %s" % ", ".join(sorted(set(known_firmwares.values()))))
    if "places" in entry and not (is_int(entry["places"]) and entry["places"] >= 0):
        faults.append("its count of places is not a whole number from 0")
    if "needs" in entry and not (isinstance(entry["needs"], list) and all(isinstance(n, str) for n in entry["needs"])
                                 and len(set(entry["needs"])) == len(entry["needs"])):
        faults.append("what it needs is not a list of packs' ids, each once")
    for key in OPTIONAL:
        if key in entry and not (is_int(entry[key]) and entry[key] >= 1):
            faults.append("its %s is not a whole number from 1" % key.replace("_", " "))
    if "folders" in entry:
        faults += folder_faults(entry["folders"])
    if PICTURE in entry:
        if status != "listed":                                  # the base row shows no picture
            faults.append("only a listed entry has a picture")
        elif followed:                                          # the page shows a picture only from the entry's commit
            faults.append("a followed entry has no picture: a picture lies at the commit an entry names")
        faults += picture_faults(entry[PICTURE])
    return faults


def base_offers(packs, out_of_form=(), firmware=None):
    """The highest level of the base the listing offers: the highest `offers_level` of an entry in order that is not
    retired, 0 for none. A pack's entry needs no higher level, and a followed pack's file needs none either: for it,
    only a base for its own firmware counts (`firmware`; an entry that names none counts for every one), as the
    install page takes the bundled base of the unit's firmware."""
    return max((entry["offers_level"] for number, entry in enumerate(packs) if number not in out_of_form and isinstance(entry, dict)
                and entry.get("status") != "retired" and is_int(entry.get("offers_level"))
                and (firmware is None or entry.get("firmware", firmware) == firmware)), default=0)


def base_version(packs, out_of_form=(), firmware=None):
    """The version of the base the listing offers for a firmware, as base_offers picks the base: the highest `version`
    of an entry in order that offers a level and is not retired, 0 for none with a file. What a pack's card asks needs
    no later version (BASE_SINCE)."""
    return max((entry["version"] for number, entry in enumerate(packs) if number not in out_of_form and isinstance(entry, dict)
                and entry.get("status") != "retired" and is_int(entry.get("offers_level")) and is_int(entry.get("version"))
                and (firmware is None or entry.get("firmware", firmware) == firmware)), default=0)


def listing_faults(listing, known_firmwares):
    """What is not in order with the listing: (faults of the whole, {index of an entry: its faults})."""
    whole, each = [], {}
    if not isinstance(listing, dict) or not TOP_KEYS <= set(listing) <= TOP_KEYS | TOP_OPTIONAL:
        return ["the listing is not an object with %s, %s if it likes, and nothing else"
                % (", ".join(sorted(TOP_KEYS)), ", ".join(sorted(TOP_OPTIONAL)))], each
    if not plain(listing["about"], 2000):
        whole.append("its about is not one line of plain characters")
    ours = listing.get("ours", [])
    if (not isinstance(ours, list) or len(set(map(str, ours))) != len(ours)
            or not all(isinstance(r, str) and REPOSITORY_FORM.fullmatch(r) for r in ours)):
        whole.append("its ours is not a list of repositories, https://github.com/<owner>/<name>, each once")
        ours = []
    categories = listing["categories"]
    if (not isinstance(categories, dict) or not categories
            or not all(CATEGORY_FORM.fullmatch(key) and plain(label, 30) for key, label in categories.items())):
        whole.append("its categories are not words of a to z, each with a label of up to 30 plain characters")
        categories = {}
    packs = listing["packs"]
    if not isinstance(packs, list):
        return whole + ["its packs are not a list"], each
    for number, entry in enumerate(packs):
        faults = entry_faults(entry, categories, known_firmwares)
        if faults:
            each[number] = faults
        elif entry["author"] == OURS and "repository" in entry and entry["repository"] not in ours:
            each[number] = ["its author is %s, and its repository is not one of the listing's ours" % OURS]
        elif entry["author"] != OURS and entry.get("repository") in ours:
            each[number] = ["its repository is one of the listing's ours, and its author is not %s" % OURS]
    held_ids, held_folders = {}, {}
    taken = {name.casefold(): whose for name, whose in pack_build.FOLDER_NAMES_TAKEN.items()}
    for number, entry in enumerate(packs):
        if number in each:                                      # an entry out of form is not held against the others
            continue
        key = entry["id"].casefold()
        if key in held_ids:
            each.setdefault(number, []).append("its id is the id of %r, in small letters or capitals" % held_ids[key])
        held_ids.setdefault(key, entry["id"])
        for folder in entry.get("folders", []):
            for part in ("name", "place"):
                word = folder[part].casefold()
                if word in taken:
                    each.setdefault(number, []).append("its folder's %s %r is %s" % (part, folder[part], taken[word]))
                elif word in KEPT_FOR_OURS and entry["author"] != OURS:
                    each.setdefault(number, []).append("its folder's %s %r is %s" % (part, folder[part], KEPT_FOR_OURS[word]))
                elif (part, word) in held_folders and held_folders[(part, word)] != entry["id"]:
                    each.setdefault(number, []).append("its folder's %s %r is held by %r" % (part, folder[part], held_folders[(part, word)]))
                held_folders.setdefault((part, word), entry["id"])
    present = {entry["id"]: entry for number, entry in enumerate(packs) if number not in each}
    offered = base_offers(packs, each)
    for number, entry in enumerate(packs):
        if number in each:
            continue
        for needed in entry.get("needs", []):
            if needed not in present or present[needed]["status"] == "retired":
                each.setdefault(number, []).append("it needs %r, which the listing does not offer" % needed)
        if entry.get("level", 0) > offered:
            each.setdefault(number, []).append("it needs level %d of the base, and the listing's base offers %d" % (entry["level"], offered))
    return whole, each


def level_of(names, needed_only):
    levels = [int(name[len(LEVEL_NAME):]) for name, flag in names if name.startswith(LEVEL_NAME) and name[len(LEVEL_NAME):].isdigit()
              and (flag or not needed_only)]
    return max(levels) if levels else None


def file_faults(entry, data, known_firmwares, base=None):
    """What the pack file says of itself that the entry does not, as texts: none for a file that is the entry's. With
    `base`, the version of the listing's base for the entry's firmware (base_version), also what its card asks that
    this base is too old to serve."""
    faults = []
    if len(data) != entry["bytes"]:
        faults.append("the file has %d bytes, the entry says %d" % (len(data), entry["bytes"]))
    if hashlib.sha256(data).hexdigest() != entry["sha256"]:
        faults.append("the file's SHA-256 is %s, not the entry's" % hashlib.sha256(data).hexdigest())
        return faults                                           # another file: what it says of itself does not matter
    try:
        fields, _body = pack_build.parse(data)
    except ValueError as problem:
        return faults + ["the file is no pack the loader takes: %s" % problem]
    if fields["format"] != pack_build.FORMAT_LINKED:
        return faults + ["the file is a pack of format %d, which the loader no longer takes" % fields["format"]]
    lists = fields["lists"]
    said = {
        "id": fields["name"],
        "version": fields["version"],
        "firmware": known_firmwares.get(fields["os_id"], "one this repository has no profile for (0x%08x)" % fields["os_id"]),
        "places": fields["entries"],
        "needs": sorted(name for name, _interface, needed in lists["works"] if needed),
        "level": level_of([(name, needed) for name, needed in lists["imports"]], True),
        "offers_level": level_of([(name, True) for name, _offset in lists["exports"]], False),
        "folders": [{"name": name, "place": place} for name, place, _shown in pack_build.folders_in(data)],
    }
    for key, value in said.items():
        stated = sorted(entry.get(key, [])) if key == "needs" else entry.get(key)
        if stated != value:
            faults.append("the file's %s is %s, the entry says %s" % (key.replace("_", " "), json.dumps(value), json.dumps(stated)))
    if pack_build.card_in(data) is not None:
        faults += ["its card: %s" % fault for fault in pack_build.card_faults(data)]
        if base is not None:
            faults += base_faults(entry, data, base)
    try:
        pack_build.release_in(data)
    except ValueError as problem:
        faults.append("its release: %s" % problem)
    return faults


def long_name(data):
    """A fault of a followed file's lists that would cost their reading dear, as a text, or None: a name longer than
    NAME_MOST characters. Read before tools/pack_build.py reads the file, which takes every name again for each mention
    of it, so that one long name mentioned many times cannot fill the memory. Anything else out of order is left to
    pack_build to say."""
    if len(data) < pack_build.HEADER3_BYTES or data[:8] != pack_build.MAGIC:
        return None
    header = pack_build.HEADER3.unpack_from(data)
    body_bytes, lists_bytes = header[5], header[12]
    lists = data[pack_build.HEADER3_BYTES + body_bytes:pack_build.HEADER3_BYTES + body_bytes + lists_bytes]
    if len(lists) < 24 or len(lists) != lists_bytes:
        return None
    works, exports, imports, places, marks, strings_n = struct.unpack_from("<6I", lists)
    if 24 + 12 * works + 8 * exports + 8 * imports + 12 * places + 8 * marks + strings_n != len(lists):
        return None
    strings = lists[len(lists) - strings_n:]
    mentions = [24 + 12 * n for n in range(works)] + [24 + 12 * works + 8 * n for n in range(exports + imports)]
    for offset in {struct.unpack_from("<I", lists, at)[0] for at in mentions}:
        if offset < len(strings) and strings.find(b"\0", offset, offset + NAME_MOST + 1) < 0 and len(strings) - offset > NAME_MOST:
            return "a name in its lists is longer than %d characters" % NAME_MOST
    return None


def followed_faults(entry, data, known_firmwares, offered, base=None):
    """What is not in order with the file at the head of a followed entry's branch, as texts: what the install page
    holds in the owner's browser before it offers the file (docs/catalog.md, "Following a branch"), and its card, its
    release and with `base` the base's version as `file_faults` holds a pinned file's. `offered` is the level the
    listed base offers (`base_offers`), `base` its version (`base_version`). Nobody here built the file: what cannot be
    read in it is a fault, never a stop."""
    if len(data) > LARGEST:                                     # 1: the size
        return ["the file has %d bytes, and a pack has at most %d" % (len(data), LARGEST)]
    long = long_name(data)                                      # 2, first: no name that would cost its reading dear
    if long:
        return ["the file is no pack the loader takes: %s" % long]
    try:                                                        # 2: a pack of format 3, its sizes, its checksum, its lists, its card's place
        fields, _body = pack_build.parse(data)
    except UNREADABLE as problem:
        return ["the file is no pack the loader takes: %s" % (str(problem) or type(problem).__name__)]
    if fields["format"] != pack_build.FORMAT_LINKED:
        return ["the file is a pack of format %d, which the loader no longer takes" % fields["format"]]
    faults = []
    lists = fields["lists"]
    if fields["name"] != entry["id"]:                           # 3: its name
        faults.append("the file's name is %s, the entry's id is %s" % (json.dumps(fields["name"]), json.dumps(entry["id"])))
    firmware = known_firmwares.get(fields["os_id"], "one this repository has no profile for (0x%08x)" % fields["os_id"])
    if firmware != entry["firmware"]:                           # 4: its firmware
        faults.append("the file's firmware is %s, the entry says %s" % (firmware, entry["firmware"]))
    if fields["version"] < entry["version"]:                    # 5: its version, the entry's at the least
        faults.append("the file's version is %d, the listing takes %d or higher" % (fields["version"], entry["version"]))
    for needed in sorted({name for name, _interface, flag in lists["works"] if flag} - set(entry["needs"])):   # 6: what it needs
        faults.append("the file needs %s, which the entry's needs do not name" % json.dumps(needed))
    level = level_of([(name, flag) for name, flag in lists["imports"]], True)
    if level is not None and level > offered:                   # 7: the level of the base it needs
        faults.append("the file needs level %d of the base, and the listing's base offers %d" % (level, offered))
    allowed = {(folder["name"], folder["place"]) for folder in entry["folders"]}
    try:                                                        # 8: the folders its card asks for, their list inside the body
        card = pack_build.card_in(data)
        at = card["folders"] - fields["load_address"] if card and card.get("folder_count") else 0
        if card and card.get("folder_count") and (at < 0 or at + pack_build.FOLDER.size * card["folder_count"] > fields["body_bytes"]):
            raise ValueError("its list does not lie in its body")
        asked = [(name, place) for name, place, _shown in pack_build.folders_in(data)]
    except UNREADABLE as problem:
        asked = []
        faults.append("the file's card lists folders that cannot be read: %s" % (str(problem) or type(problem).__name__))
    for name, place in asked:
        if (name, place) not in allowed:
            faults.append("the file's card asks for the folder %s, which the entry's folders do not name"
                          % json.dumps({"name": name, "place": place}))
    try:
        if pack_build.card_in(data) is not None:
            faults += ["its card: %s" % fault for fault in pack_build.card_faults(data)]
            if base is not None:
                faults += base_faults(entry, data, base)
    except UNREADABLE as problem:
        faults.append("its card cannot be read: %s" % (str(problem) or type(problem).__name__))
    try:
        pack_build.release_in(data)
    except ValueError as problem:
        faults.append("its release: %s" % problem)
    return faults


def moved_firmware(new, old, known_firmwares):
    """Whether an entry's new file is the same pack built for another firmware: its firmware changed to one this
    repository has a profile for, and its version did not go down."""
    return (known_firmwares is not None and isinstance(new.get("firmware"), str) and new.get("firmware") != old.get("firmware")
            and new["firmware"] in known_firmwares.values()
            and is_int(new.get("version")) and is_int(old.get("version")) and new["version"] >= old["version"])


def before_faults(listing, before, known_firmwares=None):
    """What a change to the listing may not do, held against the listing before it: {id: faults}. A file may change
    at the same version only with its firmware (`moved_firmware`); `known_firmwares` are those this repository has a
    profile for, and without them no such move is let through. No version goes down, a followed entry's lowest
    included; a pinned entry may come to follow a branch, and back, on that rule alone."""
    out = {}
    now = {entry.get("id", "").casefold(): entry for entry in listing["packs"] if isinstance(entry, dict) and isinstance(entry.get("id"), str)}
    for old in before.get("packs", []):
        if not isinstance(old, dict) or not isinstance(old.get("id"), str):
            continue
        new = now.get(old["id"].casefold())
        if new is None:
            out[old["id"]] = ["it was taken out: an entry stays for good, retired when its pack is no longer offered"]
            continue
        faults = []
        if new["id"] != old["id"]:
            faults.append("its id was %r: an id never changes" % old["id"])
        if new.get("status") in STATUSES and old.get("status") in STATUSES and STATUSES.index(new["status"]) < STATUSES.index(old["status"]):
            faults.append("its status moved back, from %s to %s" % (old["status"], new["status"]))
        if (old.get("sha256") and new.get("sha256") and new["sha256"] != old["sha256"]
                and not (is_int(new.get("version")) and is_int(old.get("version")) and new["version"] > old["version"])
                and not moved_firmware(new, old, known_firmwares)):
            faults.append("its file changed and its version did not grow past %s (the same version is kept only by the "
                          "same pack built for another firmware this repository has a profile for)" % old.get("version"))
        elif is_int(new.get("version")) and is_int(old.get("version")) and new["version"] < old["version"]:
            faults.append("its version went down, from %d to %d: a version never goes down, and a followed entry's is the "
                          "lowest the install page takes from its branch" % (old["version"], new["version"]))
        if faults:
            out[new["id"]] = faults
    return out


def changed(entry, before):
    """Whether an entry is new, or names another file or another branch than it did before. A followed entry is also
    held to its file again when what the file is held to changed: its version, its firmware, its needs or its folders."""
    keys = LOCATION + (FOLLOW, "sha256") + (FOLLOWED[3:] if FOLLOW in entry else ())
    for old in before.get("packs", []):
        if isinstance(old, dict) and old.get("id") == entry["id"]:
            return any(old.get(key) != entry.get(key) for key in keys + (PICTURE,))
    return True


def raw_link(entry, path=None):
    """Where the install page downloads an entry's file: its repository at its commit, or at the head of the branch
    it follows; or another path there (its picture's)."""
    owner, name = REPOSITORY_FORM.fullmatch(entry["repository"]).groups()
    at = BRANCH_HEAD + entry[FOLLOW] if FOLLOW in entry else entry["commit"]
    return RAW % (owner, name, at, urllib.parse.quote(path or entry["path"]))


def fetch(url):
    """The file at a link, up to the largest a pack may be."""
    request = urllib.request.Request(url, headers={"User-Agent": "open-field-catalog-check"})
    with urllib.request.urlopen(request, timeout=60) as answer:
        data = answer.read(LARGEST + 1)
    if len(data) > LARGEST:
        raise ValueError("it is larger than %d bytes" % LARGEST)
    return data


def repository_file(path, root=None):
    """A bundled entry's file, from this repository: its bytes, or None."""
    full = os.path.join(root or ROOT, *path.split("/"))
    if not os.path.isfile(full):
        return None
    with open(full, "rb") as f:
        return f.read()


def local_file(entry, folders):
    name = os.path.basename(entry.get("path") or entry["id"] + ".pack")
    for folder in folders:
        path = os.path.join(folder, name)
        if os.path.isfile(path):
            with open(path, "rb") as f:
                return f.read()
    return None


def check(listing, known_firmwares, before=None, folders=(), download=False, get=None):
    """Every entry with its faults: (lines to print, whether all is in order)."""
    get = get or fetch
    whole, each = listing_faults(listing, known_firmwares)
    lines = ["REFUSED: the listing: %s" % fault for fault in whole]
    packs = listing.get("packs", []) if isinstance(listing, dict) and isinstance(listing.get("packs"), list) else []
    earlier = before_faults(listing, before, known_firmwares) if before is not None else {}
    for gone, faults in earlier.items():                        # an entry taken out has no place among today's
        if not any(isinstance(e, dict) and e.get("id") == gone for e in packs):
            lines += ["REFUSED: %s: %s" % (gone, fault) for fault in faults]
    moved = before is not None and isinstance(listing, dict) and listing.get("ours", []) != before.get("ours", [])
    if moved:                                                   # the maintainer changes it on main, never a pull request
        lines.append("REFUSED: the listing: its ours changed: the repositories of open-field's own packs are the maintainer's change")
    refused = bool(whole) or bool(earlier) or moved
    with_a_file = [number for number, entry in enumerate(packs)
                   if number not in each and (entry.get("sha256") or (entry.get("status") == "listed" and FOLLOW in entry))
                   and entry["status"] != "retired"]
    found = {number: local_file(packs[number], folders) for number in with_a_file} if folders else {}
    at_hand = {packs[number]["author"] for number, data in found.items() if data is not None}   # whose files the folders hold
    looked = False
    for number, entry in enumerate(packs):
        label = entry.get("id", "entry %d" % (number + 1)) if isinstance(entry, dict) else "entry %d" % (number + 1)
        faults = list(each.get(number, [])) + (earlier.get(label, []) if isinstance(label, str) else [])
        held = ""
        followed = number not in each and entry.get("status") == "listed" and FOLLOW in entry
        if number in with_a_file:
            data, where = None, None
            if entry["status"] == "bundled":
                data, where = repository_file(entry["path"]), "this repository"
                if data is None:
                    faults.append("its file is not in this repository at %s" % entry["path"])
            elif download and entry["status"] == "listed" and (before is None or changed(entry, before)):
                where = raw_link(entry)
                try:
                    data = get(where)
                except Exception as problem:                    # a link that does not answer is the entry's fault
                    faults.append("its file could not be downloaded from %s: %s" % (where, problem))
            elif folders:
                data, where, looked = found[number], "the folders given", True
                if data is None and entry["author"] in at_hand:      # another of its author's files is there: this one is missing
                    faults.append("its file is not in %s" % ", ".join(folders))
                elif data is None:                              # an author with no file at hand: held to its form only
                    held = ", its file not held: no file of %s's in the folders given" % entry["author"]
            base = base_version(packs, each, entry["firmware"])
            if data is not None and followed:
                faults += followed_faults(entry, data, known_firmwares, base_offers(packs, each, entry["firmware"]), base)
                held = ", its file held"
            elif data is not None:
                faults += file_faults(entry, data, known_firmwares, base)
                held = ", its file held"
            if PICTURE in entry:
                picture, shown = entry[PICTURE], None
                if entry["status"] == "bundled":
                    shown = repository_file(picture["path"])
                    if shown is None:
                        faults.append("its picture is not in this repository at %s" % picture["path"])
                elif where is not None and where.startswith("https://"):   # downloaded: the picture too, from the same repository and commit
                    link = raw_link(entry, picture["path"])
                    try:
                        shown = get(link)
                    except Exception as problem:
                        faults.append("its picture could not be downloaded from %s: %s" % (link, problem))
                if shown is not None:
                    faults += picture_file_faults(picture, shown)
                    held += ", its picture held"
        if followed:
            held = ", follows %s%s" % (entry[FOLLOW], held)
        if faults:
            refused = True
            lines += ["REFUSED: %s: %s" % (label, fault) for fault in faults]
        else:
            lines.append("ok: %s (%s%s)" % (label, entry["status"], held))
    if looked and not at_hand:                                  # a file named otherwise than its entry would pass unread
        refused = True
        lines.append("REFUSED: the folders given (%s) hold no entry's file: a file is looked for by the last part of its entry's"
                     " path, or as <id>.pack" % ", ".join(folders))
    lines.append("%d entries, %s" % (len(packs), "REFUSED" if refused else "all in order"))
    return lines, not refused


def read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def main(argv=None):
    parser = argparse.ArgumentParser(description="Check the listing of packs, catalog/packs.json.")
    parser.add_argument("--catalog", default=CATALOG, help="the listing (default: catalog/packs.json)")
    parser.add_argument("--before", help="the listing before the change, to hold the change against")
    parser.add_argument("--files", action="append", default=[], help="a folder with pack files to hold the entries to (more than one may be given)")
    parser.add_argument("--fetch", action="store_true", help="download each listed entry's file, at its commit or from the branch it follows, and hold the entry to it")
    args = parser.parse_args(argv)
    try:
        listing = read_json(args.catalog)
        before = read_json(args.before) if args.before else None
    except (OSError, ValueError) as problem:
        print("REFUSED: the listing cannot be read: %s" % problem)
        return 1
    if before is not None and not (isinstance(before, dict) and isinstance(before.get("packs"), list)):
        print("REFUSED: the listing before the change is not a listing")
        return 1
    lines, in_order = check(listing, firmwares(), before=before, folders=args.files, download=args.fetch)
    for line in lines:
        print(line)
    return 0 if in_order else 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Make and check a pack file: the module's code as a file the base reads at start-up (Build J, since J3).

Format 1 (J3, J3b) is the whole module as a twin of the one in the image. Format 2 (since J4) is a pack the base
loads and runs: linked for its own address inside the reservation, with the addresses of its table of stock
changes in the header's last two words (`--format 2`, which takes everything from the pack's ELF). Format 3 (since
J5c) is a pack the base may load at another address than it was linked for, beside other packs: behind the body it
carries lists of the places that hold addresses, of the names it offers and of the names it wants (`build3`).

  python -I tools/pack_build.py build --module build/build_j3.bin --elf build/build_j3.elf --profile profiles/op1f-1.7.0.json --name build_j3 --out candidates/pack-j3/01.pack
  python -I tools/pack_build.py build3 --recipe packs/all.json --profile profiles/op1f-1.7.0.json --out candidates/pack-j5c/all.pack
  python -I tools/pack_build.py build3 --recipe packs/kit.json --recipe packs/rest.json --profile profiles/op1f-1.7.0.json --out-dir candidates/pack-j5d
  python -I tools/pack_build.py placed --recipe packs/kit.json --recipe packs/rest.json --folder candidates/pack-j5d
  python -I tools/pack_build.py pictures --recipe packs/j5e/kit.json --out module/build_j5e_kit_pictures.s
  python -I tools/pack_build.py placed --recipe packs/j5e/kit.json --recipe packs/j5e/rest.json --folder candidates/pack-j5e --name build_j5e
  python -I tools/pack_build.py placed --recipe packs/l5/lfo.json ... --folder candidates/pack-l5 --name build_l5 --inside-recipe packs/l5/kit.json --inside-pack candidates/pack-l4/kit.pack
  python -I tools/pack_build.py card --recipe packs/m1/wink.json --out module/build_m1_wink_card.s
  python -I tools/pack_build.py release --recipe packs/m1/base.json --out module/build_m1_base_release.s
  python -I tools/pack_build.py check --pack candidates/pack-j3/01.pack [--module build/build_j3.bin]

The file is a 64-byte header and the module binary as `tools/module_build.py` wrote it. The header, little-endian:

  0   8   the magic "OP1FPACK"
  8   4   the format, 1
  12  4   the header's size, 64
  16  4   the OS the pack is for: the first four bytes of the stock OS image's SHA-256
  20  4   the address the body was linked for (the module's first byte)
  24  4   the body's size
  28  4   how many of its bytes are code and constant data (up to the start of `.data`): what the reader compares with
          memory, the initialised data after them being changed by the running module
  32  4   the size of the zeroed data after the body
  36  4   the body's CRC-32 (zlib's)
  40  16  a name, zero padded
  56  8   zero (format 1); format 2: the address of the pack's table of stock changes and of its end

Format 3 has a header of 96 bytes, format 2's 64 with the same fields in the same places and eight words more:

  36  4   the CRC-32 of the body and the lists behind it
  64  4   the size of the lists
  68  4   the size of what follows the lists in the file and is not loaded (nothing yet)
  72  4   the pack's version, a number that only grows: of two files with the same pack the higher runs
  76  4   its interface number: two packs that work together must agree on each other's
  80  4   the largest alignment inside the pack, a power of two, at most 32: a pack is moved by multiples of 32
  84  4   flags: 1 the pack is not named on the start screen; 2 the base logs a sum of its memory as linked
  88  4   the offset in the body of 48 bytes the base writes the pack file's path into, 0 for none
  92  4   zero
          since M1: the offset in the body of the pack's card, 0 for none; a pack without a card keeps the zero

The lists, behind the body (analysis/pack_loader_study.md, section 18): six counts (works, exports, imports,
places, marks, the strings' bytes), then

  works    name, interface number, 1 when needed      a pack this one works with
  exports  name, offset from the pack's first byte    a name this pack offers: a function, or data (its zeroed data too)
  imports  name, 1 when needed                        a name it wants from another pack
  places   offset in the body; kind, and in the high half the import's number plus one, 0 for an address of the
           pack's own; the address as linked (its own) or what to add to the import's address
           kinds: 0 the high half and 1 the low half of an address loaded into a register (16 bits at the offset),
           2 a data word, 3 a direct call or jump (the 24-bit distance of the instruction at the offset minus 2)
  marks    an entry of the table of stock changes, a number in the works list: the entry is made only when that
           pack does not run
  strings  the names, each ended by a zero byte

A recipe (`packs/<name>.json`) names the pack, its version and interface number, the address it is linked for, its
sources, the packs it works with (`works_with`: needed or not, and their interface number), the names it offers
(`exports`) and the packs it serves (`serves`: what their objects want of it is offered too, so that packs cut from
one tree need no list written by hand). `placed` links each pack of a set for the place the base gives it, with
the others' names at their real addresses: what stands in the unit's memory, and the listing to read its
addresses against. From image L5 on the image holds one pack itself (tools/base_image.py): `--inside-recipe` and
`--inside-pack` name it, and it then stands first, at the room's first byte, in front of the folder's files.

Since J5e a recipe may name `pictures` (a name and a file each). They are put behind the lists, the part of the
file the base does not load, and the pack's body holds their directory: the magic "ART1", the count, the bytes of
the pictures' part, the pack's version and name, 48 bytes for the file's path, which the base writes at the start
(the header's place for it), then for each picture its name in 24 bytes, its place in the pictures' part, its
bytes and its CRC-32. `pictures` writes the directory's source from the recipe; `build3` holds what was linked
against the recipe's pictures. The reader is the picture loader, module/build_j5e_art.cpp.

Since M1 (module/build_m1_card.h; analysis/kit_interface_study.md) the piece in the firmware file that reads the
packs is called the loader, and the base is the shared pack, the file base.pack: where the text above says "the
base" it means the loader. Four things came with that step, and a pack that uses none of them comes out byte for
byte as before.

  A card. A pack may carry one: constant data in its body that says what the pack wants of the base pack. Its
  place is the header's last word. A recipe names it as `card`: {"level": 1, "frame": a function's symbol or null,
  "pick": the same, "pictures": true or false} (`pictures` left out: whether the recipe has pictures). `card`
  writes its source from the recipe: a word that names the mark of the card's level (op1f_level_1), so that the
  pack needs that name, and behind it the card under the name op1f_card_<pack>: its mark, its 44 bytes, the level,
  the pack's version and name, the frame function, the directory of pictures and the function that chooses among
  them, 0 for each the pack has not. `build3` holds what was linked against the recipe and writes the place.
  A release. A base pack's recipe may name one as `release`: one to eight characters of 0-9, A-Z, a-z, '.', '_'
  and '-'. `release` writes the source of its sixteen bytes under the name op1f_release; `build3` holds them
  against the recipe and offers the name, by which the loader finds the text for the start screen.
  No table. A pack whose sources hold no table of stock changes (no op1f_boot_table) changes no place of the
  firmware: its header names the pack's first byte as the table and as the table's end.
  The loader's names. `placed` knows the one name the loader answers for itself, op1f_loader_1, its first byte,
  and that the loader keeps every name that begins op1f_loader_ to itself: a set in which a pack offers such a
  name is refused, and a pack that needs another of them than op1f_loader_1 finds nobody.

`card` and `release` write over a file only when it is a source they wrote themselves.

Since M2 (module/build_m2_card.h; analysis/kit_interface_study.md, section 9) a card may be of level 2: a page of
the pack's own, with keys and knobs. Its recipe names one function more, `input`, the function a key or a knob is
offered to: {"level": 2, "frame": ..., "pick": ..., "pictures": ..., "input": a function's symbol or null}. The card
is then 48 bytes, the input function's address behind level 1's three, and the word in front of it names the mark
op1f_level_2. `build3` holds the fourth address as it holds the three; `check` prints it. A card of level 1 and every
pack built before come out as they did.

Since M3 (module/build_m3_card.h; analysis/kit_interface_study.md, section 10) a card may be of level 3: folders of the
pack's own on the unit's disk, files read and written there through the base, each folder shown to the computer in
the disk mode if the pack asks, and text on its page. Its recipe may list folders, none to eight: {"level": 3, ...,
"folders": [{"name": ..., "place": ..., "shown": true or false}, ...]}. A folder's name is what the computer shows; its
place is where it lies on the disk, inside user/pf/; `shown` says whether the computer sees it. The tool fills what a
folder leaves out: the name is the pack's own where the pack lists one folder and does not name it; the place is the
name; a folder is shown. A name and a place are one to fifteen characters of A-Z, a-z, 0-9, space, '_', '-' and '.',
not beginning with a space, a dot or a dash and not ending with a space or a dot; a shown folder may not take a name
of the firmware's (synth, drum, tapes, mixdown), the loader's (packs) or the base's (fx, lfo); and two folders of a
pack may not share a name or a place, capitals counted as small letters. `card` writes the card of 56 bytes, the
list's address and count behind level 2's four addresses, and the list as constant data under the card's name and
_folders, 36 bytes a folder; the word in front of the card names op1f_level_3. `build3` holds the list and its address
as it holds the card's other fields; `check` prints the folders. Cards of levels 1 and 2 and every pack built before
come out as they did.

Since R1 a card may be of level 4: what a page needs (pages that lie one over the other, text set with an alignment,
painting on the frame's picture, a file removed, a key watched). Its recipe may carry `asks`, the list of
what the pack asks of the base: {"level": 4, ..., "asks": []}. Level 4 defines no kind of ask yet, so any ask in a
recipe is refused. `card` writes the card of 64 bytes, the list of asks' address and count behind level 3's two words
(0 and 0: no ask), and the word in front of the card names the mark op1f_level_4. `build3` holds the two words as it
holds the card's other fields; `check` prints the asks, `asks: none` for a card of level 4 without any. A card of
level 1, 2 or 3 and every earlier pack file are written as they were, to the last byte.

The readers are module/build_j_pack.cpp (format 1) and the bases (module/build_j5c_base.cpp for formats 2 and 3).
A pack file holds the module's own code and whatever the module carries. Whether one holds anything of the firmware
is what tools/publish_check.py says: a pack of format 3 whose table holds checksums and whose pictures are
instructions holds none.
Refuses to overwrite its output.
"""

import argparse
import collections
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

MAGIC = b"OP1FPACK"
FORMAT = 1
FORMAT_LOADED = 2
FORMAT_LINKED = 3
ENTRY_BYTES = 20
HEADER_BYTES = 64
HEADER = struct.Struct("<8s8I16s2I")
assert HEADER.size == HEADER_BYTES
HEADER3_BYTES = 96
HEADER3 = struct.Struct("<8s8I16s2I8I")
assert HEADER3.size == HEADER3_BYTES

KIND_HIGH, KIND_LOW, KIND_WORD, KIND_CALL = 0, 1, 2, 3
PLACE_KINDS = ("high half", "low half", "data word", "direct call")
FLAG_UNNAMED, FLAG_SUM = 1, 2
NEEDED = 1
PATH_BYTES = 48
PACK_ALIGN = 32
LIST_COUNTS = 6

# the Blackfin's relocation types (binutils, include/elf/bfin.h)
R_LUIMM16, R_HUIMM16, R_PCREL24, R_PCREL24_JUMP_L, R_BYTE4_DATA = 6, 7, 0x0a, 0x0d, 0x12
SHN_UNDEF, SHN_ABS = 0, 0xfff1
STB_WEAK = 2
SHT_SYMTAB, SHT_RELA = 2, 4
# a pack's directory of pictures, data in its body (module/build_j5e_art.cpp): a head, then one entry a picture
ART_MAGIC = 0x31545241                          # "ART1"
ART_NAME_BYTES = 24
ART_HEAD = struct.Struct("<4I16s48s")           # the magic, the count, the pictures' bytes, the pack's version, its name, the file's path
ART_ENTRY = struct.Struct("<24s3I")             # the picture's name, its place in the pictures' part, its bytes, its CRC-32
ART_PATH_AT = 32                                # where the path stands in the head: the place the base writes it at
PACK_FOLDER = "user/pk/"                        # where the pack files lie on the unit: a pack's path is this and its file's name
PACK_SCRIPT = os.path.join(ROOT, "linker", "op1f-1.7.0-pack.ld")
SCRIPT_ORIGIN = "ORIGIN = 0x8d2f36cc, LENGTH = 0x1f0000"
ROOM_START, ROOM_BYTES = 0x8d2f36cc, 0x1f0000
# since M1 (module/build_m1_card.h): a pack's card, constant data in its body, whose place is the header's last word
CARD_MARK = 0x44524143                          # "CARD"
CARD = struct.Struct("<4I16s3I")                # the mark, the card's bytes, the highest level it uses, the pack's version, its name; then
                                                # level 1's three addresses: the frame function, the directory of pictures, the choice among them
CARD_BYTES = CARD.size                          # 44: a card of level 1
CARD_LEVELS = (1,)                              # the levels this tool writes a card of
# since M2 (module/build_m2_card.h): a card of level 2 has a fourth address behind level 1's three, its input function
CARD_2 = struct.Struct("<4I16s4I")
CARD_2_BYTES = CARD_2.size                      # 48: a card of level 2
CARD_LEVELS = CARD_LEVELS + (2,)
# since M3 (module/build_m3_card.h): a card of level 3 has the address of its list of folders and their count behind
# level 2's four addresses; a folder of the list is its name, its place and whether it is shown
CARD_3 = struct.Struct("<4I16s4I2I")
CARD_3_BYTES = CARD_3.size                      # 56: a card of level 3
CARD_LEVELS = CARD_LEVELS + (3,)
FOLDER = struct.Struct("<16s16sI")              # 36 bytes a folder
FOLDERS_MOST = 8
FOLDER_FORM = re.compile(r"[A-Za-z0-9_](?:[A-Za-z0-9 _.-]{0,13}[A-Za-z0-9_-])?")    # one to fifteen characters, as the base takes them
FOLDER_NAMES_TAKEN = {"synth": "the firmware's", "drum": "the firmware's", "tapes": "the firmware's", "mixdown": "the firmware's",
                      "packs": "the loader's", "fx": "the base's", "lfo": "the base's"}
# since R1: a card of level 4 has the address of its list of asks and their count behind
# level 3's two words; an ask is its kind, its bytes and the address of what it points at. Level 4 defines no kind.
CARD_4 = struct.Struct("<4I16s4I2I2I")
CARD_4_BYTES = CARD_4.size                      # 64: a card of level 4
CARD_LEVELS = CARD_LEVELS + (4,)
ASK = struct.Struct("<3I")                      # 12 bytes an ask
ASKS_MOST, ASK_BYTES_MOST = 16, 4096
ASK_KINDS = {}                                  # the kinds of ask this tool knows, by number: none at level 4
SYMBOL = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")  # a name as a source defines it
RELEASE_NAME, RELEASE_BYTES = "op1f_release", 16                # the text a base pack carries for the start screen, zeros behind it
RELEASE_FORM = re.compile(r"[0-9A-Za-z._-]{1,8}")               # what the tool takes for one
LOADER_NAME, LOADER_ADDRESS = "op1f_loader_1", 0x8d2e36cc       # the one name the loader answers for itself: its own first byte
LOADER_FAMILY = "op1f_loader_"                                  # every name that begins so is the loader's to keep: a pack that offers one is left out


def os_id(stock_sha256):
    """The first four bytes of the stock image's SHA-256 as a little-endian word."""
    return struct.unpack("<I", bytes.fromhex(stock_sha256)[:4])[0]


ROOM_START_1_7_0 = ROOM_START                   # the room as above, kept when use_profile() moves it
VERSION_1_7_0 = "1.7.0"                         # the firmware the room above is of


def moved_room():
    """Whether use_profile() moved the room away from OS 1.7.0's (since the port to 1.7.3)."""
    return ROOM_START != ROOM_START_1_7_0


def sources_for(recipes, profile):
    """For a room other than 1.7.0's, in a tree that keeps the sources of more than one firmware (guards_sources()):
    every source of the recipes must say on its first line that it was made for the profile's firmware, so that a
    source of one firmware is never built for another. This tree keeps one firmware's sources, and the check is not
    made here. Answers the problems."""
    version = profile.get("firmware_version", "").split("+")[0]
    problems = []
    for recipe in recipes:
        for source in recipe["sources"]:
            with open(source, encoding="utf-8", errors="replace") as f:
                first = f.readline()
            translated = "Generated by tools/os_translate.py from " in first and (" to %s," % version) in first
            table = ("Generated by tools/boot_table.py from " in first or "Generated by tools/port_build.py " in first) and \
                ("%s.json" % profile.get("id")) in first
            if not version or not (translated or table):
                problems.append("'%s': %s is not a source made for %s" % (recipe["name"], source, profile.get("id")))
    return problems


def guards_sources():
    """Whether sources_for() holds the sources: only in a tree that keeps the sources of more than one firmware, which
    has a `ports` folder beside `tools`. A tree with one firmware's sources has none, and its sources are built as they
    are."""
    return os.path.isdir(os.path.join(ROOT, "ports"))


def use_profile(profile):
    """Since the port to 1.7.3 (2026-11-19): the room of the profile's own reservation. The loader stands at the
    reservation's first byte (`module_base`) and the room behind its 64 KiB, to the reservation's end; the pack's
    linker script is linker/<the profile's id>-pack.ld, or where there is none linker/op1f-<its firmware_version>-pack.ld,
    which must name that room. For 1.7.0's profile every value is the
    one above, so nothing changes (whatever the profile is called); for another reservation ROOM_START, ROOM_BYTES,
    LOADER_ADDRESS, PACK_SCRIPT and SCRIPT_ORIGIN are set for the rest of the run."""
    global ROOM_START, ROOM_BYTES, LOADER_ADDRESS, PACK_SCRIPT, SCRIPT_ORIGIN
    if "module_base" not in profile or "reserved_bytes" not in profile:
        # a profile that names no reservation (the public tree's example of trampolines): 1.7.0's room, said aloud, and
        # only for a profile of 1.7.0's stock image
        if profile.get("firmware_version", "").split("+")[0] != VERSION_1_7_0:
            raise ValueError("The profile %s names no reservation and is not of OS 1.7.0: no room to link for" % profile.get("id"))
        sys.stderr.write("pack_build: the profile %s names no reservation: the room of OS 1.7.0's, 0x%08x (%s)\n"
                         % (profile.get("id"), ROOM_START, os.path.relpath(PACK_SCRIPT, ROOT).replace("\\", "/")))
        return
    base, reserved = int(profile["module_base"], 16), int(profile["reserved_bytes"])
    room, size = base + 0x10000, reserved - 0x10000
    if (room, size, base) == (ROOM_START, ROOM_BYTES, LOADER_ADDRESS):
        return
    script = os.path.join(ROOT, "linker", "%s-pack.ld" % profile["id"])
    origin = "ORIGIN = 0x{:08x}, LENGTH = 0x{:x}".format(room, size)          # the line link3 replaces
    if not os.path.isfile(script):
        # a profile with an id of its own (an example's) takes the script of its firmware, named by its firmware_version
        version = profile.get("firmware_version", "").split("+")[0]
        named = os.path.join(ROOT, "linker", "op1f-%s-pack.ld" % version) if version else None
        if not named or not os.path.isfile(named):
            raise ValueError("No linker script for the profile %s: neither %s nor %s" % (profile["id"], script, named or "a script of its firmware (it names no firmware_version)"))
        script = named
    with open(script, encoding="utf-8") as f:
        if origin not in f.read():
            raise ValueError("The linker script %s does not name the profile's room (%s)" % (script, origin))
    ROOM_START, ROOM_BYTES, LOADER_ADDRESS, PACK_SCRIPT, SCRIPT_ORIGIN = room, size, base, script, origin


def make(body, load_address, compare_bytes, bss_bytes, name, target_os, table=None):
    """The pack file's bytes. `table` is (address, end address) of the pack's table of stock changes: format 2."""
    text = name.encode("ascii")
    if not 0 < len(text) <= 15 or any(not 0x20 <= c < 0x7f for c in text):
        raise ValueError("A pack's name is one to fifteen printable ASCII characters")
    if not body or not 0 < compare_bytes <= len(body) or load_address % 4:
        raise ValueError("A pack needs a body, a compare size inside it and an aligned load address")
    start, end = table if table else (0, 0)
    if table and not (load_address <= start <= end <= load_address + compare_bytes and start % 4 == 0 and (end - start) % ENTRY_BYTES == 0):
        raise ValueError("A pack's table lies in its code and constant data, word aligned, a whole number of entries long")
    header = HEADER.pack(MAGIC, FORMAT_LOADED if table else FORMAT, HEADER_BYTES, target_os, load_address, len(body), compare_bytes,
                         bss_bytes, zlib.crc32(body) & 0xffffffff, text, start, end)
    return header + body


def card_stands(body, place):
    """Does a card stand at this offset of a pack's body, as far as the loader looks (since M1): on a word, a card's
    mark, and a size of whole words, the mark and the size themselves at the least, that ends inside the body?"""
    if not isinstance(place, int) or place <= 0 or place % 4 or place + 8 > len(body):
        return False
    mark, size = struct.unpack_from("<2I", body, place)
    return mark == CARD_MARK and size % 4 == 0 and 8 <= size <= len(body) - place


def make3(body, link_address, compare_bytes, zeroed_bytes, name, target_os, table, version=1, interface=1, works=(), exports=(),
          imports=(), places=(), marks=(), flags=0, path_place=0, align=4, extra=b"", card_place=0, no_table=False):
    """A pack of the new kind. works: (name, interface, needed); exports: (name, offset); imports: (name, needed);
    places: (offset, kind, import's number or None, value); marks: (table entry, number in works)."""
    # Since M1. `card_place`: the offset in the body of the pack's card, 0 for none: the header's last word. With
    # `no_table` the pack changes no place of the firmware, and `table` names one address as the table and as its end.
    text = name.encode("ascii")
    if not 0 < len(text) <= 15 or any(not 0x20 <= c < 0x7f for c in text):
        raise ValueError("A pack's name is one to fifteen printable ASCII characters")
    if not body or len(body) % 4 or not 0 < compare_bytes <= len(body) or link_address % 4:
        raise ValueError("A pack needs a body of whole words, a compare size inside it and an aligned link address")
    start, end = table
    if no_table and not (link_address <= start == end <= link_address + compare_bytes and start % 4 == 0):
        raise ValueError("A pack with no table of stock changes names one address as the table and as its end, a word in its code and constant data")
    if not no_table and not (link_address <= start < end <= link_address + compare_bytes and start % 4 == 0 and (end - start) % ENTRY_BYTES == 0):
        raise ValueError("A pack's table lies in its code and constant data, word aligned, a whole number of entries long")
    if align not in (1, 2, 4, 8, 16, 32):
        raise ValueError("A pack's largest alignment is a power of two up to 32: it is moved by multiples of 32")
    if path_place and not (path_place % 4 == 0 and path_place + PATH_BYTES <= len(body)):
        raise ValueError("The place for the file's path lies in the body")
    if card_place and not card_stands(body, card_place):
        raise ValueError("The place for the card lies in the body, on a word, and a card's mark and size stand there")
    entries = (end - start) // ENTRY_BYTES
    strings = bytearray()
    index = {}

    def string(value):
        if value not in index:
            index[value] = len(strings)
            strings.extend(value.encode("ascii") + b"\0")
        return index[value]

    out = bytearray()
    for other, number, needed in works:
        out += struct.pack("<3I", string(other), number, NEEDED if needed else 0)
    for offered, offset in exports:
        if not 0 <= offset < len(body) + zeroed_bytes:      # a function, or data, the zeroed data after the body included
            raise ValueError("The export %s lies outside the pack" % offered)
        out += struct.pack("<2I", string(offered), offset)
    for wanted, needed in imports:
        out += struct.pack("<2I", string(wanted), NEEDED if needed else 0)
    for offset, kind, wanted, value in places:
        size = 4 if kind in (KIND_WORD,) else 2
        low = 2 if kind == KIND_CALL else 0
        if kind not in (KIND_HIGH, KIND_LOW, KIND_WORD, KIND_CALL) or offset % 2 or not low <= offset <= len(body) - size:
            raise ValueError("A place of kind %r at offset %d does not lie in the body" % (kind, offset))
        if wanted is None and kind == KIND_CALL:
            raise ValueError("A direct call inside the pack needs no place")
        if wanted is not None and not 0 <= wanted < len(imports):
            raise ValueError("A place names an import that is not in the list")
        if wanted is not None and kind == KIND_CALL and not imports[wanted][1]:
            raise ValueError("A direct call to the optional name %s cannot be made safe" % imports[wanted][0])
        out += struct.pack("<3I", offset, kind | ((wanted + 1) << 16 if wanted is not None else 0), value & 0xffffffff)
    for entry, work in marks:
        if not (0 <= entry < entries and 0 <= work < len(works)):
            raise ValueError("A mark names a table entry or a pack that is not there")
        out += struct.pack("<2I", entry, work)
    while len(strings) % 4:
        strings.append(0)
    lists = struct.pack("<6I", len(works), len(exports), len(imports), len(places), len(marks), len(strings)) + bytes(out) + bytes(strings)
    header = HEADER3.pack(MAGIC, FORMAT_LINKED, HEADER3_BYTES, target_os, link_address, len(body), compare_bytes, zeroed_bytes,
                          zlib.crc32(body + lists) & 0xffffffff, text, start, end, len(lists), len(extra), version, interface, align, flags,
                          path_place, card_place)
    return header + body + lists + extra


def read_lists(lists):
    """The lists behind a format-3 body as a dict of tuples, names as text."""
    if len(lists) < 4 * LIST_COUNTS:
        raise ValueError("The lists are shorter than their counts")
    works_n, exports_n, imports_n, places_n, marks_n, strings_n = struct.unpack_from("<6I", lists)
    at = 4 * LIST_COUNTS
    need = at + 12 * works_n + 8 * exports_n + 8 * imports_n + 12 * places_n + 8 * marks_n + strings_n
    if need != len(lists):
        raise ValueError("The lists' counts do not fit their size")
    strings = lists[len(lists) - strings_n:]

    def string(offset):
        end = strings.find(b"\0", offset)
        if offset >= len(strings) or end < 0:
            raise ValueError("A name lies outside the strings")
        return strings[offset:end].decode("ascii")

    out = {"works": [], "exports": [], "imports": [], "places": [], "marks": []}
    for _ in range(works_n):
        name, number, flags = struct.unpack_from("<3I", lists, at)
        out["works"].append((string(name), number, bool(flags & NEEDED)))
        at += 12
    for _ in range(exports_n):
        name, offset = struct.unpack_from("<2I", lists, at)
        out["exports"].append((string(name), offset))
        at += 8
    for _ in range(imports_n):
        name, flags = struct.unpack_from("<2I", lists, at)
        out["imports"].append((string(name), bool(flags & NEEDED)))
        at += 8
    for _ in range(places_n):
        offset, kind, value = struct.unpack_from("<3I", lists, at)
        out["places"].append((offset, kind & 0xff, (kind >> 16) - 1 if kind >> 16 else None, value))
        at += 12
    for _ in range(marks_n):
        out["marks"].append(struct.unpack_from("<2I", lists, at))
        at += 8
    return out


def parse3(data):
    """A format-3 pack's fields (its lists among them, read), its body and what follows the lists."""
    if len(data) < HEADER3_BYTES or data[:8] != MAGIC:
        raise ValueError("Not a pack file")
    (magic, fmt, header_bytes, target_os, link, body_bytes, compare, zeroed, crc, name, table, table_end, lists_bytes, extra_bytes,
     version, interface, align, flags, path_place, _reserved) = HEADER3.unpack_from(data)
    if fmt != FORMAT_LINKED or header_bytes != HEADER3_BYTES:
        raise ValueError("Unknown pack format")
    if HEADER3_BYTES + body_bytes + lists_bytes + extra_bytes != len(data) or body_bytes % 4 or compare > body_bytes:
        raise ValueError("The header's sizes do not fit the file")
    body = data[HEADER3_BYTES:HEADER3_BYTES + body_bytes]
    lists = data[HEADER3_BYTES + body_bytes:HEADER3_BYTES + body_bytes + lists_bytes]
    if zlib.crc32(body + lists) & 0xffffffff != crc:
        raise ValueError("The checksum of the body and the lists differs from the header's")
    if not (link <= table <= table_end <= link + compare and table % 4 == 0 and (table_end - table) % ENTRY_BYTES == 0):
        raise ValueError("The header's table does not lie in the pack's code and constant data")
    # Since M1 a table may have no entry (one address as the table and as its end: the pack changes no place of the
    # firmware), and the header's last word is the place of the pack's card in the body, 0 for none.
    card_place = _reserved
    if card_place and not card_stands(body, card_place):
        raise ValueError("The header's card does not lie in the pack's body, on a word, with a card's mark and size")
    fields = {"format": fmt, "os_id": target_os, "load_address": link, "body_bytes": body_bytes, "compare_bytes": compare,
              "bss_bytes": zeroed, "crc": crc, "name": name.rstrip(b"\0").decode("ascii"), "table": table, "table_end": table_end,
              "entries": (table_end - table) // ENTRY_BYTES, "lists_bytes": lists_bytes, "extra_bytes": extra_bytes, "version": version,
              "interface": interface, "align": align, "flags": flags, "path_place": path_place, "lists": read_lists(lists)}
    fields["card_place"] = card_place
    return fields, body, data[HEADER3_BYTES + body_bytes + lists_bytes:]


def parse(data):
    """The header's fields and the body, or ValueError."""
    if len(data) < HEADER_BYTES or data[:8] != MAGIC:
        raise ValueError("Not a pack file")
    if struct.unpack_from("<I", data, 8)[0] == FORMAT_LINKED:
        fields, body, _extra = parse3(data)
        return fields, body
    magic, fmt, header_bytes, target_os, load, body_bytes, compare, bss, crc, name, r0, r1 = HEADER.unpack_from(data)
    body = data[HEADER_BYTES:]
    if fmt not in (FORMAT, FORMAT_LOADED) or header_bytes != HEADER_BYTES or (fmt == FORMAT and (r0 or r1)):
        raise ValueError("Unknown pack format")
    if fmt == FORMAT_LOADED and not (load <= r0 <= r1 <= load + compare and r0 % 4 == 0 and (r1 - r0) % ENTRY_BYTES == 0):
        raise ValueError("The header's table does not lie in the pack's code and constant data")
    if body_bytes != len(body) or compare > body_bytes:
        raise ValueError("The header's sizes do not fit the file")
    if zlib.crc32(body) & 0xffffffff != crc:
        raise ValueError("The body's checksum differs from the header's")
    fields = {"format": fmt, "os_id": target_os, "load_address": load, "body_bytes": body_bytes, "compare_bytes": compare,
              "bss_bytes": bss, "crc": crc, "name": name.rstrip(b"\0").decode("ascii")}
    if fmt == FORMAT_LOADED:
        fields["table"], fields["table_end"], fields["entries"] = r0, r1, (r1 - r0) // ENTRY_BYTES
    return fields, body


def image(fields, body, place, resolve, path=None):
    """What a format-3 pack's body is once it stands at `place` and its imports are filled: what the base makes of
    it. `resolve(name)` answers an address, or None for a name nobody offers (an optional import is then zero).
    With `path`, the pack file's path on the unit, the body is as the base leaves it for a pack that asks for its
    path: the text at the header's place for it, zeros behind it."""
    out = bytearray(body)
    if path is not None and fields.get("path_place"):
        text = path.encode("ascii")[:PATH_BYTES - 1]
        out[fields["path_place"]:fields["path_place"] + PATH_BYTES] = text + bytes(PATH_BYTES - len(text))
    delta = place - fields["load_address"]
    lists = fields["lists"]
    for offset, kind, wanted, value in lists["places"]:
        if wanted is None:
            address = (value + delta) & 0xffffffff
        else:
            name, needed = lists["imports"][wanted]
            target = resolve(name)
            if target is None and needed:
                raise ValueError("The needed name %s is offered by nobody" % name)
            address = ((target or 0) + value) & 0xffffffff
        if kind == KIND_HIGH:
            struct.pack_into("<H", out, offset, address >> 16)
        elif kind == KIND_LOW:
            struct.pack_into("<H", out, offset, address & 0xffff)
        elif kind == KIND_WORD:
            struct.pack_into("<I", out, offset, address)
        else:
            distance = address - (place + offset - 2)
            if distance % 2 or not -(1 << 24) <= distance < (1 << 24):
                raise ValueError("A direct call does not reach 0x%08x" % address)
            steps = (distance >> 1) & 0xffffff
            out[offset - 2] = steps >> 16
            struct.pack_into("<H", out, offset, steps & 0xffff)
    return bytes(out)


def module_layout(elf_path, base):
    """(compare_bytes, bss_bytes, binary_bytes) from the linked module: the code and constant data end where `.data`
    starts, and the binary ends where `.data` does."""
    sys.path.insert(0, os.path.join(ROOT, "open-xy"))
    from tooling.formats import elf32
    with open(elf_path, "rb") as f:
        sections, _symbols = elf32(f.read())
    if sections[".text"]["address"] != base:
        raise ValueError("The module's first byte is not at the load address")
    data = sections[".data"]
    return data["address"] - base, sections[".bss"]["size"], data["address"] + data["size"] - base


def loaded_layout(elf_path):
    """(load address, compare_bytes, zeroed bytes after the body, binary bytes, (table, table end)) of a pack linked
    for its own address: all from its ELF. The zeroed bytes run from the binary's end to the end of its zeroed data."""
    sys.path.insert(0, os.path.join(ROOT, "open-xy"))
    from tooling.formats import elf32
    with open(elf_path, "rb") as f:
        sections, symbols = elf32(f.read())
    load = symbols["__module_reserved_start"]["value"]
    if sections[".text"]["address"] != load:
        raise ValueError("The pack's first byte is not at its load address")
    data = sections[".data"]
    binary = data["address"] + data["size"] - load
    if symbols["__module_initialized_end"]["value"] - load != binary:
        raise ValueError("The pack's initialised data does not end where the linker says")
    zeroed = symbols["__module_end"]["value"] - load - binary
    return load, data["address"] - load, zeroed, binary, (symbols["op1f_boot_table"]["value"], symbols["op1f_boot_table_end"]["value"])


# ---------------------------------------------------------------- format 3: the linker's records


class Elf:
    """An ELF file's sections, symbols and relocation records, as much as a pack's lists need."""

    def __init__(self, data):
        self.data = data
        shoff, = struct.unpack_from("<I", data, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", data, 0x2e)
        raw = [struct.unpack_from("<10I", data, shoff + i * shentsize) for i in range(shnum)]
        names = raw[shstrndx]
        self.sections = []
        for name, kind, flags, address, offset, size, link, info, align, entsize in raw:
            self.sections.append({"name": self._text(names[4] + name), "type": kind, "address": address, "offset": offset, "size": size,
                                  "link": link, "info": info, "align": align})

    def _text(self, at):
        return self.data[at:self.data.index(b"\0", at)].decode("ascii")

    def section(self, name):
        return next(s for s in self.sections if s["name"] == name)

    def symbols(self):
        """Every symbol of the symbol table: name, value, binding, section index."""
        out = []
        for s in self.sections:
            if s["type"] != SHT_SYMTAB:
                continue
            strings = self.sections[s["link"]]["offset"]
            for at in range(s["offset"], s["offset"] + s["size"], 16):
                name, value, _size, info, _other, shndx = struct.unpack_from("<IIIBBH", self.data, at)
                out.append({"name": self._text(strings + name), "value": value, "bind": info >> 4, "shndx": shndx})
        return out

    def records(self):
        """Every relocation record: (address or offset, type, the symbol, the addend, the section it is about)."""
        symbols = self.symbols()
        out = []
        for s in self.sections:
            if s["type"] != SHT_RELA:
                continue
            about = self.sections[s["info"]]["name"]
            for at in range(s["offset"], s["offset"] + s["size"], 12):
                offset, info, addend = struct.unpack_from("<IIi", self.data, at)
                out.append((offset, info & 0xff, symbols[info >> 8], addend, about))
        return out


def toolchain(name):
    path = os.path.join(ROOT, "open-xy", "local", "toolchain", "bin", name + ".exe")
    if not os.path.isfile(path):
        path = path[:-4]
    if not os.path.isfile(path):
        raise ValueError("toolchain binary not found: %s" % name)
    return path


def wanted_names(objects):
    """(needed, optional): the names these objects want and none of them defines. A name some object wants outright
    is needed; one that every object declares weak is optional."""
    defined, strong, weak = set(), set(), set()
    for path in objects:
        with open(path, "rb") as f:
            for symbol in Elf(f.read()).symbols():
                if not symbol["name"]:
                    continue
                if symbol["shndx"] == SHN_UNDEF:
                    (weak if symbol["bind"] == STB_WEAK else strong).add(symbol["name"])
                elif symbol["bind"] in (1, STB_WEAK):
                    defined.add(symbol["name"])
    return sorted(strong - defined), sorted(weak - strong - defined)


def link3(objects, origin, out_base, needed, addresses=None, keep=True, cwd=ROOT):
    """Link a pack's objects for `origin` into <out_base>.elf and .bin, with the linker's records kept (`keep`).
    Every needed name of another pack gets an address: its real one from `addresses`, or the origin as a
    placeholder a direct call can reach. An optional name gets its real address when `addresses` has one, and is
    left to the linker otherwise, which puts zero. Answers the linked file and the binary's bytes."""
    with open(PACK_SCRIPT, encoding="utf-8") as f:
        script = f.read()
    if SCRIPT_ORIGIN not in script:
        raise ValueError("The pack's linker script does not name the room as expected")
    if origin % 4 or not ROOM_START <= origin < ROOM_START + ROOM_BYTES:
        raise ValueError("A pack is linked for an aligned address inside the room")
    with open(out_base + ".ld", "w", encoding="utf-8", newline="\n") as f:
        f.write(script.replace(SCRIPT_ORIGIN, "ORIGIN = 0x%08x, LENGTH = 0x%x" % (origin, ROOM_BYTES - (origin - ROOM_START))))
    command = [toolchain("bfin-elf-ld")] + (["-q"] if keep else []) + ["--unresolved-symbols=ignore-all", "-T", out_base + ".ld",
                                                                      "-Map=" + out_base + ".map", "-o", out_base + ".elf"]
    addresses = dict(addresses or {})
    for name in needed:
        addresses.setdefault(name, origin)
    for name in sorted(addresses):
        command += ["--defsym", "%s=0x%08x" % (name, addresses[name])]
    done = subprocess.run(command + list(objects), capture_output=True, text=True, cwd=cwd)
    problems = [line for line in done.stderr.splitlines() if "cannot find entry symbol" not in line]
    if done.returncode or problems:
        raise ValueError("The link failed:\n" + "\n".join(problems or [done.stdout]))
    subprocess.run([toolchain("bfin-elf-objcopy"), "-O", "binary", out_base + ".elf", out_base + ".bin"], check=True)
    with open(out_base + ".elf", "rb") as f:
        elf = Elf(f.read())
    with open(out_base + ".bin", "rb") as f:
        return elf, f.read()


def layout3(elf, origin):
    """(compare bytes, binary bytes, zeroed bytes, largest alignment, {symbol: value}) of a pack linked for `origin`."""
    values = {s["name"]: s["value"] for s in elf.symbols() if s["name"] and s["shndx"] != SHN_UNDEF}
    if elf.section(".text")["address"] != origin or values.get("__module_reserved_start") != origin:
        raise ValueError("The pack's first byte is not at the address it was linked for")
    data = elf.section(".data")
    binary = data["address"] + data["size"] - origin
    if values["__module_initialized_end"] - origin != binary:
        raise ValueError("The pack's initialised data does not end where the linker says")
    align = max(s["align"] for s in elf.sections if s["name"] in (".text", ".rodata", ".data", ".bss"))   # ld drops an empty .rodata or .bss
    return data["address"] - origin, binary, values["__module_end"] - origin - binary, align, values


def places3(elf, origin, needed, optional):
    """The places of a linked pack that hold an address, from the linker's records: (offset, kind, import's number
    or None, value), with the imports in the order needed then optional. A record about a stock address (an
    absolute symbol of an assembly source) and a call or jump inside the pack need none."""
    number = {name: i for i, name in enumerate(list(needed) + list(optional))}
    kinds = {R_HUIMM16: KIND_HIGH, R_LUIMM16: KIND_LOW, R_BYTE4_DATA: KIND_WORD, R_PCREL24: KIND_CALL, R_PCREL24_JUMP_L: KIND_CALL}
    out = []
    for address, kind, symbol, addend, about in elf.records():
        if about not in (".text", ".rodata", ".data"):
            raise ValueError("A relocation record in %s: only code and data are expected to hold addresses" % about)
        if kind not in kinds:
            raise ValueError("A relocation record of a kind not expected: 0x%x at 0x%08x" % (kind, address))
        name, offset = symbol["name"], address - origin
        if name in number:
            if kinds[kind] == KIND_CALL and name in optional:
                raise ValueError("A direct call to the optional name %s at 0x%08x cannot be made safe" % (name, address))
            out.append((offset, kinds[kind], number[name], addend))
        elif kinds[kind] == KIND_CALL:
            if symbol["shndx"] in (SHN_ABS, SHN_UNDEF):
                raise ValueError("A direct call at 0x%08x to %r, which is not in the pack: it would not move with it" % (address, name))
            continue                                        # a distance inside the pack: it moves with the pack
        elif symbol["shndx"] == SHN_ABS:
            continue                                        # an absolute symbol of an assembly source: a stock address
        elif symbol["shndx"] == SHN_UNDEF:
            raise ValueError("A record at 0x%08x names %r, which nobody defines" % (address, name))
        else:
            out.append((offset, kinds[kind], None, (symbol["value"] + addend) & 0xffffffff))
    return sorted(out)


def read_recipe(path):
    with open(path, encoding="utf-8") as f:
        recipe = json.load(f)
    sources = recipe["sources"]
    if isinstance(sources, str):
        with open(os.path.join(ROOT, sources), encoding="utf-8") as f:
            sources = f.read().split()
    recipe["sources"] = sources
    recipe.setdefault("works_with", [])
    recipe.setdefault("exports", [])
    recipe.setdefault("serves", [])
    recipe.setdefault("pictures", [])
    recipe.setdefault("marks", [])
    recipe.setdefault("named", True)
    recipe.setdefault("sum", False)
    recipe.setdefault("path", None)
    recipe.setdefault("link_address", "0x%08x" % ROOM_START)
    return recipe


def recipe_objects(recipe, out_dir=None, cache=None, compile_them=True):
    """A recipe's object files in the order of its sources, compiled into the build folder (each source once for a
    `cache` shared by several recipes), or, with `compile_them` off, as an earlier build left them there."""
    out_dir = out_dir or os.path.join(ROOT, "build")
    cache = {} if cache is None else cache
    objects = []
    for source in recipe["sources"]:
        if source not in cache:
            built = os.path.join(out_dir, os.path.basename(source) + ".o")
            if compile_them:
                sys.path.insert(0, os.path.join(ROOT, "tools"))
                import module_build
                built = module_build.compile_sources([source], out_dir, [os.path.join(ROOT, "module")])[0]
            elif not os.path.isfile(built):
                raise ValueError("%s is not built: %s is missing" % (source, built))
            cache[source] = built
        if cache[source] in objects:
            raise ValueError("Two sources would produce %s" % cache[source])
        objects.append(cache[source])
    return objects


def objects_manifest(objects):
    """{an object file's name: its SHA-256}: what a pack was built from."""
    out = {}
    for path in objects:
        with open(path, "rb") as f:
            out[os.path.basename(path)] = hashlib.sha256(f.read()).hexdigest()
    return out


def write_manifest(path, objects):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(objects_manifest(objects), f, indent=1, sort_keys=True)
        f.write("\n")


def objects_are(build, objects, out_dir=None):
    """Are these object files the ones the build was made from, by the manifest kept beside its linked files
    (<build>.objects)? False when a unit has changed since: the tree has moved on, and the pack is rebuilt from the
    commit that built it, not from here. None when no manifest was kept or an object is missing."""
    path = os.path.join(out_dir or os.path.join(ROOT, "build"), build + ".objects")
    if not os.path.isfile(path) or not all(os.path.isfile(o) for o in objects):
        return None
    with open(path, encoding="utf-8") as f:
        return json.load(f) == objects_manifest(objects)


def places_of(packs):
    """Where the base places packs of the new kind, given as the files' bytes in the order of their files' names:
    each at the next address a multiple of 32 from where it was linked, behind the pack before it, which takes its
    body and the larger of its lists and its zeroed data (module/build_j5c_base.cpp: load3, run_new)."""
    out, behind = [], ROOM_START
    for data in packs:
        fields = parse(data)[0]
        place = behind + ((fields["load_address"] - behind) & (PACK_ALIGN - 1))
        foot = fields["body_bytes"] + max(fields["lists_bytes"], fields["bss_bytes"])
        if place + foot > ROOM_START + ROOM_BYTES:
            raise ValueError("No room is left for '%s' at 0x%08x" % (fields["name"], place))
        out.append(place)
        behind = place + ((foot + 3) & ~3)
    return out


def stem_of(name):
    """A pack's name as a part of a symbol or of a file's name: small letters, digits and underscores."""
    return re.sub(r"[^a-z0-9]", "_", name.lower())


def art_symbol(name):
    """The name of a pack's directory of pictures: op1f_art_ and the pack's name as a symbol's part."""
    return "op1f_art_" + stem_of(name)


def read_pictures(recipe):
    """[(name, bytes)]: the recipe's pictures in its order, each file read from the repository's root."""
    out, seen = [], set()
    for item in recipe.get("pictures", []):
        name = item["name"]
        text = name.encode("ascii")
        if not 0 < len(text) < ART_NAME_BYTES or name in seen or any(not 0x20 < c < 0x7f or c in b'"\\' for c in text):
            raise ValueError("A picture's name is one to %d plain characters, and is there once: %r" % (ART_NAME_BYTES - 1, name))
        seen.add(name)
        with open(os.path.join(ROOT, item["file"]), "rb") as f:
            data = f.read()
        if not data:
            raise ValueError("The picture %s is empty" % item["file"])
        out.append((name, data))
    return out


def art_directory(recipe, pictures):
    """(a pack's directory of pictures as it stands in the body before the base writes the path into it, the
    pictures' part of the file): the pictures one behind the other, each at the place its entry names."""
    entries, extra = b"", b""
    for name, data in pictures:
        entries += ART_ENTRY.pack(name.encode("ascii"), len(extra), len(data), zlib.crc32(data) & 0xffffffff)
        extra += data
    head = ART_HEAD.pack(ART_MAGIC, len(pictures), len(extra), int(recipe["version"]), recipe["name"].encode("ascii"), b"")
    return head + entries, extra


def render_pictures(recipe, recipe_name):
    """The assembly source of a pack's directory of pictures, from its recipe: data of the pack, in its body."""
    pictures = read_pictures(recipe)
    if not pictures:
        raise ValueError("The recipe names no pictures")
    symbol = art_symbol(recipe["name"])
    _directory, extra = art_directory(recipe, pictures)

    def text(value, room):
        return '    .ascii "%s"\n    .fill %d, 1, 0\n' % (value, room - len(value))

    lines = [
        "/* Generated by tools/pack_build.py pictures from %s; do not edit by hand." % recipe_name,
        " * The directory of the pictures the pack '%s' carries in its file behind the lists (module/build_j5e_art.cpp):" % recipe["name"],
        " * the magic, the count, the bytes of the pictures' part, the pack's version, its name, 48 bytes for the file's",
        " * path, which the base writes at the start, then for each picture its name in 24 bytes, its place in the",
        " * pictures' part, its bytes and its CRC-32.",
        " */",
        ".section .data",
        ".balign 4",
        ".global %s" % symbol,
        ".type %s, STT_OBJECT" % symbol,
        "%s:" % symbol,
        "    .long 0x%08x, %d, %d, %d" % (ART_MAGIC, len(pictures), len(extra), int(recipe["version"])),
        text(recipe["name"], 16).rstrip("\n"),
        ".global %s_path" % symbol,
        ".type %s_path, STT_OBJECT" % symbol,
        "%s_path:" % symbol,
        "    .fill %d, 1, 0" % PATH_BYTES,
    ]
    at = 0
    for (name, data), item in zip(pictures, recipe["pictures"]):
        lines.append(text(name, ART_NAME_BYTES).rstrip("\n"))
        lines.append("    .long %d, %d, 0x%08x    /* %s */" % (at, len(data), zlib.crc32(data) & 0xffffffff, item["file"].replace("\\", "/")))
        at += len(data)
    lines += [".size %s, .-%s" % (symbol, symbol), ""]
    return "\n".join(lines)


def pictures_in(data):
    """The pictures a pack file carries: [(name, bytes)] read from the part behind its lists by its directory, each
    held against its entry's checksum. A pack that offers no directory carries none."""
    fields, body, extra = parse3(data)
    offsets = dict(fields["lists"]["exports"])
    symbol = art_symbol(fields["name"])
    if symbol not in offsets:
        if extra:
            raise ValueError("The file has a part behind its lists and the pack offers no directory for it")
        return []
    at = offsets[symbol]
    magic, count, total, version, name, _path = ART_HEAD.unpack_from(body, at)
    if magic != ART_MAGIC or total != len(extra) or version != fields["version"] or name.rstrip(b"\0").decode("ascii") != fields["name"]:
        raise ValueError("The directory of pictures does not fit the file it is in")
    if fields["path_place"] != at + ART_PATH_AT:
        raise ValueError("The base would not write the file's path into the directory of pictures")
    out = []
    for i in range(count):
        entry, offset, size, crc = ART_ENTRY.unpack_from(body, at + ART_HEAD.size + i * ART_ENTRY.size)
        picture = extra[offset:offset + size]
        if len(picture) != size or zlib.crc32(picture) & 0xffffffff != crc:
            raise ValueError("The picture %r is not what its entry says" % entry)
        out.append((entry.rstrip(b"\0").decode("ascii"), picture))
    return out


# ---------------------------------------------------------------- since M1: a pack's card and a base pack's release

CARD_SOURCE = """/* Generated by tools/pack_build.py card from %(recipe)s; do not edit by hand.
 * The pack's card (module/build_m1_card.h): what it wants of the base pack. The loader finds the card by the place
 * the pack file's header names and hands it to the base pack. In front of the card stands a word that names the
 * mark of the level the card uses, so that the pack needs that name: where no pack offers it, the loader leaves
 * this pack out. Nothing reads that word. Then the card: its mark, its bytes, the level, the pack's version and
 * name, the function called once a frame, the directory of the pack's pictures and the function that says which
 * of them a name stands for; 0 for each the pack has not.
 */
.section .rodata
.balign 4
.global %(label)s_level
.type %(label)s_level, STT_OBJECT
%(label)s_level:
    .long %(mark)s
.size %(label)s_level, .-%(label)s_level
.global %(label)s
.type %(label)s, STT_OBJECT
%(label)s:
    .long 0x%(magic)08x, %(bytes)d, %(level)d, %(version)d
%(name)s
    .long %(frame)s
    .long %(pictures)s
    .long %(pick)s
.size %(label)s, .-%(label)s
"""

RELEASE_SOURCE = """/* Generated by tools/pack_build.py release from %(recipe)s; do not edit by hand.
 * The release of the set of packs this pack is the base of: sixteen bytes, the text and zeros behind it. The
 * loader looks for the name %(label)s among the packs that run and shows the text on the start screen.
 */
.section .rodata
.balign 4
.global %(label)s
.type %(label)s, STT_OBJECT
%(label)s:
%(text)s
.size %(label)s, .-%(label)s
"""


def card_symbol(name):
    """The name of a pack's card in its sources: op1f_card_ and the pack's name as a symbol's part. The word in
    front of the card, which names the mark of its level, is this name and _level."""
    return "op1f_card_" + stem_of(name)


def level_name(level):
    """The mark of a level of the base pack's interface: a name a pack with a card of that level needs."""
    return "op1f_level_%d" % level


def card_of(recipe):
    """A recipe's card as (level, frame, pictures, pick), or None for a recipe without one. `frame` and `pick` are
    the symbols of two functions of the pack, or None. `pictures` says whether the card names the pack's directory
    of pictures: where the recipe's card does not say, it does when the recipe has pictures."""
    card = recipe.get("card")
    if card is None:
        return None
    carried = bool(recipe.get("pictures"))
    if (not isinstance(card, dict) or "level" not in card or set(card) - {"level", "frame", "pick", "pictures", "input", "folders", "asks"}
            or not isinstance(card.get("pictures", carried), bool)):
        raise ValueError('A recipe\'s card is {"level": 1, "frame": a function\'s symbol or null, "pick": the same, "pictures": true or false}')
    level = card["level"]
    if not isinstance(level, int) or isinstance(level, bool) or level not in CARD_LEVELS:
        raise ValueError("A card of level %r: this tool writes a card of level %s" % (level, " or ".join("%d" % known for known in CARD_LEVELS)))
    if "input" in card and level < 2:
        raise ValueError("Only a card of level 2 or later names an input function: it is level 2's")
    if "folders" in card and level < 3:                     # since M3
        raise ValueError("Only a card of level 3 or later lists folders: it is level 3's")
    if "folders" in card:
        folders_of_card(recipe["name"], card["folders"])
    if "asks" in card and level < 4:                        # since R1
        raise ValueError("Only a card of level 4 or later lists asks: it is level 4's")
    if "asks" in card:
        asks_of_card(card["asks"])
    for key in ("frame", "pick", "input"):
        if card.get(key) is not None and not (isinstance(card[key], str) and SYMBOL.fullmatch(card[key])):
            raise ValueError("A card's %s is the symbol of a function of the pack, or null: %r" % (key, card[key]))
    if card.get("pictures", carried) and not carried:
        raise ValueError("The recipe's card names the pack's pictures and the recipe has none")
    return level, card.get("frame"), card.get("pictures", carried), card.get("pick")


def card_input(recipe):
    """Since M2: the symbol of the input function a recipe's card of level 2 names, or None (for a card of level 1,
    for a card that names none, and for a recipe without a card). card_of has held it."""
    card = card_of(recipe)
    return None if card is None else recipe["card"].get("input")


def card_size(level):
    """Since M2: the bytes of a card of a level: 44 at level 1, 48 from level 2 on."""
    if level >= 4:                                          # since R1: 64 at level 4 (level 3's 56 is set where its card is written)
        return CARD_4_BYTES
    return CARD_2_BYTES if level >= 2 else CARD_BYTES


def asks_of_card(asks):
    """Since R1: a card's list of asks from its recipe, held to what this tool knows. Level 4 defines no kind of ask
    yet, so the list is empty: any ask is refused, in the words of what is wrong."""
    if not isinstance(asks, list) or len(asks) > ASKS_MOST:
        raise ValueError("A card's asks are a list of none to %d asks" % ASKS_MOST)
    for number, ask in enumerate(asks, 1):
        kind = ask.get("kind") if isinstance(ask, dict) else None
        if kind not in ASK_KINDS:
            raise ValueError("Ask %d of the card, of the kind %r: level 4 defines no kind of ask yet, so a card's asks are an empty list" % (number, kind))
    return []


def card_asks(recipe):
    """Since R1: the asks a recipe's card lists; none for a card without asks and for a recipe without a card.
    card_of has held them."""
    card = card_of(recipe)
    return [] if card is None else asks_of_card(recipe["card"].get("asks", []))


def folders_of_card(pack_name, folders):
    """Since M3: a card's list of folders from its recipe, each (name, place, shown) with what the recipe leaves out
    filled in, held to the rules the base holds a folder to. Refuses a list a folder cannot have, in the words of
    what is wrong."""
    if not isinstance(folders, list) or len(folders) > FOLDERS_MOST:
        raise ValueError("A card's folders are a list of none to %d folders" % FOLDERS_MOST)
    out = []
    for number, folder in enumerate(folders, 1):
        if not isinstance(folder, dict) or set(folder) - {"name", "place", "shown"} or not isinstance(folder.get("shown", True), bool):
            raise ValueError('A folder of a card is {"name": a name, "place": a place, "shown": true or false}')
        name = folder.get("name", pack_name if len(folders) == 1 else None)
        if name is None:
            raise ValueError("Folder %d has no name: only the one folder of a pack takes the pack's name" % number)
        place = folder.get("place", name)
        for what, text in (("name", name), ("place", place)):
            if not isinstance(text, str) or not FOLDER_FORM.fullmatch(text):
                raise ValueError("Folder %d's %s %r is not a folder's: one to fifteen characters of A-Z, a-z, 0-9, space, '_', '-' and '.', "
                                 "not beginning with a space, a dot or a dash and not ending with a space or a dot" % (number, what, text))
        shown = folder.get("shown", True)
        if shown and name.lower() in FOLDER_NAMES_TAKEN:
            raise ValueError("Folder %d's name %r is %s: the disk mode never shows a pack's folder under it" % (number, name, FOLDER_NAMES_TAKEN[name.lower()]))
        for other in out:
            if place.lower() == other[1].lower():
                raise ValueError("Folder %d's place %r is the place of an earlier folder of the pack" % (number, place))
            if shown and other[2] and name.lower() == other[0].lower():
                raise ValueError("Folder %d's name %r is the name of an earlier shown folder of the pack" % (number, name))
        out.append((name, place, shown))
    return out


def card_folders(recipe):
    """Since M3: the folders a recipe's card lists, each (name, place, shown); none for a card without folders and for
    a recipe without a card. card_of has held them."""
    card = card_of(recipe)
    return [] if card is None else folders_of_card(recipe["name"], recipe["card"].get("folders", []))


def folder_table(folders):
    """Since M3: a card's list of folders as the base reads it, 36 bytes a folder."""
    return b"".join(FOLDER.pack(name.encode("ascii"), place.encode("ascii"), 1 if shown else 0) for name, place, shown in folders)


def release_of(recipe):
    """A recipe's release, or None for a recipe without one: one to eight characters of 0-9, A-Z, a-z, '.', '_' and
    '-'. The loader takes more, fifteen plain characters: it is the part that costs an install, so it is the wider."""
    release = recipe.get("release")
    if release is None:
        return None
    if not isinstance(release, str) or not RELEASE_FORM.fullmatch(release):
        raise ValueError("A release is one to eight characters of 0-9, A-Z, a-z, '.', '_' and '-': %r" % (release,))
    return release


def header_name(name):
    """A pack's name in bytes, as its header takes it: a name the header cannot hold is refused in the header's own words."""
    make(bytes(4), ROOM_START, 4, 0, name, 0)
    return name.encode("ascii")


def padded_text(value, room):
    """A text and zeros behind it to `room` bytes, as two lines of assembly. A quote and a backslash are written so
    that the assembler takes them as the characters they are."""
    return '    .ascii "%s"\n    .fill %d, 1, 0' % (value.replace("\\", "\\\\").replace('"', '\\"'), room - len(value))


def render_card(recipe, recipe_name):
    """The assembly source of a pack's card, from its recipe: constant data of the pack, in its body."""
    card = card_of(recipe)
    if card is None:
        raise ValueError("The recipe names no card")
    level, frame, pictures, pick = card
    name = header_name(recipe["name"]).decode("ascii")
    text = CARD_SOURCE % {"recipe": recipe_name, "label": card_symbol(name), "mark": level_name(level), "magic": CARD_MARK, "bytes": card_size(level),
                          "level": level, "version": int(recipe["version"]), "name": padded_text(name, 16), "frame": frame or "0",
                          "pictures": art_symbol(name) if pictures else "0", "pick": pick or "0"}
    if level >= 2:                                          # since M2: the input function behind level 1's three
        end = "    .long %s\n.size %s," % (pick or "0", card_symbol(name))
        text = text.replace(" */\n.section", " * Level 2 (module/build_m2_card.h): behind those three, the function a key or a knob is offered to.\n */\n.section", 1)
        text = text.replace(end, "    .long %s\n    .long %s\n.size %s," % (pick or "0", card_input(recipe) or "0", card_symbol(name)), 1)
    if level >= 3:                                          # since M3: the list of folders and its count behind level 2's four, and the list itself
        folders, label = card_folders(recipe), card_symbol(name)
        size_line = ".size " + label + ", .-" + label + "\n"                       # the card's last line, which the list follows
        end = "    .long " + (card_input(recipe) or "0") + "\n" + size_line
        text = text.replace(" */\n.section", " * Level 3 (module/build_m3_card.h): behind it, the pack's list of folders and how many it holds; the list follows the card.\n */\n.section", 1)
        text = text.replace("0x%08x, %d, " % (CARD_MARK, card_size(level)), "0x%08x, %d, " % (CARD_MARK, CARD_3_BYTES), 1)     # the card's bytes: 56
        rows = "".join("%s\n%s\n    .long %d\n" % (padded_text(f_name, 16), padded_text(f_place, 16), 1 if shown else 0) for f_name, f_place, shown in folders)
        table_label = label + "_folders"
        table = (".global " + table_label + "\n.type " + table_label + ", STT_OBJECT\n" + table_label + ":\n" + rows + ".size " + table_label + ", .-" + table_label + "\n") if folders else ""
        text = text.replace(end, "    .long " + (card_input(recipe) or "0") + "\n    .long " + (table_label if folders else "0") + "\n    .long " + str(len(folders)) + "\n"
                            + size_line + table, 1)
    if level >= 4:                                          # since R1: the list of asks and its count behind level 3's two; level 4 knows no kind, so none
        card_asks(recipe)
        count_line = "    .long " + str(len(folders)) + "\n" + size_line                # level 3's last word and the card's last line
        text = text.replace(" */\n.section", " * Level 4: behind them, the pack's list of asks and how many it holds; level 4 defines no kind of ask yet, so both are 0.\n */\n.section", 1)
        text = text.replace("0x%08x, %d, " % (CARD_MARK, CARD_3_BYTES), "0x%08x, %d, " % (CARD_MARK, CARD_4_BYTES), 1)    # the card's bytes: 64
        text = text.replace(count_line, "    .long " + str(len(folders)) + "\n    .long 0\n    .long 0\n" + size_line, 1)
    return text


def render_release(recipe, recipe_name):
    """The assembly source of the release a base pack carries, from its recipe: sixteen bytes of its constant data."""
    release = release_of(recipe)
    if release is None:
        raise ValueError("The recipe names no release")
    return RELEASE_SOURCE % {"recipe": recipe_name, "label": RELEASE_NAME, "text": padded_text(release, RELEASE_BYTES)}


def write_source(path, text, command):
    """Write a source this tool generates. A file that is there is written over only when its first line says that
    the same command wrote it: a wrong --out then costs no source written by hand."""
    first = text.split(" from ", 1)[0] + " from "
    if os.path.exists(path):
        with open(path, encoding="utf-8", errors="replace") as f:
            if not f.readline().startswith(first):
                raise ValueError("Refusing to write over %s: it is no source the command `%s` wrote" % (path, command))
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def card_in(data):
    """The card a pack file carries, read by the place its header names: a dict of its fields, the three addresses as
    linked and 0 for none, or None for a pack without a card. A field the card is too short for is 0."""
    fields, body, _extra = parse3(data)
    at = fields["card_place"]
    if not at:
        return None
    size = struct.unpack_from("<I", body, at + 4)[0]
    held = bytes(body[at:at + min(size, CARD_2_BYTES)])
    _mark, size, level, version, name, frame, pictures, pick, input_function = CARD_2.unpack(held + bytes(CARD_2_BYTES - len(held)))
    card = {"place": at, "bytes": size, "level": level, "version": version, "name": name.split(b"\0")[0].decode("latin-1"),
            "frame": frame, "pictures": pictures, "pick": pick}
    if size >= CARD_2_BYTES and level >= 2:                 # since M2: a card of level 2, of level 2's size, has its input function
        card["input"] = input_function
    if size >= CARD_3_BYTES and level >= 3:                 # since M3: a card of level 3, of level 3's size, has its list of folders
        card["folders"], card["folder_count"] = struct.unpack_from("<2I", body, at + CARD_2_BYTES)
    if size >= CARD_4_BYTES and level >= 4:                 # since R1: a card of level 4, of level 4's size, has its list of asks
        card["asks"], card["ask_count"] = struct.unpack_from("<2I", body, at + CARD_3_BYTES)
    return card


def asks_in(data):
    """Since R1: the asks a pack file's card lists, each (kind, bytes, what) as the bytes have them, read from the
    list's place as linked; none for a card without asks or with a list that does not lie whole in the body."""
    fields, body, _extra = parse3(data)
    card = card_in(data)
    if card is None or not card.get("ask_count") or not card.get("asks") or card["ask_count"] > ASKS_MOST:
        return []
    at = card["asks"] - linked_at(fields, 0)
    if at < 0 or at % 4 or at + ASK.size * card["ask_count"] > len(body):
        return []
    return [ASK.unpack_from(body, at + ASK.size * number) for number in range(card["ask_count"])]


def linked_at(fields, offset):
    """Since M3: the address a place of a pack's body was linked for."""
    return fields["load_address"] + offset


def folders_in(data):
    """Since M3: the folders a pack file's card lists, each (name, place, shown) as the bytes have them, read from the
    list's place as linked (the pack's load address subtracted); none for a card without folders."""
    fields, body, _extra = parse3(data)
    card = card_in(data)
    if card is None or not card.get("folder_count"):
        return []
    at = card["folders"] - linked_at(fields, 0)
    out = []
    for number in range(card["folder_count"]):
        name, place, shown = FOLDER.unpack_from(body, at + FOLDER.size * number)
        out.append((name.split(b"\0")[0].decode("latin-1"), place.split(b"\0")[0].decode("latin-1"), bool(shown)))
    return out


def card_faults(data):
    """What is not in order with the card a pack file carries, as texts: none for a card as `build3` leaves one."""
    fields, _body, _extra = parse3(data)
    card, lists = card_in(data), fields["lists"]
    faults = []
    if card["bytes"] < CARD_BYTES:
        faults.append("it is shorter than a card of level 1, which has %d bytes" % CARD_BYTES)
    if card["name"] != fields["name"]:
        faults.append("its name is not the pack's")
    if card["version"] != fields["version"]:
        faults.append("its version is not the pack's, %d" % fields["version"])
    for number, (key, what) in enumerate((("frame", "frame function"), ("pictures", "directory of pictures"), ("pick", "choice among its pictures"), ("input", "input function"))):
        if card.get(key) and (card["place"] + CARD_BYTES - 12 + 4 * number, KIND_WORD, None, card[key]) not in lists["places"]:
            faults.append("its %s is no address of the pack's own, which would move with the pack" % what)
    directory = dict(lists["exports"]).get(art_symbol(fields["name"]))
    if card["pictures"] and (directory is None or card["pictures"] != linked_at(fields, directory)):
        faults.append("its directory of pictures is not the one the pack offers, %s" % art_symbol(fields["name"]))
    mark = level_name(card["level"])
    if (mark, True) not in lists["imports"] and mark not in [name for name, _offset in lists["exports"]]:
        faults.append("the mark of its level, %s, is no name the pack needs" % mark)
    if card.get("folder_count") or card.get("folders"):     # since M3: the list of folders moves with the pack and holds what the base takes
        if card["folder_count"] > FOLDERS_MOST or not card["folders"]:
            faults.append("its list of folders is not in order")
        elif (card["place"] + CARD_2_BYTES, KIND_WORD, None, card["folders"]) not in lists["places"]:
            faults.append("its list of folders is no address of the pack's own, which would move with the pack")
        else:
            try:
                folders_of_card(card["name"], [{"name": n, "place": p, "shown": s} for n, p, s in folders_in(data)])
            except ValueError as problem:
                faults.append("its list of folders is not one the base takes: %s" % problem)
    if card.get("ask_count") or card.get("asks"):           # since R1: the list of asks moves with the pack and lies in it, as the base holds it
        listed = asks_in(data)
        if card["ask_count"] > ASKS_MOST or not card["asks"] or not card["ask_count"]:
            faults.append("its list of asks is not in order")
        elif (card["place"] + CARD_3_BYTES, KIND_WORD, None, card["asks"]) not in lists["places"]:
            faults.append("its list of asks is no address of the pack's own, which would move with the pack")
        elif len(listed) != card["ask_count"] or any(size > ASK_BYTES_MOST or (what and (what % 4 or not linked_at(fields, 0) <= what <= linked_at(fields, len(_body)) - size))
                                                     for _kind, size, what in listed):
            faults.append("its list of asks does not lie in its pack on a word")
    return faults


def release_in(data):
    """The release a pack file offers under the name op1f_release, as the loader takes it: one to fifteen plain
    characters ended by a zero, in sixteen bytes that lie whole in the body. None for a pack that does not offer
    the name."""
    fields, body, _extra = parse3(data)
    at = dict(fields["lists"]["exports"]).get(RELEASE_NAME)
    if at is None:
        return None
    held = bytes(body[at:at + RELEASE_BYTES])
    end = held.find(b"\0")
    if len(held) != RELEASE_BYTES or end < 1 or any(not 0x21 <= c <= 0x7e for c in held[:end]):
        raise ValueError("The release the pack offers is not one the loader shows: one to fifteen plain characters ended by a zero, in sixteen bytes of the body")
    return held[:end].decode("ascii")


def card_lines(data):
    """What `check` prints of a pack file's card and of its release: (the lines, whether both are in order). A pack
    that has neither prints nothing."""
    lines, in_order = [], True
    card = card_in(data)
    if card is not None:
        said = ["%s at 0x%08x" % (what, card[key]) if card[key] else "no " + what.split(" ", 1)[-1]
                for key, what in (("frame", "a frame function"), ("pictures", "its pictures"), ("pick", "a choice among them"))]
        if "input" in card:                                 # since M2: a card of level 2's size names its input function too
            said.append("an input function at 0x%08x" % card["input"] if card["input"] else "no input function")
        lines.append("card: at %d in the body, %d bytes: '%s' version %d, level %d; as linked %s"
                     % (card["place"], card["bytes"], card["name"], card["version"], card["level"], ", ".join(said)))
        if "folder_count" in card:                          # since M3: a card of level 3's size, with its folders
            listed = folders_in(data) if 0 < card["folder_count"] <= FOLDERS_MOST and card["folders"] else []
            lines.append("card: %d folder(s)%s" % (card["folder_count"], "".join("; '%s' at user/pf/%s/, %s" % (n, p, "shown" if s else "not shown") for n, p, s in listed)))
        if "ask_count" in card:                             # since R1: a card of level 4's size, with its asks
            asked = asks_in(data)
            lines.append("card: asks: none" if not card["ask_count"] else "card: %d ask(s)%s; level 4 defines no kind of ask, so the base leaves them aside"
                         % (card["ask_count"], "".join("; kind %d, %d bytes at 0x%08x" % ask for ask in asked)))
        faults = card_faults(data)
        if faults:
            lines.append("card: NOT in order: %s" % "; ".join(faults))
            in_order = False
    try:
        release = release_in(data)
        if release is not None:
            lines.append("release: %s" % release)
    except ValueError as problem:
        lines.append("release: NOT in order: %s" % problem)
        in_order = False
    return lines, in_order


def build3(recipe, target_os, out_dir=None, objects=None, log=print, serve=None):
    """Compile, link and prove a pack of the new kind from its recipe, and answer (the file's bytes, a summary).
    Written into the build folder: <build>.elf, .map, .bin and .ld, the pack as linked for its address. `serve` is
    {pack: (needed, optional)}: the names the packs this one serves want; those this pack has are offered beside
    the recipe's own list."""
    out_dir = out_dir or os.path.join(ROOT, "build")
    if objects is None:
        objects = recipe_objects(recipe, out_dir)
    origin = int(recipe["link_address"], 16)
    if (origin - ROOM_START) % PACK_ALIGN:
        raise ValueError("A pack is linked for an address a multiple of %d from the room's first byte" % PACK_ALIGN)
    needed, optional = wanted_names(objects)
    out_base = os.path.join(out_dir, recipe["build"])
    elf, body = link3(objects, origin, out_base, needed)
    write_manifest(out_base + ".objects", objects)          # what it was built from: a unit changed later shows here
    compare, binary, zeroed, align, values = layout3(elf, origin)
    if binary != len(body):
        raise ValueError("The pack's binary is %d bytes and its linked file says %d" % (len(body), binary))
    if align > PACK_ALIGN:
        raise ValueError("The pack asks an alignment of %d: it could not be moved by multiples of %d" % (align, PACK_ALIGN))
    places = places3(elf, origin, needed, optional)
    # the proof: the same objects linked 32 bytes on are this binary with its places moved
    other_base = os.path.join(out_dir, recipe["build"] + "_moved")
    _other, moved = link3(objects, origin + PACK_ALIGN, other_base, needed, keep=False)
    for extension in (".elf", ".bin", ".ld", ".map"):
        os.remove(other_base + extension)
    while len(body) % 4:                                    # whole words: the lists stand right behind the body
        body += b"\0"
        moved += b"\0"
        zeroed -= 1
    imports = [(name, True) for name in needed] + [(name, False) for name in optional]
    fields = {"load_address": origin, "lists": {"places": places, "imports": imports}}
    if image(fields, body, origin + PACK_ALIGN, lambda name: origin + PACK_ALIGN if name in needed else None) != moved:
        raise ValueError("Moved by its places, the pack is not what the linker builds %d bytes on: a place is missing or wrong" % PACK_ALIGN)
    def has(name):
        return name in values and name not in needed and origin <= values[name] < origin + len(body) + zeroed

    exports = []
    for name in recipe["exports"]:
        if not has(name):
            raise ValueError("The export %s is not a name of this pack" % name)
        exports.append((name, values[name] - origin))
    served = set()
    for other in recipe.get("serves", []):                  # what the packs it serves want of it, by their own objects
        if serve is None or other not in serve:
            raise ValueError("The recipe serves '%s', whose wanted names were not handed over" % other)
        served.update(name for name in serve[other][0] + serve[other][1] if has(name))
    exports += [(name, values[name] - origin) for name in sorted(served - set(recipe["exports"]))]
    works = [(w["name"], int(w["interface"]), bool(w.get("needed", False))) for w in recipe["works_with"]]
    if ("op1f_boot_table" in values) != ("op1f_boot_table_end" in values):
        raise ValueError("The sources name a table of stock changes without its end, or its end without the table: op1f_boot_table and op1f_boot_table_end go together")
    no_table = "op1f_boot_table" not in values              # since M1: sources with no table make a pack that changes no place of the firmware
    table = (origin, origin) if no_table else (values["op1f_boot_table"], values["op1f_boot_table_end"])
    sites = [struct.unpack_from("<I", body, at - origin + 4)[0] for at in range(table[0], table[1], ENTRY_BYTES)]
    marks = []
    for mark in recipe["marks"]:
        site, away = int(mark["site"], 16), [w[0] for w in works].index(mark["away"])
        marks.append((sites.index(site), away))
    path_place = 0
    if recipe["path"]:
        path_place = values[recipe["path"]] - origin
    extra = b""
    pictures = read_pictures(recipe)
    if pictures:                                            # they go behind the lists; the pack's body holds their directory
        symbol = art_symbol(recipe["name"])
        directory, extra = art_directory(recipe, pictures)
        if not has(symbol) or values.get(symbol + "_path") != values[symbol] + ART_PATH_AT:
            raise ValueError("The recipe names pictures and the sources hold no directory %s: write it with `pictures`" % symbol)
        at = values[symbol] - origin
        if bytes(body[at:at + len(directory)]) != directory:
            raise ValueError("The directory of pictures in the sources is not the recipe's: write it again with `pictures`")
        if recipe["path"] not in (None, symbol + "_path"):
            raise ValueError("A pack with pictures takes its file's path into its directory")
        path_place = at + ART_PATH_AT
        if symbol not in [name for name, _offset in exports]:
            exports.append((symbol, at))                    # the picture loader of another pack finds it by this name
    # Since M1: the card and the release. Each is a source `card` or `release` wrote from the recipe, and what was
    # linked is held against the recipe here: a number the recipe changes (the pack's version stands in the card)
    # is followed into the source, or the build stops.
    card, card_place, label = card_of(recipe), 0, card_symbol(recipe["name"])
    if card is None and (has(label) or has(label + "_level")):
        raise ValueError("The sources hold a card %s and the recipe names none" % label)
    if card is not None:
        level, frame, with_pictures, pick = card
        if not (has(label) and has(label + "_level")):
            raise ValueError("The recipe names a card and the sources hold no card %s: write it with `card`" % label)
        code_end, functions = origin + elf.section(".text")["size"], []
        for function in (frame, pick, card_input(recipe)):  # each a function of this pack: a name in its code
            if function is not None and not (has(function) and values[function] < code_end and values[function] % 2 == 0):
                raise ValueError("The card's function %s is not a function of this pack" % function)
            functions.append(values[function] if function is not None else 0)
        directory = values[art_symbol(recipe["name"])] if with_pictures else 0      # held above: the recipe has pictures
        card_place, mark = values[label] - origin, level_name(level)
        level_place = (values[label + "_level"] - origin, KIND_WORD, needed.index(mark) if mark in needed else None, 0 if mark in needed else values.get(mark))
        linked = CARD.pack(CARD_MARK, card_size(level), level, int(recipe["version"]), recipe["name"].encode("ascii"), functions[0], directory, functions[1])
        if level >= 2:                                      # since M2: the input function behind level 1's three
            linked += struct.pack("<I", functions[2])
            if functions[2] and (card_place + CARD_BYTES, KIND_WORD, None, functions[2]) not in places:
                raise ValueError("The card's input function is no address of the pack's own, which would move with the pack")
        if level >= 3:                                      # since M3: 56 bytes, the list of folders and its count behind
            folders, table_label = card_folders(recipe), label + "_folders"
            if bool(folders) != has(table_label):
                raise ValueError("The card's list of folders in the sources is not the recipe's: write it again with `card`")
            table_at = values[table_label] if folders else 0
            linked = linked[:4] + struct.pack("<I", CARD_3_BYTES) + linked[8:] + struct.pack("<2I", table_at, len(folders))
            if folders and (bytes(body[table_at - origin:table_at - origin + FOLDER.size * len(folders)]) != folder_table(folders)
                            or (card_place + CARD_2_BYTES, KIND_WORD, None, table_at) not in places):
                raise ValueError("The card's list of folders in the sources is not the recipe's: write it again with `card`")
        if level >= 4:                                      # since R1: 64 bytes, the list of asks and its count behind; level 4 knows no kind, so none
            if card_asks(recipe) or has(label + "_asks"):
                raise ValueError("The card's list of asks in the sources is not the recipe's: write it again with `card`")
            linked = linked[:4] + struct.pack("<I", CARD_4_BYTES) + linked[8:] + struct.pack("<2I", 0, 0)
        if bytes(body[card_place:card_place + len(linked)]) != linked or level_place not in places:
            raise ValueError("The card in the sources is not the recipe's: write it again with `card`")
        if not card_place:
            raise ValueError("The card stands at the pack's first byte, where the header's word for it says there is none")
    release = release_of(recipe)
    if release is None and has(RELEASE_NAME):
        raise ValueError("The sources hold a release %s and the recipe names none" % RELEASE_NAME)
    if release is not None:
        if not has(RELEASE_NAME):
            raise ValueError("The recipe names a release and the sources hold no %s: write it with `release`" % RELEASE_NAME)
        release_place = values[RELEASE_NAME] - origin
        if bytes(body[release_place:release_place + RELEASE_BYTES]) != release.encode("ascii").ljust(RELEASE_BYTES, b"\0"):
            raise ValueError("The release in the sources is not the recipe's: write it again with `release`")
        if RELEASE_NAME not in [name for name, _offset in exports]:
            exports.append((RELEASE_NAME, release_place))   # the loader finds the text by this name
    flags = (0 if recipe["named"] else FLAG_UNNAMED) | (FLAG_SUM if recipe["sum"] else 0)
    pack = make3(body, origin, compare, zeroed, recipe["name"], target_os, table, int(recipe["version"]), int(recipe["interface"]),
                 works, exports, imports, places, marks, flags, path_place, align, extra, card_place=card_place, no_table=no_table)
    own = collections.Counter(PLACE_KINDS[p[1]] for p in places if p[2] is None)
    summary = ("'%s' version %d, interface %d, linked for 0x%08x: body %d, zeroed %d, %d stock changes, %d places of its own (%s), "
               "%d names offered, %d needed and %d optional names wanted in %d places; moved by %d bytes it is the linker's own link"
               % (recipe["name"], int(recipe["version"]), int(recipe["interface"]), origin, len(body), zeroed, len(sites), sum(own.values()),
                  ", ".join("%d %s" % (n, k) for k, n in own.items()), len(exports), len(needed), len(optional),
                  sum(1 for p in places if p[2] is not None), PACK_ALIGN))
    if pictures:
        summary += "; %d pictures in %d bytes behind the lists" % (len(pictures), len(extra))
    if card is not None:
        summary += "; a card of level %d at %d in the body" % (card[0], card_place)
    if release is not None:
        summary += "; the release %s" % release
    return pack, summary


def cmd_build(args):
    with open(args.profile, encoding="utf-8") as f:
        profile = json.load(f)
    with open(args.module, "rb") as f:
        body = f.read()
    if args.format == FORMAT_LOADED:
        base, compare, bss, binary, table = loaded_layout(args.elf)
    else:
        base, table = int(profile["module_base"], 16), None
        compare, bss, binary = module_layout(args.elf, base)
    if binary != len(body):
        raise ValueError("The module binary is %d bytes and its ELF says %d" % (len(body), binary))
    pack = make(body, base, compare, bss, args.name, os_id(profile["stock_sha256"]), table)
    if os.path.exists(args.out):
        raise ValueError("Refusing to overwrite %s" % args.out)
    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "xb") as f:
        f.write(pack)
    fields, _ = parse(pack)
    print("wrote %s: %d bytes; '%s' for OS %08x at 0x%08x: body %d, compared %d, zeroed %d, checksum %08x"
          % (args.out, len(pack), fields["name"], fields["os_id"], fields["load_address"], fields["body_bytes"],
             fields["compare_bytes"], fields["bss_bytes"], fields["crc"]))
    return 0


def from_the_root():
    """The module's assembly sources include their pictures by paths from the repository's root."""
    if not os.path.samefile(os.getcwd(), ROOT):
        raise ValueError("Run this from the repository's root: %s" % ROOT)


def partner_recipe(name, beside):
    path = os.path.join(os.path.dirname(os.path.abspath(beside)), name + ".json")
    if not os.path.isfile(path):
        raise ValueError("No recipe %s for the pack '%s'" % (path, name))
    return read_recipe(path)


def cmd_build3(args):
    """One recipe to --out, or several that are to run together to --out-dir, each under its pack's name. A recipe
    that serves other packs offers what their objects want of it: their recipes are those given, or lie beside it."""
    from_the_root()
    with open(args.profile, encoding="utf-8") as f:
        profile = json.load(f)
    use_profile(profile)                                    # since the port to 1.7.3: the room of the profile's reservation
    if (args.out is None) == (args.out_dir is None) or (args.out and len(args.recipe) != 1):
        raise ValueError("One recipe goes to --out; several go to --out-dir, each under its pack's name")
    recipes = [read_recipe(path) for path in args.recipe]
    if moved_room() and guards_sources():                   # since the port to 1.7.3: another firmware's room takes its own sources only
        problems = sources_for(recipes, profile)
        if problems:
            raise ValueError("Sources not made for the profile's firmware:\n  " + "\n  ".join(problems))
    outs =[args.out if args.out else os.path.join(args.out_dir, recipe["name"] + ".pack") for recipe in recipes]
    for out in outs:
        if os.path.exists(out):
            raise ValueError("Refusing to overwrite %s" % out)
    known = {recipe["name"]: recipe for recipe in recipes}
    cache, wants = {}, {}
    for recipe, path, out in zip(recipes, args.recipe, outs):
        serve = {}
        for other in recipe["serves"]:
            if other not in wants:
                wants[other] = wanted_names(recipe_objects(known.get(other) or partner_recipe(other, path), args.build_dir, cache=cache))
            serve[other] = wants[other]
        pack, summary = build3(recipe, os_id(profile["stock_sha256"]), args.build_dir, objects=recipe_objects(recipe, args.build_dir, cache=cache), serve=serve)
        os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
        with open(out, "xb") as f:
            f.write(pack)
        print("wrote %s: %d bytes; %s" % (out, len(pack), summary))
    return 0


def from_root(path):
    """A path as the build files name it: from the repository's root with forward slashes where it lies under it, so
    that a map or a listing says the same on any machine."""
    path = os.path.abspath(path)
    inside = os.path.splitdrive(path)[0].lower() == os.path.splitdrive(ROOT)[0].lower() and os.path.commonpath([path, ROOT]) == ROOT
    return os.path.relpath(path, ROOT).replace("\\", "/") if inside else path


def placed(recipes, folder, out_dir=None, listing=True, log=print, objects_dir=None, set_name=None, inside=None):
    """The pack files of `folder` that these recipes built, as the base places them when they run together: for each
    the linker's own link for its place, with the other packs' names at their real addresses, written as
    <build>_placed.elf, .bin, .map and .ld (and .dis.txt, its listing) into the build folder. That link is what stands
    in the unit's memory, so its listing is the one to read an address from the unit against; the tool's own
    reference of each pack at its place is held against it. The objects are those the build left in `objects_dir`
    (the build folder). With `set_name`, the set's own name, the files are <set_name>_<pack>_placed.*: a pack that was not
    built anew for the set stands at another place in it than in the set it was built for. A pack that asks for its
    file's path has it written by the base into its body: the sum that is printed, the one the base logs, is taken
    with the path of the file under its built name. `inside` is (its recipe, its file's path) of the pack the image
    holds itself (from image L5 on): it is the first of the set, at the room's first byte, whatever the files are
    called, and it is no file of the folder, so its place in the answer has no file's name. Answers [(file, fields,
    place, the link's bytes)]."""
    from_the_root()
    out_dir = out_dir or os.path.join(ROOT, "build")
    objects_dir = objects_dir or os.path.join(ROOT, "build")
    files = sorted((recipe["name"] + ".pack", recipe) for recipe in recipes)       # the order of the files' names, as the base takes them
    packs = []
    for name, _recipe in files:
        with open(os.path.join(folder, name), "rb") as f:
            packs.append(f.read())
    if inside is not None:                                  # the image's own pack: in front of every file
        files.insert(0, (None, inside[0]))
        with open(inside[1], "rb") as f:
            packs.insert(0, f.read())
    places = places_of(packs)
    parsed = [parse(data) for data in packs]
    offered = {}
    offered[LOADER_NAME] = LOADER_ADDRESS                   # since M1 the loader answers for its own name, before it looks at the packs
    for (fields, _body), place in zip(parsed, places):
        for name, offset in fields["lists"]["exports"]:
            if name.startswith(LOADER_FAMILY):
                raise ValueError("'%s' offers the name %s, and the loader keeps every name that begins %s to itself: it leaves such a pack out"
                                 % (fields["name"], name, LOADER_FAMILY))
            offered.setdefault(name, place + offset)        # the first that offers a name: the base leaves a second one out
    out = []
    for (file_name, recipe), (fields, body), place in zip(files, parsed, places):
        objects = [from_root(path) for path in recipe_objects(recipe, objects_dir, compile_them=False)]
        needed, _optional = wanted_names(objects)
        mine = {name for name, _offset in fields["lists"]["exports"]}
        addresses = {name: offered[name] for name, _needed in fields["lists"]["imports"] if name in offered and name not in mine}
        base_name = "%s_%s_placed" % (set_name, stem_of(fields["name"])) if set_name else recipe["build"] + "_placed"
        out_base = from_root(os.path.join(out_dir, base_name))
        _elf, linked = link3(objects, place, out_base, needed, addresses=addresses, keep=False)
        linked += b"\0" * (-len(linked) % 4)
        if image(fields, body, place, addresses.get) != linked:
            raise ValueError("'%s', moved by its places and joined by its lists, is not the linker's own link for 0x%08x" % (fields["name"], place))
        if listing:
            with open(out_base + ".dis.txt", "wb") as f:
                subprocess.run([toolchain("bfin-elf-objdump"), "-d", out_base + ".elf"], stdout=f, check=True)
        if file_name is None and (place != ROOM_START or fields["path_place"]):
            raise ValueError("The image's own pack '%s' stands at the room's first byte and has no file's path" % fields["name"])
        there = image(fields, body, place, addresses.get, path=PACK_FOLDER + file_name if file_name else None)
        log("%s '%s': at 0x%08x to 0x%08x (body %d, zeroed %d), %d from where it was linked; its sum as linked %08x%s; %s"
            % (file_name or "the image's own pack", fields["name"], place, place + fields["body_bytes"] + fields["bss_bytes"], fields["body_bytes"],
               fields["bss_bytes"], place - fields["load_address"], zlib.crc32(there) & 0xffffffff,
               " (the link's %08x, and the path %s%s written into it)" % (zlib.crc32(linked) & 0xffffffff, PACK_FOLDER, file_name) if fields["path_place"] else "",
               out_base + ".*"))
        out.append((file_name, fields, place, linked))
    return out


def cmd_placed(args):
    if (args.inside_recipe is None) != (args.inside_pack is None):
        raise ValueError("The image's own pack is named by both --inside-recipe and --inside-pack")
    if args.profile:                                        # since the port to 1.7.3: the room of the profile's reservation
        with open(args.profile, encoding="utf-8") as f:
            use_profile(json.load(f))
    else:
        sys.stderr.write("pack_build: no --profile: the packs are placed in the room of OS 1.7.0's, 0x%08x\n" % ROOM_START)
    inside = (read_recipe(args.inside_recipe), args.inside_pack) if args.inside_recipe else None
    placed([read_recipe(path) for path in args.recipe], args.folder, args.build_dir, set_name=args.name, inside=inside,
           objects_dir=args.build_dir)
    return 0


def cmd_pictures(args):
    """Write the assembly source of a pack's directory of pictures from its recipe. It is one of the pack's sources;
    `build3` holds what was linked against the recipe's pictures and refuses a directory that is not theirs."""
    from_the_root()
    text = render_pictures(read_recipe(args.recipe), args.recipe.replace("\\", "/"))
    with open(args.out, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print("wrote %s: %d pictures" % (args.out, text.count("    .long ") - 1))
    return 0


def cmd_card(args):
    """Write the assembly source of a pack's card from its recipe (since M1). It is one of the pack's sources; `build3`
    holds what was linked against the recipe, sets the header's word and refuses a card that is not the recipe's."""
    from_the_root()
    recipe = read_recipe(args.recipe)
    write_source(args.out, render_card(recipe, from_root(args.recipe)), "card")           # the recipe named from the root: the same source on any machine
    level, frame, pictures, pick = card_of(recipe)
    print("wrote %s: the card of '%s' version %d, level %d: %s, %s, %s"
          % (args.out, recipe["name"], int(recipe["version"]), level, "the frame function %s" % frame if frame else "no frame function",
             "its pictures" if pictures else "no pictures", "the choice among them %s" % pick if pick else "no choice among them"))
    if level >= 2:                                          # since M2
        print("  and level 2's input function: %s" % (card_input(recipe) or "none"))
    if level >= 3:                                          # since M3
        print("  and level 3's folders: %s" % ("; ".join("'%s' at user/pf/%s/, %s" % (n, p, "shown" if s else "not shown") for n, p, s in card_folders(recipe)) or "none"))
    if level >= 4:                                          # since R1
        print("  and level 4's asks: %s" % ("none" if not card_asks(recipe) else "%d" % len(card_asks(recipe))))
    return 0


def cmd_release(args):
    """Write the assembly source of the release a base pack carries, from its recipe (since M1). It is one of the
    pack's sources; `build3` holds what was linked against the recipe and offers the name."""
    from_the_root()
    recipe = read_recipe(args.recipe)
    write_source(args.out, render_release(recipe, from_root(args.recipe)), "release")
    print("wrote %s: the release %s of '%s', under the name %s" % (args.out, release_of(recipe), recipe["name"], RELEASE_NAME))
    return 0


def cmd_check(args):
    with open(args.pack, "rb") as f:
        data = f.read()
    fields, body = parse(data)
    lists = fields.pop("lists", None)
    in_order = True
    if not fields.get("card_place"):                        # since M1: a pack without a card is printed as it always was
        fields.pop("card_place", None)
    print(json.dumps(fields, indent=2))
    if lists is not None:
        own = collections.Counter(PLACE_KINDS[p[1]] for p in lists["places"] if p[2] is None)
        print("works with: %s" % (", ".join("%s (interface %d%s)" % (n, i, ", needed" if needed else "") for n, i, needed in lists["works"]) or "nobody"))
        print("offers: %s" % (", ".join(name for name, _ in lists["exports"]) or "nothing"))
        print("wants: %s" % (", ".join("%s%s" % (name, "" if needed else " (optional)") for name, needed in lists["imports"]) or "nothing"))
        print("places of its own: %s; places for names of others: %d" % (", ".join("%d %s" % (n, k) for k, n in own.items()),
                                                                           sum(1 for p in lists["places"] if p[2] is not None)))
        print("stock changes made only when a pack is away: %d" % len(lists["marks"]))
        pictures = pictures_in(data)
        print("pictures behind the lists: %s" % (", ".join("%s (%d)" % (name, len(picture)) for name, picture in pictures) or "none"))
        said, in_order = card_lines(data)                   # since M1: the card and the release, of a pack that has them
        for line in said:
            print(line)
    if args.module:
        with open(args.module, "rb") as f:
            module = f.read()
        if module != body:
            print("the body DIFFERS from %s" % args.module)
            return 1
        print("the body is %s" % args.module)
    if not in_order:                                        # a card or a release that is not as the tool leaves one
        return 1
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build")
    b.add_argument("--module", required=True, help="the module binary")
    b.add_argument("--elf", required=True, help="its ELF, for the sections")
    b.add_argument("--profile", required=True)
    b.add_argument("--name", required=True)
    b.add_argument("--out", required=True)
    b.add_argument("--format", type=int, default=FORMAT, choices=(FORMAT, FORMAT_LOADED), help="1: the module as a twin (J3); 2: a pack the base loads (J4)")
    b.set_defaults(func=cmd_build)
    b3 = sub.add_parser("build3", help="packs of the new kind (J5c) from their recipes: compiled, linked, proven, written")
    b3.add_argument("--recipe", required=True, action="append", help="a pack's recipe (repeatable: packs that are to run together)")
    b3.add_argument("--profile", required=True)
    b3.add_argument("--out", help="the pack file, for one recipe")
    b3.add_argument("--out-dir", help="the folder for several: each file is <pack name>.pack")
    b3.add_argument("--build-dir", help="the folder for the objects and the linked files (default build/; since the port to 1.7.3)")
    b3.set_defaults(func=cmd_build3)
    pl = sub.add_parser("placed", help="the packs of a folder as the base places them: the linker's own link of each for its place, and its listing")
    pl.add_argument("--recipe", required=True, action="append", help="the recipe of each pack that runs (repeatable)")
    pl.add_argument("--folder", required=True, help="where the built pack files lie, each as <pack name>.pack")
    pl.add_argument("--name", help="the set's name: the files are build/<name>_<pack>_placed.* (default: each recipe's build name)")
    pl.add_argument("--inside-recipe", help="the recipe of the pack the image holds itself (from image L5 on): it stands first, at the room's first byte")
    pl.add_argument("--inside-pack", help="that pack's file, which is no file of the folder")
    pl.add_argument("--profile", help="the profile whose reservation the packs run in (default: 1.7.0's room; since the port to 1.7.3)")
    pl.add_argument("--build-dir", help="where the build left the objects and where the placed files go (default build/; since the port to 1.7.3)")
    pl.set_defaults(func=cmd_placed)
    pi = sub.add_parser("pictures", help="the source of a pack's directory of pictures, from its recipe")
    pi.add_argument("--recipe", required=True)
    pi.add_argument("--out", required=True, help="the assembly source to write (one of the recipe's sources)")
    pi.set_defaults(func=cmd_pictures)
    ca = sub.add_parser("card", help="the source of a pack's card, from its recipe (since M1)")
    ca.add_argument("--recipe", required=True, help="the recipe, which names a `card`")
    ca.add_argument("--out", required=True, help="the source to write, module/build_m1_<pack>_card.s: the recipe names it among its sources")
    ca.set_defaults(func=cmd_card)
    rl = sub.add_parser("release", help="the source of the release a base pack carries, from its recipe (since M1)")
    rl.add_argument("--recipe", required=True, help="the recipe, which names a `release`")
    rl.add_argument("--out", required=True, help="the source to write, module/build_m1_base_release.s: the recipe names it among its sources")
    rl.set_defaults(func=cmd_release)
    c = sub.add_parser("check")
    c.add_argument("--pack", required=True)
    c.add_argument("--module", help="a module binary the body must equal")
    c.set_defaults(func=cmd_check)
    args = ap.parse_args(argv)
    try:
        return args.func(args)
    except ValueError as e:
        print("error: %s" % e)
        return 2


if __name__ == "__main__":
    sys.exit(main())

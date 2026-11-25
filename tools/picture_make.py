#!/usr/bin/env python3
"""Pictures made on the unit: the instructions a pack carries in place of a picture made from a stock picture.

  python -I tools/picture_make.py compile --stock "op-1field backup/content/display/singlelfo.svg" \\
         --picture candidates/content-i3/display/singlelfo.svg --out candidates/content-p1/display/singlelfo.svg.make
  python -I tools/picture_make.py make --instructions candidates/content-p1/display/singlelfo.svg.make \\
         --display "op-1field backup/content/display" [--out made.svg]
  python -I tools/picture_make.py check --instructions FILE [more] --display "op-1field backup/content/display"

A pack's picture is either a picture that is ours alone, handed to the firmware's reader as it is, or a set of
instructions by which the unit makes the picture from its own stock pictures when a screen asks for it
(analysis/picture_maker_study.md). The maker on the unit is in `kit` (module/build_p1_art.cpp); `make` here is the
same steps in Python, the reference the unit's code is held against on the PC.

A set of instructions is text, one instruction a line, a line ended by a line feed:

  op1f picture 1                  the first line: what this is
  size N                          the second: the bytes of the picture that is made
  from NAME BYTES CRC             the text to work on is the unit's own content/display/NAME, which must be BYTES long
                                  and have the CRC-32 CRC (eight hex digits): a unit whose picture is another one gets
                                  its own screen and a line on the log, never a picture put together from the wrong parts
  add end BYTES                   the BYTES bytes that follow this line (and a line feed behind them) are put in before
                                  the last closing tag of the picture, `</svg`

Step P1 has these and no more: a stock picture with groups of ours added, which is what the five LFO pages are.
Anything else in a set of instructions is refused, by the unit too.

Step P2 has a second kind, told by its first line, for a picture that is put together from stretches of stock
pictures and text of ours, a stretch moved or scaled where the picture needs it (tools/picture_delta.py writes them):

  op1f picture 2                  the first line
  size N                          the second, as above
  from NAME BYTES CRC             a stock picture the instructions take stretches from; up to four, numbered from 0
                                  in their order, a megabyte together, each checked by its length and CRC-32 as above
  c S OFFSET LENGTH               LENGTH bytes of the stock picture S from OFFSET, as they are
  m S OFFSET LENGTH SCALE DX DY   the same stretch with its geometry mapped: x' = SCALE/1000 x + DX/1000,
                                  y' = SCALE/1000 y + DY/1000 (tools/picture_map.py: whole numbers, three decimals)
  a BYTES                         the BYTES bytes that follow this line (and a line feed behind them), which are ours

So "shrink the tape" is one `m` over the whole picture, and "draw that part of theirs here" is a `c` or an `m` of the
part's stretch. The pack holds numbers and text of ours; the stretches are read from the unit's own files.

`compile` writes the instructions for a picture that is a stock picture with one stretch of ours before its closing
tag, and refuses any other picture. `check` holds that what a set of instructions carries is ours: no tag and no
shape of any stock picture (tools/stock_search.py's measure), and that it makes a picture, one the unit's reader of
pictures does not stop at (below).

Since R1 a fourth command, `census`: from every picture of `--display`, the unit's own, it collects the element names,
the attribute names by element, the values of every attribute that is not a number, a colour, an id (or a reference
to one, or a label naming the element as an id does), a path's `d` or a `points` list, the letters of the path
commands in use and the kinds of markup that are no element (the head `<?`, a comment). The census is made at every
run from the folder handed in, never kept in a file: it is the unit's own vocabulary. Then it reads what the
instructions' own bytes use (what an `a` or `add end` adds, read where it stands in the picture that is made). A path
command outside COMMANDS_DRAWN is refused, `<path>: uses the path command '<x>', which the unit's reader of pictures
does not know: it can stop the unit at its start`, and the answer is 1. Any other element, attribute, such a value or
markup the census has not is listed, `<path>: uses <what> '<value>', which no picture of the unit has: not proven on a
unit`; the answer stays 0, and is 1 with `--strict`. A folder with no picture in it is an error, never a census that
lets everything through.

`census` holds one thing more: a picture served under the name of one of the unit's own pictures takes that picture's
place, and the firmware's own screen looks the picture's parts up by their names. So the picture that is made must hold
every part (`id`) the unit's own picture of that name holds, the names compared as the firmware reads them (`_x5F_` is
an underscore, `_x2D_` a `-`, and so on), or the screen can stop the unit at its start. Each missing part is refused,
`<path>: takes the place of the unit's own <name> and has no part '<id>', which the unit's own has: the firmware's
screen can stop the unit at its start`, the first five named and a line with the count, and the answer is 1. The
picture's name is its instructions' file name less `.make`, or `--name` for one set.

`check` and `census` hold one thing more of the bytes of ours: what the unit's reader of pictures stops at. At three
things a picture can hold, the reader ends in a message on the screen and the unit does not start:

- a part's name (`id`) with a `_x` that begins no code. The reader takes `_x` in a name for the start of a character
  written by its code, as the unit's own pictures write one (`_x2D_`), and looks for the `_` that ends the code. With
  no `_` behind it, it stops (the screen says `parser.cpp:165 x_end`). With two characters before that `_` which are
  not both hex digits, it stops too. With any other count of characters it leaves the name as it is and does not
  stop. So `knob_x`, `knob_x1` and `my_xml_page` stop a unit; `knob_x_1` and `max_xpos_a` do not;
- a number with more than four digits behind its point, in a path's `d`, a `points` list, a `matrix`, or an attribute
  that is a number. A number written with an exponent is read another way and is not held to this;
- a `matrix` with fewer than six numbers, and a `translate` or a `scale` that is not two numbers with a comma or a
  space between them: `translate(10)` stops a unit, and so does `translate(10-20)`.

Each is refused, `<path>: ...: it stops the unit at its start`, the first five named and a line with the count, and
the answer is 1. The first was seen on a unit: a part named `..._x`, and the unit started again with the part
renamed. The others are read from the reader's code and were not tried on a unit. None of the unit's own pictures
holds any of the three.

Reads its inputs; `compile` and `make --out` write one file each and refuse to overwrite it.
"""

import argparse
import bisect
import os
import re
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

MARK = b"op1f picture 1\n"
MARK2 = b"op1f picture 2\n"         # the second kind (P2): stretches of stock pictures, kept or mapped, and text of ours
CLOSING = b"</svg"
SOURCES = 4                     # the stock pictures one set of the second kind takes stretches from, at most
LARGEST = 1 << 20               # the largest picture the unit makes: a megabyte
NAME_BYTES = 32                 # a stock picture's name with its ending, a zero byte counted


class Refused(ValueError):
    """A set of instructions that cannot be carried out: the text is what the unit's log line says."""


def is_instructions(data):
    return data[:len(MARK)] in (MARK, MARK2)


def instructions_for_added(stock_name, stock, picture):
    """The instructions for `picture`, which is the stock picture `stock` with one stretch of ours put in before its
    closing tag. ValueError for any other picture."""
    at = stock.rfind(CLOSING)
    if at < 0:
        raise ValueError("The stock picture has no closing tag")
    ours = len(picture) - len(stock)
    if ours <= 0 or picture[:at] != stock[:at] or picture[at + ours:] != stock[at:]:
        raise ValueError("The picture is not the stock picture with one stretch of ours before its closing tag")
    name = stock_name.encode("ascii")
    if not 0 < len(name) < NAME_BYTES or any(not 0x20 < c < 0x7f for c in name):
        raise ValueError("A stock picture's name is one to %d plain characters" % (NAME_BYTES - 1))
    if len(picture) > LARGEST:
        raise ValueError("The picture is larger than the unit makes")
    return (MARK + b"size %d\n" % len(picture) + b"from %s %d %08x\n" % (name, len(stock), zlib.crc32(stock) & 0xffffffff)
            + b"add end %d\n" % ours + picture[at:at + ours] + b"\n")


HEX = b"0123456789abcdef"


class _Lines:
    """A set of instructions read line by line, as the unit reads it."""

    def __init__(self, data, mark=MARK):
        if data[:len(mark)] != mark:
            raise Refused("it is no set of instructions")
        self.data, self.at = data, len(mark)

    def more(self):
        return self.at < len(self.data)

    def line(self):
        end = self.data.find(b"\n", self.at)
        if end < 0:
            raise Refused("its instructions end inside a line")
        words = self.data[self.at:end].split(b" ")
        self.at = end + 1
        return words

    def size(self):
        first = self.line()
        if len(first) != 2 or first[0] != b"size":
            raise Refused("its instructions do not begin with the picture's size")
        size = _number(first[1])
        if not 0 < size <= LARGEST:
            raise Refused("its instructions name a size the unit does not make")
        return size


def _number(text):
    if not 0 < len(text) <= 9 or any(not 0x30 <= c <= 0x39 for c in text):
        raise Refused("its instructions name a number that is none")
    return int(text)


def _plain_name(text):
    return 0 < len(text) < NAME_BYTES and all(0x20 < c < 0x7f and c not in b"/\\" for c in text)


def _signed(text):
    """A whole number that may have a minus before it: an offset."""
    return -_number(text[1:]) if text[:1] == b"-" else _number(text)


def _stock(words, read_stock):
    """The stock picture a `from` line names, checked as the unit checks it."""
    stock = read_stock(words[1].decode("ascii"))
    if stock is None:
        raise Refused("the unit's own picture could not be opened")
    if len(stock) != _number(words[2]):
        raise Refused("the unit's own picture is not as long as the instructions say")
    if zlib.crc32(stock) & 0xffffffff != int(words[3], 16):
        raise Refused("the unit's own picture is not the one the instructions were made for")
    return stock


def _stretch(sources, words):
    """The stretch a `c` or an `m` line names: (the stock picture, offset, length)."""
    number, offset, length = _number(words[1]), _number(words[2]), _number(words[3])
    if number >= len(sources):
        raise Refused("its instructions name a stock picture they did not take")
    if offset > len(sources[number]) or length > len(sources[number]) - offset:
        raise Refused("its instructions name a stretch outside the stock picture")
    return sources[number], offset, length


def make2(data, read_stock, blocks=None, most=SOURCES, spans=None):
    """The picture a set of instructions of the second kind makes, by the unit's steps in the unit's order. `most` is
    the count of stock pictures the kit holds for one set: four in the kit of P2, eight from the kit of P3 on."""
    # since R1: `spans`, a list, is given where each block of ours stands in the picture made, (first byte, end)
    import picture_map
    lines = _Lines(data, MARK2)
    size = lines.size()
    out = bytearray()
    sources = []
    held = 0
    while lines.more():
        words = lines.line()
        if len(words) == 4 and words[0] == b"from":
            if not _plain_name(words[1]) or len(words[3]) != 8 or any(c not in HEX for c in words[3]):
                raise Refused("its instructions name a stock picture badly")
            named = _number(words[2])
            if len(sources) == most:
                raise Refused("its instructions take more stock pictures than the unit holds")
            if named > LARGEST - held:                          # stock's allocator aborts when it has no room: what a set asks for is bounded
                raise Refused("its instructions take stock pictures larger than the unit holds at once")
            sources.append(_stock(words, read_stock))
            held += named
        elif len(words) == 4 and words[0] == b"c":
            source, offset, length = _stretch(sources, words)
            if len(out) + length > size:
                raise Refused("its instructions make more than the size they name")
            out += source[offset:offset + length]
        elif len(words) == 7 and words[0] == b"m":
            scale, dx, dy = _number(words[4]), _signed(words[5]), _signed(words[6])       # the numbers first, in their order
            _number(words[1]), _number(words[2]), _number(words[3])
            source, offset, length = _stretch(sources, words)
            try:
                made = picture_map.map_range(source[offset:offset + length], scale, dx, dy)
            except picture_map.Refused as refused:
                raise Refused(str(refused))
            if len(out) + len(made) > size:
                raise Refused("its instructions make more than the size they name")
            out += made
        elif len(words) == 2 and words[0] == b"a":
            count = _number(words[1])
            if count + 1 > len(data) - lines.at or data[lines.at + count:lines.at + count + 1] != b"\n":
                raise Refused("its instructions end inside what they add")
            if len(out) + count > size:
                raise Refused("its instructions make more than the size they name")
            block = data[lines.at:lines.at + count]
            if spans is not None:
                spans.append((len(out), len(out) + count))
            out += block
            if blocks is not None:
                blocks.append(block)
            lines.at += count + 1
        else:
            raise Refused("its instructions hold one this kit does not know")
    if len(out) != size:
        raise Refused("its instructions do not make the size they name")
    return bytes(out)


def make(data, read_stock, blocks=None, most=SOURCES, spans=None):
    """The picture a set of instructions makes, by the unit's own steps in the unit's own order, so that a set the
    unit refuses is refused here for the same reason. `read_stock(name)` answers a stock picture's bytes, or None
    when the unit has none of that name. `blocks`, a list, is given what the instructions add of their own."""
    # since R1: `spans`, a list, is given where each block of ours stands in the picture made, (first byte, end)
    if data[:len(MARK2)] == MARK2:
        return make2(data, read_stock, blocks, most, spans)
    lines = _Lines(data)
    size = lines.size()
    work = None
    while lines.more():
        words = lines.line()
        if len(words) == 4 and words[0] == b"from":
            if not _plain_name(words[1]) or len(words[3]) != 8 or any(c not in HEX for c in words[3]):
                raise Refused("its instructions name a stock picture badly")
            count = _number(words[2])
            if work is not None:
                raise Refused("its instructions take a stock picture twice")
            if count > size:
                raise Refused("its instructions name a stock picture larger than the result")
            stock = read_stock(words[1].decode("ascii"))
            if stock is None:
                raise Refused("the unit's own picture could not be opened")
            if len(stock) != count:
                raise Refused("the unit's own picture is not as long as the instructions say")
            if zlib.crc32(stock) & 0xffffffff != int(words[3], 16):
                raise Refused("the unit's own picture is not the one the instructions were made for")
            work = bytearray(stock)
        elif len(words) == 3 and words[0] == b"add" and words[1] == b"end":
            count = _number(words[2])
            if count + 1 > len(data) - lines.at or data[lines.at + count:lines.at + count + 1] != b"\n":
                raise Refused("its instructions end inside what they add")
            if work is None:
                raise Refused("its instructions add to a picture before they take one")
            at = bytes(work).rfind(CLOSING)
            if at < 0:
                raise Refused("the picture has no closing tag to add before")
            if len(work) + count > size:
                raise Refused("its instructions make more than the size they name")
            block = data[lines.at:lines.at + count]
            if spans is not None:                               # since R1: a block put in moves what stands behind it
                spans[:] = [(s + count, e + count) if s >= at else (s, e) for s, e in spans] + [(at, at + count)]
            work[at:at] = block
            if blocks is not None:
                blocks.append(block)
            lines.at += count + 1
        else:
            raise Refused("its instructions hold one this kit does not know")
    if work is None or len(work) != size:
        raise Refused("its instructions do not make the size they name")
    return bytes(work)


def own_text(data, read_stock, most=SOURCES):
    """What a set of instructions carries of its own: the blocks it adds, one behind the other."""
    blocks = []
    make(data, read_stock, blocks, most)
    return b"".join(blocks)


def stock_reader(folder):
    def read_stock(name):
        path = os.path.join(folder, name)
        if os.sep in name or "/" in name or not os.path.isfile(path):
            return None
        with open(path, "rb") as f:
            return f.read()
    return read_stock


# ---------------------------------------------------------------- since R1: the census of the unit's own pictures

_NUMBER = rb"[-+]?(?:[0-9]+\.?[0-9]*|\.[0-9]+)(?:[eE][-+]?[0-9]+)?"
_MARKUP = re.compile(rb"<\?.*?\?>|<!--.*?-->|<!\[CDATA\[.*?\]\]>|<![^>]*>"
                     rb"|<(/?)([A-Za-z_][\w:.-]*)((?:\s+[\w:.-]+\s*=\s*(?:\"[^\"<]*\"|'[^'<]*'))*)\s*/?>", re.S)
_ATTRIBUTE = re.compile(rb"([\w:.-]+)\s*=\s*(?:\"([^\"]*)\"|'([^']*)')")
_FREE = (b"id", b"d", b"points", b"inkscape:label")       # an id, a path's d, a points list, a label naming the element
_NUMBERS = re.compile(rb"\s*(?:" + _NUMBER + rb"(?:px|pt|pc|em|ex|mm|cm|in|%)?[\s,]*)+")
_COLOUR = re.compile(rb"#[0-9A-Fa-f]{3}|#[0-9A-Fa-f]{6}|#[0-9A-Fa-f]{8}|rgba?\(\s*[0-9.%\s,]+\)")
_REFERENCE = re.compile(rb"url\(\s*#[\w:.-]+\s*\)|#[\w:.-]+")                  # a reference to an id
_FUNCTIONS = re.compile(rb"\s*(?:[A-Za-z]+\(\s*(?:" + _NUMBER + rb"[\s,]*)*\)[\s,]*)+")   # a transform: numbers only
_COMMAND = re.compile(_NUMBER + rb"|([A-Za-z])")


def _free_value(name, value):
    """Whether an attribute's value is outside the census: a number or numbers, a colour, an id or a reference to
    one, a path's d, a points list or a transform of numbers."""
    return (name in _FREE or _NUMBERS.fullmatch(value) is not None or _COLOUR.fullmatch(value) is not None
            or _REFERENCE.fullmatch(value) is not None or _FUNCTIONS.fullmatch(value) is not None)


def _words_of(picture):
    """What a picture uses, each as (what, value, first byte, end): what is "the element", "the attribute of
    <element>", "the value of <attribute>", "the path command" or "the markup"; the bytes are where it stands. A `<`
    that begins nothing the census reads is markup of its own, its first bytes the value."""
    found = []
    at = picture.find(b"<")
    while at >= 0:
        token = _MARKUP.match(picture, at)
        if token is None:
            found.append(("the markup", picture[at:at + 12].split(b"\n")[0].decode("latin-1"), at, at + 1))
            at = picture.find(b"<", at + 1)
            continue
        if token.group(2) is None:
            text = token.group(0)
            kind = b"<?" if text.startswith(b"<?") else b"<!--" if text.startswith(b"<!--") else b"<![CDATA[" if text.startswith(b"<![CDATA[") \
                else b"<!" + re.match(rb"<!([A-Za-z]*)", text).group(1)
            found.append(("the markup", kind.decode("latin-1"), at, at + len(kind)))
        else:
            element = token.group(2).decode("latin-1")
            found.append(("the element", element, token.start(2), token.end(2)))
            for attribute in _ATTRIBUTE.finditer(picture, token.start(3), token.end(3)):
                name = attribute.group(1)
                value_group = 2 if attribute.group(2) is not None else 3
                value, first = attribute.group(value_group), attribute.start(value_group)
                found.append(("the attribute of " + element, name.decode("latin-1"), attribute.start(1), attribute.end(1)))
                if name == b"d":
                    for command in _COMMAND.finditer(value):
                        if command.group(1) is not None:
                            found.append(("the path command", command.group(1).decode("latin-1"), first + command.start(1), first + command.end(1)))
                elif not _free_value(name, value):
                    found.append(("the value of " + name.decode("latin-1"), value.decode("latin-1"), first, first + len(value)))
        at = picture.find(b"<", token.end())
    return found


def census(folder):
    """The unit's vocabulary, from every picture of its own in `folder`: the set of (what, value) its pictures use.
    Made at every run, never kept. A folder with no picture is an error: an empty census would let everything by."""
    if not os.path.isdir(folder):
        raise ValueError("%s is no folder: the census is made from the unit's own pictures" % folder)
    names = sorted(name for name in os.listdir(folder) if name.lower().endswith(".svg") and os.path.isfile(os.path.join(folder, name)))
    if not names:
        raise ValueError("%s holds no picture: the census is made from the unit's own pictures, and none is no census" % folder)
    held = set()
    for name in names:
        with open(os.path.join(folder, name), "rb") as f:
            held.update((what, value) for what, value, _first, _end in _words_of(f.read()))
    return held


def not_in_census(picture, spans, held):
    """What the bytes of ours in a picture use that the census has not, each (what, value) once, in the order met.
    `spans` are where the bytes of ours stand, as `make` gives them."""
    spans = sorted(spans)
    starts = [first for first, _end in spans]
    out, said = [], set()
    for what, value, first, end in _words_of(picture):
        place = bisect.bisect_right(starts, end - 1) - 1         # the last block that begins before this ends
        if place < 0 or spans[place][1] <= first:
            continue                                            # none of its bytes is ours
        if (what, value) not in held and (what, value) not in said:
            said.add((what, value))
            out.append((what, value))
    return out


# An id as the unit's pictures write it: a character that is no letter, digit or underscore by its code, `_x2D_` for
# `-`, and an underscore itself as `_x5F_` (the artwork tool's way). The firmware's screens look their parts up by the
# name so written out: `dot_x5F_white` is the part 'dot_white'.
_ID_CODE = re.compile(r"_x([0-9A-Fa-f]{2,4})_")


def part_name(written):
    """The name of a part as the firmware reads it, from its id as a picture writes it."""
    return _ID_CODE.sub(lambda m: chr(int(m.group(1), 16)), written)


def parts_of(picture):
    """The names of the parts (`id`) of a picture, as the firmware reads them, each once, in the order met."""
    out, said = [], set()
    for token in _MARKUP.finditer(picture):
        if token.group(2) is None:
            continue
        for attribute in _ATTRIBUTE.finditer(picture, token.start(3), token.end(3)):
            if attribute.group(1) == b"id":
                value = attribute.group(2) if attribute.group(2) is not None else attribute.group(3)
                name = part_name(value.decode("latin-1"))
                if name not in said:
                    said.add(name)
                    out.append(name)
    return out


def units_own(folder, name):
    """The unit's own picture of this name in `folder` (the name compared without regard to case, as the stricter
    reading), or None when the unit has none of it."""
    for own in sorted(os.listdir(folder)):
        if own.lower() == name.lower() and os.path.isfile(os.path.join(folder, own)):
            with open(os.path.join(folder, own), "rb") as f:
                return own, f.read()
    return None


def parts_missing(picture, own):
    """The parts the unit's own picture `own` has that `picture`, which takes its place, has not, in `own`'s order."""
    have = set(parts_of(picture))
    return [name for name in parts_of(own) if name not in have]


def picture_name(path):
    """The name a set of instructions' picture is served under: its file's name less `.make`."""
    base = os.path.basename(path)
    return base[:-len(".make")] if base.endswith(".make") else base


# ---------------------------------------------------------------- what the unit's reader of pictures stops at

DIGITS_READ = 4                 # the digits behind a number's point the unit's reader of pictures takes
STOPS_SAID = 5                  # the things it stops at named one a line; the count of all of them follows
_HEX_DIGITS = frozenset("0123456789abcdefABCDEF")
_PARTS_OF_NUMBER = re.compile(rb"[-+]?(?:[0-9]+(?:\.([0-9]*))?|\.([0-9]+))([eE][-+]?[0-9]+)?")       # the digits behind the point; an exponent
_FUNCTION = re.compile(rb"([A-Za-z]+)\s*\(([^)]*)\)")                                              # one function of a transform
_PAIR = re.compile(rb"\s*" + _NUMBER + rb"[ ,]+\s*" + _NUMBER)                                     # two numbers, a comma or a space between


def id_stop(written):
    """Why the unit's reader of pictures stops at a part's name as a picture writes it, or None when it does not. The
    reader's own steps: the first `_x`, then the next `_` behind it. None: it stops. Two characters between: a code,
    and it stops unless both are hex digits; it puts the character in the code's place and looks again from the
    front. Any other count: it leaves the name as it is and looks no further."""
    while True:
        at = written.find("_x")
        if at < 0:
            return None
        end = written.find("_", at + 2)
        if end < 0:
            return "no `_` ends it"
        if end - at != 4:
            return None
        code = written[at + 2:end]
        if any(c not in _HEX_DIGITS for c in code):
            return "'%s' is no code" % code
        if int(code, 16) == 0:
            return None                                         # the name ends at a zero
        written = written[:at] + chr(int(code, 16)) + written[end + 1:]


def _long_numbers(text):
    """The numbers of a text with more digits behind the point than the unit's reader takes, each as written. A
    number with an exponent is read another way and is not one of them."""
    return [number.group(0).decode("latin-1") for number in _PARTS_OF_NUMBER.finditer(text)
            if number.group(3) is None and len(number.group(1) or number.group(2) or b"") > DIGITS_READ]


def stops_in(picture, spans):
    """What the bytes of ours in a picture hold that the unit's reader of pictures stops at: the lines to say, in
    the order met. `spans` are where the bytes of ours stand, as `make` gives them."""
    spans = sorted(spans)
    starts = [first for first, _end in spans]
    start = "the unit's reader of pictures "
    out = []
    for token in _MARKUP.finditer(picture):
        if token.group(2) is None:
            continue
        for attribute in _ATTRIBUTE.finditer(picture, token.start(3), token.end(3)):
            group = 2 if attribute.group(2) is not None else 3
            name, value, first = attribute.group(1), attribute.group(group), attribute.start(group)
            place = bisect.bisect_right(starts, first + len(value) - 1) - 1      # the last block that begins before this ends
            if place < 0 or spans[place][1] <= first:
                continue                                        # none of its bytes is ours
            long_numbers = []
            if name == b"id":
                why = id_stop(value.decode("latin-1"))
                if why is not None:
                    out.append("names a part '%s': %stakes the `_x` in it for the start of a character's code, as in `_x2D_`, and %s"
                               % (value.decode("latin-1"), start, why))
            elif name == b"transform":
                for function in _FUNCTION.finditer(value):
                    word, numbers = function.group(1), function.group(2)
                    said = re.sub(rb"\s+", b" ", function.group(0)).decode("latin-1")
                    if word == b"matrix":
                        count = len(re.findall(_NUMBER, numbers))
                        if count < 6:
                            out.append("has the transform %s, with %d number(s): %swants six" % (said, count, start))
                        long_numbers += _long_numbers(numbers)
                    elif word in (b"translate", b"scale") and _PAIR.match(numbers) is None:
                        out.append("has the transform %s: %swants two numbers with a comma or a space between them" % (said, start))
            elif name in (b"d", b"points") or _NUMBERS.fullmatch(value) is not None:
                long_numbers = _long_numbers(value)
            for number in long_numbers:
                out.append("writes the number %s, with %d digits behind the point: %stakes %d at most"
                           % (number, len(number) - number.index(".") - 1, start, DIGITS_READ))
    return out


def say_stops(path, lines):
    """Says what `stops_in` found for the instructions at `path`, the first few a line each and then the count of all;
    answers that count."""
    for line in lines[:STOPS_SAID]:
        print("%s: %s: it stops the unit at its start" % (path, line))
    if len(lines) > STOPS_SAID:
        print("%s: %d thing(s) the unit's reader of pictures stops at in all" % (path, len(lines)))
    return len(lines)


def write_new(path, data):
    if os.path.exists(path):
        raise ValueError("%s exists; this tool does not overwrite" % path)
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "xb") as f:
        f.write(data)


def cmd_compile(args):
    with open(args.stock, "rb") as f:
        stock = f.read()
    with open(args.picture, "rb") as f:
        picture = f.read()
    data = instructions_for_added(os.path.basename(args.stock), stock, picture)
    blocks = []
    if make(data, lambda name: stock if name == os.path.basename(args.stock) else None, blocks) != picture:
        raise ValueError("The instructions do not make the picture")
    write_new(args.out, data)
    print("wrote %s: %d bytes of instructions for a picture of %d, %d of them ours; from %s, %d bytes, CRC-32 %08x"
          % (args.out, len(data), len(picture), sum(map(len, blocks)), os.path.basename(args.stock), len(stock), zlib.crc32(stock) & 0xffffffff))
    return 0


def cmd_make(args):
    with open(args.instructions, "rb") as f:
        data = f.read()
    picture = make(data, stock_reader(args.display))
    if args.out:
        write_new(args.out, picture)
    print("%s makes a picture of %d bytes, CRC-32 %08x%s" % (args.instructions, len(picture), zlib.crc32(picture) & 0xffffffff,
                                                           ", written to " + args.out if args.out else ""))
    return 0


def cmd_check(args):
    import stock_search
    display = stock_search.Display(args.display)
    bad = 0
    for path in args.instructions:
        with open(path, "rb") as f:
            data = f.read()
        blocks, spans = [], []
        picture = make(data, stock_reader(args.display), blocks, args.sources, spans)
        ours = b"".join(blocks)
        measure = display.measure(ours)
        stock_bytes = measure["stock tags"] + measure["stock shapes"] + measure["stock shapes, moved"]
        bad += 1 if stock_bytes else 0
        bad += say_stops(path, stops_in(picture, spans))        # what the unit's reader of pictures stops at
        print("%s: %d bytes of instructions, %d of them ours in %d tags, %d bytes of them a stock picture's; makes %d bytes, CRC-32 %08x"
              % (path, len(data), len(ours), measure["tags"], stock_bytes, len(picture), zlib.crc32(picture) & 0xffffffff))
    return 1 if bad else 0


# The path commands the unit's reader of pictures is known to draw: those the unit's own pictures use (M L H V C S, their
# small letters and z) and Z, which pictures of ours have drawn on a unit for days. A command outside them (an arc, once)
# can stop the unit at its start.
COMMANDS_DRAWN = frozenset("MmLlHhVvCcSsZz")
PARTS_SAID = 5                  # the missing parts named one a line; the count of all of them follows


def cmd_census(args):
    """Since R1: what the instructions' own bytes use, against the census of the unit's own pictures. A path command
    outside COMMANDS_DRAWN is refused; anything else no picture of the unit has is listed as not proven on a unit,
    and refused too with --strict. A picture that takes the place of one of the unit's own and lacks one of its parts
    is refused. So is one with something the unit's reader of pictures stops at (`stops_in`)."""
    held = census(args.display)                                 # the unit's vocabulary, made now from its own pictures
    if args.name is not None and len(args.instructions) != 1:
        raise ValueError("--name names the picture of one set of instructions; %d were handed in" % len(args.instructions))
    refused = not_proven = 0
    for path in args.instructions:
        with open(path, "rb") as f:
            data = f.read()
        spans = []
        picture = make(data, stock_reader(args.display), None, args.sources, spans)
        own = units_own(args.display, args.name if args.name is not None else picture_name(path))
        if own is not None:                                     # it takes the place of one of the unit's own pictures
            missing = parts_missing(picture, own[1])
            refused += len(missing)
            for part in missing[:PARTS_SAID]:
                print("%s: takes the place of the unit's own %s and has no part '%s', which the unit's own has: the firmware's screen can stop the unit at its start"
                      % (path, own[0], part))
            if missing:
                print("%s: %d part(s) of the unit's own %s missing in all" % (path, len(missing), own[0]))
        refused += say_stops(path, stops_in(picture, spans))    # what the unit's reader of pictures stops at
        unknown = not_in_census(picture, spans, held)
        for what, value in unknown:
            if what == "the path command" and value not in COMMANDS_DRAWN:
                refused += 1
                print("%s: uses the path command '%s', which the unit's reader of pictures does not know: it can stop the unit at its start" % (path, value))
            elif what != "the path command":
                not_proven += 1
                print("%s: uses %s '%s', which no picture of the unit has: not proven on a unit" % (path, what, value))
    return 1 if refused or (args.strict and not_proven) else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="command", required=True)
    one = sub.add_parser("compile", help="the instructions for a stock picture with a stretch of ours added")
    one.add_argument("--stock", required=True, help="the stock picture")
    one.add_argument("--picture", required=True, help="the picture as the artwork tool made it")
    one.add_argument("--out", required=True)
    one.set_defaults(run=cmd_compile)
    two = sub.add_parser("make", help="the picture a set of instructions makes, as the unit makes it")
    two.add_argument("--instructions", required=True)
    two.add_argument("--display", required=True, help="the folder of stock pictures")
    two.add_argument("--out")
    two.set_defaults(run=cmd_make)
    three = sub.add_parser("check", help="hold that what the instructions carry is ours, and that they make a picture")
    three.add_argument("--instructions", required=True, nargs="+")
    three.add_argument("--display", required=True)
    three.add_argument("--sources", type=int, default=SOURCES, help="the stock pictures the kit holds for one set: 4 (the kit of P2), 8 (from P3 on)")
    three.set_defaults(run=cmd_check)
    four = sub.add_parser("census", help="hold what the instructions use against the unit's own pictures (since R1)")
    four.add_argument("--instructions", required=True, nargs="+")
    four.add_argument("--display", required=True, help="the unit's own pictures: the census is made from them at every run")
    four.add_argument("--sources", type=int, default=8, help="the stock pictures the kit holds for one set: 8 from P3 on")
    four.add_argument("--strict", action="store_true", help="what no picture of the unit has counts as refused too")
    four.add_argument("--name", help="the name the picture is served under, for one set of instructions (default: its file's name less .make)")
    four.set_defaults(run=cmd_census)
    args = ap.parse_args(argv)
    try:
        return args.run(args)
    except ValueError as e:
        print("error: %s" % e)
        return 2


if __name__ == "__main__":
    sys.exit(main())

# The base: what it offers a pack

**The base** is the shared pack every other pack relies on: the file `base.pack`, in the folder `user/pk` of the unit like any pack file. It holds the interface every pack uses, ours and yours alike, and nothing made for one pack: a call every frame, pictures, a page of its own with keys and knobs, folders and files on the unit's disk, and text on its page (levels 1 to 3, on this page), then painting and pages that stack (level 4), a sequencer of a pack's own and the notes heard (level 5), a place in the sound and working memory (level 6), values kept in the unit's own tapes and sounds (level 7), an effect of a pack's own in the unit's lists of effects (level 8), a synth engine of a pack's own in the unit's list of engines (level 9), and the value as the sound plays it (level 10), each on a page of its own. After level 10 come parts with a mark of their own instead of a level: a page that holds its picture or redraws only its changed parts (bases 33 and 36), layers, rows of a pack's own in the unit's lists of effects and LFOs (bases 35 and 37), and a cut the tape has lifted, dropped into the sample of a sound that plays a pack's engine (base 39), and more working memory, from the unit's spare memory (base 41). This page says what it offers, how a pack asks for it, and what it does not offer yet.

The base is a pack like any other ([packs](packs.md)), built from `packs/base.json`. Two things set it apart: **every unit with open-field needs it**, and its release is what the start screen shows. Without `base.pack` no feature runs, and the start screen says `1.7.3 stock (4 need base)`.

| | |
| --- | --- |
| Name | `base` |
| Version | 43 |
| Release | `0.25`: the start screen reads `1.7.3 - open-field 0.25` |
| Level | 10: it offers the marks `op1f_level_1` to `op1f_level_10` |
| Interface number | 3 |
| On the start screen | Not named: its release is shown |
| Its changes of the firmware | Sixty-six: [the change list](changes.md), "The base, `base.pack`" |

Since the base is a file, a new base is a new file copied into `user/pk`, with no install. The **loader**, our small piece inside the firmware file, only reads, places and joins the pack files. It hands the base what it needs and knows nothing of what the base offers.

## The card

A pack that wants something of the base carries a **card**: a small table in its own body that says what it wants. The base reads the card and serves the pack through it. It never needs to know the pack by name, and the pack names no address of the firmware.

You never write a card by hand. The recipe says what goes into it, and `tools/pack_build.py card` writes the card's source from the recipe. This is the card of the example `wink`, in `examples/wink/recipe.json`:

```json
"card": { "level": 1, "frame": "wink_frame", "pick": null, "pictures": true }
```

| Field | What |
| --- | --- |
| `level` | The highest level of the base the pack uses: 1 to 10 (below) |
| `frame` | The pack's function the base calls once a frame, or null |
| `pictures` | true when the base is to serve the pack's pictures (the recipe's `pictures`) |
| `pick` | The pack's function that says which of its pictures a name stands for now, or null |
| `input` | Level 2: the pack's function that is offered every key and knob event, or null |
| `folders` | Level 3: the pack's folders, none to eight ([below](#level-3-folders-files-and-text)) |
| `asks` | Level 4: what the pack asks of the base besides, up to sixteen asks: a sequencer, the notes heard ([level 5](level-5.md)), a sound stage, working memory, pictures that take a place, settings ([level 6](level-6.md)), values kept in the unit's files ([level 7](level-7.md)), an effect ([level 8](level-8.md)), a synth engine ([level 9](level-9.md)) |

The card's source is one of the recipe's sources (`examples/wink/wink_card.s`). It is written again from the recipe by this command, into a new file:

```bash
python -I tools/pack_build.py card --recipe examples/wink/recipe.json --out <the file to write>
```

`tools/pack_build.py check` prints a pack's card. The loader finds the card by the last word of the pack file's header and hands it to the base, with the cards of every pack that runs, in the order of the files' names.

### The levels, and their marks

| Level | What it adds | A card's size | The mark the pack needs |
| --- | --- | --- | --- |
| 1 | A call every frame, pictures for the firmware's screens and of the pack's own, and helpers for a screen's parts and the log | 44 bytes | `op1f_level_1` |
| 2 | A page of the pack's own, with keys and knobs | 48 bytes | `op1f_level_2` |
| 3 | Folders and files on the unit's disk, and text on a page | 56 bytes | `op1f_level_3` |
| 4 | [What a page needs](level-4.md): pages that stack, a blank page to paint, painting, text placed and measured, a file deleted, a key watched, the thread an event came on; and the card's list of asks | 64 bytes | `op1f_level_4` |
| 5 | [A sequencer of the pack's own](level-5.md) in the unit's list, and the notes heard | 64 bytes | `op1f_level_5` |
| 6 | [A place in the sound](level-6.md), working memory, and pictures that take the place of the unit's own by name; from base 31, pictures that [give way](level-6.md#giving-way-base-31) to another pack's; from base 32, the pack's own [settings](level-6.md#settings-base-32) in the unit's settings | 64 bytes | `op1f_level_6` |
| 7 | [Values kept in tapes and sounds](level-7.md), in the unit's own files | 64 bytes | `op1f_level_7` |
| 8 | [An effect of the pack's own](level-8.md) in the unit's lists of effects, for the sounds and the master, and the list of every pack's effects, to run inside another pack | 64 bytes | `op1f_level_8` |
| 9 | [A synth engine of the pack's own](level-9.md) in the unit's list of engines, with sounds of its own in the sound list | 64 bytes | `op1f_level_9` |
| 10 | [The value as the sound plays it](level-10.md), with the LFOs' moves in it, for a page to draw | 64 bytes | `op1f_level_10` |

A **mark** is a name that says what a base can do. It is never called or read. The card's source makes its level's mark a name the pack needs, so nothing is written by hand. Under a base that does not offer the mark, the loader leaves the pack out with a line in the log, `nobody offers the name it needs, op1f_level_3`, and the start screen counts it as left out. Without any `base.pack`, a pack with a card is left out the same way, for its level's mark.

A base offers the marks of every level up to its own: a pack built for an earlier level runs on it as it is. **A card only grows**: a later level adds fields behind the last one, and a base reads the part it knows. From level 4 on the card keeps its size, and a level adds kinds of ask instead. Level 10 adds neither: its mark says the base offers one more helper.

### After level 10: parts with a mark of their own

From base 33 the base grows without a new level. A part adds helpers, a kind of ask or a flag behind what was there, and a mark of its own that a pack needing it names. The level stays 10, and the card keeps its size. Under an older base, a pack that names the mark is left out with the loader's line that names it, as for a level.

| Base | What it adds | The mark |
| --- | --- | --- |
| 33 | [A page that holds its picture](held-page.md): the unit draws nothing of the page in the next frame, and the pack repaints what changed | `op1f_hold_1` |
| 34 | The unit's logs in the disk mode, in a folder `logs` ([methods](methods.md), "The log"). Nothing for a pack to ask | none |
| 35 | [Layers](layers.md): an effect or an LFO kind of the pack's own as a row in the unit's own lists, riding over the unit's own effect or LFO, which plays under it as always. And for every pack's effect, the list opens on its row | `op1f_layer_1` |
| 36 | [A held page whose changed parts are drawn again](held-page.md#redrawing-only-the-parts-that-changed-base-36): the pack names the rectangles that changed and the parts of its picture there | `op1f_parts_1` |
| 37 | [Drum sounds ride an LFO layer](layers.md#an-lfo-layer) too, when it says so | none: `op1f_layer_start` answers for a drum's place |
| 38 | An effect switched off for its cost plays again after a new pick, and the runs of effects inside packs are handed on once no place runs them ([level 8](level-8.md)); no canvas while the base's own top page is up ([level 4](level-4.md)) | none |
| 39 | [A sound's sample](sample.md): a cut the tape has lifted, dropped by the unit into the sample of a synth sound that plays the pack's engine, as its sampler takes one and kept over a restart; and any synth or drum sound's sample read | `op1f_sample_1` |
| 40 | Ready for the loader N2, which takes more pack files: the base reads that loader's numbers and offers it a second, longer list of cards ([below](#base-40-and-the-next-loader)). Under the first loader, N1, a unit runs as on base 39, but for one thing: a folder with too many pack files can still take a change that does not add files. Nothing for a pack to ask | none |
| 41 | [More working memory](level-6.md#working-memory): the unit's spare memory first, then its heap as before; up to 32 MiB an ask and 64 asks in all | `op1f_memory_2`, for a pack that cannot do without these limits |
| 42 | N2's longer list of cards: the base serves up to 128 of them ([below](#base-40-and-the-next-loader)). Under N1 a unit runs as on base 41. Nothing for a pack to ask | none |
| 43 | [A drum engine](drum-engine.md): a pack's engine in the drum's engine list, riding the drum sampler; picked, its page stands on T1 with the keys and knobs offered to it first, and the drum sound's file keeps its name; one pack holds it | `op1f_drum_engine_1` |

A pack with a card needs no entry for the base in its recipe's `works_with`: the mark says it. Faust's four packs in [open-field-packs](https://github.com/LaP0573/open-field-packs) name only each other there (`8 tracks` and `sidechain`, `fx train` and `lfo`), for the names they offer one another.

### Base 40 and the next loader

Base 40 gets ready for the loader N2, the one the install page offers since 2026-11-26. Under the first loader, N1, the door takes what it took before. Under either, a pack has nothing to do.

- **It asks for `op1f_loader_2`**, an optional name. N1 does not offer it, so it reads 0. N2 offers it with the address of a record of six numbers: the record's size in bytes, the loader's generation, the first address of the room for packs, the room's bytes, the largest number of pack files it reads, and the largest number of cards it hands over.
- **The door reads its limits from that record.** It takes the room, its first address and the largest number of pack files from there. With no record it takes N1's: 2,031,616 bytes and 32 files. The door now counts up to 192 files in `user/pk`. New with base 40: a change that does not raise the count of files is never refused for the count, so a folder that already holds too many can still drop files and take a new `base.pack`.
- **`hello 04`** answers with the 68 bytes of `hello 03` and 8 more: the loader's generation (1 for N1, 2 for N2), the largest number of pack files, and the room's bytes. `hello 03` answers as before. A door before base 40 answers `hello 04` with the plain 68 bytes, and the page then takes N1's numbers.
- **It offers `op1f_cards_2`**, a second list of cards with 128 slots. A loader that fills it hands every pack's card over there too, and from base 42 the base serves up to 128 of them (base 40 and 41 serve the first 32). N2 fills it. N1 does not, and there the base serves the cards of the first list, 32 as before.
- **From base 42 the door's list of `user/pk` takes the file to start from**, 0 to 191, so a page can read all 192 files a few at a time. An older door lists the first 64.

### The words a pack includes

`module/build_r6e_card.h` is the interface's own text: the card, what a frame function and an input function are handed, every kind of ask and every helper of levels 1 to 10 and of the parts after them, with the thread each may be called from. A pack includes it. It names no address of the firmware. Where this page and the header differ, the header is right.

## The examples, one or two for each level

| Example | Level | What it shows | How to see it |
| --- | --- | --- | --- |
| [`wink`](../examples/wink/README.md) | 1 | A call every frame and one picture: a red dot of ours on the tempo screen, blinking | Press the metronome key |
| [`yoyo`](../examples/yoyo/README.md) | 2 | A page of its own, with keys and a knob | Shift and the metronome key; the blue knob moves the yoyo |
| [`list`](../examples/list/README.md) | 3 | One folder of its own, shown in the disk mode, and words on its page | Shift and the help key; the blue knob scrolls |
| [`sketch`](../examples/sketch/README.md) | 4 | A drawing board: a page painted at every frame, a text centred and measured, a file deleted | Shift and the metronome key; the blue and second knobs draw |
| [`echo`](../examples/echo/README.md) | 5 | A sequencer in the unit's own list, which repeats every key in time | Pick `echo` in the sequencer list and switch it on |
| [`dub`](../examples/dub/README.md) | 6 and 7 | An echo on everything the unit plays, in a megabyte of working memory, its values kept with each tape | Shift and the metronome key; the third knob brings the echo in |
| [`crush`](../examples/crush/README.md) | 8 and 10 | An effect in the unit's own lists that makes a sound rougher, standing on `punch` | Shift and T3 on a sound, pick `crush`. T3 shows its page |
| [`ping`](../examples/ping/README.md) | 8 and 10 | An echo on the beat that bounces left and right, standing on `delay`, its tail ringing out when switched off | The master effect's list on the mixer, pick `ping`. T3 shows its page |
| [`pluck`](../examples/pluck/README.md) | 9 and 10 | A plucked string as a synth engine, eight values, memory for each voice, two sounds of its own | Shift and T1 on a synth sound, pick `pluck`. T1 shows its page |
| [`fold`](../examples/fold/README.md) | 9 and 10 | A wave folder as a synth engine that renders two channels, two sounds of its own | Shift and T1 on a synth sound, pick `fold`. T1 shows its page |

Shift and the metronome key open `dub`, `sketch` and `yoyo` alike: the one whose file sorts first gets them, so keep one of the three on a unit at a time. The four examples of levels 8 and 9 take no key: you meet them in the unit's own lists, and their pages draw their values as the sound plays them (level 10).

Each changes no place of the firmware and names no address of it. `python -I tools/build_all.py` builds them into `out/examples/`, and each folder's page gives the command that builds it alone. [A first pack](first-pack.md) goes through `wink` step by step.

## Level 1: a call every frame

The display thread draws a frame some sixty times a second. Before each frame is shown, the base's frame hook runs. It calls **the frame function of each card, in the order of the packs' file names**, the four packs of open-field-packs among them, and then dresses its own screens, so that they show what the frame's work has made.

A frame function is handed what the frame is, and answers nothing:

```cpp
extern "C" void wink_frame(const Op1fFrame* now) OP1F_NAMED("wink_frame");
```

| Field | What |
| --- | --- |
| `bytes` | The size of what is handed: 28 at level 1, 32 at level 2, 36 at level 3, 40 from level 4. A later base may hand more: read what you know |
| `frame` | The number of the frame, counted by the base |
| `ms` | Milliseconds since the base's first frame. It moves once a frame |
| `mode` | The mode the unit is in: 0 synth, 1 drum, 2 tape, 3 mixer. A pack with a sequencer also sees 7 while the sequencer's screen is up ([level 5](level-5.md)) |
| `screen` | The number of the screen that is shown. The tempo screen is 42 |
| `shift` | Not 0 while shift is held |
| `popup` | Not 0 while the power popup is on the screen: the last moment to say something in the log |
| `page` | Level 2: the card whose page is shown, or 0 ([below](#level-2-a-page-with-keys-and-knobs)) |
| `disk` | Level 3: not 0 while the disk mode is open ([below](#the-disk-mode)) |
| `canvas` | Level 4: this frame's picture, for the painting helpers ([level 4](level-4.md), "Painting"). From base 38, 0 while the base's own top page is asked for or up |

What a frame function must keep to:

- **It comes back quickly, every time.** The picture stands still for as long as it runs. All of our own frame work together takes half a millisecond to a millisecond.
- **A frame function that never comes back stops the display for good**, and no line names the pack. The help key at power-on is then the way out ([packs](packs.md), "The safe start").
- The base times every call and prints the time by the pack's name, over periods of 256 frames: `cards: 'wink' frame call: 256 calls, 2216 us in all, 1 least, 585 most`. A slow pack is named, not switched off. What is timed includes what a thread of higher priority took meanwhile.
- It may write into the log, and it is the place to say what the other threads only counted.
- A function written in assembly keeps the registers the calling convention says a function keeps ([methods](methods.md), section 7).

## Level 1: pictures

A screen of the firmware loads its picture by a file name. The base stands at the entry of that loader. For a name a pack carries a picture for, the picture comes from the pack; every other name goes on in the firmware's own loader. That is how a pack changes a screen of the device, and how a page of its own gets its picture.

**Who serves a name.** The base asks the cards in the order of the packs' file names, each pack's `pick` first. A card of level 6 or later serves a screen of the firmware only the names it lists ([level 6](level-6.md), "Pictures that take a place"): a picture of its own named like one of the unit's stays its own. The first pack that has the name serves it, unless it gives way: from base 31 a pack can list names it serves only when no other pack does ([level 6](level-6.md), "Giving way"). So a recolour of the signal flow screen can sit next to a pack whose working page lives on that screen, and neither names the other. Where two packs both hold on to one name, the first by file name serves it, and one line in the log says so.

**A pack's own picture**, for a page of its own, has a name the firmware never asks for. `op1f_picture(card, name)` looks for it in that pack's directory alone, and makes it into a **document**: the picture as the firmware holds it once it is read, with parts that can be found by their names.

**`pick`**, when the card names one, is asked before the pack's directory is looked into: handed the name a screen asks for, it answers the name to look for, or 0 for nothing of this pack's now. It is asked every time the firmware asks for a picture: while the firmware makes its screens at the start, and later, when a tape is loaded or a page of the base is opened. A later call is not always on the display thread. So `pick` only reads, shares nothing but single words, and takes no time.

**A pack never carries a picture of the device, or one made from it.** It carries *instructions*, and the base puts the picture together on the unit, from the unit's own picture files, each time the screen is made. There are two kinds, told by their first line.

*The first kind* is one of the unit's pictures with something of ours added before its last tag. This is `examples/wink/tempo.svg.make` whole:

```
op1f picture 1
size 44094
from tempo.svg 43996 eb050950
add end 98
<circle id="wink_dot" fill="#E52817" stroke="#E52817" stroke-width="2" cx="228.8" cy="30" r="3"/>
```

The unit's own tempo screen, 43,996 bytes long with that CRC-32, and one red dot of ours, 98 bytes, named `wink_dot`.

*The second kind* is a picture put together from stretches: stretches of the unit's pictures as they are, the same moved or scaled, and text of ours, one behind the other.

| Line | What it does |
| --- | --- |
| `op1f picture 2` | The first line |
| `size N` | The size of the picture that is made |
| `from NAME BYTES CRC` | One of the unit's own pictures, by its file name, its length and its CRC-32. Up to eight, numbered from 0 |
| `c S OFFSET LENGTH` | LENGTH bytes of picture S from OFFSET, as they are |
| `m S OFFSET LENGTH SCALE DX DY` | The same stretch with its geometry moved and scaled: x' = SCALE/1000 · x + DX/1000, and the same for y |
| `a BYTES` | The BYTES bytes behind this line are ours |

`examples/yoyo/yoyo.svg.make` and `examples/list/list.svg.make` are of this kind: the head and the last tag of the unit's own tempo screen, by reference, around a drawing of ours.

A `from` line names the unit's picture by its length and checksum. A unit whose picture is another one gets no picture put together from the wrong parts: the screen keeps its own picture, a page of a pack's own is not made, and the log says why.

`tools/picture_make.py` is the same maker in Python. It says what a set of instructions holds and makes the picture on your computer, from a copy of your own unit's picture folder (`content/display` on its disk, kept outside the repository):

```bash
python -I tools/picture_make.py check --instructions examples/list/list.svg.make --display <a copy of your unit's content/display>
```

**What a picture may hold.** The firmware's reader of pictures knows only what its own pictures use. Anything else can hang the unit at the start, with no message.

- Path commands: `M L H V C S` and `z`, in capitals or small letters. **No arcs.**
- No element, attribute or attribute value that none of the unit's own pictures has. There is no `display` attribute: a part is hidden by the code, with `op1f_show`.
- Letters are not text: a text that never changes is a row of shapes, and one that changes is set by `op1f_text` ([below](#text)).
- Colours from the unit's own palette file, `content/display/palette.svg`, so that a page looks like the device's.
- No picture larger than a megabyte: the base does not use a card whose directory holds one.
- No `_x` in a part's name (`id`), unless it begins a character written by its code as the unit's own pictures write one: `_x2D_` for a `-` passes. `knob_x`, `knob_x1` and `my_xml_page` stop the unit at its start, with a message on the screen.
- No number with more than four digits behind its point: `1.23456` stops the unit at its start, `1.2345` passes.
- A `matrix` has six numbers: with fewer, it stops the unit at its start. A `translate` or a `scale` has two, with a comma or a space between them: `translate(10)` stops the unit at its start.

`tools/picture_make.py check` refuses a picture with one of the last three, and names the part, the number or the transform.

The build makes a pack's directory of pictures from the `pictures` of its recipe, as a source of the pack (`examples/wink/wink_pictures.s`), into a new file:

```bash
python -I tools/pack_build.py pictures --recipe examples/wink/recipe.json --out <the file to write>
```

## Level 1: the helpers

Each is a name the pack needs once it calls it, and the loader joins it to the base's function. Every name sorts behind the marks, so a pack whose base is too old is left out for the mark, which says what is missing.

| Helper | What it does | From where |
| --- | --- | --- |
| `op1f_log(level, file, line, format, ...)` | A line in the unit's log. `level` 1 an error, 2 a warning, 3 information. The log shows the name in `file` between its last slash and its dot as the line's tag | A frame function, never an input function |
| `op1f_ms()` | Milliseconds since the base's first frame | Any thread |
| `op1f_screen(number)` | The document of one of the firmware's screens, 1 to 85. 0 while that screen has no document yet: ask again at a later frame | The display thread |
| `op1f_node(document, name)` | A part of a document, by its `id` in the picture, or 0 | The display thread |
| `op1f_show(part, on)` | A part shown or hidden, with everything inside it | The display thread |
| `op1f_move(part, matrix)` | A part moved, turned or scaled. `matrix` is nine whole numbers in 1024ths: `[1024, 0, 0, 0, 1024, 0, 1024 x, 1024 y, 1024]` moves it x and y pixels | The display thread |
| `op1f_picture(card, name)` | One of the pack's own pictures made into a document, or 0 | The display thread |

**Ask once for a picture and keep the answer**, even when it is 0. Each call reads the pack's file and takes memory, and a picture that cannot be made is an error line at every call. The base keeps the first eight documents a card made: those are the ones a page and a text take.

A line is cut at about 260 characters with the logger's own prefix, and a line every frame fills the log. Say a count when it has changed, not every time.

## Level 2: a page with keys and knobs

Only one page is on the screen at a time, so the base lends the one it has, screen 77, to every pack in turn, our effect train among them. A pack never names a screen's number. Level 4 lets pages stack, one over another ([level 4](level-4.md), "Pages that stack").

A pack makes a document from a picture of its own (`op1f_picture`), finds its parts by their names (`op1f_node`), and opens its page:

| Helper | What it does | From where |
| --- | --- | --- |
| `op1f_page_open(card, document)` | The pack's page opened in the mode the unit is in, with that document, in place of whatever the mode would show. Answers not 0 when the base took the request | The frame function or the input function |
| `op1f_page_close(card)` | The pack's page closed, when it is the one open. The mode's own screen comes back | The frame function or the input function |

- The base takes a request at its next frame, and the page shows from the frame after. From then on the frame's `page` is the pack's card: **compare it with your card's address to know your page shows.**
- The page is closed by the pack, **when the mode changes**, and when another pack opens its page: the page opened last is the one shown.
- A request can be dropped before the page ever shows, with no line: when the mode changes before the base's next frame. `page` is what says the page shows.
- **Open a page on a key**, not at every frame: a page opened again at every frame closes every other pack's page as soon as it opens.
- `document` must be one `op1f_picture` made for this card; the base answers 0 for any other. The document stays the pack's: the base never frees it, and the pack may dress it at every frame, shown or not.
- A pack with no input function may open a page from its frame function. The keys then go on to the firmware as they always did.

### Keys and knobs

A card's input function is offered every key and knob event, **before the firmware's own code for the mode**:

```cpp
extern "C" int yoyo_input(const Op1fInput* e) OP1F_NAMED("yoyo_input");
```

It answers not 0 to take the event: the firmware then never sees it, and no pack after this one is asked. It answers 0 to let the event go on.

**The order.** An event is offered first to the pack whose page is shown, then to every card that has an input function, in the order of the packs' file names, until one takes it. An event nobody takes goes on as it always did.

**An up goes where its down went.** A pack is handed the up of every key whose down it took, whatever it answers, and the up of no other key. An up whose down went to the firmware goes to the firmware.

| Field | What |
| --- | --- |
| `bytes` | The size of what is handed: 36 at level 2 |
| `what` | `OP1F_DOWN`, `OP1F_DOWN_SHIFT` (a key went down while shift was held), `OP1F_UP` or `OP1F_TURN` |
| `key` | The key's number, or for a turn the knob: 0 blue, 1 the second, 2 the third, 3 red |
| `turn` | For a turn: 1024 a step of the knob, more than 0 the way that makes a value go up |
| `mode`, `screen`, `shift` | As a frame function is handed them |
| `mine` | Not 0 when this pack's page is the one shown |
| `record` | The firmware's own record of a key that went down: sixteen bytes, the key's number, the value (127, or how hard a key of the keyboard was struck), and two words of the clock at the press. 0 for an up and a turn. It lives only for the call: copy what you need, never keep the pointer |

| Key numbers | Keys |
| --- | --- |
| 0 | The help key |
| 1 | The metronome key |
| 2 to 5 | The synth, drum, tape and mixer keys |
| 6 to 9 | The four keys under the screen, T1 to T4 |
| 10 | The lift key |
| 11 | The drop key |
| 38 | Shift |
| 54 to 57 | A knob's click: the firmware makes one, a down and then an up, when a knob that was pressed comes up without having been turned |
| 61 to 64 | A knob pressed down and let go |

**A pack picks its own keys, anywhere**: a combination in a mode, or a key on another screen, another pack's page among them. Nobody keeps a list of them. Where two packs pick the same keys, **the pack of the earlier file gets them**. Our own features stand in the mode's own code, so a pack that takes one of their keys has it first:

| Keys | Whose |
| --- | --- |
| The tape key held, with T1 to T4; the mixer key held | `8 tracks` |
| Shift and the mixer key | `sidechain` |
| T3 and the knobs on the page of a synth or drum sound that rides `fx train`'s row `train` | `fx train` (from version 18; shift and T3 stay the unit's) |
| T4 and the knobs on the page of a synth or drum sound that rides `lfo`'s row `ufo` | `lfo` (from version 12; shift and T4 stay the unit's) |
| Shift and the metronome key | the examples `dub`, `sketch` and `yoyo`, the earliest file of them |
| Shift and the help key | the example `list` |

**While a pack's page is shown, every key and knob it does not take still goes to the mode's own code**, which acts on the screen the page hides: in the mixer the other knobs move levels nobody sees, and over the effect train they move effects one hears. A pack that wants its page to own the knobs takes all four while its page is shown. The examples do the opposite: they take only the keys and the knob they use.

### The threads

- **A frame function runs on the display thread.** It may write into the log, and at level 3 read and write files.
- **An input function runs on the thread that handles keys, never on the display thread.** For the keys of the keyboard, the ones that play notes, the firmware may hand a key in at once on the thread that brought it, **which can be the audio thread**. So an input function does what the audio thread may do: it writes no line in the log, it waits for nothing, it calls nothing of the firmware's, and it comes back quickly. It counts, and the frame function says it. It may call `op1f_page_open` and `op1f_page_close`, and from base 39, for an event on the key thread, `op1f_sample_drop`, which asks the unit for a drop and waits for nothing ([your engine's sample](sample.md)).
- The two share single words, never a structure: either can find the other halfway through its work. `examples/yoyo/yoyo.cpp` shows the form: a few words the input function writes and the frame function reads.

## Level 3: folders, files and text

### Folders: none, one or several

**Most packs need no folder.** A pack whose card lists none gets none, and nothing of the unit's disk is touched for it. A pack that keeps something, its settings or files the computer brings, lists its folders in its card: **none, one or several, up to eight**. Each folder has

| | What |
| --- | --- |
| `name` | What the computer shows in the disk mode, beside `synth`, `drum`, `tapes`, `mixdown`, `packs`, `logs` and the folders of the other packs (our `fx train` shows `fx`, our `lfo` shows `lfo`) |
| `place` | Where the folder lies on the unit's disk: `user/pf/` and the place, a folder of its own |
| `shown` | Whether the computer sees it. A folder that is not shown is the pack's private folder, for its settings |

The recipe may leave out what follows from the rest: a folder's `name` is the pack's own name when the pack has one folder and does not name it; its `place` is its name; it is `shown` unless the recipe says false. A pack names a folder to the helpers by its number in the list, from 0.

**No folder.** A card of level 1 or 2 has none. A card of level 3 that uses the text alone leaves `folders` out, or gives an empty list:

```json
"card": { "level": 3, "frame": "clock_frame", "pick": null, "pictures": true, "input": "clock_input", "folders": [] }
```

**One folder**, as the example `list` has it. `{}` takes everything from the pack's name: the folder `list`, at `user/pf/list/`, shown in the disk mode.

```json
"folders": [ {} ]
```

**Several folders.** Each names itself. Here one is shown to the computer and one is the pack's alone:

```json
"folders": [ { "name": "grooves", "place": "grooves" }, { "name": "settings", "place": "groove-settings", "shown": false } ]
```

That pack has `user/pf/grooves/`, which the computer sees as `grooves`, and `user/pf/groove-settings/`, which it does not see. Folder 0 and folder 1 to the helpers.

**What a name and a place may be.** One to fifteen characters of `A`-`Z`, `a`-`z`, `0`-`9`, space, `_`, `-` and `.`, not beginning with a space, a dot or a dash and not ending with a space or a dot. The tool refuses any other, and the base does not use one.

**Who keeps a name or a place that two folders want.** The earlier folder keeps it: in the order of the packs' file names, and then of each pack's list. Capitals count as small letters, since the computer's disk cannot keep two names that differ only in capitals.

- **A name** that is the firmware's (`synth`, `drum`, `tapes`, `mixdown`), the loader's (`packs`), the base's (`logs`) or an earlier shown folder's: the folder is not shown, and its line in the log says so. The pack still has it on the disk.
- **A place** an earlier folder has: the folder is not the pack's. Its line says so, and the helpers answer `OP1F_NO_FOLDER` for it.

Within one pack, the tool refuses both at build time.

**When the folders are made.** At the base's first frame, before any frame function is called, the base makes each folder the pack keeps (and `user/pf/` with it) and says one line a folder:

```
folders: 'list' folder 1 of 1, 'list' at user/pf/list/: shown in the disk mode, already on the disk
folders: 1 folder(s) of 1 card(s): 1 shown in the disk mode, 0 the pack's alone, 0 not the pack's, 0 more than the base holds
```

A folder stays on the disk when its pack is taken off the unit. It is read by nothing, and it can be deleted from the computer in the bootloader's drive mode.

### The disk mode

This is how an owner meets a pack's folder. The disk mode is the one used to copy samples: the cable in, the COM key, and the computer shows the unit as a drive.

- **The computer sees each shown folder at the top**, beside the firmware's folders, the loader's `packs`, the other packs' shown folders and, since base 34, the base's own `logs`, which holds the unit's log files ([methods](methods.md), "The log").
- **The files of the folder, one level of them**: every file whose name is one to thirty-nine plain characters and does not begin with a dot. Folders inside a pack's folder are not shown. The first 64 files are shown.
- **The computer may read a file, bring a new one, and delete one.** A delete takes the file off the disk. A file the computer brings needs a name the folder shows; another is refused. A copy the computer gives up halfway leaves an empty file, which can be deleted from the computer.
- **The firmware builds what the computer sees once, when the disk mode starts.** A file that comes to the folder afterwards is seen at the next disk mode.

While the disk mode is open, the frame's `disk` is not 0: **the computer owns the folders**. A write is refused (`OP1F_NOT_NOW`) and a read sees what the disk holds. When `disk` goes back to 0, the computer may have brought, changed or deleted files. A pack that keeps what a file says reads it again then.

The base says what the computer was shown, and each file it brings, in the log:

```
folders: the disk mode shows 'list' with 1 of the 1 file(s) of user/pf/list/ (listed); 0 more, 0 folder(s) inside and 0 other name(s) not shown
folders: the computer brings a new file, user/pf/list/note 01.txt: a place is made for it
```

### Files

A pack's file is bytes: the base reads and writes them as they are. A pack names a file by its folder's number and its name, never by a path.

| Helper | What it does |
| --- | --- |
| `op1f_list(card, folder, names, room)` | The names of the files in the folder, in the order the disk lists them: each name and a zero behind it, one after another, as many as fit whole in `room`. The names are those the disk mode shows: folders inside, names that begin with a dot and names of more than thirty-nine characters are left out. Answers how many names it wrote |
| `op1f_read(card, folder, name, buffer, room)` | A file read whole into `buffer`. Answers the bytes read, 0 for an empty file |
| `op1f_write(card, folder, name, data, bytes)` | `bytes` bytes written as the file, in place of what it held, or as a new file. Answers `bytes` when the file landed. At most a megabyte |
| `op1f_path(card, folder, name, out, room)` | The path on the unit's disk of a file of the folder (`user/pf/list/list.txt`), or of the folder itself when `name` is 0, with its end. For a line in the log or a page |

**The answers.** When a helper does not do what was asked, it answers less than 0:

| Answer | When |
| --- | --- |
| `OP1F_NO_FILE` (-1) | `op1f_read`: the folder holds no file of that name. No line is said |
| `OP1F_TOO_LARGE` (-2) | `op1f_read`: the file has more than `room` bytes, and nothing is read. `op1f_path`: `room` is too small for the path |
| `OP1F_NOT_NOW` (-3) | The call did not come from this pack's frame function; or, for `op1f_write`, the disk mode is open |
| `OP1F_NO_FOLDER` (-4) | The card is not one the base has in use, or the folder's number is not one of its folders the base made |
| `OP1F_BAD_NAME` (-5) | A file's name is empty, longer than thirty-nine characters, holds a slash or a character that is not plain, or begins with a dot |
| `OP1F_FAILED` (-6) | The disk did not do it. The line in the log says which step |

**From the pack's own frame function only.** `op1f_list`, `op1f_read` and `op1f_write` answer `OP1F_NOT_NOW` to a call that does not come from the frame function of the pack that asks, on the display thread, while the base calls it. So nothing of the disk is touched from the thread that handles keys or from the audio thread. An input function that wants a file read or written sets a word, and the frame function does it. `op1f_path` may be called from any thread, once the base's first frame has made the folders.

**A write goes to a file of the base's in `tmp/` first**, and is then renamed over the pack's file, as the firmware writes its own files. So a file is the old one or the new one, never half of each. It is held by the file's size afterwards. Every write that fails, and the first ones that land, are said in the log by the pack's name:

```
files: 'list' wrote user/pf/list/list.txt, 45 bytes: it landed (1 written since the start)
```

**Read once, and write when something changed.** A file is read or written while the frame function waits, so the picture stands still for as long as it takes: some milliseconds for a small file. Read a file at the first frame, or when the page opens, and keep what it says. Write it when something changed, not at every frame. `examples/list/list.cpp` reads its folder when its page opens and when the disk mode closes under it, and writes `list.txt` when the page closes, only when the names changed, and never while `disk` is not 0: it waits for the disk mode to close.

### Text

`op1f_text(document, part, text, width)` writes a text into a part of a document `op1f_picture` made, in the firmware's own letters, as the firmware's own lists show theirs.

- **The part is a placeholder** in the pack's picture: a group (`<g id="...">`) holding a path with a stroke. The top-left of the path is where the text begins, and the letters take the stroke's colour. In `examples/list/list.svg.make` each row is a group with one upright stroke of ours, fifteen pixels tall:

```
<g id="list_row1"><path fill="none" stroke="#CDD0C2" stroke-width="1.3" stroke-linecap="square" d="M56,61V46"/></g>
```

- The part becomes a text at its first use and stays one. The text is set again only when it changed.
- `width`: a text wider than so many pixels is shortened, with a mark at its end. 0 for the screen's width. An empty text shows nothing.
- **Small letters only.** The firmware's font has no capitals of its own: a capital is shown as its small letter. A byte that is not a plain character, below a space or above `~`, is shown as `?`. A text read from a file can hold such bytes.
- It answers 1 when the text is in place, 0 when it is not: no such document or part, a part that is no group with a stroke, a part's name longer than 23 characters, the base's room for texts full (512 for all packs, 256 for one), or a call that did not come from a frame function. The first refusals are said in the log.
- Level 4 sets a text centred or to the right, measures it and moves it ([level 4](level-4.md), "Text, placed and measured").
- **From a frame function only**, on the display thread, where the document is drawn.

## The lines the base writes into the log

The base's lines tell how a pack fares on a unit. Each line of a log begins with a time, a letter for its level and a tag.

| Tag | Line begins | What it says |
| --- | --- | --- |
| `op1f_cards` | `cards:` | The cards the loader handed over, each card's level and what it asks for; a picture two packs carry, and which serves it; then, every 256 frames through a gate, each frame function's time |
| `op1f_cards` | `page:` | A page opened, with the mode, and closed, with why; from base 33, a page that holds its picture ([held pages](held-page.md#the-lines-the-base-writes)) |
| `op1f_cards` | `input:` | The keys and turns that came, and how many packs took; at each change of screen, the last key down and up |
| `op1f_folders` | `folders:` | Each folder's place and whether it is shown; what the disk mode shows; each file the computer brings or deletes |
| `op1f_folders` | `files:` | A write that landed, a write or read that failed, a call that was refused |
| `op1f_text` | `text:` | A text made of a part, or refused |
| `op1f_cards` | `paint:` | Level 4: a pack's paint calls that were refused |
| `op1f_seq` | `seq:` | Level 5: the packs' sequencers in the unit's list, the choice of each instrument, each sequencer's period |
| `op1f_notes` | `notes:` | Level 5: the packs that hear the notes |
| `op1f_stages` | `stage:` | Level 6: each sound stage, its cost, and why one was left aside or switched off |
| `op1f_memory` | `memory:` | Level 6: the working memory given, or refused |
| `op1f_options` | `options:` | Level 6, from base 32: the packs with settings, how many values were read from the file, where each pack's settings show, a pack whose category was full, each write of the file, a pack's own settings page opened |
| `op1f_keeps` | `keeps:` | Level 7: the values kept in the unit's files, written and read |
| `op1f_effects` | `effect:` | Level 8: the packs' effects in the unit's lists, which place plays which, their memory and their period |
| `op1f_lfos` | `lfo:` | Base 35: the packs' LFO layers in the unit's lists, which place rides which ([layers](layers.md#the-lines-the-base-writes)) |
| `op1f_engines` | `engine:` | Level 9: the packs' engines in the unit's list, which sound plays which, their memory, their period, their sounds and the unit's count of user sounds |
| `op1f_sample` | `sample:` | Base 39: each drop a pack asked for, its end with the frames put in, a refusal and why, and the count of the unit's own drops into its sampler ([your engine's sample](sample.md#the-lines-the-base-writes)) |
| `op1f_sample_restore` | `sample:` | Base 39: after a start, each cut put back into a sound that plays a pack's engine, or why not |
| `op1f_drum` | `drum:` | Base 43: the drum engine in the drum's engine list, which drum sound rides it, and when a ride ends ([a drum engine](drum-engine.md#the-lines-the-base-writes)) |
| `op1f_tape_art` | `artwork` | A picture put together on the unit, and from what |
| | `aud` | Every sixteenth second: the share of the processor the audio thread took, the longest block, and the blocks a second |
| | `os heap:` | Every sixteenth second: how much of the firmware's own heap is free at its top, and the least it has been |
| | `frame cost:` | When another screen is shown, and now and then: what a frame costs, split into the firmware's work before the draw, the firmware's draw and our own frame work |

These are lines from a unit with the three examples on it:

```
cards: 3 card(s) from the loader, 3 in use, 0 not used, 0 more than the list holds
cards: 'list' version 1, level 3: a frame call yes, 1 picture(s), a choice among them no, an input function yes
artwork list.svg: put together on the unit, 1694 bytes: 378 from 1 of the unit's own picture(s) and 1316 from user/pk/list.pack, in 2706450 cycles
page: 'list' opened in mode 4, its document at 0x8cb5b0c8 under screen 77, the train's
text: 'list' made a text of the part list_title of its picture
page: 'list' closed: the pack closed it; 1 opened since the start
input: 'list' took 3 down, 3 up, 0 turns since the start
```

A pack that makes the audio thread slower shows in `aud`, a page that draws slowly in `frame cost:`. `module/build_q_quiet.h` is a small gate for a pack's lines that come every few seconds: they are said while something changes, and rarely when nothing does. The three examples use it.

## The four packs of open-field-packs

`8 tracks`, `fx train`, `lfo` and `sidechain` are packs with cards like any other, in a repository of their own like any other, [open-field-packs](https://github.com/LaP0573/open-field-packs): read them for how a bigger pack uses what this page describes. They use only what any pack may use: their frame functions, their pictures, their folders (`fx train`'s presets are in its folder `fx`, `lfo`'s in `lfo`), their values in the tapes' and the sounds' files (level 7), and a page that stays with its mode. From version 12, `lfo` is an LFO layer: its row `ufo` in the unit's own list of LFOs, for synth and drum sounds ([layers](layers.md)). From version 18, `fx train` is an effect layer: its row `train` in the unit's own lists of effects, for synth and drum sounds. `fx train` offers every effect the packs registered on each of its wagons, through level 8's list, as any pack may. The base holds nothing made for one of them and knows none of them by name. What the base does not offer, each still does through its own table of changes, made from the `profile.json` in its folder there.

## What the base does not offer yet

- **A sound stage before an instrument's effect, or on one track of the tape.** Level 6 offers three points: an instrument after its effect, and the master before and after its effect.
- **An engine for the drum.** Level 9's engines are the synth's.
- **More than four values on an effect's knobs, or eight on an engine's.** Anything more is a value of level 7, set from the pack's own page.
- **Folders inside a pack's folder, in the disk mode.** A pack may make them on the disk itself; the disk mode does not show them, and `op1f_list` leaves them out.
- **A write from an input function.** It is refused: the frame function writes.
- **A value of the whole unit** in the unit's own settings file. From base 32 a pack's settings show in the unit's settings and the base keeps them in a file of its own ([level 6](level-6.md#settings-base-32)); anything else a pack keeps in its own folder (level 3).
- **A place in the firmware.** A pack that has to stand in the firmware's own code changes it through its own table of changes, as the example `trampolines` does ([a first pack](first-pack.md), "A pack that changes the firmware"). Two packs cannot change the same place: the one whose file sorts later is left out, and when that is `base.pack`, every pack that needs the base goes with it ([level 6](level-6.md), "Two packs at one place of the firmware").

A pack that needs something new of the base comes with a change to the base ([below](#a-change-to-the-base)). Open an issue before you start on one ([CONTRIBUTING.md](../CONTRIBUTING.md)).

## A change to the base

The base is a pack: a change to it is a new `base.pack`, built from `packs/base.json` and the sources it names, and copied onto the unit. No install.

1. **The sources** are the ones `packs/base.json` names: the cards, the pages, keys and knobs, folders and files, text and painting, sequencers, notes, stages, memory, the values kept, effects, engines, each in a unit of its own. The interface's text is the header the examples include.
2. **A new level** is a new header beside the last one and a new mark, `op1f_level_11` say, which the base offers and a card of that level needs. From level 4 on, a level adds kinds of ask and helpers, and the card keeps its size. Every field, every kind and every name of the levels before stays where it is, with its meaning: a pack built for an earlier level runs on the new base as it is.
3. **`packs/base.json`**: its `version` and `release` stay as they are. They belong to the release of the base, the maintainer's step, which gives the base its next version once your change is merged ([building](build.md#a-change-to-the-base-and-its-release)). A new `interface` number only when a name other packs use changes its meaning; then each of them is built again naming the new number.
4. **Build.** The build says `ahead` for `base.pack`: your sources are ahead of the released base, and that is right. Leave `builder/base.pack`, `builder/base-pack.json`, `catalog/packs/base.json` and the entry of `base.pack` in `packs/built.json` as they are: the checks on a pull request refuse a change to them.
5. **A try on a unit**: copy the new `base.pack` from `out/packs/` over the old one, start, read the log.

A change to the base that needs something new of the loader is rarer: it is a change to the loader, and an install ([building](build.md), "A change to the loader"). The base then needs the loader's next mark, and under an older loader it is left out with a line that names the mark.

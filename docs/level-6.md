# Level 6: a place in the sound, working memory, and pictures that take a place

Level 6 lets your pack work on the sound itself, without changing the firmware. You hand over a function, and the base calls it once a block with the sound at the point you picked. Your pack can also ask for a block of memory, far more than fits in its own file, and say exactly which of the unit's own pictures its pictures replace.

The words of the interface are in the header `module/build_r6e_card.h`. Where this page and the header disagree, the header is right.

## The example: `dub`

[`dub`](../examples/dub/README.md) puts an echo on everything the unit plays: every note, the tape, all of it, in time with the tempo. Hold **shift** and press the **metronome** key, in any mode, to open its page. The **blue** knob sets how far behind the echo comes (1 to 8 sixteenth notes), the **second** knob how much of it comes back again, and the **third** knob how loud it is. While the page is up `dub` takes all four knobs, so nothing on the screen under it moves without you seeing it.

`dub` starts with the level at 0, so a unit that gets the pack sounds exactly as before until you turn the third knob. The code quoted below is `dub`'s own.

The examples `sketch` and `yoyo` open with the same keys. `dub`'s file sorts first, so while it's on your unit they have no way in.

## The three kinds of ask

| Kind | In the recipe | What the ask's table is |
| --- | --- | --- |
| A sound stage | `"kind": "stage"` | an `Op1fStage` |
| Working memory | `"kind": "memory"` | an `Op1fMemory` |
| Pictures that take a place | `"kind": "places"` | an `Op1fPlaces` |

From base 32 there's a fourth kind, `"kind": "options"`: settings of your pack's own in the unit's settings ([below](#settings-base-32)).

Each table starts with the head every kind shares ([level 5](level-5.md), "Asks"). Its `name` is what the base's lines in the log call the ask. None of the three has a page, so `shown` is always 0.

A card that uses one of them says level 6, and its source makes `op1f_level_6` a name your pack needs. On an older base the loader leaves your pack out and says why. As with every kind, the pack tool writes the tables from the recipe and refuses what the base would leave aside.

These are the two asks of `dub`'s card that level 6 serves. Its third ask, the values it keeps with each tape, is [level 7](level-7.md)'s.

```json
"asks": [
  { "kind": "memory", "name": "dub", "bytes": 1048576, "align": 32 },
  { "kind": "stage", "name": "dub", "point": "master_out", "cycles": 40000,
    "listens": false, "memory": "dub", "process": "dub_process" }
]
```

## A sound stage

**The points.** A stage stands at one of three points of the sound:

| `"point"` | In the header | What the stage gets |
| --- | --- | --- |
| `instrument` | `OP1F_AT_INSTRUMENT` | An instrument's sound after its own effect. The synth and the drum each come in a call of their own, and `host` says which (0 the synth, 1 the drum) |
| `master_in` | `OP1F_AT_MASTER_IN` | The unit's whole sum, before the master effect |
| `master_out` | `OP1F_AT_MASTER_OUT` | The unit's whole sum, after the master effect: what you hear |

At every point the unit's own code still works on the sound after the stages, as it did before them.

**What your function gets.** `process` is called once a block of 64 frames, at 44,100 frames a second, so about 690 times a second. It's handed an `Op1fBlock`:

| Field | What it is |
| --- | --- |
| `point`, `host` | Where the call comes from. `host` is 2 at both master points |
| `frames` | 64 |
| `left`, `right` | The two buffers, `frames` words each. Each word is a signed 32-bit sample, a Q1.31 fraction: full scale is -2^31 to 2^31 - 1, not 24 bits in a 32-bit word. A stage that changes the sound writes into them, in place |
| `period` | The length of a 96th of a beat at the unit's tempo, in frames, times 2^18: about 60,211,200 at 120 beats a minute. It's a big number, and times 96 it no longer fits in 32 bits. A beat is `(period × 3) >> 13` frames (fine above 5 beats a minute), a sixteenth note `((period >> 6) × 24) >> 12`. No division needed |
| `playing` | Not 0 while the tape plays |
| `block` | How many times this stage was called before at this point and host |

The block lives for the call only. Never keep it, or the buffers' addresses.

**Several packs at one point.** Any number of packs may stand at a point. The stages that change the sound come first, in the order of the files' names and then of each card's asks. Then come the stages that only listen (`"listens": true`), in the same order. A stage that listens hears what the others made, and is handed a copy of it, so whatever it writes there is never heard.

**The rules on the audio thread.** `process` runs on the audio thread while the unit's sound waits for it. A late call is a gap in what the owner hears. So, firmly:

- Nothing allocated, no file, no waiting, no line in the log. Count what you want to say in words of your own, and say it from your frame function.
- Of the base's helpers, only `op1f_memory`, `op1f_stage`, `op1f_mask` and `op1f_unmask`. Nothing of the firmware's.
- No division by a number that isn't a power of two, no 64-bit multiply, no floating point. A pack links no runtime, so the build fails, or the unit stops where one is called. Use shifts, tables and 32-bit multiplies.
- What it shares with your frame function is single words, or what it reads and writes inside an `op1f_mask` section of a few microseconds.
- **Saturate.** A sum that passes full scale wraps around to the other side, and the owner hears a loud crack. Hold every sum at full scale.
- **At most 1 KiB of stack** (`OP1F_STAGE_STACK_MOST`), your function's own frame and those of whatever it calls, together. `process` runs on the audio thread's own small stack, below the unit's own frames. A local array of a few kilobytes, the obvious way to write a delay line, runs past its end and wrecks the unit's memory at every block. A delay line, a table or a buffer belongs in your working memory (below) or in your pack's data, never in a local of `process`.

**The cost you declare.** `"cycles"` is what one call takes at most, in the processor's cycles. A block has some 580,000, and the unit's own work takes a good part of that. One stage may declare 1 to 116,000. All stages together may declare 174,000 a block, and a stage at `instrument` counts twice, since it's called for the synth and for the drum. An ask beyond either limit is left aside at the start, with a line, and your pack still runs without it.

The base times every call with the processor's own counter and holds you to what you declared:

- **A stage is switched off until the next start** when eight of its calls within one second go over, or when a single call takes more than 290,000 cycles (half a block). Going over means more than twice what it declared, and more than what it declared plus 20,000 (`OP1F_STAGE_CYCLES_SPARE`). After that the sound goes past it untouched, and the log names your pack and which rule it was.
- A call is timed from start to end, so the time includes interrupts the unit served meanwhile. The 20,000 cycles of slack are there so a light stage that keeps to its cost isn't blamed for them. A heavy one is held to twice its cost.
- Once a period the log gives, for each stage, its calls, the most cycles, the average, and how many calls went over. Read that line on your unit and set `cycles` from it, with room to spare. A stage that got switched off declared too little, or does too much.

Declare honestly. Too low and your stage gets switched off at the owner's busiest moment. Too high and you take room another pack needs.

`op1f_stage(card, ask)` answers 1 while the base calls the stage, and 0 when it doesn't: left aside, waiting for its memory, refused its memory, or switched off. Your frame function can ask it and say so on your page.

## Working memory

A pack's own zeroed data is small, and the room for pack files is shared by every pack. For more (a delay line, a loop, a big table) ask the base for memory:

```json
{ "kind": "memory", "name": "dub", "bytes": 1048576, "align": 32 }
```

- **How much**: from base 41, 1 byte to 32 MiB an ask, and 64 asks of all packs together. The base hands out first the unit's spare memory, 95.75 MiB, and only when an ask doesn't fit there, the unit's heap, up to 16 MiB for all asks together. A base before 41 gives 16 MiB an ask and for all asks together, and 16 asks. `align` is a power of two from 4 to 4,096 (4 if you leave it out).
- **When it comes**: the base takes it at its first frame, in the order of the files' names and then of the asks. It clears it to 0, a quarter of a megabyte a frame, and only then hands it over. The asks that come to 16 MiB together are cleared first, in their order, and the others after them. `op1f_memory(card, ask)` answers 0 until then, and the same block from then on, from any thread. A megabyte is there some four frames after the start, the first 16 MiB within some 66 frames as on older bases, and every 32 MiB after them some 128 frames more.
- **When it's refused**: when the ask is over 32 MiB, when it would be the 65th, or when there's no room left for it: none in the spare memory, and none in the heap either, where the packs take 16 MiB at most and the unit keeps 8 MiB free. The log says so, with your pack's name and the bytes. `op1f_memory` then answers 0 for good, and your pack runs without it. **Write your pack so it does nothing harmful with no memory**, and say so on its page.
- **The mark `op1f_memory_2`**: if your pack can't do without base 41's limits (an ask over 16 MiB, or more asks than an older base gives), name `op1f_memory_2` in your code, as a line in the log that prints it. Under an older base the loader then leaves your pack out, and its line names the mark.
- It's never given back while the unit runs, and it holds nothing of an earlier start: the base hands it out anew at every start, cleared.

**A stage that needs the memory waits for it.** Name the memory ask in the stage's `"memory"`. The base doesn't call the stage before the memory is handed over, and never when it was refused. So `process` can count on the memory being there. `dub` still checks it once, which costs nothing.

## Pictures that take a place

A screen of the unit asks for its pictures by name (`tape.svg`, `sketch.svg`). Before level 6, any picture your pack carried under such a name was served to that screen. That's how a pack changes one of the unit's screens. It's also a trap. A screen looks up the parts of its own picture by their names, and a pack's picture that lacks one of them can stop the unit at its start. One pack named a picture `sketch.svg` for its own page, and the unit didn't start.

**From level 6, a card takes the place of a picture of the unit only for the names it lists:**

```json
{ "kind": "places", "name": "tapes", "names": [ "tape.svg" ] }
```

Any other picture your pack carries is its own, whatever its name. You get it with `op1f_picture(card, name)`, and a screen of the unit that asks for that name gets the unit's own. The log says so once: `cards: '<pack>' carries a picture named <name> and does not take its place: the unit's own is shown`.

- Up to 256 names an ask. One plain ask of this kind a card, and from base 31 one that gives way after it (below).
- For each name you list, the pack tool checks that the unit has a picture of that name, and that yours holds every part the unit's own holds. It refuses the card otherwise. Keep that check. It's the one thing standing between your picture and an owner's unit that won't start.
- **Cards of levels 1 to 5 keep the old rule**: every picture they carry is offered to the screens by its name. An older pack that carries a picture named like one of the unit's own, by chance, can still stop a start. Build at level 6, or run `tools/picture_make.py census` on your pictures.
- Give your own pictures names of their own anyway (`dub-page.svg`, not `page.svg`). Examples get copied, and a name that happens to clash teaches the wrong habit.

### Giving way (base 31)

Two packs can carry a picture for the same screen. Without anything more, the pack whose file name sorts first wins, and that is luck. Some pictures are a matter of looks, a recolour say. Others carry a working page, like `sidechain`'s duck page, which lives on the signal flow screen. If yours is the first kind, let it give way:

```json
"asks": [
  { "kind": "places", "name": "looks", "names": [ "master.svg" ] },
  { "kind": "places", "name": "flow", "names": [ "signalflow.svg" ], "gives_way": true }
]
```

A name in the second list is served only when no other pack serves it. A pack that lists the name the plain way, or carries it at level 5 or lower, wins, and its picture shows. If that pack's `pick` answers 0 for the name, yours shows. You don't need to know which packs are out there, and your source names none of them.

- The plain ask first, then the one that gives way. The pack tool refuses the other order: a base before 31 reads only the first `places` ask of a card, and your plain list would be lost there.
- On a base before 31 the ask that gives way is left aside, with a line in the log, and its pictures aren't shown: the unit's own picture is. So on an older base your pack hides nobody's page, and only its looks are missing.
- The pack tool checks both lists the same way: every part of the unit's own picture has to be in yours.
- Two packs that both give way on one name, or both hold on to it, still go by file name, and the log says so.
- When a pack is served over one that gives way, the log says that once: `cards: '<pack that gives way>' gives way to '<pack served>' for the picture <name>`.
- In the header the flag is `OP1F_PLACES_GIVES_WAY`, a bit of the table head's `flags`.

## Settings (base 32)

Your pack can put things the owner may want to set (a speed, a number of steps, a drawing on or off) into the unit's own settings: hold **shift** and press **COM**, then **T1**. They look and turn like the unit's own items, and the base keeps them from one start to the next. hasis wrote the first form of this, in pull request 20.

**The ask.** One ask of kind `options` a card, on a card of level 6 or later, with one to eight options:

```json
{ "kind": "options", "name": "mine", "changed": "mine_changed", "open": null,
  "options": [
    { "name": "dot",   "choices": null, "first": false, "category": "keyboard" },
    { "name": "speed", "choices": [ "slow", "mid", "fast" ], "first": 1, "category": "midi" },
    { "name": "depth", "low": 0, "high": 100, "first": 50 }
  ] }
```

This one is made up for this page. An option's `name` is 1 to 15 characters of `A`-`Z`, `a`-`z`, `0`-`9`, space and `. + # % - /`. The unit shows it in its own capitals, and it has no `_`. Names differ within an ask.

**The three kinds.**

| Kind | In the recipe | Shown as | `first` |
| --- | --- | --- | --- |
| A switch | `"choices": null` | `OFF` or `ON` | `false` or `true` |
| A list | `"choices"`: 2 to 8 words of 1 to 7 such characters | the word | the word's number, from 0 |
| A number | `"low"` and `"high"`, whole numbers from -128 to 127, high above low, and no `choices` | the number | the number it starts at (`low` if you leave it out) |

**Where they show.** `"category"` picks the row on the left of the settings:

- `"keyboard"`, `"system"`, `"midi"`: after the unit's own lines in KEYBOARD, SYSTEM or MIDI. The right side holds seven lines, so there's room for four lines of the packs' in KEYBOARD, one in SYSTEM and two in MIDI. They're handed out in the order of the packs' file names. **An option that finds its category full shows on its pack's page under PACKS instead**, and the log says so once a start. So write your pack to work wherever its options end up.
- `"packs"`, or no `category`: on your pack's page under PACKS, a row the base adds after the unit's six. Its right side lists the packs that have options there or a page of their own, a name a line.

The owner turns them the way the unit's own items turn: the **ochre** knob picks a line, the **red** knob changes it. A switch or a list moves one step for four detents of the red knob, a number one step a detent, and they stop at their ends. Under PACKS, pressing the ochre knob on a pack's name shows that pack's options in the same right side, or opens the pack's own page (`open`, below). **T1** goes back to the list of packs, and on the list T1 leaves the settings as before. With more than seven lines, the seven shown follow the chosen one.

`"where"` is `"unit"` (the default): one value for the whole unit. `"tape"` is held for a value kept with each tape. Base 32 doesn't do that yet and keeps such a value for the unit.

**Reading them: positions.** `op1f_option(card, i)` gives option `i` (from 0, in the ask's order) as a position from 0 to 255, for every kind: a switch 0 or 1, a list its word's number, a number its distance from `low`. **Add your own `low` to get the number**: with `low` -20, a position of 17 is -3. Until the base has read its file, at the first frame, it gives the option's `first`. A card the base doesn't use, a card with no such ask (or one left aside) and an option it doesn't list give a number below 0, one of the error codes. It reads one byte, so any thread may call it.

```cpp
int p = op1f_option(&op1f_card_mine, 2);
int depth = p < 0 ? 50 : p;                       // below 0: no such option here, keep the start
```

**When one changes.** `changed(option, value)` is called on the display thread, at the frame after the owner changed it and before any frame function, with the new position. Note what you're told and do no more there, as with level 7's `changed`. Leave it `null` if you'd rather ask `op1f_option` each frame.

**A page of your own: `open`.** Name a function as `open` and your pack is listed under PACKS. When the owner presses the ochre knob on its name, the base calls `open` on the display thread, before the frame functions. Open your page there (`op1f_page_open`) and close it when you're done. The page is yours: the base shows none of your PACKS options for you, so set them from your page with `op1f_option_set(card, i, position)`. Your KEYBOARD, SYSTEM and MIDI options still show in those rows.

`op1f_option_set` takes a position, as `op1f_option` gives it, and works from your own frame function only; anywhere else it answers `OP1F_NOT_NOW`. A position past the option's last (a switch 1, a list its count less one, a number `high` - `low`), or an option or card it doesn't know, gets `OP1F_BAD_NAME`. Otherwise it answers 0: the value is kept, the file written as after a change in the settings, and a settings line that shows it is written again. `changed` isn't called for a value your pack set itself.

**The file.** The base keeps every pack's values in `user/pf/.options`, one line `<pack>/<option>=<position>` each. It reads the file at the first frame (lines that end in CR LF too) and writes it about a second and a half after the last change. Lines of packs that aren't on the unit this time are written back as they were, so taking a pack off and putting it back keeps its values.

- **A change in the last second and a half before the unit is switched off is lost.**
- A file over 4,096 bytes is neither read nor written over. The values then hold until the unit is switched off, and the log says so.
- A value the file holds that's past an option's last, after you changed the option, is dropped and the option takes its `first`.

**What the base checks.** It leaves the whole ask aside, with a line in the log, when it lists no option or more than eight, when an option isn't in order (a kind it doesn't know, a count or a `first` outside its kind's, a name not 1 to 15 characters), or when `changed` or `open` isn't a function of your pack. The pack tool refuses such an ask when it writes the card.

**On an older base.** A base before 32 doesn't know the kind: it leaves the ask aside with its line for kinds it doesn't know and runs the rest of your pack. It doesn't offer `op1f_option` or `op1f_option_set` either. So ask for them as optional names ([packs](packs.md), "Names: how packs find each other"), and when the name is 0 or answers below 0, use each option's `first`. Your settings simply don't show there. The listing's check refuses an entry with an options ask while the listed base is older than 32 ([the listing](catalog.md)).

In the header: the ask is `OP1F_ASK_OPTIONS`, its table an `Op1fOptions` (712 bytes, `open` at +708; a table of 708 bytes has no `open`), each option an `Op1fOption`, and `op1f_options_1` is the mark of a base that carries the ask out.

### Words your pack names while it runs (base 43)

A list's words sit in your card, which nothing changes while the unit runs. When the choices are only known then (the looks other packs hand yours at their first frame, the themes installed), give the option a function instead of words:

```json
{ "name": "look", "words": "my_look_word", "first": 0 }
```

```cpp
extern "C" const char* my_look_word(unsigned choice) OP1F_NAMED("my_look_word");
extern "C" const char* my_look_word(unsigned choice) { return choice < count ? names[choice] : 0; }
```

`word(choice)` answers the word of each choice from 0 (1 to 15 characters the unit's letters draw) and 0 past the last; the base asks for at most 32. It calls it on the display thread only, so answer from what your pack holds and do no more. The option shows on your pack's page under PACKS, and the **red** knob steps through the words there are at that moment, four detents a step. `op1f_option` gives the chosen word's number, `changed` and `op1f_option_set` work as for a list.

**The file keeps the word**, `<pack>/<option>=<word>`, not its number. At the first frame, before the other packs have handed yours anything, the option stands at `first`; at each frame after, while the word kept is not the word of the value, the base looks for it among the words and, found, takes its number and tells `changed`. So adding or removing a pack whose words come before it doesn't move the choice, and a word that is gone (its pack taken off) keeps its line until the owner picks another.

**Options of words come last** in the ask. The table's `count` holds only the options before them; they stand behind it, where a base 42 or earlier doesn't read. So **an older base runs the ask without them**: your other options show as before and `op1f_option` of an option of words answers below 0 there. An ask of options of words only has a `count` of 0, which an older base leaves aside, with its line. Base 43 checks each: a name of 1 to 15 characters, `count` 0, `first` below 32, its function one of your pack's (the whole ask left aside otherwise). It keeps the word of eight such options on the unit in all; a ninth works but starts at `first` each time. In the header: `OP1F_OPTION_WORDS` (kind 3), its function in the option's first four bytes of `choices`.

## How `dub` works

`dub` is one stage after the master effect that changes the sound, a megabyte of memory for its two delay lines (2^17 words each, about three seconds), a page on the base's blank page, and its three values. It changes no place of the firmware and names no address of it.

**The echo**, for each frame, left and right alike:

```
delayed = the line, `delay` frames back
line    = input + back × delayed        held at full scale
output  = input + level × delayed       held at full scale, only when the level is not 0
```

The delay is the time in sixteenth notes, worked out from the block's `period` with no division:

```cpp
static inline u32 delay_of(u32 period, u32 steps) {
    u32 p = period >> 6;                                  // a 96th of a beat, in frames times 2^12
    if (p > PERIOD_MOST) return LINE - 1;                 // slower than the lines can hold anyway
    u32 d = (p * (steps * (u32)SIXTEENTH)) >> 12;
    if (d < 1u) return 1u;
    return d > (u32)(LINE - 1) ? (u32)(LINE - 1) : d;
}
```

Shifting by 6 first keeps the multiply inside 32 bits at any tempo the unit has. At a very slow tempo eight sixteenths are longer than the line, and the delay is held to the line's length.

**What keeps the owner's ears safe**, each held by a test on a computer:

- Every sum is held at full scale. With full-scale sound coming in and any setting, nothing wraps.
- `back` is at most six tenths and `level` just under one. What comes back is always below full scale, and each echo is softer than the one before.
- The products are rounded toward 0, not down. Rounded down, a sample of -1 times six tenths stays -1 forever and the line never goes quiet. Rounded toward 0, silence in gives exact silence out once the tail has died away.
- At level 0 the stage writes nothing into the buffers, so the sound leaves it bit for bit as it came.
- The stage reads the three values as single words once a block, and counts its blocks and the frames it held at full scale in words only it writes. The frame function puts them in the log.

**Its cost.** `dub` declares 40,000 cycles a call. Its loop does about 200 cycles of work a frame, 13,000 a block, and each frame reads and writes the delay lines in the unit's main memory, which is slower than the memory next to the processor. The rest of the 40,000 is room for that. On a unit, the period's line showed at most 19,581 cycles a call and no call over.

**Its page** is the base's blank page, on which the frame function paints the word `dub`, the three numbers and three bars in the knobs' colours. If the memory was refused or the stage switched off, the page says so in one red line.

**Its values** go with each tape ([level 7](level-7.md)). With no tape loaded yet, or on a base that doesn't keep them, they're in `dub.bin`, in a folder of its own the computer doesn't see.

## The lines the base writes

| Line begins | What it says |
| --- | --- |
| `stage: '<name>' of '<pack>' at <point>, <n> cycles a block declared` | Your stage is in place, once a start |
| `stage: '<name>' of '<pack>' is left aside: ...` | Why not: its cost, the sum of all stages, its table, its memory refused |
| `stage: '<name>' this period: ...` | Through the base's quiet gate: calls, the most cycles, the average, and calls over what it declared |
| `stage: '<name>' of '<pack>' is switched off until the next start: ...` | The call that went too far, or how many calls went over in one second and the most of them, against what it declared |
| `memory: '<name>' of '<pack>' is given <bytes> bytes at ...` | Your memory, and how much is left where it came from: `of the spare memory stay`, or `stay free` for the unit's heap |
| `memory: '<name>' of '<pack>' is refused <bytes> bytes: ...` | Why not |
| `cards: '<pack>' carries a picture named ... and does not take its place` | A screen asked for a name your pack carries and doesn't list |
| `options: <n> pack(s) with options, <n> value(s) read from user/pf/.options ...` | From base 32, once a start: how many values the file gave, how many packs are under PACKS and how many lines went to KEYBOARD, SYSTEM and MIDI |
| `options: '<pack>': <n> option(s) on its page under PACKS, full: ...` | Options that found their category full, and which |
| `options: '<pack>' asks for options not in order (...): left aside` | Why your ask was left aside |
| `options: wrote user/pf/.options, ...` | The file written after a change, or `could NOT write` |
| `options: '<pack>' opens its page` | The owner pressed the ochre knob on your pack's name, and `open` was called |

## Limits, in one place

| | |
| --- | --- |
| Points | Three: an instrument after its effect, the master before its effect, the master after it |
| A stage's declared cost | 1 to 116,000 cycles a call |
| All stages together | 174,000 cycles a block, a stage at the instrument counted twice |
| Switched off | Eight calls in one second over twice the declared cost and over the declared cost plus 20,000, or one call over 290,000 |
| Stack | 1 KiB a call of `process`, with whatever it calls |
| Stages | 16 in all |
| Memory (base 41) | 32 MiB an ask, 64 asks in all, align 4 to 4,096; 95.75 MiB of spare memory first, then up to 16 MiB of the unit's heap with 8 MiB kept free |
| Memory (before base 41) | 16 MiB an ask and for all asks, 8 MiB kept free for the unit, align 4 to 4,096, 16 asks in all |
| Names of pictures that take a place | 256, one ask a card |
| Settings (base 32) | One ask a card, 1 to 8 options; a list 2 to 8 words; a number -128 to 127; room for 4 of the packs' lines in KEYBOARD, 1 in SYSTEM, 2 in MIDI; the file 4,096 bytes |

## Two packs at one place of the firmware

This is the one warning on this page that can cost an owner every pack at once.

**When two packs change the same place of the firmware, the loader keeps the one whose file name sorts first** and leaves the other out. The base is a pack too, `base.pack`, and from level 6 it holds the places of the sound's path. A pack whose file name sorts before `base.pack` (any name that starts with a digit, a capital letter or an `a`) and that changes one of the base's places keeps that place, and **the base is left out instead**. With the base go all the packs that need it. Nothing of open-field runs at that start, and the start screen says how many were left out.

A pack that works on the sound through a stage changes no such place, whatever its name. If your pack changes the firmware's sound path itself today, move it to a stage. If it really must change a place of its own, check first that the place isn't one the base holds. They're all in [the change list](changes.md), under the base.

# A first pack

Build a small pack, see what it is made of, put it on your own unit, see it work, read its lines in the log, and change it. The pack is `wink`, in `examples/wink/`. It puts a small red dot of ours on the tempo screen and makes it blink. It changes no place of the firmware and names no address of it: everything it does goes through the base, the shared pack `base.pack` ([the base](base.md)). It is the model for a pack of your own.

You need the tools of [building](build.md) for the first two steps and nothing else. From step 3 on you need a unit with open-field installed and `base.pack` on it ([the guide](guide.md)), on OS 1.7.3.

Read [NOTICE.md](../NOTICE.md) first if you have not: a pack runs inside the firmware of your unit, and a pack that is wrong can stop the unit from starting until it is taken off again. Step 3 says how to take one off.

## 1. Build it

From the top of the repository:

```bash
python -I tools/build_all.py
```

That builds everything, the examples among it, into `out/`, and holds each file to the size and SHA-256 kept in `packs/built.json`. The example is `out/examples/wink.pack`, 2,130 bytes.

The build overwrites nothing: the folder `out/` has to be new or empty. Delete it before you build again, or name another folder with `--out`.

To build the example by itself, into a file that is not there yet (the tool does not overwrite one either):

```bash
python -I tools/pack_build.py build3 --recipe examples/wink/recipe.json --profile profiles/op1f-1.7.3.json --out out/wink.pack
```

The profile is there for one thing: the pack's header names the firmware file it is for, by the first four bytes of its SHA-256. `wink` changes nothing in the firmware.

To see what a pack file holds:

```bash
python -I tools/pack_build.py check --pack out/examples/wink.pack
```

It prints the header's fields, the names the pack offers and wants, its card and its pictures. [Packs](packs.md) says what each field means.

## 2. What it is made of

Six files, in `examples/wink/`.

| File | What it is |
| --- | --- |
| `recipe.json` | What the pack is called, its version, which sources it is built from, its picture, and what its card asks for |
| `wink.cpp` | The pack's code: one function the base calls once a frame |
| `wink_card.s` | The pack's card, **made from the recipe** by `tools/pack_build.py card`. Never written by hand |
| `wink_pictures.s` | The directory of the pack's pictures, **made from the recipe** by `tools/pack_build.py pictures`. Never written by hand |
| `tempo.svg.make` | The instructions for its picture: how the unit makes it from its own tempo screen |
| `README.md` | What the example shows, in short |

**The card.** A pack that wants something of the base says so in a card, a small table in its own body. The recipe says what goes into it:

```json
"card": { "level": 1, "frame": "wink_frame", "pick": null, "pictures": true }
```

Level 1 of the base: a call every frame, and pictures. `wink_frame` is the function the base calls, and `pictures` asks the base to serve the pack's picture to the screen that asks for it. The card's source also makes the mark `op1f_level_1` a name the pack needs: on a unit without a base that offers it, the pack is left out with a line in the log, and nothing of it runs.

**The picture.** The tempo screen loads its picture, `tempo.svg`, from the unit's own files. `wink` carries a picture of that name, so the base serves the screen the pack's picture in place of the unit's. But the pack holds nothing of the device's picture: it holds *instructions*, `tempo.svg.make`, and the unit makes the picture from its own `tempo.svg` with one red dot of ours added before its last tag, named `wink_dot`. [The base](base.md), "Level 1: pictures", has the two kinds of instructions.

**The frame function** (`wink.cpp`). The base calls it once a frame, on the display thread, and hands it what the frame is: among it the number of the screen that is shown, and a clock in milliseconds.

```cpp
extern "C" void wink_frame(const Op1fFrame* now) OP1F_NAMED("wink_frame");
```

It finds the dot once, by its name, in the tempo screen's document, and keeps it. While the tempo screen (number 42) is the one shown, it shows the dot for half a second and hides it for half a second. On every other screen it does nothing. It calls four helpers of the base, each by a name the loader joins to the base's function:

| Helper | What `wink` asks of it |
| --- | --- |
| `op1f_screen(42)` | The document of the tempo screen: the picture as the firmware holds it, with parts that can be found by their names |
| `op1f_node(document, "wink_dot")` | The dot, by its name in the picture |
| `op1f_show(dot, on)` | The dot shown or hidden |
| `op1f_log(...)` | A line in the unit's log |

`module/build_r6e_card.h` declares every helper of the base, with what each takes and gives back and the thread it may be called from. `wink` includes the header of level 1, which holds the same words for that level.

**Its line in the log** goes through a small gate, `module/build_q_quiet.h`: it is said for the first periods of a start, when the dot blinked, and when the power popup comes up, and rarely otherwise. A line every frame would fill the log.

## 3. Put it on your unit

1. Put the unit into its disk mode, as you do to copy samples. It shows a folder `packs`, which holds `base.pack`.
2. Copy `out/examples/wink.pack` into `packs`.
3. Leave the disk mode. Switch the unit off and on.

Or, with no disk mode (on a Mac, the only way): on the install page's packs section press **connect**, then the **+** at the end of the list, pick `out/examples/wink.pack` and send. The unit restarts by itself once you press T1.

The start screen reads `1.7.3 - open-field 0.24`, as before: it names no pack. Press the metronome key: on the tempo screen, a small red dot at the top blinks, half a second on and half a second off.

Without `base.pack` on the unit the example does not run: the start screen counts it as left out, and the log says `nobody offers the name it needs, op1f_level_1`.

**To take it off again:** delete the file from `packs` and switch the unit off and on.

**If a pack ever stops your unit from starting:** hold the **help** key while you switch the unit on, until the first screen shows. That is the safe start: no pack is read, the unit starts as the original firmware, and the disk mode still shows `packs`, so you can delete the file. This example has run on a unit and does not need it. A pack you write yourself may.

## 4. Read its lines in the log

The unit writes a log whenever it runs. To get it:

1. On the unit, hold shift and press COM, then press T4: the disk mode starts.
2. The computer shows a folder `logs`. Copy the newest file from it: that is the start the unit is running now.
3. Leave the disk mode on the unit.

A base before 34 (a release before 0.16 on the start screen) shows no `logs`. Then take the log from the bootloader's drive:

1. Switch the unit off. Hold **COM** and switch it on: the bootloader's menu shows.
2. Press `3` for the developer menu and then `2`. **Press no other key there**: key `1` is a factory reset that asks nothing.
3. The unit shows on the computer as a drive. Copy the newest file from its folder `logs`.
4. Press `2` on the unit to close the drive, and switch it off.

In the log, the loader says what it did with each pack file, and the base what it did with each card. These lines are from a unit that ran the example beside the base, the four features and the two other examples. Each line of a log begins with a time, a letter for its level and a tag; they are left out here.

```
loader: user/pk/wink.pack 'wink' version 1 is placed at 0x8d3ae554, 762656 from where it was linked: 38 places of its own moved
loader: 'wink' runs: 0 words, 0 in DDR code, 0 in L1 code written and read back, 0 already there, 0 left to another pack
loader: 'wink' is joined: 5 names of other packs in 6 places, 0 optional names nobody offers; its sum as linked 7c44f9f4
cards: 'wink' version 1, level 1: a frame call yes, 1 picture(s), a choice among them no, an input function no
artwork tempo.svg: made on the unit, 44094 bytes: 43996 of the unit's own picture and 98 from user/pk/wink.pack, in 2167384 cycles
```

`0 in DDR code`: the example changes no place of the firmware. `5 names of other packs`: the mark and the four helpers, all of them the base's. The `artwork` line is the picture put together on the unit, from the unit's own picture and the 98 bytes of the dot.

Further down, the pack's own line, here as the log has it. Its tag, `wink`, comes from the file name the function hands to the logger:

```
  85.542.107: I wink[1]: wink: dot found in tempo.svg; looked for 1 times; on 2 times in 256 frames, 2 in all; after 11 periods held (activity); 4 printed, 11 held
```

And the base's line on the time the frame function takes, every 256 frames, in microseconds:

```
cards: 'wink' frame call: 256 calls, 2216 us in all, 1 least, 585 most
```

Where the pack is placed depends on which other pack files are on the unit: they stand one behind the other in the order of their files' names ([packs](packs.md), "Reading the placed code"). A line that says `is left out` tells you why a pack does not run. [Packs](packs.md) has every such line.

## 5. Change it

Open `examples/wink/wink.cpp` and change `HALF_A_SECOND = 500` to `HALF_A_SECOND = 250`: the dot will blink twice as fast. Give the pack a new version: in `examples/wink/recipe.json`, set `"version"` to 2.

The card and the directory of pictures carry the pack's version too, so write both again from the recipe, over the two sources the recipe names:

```bash
python -I tools/pack_build.py card --recipe examples/wink/recipe.json --out examples/wink/wink_card.s
```

```bash
python -I tools/pack_build.py pictures --recipe examples/wink/recipe.json --out examples/wink/wink_pictures.s
```

A build holds the two against the recipe and stops when one is not the recipe's. Build again, into a fresh folder:

```bash
python -I tools/build_all.py --out out-changed
```

The build now says `DIFFERS` for `examples/wink.pack` and answers 1. That is right: you changed the pack, so it is another file. If the change is one you want to keep, this writes the new size and SHA-256 into `packs/built.json`:

```bash
python -I tools/build_all.py --out out-kept --update
```

Copy the new file over the one in `packs` and restart. The dot blinks faster, and the log says `'wink' version 2`.

Put the example back as it was when you are done (`git checkout examples packs/built.json`), unless you mean to send the change in.

## 6. A pack of your own

Copy the folder `examples/wink` to a folder of your own and give the pack a new name: in `recipe.json` the `name`, the `build` name, the paths of the sources and of the picture, and the function the card names; in the C++ file the function's name and the name of the file it hands to the logger. Then make the card's source and the directory of pictures from your recipe, as in step 5 with your own recipe and file names, and build it with the second command of step 1 and your own recipe.

What your pack can ask of the base comes in levels, each with an example:

| Level | What it adds | Example |
| --- | --- | --- |
| 1 | A call every frame, pictures for the firmware's screens and of the pack's own, helpers for a screen's parts and the log | `wink` |
| 2 | A page of the pack's own, with keys and knobs | [`yoyo`](../examples/yoyo/README.md) |
| 3 | Folders and files on the unit's disk, shown in the disk mode if the pack asks, and text on its page | [`list`](../examples/list/README.md) |
| 4 | Painting on the frame, pages that stack, text placed and measured, a file deleted, a key watched | [`sketch`](../examples/sketch/README.md) |
| 5 | A sequencer of your own in the unit's list, and the notes heard | [`echo`](../examples/echo/README.md) |
| 6 | A place in the sound, working memory, pictures that take a place by name | [`dub`](../examples/dub/README.md) |
| 7 | Values kept with each tape and each sound, in the unit's own files | [`dub`](../examples/dub/README.md) |
| 8 | An effect of your own in the unit's lists of effects, for the sounds and the master | [`crush`](../examples/crush/README.md), [`ping`](../examples/ping/README.md) |
| 9 | A synth engine of your own in the unit's list of engines, with sounds of its own | [`pluck`](../examples/pluck/README.md), [`fold`](../examples/fold/README.md) |
| 10 | The value as the sound plays it, LFOs and all, for your page to draw | the four of levels 8 and 9 |

When the examples are too small for what you are after, look at [open-field-packs](https://github.com/LaP0573/open-field-packs). It holds four real packs on cards with their sources: `lfo` (level 4), `8 tracks` and `sidechain` (level 7) and `fx train` (level 8). It is also the model for a repository of your own: a folder a pack, and a `build.py` that builds them with the tools and headers of this one at a commit it names.

[The base](base.md) has levels 1 to 3: the card, the order in which keys are offered, the threads, the folders and files, the text. Levels 4 to 10 each have a page of their own: [level 4](level-4.md), [level 5](level-5.md), [level 6](level-6.md), [level 7](level-7.md), [level 8](level-8.md), [level 9](level-9.md), [level 10](level-10.md).

A picture of your own is instructions too. `tools/picture_make.py` makes the picture on your computer from a copy of your unit's picture folder and says what it holds; `tools/picture_delta.py` works out the stretches of a picture of the second kind.

Before you send a pack or a change in, read [CONTRIBUTING.md](../CONTRIBUTING.md). Two of its rules matter most here: say in your own words what you found about the firmware, and never put anything of the firmware into a file, not even a few bytes in a comment.

To offer your pack to every owner on the install page, add it to [the listing](catalog.md).

## 7. A pack that changes the firmware

Some features cannot be had through the base: they have to stand in the firmware's own code, where it does something. `8 tracks` is such a feature ([its sources](https://github.com/LaP0573/open-field-packs/tree/main/8%20tracks)). A pack of that kind carries a **table of changes**: where the running firmware is changed, and which function of the pack goes there. The loader makes the changes at every start, and only where a checksum says the firmware is the one the pack was made for.

The smallest such pack is `trampolines`, in `examples/trampolines/` ([its page](../examples/trampolines/README.md)). It adds nothing you can see or hear: it stands at two functions of the firmware, counts, and writes one line into the log. It needs no base.

| File | What it is |
| --- | --- |
| `recipe.json` | What the pack is called, its version, and which sources it is built from |
| `profile.json` | The two places of the firmware the pack changes: an address, a length and a checksum each, and which function of the pack goes there |
| `trampolines_table.s` | The pack's table of changes, **made from `profile.json`** by `tools/table_build.py`. Never written by hand |
| `trampolines.s` | Two small routines in assembly: where the firmware arrives when it reaches a change |
| `trampolines.cpp` | One C++ function that writes the line into the log |

To build it by itself, with its own profile:

```bash
python -I tools/pack_build.py build3 --recipe examples/trampolines/recipe.json --profile examples/trampolines/profile.json --out out/trampolines.pack
```

**The two places.** Each is the first instruction of a function of the firmware that the start-up calls once:

- `0x8c0dd488` draws a text with the firmware's font. The start-up calls it for the version text on the start screen.
- `0x8c053a9c` is a small function with one argument, called right after that.

At each, the loader writes a jump to a routine of the pack. That is a **hook**: the firmware's function is entered, and our routine runs first.

**The trampolines.** A hook's jump takes the place of the function's first instructions, and the function still needs them. The loader copies them from the running firmware into a **room**, some bytes of the pack's own memory, and puts a jump back behind them. Our routine ends in a jump to its room: the copied instructions run, then the firmware's function goes on as if nothing had happened. That is a trampoline, and it is why the pack holds none of the firmware's instructions.

**The routines** (`trampolines.s`). The first counts and jumps to its room. The second saves two registers, calls the C++ function, restores them and jumps to its room. Its comments say which registers a routine may use at the first instruction of a function, and why a call needs twelve free bytes on the stack; [methods](methods.md) has the rules.

**The C++ function** (`trampolines.cpp`) calls one function of the firmware, the logger. A pack with a card has the base's `op1f_log` for that; a pack that stands in the firmware calls the firmware's functions itself. The two lines at its top are how any function of the firmware is called: a type of our own, and a name for the address.

```cpp
typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
#define STOCK_LOG ((LogFn)0x8c053a20u)
```

[The firmware's functions our code uses](firmware-interfaces.md) lists the ones that are known, with what each takes and gives back.

**The profile** (`profile.json`) has each place once, under `patches`, with a checksum of the firmware's bytes there, and under `tables` the pack's table: for each place the form of the change (`jump`), the pack's function (`target`), the room, and how many bytes the trampoline copies. The second place copies 12 bytes, which is more than the eight the jump replaces, so its entry has a checksum of its own over those twelve.

**Its line in the log**, here as a unit on OS 1.7.0 had it (the example has not run on a unit under 1.7.3 yet):

```
   1.089.800: I trampolines[1]: trampolines: the draw of the version text came through its trampoline 1 time(s), and this is call 1 through the second; the rooms' sums are 6dc09fde and 096568b4
```

The two sums are checksums of the two rooms as the loader filled them. On every unit with the same firmware they are the same two numbers; under 1.7.3 they are others, since each room ends in a jump back to another address.

**A pack of this kind of your own.** Copy the folder `examples/trampolines` and give the pack a new name: in `recipe.json` the `name`, the `build` name and the paths of the sources; in `profile.json` the `id`, the `group`, the `part` and the table's `file`. Then, in the order you will need them:

1. **Find the place.** [Methods](methods.md) says how to read your own firmware file: how to find a function by a message it logs, how to see who calls it, and how to choose where a change can stand.
2. **Work out its checksum** from your own firmware file. The tool prints the lines for your profile, and says how many bytes a trampoline has to copy there:

```bash
python -I tools/place_sum.py --stock <your own os.ldr> 0x8c0dd488 0x8c053a9c
```

3. **Write the profile's entries**, in your own words: what stands at the place, and what your function does there. Then let the tool write the table:

```bash
python -I tools/table_build.py --profile examples/trampolines/profile.json --all
```

4. **Write your routine and your function**, with the example as the model. If a routine is in assembly, hold its calls to the rule of the twelve bytes:

```bash
python -I tools/stub_calls.py --listing build/example_trampolines_placed.dis.txt --all examples/trampolines/trampolines.s
```

   (The listing is made by the command under "Reading the placed code" in [packs](packs.md).)
5. **Build, copy, restart, read the log.** Build with the command above, with your own recipe and profile. Keep the help key in mind.

A pack may do both: carry a card and change places of the firmware. Two packs cannot change the same place: the later one is left out ([packs](packs.md)).

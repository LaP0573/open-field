# Packs: the file, and what the loader does with it

A **pack** is one file that holds a feature: our own compiled code and, as the pack needs them, a **card** that says what it wants of the base, a list of places where its code changes the firmware, and the instructions for its pictures. The **loader** is the small piece of our code inside the firmware file. At every start it looks for pack files on the unit's disk, checks each one, puts those that fit into memory, joins them by their names, makes their changes, and hands the cards to the base.

**The base** is a pack too, the file `base.pack`: the shared pack every other pack relies on. Every unit with open-field has it. What it offers a pack through its card is on [its own page](base.md).

This page says what a pack file is made of and what the loader does with it, step by step, with the lines it writes into the log. If you have not built a pack yet, start with [a first pack](first-pack.md).

Everything here is for OS 1.7.3 and the loader of this repository (`module/build_m4_loader.cpp`).

## Where pack files lie

On the unit's disk, in the folder `user/pk`. The unit's disk mode shows the same folder as `packs`.

The loader takes every file there whose name ends in `.pack`, in small letters or capitals. A name is 6 to 39 characters long with its ending, in plain characters, without any of `/ \ : * ? " < > |`, and does not begin with a dot or a space. What a pack is stands inside the file, not in its name. At most 128 pack files are looked at under N2, the loader the install page offers by itself, and 32 under N1 ([below](#two-loaders-n1-and-n2)).

Nothing of a pack is inside the firmware file: the base, too, is a file in this folder.

## Two loaders, N1 and N2

There are two versions of the loader, and every pack file runs under both, as it is. The install page's builder has both: loader v2 is N2, the one the page offers by itself since 2026-11-26, and loader v1 is N1, which the page gives to someone who asks for it by name (`?loader=1.7.3-v1` behind its address). N1 stays for the way back.

| | N1 (loader v1) | N2 (loader v2) |
| --- | --- | --- |
| The room for all packs | 2,031,616 bytes, from `0x8d2f4234` | 16 MiB, from `0x85000000` |
| Pack files looked at | 32 | 128 |
| The log's first loader line | `M4 at the load point` | `N2 at the load point` |

The start screen reads the same under both. The log's first line tells them apart, and so does the install page once it is connected: on a unit with N1 it says so when the page offers N2. Going from N1 to N2 is a new install of the firmware file from the page, with the same steps and the same risk as the first. Your pack files stay where they are. With 128 files, N2 takes about 1.3 seconds longer to start.

A base that knows N2 (base 40 and later) takes its numbers from the loader. An older base counts against N1's numbers under both loaders: under N2 it refuses a change from the install page that would leave more than 32 pack files or more than N1's room, though the unit would run it.

**Stuck with more than 32 files.** With more than 32 pack files and a base older than 40 (copied by hand, say, or under N2), the base refuses every change from the install page, the new `base.pack` too, since the count after the change is still over 32. The way out is the disk mode: its folder `packs` shows every file under N2. Take files off there until 32 are left, or copy the new `base.pack` there yourself. Then switch the unit off and on, and the page works again.

**Going back from N2 to N1.** Before you write N1's firmware file, cut the folder `packs` down to 32 files at most and N1's room of 2,031,616 bytes, with `base.pack` among them. N1 looks at the first 32 files in the order the folder happens to list them, not by name, so with more files `base.pack` may be one of those it never sees. Nothing is harmed either way, but which packs run would be chance.

## What is in the file

Three or four parts, one behind the other. All numbers are 32 bits wide with the low byte first.

**1. A header of 96 bytes.**

| At | Bytes | What |
| --- | --- | --- |
| 0 | 8 | The text `OP1FPACK` |
| 8 | 4 | The format, 3 |
| 12 | 4 | The header's size, 96 |
| 16 | 4 | The firmware the pack is for: the first four bytes of the firmware file's SHA-256 |
| 20 | 4 | The address the body was linked for |
| 24 | 4 | The body's size, a multiple of four |
| 28 | 4 | How many of the body's bytes are code and constant data. The pack's table of changes and its card lie in them |
| 32 | 4 | The size of the zeroed data behind the body: memory the pack wants cleared before it runs |
| 36 | 4 | The CRC-32 of the body and the lists |
| 40 | 16 | The pack's name: 1 to 15 plain characters, then zeros |
| 56 | 8 | The address of the pack's table of changes and of the table's end, as linked. A pack that changes no place of the firmware names its own first byte as both |
| 64 | 4 | The size of the lists |
| 68 | 4 | The size of what follows the lists and is not put into memory: the pictures |
| 72 | 4 | The pack's version, a number that only grows |
| 76 | 4 | Its interface number |
| 80 | 4 | The largest alignment inside the pack: a power of two, 32 at most |
| 84 | 4 | Flags: 1, the pack is not to be named on the start screen (this loader names no pack there); 2, the loader writes a checksum of the pack's memory into the log |
| 88 | 4 | Where in the body 48 bytes stand for the path of the pack's own file, or 0 |
| 92 | 4 | Where in the body the pack's card stands, or 0 for none |

**2. The body.** The pack's code, its constant data and its initialised data, as the linker made them for the address in the header.

**3. The lists.** Six counts, then five lists and the names:

| List | An entry | What it is |
| --- | --- | --- |
| works | a name, an interface number, 1 when needed | A pack this one works with |
| exports | a name, an offset from the pack's first byte | A name this pack offers to others: a function or data |
| imports | a name, 1 when needed | A name it wants from another pack |
| places | an offset in the body, a kind, a value | A spot in the body that holds an address, so that the loader can set it right once it knows where the pack stands |
| marks | the number of a table entry, the number of a pack in the works list | A change that is made only when that other pack is away |
| names | texts, each ended by a zero byte | The names the lists use |

A place has one of four kinds: the high half or the low half of an address that code loads into a register, a whole word of data, or a direct call. Its value is the address as linked, for an address of the pack's own, or what is to be added to another pack's address.

**4. The pictures**, for a pack that has any: the instructions for each, one behind the other. The loader does not put them into memory. The body holds their directory, and the base's picture loader reads one from the file when a screen asks for it ([the base](base.md), "Level 1: pictures").

You never write any of this by hand. `tools/pack_build.py` makes the file from a recipe, and prints what a file holds:

```bash
python -I tools/pack_build.py check --pack out/examples/trampolines.pack
```

## The recipe

A recipe is a small JSON file that says what a pack is built from. `examples/wink/recipe.json` is a small one with a card, `examples/trampolines/recipe.json` one with a table of changes, and [the recipe of `lfo`](https://github.com/LaP0573/open-field-packs/blob/main/lfo/recipe.json), in open-field-packs, one that uses most fields.

| Field | What |
| --- | --- |
| `name` | The pack's name, 1 to 15 plain characters. It is what the log shows, and what other packs name it by |
| `version` | A number that only grows. Of two files that hold the same pack, the one with the higher version runs |
| `interface` | A number that changes when what the pack offers to other packs changes its meaning |
| `build` | The name of the files the build leaves in `build/` |
| `sources` | The assembly and C++ files, in the order they are linked. A pack's table of changes, its card and its directory of pictures are among them |
| `works_with` | The packs this one works with: for each its `name`, the `interface` number this pack was made for, and `needed`, true when this pack cannot run without it |
| `exports` | Names this pack offers to any pack |
| `serves` | Packs built together with this one. What their code wants of this pack is offered without a list written by hand |
| `pictures` | For each picture, the `name` a screen asks for and the `file` with its instructions |
| `card` | What the pack wants of the base: its level, its frame function, its pictures, its input function, its folders ([the base](base.md), "The card") |
| `release` | The base's alone: the short text the start screen shows, one to eight characters |
| `marks` | Changes made only when another pack is away: the `site`, an address of the pack's table, and `away`, that pack's name |
| `named` | false sets the header's flag 1. The loader of this repository names no pack on the start screen, so the flag changes nothing there |
| `sum` | true makes the loader write a checksum of the pack's memory into the log |
| `path` | The name of 48 bytes in the pack into which the loader writes the path of the pack's file, or null. A pack with pictures gets this by itself |

## What a pack is, once it runs

A pack has no function that is called at the start. **It runs when the firmware reaches one of its changes**, or **when the base calls it through its card**:

- a change is a call that now goes to the pack, a jump at the first instruction of a function, or a word in a table of functions. Those changes are the pack's table, and the next sections say how the loader makes them;
- a card names the pack's frame function, which the base calls once a frame, and its input function, which the base offers every key and knob event ([the base](base.md)).

A pack can have either, or both. Every example has a card and no table, but `trampolines`, which has a table and no card.

Its code is plain C++ and assembly for the Blackfin processor, with no library behind it:

- no `malloc` or `new`, no `memcpy` or `memset`, no `printf`;
- no division or remainder by a variable, and no floating-point arithmetic of the compiler's own;
- no object whose constructor would have to run before `main`: nothing runs before the first change is reached or the first frame is called.

What it may count on:

- its zeroed data is zero, and its initialised data is as built, when the first of its functions runs;
- every address inside it is right for the place it stands at;
- every one of its changes is in, or none: the loader makes a pack's changes all together or leaves the pack out.

## What the loader does at a start

The loader runs once, early in the start-up, on the main thread, right after the firmware has mounted the disk and before the tape, the sound engine and the screens are set up. Every line below is in the log with the tag `op1f_loader`.

**1. It looks at itself.** It checks its own three changes and tries a write of code on a function of its own. Its first line names it: M4 for N1, as here, or N2, whose line reads `N2 at the load point` and `packs are loaded from 0x85000000, room 16777216 bytes`.

```
loader: M4 at the load point: control 0x00000207, packs are loaded from 0x8d2f4234, room 2031616 bytes; 3 of its own 3 changes of stock pass their check, 3 by a checksum
loader: self-test of a code write in DDR: 1 before, 2 after
```

If that write does not work, nothing of the firmware is changed and the unit starts as the original firmware.

**2. The safe start.** It asks the keyboard controller which keys are down. With the help key held, no pack is read ([below](#the-safe-start)).

```
loader: keys at the start 00000000 00000000 (answer 0x42, 8 bytes, 2079803 cycles): the help key is not held
```

**3. It lists the folder** and reads the header of each pack file.

```
loader: the folder user/pk listed: 8 pack(s) of the new kind and 0 of the old in 8 file(s); 0 more not looked at, 0 other name(s)
```

`of the old` counts files in an earlier pack format. This repository does not build that format. N1 still reads it, and such a file runs only when no pack file of today's format does. N2 does not run it: it is left out with the line for a format the loader does not know, and the start screen counts it among the files left out.

**4. It places the packs.** Packs stand one behind the other in a room of 16 MiB that begins at `0x85000000` (under N1, 2,031,616 bytes from `0x8d2f4234`), in the order of their files' names. Each pack is read from its file straight into its place, body and lists. Its CRC-32 is taken there and held against the header's.

A pack is linked for `0x8d2f4234`, the first byte of N1's room, under either loader, and stands somewhere else. So the loader **moves** it: at every place of the pack's own, it writes the address the pack was linked with, further on by the distance. A pack stands at the next address that is a multiple of 32 bytes away from where it was linked.

```
loader: user/pk/base.pack 'base' version 17 is placed at 0x8d3626b4, 451712 from where it was linked: 2683 places of its own moved
loader: its 107396 bytes read in 3155090 cycles, checksum 6309e0a3 ok in 1440202 cycles
```

That example is from N1. Under N2 the first pack stands at `0x85000014`, -137314848 from where it was linked: the distance is negative, since the room lies below the address packs are linked for.

Of two files that hold the same pack, the one with the higher version is placed, and the other stands by with a line that ends `stands by and is not read: ... holds this pack, version 2`. A newer file of a pack that cannot run is still the one placed: it shadows an older file that would run.

**5. It settles which packs run**, before anything of the firmware is written. A pack is left out when one of the checks of the next section fails. Leaving one pack out can leave another without a pack or a name it needs, so the loader goes round until nothing changes. With no pack file left that runs, nothing is changed and the unit starts as the original firmware.

**6. It joins the packs, and hands the cards over.** For every name a pack wants, the loader writes the address of the pack that offers it into the wanting pack's places. A wanted name that is optional and that nobody offers gets the address 0. Then each pack's zeroed data is cleared, and the loader writes the cards of the packs that run into the list the base offers, in the order of the files' names. Then each pack's memory is written out of the processor's cache, so that the processor runs what was just loaded.

```
loader: 3 card(s) handed to 'base', its list at 0x8d375510; 0 more than the list holds
```

That list has 32 slots, and cards past them are counted as more. N2 also fills a second list of 128 slots, `op1f_cards_2`, when the base offers one, and then writes that line once for each list.

**7. It makes the changes.** First its own, then each pack's table. A pack's table is written after the tables of the packs that lean on it: those that need it, or need a name it offers. So the base comes last. For every entry the loader reads the place again, takes the checksum again, makes the trampoline if the entry has one, writes, and reads back what it wrote.

```
loader: the disk mode's folder 'packs' is the loader's: 1 word, 1 in DDR code written and read back; a pack's folders after it: yes
loader: 'wink' runs: 0 words, 0 in DDR code, 0 in L1 code written and read back, 0 already there, 0 left to another pack
loader: 'wink' is joined: 5 names of other packs in 6 places, 0 optional names nobody offers; its sum as linked 57204c57
loader: 'base' runs: 2 words, 12 in DDR code, 0 in L1 code written and read back, 0 already there, 0 left to another pack
loader: 'base' has 14 of its changes held by a checksum in place of stock's bytes; 5 trampolines made in its zeroed data
loader: done: 8 pack(s) run, 0 left out, control 0x00000207, 49158269 cycles
```

**8. It sets the start screen's text** ([below](#what-the-start-screen-says)).

```
loader: the start screen's version text is followed by ' - open-field 0.25': 1 in DDR code written and read back, 0 already there
loader: the start screen says '1.7.3 - open-field 0.25'
```

A change that passed its check and then does not go in, or a place that changed between the check and the write, **stops the start-up** with a line that begins `loader: STOPPED`. The firmware would otherwise run with half of a pack's changes. This has not happened on a unit.

## Why a pack is left out, and what the log says

Each of these is a line of the level E, an error, of the form `loader: <file> '<pack>' is left out: <why>`, or `loader: '<pack>' is left out, <why>` for an entry of its table. Search a log for `: E ` to find them. A pack file that is left out is counted on the start screen; everything else runs.

| The line says | What it means |
| --- | --- |
| `is left out: its header could not be read` | The file could not be opened or read |
| `is left out: a pack of format N, which this loader does not know` | A file made for another loader |
| `is left out: its header does not fit this loader` | A field of the header is not possible: another firmware, sizes that do not add up to the file's size, a table that is not inside the code. The next line prints the header |
| `is left out: it could not be read` | The read stopped short |
| `is left out: its checksum is not its header's` | The file is damaged, or was cut off while it was copied |
| `is left out: its card is not in order` | The header names a card that does not lie on a word in the body, or that does not begin with a card's mark and a size that ends inside the body. The next line prints what was found |
| `is left out: its lists are not in order` | The lists do not fit their own counts |
| `is left out: no room is left for it` | The packs before it fill the room |
| `is left out: it needs the pack X` | A pack named in `works_with` with `needed` does not run. **`it needs the pack base`: copy `base.pack`** |
| `is left out: it was made for another interface of the pack X` | The pack X runs with another interface number than this pack was built for |
| `is left out: it offers a name the loader keeps to itself, X` | Every name that begins `op1f_loader_` is the loader's |
| `is left out: an earlier pack offers the same name X` | Two packs offer one name. The earlier one, by the order of the files' names, keeps it. That holds for the base too: a file sorted before `base.pack` that offers one of its names leaves the base out |
| `is left out: nobody offers the name it needs, X` | A name the pack cannot run without. For a mark, `op1f_level_3` say, the base on the unit is older than the pack's card, or there is no `base.pack`; for `op1f_loader_2`, the loader is N1 and the pack needs N2. Naming that mark as needed is how a pack says it runs only under N2 |
| `is left out: a direct call does not reach the name X` | Should not happen inside the room |
| `is left out, a kind this loader does not know` | A table entry of a kind a newer loader has |
| `is left out, a site outside stock's memory or off its alignment` | A table entry's address is no place code or a word can stand at |
| `is left out, a target outside the code it came with` | A table entry names a function that is not in the pack |
| `is left out, not the stock bytes at the site` | **The checksum of what stands at a place is not the entry's.** The unit runs another firmware than the pack was made for, or the address or the checksum is wrong |
| `is left out, two entries on the same bytes` | Two entries of one table overlap |
| `is left out, a trampoline's room outside the zeroed data, or shared` | A room must lie in the pack's zeroed data, on an even address, and belong to one entry |
| `is left out, a load entry on a site that holds no load of a register` | The form `load` on a place that holds something else |
| `is left out: one of its changes of stock, at A, meets one of X` | A place of this pack overlaps one that the loader or an earlier pack changes. The earlier one keeps it |

The line for a table entry also gives the entry's number, its address, what was found and what was wanted.

A pack the loader runs can still find its card refused by the base, which then does nothing for it. The start screen does not count that: the log has the base's line, `cards: the card of the pack at ... is not used: ...`, with why ([the base](base.md)).

## The table of changes

A pack's table says where the running firmware is changed and which function of the pack goes there. You do not write it: `tools/table_build.py` makes it, as an assembly source, from the pack's profile ([a first pack](first-pack.md), "A pack that changes the firmware", shows one). One entry is five words:

| Word | What |
| --- | --- |
| kind | `0x10`, plus the form (below), plus, in the high half, the number of bytes its trampoline copies |
| place | The address in the firmware |
| target | The pack's function. The linker puts its address here |
| sum | The CRC-32 of the firmware's own bytes that the change replaces, or that its trampoline copies when those are more |
| room | The address of the pack's room for the trampoline, or 0 |

**The forms**, as a profile names them:

| Form | Kind | What the loader writes |
| --- | --- | --- |
| `word` | 0 | Four bytes of data, such as an entry of a table of functions, become the address of the pack's function |
| `call` | 1 in L1 code, 2 in DDR | Eight bytes of code become a call of the pack's function |
| `jump` | 3 in L1 code, 4 in DDR | Eight bytes of code become a jump to the pack's function |
| `load` | 5 | Eight bytes of code that load an address into a register load the address of the pack's function into that register |

*L1 code* is the fast memory inside the processor, `0x11a00000` to `0x11a10000`, where the audio code runs. *DDR* is the main memory, `0x80000000` to `0x90000000`. A table names the kind of memory because the loader writes the two in different ways.

**The checksum is the pack's safety.** The loader takes the CRC-32 of what stands at the place in the unit's memory. Only when it is the entry's does anything go in. So a pack made for OS 1.7.3 does nothing on a unit with another firmware, and a wrong address is caught before it does harm. No profile, table or pack holds the firmware's bytes themselves. `tools/place_sum.py` works a checksum out from your own firmware file ([methods](methods.md)).

**A trampoline** is how a function of the firmware goes on after a jump of ours at its first instruction. The jump takes the place of the function's first eight bytes, and those instructions are still needed. The loader copies them from the running firmware into a **room**, some bytes of the pack's zeroed data, and puts a jump back behind them. The pack's routine does its work and ends in a jump to the room. So no pack holds a copy of the firmware's instructions.

- Instructions are 2, 4 or 8 bytes long, so those under an eight-byte change end 8, 10, 12 or 14 bytes in. The trampoline copies up to there. A room is that many bytes and eight more.
- An instruction that names a place by its distance from itself (a short jump, a call by distance, the set-up of a loop) means something else at another address. A place with such an instruction under the change cannot have a trampoline.
- Only a `call` or a `jump` has one.

**A change made only when another pack is away** (`marks` in the recipe). Two packs may both have something to do at one place of the firmware. The pack that can do without the other marks its entry: the loader makes it only when the other pack does not run. The pack `sidechain` does this at four places it shares with `8 tracks`.

## Names: how packs find each other

Packs are built apart and joined on the unit, by name.

- A pack **offers** names: functions and data. In C++ they are ordinary names with an `asm("...")` label.
- A pack **wants** names of other packs. A name wanted outright is *needed*: if nobody offers it, the pack is left out. A name declared weak is *optional*: if nobody offers it, its address is 0, and the code tests for that before it uses the name.

```cpp
extern "C" void __attribute__((weak)) op1f_duck_frame() asm("op1f_duck_frame");

if (&op1f_duck_frame != 0) op1f_duck_frame();     // the pack that offers it may be away
```

The build works the lists out from the compiled code. Call an optional name only behind such a test: the build refuses a direct call to one, since nothing could make it safe.

A needed name can also stand for an ability and not for a function to call: a **mark**. It is never called or read.

- The base offers `op1f_level_1` to `op1f_level_10`: "this base serves cards of that level". A pack's card makes its level's mark a name the pack needs ([the base](base.md), "The levels, and their marks").
- The loader answers itself for `op1f_loader_1`: "this loader hands the cards over". The base needs it. A pack that needs something only a later loader does names that loader's mark, and under an older loader it is left out with a line that says which mark was missing.
- N2 answers for `op1f_loader_2` as well, with the address of six words of its own: their size in bytes (24), the loader's generation (2), the room's first address, the room's bytes, the most pack files it looks at (128) and the slots of the second list of cards (128). Read the size first and only the words it covers: a later loader adds words behind and never changes the meaning of one. Under N1 the name gives 0, so a pack that names it as optional can tell the two loaders apart.

## What the start screen says

The loader adds its words behind the firmware's own version text on the start screen.

| The screen reads | When |
| --- | --- |
| `1.7.3 - open-field 0.25` | Pack files run, and none is left out. `0.25` is the base's release |
| `1.7.3 - open-field 0.25 (1 left out)` | One pack file is there and does not run. The log says why |
| `1.7.3 - open-field 0.25 (1 needs foo)` | A file is left out because it needs the pack `foo`, and no file of the folder holds `foo` |
| `1.7.3 - open-field` | Pack files run, and none offers a release: `base.pack` is not among them |
| `1.7.3 stock (no pack)` | No pack file: the unit runs as the original firmware |
| `1.7.3 stock (4 need base)` | Four pack files are there, and none runs because `base.pack` is not: copy it |
| `1.7.3 stock (2 left out)` | Pack files are there and none runs |
| `1.7.3 stock (help key)` | The safe start |
| `1.7.3` | The loader could not change the text, or did not run |

The names of the packs that run are not shown: the log has them. Where the words would be wider than their share of the screen, the files left out are given as one count, and then the release is left out too.

A pack is named as needed only when no file of the folder holds it. A file that lies in the folder and cannot run is counted as left out, and the log says why.

## The safe start

Hold the **help** key while the unit is switched on, until the first screen shows. The loader then reads no pack: the unit starts as the original firmware for this one start, and the disk mode still shows the folder `packs`, so a pack that misbehaves can be deleted. The log has:

```
loader: keys at the start 00000000 00000001 (answer 0x42, 8 bytes, 1842053 cycles): the help key is held, no pack is read
loader: no pack runs: the unit starts as stock firmware
```

It catches a pack that passes every check and then hangs or crashes the unit, a frame function that never comes back among them. It does not catch a loader that fails: that takes the way back in [the guide](guide.md).

## Reading the placed code

A pack stands at another address on the unit than it was linked for, so an address from a log or a crash has to be read against the pack as placed. `tools/pack_build.py placed` links each pack of a set for its place, as the loader places them when those files lie in `user/pk`, and writes a listing of it into `build/`. It reads the packs from one folder, each under its pack's name, and the objects the build left in `build/`, so run it after a build.

The example `trampolines`, alone:

```bash
python -I tools/pack_build.py placed --recipe examples/trampolines/recipe.json --profile examples/trampolines/profile.json --folder out/examples --name example
```

The example `wink` behind the base, as on a unit with those two files. Build the two into one folder first:

```bash
python -I tools/pack_build.py build3 --recipe packs/base.json --recipe examples/wink/recipe.json --profile profiles/op1f-1.7.3.json --out-dir out/set
```

```bash
python -I tools/pack_build.py placed --recipe packs/base.json --recipe examples/wink/recipe.json --profile profiles/op1f-1.7.3.json --folder out/set --name example
```

It prints each pack's place and the checksum the loader logs for it (`its sum as linked`): when the log's number is the tool's, the pack in the unit's memory is the one you built, to the byte. On a unit with more pack files, give every one of them, so that each stands where the loader puts it.

## Limits

| What | Limit |
| --- | --- |
| The room for all packs | 16 MiB under N2, 2,031,616 bytes under N1, a pack's code, data and zeroed data together. The loader takes any pack that fits. A pack built by this repository's tools is linked in a window of 2,031,616 bytes: a limit of the tools, not of the loader. Under N1 the base alone takes 526,512 of them and the base with the four features 1,296,388, which leaves 735,228. A delay line, a loop or a big table belongs in [working memory](level-6.md#working-memory) instead, up to 16 MiB for all packs together |
| Pack files looked at | 128, and 32 under N1 |
| A pack's name | 15 characters |
| A file's name | 39 characters with `.pack` |
| Entries in a table of changes | 4,096 |
| Changes marked "only when another pack is away" | 64 a pack |
| The largest alignment inside a pack | 32 bytes |
| A picture's name | 23 characters |
| A picture | A megabyte |
| Cards the base serves | 128 under N2 with base 42 or later; 32 under N1, and 32 with an older base. With more packs that carry a card than the base serves, it serves the first ones in the order of the files' names; the others run, without their card |
| A card's folders | 8, and 64 in all |
| Texts on pages | 64 in all |

## What a change to the loader means

A pack is a file: a new one is copied onto the unit and runs at the next start. That holds for the base too: a new `base.pack` is a file copied, with no install ([the base](base.md), "A change to the base").

The loader is inside the firmware file. A change to it needs a new firmware file from the builder and a new install on every unit. For the builder that means a new loader's file under a new version name, and a manifest that describes it. You can make both on your own computer, with your own firmware file: [building](build.md), "A change to the loader", has the steps. A new version stands beside the one the builder's page offers until the maintainer has tried it on a unit and makes it the page's own.

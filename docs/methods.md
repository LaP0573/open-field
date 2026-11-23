# Methods: how to get what a pack needs from your own firmware

A pack that stands in the firmware changes it at places it names by their address, and calls functions of the firmware by their address. This page says **how such an address is found, checked and used**: from the firmware file on your own unit, with the tools in `tools/`. It is written for people and for coding assistants alike.

A pack that only uses the base, through a card, names no address of the firmware and needs none of this page: the base does that work for it ([the base](base.md)). Start there, and come here when your feature has to stand in the firmware's own code.

Two things it does not do, on purpose:

- **It does not say how the firmware works inside.** This repository holds what our code needs where it meets the firmware, and the methods to find more. A write-up of the firmware's inner workings is not published here ([CONTRIBUTING.md](../CONTRIBUTING.md), "Writing about the firmware").
- **It holds nothing of the firmware.** What these tools print is the firmware's own code and data. Read it on your screen, and keep it out of this repository: no listing in a file, no bytes in a comment. The check before a commit refuses them ([checking](checking.md)).

Every address on this page is for OS 1.7.3.

## 1. Your own firmware file

The firmware file is `os.ldr`. Your unit keeps a copy on its disk, in the folder `upgrade/firmware/blackfin/bin`. The install's helper copies it off for you, into the working folder you pick on the install page, as `os-1.7.3-yours.ldr` ([the guide](guide.md), "The install, once"). Take that one, not an `os.ldr` at the top of the disk: that is whatever an install put there.

Keep it **outside the repository**. Many tools take its path from the environment, so set it once:

```bash
export OPEN_FIELD_STOCK=<the path of your own os.ldr>
```

(In PowerShell: `$env:OPEN_FIELD_STOCK = "<the path of your own os.ldr>"`.)

Is it the file everything here was made for? This prints its size and SHA-256 and holds every place of the profile against it:

```bash
python -I tools/place_sum.py --stock <your own os.ldr> --profile profiles/op1f-1.7.3.json
```

The file is not encrypted. It is a row of **records**: each says "put these bytes at this address", and the processor's loader does that at every start. This lists them:

```bash
python -I tools/ldr_inspect.py <your own os.ldr>
```

Where things lie, as far as a pack needs it:

| Addresses | What |
| --- | --- |
| `0x11a00000` to `0x11a10000` | *L1 code*: fast memory inside the processor. The audio code runs here |
| `0x80000000` to `0x90000000` | *DDR*, the main memory: most of the code, and its data |
| `0x0800....`, `0x1190....` | Data memory inside the processor |

An address the file stores no bytes for is memory the firmware fills while it runs. Only a running unit shows what stands there.

## 2. The records as files

The reading tools take the records as a folder, one file a record:

```bash
python -I tools/ldr_dump_records.py <your own os.ldr> --out <a new folder outside the repository>
```

That folder is a copy of the firmware. It never goes into the repository. Below it is called *the records folder*.

## 3. Reading code

The toolchain has a disassembler, a program that turns bytes back into instructions. For one record:

```bash
open-xy/local/toolchain/bin/bfin-elf-objdump -D -b binary -m bfin --adjust-vma=0x<address> <the records folder>/<index>_<address>.bin
```

**What it gets wrong.** The processor is a newer Blackfin than this disassembler knows. Three instructions that the firmware uses all the time are eight bytes long and unknown to it: the load of a 32-bit value into a register, the call of and the jump to a fixed address, and the load or store of a variable at a fixed address. It prints something else for them, loses its place, and the lines that follow are wrong until it happens to fall into step again. A listing of a whole record is therefore full of nonsense around exactly the instructions that matter most.

**Use the sweep instead.** It walks the bytes one instruction at a time, decodes those forms itself and asks the disassembler only for the rest:

```bash
python -I tools/bfin_sweep.py <the records folder> 0x8c053a9c 0x8c053aac
```

How to read what it prints:

- One line is one instruction: its address, its bytes, its text.
- A line with `||` in it is one instruction made of several parts that happen together. Such a line can load and store even when it ends in `NOP`. Never filter lines out by their last word.
- `ILLEGAL` means neither the tool nor the disassembler knows the first word. The one to three lines behind it can then be wrong. Go on from the next address you are sure of, such as a place another instruction jumps to. The set-up of a loop whose count comes from a pointer register is one such form, and so is a short instruction that puts the inverse of the condition flag into a register.
- An instruction is 2, 4 or 8 bytes long, and its first 16-bit word says which. `tools/place_sum.py` has the rule in a few lines of code (`instruction_bytes`).
- A function usually begins with `LINK`, which makes its frame on the stack, and ends with `UNLINK` and `RTS`.

## 4. Finding a function by a message it logs

The firmware writes a log, and each line names the source file it came from ([the log](#11-the-log), below). That is the best way in: a message says in plain words what the code around it does.

Take a line from your own unit's log, or any text you expect, and ask where the file holds it and what refers to it:

```bash
python -I tools/ldr_xref.py <your own os.ldr> --string "ignoring screen"
```

It prints the address the text stands at, and every place in the code that loads that address. Sweep around such a place. The function it belongs to begins at the nearest `LINK` above it.

A message that is a format, with `%d` or `%s` in it, is logged with values: the instructions before the call of the logger show which registers hold them, and so what the function knows at that point. The same works for a file name, the key of a setting, or the name of a picture: any text the code uses.

## 5. Who refers to an address

Once you have an address, of a function or of data, this lists what reaches it:

```bash
python -I tools/bfin_callers.py <the records folder> 0x8c0dd488
```

It finds calls and jumps, loads of the address into a register, loads and stores of a variable at the address, and words of data that hold it. For the address above it finds one call, in the start-up: that function is called from one place in the whole firmware, which is why the example `trampolines` can stand there and run once a start.

What the answer tells you:

- **Several calls**: a function many parts use. A change at its entry changes all of them.
- **No call, and a word of data**: the function is reached through a table of functions. Change the word, not the function ([below](#7-choosing-where-a-change-stands)).
- **Nothing**: the address is worked out while the firmware runs, or it is built from two halves, which this tool does not look for. `tools/ldr_xref.py --value 0x...` finds those.

To look at data, as words, as bytes or as a text:

```bash
python -I tools/ldrmem.py <the records folder> str 0x848daec0
```

## 6. Reading a caller before you bind a function

To call a function of the firmware, your code needs its address and its type: what it takes and what it gives back. **Take the type from a place where the firmware itself calls the function**, never from a guess.

At the call, look at the instructions before it:

- The first three arguments are put into `R0`, `R1` and `R2`.
- A fourth and later ones are stored on the stack, at `[SP + 12]`, `[SP + 16]` and so on. Count them at the caller: the function itself does not show how many it takes.
- **`P0` set to the address of some memory right before the call** means the function gives back a structure, and that memory is where it goes. Bind the function as one that returns the structure and the compiler does the same. Handing the memory over as an ordinary argument crashes.
- What the caller does with `R0` afterwards shows what comes back and how a failure looks. Read that; do not assume it. In this firmware an open that fails gives back a negative number, a size of a missing file is -1, and one of the allocators never gives back "nothing": it stops the unit when it has no room.

Then find out **which thread** calls it: follow the callers up until you reach a loop that never ends, or a function that is registered as a handler. Call the function on the thread the firmware calls it on. Where you cannot tell, do not call it from the audio thread.

Bind it in two lines, with a type of your own:

```cpp
typedef int (*FileSizeFn)(const char* path);
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
```

[The firmware's functions our code uses](firmware-interfaces.md) lists what is bound already. When you add one, add its words to `profiles/op1f-1.7.3-interfaces.json` and make the page again:

```bash
python -I tools/interfaces.py
```

## 7. Choosing where a change stands

A change is one entry of your pack's table ([packs](packs.md)). There are four ways to stand in the firmware. Take the first that does the job.

**A word in a table of functions** (form `word`). Many functions of the firmware are called through a table: a key handler, a function that says which screen to show. The table's word holds the function's address, and the change puts yours there. Nothing is displaced. Your function gets the arguments the firmware's would have got, and calls the firmware's function itself, by its address, for everything it does not handle.

**A call that is there already** (form `call`). Where the firmware calls a function with the eight-byte call of a fixed address, the change makes that one call go to yours. Yours gets the same arguments, calls the firmware's function when it should, and gives back what the caller expects. Every other caller of that function is untouched.

**The first instruction of a function** (form `jump`, with a trampoline). Every caller now arrives in your routine first. It does its work and ends in a jump to its room, where the loader has put the function's displaced instructions ([a first pack](first-pack.md), "A pack that changes the firmware"). Use it when the function has many callers and all of them are meant.

**Instructions in the middle of a function** (form `call`, `jump` or `load`). The hardest. The eight bytes must be whole instructions, your routine has to do what they did, and it must keep everything the following code still reads.

What to check for any place:

- **What is live there.** At a function's first instruction: the arguments, the return address in `RETS`, and `P0` when the function gives back a structure. Nothing else. In the middle of a function: every register the code behind the place reads before it writes it. Sweep on to the end of the function to see.
- **What your routine must leave alone.** A function may change `R0` to `R3`, `P0` to `P2` and the flags. It must give back `R4` to `R7`, `P3` to `P5`, `FP` and `SP` as they came.
- **Whether a trampoline is possible.** Not when one of the instructions under the eight bytes names a place by its distance from itself. The tool of the next section tells you.
- **That nothing jumps into the middle of your eight bytes.** A place that code reaches from two sides can have nothing live in the registers a call may change. A place inside the body of a hardware loop is out: no tool here can see a loop's set-up further up that names it.
- **One place, one owner.** Two packs cannot change the same bytes: the loader leaves the later one out. The base holds fourteen places of its own ([the change list](changes.md)): a place it holds is not yours.

## 8. The checksum and the table

For each place, your profile needs its address, its length and a checksum of the firmware's own bytes there. The tool takes them from your file and prints the lines for the profile:

```bash
python -I tools/place_sum.py --stock <your own os.ldr> 0x8c0dd488 0x8c053a9c
```

For a place in code it also says where the instructions under the eight bytes end, 8, 10, 12 or 14 bytes in: that is what a trampoline copies, and a room is that many bytes and eight more. With `--word` it treats the addresses as words of data.

Write the two entries into your profile, with **your own words** for what stands at the place and for what your function does there. `examples/trampolines/profile.json` shows the form. Then the table is made, not written:

```bash
python -I tools/table_build.py --profile examples/trampolines/profile.json --all
```

The checksum is what keeps a pack from harming a unit with another firmware: the loader takes the same checksum from the unit's memory and changes nothing when it differs.

## 9. Writing the code

**C++.** There is no library behind your code ([packs](packs.md), "What a pack is, once it runs"). Give every function that another file or the table names a fixed name:

```cpp
extern "C" void op1f_tramp_seen() asm("op1f_tramp_seen");
```

**Assembly.** You need it where the firmware does not arrive by an ordinary call: at a function's first instruction, or in place of an instruction. `examples/trampolines/trampolines.s` has both shapes. The rules:

- **Keep `RETS` around a call.** A call overwrites it: push it first, pop it afterwards.
- **Give every call twelve free bytes.** The function you call may store its first three arguments in the twelve bytes above the return address, and compiled code does. `SP += -12;` before the call and `SP += 12;` after it. A routine that calls with its saved `RETS` in that place works until the day the function it calls writes there.
- **Mind the tail call.** A compiled function whose last act is a call may jump there instead. The function it jumps to then uses *your* twelve bytes. So the rule above holds even when the function you call looks harmless.
- **Keep what is live** (section 7), on the stack, around your own work.
- **Copy no instruction of the firmware into your routine.** That is what the trampoline is for.
- A room is zeroed data: `.section .bss`, a name, `.space` of the copied bytes and eight more.

This checks every call of a hand-written routine for the twelve bytes, and reads the functions that are called without them:

```bash
python -I tools/stub_calls.py --listing build/example_trampolines_placed.dis.txt --all examples/trampolines/trampolines.s
```

(The listing comes from `tools/pack_build.py placed`: [packs](packs.md), "Reading the placed code".)

**The audio thread** makes a block of 64 samples some 690 times a second and must never wait. Code that runs there writes no log line, opens no file and asks for no memory. It counts, and a frame function prints the count. A card's input function may be called there too, for a key of the keyboard, so it keeps to the same rules ([the base](base.md), "The threads").

## 10. Code of the firmware is changed through the table only

A pack never writes into the firmware's code itself. It is not a matter of style:

- Code in the main memory sits behind the processor's caches. A change has to be written out of one cache and dropped from the other, or the processor goes on running the old bytes.
- Code in the fast memory inside the processor cannot be written by an ordinary store at all.
- A change that is half in is worse than none.

The loader does all of it: it checks every place of a pack by its checksum first, writes each in the way its memory needs, reads it back, and makes a pack's changes all together or not at all. So every change of the firmware is an entry of a table.

## 11. The log

The unit writes a log into the folder `logs` on its disk: a new file at every start, named by the date and time, and a further one whenever a file is full. It keeps a limited number and removes the oldest. [A first pack](first-pack.md), step 4, says how to copy one off the unit.

A line looks like this:

```
   1.089.800: I trampolines[1]: trampolines: the draw of the version text came through its trampoline 1 time(s), and this is call 1 through the second; the rooms' sums are 6dc09fde and 096568b4
```

The time since the unit was switched on, in seconds, thousandths and millionths. A letter: `I` information, `W` a warning, `E` an error. A tag, which is the name of the source file the line comes from, and a number. Then the text.

How to read one:

1. **The first lines say what runs.** The lines with the tag `op1f_loader` are the loader's account of the start: which pack files it found, where it placed each, what it left out and why ([packs](packs.md) has every line). The base's lines follow, with the tags `op1f_cards`, `op1f_folders` and `op1f_text`: which cards it serves, which folders it made, what a page and a file did ([the base](base.md)).
2. **Then search for errors**: `: E `. The firmware writes a few of its own at every start. Learn which by reading a log from before your change, and look for the ones that are new.
3. **A crash is in the log.** When the processor stops on a fault, the firmware writes down the kind of fault and, as `RETX`, the address of the instruction that failed. If that address lies in a pack, read it against the pack as placed ([packs](packs.md), "Reading the placed code").
4. **A freeze is not.** A unit that hangs writes nothing more, and the log of that start can be empty. Then the files your code wrote before it hung are the only witness.

A log is a record of your unit. It does not go into the repository, and the check refuses one. Quoting a few lines in a page or a pull request is fine.

When your own code writes lines: a line is cut at about 260 characters, and a line that comes every few seconds fills the log. Print a count once it has changed, not every time ([the base](base.md), "The lines the base writes into the log").

## 12. Trying a pack safely

1. **One change at a time.** Build, and look at what you built: `tools/pack_build.py check` prints its changes and its names.
2. **Copy what you care about off the unit first.** A pack that touches the tape or the files can damage them.
3. Copy the pack into `packs`, restart, look at the start screen, read the log. `(1 left out)` on the start screen means the loader refused the pack: the log says why, and nothing of it ran.
4. **If the unit does not come up**, or behaves wrongly: hold the **help** key while you switch it on. That start reads no pack. Delete the file in the disk mode.
5. The help key does not cover a loader that fails. For that, and for the way back to the original firmware, see [the guide](guide.md).

When you report what you tried, say what you did on the unit, with which pack files on it, and what you did not try. A change that has only been built is welcome too, as long as it says so.

## 13. Moving the work to another firmware version

Everything here is tied to one firmware file by addresses: the places of the profiles, and every function and variable the sources bind. Another version of the firmware has the same code at other addresses, and some of it changed.

**Today only OS 1.7.3 is supported** (the original, `1.7.3+0`). The release before it was for OS 1.7.0: it stays in the history of this repository, at the commit `b6501e4`. Every source of `module/` was moved from 1.7.0 to 1.7.3 address by address, with the method below, and each one says so on its first line. A unit on a later version cannot use open-field yet.

A port needs the plain `os.ldr` of both versions, each copied off a unit that runs it. This looks, for every place of a profile and every address the sources name, for the same bytes in the other file:

```bash
python -I tools/os_port.py --source <your own os.ldr> --self-check
```

```bash
python -I tools/os_port.py --source <your own os.ldr> --target <the os.ldr of the other version> --out <a new folder outside the repository>
```

The first holds the tool against itself: every place must be found once, where it is. The second sorts each place and address into *once* (found at one clear place), *many* (the same bytes stand at several places: a person picks) and *none* (the code there changed: find it again by the methods above). Addresses the file stores no bytes for are listed as *carry*: find each again through the code that uses it. Its report holds short runs of the firmware's bytes, so keep it outside the repository.

What is left for a person after the tool:

- Each moved place gets a new checksum from the new file (section 8), and the count of bytes a trampoline copies is walked again: the instructions under a change can end elsewhere.
- Each routine that stands in the middle of a function is read against the new code around it.
- What our code reads inside the firmware's own objects, such as the word of a screen object that holds its picture, is checked by no tool.
- The loader holds the number of the firmware it is for, and so does every pack's header. The base, too, stands at places of the firmware and binds its functions: it moves with the rest.
- The install is a question of its own: the pages check the bootloader's version too.

## 14. What never to do

- Never put anything of the firmware into this repository: not the file, not a record, not a listing, not bytes in a comment, not a log, not a picture of the device.
- Never write into the firmware's code from a pack (section 10).
- Never touch the bootloader. It is the way back when everything else has failed.
- In the bootloader's developer menu, press only the keys a page here names: one of the others erases the unit's recordings without asking.
- Never try a pack on a unit whose owner has not agreed to the risk.

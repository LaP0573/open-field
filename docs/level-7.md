# Level 7: values kept in tapes and sounds

Up to level 6 a pack keeps its settings in its own folder, the same for every tape and every sound. Level 7 lets you keep values **with a sound** and **with a tape**, inside the unit's own files. Load another tape and your pack gets that tape's values. Save a synth preset and your values go into the preset with it. Copy a tape and the copy carries them along.

Each value is a member of its own at the top of the unit's file, under a name your pack picks, and you read and set it as JSON text. The base never touches a member the unit's own file holds, so a file with your values in it is the unit's file with something added, and a unit without your pack, or without open-field, reads it as it always would.

Level 7 also brings a page that stays with its mode while the unit visits another one (below). Any pack may use that, at any level.

The words of the interface are in the header `module/build_r6e_card.h`. Where this page and the header disagree, the header is right.

## The example: `dub`

[`dub`](../examples/dub/README.md) is the echo of [level 6](level-6.md). Its three values (how far behind the echo comes, how much comes back, how loud) go with each tape, as a member called `dub_echo`:

```json
"dub_echo": {"back":3,"level":0,"time":6,"version":1}
```

What that means when you play:

- **Load a tape** and `dub` takes that tape's values. Turn a knob and the tape's values change with it, and they're written into the tape's file the next time the unit writes the tape. Load another tape, come back, and your echo is as you left it.
- **A new tape** has none of `dub`'s values, so it gets `dub`'s first values: dry, as the unit sets its own for a new tape.
- **A copy of a tape** carries the original's values, so it keeps its echo as it keeps its sound.
- **A tape from before** `dub` had any of this takes the values of `dub.bin`, `dub`'s own file, and keeps them from then on.

The `version` inside is `dub`'s own, for its own form. A later `dub` that keeps another form can tell an old value from a new one. Do the same in yours.

## The ask

A card that keeps values says level 7, and has one ask of the kind `keeps`. This is `dub`'s:

```json
{ "kind": "keeps", "name": "dub", "changed": "dub_changed",
  "keys": [ { "name": "dub_echo", "files": [ "tape" ] } ] }
```

| Field | What |
| --- | --- |
| `name` | What the base's lines call the ask |
| `changed` | Your function the base tells when something happened to the values (below), or null |
| `keys` | Up to eight members, each with its `name` and the `files` it's kept in: `"synth"`, `"drum"`, `"tape"`, any of them, and from level 8 `"master"` |

One ask of this kind a card. The card's source makes `op1f_level_7` a name your pack needs.

**A member's name** is 1 to 31 characters of `a`-`z`, `0`-`9` and `_`. A name the unit's own file already uses is never written, and the pack tool refuses it. Pick a name that's clearly your own, your pack's name and a word, as `dub_echo` is. Our own four packs use `op1f`, `op1f_mixer_ext`, `op1f_track` and `op1f_duck`. The base keeps two of its own, `op1f_effect` and `op1f_engine` ([level 8](level-8.md) and [level 9](level-9.md)), and the pack tool refuses those too. When two packs ask for the same name, the earlier file keeps it and the later ask's key is left aside, with a line.

**Where a value is kept** (`where` in the helpers and in `changed`):

| | In the header | Kept |
| --- | --- | --- |
| `"synth"` | `OP1F_KEEP_SYNTH` | With the synth's sounds: in a synth preset, and with each of the eight slots, slot by slot |
| `"drum"` | `OP1F_KEEP_DRUM` | The same for the drum |
| `"tape"` | `OP1F_KEEP_TAPE` | With the tape, in its own file |
| `"master"` | `OP1F_KEEP_MASTER` | With the master's effect, in the base's own store, since the unit keeps no file of the master's a pack may write in. A card of level 8 or later only ([level 8](level-8.md)) |

`which` says the slot, 0 to 7, and is 0 for the tape and the master.

## What a value can be

Whole numbers from -2147483648 to 2147483647, `true` and `false`, `null`, texts (with no zero character in them), and lists and objects of these, nested up to four deep. An object names each of its members once.

- **No fractions.** A number with a fraction or an exponent isn't kept: `op1f_value_set` answers `OP1F_BAD_VALUE`, and a member in a file that holds one isn't taken from the file. Keep such a number as a whole number of your own units, or as a text.
- An object's members come back in the order of their names once the value has been through a file. Don't count on the order you wrote them in.
- One value's text is at most 2,048 bytes. All the values of one slot, or of the tape, take at most 4 KiB in the base, about as much as their text. Beyond that, `op1f_value_set` answers `OP1F_TOO_LARGE`.

## Being told: `changed`

```cpp
extern "C" void dub_changed(unsigned where, unsigned which, unsigned how) {
    if (where != (unsigned)OP1F_KEEP_TAPE || which != 0 || how > 31u) return;
    told |= 1u << how;
    if (how == (unsigned)OP1F_KEPT_LOADED || how == (unsigned)OP1F_KEPT_MADE) last_load = how;   // of two loads in one frame, the later counts
}
```

The base calls `changed` on the display thread, before the frame functions of a frame, and outside your own frame function. Like level 5's `shown`, **it notes what it's told and does nothing else**. Your frame function then reads the values. `how` is one of:

| `how` | What happened |
| --- | --- |
| `OP1F_KEPT_LOADED` | A sound was loaded onto the slot from a preset, or the unit switched to a tape |
| `OP1F_KEPT_MADE` | (The tape only) a tape the unit has just made was loaded: a new tape (the tape list's T3), which holds no value, or a copy (shift and T3), which holds the original's values as its file does |
| `OP1F_KEPT_ENGINE` | The slot's engine changed. The values stay the slot's |
| `OP1F_KEPT_SAVED` | The values were written: into a preset of the slot's sound, or into the tape's file |
| `OP1F_KEPT_START` | The start: the slot's values as the base kept them, or none |
| `OP1F_KEPT_DROPPED` | The start: the values the base kept for the slot didn't fit the sound on it any more, and were let go |

**The sound list loads as the owner scrolls.** The unit loads the sound under the cursor at every row the cursor passes, so while the owner walks the sound list, `changed` can say `OP1F_KEPT_LOADED` for the same slot at frame after frame. Read the values each time, as after any load, and keep that work small.

**`OP1F_KEPT_MADE` comes in place of `OP1F_KEPT_LOADED`** for a tape the unit just made. Read the values as after a load. Then set whatever your pack keeps of the tape's own state the way the unit sets its own for a new tape or a copy. The unit puts tracks 1 to 4's levels and pans back to where they start, for instance, and `8 tracks` does the same for tracks 5 to 8. A tape you switch to (`OP1F_KEPT_LOADED`) is taken as it was left.

## Reading and setting

| Helper | What it does |
| --- | --- |
| `op1f_value_get(card, where, which, key, out, room)` | The value's JSON text, ended with a zero, written into `out`. Answers its bytes |
| `op1f_value_set(card, where, which, key, text)` | The value set from a JSON text, or taken away when `text` is 0. Answers 0 |

Both are your frame function's alone. What they answer when they don't do it:

| Answer | `op1f_value_get` | `op1f_value_set` |
| --- | --- | --- |
| `OP1F_NO_FILE` | No value there | |
| `OP1F_TOO_LARGE` | `room` too small, nothing written | The text is over 2,048 bytes, or the slot's or the tape's 4 KiB would be passed |
| `OP1F_BAD_VALUE` | | Not JSON, or a kind of value the base can't write |
| `OP1F_BAD_NAME` | A key your ask doesn't list for that kind of file | The same |
| `OP1F_NOT_NOW` | Not from your frame function | Not from your frame function, **or a tape was just loaded and your pack hasn't been told yet**, or another thread wrote the slot in the meantime |

### When the base says not now

When the unit switches tapes, there's a moment where a tape is loaded but `changed` hasn't told your pack yet. In that moment `op1f_value_set` answers `OP1F_NOT_NOW` for the tape. Otherwise a value meant for the old tape could land in the new one.

**Try again at the next frame, and say nothing.** At the next frame `changed` tells you about the load first, you read the new tape's values, and then you set yours. The answer is expected at every tape switch, so don't put it in the log and don't fall back to some other way of keeping the value. `dub` does exactly this:

```cpp
static void tape_keep() {
    if (!keeps.tape || keeps.load_waits) return;              // nothing set over a tape whose load is still to be taken
    if (keeps.held && keeps.held_time == state.time && keeps.held_back == state.back && keeps.held_level == state.level) return;
    text_kept(state.time, state.back, state.level);
    int answer = op1f_value_set(&op1f_card_dub, OP1F_KEEP_TAPE, 0, KEPT, keeps.text);
    if (answer == OP1F_NOT_NOW) return;                   // not now: set at the next frame, nothing said
    if (answer != 0) {
        keeps_off(answer);
        state.changed = 1;                                // dub.bin keeps them now: written at the page's close
        return;
    }
    ...
}
```

It handles the read the same way. After a load it asks for the tape's value, and on `OP1F_NOT_NOW` it keeps the load waiting and asks again at the next frame:

```cpp
static void tape_loaded(int made) {
    int got = op1f_value_get(&op1f_card_dub, OP1F_KEEP_TAPE, 0, KEPT, keeps.text, sizeof keeps.text);
    if (got == OP1F_NOT_NOW) return;                      // not now: asked again at the next frame, nothing said
    ...
}
```

Any other refusal is a real one. `dub` then goes back to `dub.bin` and says so once.

## When values reach the files

**You can't make the unit save.** Your values are written when the unit writes the file, exactly like the unit's own members: into a preset when the owner saves it, into the tape's file when the unit writes the tape. Set a value and it waits, held by the base, until then. `OP1F_KEPT_SAVED` tells you it's been written.

For the sounds, the base also keeps each slot's values across a start, in a small file of its own, `user/pf/.sounds`. At the start you're told `OP1F_KEPT_START` for each slot that has values. If the owner loaded another sound onto a slot while your pack or the base was away, the values don't fit it any more: they're let go and you're told `OP1F_KEPT_DROPPED`.

**Taking a value away** (`op1f_value_set` with a text of 0) means it isn't written again. The base adds members to a file and never removes one, so a file that already holds the value keeps it. To make a tape forget a value, set it to `null`.

## When your pack, or the base, is away

- **Your pack taken off the unit.** The base keeps your members with the sounds by the names it knows: the keys of this start's asks, and up to 64 names it remembers from earlier starts. A preset loaded and saved again while your pack is away keeps your member. A tape keeps it too.
- **The base away** (the help key held at power-on, or open-field taken off). The firmware reads its files as it always does and ignores members it doesn't know. Nothing of the unit's own is lost.
- **Never worse than the firmware alone.** The base never sets, changes or removes a member the unit's own file holds. It writes only the members your ask lists, from text it checked when you set it, and a write it can't make whole isn't made at all, with a line in the log.

One rare case to know about. If the unit was already saving the old tape at the very moment it switches tapes, one of your values from the old tape can land in the new tape's file. The base puts it right when the new tape has its own value. If the new tape had none of yours, the stray value stays until your pack sets that value again, and the log has a line for it. Set your values again after every load, as `dub` does, and it never matters.

## A page that stays with its mode

By default a mode change closes your page. Show it with the flag `OP1F_PAGE_STAYS` and it stays with its mode instead:

```cpp
op1f_page_show(&op1f_card_mine, document, OP1F_PAGE_STAYS);
```

- **When the unit goes to another mode**, your page isn't closed. While the unit is elsewhere the frame's `page` is 0 and keys and knobs go as they go with no page. When the unit comes back to your page's mode, your page is shown at once, with no frame of the mode's own screen in between. The synth key back from the mixer shows your page, not the synth's first page.
- **While it waits in another mode and something else takes the screen** (another pack's page, or a page standing in for the sequencer's screen), your page is set aside. When that page goes, yours waits for its mode again, and the unit coming back to your mode shows it at once, straight from the other page too.
- **When a page that stands in is due in your mode as well** (a pack's effect page on T3, an engine's page on T1), your page comes back on top and the stand-in lies under it, as the two lay before the unit left.
- **It's closed**, with a line in the log, by your own `op1f_page_close` in any mode, when the instrument changes while it waits (a page of the synth's doesn't come back after the drum), and when another staying page is set aside in its place.
- **One page stays at a time**: the page on top when the mode changes, if it was shown with the flag. Pages kept under it close as usual. A page shown over it in its own mode lies over it like over any page.
- The base's own confirm page can come over it and leaves it as it found it.
- A page your own pack asks for while yours waits becomes your page from then on, in the mode the unit is in, and stays with that mode only if you asked with the flag again.

The flag needs no mark. Our effect train shows its page this way, which is why it's back the moment you return from the mixer or the tape.

## The lines the base writes

| Line begins | What it says |
| --- | --- |
| `keeps: '<pack>' keeps <n> value(s) in <files>` | Your ask is in use, once a start |
| `keeps: '<pack>' asks to keep '<name>', which ...: left aside` | A key the base won't keep, and why |
| `keeps: '<name>' of '<pack>' written into the tape (<n> bytes)` | Your value reached the tape's file (a sound's line names the preset or the slot) |
| `keeps: '<name>' of '<pack>' not written into ...: <why>` | A write the base wouldn't make |
| `keeps: '<name>' of '<pack>' was not taken from a file: <why>` | A member in a file the base couldn't read as a value |
| `keeps: the tape loaded as a new tape or a copy` | `OP1F_KEPT_MADE`, with how many values of packs were in the file |
| `keeps: '<name>' of '<pack>' of the tape before may have reached the tape loaded during its save` | The rare case above |
| `keeps: <n> slot(s) of sounds with values read from the base's file, <n> let go` | The start: the slots' values kept, and those dropped |
| `page: '<pack>' stays with mode <n> while the unit is in mode <n>` | Your page is waiting for its mode |
| `page: '<pack>' shown again in mode <n>, as it stayed` | It's back |
| `page: '<pack>' set aside while another page takes the screen in mode <n>` | Another page took the screen while yours waited |

## Files per tape (base 43)

A value of level 7 is at most 2,048 bytes, and all the values of a tape at most 4 KiB. Some packs keep more for each tape: the notes of a tape's MIDI tracks, a tape's markers, a list of loops. Such a pack keeps them in **files of its own folder**, one file or several for each tape, named `tape<N>.<anything>`, `N` being the tape's number, 1 to 8. The unit knows nothing of these files: when it copies a tape, the copy would start with none; when it deletes one, a tape made later in its place would find the old ones. One ask has the base keep them with the tapes:

```json
{ "kind": "tapes", "name": "notes", "folder": 0, "changed": "notes_tapes" }
```

| Field | What |
| --- | --- |
| `name` | What the base's lines call the ask |
| `folder` | The number of the card's folder ([level 3](base.md#folders-none-one-or-several), from 0) whose `tape<N>.*` files follow the tapes |
| `changed` | Your function the base tells when it carried a copy or a delete into the folder, or null |

One ask of this kind a card, on a card of level 7 or later. Its words are at the end of `module/build_r6e_card.h`: `OP1F_ASK_TAPES`, the table `Op1fTapes`, `OP1F_TAPES_COPIED` and `OP1F_TAPES_DELETED`, and the mark `op1f_tapes_1`.

What the base does:

- **The unit copies a tape** (shift and T3 in the tape list): every `tape<from>.<rest>` of the folder is copied to `tape<to>.<rest>`, and a `tape<to>.<rest>` the source has not is removed. A copy is written as a write is, to `tmp/op1f_tape.part` and renamed, so a file is whole or not there.
- **The unit makes a tape anew** (T3 in the tape list on an empty slot, say after a delete, or a unit's first tape): every `tape<N>.<rest>` of the folder is removed, so the new tape never finds a deleted one's files. A delete itself leaves them until then; nothing reads them meanwhile, since no tape stands there.
- Other names are left alone: `tape10.txt`, `song.mid`, a folder inside.

The base does it on the display thread, at its frame, behind the values of level 7 and before any frame function, and never while the disk mode is open: a tape made while it is waits for it to close. A tape deleted from the computer has its files removed when a tape is next made in its place. A file of any size is copied, a piece of 4 KiB at a time, in that one frame.

```cpp
static volatile unsigned tapes_told;
extern "C" void notes_tapes(unsigned how, unsigned from, unsigned to) { tapes_told = 1; }   // notes it, does nothing else
```

`changed` is called right after, before the frame functions of that frame, and outside your own: `how` is `OP1F_TAPES_COPIED` (tape `from` was copied to tape `to`) or `OP1F_TAPES_DELETED` (tape `from` was made anew, so whatever a deleted tape there had is gone; `to` is `from`), the tapes numbered 1 to 8. Like `changed` of the values, **it notes what it's told and does nothing else**. Your frame function then reads the mounted tape's file again, and drops what it holds unsaved for that tape: a write would bring its file back. The tape list takes the mounted tape out before it copies and loads the copy afterwards, so a pack that reads its file when the mounted tape changes finds the copy's files there.

**Which to use.** A few numbers for each tape: a value of level 7, kept inside the tape's own file, which the unit copies itself. More than that: files per tape.

**Under base 42 and earlier**, which does not know the kind, the ask is left aside with the base's line for kinds it does not know, and the card is used: the pack runs, and its files do not follow the tapes. A pack that must know imports `op1f_tapes_1` as optional (a weak name in its source): it is 0 under such a base.

The lines:

| Line begins | What it says |
| --- | --- |
| `tapes: '<pack>' tape <n> copied to tape <n>: <folder> <n> file(s) copied, <n> removed, <n> NOT done` | A copy carried into the folder |
| `tapes: '<pack>' tape <n> deleted: <folder> <n> file(s) removed, <n> NOT done` | A tape made anew: its slot's old files removed from the folder |
| `tapes: '<pack>' asks for its folder <n>, which it does not have: left aside` | The ask names a folder the card has not, or one the base did not give it |
| `tapes: <n> tape event(s) passed over` | More than sixteen copies and tapes made between two frames |

The base finds the copy at the entry of the unit's own function, through one place of its table ([the change list](changes.md)), and a tape made anew from the tape load the values of this level already watch, with no place of its own. The copy of tracks 5 to 8 by `8 tracks` stands at another place, the tape list's call of the copy, and both work together.

Why one place and not two: a base's door judges a new `base.pack` against every pack whose file sorts before it (`8 tracks.pack`), and the door of bases 33 to 36 reads at most 64 places of a base. A base it cannot read whole it holds to clash, and refuses the pack: "it clashes with the base, so the base would be left out at the start". Base 42 has 64 of its own, so a base with this one has 65: a unit on base 36 or older gets it with the install page's whole install, as it gets base 40 and later, and not through its door.

## Limits, in one place

| | |
| --- | --- |
| Asks of this kind | One a card |
| Keys | Eight an ask, 64 names the base remembers in all |
| A member's name | 1 to 31 characters of `a`-`z`, `0`-`9`, `_` |
| A value | 2,048 bytes of JSON text, nested up to four deep, whole numbers only |
| The values of one slot or of the tape | 4 KiB |
| Pages that stay | One at a time |
| Asks of files per tape (base 43) | One a card, one folder each, files `tape1.*` to `tape8.*` |

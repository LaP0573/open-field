# A drum engine (base 40)

At [level 9](level-9.md) a pack adds a synth engine that rides the unit's `dsynth`. From base 40 a pack can add an engine to the **drum's** engine list too (shift and T1 in the drum's mode, where the unit offers the drum sampler and `dbox`). A drum engine rides the unit's **drum sampler** and makes no sound of its own: under it the drum sampler plays on as always, with its kit, its sample, its keys' points, and the unit saves and loads the kit as it did. What is yours is the engine's name in the list and in the sound's file, and its page on T1, with the keys and knobs offered to it first. A pack that works on the kit (one that cuts the kit's sample into its keys, say) is such an engine.

This is not a level. The level stays 10. Base 40 adds one kind of ask, `OP1F_ASK_DRUM_ENGINE` (12), on a card of level 8 or later, and a mark, `op1f_drum_engine_1` ([below](#the-mark)).

The words of the interface are in the header `module/build_r6e_card.h`, the section "Base 40: a drum engine". Where this page and the header disagree, the header is right.

## What the owner meets

- **The drum's engine list** (shift and T1 in the drum's mode) shows your engine's name among the unit's own. The unit sorts the list by name, as it sorts the synth's, so your engine stands where its name falls, not at the end. Its row has no sounds under it.
- **Picked on a drum sound that plays the drum sampler**: the sound stays as it was (the same kit, the same keys), and T1 shows your page. Until your frame function gives a document with `op1f_page_stand_in(card, ask, document)`, it is the base's blank page with your ask's name, for you to paint on (a document of 0), as for an engine's page.
- **Picked on a drum sound that plays `dbox`**: nothing changes, and the base says a line. Pick the drum sampler (one of its kits) on that sound first, then your engine.
- **Keys and knobs** go to your input function first, as for any page: the four knobs, shift, and the keyboard's keys. What you don't take goes to the unit as it would on the drum sampler's own page: a key plays its drum sound, a knob turns what the unit's page turns.
- **Another screen** (T2 to T4, shift and T1, another mode) takes your page off as any page that stands in, and T1 brings it back at once.
- **As the list closes** the unit loads a kit of the drum sampler on that sound (seen on the unit, some 0.1 to 1 s after the pick). A kit loaded, or a kit picked, on that sound within about 2.5 s of your pick keeps the ride; after that, it ends it.
- **The ride ends** when the owner picks the drum sampler (one of its kits) or `dbox` in the list on that sound, or loads a sound there that does not name your engine. T1 shows the unit's own page again.
- **The sound's file** keeps your engine's name in the base's member `op1f_engine`, as a synth sound that rides a pack's engine does ([level 9](level-9.md)). Loaded again, the sound rides your engine again. Without your pack, or on a stock unit, the same file loads as the drum sampler with its kit and its keys' points: the unit's own members are untouched.
- **`shown(where, how)`** is told when your page comes up and goes: `where` is the drum sound's place, `OP1F_PLACE_DRUM + slot` (8 to 15), `how` is `OP1F_SHOWN_UP`, and 0 when the page leaves the screen.

Each of the eight drum sounds rides your engine or not on its own: picking it on one drum sound changes no other.

## The ask

```cpp
static const Op1fDrumEngine chop_engine = {{sizeof(Op1fDrumEngine), "chop", 0, chop_shown}};
static const Op1fAsk asks[] = {{OP1F_ASK_DRUM_ENGINE, &chop_engine, sizeof chop_engine}};
```

In a recipe, the card's ask is `{"kind": "drum engine", "name": "chop", "shown": "chop_shown"}` (`shown` may be `null`); `tools/pack_build.py card` writes the table and the word that names the mark.

The table is level 5's head, 28 bytes: `name` 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_`, never one of the unit's own (`dbox`, `drum`, `drums`, `sampler`, `drum sampler`, `drum_sampler`, `synth`) nor a name the drum's list already shows; `flags` 0; `shown` your function or 0. It names no address of the unit's.

**One pack holds it.** The first card in the list's order (the packs' file names) with such an ask holds the drum engine. The ask of any other card is left aside with a line ([below](#the-lines-the-base-writes)), and that pack runs without it. Ask for it only if your pack needs it.

## Beside the other pages

The drum engine's page uses the same place in the base as an engine's page (the engine's entry of the table of screens): an engine's page is the synth's and the drum engine's is the drum's, so the two never stand in one mode. A page of the whole mode (a page you open with `op1f_page_open`, a sequencer's page that stands in for the sequencer's screen) goes over it as over any page that stands in for one screen. The drum's effect page (T3) and LFO page (T4) of a pack are told apart by their screens: each has its own.

## The mark

A pack with an ask of this kind names `op1f_drum_engine_1` (a constant 1), so that a base before 40 leaves it out. A base before 40 that loads a pack without naming the mark leaves the ask aside with its line for kinds it does not know.

The base built at e005db8 knew kind 12 as "the drum's page" (a page on every drum sound that plays the drum sampler, mark `op1f_drum_page_1`). That form was never released; base 40 has the drum engine in its place, and a pack built for the page is left out (its mark is not offered).

## The lines the base writes

| Line | What it means |
| --- | --- |
| `drum: '<name>' of '<pack>' rides the drum sampler, entry 32 of the drum's engine list` | Your engine is in the list, once a start |
| `drum: the drum engine of '<pack>' (its ask <n>) is left aside: <why>` | Your engine is not in the list: its name is one the list shows or cannot show, or the unit's catalog has no list of drum engines or no row for `dbox` |
| `drum: '<name>' picked on drum <n>, which plays another engine: pick the drum sampler there first` | Picked on a drum sound that plays `dbox`; nothing changed |
| `drum: drum <n> rides '<name>'` | That drum sound rides your engine now (picked, or loaded from a file that names it) |
| `drum: drum <n> plays the drum sampler's own page again` | The ride on that drum sound ended |
| `drum: drum <n> keeps '<name>' over the kit loaded just after the pick` | The list loaded a kit of the drum sampler as it closed (within about 2.5 s of your pick); the ride stays, and your engine's name goes into that kit's sound |
| `cards: '<pack>' asks for a drum engine (its ask <n>) that is left aside: '<other>' holds it` | Another pack, earlier by file name, holds the drum engine |
| `cards: '<pack>' asks for a drum engine (its ask <n>) that is left aside: its table is not a head` | The ask points at nothing, or at fewer than 28 bytes |
| `page: '<pack>' stands in for the drum engine's page of drum <n>, its document at 0x...` | Your page went up |

The `drum:` lines come from `../src/main/op1f_drum.cpp` in the log; at most 32 of the last three kinds a start.

## A lifted cut dropped into the drum sound

Your engine may drop what the tape has lifted into the drum sound it rides, with the base's [sample helpers](sample.md): `op1f_sample_drop(card, ask, OP1F_PLACE_DRUM + slot)` from your input function (the drop key on your page, key 11), then watch `done` with `op1f_sample_get` in your frame function. The unit does the drum sampler page's own drop in its own turn. It changes one more place of the firmware, the head of the drum's drop job (`0x8f00ba38`, [the change list](changes.md)): a drop no pack asked for goes on there as before.

## What the base reads and changes of the firmware

One place of the firmware is changed: the entry of the drum's engine pick, the function the drum's engine list and MIDI call with the id picked and the drum sound ([the change list](changes.md), `0x8d4eb160`). A pick of your engine on a sound that plays the drum sampler goes no further: the firmware makes no pick, and the sound stays as it was. Every other pick goes on to the firmware's function as before, after the base has ended the ride. The base adds your engine's row to the catalog's `drum_types` when it reads the catalog, as it adds a synth engine's to `synth_types` (the place it already had for level 9), and leaves it out of the catalog it writes back. The member `op1f_engine` of a drum sound is kept by the base's values unit, which already reads and writes the base's members in the drum's files. It reads the drum sound the owner sees and the engine type each drum sound plays, and, once, the drum's own function that answers the screen of an engine type ([the firmware's functions](firmware-interfaces.md)).

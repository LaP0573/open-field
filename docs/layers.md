# Layers: a row of your own over the unit's own effect or LFO (bases 35 and 37)

An effect of [level 8](level-8.md) stands on one of the unit's effects and takes its place: where it plays, the unit's effect is switched off. A **layer** does not take a place. It is a row of your own in one of the unit's lists, and where a sound rides it, the unit's own effect or LFO stays under it and plays as always, whatever the owner, a pack or MIDI puts there. Your pack's work comes on top.

- **Base 35** adds layers: an effect may be a layer, in the unit's lists of effects, and a new kind of ask, `lfo`, is a layer of a sound's list of LFOs. Three helpers start and end a ride and choose the page.
- **Base 37** lets drum sounds ride an LFO layer too.

Neither is a level. The level stays 10, and a card with a layer names the mark `op1f_layer_1`, so the loader leaves it out under a base before 35.

Our `lfo` in [open-field-packs](https://github.com/LaP0573/open-field-packs), from its version 12, is an LFO layer: its row `ufo` in the LFO list of every synth and drum sound brings its four LFOs. Read it for a whole pack built this way.

The words of the interface are in the header `module/build_r6e_card.h`, the section "Base 35: layers". Where this page and the header disagree, the header is right.

## What the owner meets

Say your layer is an effect called `halo`. On a sound or on the master:

- **The list.** Shift and T3 opens the unit's list of effects (on the mixer, the master effect's list). `halo` shows among the unit's own and the packs' effects, by its name, for every synth and drum sound and for the master. While a place rides it, the list opens with its cursor on `halo`.
- **The pick.** The unit's list picks as its cursor moves, so walking to `halo` passes other rows and puts each on the place. When the cursor stops on `halo`, the place rides it, and the base puts back under it the unit's own effect as the place held it when the list opened: its type, its knobs and its switch. Walking off the row ends the ride. Coming back starts it again, over the effect the list opened on.
- **The sound.** The unit's own effect plays as always: its own key switches it, the knobs are its own, the unit's LFOs move them. Then your `process`, once a block where the place plays.
- **Under the ride**, the unit's effect may change: the owner on its own page, another pack or MIDI may put another of the unit's effects there. The ride goes on over the new one.
- **The end.** A ride ends when another row of the list is picked, when a sound is loaded onto the place without the layer in it (or with another), or when your pack calls `op1f_layer_end`.
- **The file.** The unit's own effect is saved as the owner left it, its switch too: never forced off. The base's member `op1f_effect` in the sound names the layer. The master's ride is kept in the base's store.
- **The page.** T3 shows the unit's own effect's page, as if no layer were there, until your pack chooses its own with `op1f_layer_page` (below).

An LFO layer is the same in the sound's list of LFOs: shift and T4 for the list, T4 for the page, the unit's own LFO under it as the place held it when the list opened (its kind, its values, its switch), and the member `op1f_lfo` in the saved sound, beside `op1f_effect` and `op1f_engine`.

**Without your pack**, under a base before 35, or on the original firmware, the sound plays the unit's own effect or LFO as it was saved. With open-field and without your pack, the log says so once a start (below). When the pack comes back, a sound that still names the layer rides it again.

## For every ride, from base 35 on

The unit's list of effects (shift and T3, and the mixer's) opens with its cursor on the row of the pack's effect the place rides. An older base opened it on the row of the unit's effect that pack's effect stands on. Your pack asks for nothing: it is the base's, for level 8's effects too.

## An effect that is a layer

An ask of the kind `effect` with `"layer": true`:

```json
"asks": [
  { "kind": "effect", "name": "halo", "layer": true, "process": "halo_process", "cycles": 30000,
    "reset": null, "shown": "halo_shown" }
]
```

The table is level 8's `Op1fEffect` with the flag `OP1F_EFFECT_LAYER` in its head, read so:

| In the recipe | For a layer |
| --- | --- |
| `name` | Level 8's rule: 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_`, none of the unit's eleven effects. Not the name of one of the unit's LFOs or of a pack's LFO layer either |
| `rides` | Left out: a layer stands on none |
| `params` | Left out: no values. The knobs are the unit's effect's, so `op1f_param_get`, `op1f_param_set` and `op1f_param_heard` answer `OP1F_BAD_NAME` for a layer, and the LFO pages name the unit's effect's knobs |
| `process` | A function, or null. With null, the layer is a row, a ride, a member and a page, and its sound work, if any, is the pack's own: a stage of [level 6](level-6.md), or an effect it runs through `op1f_rack_run` |
| `cycles` | 0 or left out with no `process`, else as level 8 |
| `reset`, `state_bytes`, `state_align`, `shown` | As level 8 |
| `mono` | As level 8. `tails` means nothing for a layer: its `process` is called at every block where the place plays |

**What `process` is handed.** Level 8's `Op1fEffectBlock`, with two words read so:

- `on` is 1 when the unit's effect under the layer ran in this block (it is switched on), 0 when it did not. Your `process` comes after the unit's effect when that is on, and where it would have played when it is off.
- `values` is 0.
- `fresh` is 1 at the first call after a ride began (a pick, a load, `op1f_layer_start`, a start) and after the unit's effect under it was made anew (another of its effects put there, another sound played).

`op1f_riding` and `op1f_rack_at` answer for a layer as for any effect. `op1f_rack_ready` answers `OP1F_BAD_NAME` for it: a layer runs only over a place of the unit's.

A base before 35 leaves aside an effect that has the flag, with its line for a table not in order, and uses the rest of the card. But a card with a layer names `op1f_layer_1`, so it never gets that far.

## An LFO layer

An ask of the kind `lfo`, on a card of level 8 or later:

```json
"asks": [ { "kind": "lfo", "name": "ufo", "drum": true, "shown": "lfo_ufo_shown" } ]
```

| In the recipe | In the table | What it is |
| --- | --- | --- |
| `name` | `head.name` | What the list of LFO kinds shows. 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_`. None of the unit's kinds (`tremolo`, `value`, `random`, `element`, `midi`, `velocity`), nor a name an earlier pack took for an LFO layer. The lists of effects and of LFOs are separate, but the pack tool refuses one name for an effect and an LFO layer of the same card |
| `drum` | `head.flags` | `OP1F_LFO_DRUM`: drum sounds may ride it too (below) |
| `shown` | `head.shown` | Told when your own page comes onto the screen and goes. May be null |

The table is `Op1fLfo`, the head alone: 28 bytes. An LFO layer has **no function on the audio thread**. A pack that moves the sound itself does it through level 6's stages, or as `lfo` does, through its own table of changes.

**The row never reaches the firmware.** No pick, no file and no list the unit writes ever holds its name, so a sound saved on it is a sound with the unit's own LFO, and the base's member says the rest.

**The places.** A synth sound's slot, 0 to 7. With `OP1F_LFO_DRUM`, also a drum sound's: `OP1F_PLACE_DRUM` + slot, 8 to 15, from base 37 on. Never the master: it has no LFO.

**Drums, and the flag.** The row shows in the drum's list of LFOs either way: the unit builds the drum's list from the same rows as the synth's. Without the flag, or under base 35 or 36, a pick of it on a drum sound starts no ride and hands the unit nothing, and the drum's LFO stays as the list's cursor left it. Bases 35 and 36 accept the flag and let only synth sounds ride.

**`shown(where, how)`** is told as an effect's is ([level 8](level-8.md), "The page on T3"): `where` is the place, 0 to 15, `how` `OP1F_SHOWN_UP`, with `OP1F_SHOWN_OFF` while the unit's LFO under the layer is switched off, and 0 when your page leaves the screen. It is not told while the unit's own LFO page shows.

`op1f_riding(card, ask)` answers bit n for place n, as for an effect. The value helpers answer `OP1F_BAD_NAME`.

**Up to eight LFO layers** of all packs together (`OP1F_LFOS_MOST`). An ask beyond that, or one not in order, is left aside with a line, and the rest of the card is used.

## The helpers

Each takes your card, the ask's number in your card's list (from 0: an `effect` ask with `"layer": true`, or an `lfo` ask) and a place. All three only from your frame function.

```cpp
int op1f_layer_start(const Op1fCard* card, unsigned ask, unsigned place);
int op1f_layer_end(const Op1fCard* card, unsigned ask, unsigned place);
int op1f_layer_page(const Op1fCard* card, unsigned ask, unsigned which);
```

**`op1f_layer_start`** starts the layer's ride at `place` (0 to 15, or `OP1F_PLACE_MASTER` for an effect), as a pick of its row would, but over the unit's effect or LFO as the place holds it now: nothing of the unit's is put back or changed. A ride of another pack's effect or layer there ends first, as a pick of another row would. The base's member is written, and `fresh` says it at the next block. It is for what your pack brings back of its own: a sound it kept before the layer was there.

| It answers | When |
| --- | --- |
| 0 | The place rides it, also when it rode it already |
| `OP1F_BAD_NAME` | No such layer of the card, or a place it cannot ride: the master for an LFO, a drum's place for an LFO without `OP1F_LFO_DRUM` or under base 35 or 36 |
| `OP1F_NOT_NOW` | Not from your frame function, or a sound loaded onto the place is not told yet: try again at the next frame |

**`op1f_layer_end`** ends the ride at `place`. The base's member is taken away, and the unit's own effect or LFO is left as it is, playing as it plays: its switch, its knobs, its type. It answers 0, `OP1F_NOT_HERE` (the place does not ride it), `OP1F_BAD_NAME` or `OP1F_NOT_NOW` (not from your frame function).

**`op1f_layer_page`** says which page T3 (an effect) or T4 (an LFO) shows for a place that rides the layer:

- `OP1F_LAYER_PAGE_UNIT`: the unit's own page of the effect or LFO under it, as if no layer were there. Every ride starts on it.
- `OP1F_LAYER_PAGE_OWN`: your own, the document `op1f_page_stand_in` gave for the ask ([level 5](level-5.md)), or the base's blank page with the layer's name until you give one.

It holds for every place that rides the layer, until you call it again, and you may switch as often as you like. While one of the two pages is on the screen, the other takes its place when the unit next asks which screen to show. While your own page is up, every key and knob is offered to your input function first, as for any page. While the unit's page is up, your input function is offered them as a card's without a page. It answers 1, or 0: no such layer of the card, `which` not one of the two, or a call from anywhere but your frame function.

`lfo` 12 uses both. Its page of four UFOs is its own. A tap of T4 there shows the stock page of the LFO under the searchlight (`OP1F_LAYER_PAGE_UNIT`), and T4 on that stock page, which its input function takes, brings the UFOs back (`OP1F_LAYER_PAGE_OWN`). The input function only notes the key; the frame function makes the call.

## Starting a ride after a load

**Start a ride a few frames after a sound was loaded onto the place, not in the frame of the load.** A ride started in that frame can be ended by the base one frame later, as "a sound without it was loaded" (the header says so at `op1f_layer_start`). Wait a few frames after the load, call `op1f_layer_start`, and call it again at the next frame while it answers `OP1F_NOT_NOW`. `lfo` 12 waits 4 frames, then tries at each frame for up to 120 frames.

## The mark

A card with an effect that has `"layer": true` or with an `lfo` ask names `op1f_layer_1` in a word behind its level's mark: `tools/pack_build.py card` writes it from the recipe. Under a base before 35 the loader then leaves the pack out with `nobody offers the name it needs, op1f_layer_1`. There is no mark for drums: a pack whose layer must ride drum sounds checks the answer of `op1f_layer_start` (`OP1F_BAD_NAME` under base 35 or 36), and the owner's pick there does nothing.

## The lines the base writes

Places are said as `synth 1` to `synth 8`, `drum 1` to `drum 8` and `the master`.

| Tag | Line begins | What it says |
| --- | --- | --- |
| `op1f_effects` | `effect: '<name>' of '<pack>' stands on the unit's own effect, a layer, entry <n> of the lists, <c> cycles a block declared` | Your effect layer is in the lists, once a start |
| `op1f_effects` | `effect: <place> rides '<name>'`, `effect: <place> rides nothing now: ...` | As for any effect ([level 8](level-8.md), "The lines the base writes") |
| `op1f_lfos` | `lfo: '<name>' of '<pack>' is a layer of the lists of LFOs, entry <n>, the drum's too` | Your LFO layer is in the lists, once a start. `the drum's too` when it has `OP1F_LFO_DRUM` |
| `op1f_lfos` | `lfo: <place> rides '<name>'` | The layer was picked there, started there, or found there at a start |
| `op1f_lfos` | `lfo: <place> rides nothing now: ...` | It is not there any more: another LFO was picked, a sound without it was loaded, the layer it names is not here, or the drum does not take the layer it names |
| `op1f_lfos` | `lfo: <place> rides '<name>', whose pack is not here: the sound plays its own LFO` | A warning: a sound saved on a layer whose pack is away |
| `op1f_lfos` | `lfo: the unit's list of LFOs could not be read ...` | A warning: no LFO layer is registered in this start |
| `op1f_cards` | `cards: '<pack>' asks for an LFO layer (its ask <n>) that is left aside: ...` | Why: its table, its name, a flag this base does not know, or more LFO layers than the base takes |
| `op1f_cards` | `page: '<pack>' stands in for the LFO's page of <place>, its document at <address>` | Your own page came up on T4 |

## Limits, in one place

| | |
| --- | --- |
| Effect layers | Effects of packs, layers or not: 16 in all, as level 8 |
| LFO layers | 8 of all packs together |
| A card | Level 8 or later for an `lfo` ask |
| Places for an effect layer | Every synth and drum sound, and the master |
| Places for an LFO layer | The synth's eight sounds; the drum's eight with `OP1F_LFO_DRUM`, from base 37 |
| Values on the knobs | None of the layer's: the knobs are the unit's |
| An LFO layer's sound work | None on the audio thread through the ask |
| The helpers | From your frame function only |
| A ride after a load | A few frames after it; again on `OP1F_NOT_NOW` |
| The mark | `op1f_layer_1`, base 35 or later |

Base 35 ran on the maintainer's unit with a test pack of his: an effect layer on the master and on a synth sound's effect, and an LFO layer on a synth sound's LFO, with the lists, the picks, the pages and a restart. Base 37 ran with the same test pack on a drum sound's lists of effects and LFOs, and with `lfo` 12, whose `ufo` rode a synth sound and a drum sound beside the other packs. No example in this repository uses a layer yet.

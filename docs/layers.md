# Layers: a row of your own over the unit's own effect or LFO (bases 35, 37 and 45)

An effect of [level 8](level-8.md) stands on one of the unit's effects and takes its place: where it plays, the unit's effect is switched off. A **layer** does not take a place. It is a row of your own in one of the unit's lists, and where a sound rides it, the unit's own effect or LFO stays under it and plays as always, whatever the owner, a pack or MIDI puts there. Your pack's work comes on top.

- **Base 35** adds layers: an effect may be a layer, in the unit's lists of effects, and a new kind of ask, `lfo`, is a layer of a sound's list of LFOs. Three helpers start and end a ride and choose the page.
- **Base 37** lets drum sounds ride an LFO layer too.
- **Base 45** lets an LFO layer carry a **wave**: a function of yours the base calls once a block, which moves an effect's knob values and a pack engine's values for that block ([below](#an-lfo-layers-wave-base-45)).

None of them is a level. The level stays 10, and a card with a layer names the mark `op1f_layer_1`, so the loader leaves it out under a base before 35.

Our `lfo` in [open-field-packs](https://github.com/LaP0573/open-field-packs), from its version 12, is an LFO layer: its row `ufo` in the LFO list of every synth and drum sound brings its four LFOs. Our `fx train` there, from its version 18, is an effect layer with no `process`: its row `train` in the lists of effects of every synth and drum sound brings its four wagons, which the pack plays itself, and T3 shows its own page for a sound that rides it. Read them for whole packs built this way.

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

The table is `Op1fLfo`, the head alone: 28 bytes. Such an LFO layer has **no function on the audio thread**, except its wave from base 45 (next section). A pack that moves the sound without a wave does it through level 6's stages, or as `lfo` does, through its own table of changes.

**The row never reaches the firmware.** No pick, no file and no list the unit writes ever holds its name, so a sound saved on it is a sound with the unit's own LFO, and the base's member says the rest.

**The places.** A synth sound's slot, 0 to 7. With `OP1F_LFO_DRUM`, also a drum sound's: `OP1F_PLACE_DRUM` + slot, 8 to 15, from base 37 on. Never the master: it has no LFO.

**Drums, and the flag.** The row shows in the drum's list of LFOs either way: the unit builds the drum's list from the same rows as the synth's. Without the flag, or under base 35 or 36, a pick of it on a drum sound starts no ride and hands the unit nothing, and the drum's LFO stays as the list's cursor left it. Bases 35 and 36 accept the flag and let only synth sounds ride.

**`shown(where, how)`** is told as an effect's is ([level 8](level-8.md), "The page on T3"): `where` is the place, 0 to 15, `how` `OP1F_SHOWN_UP`, with `OP1F_SHOWN_OFF` while the unit's LFO under the layer is switched off, and 0 when your page leaves the screen. It is not told while the unit's own LFO page shows.

`op1f_riding(card, ask)` answers bit n for place n, as for an effect. The value helpers answer `OP1F_BAD_NAME`.

**Up to eight LFO layers** of all packs together (`OP1F_LFOS_MOST`). An ask beyond that, or one not in order, is left aside with a line, and the rest of the card is used.

## An LFO layer's wave (base 45)

An LFO layer may carry a **wave**: a function of yours, which the base calls on the audio thread once a block. The block is 64 frames, about 1.45 ms. The wave may move the four knob values of the place's effect and the eight values of a pack's engine, for that block only. The knob stays where it is: the base writes no stored word, and the saved sound is as the owner left it.

```json
"asks": [ { "kind": "lfo", "name": "sway", "drum": true, "shown": null, "wave": "sway_wave", "cycles": 3000 } ]
```

`wave` and `cycles` come together or not at all. `wave` is a function of your pack, on an even address. `cycles` is the most one call takes, 1 to 8,000. The table is `Op1fLfoWave`, 36 bytes: the head, then `wave` at +28 and `cycles` at +32. Without them the table is 28 bytes, an LFO layer as before. The base reads the part it knows: a table of 36 bytes or more whose `wave` is not 0 carries a wave, and any other table of 28 or more is a plain layer, as on base 44.

```cpp
void sway_wave(Op1fWaveBlock* now);
```

**The block** is `Op1fWaveBlock`, 184 bytes at base 45 (read `bytes`: a later base may hand more). It lives for the call only: read it and write its values inside the call, and keep no pointer to it.

| Word | What it says |
| --- | --- |
| `place` | 0 to 7 a synth sound's slot, 8 to 15 a drum sound's |
| `frames` | The block's frames, 64 |
| `period` | A 96th of a beat, in samples times 2^18, as an effect's block has it |
| `playing` | 1 while the tape plays |
| `fresh` | 1 at the first call after a ride began (a pick, a load, `op1f_layer_start`, a start), and when the call before at this place was not in the instrument's block just before |
| `blocks` | The instrument's blocks since the start. The synth and the drum count their own |
| `on` | 1 when the place's effect is switched on |
| `effect_count`, `effect_values`, `effect_low`, `effect_high` | The effect's values, in and out, with their ranges. Up to 4 |
| `engine_count`, `engine_values`, `engine_low`, `engine_high` | A pack engine's values, in and out, with their ranges. Up to 8 |

A value beyond its count is not read. The base holds every written value to its range.

**The call.** Once a block, for every place that rides the layer and whose sound the instrument's block plays now: a synth sound's slot, a drum sound's with `OP1F_LFO_DRUM`, never the master. It comes before the place's effect plays that block, and it comes whether the effect is switched on or not (`on` says which). It does not come for a place that does not ride the layer, nor while another sound plays, nor while the wave is switched off for its cost.

**What it moves.**

- For a pack's effect that the place rides, `effect_values` are the pack's own numbers and `effect_low`, `effect_high` its ranges.
- For the unit's own effect (alone, or under an effect layer), each value is the upper half of the knob's word and the range is that knob's row. `effect_count` is 0 for a block where the base cannot reach the knobs: the effect played already, or the rows of its type are not in order. When the effect is on and its object was made before the base knew of the waves, for a second of the sound's blocks in a row, the log says so once (below).
- For a pack's engine on a synth sound's slot, the eight values in the engine's numbers.
- **Never the unit's own engines, nor the envelope, nor a drum sound's engine:** `engine_count` is 0 there. A pack's drum engine (base 43) is a drum sound's engine too.
- **Never the wagons a pack runs itself.** With `fx train`, only the wagon under the lamp is moved. The other wagons stay as they are: the wave never reaches them. They run on `fx train`'s own objects or through `op1f_rack_run`, and a wagon's page shows its value unmoved.

**A move lasts one block.** The next call is handed the value as set again. An effect's move plays in the same block. **An engine's move plays one block late**, about 1.45 ms: the voices of that block were rendered before the call. It is dropped at once when the wave was not called in the block after, so a pick of another LFO brings the value back at once.

**Only a moved value is rewritten.** A value you write as it was handed counts as no move: a knob left as handed keeps its word to the last bit. A value you change goes into the knob's word with its lower 16 bits cleared.

**`fresh`** tells a wave that its counters are stale: start the wave's phase again.

**The cost.** `cycles` is the most one call takes. A call that takes more than `OP1F_STAGE_CYCLES_OFF` switches the wave off at once. So do `OP1F_STAGE_OVER_CALLS` calls in one second over twice `cycles` and over `cycles` and `OP1F_STAGE_CYCLES_SPARE`. Off, it is never called and moves nothing, until its row is picked again in a list of LFOs, on any place, or the next start. A block is about 580,000 cycles and at most two waves run in a block (the synth's and the drum's), so a wave that computes a dozen numbers is cheap and 8,000 is room enough. The cost is not counted in level 6's sum, as an effect's is not.

**The mark.** A card with a wave names `op1f_wave_1` in a word behind `op1f_layer_1`: `tools/pack_build.py card` writes it from the recipe. A base from 35 to 44 would take the 36-byte table as a plain layer and never call the wave, so under such a base the loader leaves the pack out.

**Notes since the last call are not handed.** A wave that wants them asks for level 5's notes heard itself and reads its own counter.

The words are in the header, the section "Base 45: an LFO layer's wave". Where this page and the header disagree, the header is right.

An earlier build of this change, on base 42's sources, ran on one unit on 2026-11-27 with a test pack, `sway`. The wave was called once a block, about 689 times a second, and only for the sound that played and rode it: with the effect on and off, on a synth sound and on a drum sound. Its move was heard on one of the unit's own effects (`nitro`) and on a pack's engine (`sid`), and seen on a pack's effect's page (`crush`). The stored knob values stayed as they were, and one call far over its cost switched the wave off until its row was picked again. One warning was false: an effect "made before the waves", said once while a sound's effect was being changed. The base now says it only after a second of such blocks in a row.

The knob rows the base read there, knobs 1 to 4 (`vocoder` was not tried):

| Effect | 1 | 2 | 3 | 4 |
| --- | --- | --- | --- | --- |
| `cwo`, `mother`, `fazer`, `terminal` | 0..32767 | 0..32767 | 0..32767 | 0..32767 |
| `delay` | 1024..11264 | 3276..32767 | 0..16384 | 0..32767 |
| `grid` | 1344..16704 | 1344..16704 | 0..32767 | 0..32767 |
| `nitro` | 64..16448 | -32768..32767 | 0..20643 | 64..16448 |
| `phone` | 204..20480 | 3072..17408 | 1536..16896 | 0..32767 |
| `punch` | 1344..12480 | 0..32767 | 1536..25088 | 0..32767 |
| `spring` | 1344..16448 | 7744..16448 | 0..16384 | 0..32767 |

This change on base 44's sources ran on the same unit the same night, beside `lfo` 12, `fx train` 18 and thirty more packs, with the same results and the same knob rows, and the false warning did not come again. Not tried on a unit: `vocoder`, a train with more than one wagon, a restart with a sound that rides a wave, and a unit with no wave pack on this base.

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

**From base 45, `op1f_wave_1` as well, for a wave (above).** A card with an effect that has `"layer": true` or with an `lfo` ask names `op1f_layer_1` in a word behind its level's mark: `tools/pack_build.py card` writes it from the recipe. Under a base before 35 the loader then leaves the pack out with `nobody offers the name it needs, op1f_layer_1`. There is no mark for drums: a pack whose layer must ride drum sounds checks the answer of `op1f_layer_start` (`OP1F_BAD_NAME` under base 35 or 36), and the owner's pick there does nothing.

## The lines the base writes

Places are said as `synth 1` to `synth 8`, `drum 1` to `drum 8` and `the master`.

| Tag | Line begins | What it says |
| --- | --- | --- |
| `op1f_effects` | `effect: '<name>' of '<pack>' stands on the unit's own effect, a layer, entry <n> of the lists, <c> cycles a block declared` | Your effect layer is in the lists, once a start |
| `op1f_effects` | `effect: <place> rides '<name>'`, `effect: <place> rides nothing now: ...` | As for any effect ([level 8](level-8.md), "The lines the base writes") |
| `op1f_lfos` | `lfo: '<name>' of '<pack>' is a layer of the lists of LFOs, entry <n>, the drum's too` | Your LFO layer is in the lists, once a start. `the drum's too` when it has `OP1F_LFO_DRUM` |
| `op1f_lfos` | `..., a wave of <c> cycles a block declared` | Behind the line above, for a layer with a wave (base 45) |
| `op1f_lfos` | `lfo: the wave of '<name>' is switched off until its row is picked again: <n> cycles in one call, <c> declared` | The wave took too long once (base 45). The other form says `<n> calls in one second over twice its cost and its allowance, the most <m> cycles, <c> declared` |
| `op1f_lfos` | `lfo: the wave of '<name>' this period: <n> call(s), <m> cycles at most, <a> on average, <o> over` | What a wave cost in the last period (base 45) |
| `op1f_effects` | `effect: a wave rides <place>, whose effect was made before the waves: its knobs are out of reach until it is made anew` | A warning (base 45): that effect object was made before the base knew of the waves, so the wave is handed no knob of it until the unit makes it anew. Said once, and only after a second of the sound's blocks in a row with the effect on and that object in place: while the unit changes a sound's effect, such a block or two comes and goes, and says nothing |
| `op1f_effects` | `effect: a wave is handed no knob of <type>: its knob rows are not in order (<lo>..<hi>, <lo>..<hi>, <lo>..<hi>, <lo>..<hi>)` | A warning (base 45): the base reads the first four knobs of each type from the unit's rows and found them out of order, so that type's wave gets `effect_count` 0 |
| `op1f_lfos` | `lfo: <place> rides '<name>'` | The layer was picked there, started there, or found there at a start |
| `op1f_lfos` | `lfo: <place> rides nothing now: ...` | It is not there any more: another LFO was picked, a sound without it was loaded, the layer it names is not here, or the drum does not take the layer it names |
| `op1f_lfos` | `lfo: <place> rides '<name>', whose pack is not here: the sound plays its own LFO` | A warning: a sound saved on a layer whose pack is away |
| `op1f_lfos` | `lfo: the unit's list of LFOs could not be read ...` | A warning: no LFO layer is registered in this start |
| `op1f_cards` | `cards: '<pack>' asks for an LFO layer (its ask <n>) that is left aside: ...` | Why: its table (28 bytes or more; with a wave, `its wave does not lie in its pack on an even address` or `its wave's cost is not between 1 and 8000 cycles a call`), its name, a flag this base does not know, or more LFO layers than the base takes |
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
| An LFO layer's sound work | None on the audio thread through the ask, but its wave from base 45 |
| An LFO table | 28 bytes or more; a wave at 36 or more when its `wave` is not 0, else a plain layer |
| A wave's cost | 1 to 8,000 cycles a call, declared; switched off over it, until its row is picked again |
| What a wave moves | An effect's four knob values, a pack engine's eight, for one block. Never the unit's own engines, the envelope or a stored word |
| An engine's move | Plays one block late |
| With `fx train` | The wagon under the lamp moves, the other wagons never |
| The helpers | From your frame function only |
| A ride after a load | A few frames after it; again on `OP1F_NOT_NOW` |
| The marks | `op1f_layer_1`, base 35 or later; `op1f_wave_1` for a wave, base 45 or later |

Base 35 ran on the maintainer's unit with a test pack of his: an effect layer on the master and on a synth sound's effect, and an LFO layer on a synth sound's LFO, with the lists, the picks, the pages and a restart. Base 37 ran with the same test pack on a drum sound's lists of effects and LFOs, and with `lfo` 12, whose `ufo` rode a synth sound and a drum sound beside the other packs. No example in this repository uses a layer yet. For a wave, see [its section](#an-lfo-layers-wave-base-45).

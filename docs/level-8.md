# Level 8: an effect of your own, in the unit's own lists

Level 8 lets your pack add an effect to the unit's own lists of effects, for every synth and drum sound and for the master. You pick it there the way you pick `delay` or `punch`. The unit's own key switches it on and off, T3 shows its page, the four knobs turn its values, the LFOs move them, and saving the sound keeps it. Your pack changes no place of the firmware for any of that.

Level 8 also keeps one list of every effect the packs on the unit have added, and lets any pack run one of them inside its own sound work. A chain of effects can offer yours on each of its links, a mixer lane beside its other choices. Our own `fx train` does exactly that with its wagons.

The words of the interface are in the header `module/build_r6e_card.h`. Where this page and the header disagree, the header is right.

## The two examples: `ping` and `crush`

[`ping`](../examples/ping/README.md) is an echo in time with the tempo that bounces left and right. It stands on `delay`, takes a megabyte of working memory for each place it plays at, and lets its echoes ring out after you switch it off.

[`crush`](../examples/crush/README.md) makes a sound rougher: fewer bits and a lower sample rate. It stands on `punch`, needs no memory and keeps nothing from one block to the next.

To hear them, with `base.pack`, `crush.pack` and `ping.pack` in `packs`:

- **On a synth sound**: hold **shift** and press **T3**, and pick `crush`. The list is sorted by name, so it's near the top, just before `cwo`. Play: it's rougher. Press **T3** to see its page.
- **On the master**: on the mixer, open the master effect's list as always and pick `ping`, between `phone` and `punch`. Everything you play now echoes on the beat. **T3** on the mixer shows its page.
- Switch `ping` off with the unit's own key on its page and the last echoes ring out.

Neither example takes a key or a knob. The unit turns their values, and their pages show where the values are as the sound plays them. That last part is [level 10](level-10.md), which is why both cards say level 10. The code quoted below is theirs.

## The ask

A card that adds an effect says level 8 or later and has one ask of the kind `effect` for each effect. Its source makes the level's mark a name your pack needs, `op1f_level_8` for a card of level 8, so on an older base the loader leaves your pack out and says which mark was missing. As with every kind, the pack tool writes the table from the recipe and refuses what the base would leave aside. This is `ping`'s:

```json
"asks": [
  {
    "kind": "effect", "name": "ping", "rides": "delay", "mono": false, "tails": true,
    "cycles": 42000, "state_bytes": 1048576, "state_align": 32,
    "process": "ping_process", "reset": "ping_reset", "shown": "ping_shown",
    "params": [
      {"name": "time", "low": 1, "high": 4, "start": 3},
      {"name": "feedback", "low": 0, "high": 90, "start": 40},
      {"name": "spread", "low": 0, "high": 100, "start": 100},
      {"name": "mix", "low": 0, "high": 100, "start": 30}
    ]
  }
]
```

The table the tool writes is an `Op1fEffect`:

| In the recipe | In the table | What it is |
| --- | --- | --- |
| `name` | `head.name` | What the lists show. 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_` |
| `mono`, `tails` | `head.flags` | `OP1F_EFFECT_MONO`, `OP1F_EFFECT_TAILS` (below) |
| `shown` | `head.shown` | Told when your page comes onto the screen and goes (below). May be null |
| `rides` | `rides` | The unit's own effect yours stands on (below) |
| `cycles` | `cycles` | What one call of `process` takes at most, 1 to 84,000 |
| `state_bytes`, `state_align` | the same | The working memory one place needs, 0 to 8 MiB, and its alignment, a power of two from 4 to 4,096 |
| `process` | `process` | The effect itself, once a block, on the audio thread |
| `reset` | `reset` | Clears your memory for a place, on the display thread. May be null |
| `params` | `count`, `params` | One to four values, one on each knob |

**The name.** Not one of the unit's own eleven: `delay`, `grid`, `punch`, `phone`, `spring`, `mother`, `nitro`, `cwo`, `vocoder`, `terminal`, `fazer`. **Where two packs ask for the same name, the earlier file keeps it**, in the order of the files' names, and the later ask is left aside with a line. Your pack still runs without it. Sixteen effects of packs in all. The seventeenth is left aside.

**Where it shows.** The unit sorts its lists of effects by name, so yours stands among the unit's own where its name falls, not at the end: `crush` before `cwo`, `ping` between `phone` and `punch`. The base gives each effect a number from 15 up (the unit's own are below), in the order of the files' names and then of each card's asks, at every start. So a pack that keeps a choice of effect keeps its name, never its number.

## What your effect stands on

Every effect of a pack stands on one of eight of the unit's own: `delay`, `grid`, `punch`, `phone`, `spring`, `nitro`, `cwo` or `terminal`. That one is what the unit keeps in the sound's file: its name, its four knobs, each inside that effect's own range, and **switched off**. The base keeps which pack's effect plays there and whether it's on, beside the sound in its preset and for each slot in a small file of its own.

That choice does three things you'll notice:

- **Your values move the way that effect's knobs move.** The base maps each knob's whole travel onto your value's range, both ways, but the travel, the steps a turn makes and what an LFO does to the knob are that effect's.
- **On a unit without your pack, a sound saved with your effect plays dry.** So does the master. The effect list opens on the effect yours stands on, T3 shows that effect's page with the knobs where yours left them and the effect switched off, and T3 again switches it on. That's the same on the original firmware.
- **With open-field and without your pack** it's the same, and the log says so once a start: `effect: synth 3 rides 'crush', whose pack is not here: the sound plays dry`. Nothing is forgotten. When your pack comes back, the sound plays your effect again.

Pick the effect nearest yours in character. `ping` stands on `delay`, `crush` on `punch`.

## The values

One to four values. Value 1 is turned by the **blue** knob, 2 by the **second**, 3 by the **third**, 4 by the **red** one, as with the unit's own effects.

| Field | What |
| --- | --- |
| `name` | What the LFO pages and your page show for it: 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_` |
| `low`, `high` | Its range in your own numbers: `low` below `high`, at most 65,535 apart. Negative numbers are fine |
| `start` | Its value when the owner picks your effect, inside the range |

Your `process` gets the values in your own numbers, as the knobs, the LFOs and the random key left them for that block. You never see the knob itself.

**Anything you keep beyond four values** (a mode, a pattern, a switch) is a value of [level 7](level-7.md): a member of your own with the sound. For the master, a card of level 8 or later may keep a level 7 value there too, with `"master"` in the key's `files` (`OP1F_KEEP_MASTER`). It's kept in the base's own store, since the unit keeps no file of the master's that a pack may write in.

## What `process` is handed

`process` is called once a block of 64 frames, where the unit runs its own effect: on the sound that plays, or on the unit's sum for the master. It's handed an `Op1fEffectBlock`:

| Field | What it is |
| --- | --- |
| `bytes` | 44 at level 8. A later base may hand more, so read what you know |
| `place` | Where it plays now: 0 to 7 a synth sound's slot, `OP1F_PLACE_DRUM` (8) plus the slot for a drum sound, `OP1F_PLACE_MASTER` (16) for the master, `OP1F_PLACE_HOSTED` (32) plus a place of another pack's when that pack runs your effect (below) |
| `frames` | 64 |
| `left`, `right` | The two buffers, signed 32-bit words, Q1.31 as at [level 6](level-6.md). Change them in place and hold every sum at full scale |
| `values` | Your values in your own numbers, `count` of them |
| `on` | 1. It's 0 only for an effect with `OP1F_EFFECT_TAILS` while the place's effect is switched off |
| `fresh` | 1 at the first block after the place's effect was made anew: picked, a sound loaded, another slot played, the unit started. Or, inside another pack, when that pack says so |
| `period` | A 96th of a beat in frames, times 2^18, as at level 6: a sixteenth note is `((period >> 6) × 24) >> 12` frames |
| `playing` | Not 0 while the tape plays |
| `state` | Your working memory for this place, or 0 when you asked none |

The block lives for the call only. Never keep it, or the buffers' addresses.

**The rules on the audio thread are level 6's, whole.** Nothing allocated, no file, no waiting, no line in the log, nothing of the firmware's. No division by a number that isn't a power of two, no 64-bit multiply, no floating point. **Hold every sum at full scale.** At most 1 KiB of stack (`OP1F_STAGE_STACK_MOST`), so a delay line lives in your working memory, never in a local. Of the base's helpers, `process` may call `op1f_memory`, `op1f_stage`, `op1f_mask`, `op1f_unmask` and `op1f_rack_run`. Count what you want to say and say it from your frame function.

**One effect, many places.** Your effect may be on five sounds and the master at once. Only one sound plays at a time, so at most two places run together: the sound that plays and the master. When the owner plays another sound that has your effect, the effect is made anew there and `fresh` says so, as with the unit's own. A tail doesn't cross from one sound to the next.

## Two flags

**`"mono": true`** (`OP1F_EFFECT_MONO`): your effect works on one buffer. The base hands you the mean of the two sides in `left` (`right` is the same buffer), and copies `left` to both sides after your call. A stereo sound then comes out in the middle, **even at a mix of 0**. `crush` doesn't use it. It works on each side apart, the work is small, and at a mix of 0 the sound has to leave untouched.

**`"tails": true`** (`OP1F_EFFECT_TAILS`): while the place's effect is switched off, your `process` is still called every block with `on` 0, after the unit's own sound there, so an echo or a hall can ring out. Without the flag a switched-off effect isn't called and the sound passes dry. With it:

- **Let nothing new in while `on` is 0.** Add your tail to the sound and leave the sound itself as it came.
- **Stop working once the tail has died.** You're called every block for as long as the effect stays off, which can be hours. `ping` keeps a count of the frames since anything but silence went into its lines, and returns at once when it covers the whole of them:

```cpp
if (!on && lines->quiet >= (u32)LINE) return;         // switched off, and the lines hold nothing: the sound goes by
```

## The cost you declare

`"cycles"` is what one call takes at most, in the processor's cycles, from 1 to 84,000 (`OP1F_EFFECT_CYCLES_MOST`), about what the unit's costliest own effect takes. It isn't counted in level 6's sum for stages: your effect takes the place of one of the unit's, which costs about as much.

The base times every call and holds you to it **at each place**, as it holds a stage. It's **switched off at that place until an effect is picked there again** when eight calls in one second take more than twice what you declared and more than that plus 20,000, or when one call takes more than 290,000. That place then passes the sound dry, its page says `switched off`, and the log names your effect, the place and the call. Any pick in the unit's list at that place gives it another go, even of the row it already rides. Once a period the log gives your effect's calls, the most cycles, the average and the calls over. Read that line on your unit and set `cycles` from it.

Before you have a unit's reading, count. Both examples worked theirs out from the compiled code: the longest way through one turn of the loop, in instructions, times 64 frames, plus everything outside the loop. Then that count turned into cycles at about two cycles an instruction, which is what `dub`'s stage showed on a unit for a loop that reads and writes lines in the unit's main memory. Then half again on top. That gave `ping` 42,000 and `crush` 31,000. On a unit the period lines then showed `ping` at most 25,299 cycles a call and `crush` 17,427, with no call over.

**Write the loop with no branches where you can.** A loop that costs the same whatever the sound is easy to count and never surprises you in a loud passage. Both examples hold their sums this way:

```cpp
static inline int add_held(int a, int b) {
    u32 s = (u32)a + (u32)b;
    u32 over = 0u - ((~((u32)a ^ (u32)b) & ((u32)a ^ s)) >> 31);   // all ones when the sum went past full scale
    u32 most = ((u32)a >> 31) + 0x7fffffffu;                        // full scale on a's side: 2^31 - 1, or -2^31
    return (int)((s & ~over) | (most & over));
}
```

## Working memory

`state_bytes` is the memory one place needs, up to 8 MiB (`OP1F_EFFECT_STATE_MOST`). You don't ask it with a separate ask. The base takes it for you, from level 6's working memory and inside its limits (16 MiB for every pack's asks together, 8 MiB kept free for the unit):

- **One block for the sound that plays** (every synth and drum sound shares it, since only one plays at a time), and **one for the master**, each the first time your effect plays there.
- **One for each place another pack runs your effect at** (below), the first time it asks. Once no place runs it any more, the base may hand it on to the next place that asks for your effect, cleared to 0 again first.

Each is cleared to 0 over the frames after it's taken, a quarter of a megabyte a frame, and the place passes the sound dry until it's handed over. It's never given back. **When it's refused**, the log says why, that place passes the sound dry, and your page says `no room`. `state` is 0 when you asked none.

## Starting anew: `fresh` and `reset`

`fresh` is 1 at the first block after the place's effect was made anew. Clear whatever you keep from block to block.

`reset(place, state)` is optional. The base calls it on the display thread before that first block, when your effect rides a sound or the master, and when another pack readies your effect at a place of its own. The place passes the sound dry until it returns. **That's where a large memory gets cleared.** Clearing a megabyte would take most of a block on the audio thread. `ping`'s clears the whole megabyte:

```cpp
for (u32 i = 0; i < (u32)(STATE_BYTES >> 2); ++i) word[i] = 0;
((Lines*)state)->quiet = LINE;                        // nothing in the lines
```

**But `fresh` can come without a `reset`.** A pack that runs your effect may start it anew itself (a chain rebuilt, a sound changed) and say `fresh`, and you get no `reset` first. So `process` has to be able to start again on its own, cheaply. `ping` doesn't clear its lines then, it forgets them: it starts writing at the top and reads nothing older than what it has written since.

```cpp
if (now->fresh) {                                     // started anew: the lines forgotten, not cleared
    lines->at = 0;
    lines->filled = 0;
    lines->quiet = LINE;
}
```

An effect with nothing to clear needs neither. `crush` keeps nothing between blocks, so it has no `reset` and `fresh` has nothing to do.

## The page on T3

While a sound that has your effect shows its effect page (T3), or the mixer shows the master effect's page, **your page stands in for it**, as a sequencer's does at [level 5](level-5.md). You don't open or close it. Until you call `op1f_page_stand_in` for the ask, it's the base's blank page with your effect's name at the top, and your frame function paints on the frame's `canvas` below the name (level 4's painting helpers). Or hand it a document of your own picture with `op1f_page_stand_in(card, ask, document)`. The unit's own popups come up over it as over any pack's page ([level 4](level-4.md)).

**`shown(where, how)`** is called on the display thread at each frame where what it says changes, before the frame functions. `where` is the place whose page is up. `how` is `OP1F_SHOWN_UP`, with `OP1F_SHOWN_OFF` beside it while that place's effect is switched off, and 0 when the page leaves the screen. From the frame where it says `OP1F_SHOWN_UP` the frame's `page` is your card. Like any `shown`, it notes what it's told in single words and does nothing else.

**Keys and knobs** go to your input function first, as for any page. What you don't take goes to the unit: T3 switches the effect on and off, and the four knobs move your values. If you take a knob yourself, note the turn in your input function and set the value from your frame function with `op1f_param_set`.

**Show `off` yourself.** Nobody has seen yet whether the unit marks an effect's page while it's switched off, so both examples paint the word when `shown` says `OP1F_SHOWN_OFF`. If the base had to pass the place dry (its memory refused, or switched off for its cost), the base paints `no room` or `switched off` under your name itself.

Both examples paint their page the same way: for each value, the number in its knob's colour over its name. This is `crush`'s:

```cpp
if (op1f_param_heard(&op1f_card_crush, ASK, where, i, &value) == 0) {
...
paint_centred(canvas, middle, VALUE_Y, text, KNOB[i], VALUE_SCALE);
paint_centred(canvas, middle, NAME_Y, effect->params[i].name, LIGHT_GREY, NAME_SCALE);
```

`op1f_param_heard` is level 10's: the value as the sound plays it, so an LFO's move shows on the page. A pack that stops at level 8 calls `op1f_param_get` with the same arguments and shows the value as set. The names come from the pack's own table, the one the recipe made, so the page and the LFO pages always agree.

## The helpers

| Helper | What it does | From |
| --- | --- | --- |
| `op1f_param_get(card, ask, place, index, &out)` | The value `index` of your effect at `place`, as set, in your own numbers | The frame and input functions |
| `op1f_param_set(card, ask, place, index, value)` | The value set: held to your range and written as the knob would write it, so the sound plays it at the next block and the unit's own save keeps it. A sound keeps it in the sound itself, the master in the base's store | The frame function |
| `op1f_riding(card, ask)` | The places that have your effect now: bit n for place n, bit 16 the master | The frame and input functions |
| `op1f_rack_at(card, place)` | The effect a sound's slot (0 to 15) or the master has now, by its number, or 0 for one of the unit's own or none. Any pack may ask | The frame and input functions |
| `op1f_rack_list(card, &count)` | The list of every effect the packs registered (below) | The frame and input functions |
| `op1f_rack_ready(card, which, id)` | An effect made ready at a place of your own | The frame function |
| `op1f_rack_run(card, which, &run)` | That effect run once on your buffers | Your own sound work, on the audio thread |

`op1f_param_get` and `op1f_param_set` answer 0, or `OP1F_NOT_HERE` (that place doesn't have your effect now), `OP1F_BAD_NAME` (no such ask, or no such value) or `OP1F_NOT_NOW` (called from somewhere they may not be). A value set is held to its range first. One you get is always inside it.

## Running another pack's effect inside yours

The list `op1f_rack_list` answers is the base's, the same from the first frame to the end, one entry for each effect in the order of their numbers. Each entry is an `Op1fRackEntry`:

| Field | What |
| --- | --- |
| `bytes` | 20 at level 8 |
| `id` | The effect's number |
| `pack` | The name of the pack that added it |
| `effect` | Its table as its pack wrote it: name, flags, values, cost |
| `rides` | The number of the unit's own effect it stands on, in the unit's own order of its effects |

To run one inside your own sound work:

1. **Make it ready at a place of your own**, from your frame function: `op1f_rack_ready(card, which, id)`, with `which` 0 to 7, your own numbering (a link of a chain, a lane). The first time, the base takes the effect's working memory for that place, or hands it one no place uses any more, and calls its `reset`. Meanwhile it answers `OP1F_NOT_NOW`, so ask again at the next frame. It answers 0 when the effect is ready. Asking again every frame with the same effect costs a few comparisons. It answers `OP1F_TOO_LARGE` or `OP1F_FAILED` when the memory was refused (with a line), and `OP1F_NOT_HERE` when the effect was switched off at your place for its cost. Ask for another effect there and then for this one again, or stop asking for it and running it for a few frames (your place held something else) and ask again, and it gets another go.
2. **Run it** from your own sound work, a stage's `process` for instance: `op1f_rack_run(card, which, &run)`. The effect's `process` is called on your buffers with `place` `OP1F_PLACE_HOSTED + which` and your place's memory. It answers 0 when it ran, else a code and your buffers untouched. It writes no line and waits for nothing.

An `Op1fRackRun` holds `bytes` (36), `id`, `form`, `frames` (64), `left` and `right`, `values`, `on` and `fresh`:

- **`form`** `OP1F_RACK_VALUES`: `values` are the effect's own numbers, `count` of them, each held to its range. `OP1F_RACK_WORDS`: four words as the unit keeps a knob of the effect it stands on (a value shifted left by sixteen, inside that knob's range), for a pack that keeps the unit's own knob words, with the unit's LFOs on them.
- **`on`** 1, or 0 to let an effect with tails ring out after you switched it off.
- **`fresh`** 1 when you start the effect anew at that place. The base hands 1 itself at the first run after `op1f_rack_ready` made the instance.

A pack with a lane on the master might do this. The frame function makes the owner's choice ready, and the stage runs it:

```cpp
static const Op1fRackEntry* rack;                 // the base's list, once
static unsigned rack_count, chosen;               // the owner's choice, an index into the list
static volatile unsigned lane;                    // the effect the stage runs, 0 for none: written by the frame
static int lane_values[4];                        // its values in its own numbers: single words the frame writes

extern "C" void lane_frame(const Op1fFrame* now) {
    if (rack == 0) rack = op1f_rack_list(&op1f_card_lane, &rack_count);
    unsigned id = chosen < rack_count ? rack[chosen].id : 0;
    lane = id != 0 && op1f_rack_ready(&op1f_card_lane, 0, id) == 0 ? id : 0;
}

extern "C" void lane_process(const Op1fBlock* now) {
    unsigned id = lane;
    if (id == 0) return;
    Op1fRackRun run = { 36, id, OP1F_RACK_VALUES, now->frames, now->left, now->right, lane_values, 1, 0 };
    op1f_rack_run(&op1f_card_lane, 0, &run);      // 0 when it ran; otherwise the sound went by untouched
}
```

Three things to keep in mind:

- **Your own stage is timed with the effect inside it.** Declare your stage's cost with the costliest effect it may run counted in. The effect is also held to its own declared cost at your place. When it goes over, it's switched off there, and `op1f_rack_run` answers `OP1F_NOT_HERE`, until you pick it at that place again after another effect, or after a few frames of not asking for it.
- **The effect can't tell your place from a sound's** but by `place`. Hand it what the unit would: 64 frames, Q1.31, its values inside their ranges.
- **Each of your places keeps its own memory for the effect it last ran there.** Put another effect at that place and the base readies it afresh. Once no place of any pack has run an effect there for two frames, the base hands that run to the next place that asks: one of the same effect first, its memory cleared to 0 again over the frames, a quarter of a megabyte a frame, with `OP1F_NOT_NOW` meanwhile and `reset` after, as for a new one. So walking a place through the list uses nothing up. The base keeps 64 runs at most for all packs together. Once all 64 are made, a new one takes a spare run without memory, or one whose memory is big enough for it; when there is none, `op1f_rack_ready` answers `OP1F_FAILED` with a line.

**Our `fx train` is such a pack.** With it on the unit, your effect is offered on the wagons of the train too, in the same order as the unit's list. From version 18 it is not offered on the wagon under the lamp, which holds the sound's own effect, and a tap on a wagon that holds yours opens nothing. A wagon that holds it is drawn as the effect yours stands on, with your effect's name in the unit's own letters. If your pack is taken off, the wagon stays on the train, silent, keeps the name, and plays again when the pack is back.

## Two packs, and your pack's own places

- **Two effects of two packs** play together at two places: one on a sound and one on the master, or each on different sounds. At one place there's one effect, as on the unit.
- **A pack with an effect needs no place of the firmware.** If your effect pack changes the firmware's sound path today, move it to an ask. The warning of [level 6](level-6.md) about two packs at one place then no longer applies to it.

## The lines the base writes

Places are said as `synth 1` to `synth 8`, `drum 1` to `drum 8` and `the master`. The tag is `op1f_effects`.

| Line begins | What it says |
| --- | --- |
| `effect: '<name>' of '<pack>' stands on <effect>, entry <n> of the lists, <c> cycles a block declared` | Your effect is in the lists, once a start |
| `effect: '<name>' of '<pack>' is left aside: ...` | Why not: its name, what it stands on, its cost, its table, or that it would be the seventeenth |
| `effect: the knob rows of <effect>: ...` | The four knobs' ranges of the effect yours stands on, as the unit holds them, once a start |
| `effect: <place> rides '<name>'` | Your effect was picked there, or found there at a start |
| `effect: <place> rides nothing now: ...` | It isn't there any more: another effect was picked, a sound without it was loaded, or the unit put another effect there |
| `effect: <place> rides '<name>', whose pack is not here: ...` | A sound or the master saved with an effect whose pack is away: it plays dry |
| `effect: '<name>' has <bytes> bytes for <place> at ...` | Your working memory for that place |
| `effect: '<name>' has no room for <place>: ...` | Why not |
| `effect: '<name>' this period: ...` | Through the base's quiet gate: calls, the most cycles, the average, and calls over what you declared |
| `effect: '<name>' at <place> is switched off until ...: ...` | The call that went too far, or how many went over in one second, against what you declared. Until `an effect is picked there again` at a sound or the master, `it is picked there again` at another pack's place |
| `effect: '<name>' value <n> '<value>' stands at <v>: the knob row of <effect> has no span` | A knob of the effect yours stands on doesn't move, so that value stays at its `start` |
| `effect: <place> waited <n> frames for the loaded sound's <effect>, which did not come: ...` | A warning: a sound was loaded onto a slot and the unit never applied it there (a sound sent over MIDI to a slot that isn't shown, say). The base reads the slot as it stands |

## Limits, in one place

| | |
| --- | --- |
| Effects of packs | 16 in all, numbered 15 to 30 |
| Where they show | In the unit's lists of effects, for every sound and the master, among the unit's own by name |
| A name | 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-`, `_`. None of the unit's eleven |
| What it stands on | `delay`, `grid`, `punch`, `phone`, `spring`, `nitro`, `cwo`, `terminal` |
| Values | 1 to 4, a range of at most 65,535 |
| Declared cost | 1 to 84,000 cycles a call |
| Switched off | Eight calls in one second over twice the declared cost and over the declared cost plus 20,000, or one call over 290,000 |
| Stack | 1 KiB a call of `process`, with whatever it calls |
| Working memory | Up to 8 MiB a place, from level 6's 16 MiB, 8 MiB kept free |
| Places that play at once | Two: the sound that plays and the master, besides those other packs run it at |
| Your own places for other packs' effects | 8 (`which` 0 to 7) |

## How `ping` works

`ping` is one ask of kind `effect`, a frame function that paints its page, and nothing else: no input function, no folder, no picture of its own.

**The echo**, for each frame. `a` and `b` are what the left and the right line hold `delay` frames back:

```
in              the mean of the two sides; nothing while switched off
left line       gets in + feedback × b      held at full scale: the right's echo comes back on the left
right line      gets      feedback × a      the left's echo goes over to the right
wet left        a - far × (a - b), wet right b + far × (a - b), far = (1 - spread) / 2
out             each side × (1 - mix) + its wet × mix; switched off, each side as it came + its wet × mix
```

So a sound comes back left, then right, then left, each echo softer than the one before by the feedback. At full spread each side hears its own line. At none, both hear the mean of the two.

**The time** comes from the block's `period`, with no division, held to the lines:

```cpp
static inline u32 delay_of(u32 period, u32 sixteenths) {
    u32 p = period >> 6;                                  // a 96th of a beat, in frames times 2^12
    if (p > PERIOD_MOST) return LINE - 1;                // slower than the lines can hold anyway
    u32 d = (p * (sixteenths * (u32)SIXTEENTH)) >> 12;
    if (d < 1u) return 1u;
    return d > (u32)(LINE - 1) ? (u32)(LINE - 1) : d;
}
```

At 120 beats a minute a sixteenth is 5,512.5 frames, and `ping`'s four times land on 5,512, 11,025, 16,537 and 22,050: the true sample counts, rounded down. At any tempo they're within a frame of it.

**Its memory** is the megabyte it asks: a head of four words, then the two lines side by side, 131,070 frames each, about three seconds. A quarter note at a very slow tempo is held to that.

**What keeps the owner's ears safe**, each held by a test on a computer:

- Every sum is held at full scale. Full-scale sound in, at every value's ends: nothing wraps.
- The feedback is at most 0.9, so each echo is softer than the one before.
- Every product is rounded toward 0, so a tail dies away to exact silence, and then `ping` costs nothing while switched off.
- At mix 0 the buffers aren't written. The sound leaves bit for bit as it came.

## How `crush` works

For each side, left and right alike:

- **The rate.** The side is cut into runs of 2^rate frames, and every frame of a run takes the run's first frame. A block's 64 frames hold a whole number of runs at every rate, so nothing is kept between blocks.
- **The drive.** That frame times 2^drive, held at full scale.
- **The bits.** Its size cut down to a step of 2^(32 − bits), toward 0, so silence stays silence.
- **Out.** Each frame × (1 − mix) plus the run's crushed frame × mix. At mix 0 the block isn't touched.

```cpp
for (u32 i = 0; i < frames; i += run) {
    int crushed = scaled(rough(side[i], drive, steps), mix);
    u32 end = i + run > frames ? frames : i + run;
    for (u32 k = i; k < end; ++k) side[k] = add_held(scaled(side[k], keep), crushed);
}
```

At drive 0 nothing comes out louder than the loudest of what went in. At the unit's own rate, nothing comes out louder than the same frame went in.

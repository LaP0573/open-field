# Level 9: a synth engine of your own

Level 9 lets your pack add a synth engine to the unit's own list of engines. You pick it with **shift** and **T1** on a synth sound, as you'd pick `fm` or `string`. Every key then plays your engine, T1 shows its page, the four knobs turn its values (and with **shift** four more), the LFOs move them, and saving the sound keeps it. Your engine can bring sounds of its own too, and they show up in the unit's sound list in a folder named after it. Your pack changes no place of the firmware for any of that.

The words of the interface are in the header `module/build_r6e_card.h`. Where this page and the header disagree, the header is right.

## The two examples: `pluck` and `fold`

[`pluck`](../examples/pluck/README.md) is a plucked string. Each voice is a short delay line plucked by a burst of noise, louder and brighter the harder you play. It has eight values, so it uses the knobs with and without shift, and 16 KiB of working memory for each voice.

[`fold`](../examples/fold/README.md) folds a sine wave over itself for a hollow, buzzy tone, a little different on the left and the right. It renders two channels, has four values and needs no memory.

To hear them, with `base.pack`, `pluck.pack` and `fold.pack` in `packs`:

- On a synth sound, hold **shift** and press **T1**. The list is sorted by name, so `fold` is right after `fm` and `pluck` right after `phase`. Pick one and play.
- Press **T1** to see its page: the values as the sound plays them, in the knobs' colours.
- Switch off and on once, then open the sound list as always. There's a folder `pluck` with `nylon` and `harp`, and a folder `fold` with `hollow` and `buzz`, as long as your unit had room for them (below).

## The ask

A card that adds an engine says level 9 or later and has one ask of the kind `engine` for each engine. Its source makes the level's mark a name your pack needs, `op1f_level_9` for a card of level 9, so on an older base the loader leaves your pack out and says which mark was missing. The pack tool writes the table from the recipe and refuses what the base would leave aside. This is `pluck`'s, with six of its eight values left out here:

```json
"asks": [
  {
    "kind": "engine", "name": "pluck", "stereo": false,
    "cycles": 33000, "voice_bytes": 16384, "voice_align": 32,
    "render": "pluck_render", "reset": "pluck_reset", "shown": "pluck_shown",
    "params": [
      {"name": "tone", "low": 0, "high": 100, "start": 60},
      {"name": "decay", "low": 0, "high": 100, "start": 70},
      ...
    ],
    "sounds": [
      {"name": "nylon", "file": "nylon.json"},
      {"name": "harp", "file": "harp.json"}
    ]
  }
]
```

The table the tool writes is an `Op1fEngine`:

| In the recipe | In the table | What it is |
| --- | --- | --- |
| `name` | `head.name` | What the engine list and the sound list's folder show. 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-` and `_` |
| `stereo` | `head.flags` | `OP1F_ENGINE_STEREO`: your engine renders two channels (below) |
| `shown` | `head.shown` | Told when your page comes onto the screen and goes. May be null |
| `cycles` | `cycles` | What one call of `render` takes at most, for one voice, 1 to 40,000 |
| `voice_bytes`, `voice_align` | the same | The working memory one voice needs, 0 to 1 MiB, and its alignment, a power of two from 4 to 4,096 |
| `render` | `render` | The engine itself: one voice's block of sound, on the audio thread |
| `reset` | `reset` | Clears one voice's memory, on the display thread. May be null |
| `params` | `count`, `params` | One to eight values, as an effect's at [level 8](level-8.md) |
| `sounds` | `sounds`, `sound` | None to 32 sounds of your engine's own, each a file beside the recipe (below) |

**The name.** Not one of the unit's own engines (`amp`, `cluster`, `digital`, `dimension`, `dna`, `dr.wave`, `drwave`, `dsynth`, `fm`, `phase`, `pulse`, `string`, `vocoder`, `voltage`), nor `sampler` or `fazer`. **Where two packs ask for the same name, the earlier file keeps it**, and the later ask is left aside with a line. Eight engines of packs in all. The ninth is left aside.

**Where it shows.** The unit sorts its engine list by name, so your engine stands among the unit's own where its name falls, not at the end. In that list your engine's row has no sounds under it (its sounds are in the sound list, below). A walk that steps from sound to sound goes straight past any such row, the unit's own `fm` and `phase` as well as yours, so look for it by its name. Picked, the sound takes your engine's name, as it would take `fm`'s.

**Level 9 is the synth's.** The drum's sounds are made another way, and no pack's engine reaches them.

## What the unit keeps doing

Your engine fills one voice's block of sound from its values and its pitch, and that's all. The unit keeps the voices: which voice plays which note, stealing a voice for a seventh note, the play modes, glide, the envelope, velocity, pan and the mix, before and after your engine. **Never apply the envelope, the velocity's loudness, the pan or the mix in `render`.** The unit applies its own afterwards, as for any engine, and you'd get them twice. Using the velocity for the sound's character is yours to do: `pluck` plucks brighter the harder the note. The unit's effect and LFOs work on your sound as on any other.

A note's start and its release reach no function of yours. The next block of that voice says them in `events`.

## What `render` is handed

`render` is called on the audio thread once a block for each voice that sounds, one voice after the other, never two at once. It's handed an `Op1fVoiceBlock`:

| Field | What it is |
| --- | --- |
| `bytes` | 52 at level 9. A later base may hand more, so read what you know |
| `slot` | The synth sound the voice plays, 0 to 7 |
| `voice` | The voice, 0 to 5 |
| `frames` | 128 |
| `rate` | The frames a second they stand for: 88,200 |
| `out` | The voice's sound, which `render` writes whole: `frames` words, each a signed Q1.31 fraction (full scale is -2^31 to 2^31 - 1). With `OP1F_ENGINE_STEREO`, 2 × `frames` words, left and right in turn, left first |
| `values` | Your values in your own numbers, `count` of them, as the knobs, the LFOs, MIDI and the random key left them for this block |
| `step` | The pitch: how far one frame moves the phase of a wave at the note's frequency, a whole cycle being 2^32. Glide and pitch bend are in it. One value for the block. A4 (440 Hz) with no bend is 21,426,141 |
| `velocity` | The note's velocity, 0 (softest) to about `0x7f000000` (hardest): MIDI's 0 to 127 in the top byte |
| `events` | What happened since the voice's last block (below) |
| `period` | A 96th of a beat in frames at 44,100, times 2^18, as at [level 6](level-6.md) |
| `playing` | Not 0 while the tape plays |
| `state` | This voice's working memory, or 0 when you asked none |

The block lives for the call only. Never keep it, or `out`'s address.

**`events`**, bits that can come together (a short note brings both of the first two):

| Bit | What |
| --- | --- |
| `OP1F_VOICE_BEGINS` | A note began on this voice |
| `OP1F_VOICE_ENDS` | The note was let go. The unit's envelope goes on, and the voice keeps being rendered until it has faded |
| `OP1F_VOICE_FRESH` | The voice has just been handed to your engine: picked, a sound loaded, another sound played, the unit started. Forget what it held |

A voice may be rendered before any `OP1F_VOICE_BEGINS` comes, when a note began just before your engine was picked. **Render silence until one comes.** `pluck` does it with one flag:

```cpp
if (now->events & (u32)OP1F_VOICE_FRESH) {           // handed anew: what it held is not this note's (the reset cleared the line)
    v->burst = 0;
    v->sounding = 0;
    ...
}
...
if (!v->sounding) {                                   // nothing plucked yet: silence, and the line left as it is
    for (u32 k = 0; k < (u32)FRAMES; ++k) out[k] = 0;
    return;
}
```

**The rules on the audio thread are level 6's, and tighter.** `render` may not wait, allocate or write a line, and it calls nothing of the firmware's and not one of the base's helpers, not even `op1f_mask`. No division by a number that isn't a power of two, no 64-bit multiply, no floating point. At most 1 KiB of stack (`OP1F_STAGE_STACK_MOST`) with whatever it calls. Write all of `out` every time. What `render` shares with your frame function is single words.

**Voices one at a time** means a word of your own data for each voice is safe without a mask. `fold` keeps its two phases a voice that way, in its own data, and asks no memory.

## The cost you declare

`"cycles"` is what one render of one voice takes at most, from 1 to 40,000 (`OP1F_ENGINE_CYCLES_MOST`). Six voices can sound at once, so the real load is up to six times that a block. It isn't counted in level 6's sum for stages, since your engine takes the place of one of the unit's, which costs about as much.

The base times every render, interrupts served meanwhile included, and holds you to it. **Your engine is switched off until the next start** when eight renders in one second take more than twice what you declared and more than that plus 20,000, or when one render takes more than 120,000 (`OP1F_ENGINE_OFF`). Its sounds are then silent, T1 says `switched off`, and the log says why. Once a period the log gives the renders, the most cycles, the average and the renders over.

**Declare from the most, not the average.** `pluck`'s first version declared 24,000. On a unit it averaged about 18,000 a render, but took up to 29,415, and 245 renders went over in 32 periods of 40. It was never switched off, since it stayed under twice its cost. The version here declares 33,000: the most seen, a ninth more, rounded up to the thousand. `fold` went from 16,000 to 25,000 the same way, after 22,235 at most.

## Working memory

`voice_bytes` is the memory one voice needs, up to 1 MiB (`OP1F_ENGINE_VOICE_MOST`). The base gives six blocks of it, one a voice, at its first frame, from level 6's working memory and inside its limits (the six count as one ask). They're cleared and never given back. **When it's refused**, your engine's sounds are silent, the log says why, and the page says `no room`.

A table every voice shares (a wavetable, say) isn't voice memory. Keep it in your pack's own data, or ask for it with a memory ask of [level 6](level-6.md).

`reset(slot, voice, state)` is optional. The base calls it on the display thread at the frame after a voice was handed to your engine, before that voice's next `render`, and the voice is silent until it returns. **That's where a voice that holds a lot gets cleared**, a string's delay line for instance. `pluck`'s clears its 16 KiB. With no `reset`, `OP1F_VOICE_FRESH` says it in the voice's next block and the voice renders at once.

## Your engine's own sounds

An engine can bring up to 32 sounds, each a file beside the recipe, named by its `name` (1 to 23 characters as a name's, none twice). The base writes them into the unit's sound list, `user/synth_presets/<engine>/<sound>.json`, and **the folder shows after the next start**.

Each is one JSON object as the unit writes a synth sound, with `"type": "dsynth"` and `"knobs"` eight whole numbers from 0 to 32767, and only the members the unit's own sounds hold. At most 2,048 bytes. The base adds `"op1f_engine"` with your engine's name at the top when it's missing. This is `pluck`'s `nylon`:

```json
{"op1f_engine": "pluck", "adsr": [64, 9600, 0, 9000, 4000, 4800, 18432, 23909], "fx_active": false, "fx_params": [10000, 12000, 8000, 16000, 0, 0, 0, 0], "fx_type": "delay", "knobs": [14745, 23592, 18021, 6553, 13106, 1638, 0, 26213], "lfo_active": false, "lfo_params": [9280, 32606, 1296, 9200, 0, 0, 0, 0], "lfo_type": "value", "name": "nylon", "octave": 0, "synth_version": 3, "type": "dsynth"}
```

`knobs` are the knob words as the unit keeps them: 0 is your value's `low` and 32767 its `high`. So `pluck`'s tone at 45 of 0 to 100 is 14745.

**`adsr` is the sound's envelope and four more settings**, eight whole numbers. The unit refuses a sound with one of them outside its range (its log says `invalid json patch`, and the sound that was there stays), so hold each to its range:

| | What | Range | Notes |
| --- | --- | --- | --- |
| 0 | attack | 64 to 16320 | Larger is slower, and the rise is a straight line: 64 is at once, 4096 about 20 ms, 8192 about a quarter second, 12288 about 4 s, 16320 about 13 s |
| 1 | decay | 64 to 16320 | Larger is longer. With a sustain of 0 the note has faded by 60 dB after 0.1 s at 64, 0.6 s at 4096, 1.4 s at 8192, 3.7 s at 12288, and about 23 s at 16320 |
| 2 | sustain | 0 to 32767 | The level a held note settles at. 0 ends the voice while the key is still down |
| 3 | release | 64 to 16320 | **Larger is shorter**, the decay's scale read from the other end: 64 fades for about 24 s, 4096 for 3.6 s, 8192 for 1.4 s, 12288 for 0.6 s, 15040 for 0.2 s, 16320 for a few hundredths |
| 4 | play mode | 2048 to 14336 | The number divided by 4096: 0 poly, 1 mono, 2 legato, 3 unison. 4000 is poly |
| 5 | portamento | 64 to 16448 | The glide between notes. Up to 4800, as the examples have it, none that can be measured; 8192 about 35 ms for an octave, 12288 about 0.2 s, 16448 over a second |
| 6 | bend range | 2048 to 18432 | The number divided by 4096: 0 one semitone, 1 two, 2 four, 3 seven, 4 an octave |
| 7 | volume | 3276 to 32767 | The sound's own volume, linear: half the number is 6 dB less. The unit's own sounds hold about 12600 to 32767 |

The release works on your engine's output after a key-up, on top of whatever your engine does itself at `OP1F_VOICE_ENDS`. A voice stays in use, and `render` keeps being called, until the release has run out: a long release keeps all six voices busy long after the keys are up. If your engine damps the note itself, give the sound a release a little longer than that and no more.

The volume is where an engine's loudness is matched to the unit's other sounds: `pluck`'s sounds carry 23909 and `fold`'s, whose engine is louder, 4000.

**When they're written:**

- Once for each version of your pack. The base remembers, in a small file of its own, which version's sounds it wrote. A sound the owner deletes comes back only with your pack's next version, and a folder he deletes likewise.
- **Only where the unit has room for all of them and 20 more of the owner's own.** The unit takes 500 user sounds, synth and drum together (100 on a unit set to the classic content), and refuses to save past that with "full". So the base counts first. With too little room it writes none, says so in the log, and weighs again at the next start. It's all of your sounds or none.
- Never over a file, and never into a folder of that name the base didn't make. That folder is the owner's, and your engine then shows no sounds of its own.
- The base never takes a file off the disk.

**Keep your list short.** Every sound you bring takes a place of the owner's 500. The examples bring two each.

## A sound saved with your engine

The unit keeps it as a sound of its own engine `dsynth`, with its eight knobs inside `dsynth`'s own ranges, and the base adds the member `op1f_engine` with your engine's name at the top. So:

- **With open-field and your pack**, the sound plays your engine again when it's loaded.
- **With open-field and without your pack**, it's silent. T1 shows the base's own page: your engine's name at the top and `'pluck' is not on this unit` under it, in the unit's own letters. The log says so once a start. Nothing is forgotten, and the sound plays your engine again when the pack is back.
- **On the original firmware**, or with the help key held at power-on, it plays the unit's `dsynth`, with its knobs where your engine left them.

Nothing is ever reset, either way.

## The page on T1, the knobs, the LFOs

While a synth sound that plays your engine shows its engine page (T1), **your page stands in for it**, as a sequencer's does at [level 5](level-5.md) and an effect's at [level 8](level-8.md). Until you call `op1f_page_stand_in(card, ask, document)` it's the base's blank page with your engine's name. `pluck` and `fold` ask for the blank page, with a document of 0, and paint it:

```cpp
if (!page.stood) {                                    // the base's blank page, to paint on
    if (op1f_page_stand_in(&op1f_card_pluck, ASK, 0)) page.stood = 1;
    ...
}
u32 how = shown_how, where = shown_where;
if (now->page == &op1f_card_pluck && now->canvas != 0 && (how & (u32)OP1F_SHOWN_UP)) paint(now->canvas, where, how);
```

**`shown(where, how)`** says which synth sound's page is up (`where`, 0 to 7), with `OP1F_SHOWN_UP`, and `OP1F_SHOWN_OFF` beside it while your engine is switched off for its cost. 0 when the page leaves the screen.

**The knobs.** Values 1 to 4 are turned by the four knobs (**blue**, **second**, **third**, **red**), and values 5 to 8 by the same knobs with **shift**. Keys and knobs go to your input function first, as for any page, and what you don't take goes to the unit, which moves your values. The base maps each knob's whole travel onto your value's range, both ways. The steps a turn makes and what an LFO does to a knob are `dsynth`'s.

**The LFOs.** The unit's LFOs, MIDI and the random key move your values as they'd move `dsynth`'s knobs, and the LFO pages name them by your names. A page that shows the values should show them as the sound plays them: that's [level 10](level-10.md).

If the base passes your engine by (no memory, switched off), the base's page says `no room` or `switched off` under the name. `pluck` and `fold` also paint `switched off` themselves when `shown` says so.

## The helpers

Level 9 adds no helper. Level 8's answer for an engine as for an effect, with a synth sound's slot, 0 to 7, as the place:

| Helper | What it does | From |
| --- | --- | --- |
| `op1f_param_get(card, ask, slot, index, &out)` | The value `index` of your engine on that sound, as set, in your own numbers. `OP1F_NOT_HERE` when that sound doesn't play your engine | The frame and input functions |
| `op1f_param_set(card, ask, slot, index, value)` | The value set: held to your range, written as the knob would write it, heard at the next block and kept by the unit's own save | The frame function |
| `op1f_riding(card, ask)` | The sounds that play your engine now: bit n for slot n | The frame and input functions |

[Level 10](level-10.md)'s `op1f_param_heard` answers for an engine too.

From base 39 a sound that plays your engine can take a cut the tape has lifted into its sample, as the sampler does, and your engine can read any sound's sample: [your engine's sample](sample.md).

## Two engines

Two engine packs play on different sounds. Only one synth sound plays at a time, so only one engine renders at a time, and switching sounds hands the voices to the other one with `OP1F_VOICE_FRESH`. Their memory is their own.

**With no engine pack on the unit, and no sound that names one, level 9 costs nothing**: not one instruction in the audio's blocks, and one line at the start.

## The lines the base writes

Sounds are said as `synth 1` to `synth 8`. The tag is `op1f_engines`.

| Line begins | What it says |
| --- | --- |
| `engine: none registered and no sound names one: level 9 is at rest` | Once a start, on a unit with no engine pack |
| `engine: '<name>' of '<pack>' rides dsynth, entry <n> of the engine list, <c> cycles a voice declared, <b> bytes a voice` | Your engine is in the list, once a start |
| `engine: '<name>' of '<pack>' is left aside: ...` | Why not: its name, its cost, its table, or that it would be the ninth |
| `engine: the knob rows of dsynth: ...` | The eight knobs' ranges your values are mapped onto, as the unit holds them, once a start |
| `engine: '<name>' value <n> '<value>' stands at <v>: the knob row of dsynth has no span` | That knob doesn't move, so that value stays at its `start` |
| `engine: '<name>' has <bytes> bytes for its six voices at ...` | Your working memory |
| `engine: '<name>' has no room for its six voices: ...` | Why not. Your sounds are silent |
| `engine: synth <n> rides '<name>'` | Your engine was picked there, or found there at a start |
| `engine: synth <n> rides nothing now: ...` | It isn't there any more: another engine was picked, a sound without it was loaded, or the unit built another engine there |
| `engine: synth <n> names '<name>', which is not on this unit: it is silent` | A sound saved with an engine whose pack is away |
| `engine: '<name>' this period: ...` | Through the base's quiet gate: renders, the most cycles, the average, and renders over |
| `engine: '<name>' is switched off until the next start: ...` | The render that went too far, or how many went over in one second |
| `engine: the unit holds <n> user sound(s) of the <m> it takes, synth and drum together` | The count, at each start an engine is registered |
| `engine: '<name>' writes its <n> sound(s): the unit holds ...` | There's room, and the sounds go in |
| `engine: '<name>' writes no sounds: the unit holds ...` | A warning: your sounds would leave fewer than 20 free. Weighed again at the next start |
| `engine: '<name>' writes no sounds: ...` | Another reason: the folder is the owner's, a sound isn't one the base can write, a file couldn't be written |
| `engine: '<name>' wrote <n> sound(s) into synth_presets/<name>` | Done for this version. 0 when the folder already held them all |
| `page: '<pack>' stands in for the engine's page of synth <n>, ...` | Your page went up |
| `page: the base's own page says '<name>' is not on this unit, for synth <n>` | The page for an engine that's away |

## Limits, in one place

| | |
| --- | --- |
| Engines of packs | 8 in all, entries 32 to 39 of the engine list |
| Where they show | In the unit's engine list (shift and T1 on a synth sound), among the unit's own by name. The synth only |
| A name | 1 to 15 characters of `a`-`z`, `0`-`9`, space, `-`, `_`. None of the unit's engines, nor `sampler` or `fazer` |
| Values | 1 to 8, a range of at most 65,535. 1 to 4 on the knobs, 5 to 8 with shift |
| A render | 128 frames at 88,200 a second, one channel or two |
| Declared cost | 1 to 40,000 cycles a voice's render |
| Switched off | Eight renders in one second over twice the declared cost and over the declared cost plus 20,000, or one render over 120,000 |
| Stack | 1 KiB a render, with whatever it calls |
| Working memory | Up to 1 MiB a voice, six voices, from level 6's working memory and inside its limits ([level 6](level-6.md#limits-in-one-place)) |
| Sounds | 32 an engine, 2,048 bytes each, written only with room for all and 20 more of the owner's |

## How `pluck` works

One voice is a delay line one period of the note long, the old trick for a string. A note plucks it: for one period a burst of noise goes in, louder and brighter the harder the note and the higher `tone`. The line feeds back through a gentle low-pass (`damping`, darker as it rises) and a gain a little under one (`decay`, longer as it rises). What you hear is read off the line at the note's pitch, with a comb at the pick's place (`pick`), a little warmth (`body`), a slow wander of the pitch (`drift`), a breath of noise that keeps the string moving (`noise`), and a `level`.

The line's length is 2^32 / `step` frames, worked out once a block by shifts and subtractions, since a pack links no division. A linear read between two frames takes care of the fraction. Its memory is 16 KiB a voice: a short header and 8,160 frames of line, enough for a note of 10.8 Hz. `pluck_reset` clears it on the display thread, so `render` never has to.

## How `fold` works

A sine at the note's pitch, made louder than full scale by `fold` (and a little by the velocity), is folded back each time it passes the top or the bottom, so one cycle turns over itself again and again. `symmetry` shifts the fold one way on the left and the other way on the right, which widens the sound. `sub` mixes in a sine an octave down, and `drive` pushes the sum into a soft limit. The sine is a parabola of the phase, with no table and no division, and the fold is one mask and one subtraction. It writes 256 words a render, left and right in turn.

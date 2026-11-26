# Your engine's sample (base 39)

**A drum sound (base 40).** With your card's [drum engine](drum-engine.md) ask, `op1f_sample_drop(card, ask, OP1F_PLACE_DRUM + slot)` drops into a drum sound that rides that engine. The unit does what the drop key on the drum sampler's page does, in its own turn: the cut goes into the drum sampler's sample; the 24 keys keep their points (the drop does not slice). The checks, the answers, the ticket and `done` are the synth's; the bound is the drum's memory, 1,764,000 halfwords (882,000 stereo frames). The unit keeps a drum sampler's sample over a restart itself, so after a restart `dropped` reads 0 and the sample is the one the unit loaded. The base's lines say `drum` where they say `synth`.

Every synth sound and every drum sound of the unit has a sample of its own, whatever engine it plays. The unit's sampler plays it, and the drop key on the sampler's page puts into it what the tape has lifted. From base 39 a sound that plays your engine ([level 9](level-9.md)) can take a lifted cut the same way. `op1f_sample_drop` asks the unit to put the cut into the sample of a synth sound that plays one of your card's engines. The unit does the work in its own turn, a few frames later, as it does for its sampler, and keeps the cut over a restart as it keeps the sampler's. `op1f_sample_get` reads the sample of any synth or drum sound, whatever it plays, so that your `render` can play it.

This is not a level. The level stays 10 and the card keeps its fields. Base 39 adds two helpers, two answers and a mark, `op1f_sample_1` ([below](#the-mark)). A pack that calls neither helper runs as before, and the unit's own sampler and its drop key work as they always did.

The words of the interface are in the header `module/build_r6e_card.h`, the section "Base 39: a sound's sample". Where this page and the header disagree, the header is right.

## What the owner meets

Say your engine is `grain`, and synth sound 2 plays it.

- **The lift.** On the tape, the owner lifts a cut with the lift key, from one track or from several, as for the sampler.
- **The drop.** On sound 2, T1 shows your engine's page, and the owner presses the drop key there. Your input function takes the key: on its down your voices fade, on its up your pack asks for the drop.
- **The sound.** A few frames later the cut is in the sound's sample. Your frame function sees the drop end and hands the sample to `render`, and the keys play it.
- **A restart.** The cut is still there after the unit is switched off and on. It comes back about four seconds after the first frame.
- **The sampler.** A sound that plays the unit's `sampler` takes a cut from its own drop key, on its own page, as before.

The base picks no key for you: any key or knob your input function takes, for an event on the key thread, may ask. The drop key, 11, is the one the owner knows from the sampler. The lift key is 10.

## How, in order

1. **On the key's down, fade your voices on that sound.** The unit mutes nothing. While it writes the sample, a voice that reads it plays something wrong (it never faults), and only your engine can fade without a click.
2. **On the key's up, ask**, from your input function: `op1f_sample_drop(card, ask, place)`, with `ask` your engine's number in your card's list of asks and `place` the synth sound, 0 to 7. It waits for nothing. It answers a ticket, 1 or more, or a refusal below 0, and after a refusal nothing is asked of the unit.
3. **In your frame function, read `op1f_sample_get`** for that place at every frame until its `done` is your ticket. On the maintainer's unit that took three or four frames.
4. **Read `answer` and `dropped`.** `answer` is the frames put in, or a refusal from the unit's turn with nothing put in. Hand `data`, `dropped` and `stereo` to `render` through words of your own, and let the voices play again.

The example takes the drop key on your engine's page. `on_page` is the synth sound whose page of yours shows, 8 when none, as your engine's `shown` notes it ([level 9](level-9.md#the-page-on-t1-the-knobs-the-lfos), "The page on T1, the knobs, the LFOs"). `render` fades while `quiet` is not 0. `set_sample` is yours too: it writes the words `render` reads, the length 0 first and the new length last, so that `render` never plays a length with another sample's address.

```cpp
static volatile unsigned on_page = 8, place, quiet, asked;           // single words, shared with render and shown

extern "C" int grain_input(const Op1fInput* e) {
    if (e->key != 11) return 0;                                      // the drop key
    if (e->what == OP1F_DOWN && on_page < 8) {                       // on our engine's page: fade first
        place = on_page;
        quiet = 1;
        return 1;
    }
    if (e->what != OP1F_UP) return 0;                                // an up comes only for a down we took
    int ticket = op1f_sample_drop(&op1f_card_grain, 0, place);       // ask 0: our engine
    if (ticket > 0) asked = ticket;
    else if (!asked) quiet = 0;                                      // refused at once, and no drop of ours is under way
    return 1;
}

extern "C" void grain_frame(const Op1fFrame* now) {
    Op1fSample got;
    if (!asked || op1f_sample_get(&op1f_card_grain, place, &got) != 0 || got.done != asked) return;
    if (got.answer >= 0) set_sample(got.data, got.dropped, got.stereo);
    asked = 0;
    quiet = 0;
}
```

## The answers

`op1f_sample_drop` answers at once:

| It answers | When |
| --- | --- |
| 1 or more | The ticket. The unit does the drop in its own turn: watch `done` |
| `OP1F_NOT_NOW` | The call did not come from your card's input function, or came for an event whose `thread` is not `OP1F_KEY_THREAD` (a key that plays notes, 13 to 53, or shift: [level 4](level-4.md#the-thread-an-event-came-on)); the disk mode is open; the sound's sample is being put back after a start |
| `OP1F_BAD_NAME` | Your card has no ask of kind engine with that number, or the place is not a synth sound's. On base 39 a drum sound's place, `OP1F_PLACE_DRUM` + slot, answers this. From base 40 a drum sound's place is taken with the number of your card's [drum engine](drum-engine.md) ask (kind 12); any other ask for it answers this |
| `OP1F_NOT_HERE` | The sound does not play that engine now, or a sound loaded there is not told to the base yet |
| `OP1F_BUSY` | A drop is under way, yours or another pack's, or the unit has other clipboard work waiting. Ask again at a later press |
| `OP1F_EMPTY` | No cut of the tape is lifted: nothing, or a copied sound in its place (the lift key on the sampler's page copies one) |
| `OP1F_TOO_LARGE` | The cut, at the sound's channel count, would pass its memory, `room` |
| `OP1F_FAILED` | The sound's sample is not where it should be, a lifted track is one the unit cannot render, or the unit did not take the drop. The log says which |

Between the ask and the unit's turn things may change: another sound loaded there, a sound copied in place of the cut. So the base checks again in the unit's turn. `answer` is then the frames put in, or one of `OP1F_NOT_HERE`, `OP1F_EMPTY`, `OP1F_TOO_LARGE`, `OP1F_NOT_NOW` and `OP1F_FAILED`, with nothing put in and no file written.

`op1f_sample_get(card, place, out)` fills `out` for any synth sound's place (0 to 7) or drum sound's (`OP1F_PLACE_DRUM` + slot), from your frame function only. It answers 0, or `OP1F_BAD_NAME` (another place, or your card is not in use), `OP1F_NOT_NOW` (not from your frame function) or `OP1F_FAILED` (the sample is not where it should be: only `bytes` is written). It calls nothing of the unit's.

## What `op1f_sample_get` hands

An `Op1fSample`, 36 bytes at base 39:

| Field | What |
| --- | --- |
| `bytes` | What the base wrote. A later base may hand more, so read what you know |
| `data` | The sample, 16-bit, left and right in turn when stereo. It stays where it is for the run: hand it to `render` through your own words |
| `stereo` | 1 or 0, as the unit holds the sound |
| `room` | The frames its memory holds at that channel count: 264,600 stereo or 529,200 mono for a synth sound. A drop never passes it |
| `capacity` | The unit's own count of frames for the sound, which a sound's load sets. A sound never loaded says a little more than `room`. A drop puts in at most this many |
| `dropped` | The frames of the last drop here, or of the cut put back at the start. 0 after a sound is loaded there |
| `ticket` | The drop into this place that is under way, waiting or running. 0 when none |
| `done` | The ticket of the last drop into this place that ended. 0 when none since the start |
| `answer` | How `done` ended: the frames put in (0 or more), or a refusal in the unit's turn (below 0) |

## What the unit never lets happen

- **Two drops at once.** One drop at a time for the whole unit, whichever pack asks. A second ask answers `OP1F_BUSY` and asks nothing.
- **A cut that would not fit.** The base refuses it with `OP1F_TOO_LARGE` and never cuts it short. The unit's own render puts in at most `capacity` frames, as it does for its sampler, so where `capacity` is below `room` a longer cut comes in cut at `capacity`, and `answer` says how many frames went in.
- **A drop into a sound that is not yours.** A pack drops only into a synth sound that plays one of its own engines. A `sampler` sound is the owner's, with its own drop key, and a sound on another pack's engine is that pack's.
- **Your code in the unit's turn.** Nothing of your pack runs outside your own functions. The base does the checks around the unit's work, and your frame function learns the end from `op1f_sample_get`.

## Tracks, channels, files and a restart

- **Several tracks.** A cut lifted from several tracks is mixed as the unit mixes one for its sampler.
- **`8 tracks`.** With `8 tracks` on the unit, a cut of its tracks 5 to 8 drops as one of tracks 1 to 4 does. Without a pack that renders those tracks, a lifted track of 5 to 8 answers `OP1F_FAILED`.
- **Channels.** The drop takes the channel count the unit holds for the sound, `stereo`. A stereo sample holds half as many frames as a mono one.
- **The tape playing.** A drop while the tape plays is like any other.
- **Files.** A drop that passes makes the unit write the sound's sample file and its file of the synth's sounds, as a drop into its sampler does. The base writes no sample file. It notes the cut's length and channels beside your engine's ride, in its own store.
- **A restart.** About four seconds after the first frame, the base puts the cut back from the unit's own file into each synth sound that still plays a pack's engine with the channel count it had at the drop. `dropped` is 0 until the cut is back, and while it is being put back, a drop into that sound answers `OP1F_NOT_NOW`. The first eight values of a sample put back, four frames of a stereo one, are not the cut's: they keep what the start left there, so a sum over the sample comes out a little different from the one after the drop. A new drop puts them in again.
- **A sound loaded** onto the place ends it: `dropped` goes to 0, and the cut is no longer put back at the next start.

## The mark

A pack that calls the two helpers needs base 39. They are names your pack needs, so under an older base the loader leaves it out with `nobody offers the name it needs`. Name `op1f_sample_1` in your code too (a line in the log that prints it will do), and the loader's line names the mark instead.

A pack that also runs on an older base names no mark, declares the two helpers weak and calls them only when their address is not 0:

```cpp
extern "C" int __attribute__((weak)) op1f_sample_drop(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_sample_drop");
extern "C" int __attribute__((weak)) op1f_sample_get(const Op1fCard* card, unsigned place, Op1fSample* out) OP1F_NAMED("op1f_sample_get");

int ticket = op1f_sample_drop ? op1f_sample_drop(&op1f_card_grain, 0, place) : 0;   // 0: this base has no drop
```

## The lines the base writes

Sounds are said as `synth 1` to `synth 8`. The tag is `op1f_sample` for the drops and `op1f_sample_restore` for what is put back after a start.

| Tag | Line begins | What it says |
| --- | --- | --- |
| `op1f_sample` | `sample: drop <t> into synth <n> for '<engine>': queued` | Your ask was taken, with the ticket `<t>` |
| `op1f_sample` | `sample: drop <t>: <f> frames rendered (stereo <s>, room <r>) after <k> frame(s)` | The drop ended. `answer` is `<f>` |
| `op1f_sample` | `sample: drop <t> into synth <n> refused in the unit's turn: <answer>, <why>, after <k> frame(s)` | A warning: nothing was put in |
| `op1f_sample` | `sample: drop <t>: lost from the unit's queue, not done` | A warning: `answer` is `OP1F_FAILED` |
| `op1f_sample` | `sample: '<pack>' asked a drop into <place> (ask <a>): <answer>, <why>` | A warning: refused at the ask. For `OP1F_TOO_LARGE` the line adds the cut's frames and the room. The same refusal again is counted, `sample: the refusal above came <n> time(s) more`, and after 32 such lines a start, `sample: more refusals are not said this start` |
| `op1f_sample` | `sample: the unit's own drops into its sampler so far: <n>` | Said when the count changes |
| `op1f_sample_restore` | `sample: synth <n>: put back <f> frames from <file>` | The cut is back after a start |
| `op1f_sample_restore` | `sample: synth <n>: not put back (<why>)` | A warning: no file, a file too short for the cut, the sound on no pack's engine now, or its channels changed |
| `op1f_sample_restore` | `sample: synth <n>: put back stopped: <what changed>` | A warning: the sound changed while its cut was being put back. `dropped` stays 0 |
| `op1f_sample_restore` | `sample: synth <n>: the put-back waits: the disk mode opened` | It begins again once the disk mode closes |
| `op1f_sample_restore` | `sample: synth <n>: the cut is kept no longer: a sound was loaded there` | The next start puts nothing back there |

## Limits, in one place

| | |
| --- | --- |
| Where a drop goes | A synth sound, 0 to 7, that plays one of your card's engines now. From base 40 also a drum sound, `OP1F_PLACE_DRUM` + slot, that rides your card's drum engine now (base 39: `OP1F_BAD_NAME`) |
| What `op1f_sample_get` reads | Every synth and drum sound, whatever it plays |
| A synth sound's memory, `room` | 264,600 frames stereo, 529,200 mono |
| The frames put in | At most `capacity`, the unit's own count for the sound |
| Drops at once | One for the whole unit, any pack's |
| The ask | `op1f_sample_drop` from your input function, for an event on the key thread |
| The read | `op1f_sample_get` from your frame function |
| The end | In the unit's turn, a few frames after the ask |
| After a restart | The cut back about four seconds after the first frame; `OP1F_NOT_NOW` while it is put back |
| The mark | `op1f_sample_1`, base 39 or later |

Base 39 ran on the maintainer's unit with a test pack of his, an engine that plays its sound's sample at the key's pitch. The drop key on its page took a cut of track 1, then a cut of track 5 with `8 tracks` on the unit, one of them while the tape played. Two asks in one event gave a ticket and `OP1F_BUSY`. A drum place gave `OP1F_BAD_NAME`, a sound on another engine `OP1F_NOT_HERE`, an ask from the frame function `OP1F_NOT_NOW`, and a copied sound in place of a cut `OP1F_EMPTY`. After a restart the cut came back, and the sampler's own drop key worked beside it. Not tried on the unit: a cut too large for the sound, nothing lifted at all, a mono sound, an ask while the disk mode is open. No example in this repository uses the drop yet.

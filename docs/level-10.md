# Level 10: the value as the sound plays it

Levels 8 and 9 give your effect or engine values the owner turns with the knobs, and `op1f_param_get` tells your page where each knob stands. But an LFO aimed at a value moves it block by block while the knob stays put, and so does any pack's offset on the unit's knob words (our own `lfo` adds four LFOs that way). The sound plays the moved value. A page that draws `op1f_param_get` shows a number that sits still while the owner hears it swing.

Level 10 adds one helper, `op1f_param_heard`. It answers the value the sound last used, in your own numbers, with every LFO's move in it, and from base 45 the moves of an LFO layer's wave ([layers](layers.md#an-lfo-layers-wave-base-45)). Draw that, and the owner sees the LFO on your page the way some of the unit's own pages show it.

The words of the interface are in the header `module/build_r6e_card.h`. Where this page and the header disagree, the header is right.

## Why it's there

It was found on the maintainer's unit with the first version of `pluck`. An LFO aimed at `drift` was heard doing its job, and the number on `pluck`'s page never moved. Nothing was wrong with the sound. The page was drawing the knob, not the sound.

## The examples: the four of levels 8 and 9

[`crush`](../examples/crush/README.md), [`ping`](../examples/ping/README.md), [`pluck`](../examples/pluck/README.md) and [`fold`](../examples/fold/README.md) all draw their values this way, and their cards say level 10 for it.

To see it with `pluck`: pick it on a synth sound (shift and T1), aim one of the unit's LFOs at `drift` on the LFO page (T4), open `pluck`'s page with T1 and hold a key. The `drift` number moves with the LFO. Let go, and a moment later it stands at the value the knob sets. The same goes for `crush` on a synth sound with an LFO on `bits`.

## The mark

A pack that calls `op1f_param_heard` asks for level 10 in its recipe, the way it asks for any level:

```json
"card": { "level": 10, "frame": "pluck_frame", "pick": null, "pictures": false, "input": null, "folders": [], "asks": [ ... ] }
```

A card of level 10 is a card of level 9: the same 64 bytes and the same kinds of ask. Level 10 adds no kind and no field. The card's source names `op1f_level_10`, so nothing is written by hand, and on a base below level 10 the loader leaves your pack out with `nobody offers the name it needs, op1f_level_10`. A base of level 10 offers `op1f_level_1` to `op1f_level_9` too.

So `crush` and `ping`, whose only ask is an effect of level 8, still say level 10: they call the helper. A pack that never calls it stays at the level of its asks and runs on older bases.

## The helper

```cpp
int op1f_param_heard(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out);
```

The same arguments as `op1f_param_get`: your card, the number of your effect's or engine's ask, the place (a sound's slot or the master for an effect, a synth sound's slot for an engine), and the value's index from 0. It writes the value into `out`.

| It answers | When |
| --- | --- |
| 0 | `out` holds the value as the sound last played it, held to its range |
| `OP1F_NOT_HERE` | That place doesn't have your effect or engine now |
| `OP1F_BAD_NAME` | No such ask of kind `effect` or `engine` on your card, or no such value |
| `OP1F_NOT_NOW` | Not called from your frame function or your input function |

From your frame function and your input function. It waits for nothing and takes no mask, so it's cheap to call for every value at every frame.

**When the place hasn't played lately**, it answers the value as set, exactly as `op1f_param_get` does. Lately means the last eight blocks, about 12 milliseconds. For an engine, that's while no voice of that sound sounds. For an effect, it's while its sound isn't the one that plays, or while the effect isn't called (switched off, with no tails). So when the owner lets go of the keys, your page settles back to where the knob is. A pack's effect on the master is called at every block, so its page shows the moved values for as long as the master plays.

**It's only for reading.** To turn a value, `op1f_param_set` from your frame function, as before. Draw `op1f_param_heard`, set with `op1f_param_set`, and the page always shows what's heard.

**It reads nothing of the unit's own pages.** The unit's pages that show an LFO's move do it their own way, and the base leaves that alone. It keeps a copy of its own where it hands your code its values, and that's what you're answered from.

## Drawing it

`pluck` paints its eight values at every frame its page is up, in two rows, each in its knob's colour over its name:

```cpp
static void paint(void* canvas, u32 where, u32 how) {
    char text[12];
    for (u32 i = 0; i < (u32)VALUES; ++i) {
        int value = 0;
        int got = op1f_param_heard(&op1f_card_pluck, ASK, where, i, &value);
        if (got == 0) put_number(text, value);
        else { text[0] = '-'; text[1] = 0; }
        u32 top = i < 4u;
        int middle = MIDDLES[i & 3u];
        centred(canvas, middle, top ? TOP_Y : LOW_Y, text, COLOURS[i & 3u], top ? TOP_SCALE : LOW_SCALE);
        centred(canvas, middle, top ? TOP_NAME_Y : LOW_NAME_Y, NAMES[i], NAME_GREY, 1);
    }
    ...
}
```

`where` is the sound whose page is up, as `shown` told it. Any answer but 0 paints a dash, so a page drawn in the moment a ride ends never shows a stale number. The other three examples do the same with their own layout. On all four pages, the change from their first versions was that one call: `op1f_param_get` became `op1f_param_heard`.

A number that moves at every frame can be hard to read. If yours flickers, draw a bar, or show the heard value small beside the set one. Both helpers take the same arguments.

## The lines

Level 10 writes no line of its own. A pack left out for the mark says so in the loader's line, as for any level.

## Limits, in one place

| | |
| --- | --- |
| What it answers for | Your own effects' and engines' values, at the unit's places |
| Falls back to the value as set | After eight blocks (about 12 ms) with no render of that sound, or no call of that effect there |
| From | The frame function and the input function |
| The card | Level 10, the same 64 bytes and kinds as level 9 |

Level 10 is the newest level of the base. What came after it, from base 33 on, has marks of its own instead of a level ([the base](base.md#after-level-10-parts-with-a-mark-of-their-own)). Level 10 has been run against the base's own code on a computer, with an LFO's move put in and read back, but nobody has watched it on a unit yet. If your numbers don't move with an LFO on a unit, open an issue.

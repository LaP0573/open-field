# The example pack: drone

The example of an engine of a pack's own: a synth engine picked in the engine list like a stock one, with its own faceplate and its own presets. DRONE is two saws a little apart, a third a fifth above, one low-pass and a slow drift. It needs the pack `engines`, which hosts it ([packs](../../docs/packs.md), "An engine of a pack's own"); it changes no place of the firmware itself and names no address of it. `module/op1f_engine.h` has the description an engine fills.

**To hear it**: with `drone.pack`, `engines.pack` and `base.pack` in `packs`, start the unit twice: the first start writes its two presets into `user/synth_presets/drone/`, and the firmware lists them from the next. Pick `drone` in the engine list, or one of its presets, `low hum` and `organ`.

| Knob | What it does |
| --- | --- |
| 1st, DETUNE | The second saw up to about a sixth of a semitone sharp: still, then beating, then wide |
| 2nd, TONE | The low-pass: dark to open |
| 3rd, FIFTH | How much of the third saw, a fifth above |
| 4th, DRIFT | A slow wobble of the pitch, up to about 3 cents, each voice its own |

The firmware's envelope, mixer and effects follow, as for its own engines: DRONE is a voice engine, rendered for each sounding voice of dsynth, the engine it rides on (its knob ranges, its six voices, its screen). Its faceplate draws the saws over dsynth's screen: the first in light grey, the second in the 1st knob's colour and as far behind as DETUNE, the fifth in the 3rd knob's colour and as tall as FIFTH.

| File | What it is |
| --- | --- |
| `recipe.json` | The pack's name and version, its source, and the name it offers, `op1f_engine_drone`; it needs `engines` |
| `drone.cpp` | The description: its name, the stock engine it rides on (dsynth, type 12), the knobs of a new patch, the render of one voice's block, the faceplate, and the two presets as patch files |

It uses no helper of the base and nothing of the firmware: everything comes through `engines`. Every product fits 32 bits and nothing is divided, since a pack has no copy of the compiler's 64-bit multiply or division, and the loader leaves out a pack that wants one.

Build it, from the top of the repository, into a file that is not there yet (the tool overwrites nothing):

```bash
python -I tools/pack_build.py build3 --recipe examples/drone/recipe.json --profile profiles/op1f-1.7.3.json --out out/drone.pack
```

`python -I tools/build_all.py` builds it too, into `out/examples/`, and holds it to the size and SHA-256 kept in `packs/built.json`.

# The example pack: juneaux

The example of a wagon for the effect train: an effect of a pack's own that `fx train` offers on every knob, with its own drawing on the train and its own faceplate as its page. JUNEAUX is the Juno-60 chorus. It changes no place of the firmware and names no address of it; it needs the base (level 3) and `fx train` 11 or later. [Packs](../../docs/packs.md), "A wagon for the effect train", says what a wagon is; `module/op1f_wagon.h` has the table.

**To hear it**: with `juneaux.pack`, `fx train.pack` and `base.pack` in `packs`, open the train (T3 on a synth or drum sound) and turn a knob past the train's own effects: JUNEAUX is the next pick, drawn as the slanted panel of a Juno with two voices weaving over it. The lamp's knob never offers it, since the firmware's slot cannot carry it. Tap the knob for its page.

| On its page | What happens |
| --- | --- |
| Turn the 1st knob | MODE: I, II, I+II. The light over each button shows the mode, and the voices over the panel move at its speed |
| Turn the 2nd knob | MIX: dry to the original balance of the Juno |
| Turn the 3rd knob | DEPTH: no sweep to twice the mode's. The voices swing wider |
| Turn the 4th knob | TONE: the low-pass after the line, about 7.5 kHz in the middle |
| T3, or a click of the 1st knob (fx train 12 or later) | Back to the train |

The train keeps the four values in its file and in its presets, with the wagon by its name. With `juneaux.pack` away, the train runs as before, and a train saved with JUNEAUX loads with that wagon empty.

**The sound**: one bucket-brigade line, two taps on opposite phases of a slow triangle, left and right. The numbers are from [pendragon-andyh/Juno60](https://github.com/pendragon-andyh/Juno60): mode I 0.513 Hz and mode II 0.863 Hz over 1.66 to 5.35 ms, mode I+II a 9.75 Hz sine over 3.3 to 3.7 ms, mono. The dry signal lies 1.62 dB under the wet; a low-pass of about 9 kHz stands before the line and one of two poles after it. A wagon costs at most 20,000 cycles a block.

| File | What it is |
| --- | --- |
| `recipe.json` | The pack's name and version, its sources, its picture, the name it offers (`op1f_wagon_juneaux`), and its card: level 3, the frame function `juneaux_frame`, its pictures, the input function `juneaux_input`, no folder |
| `juneaux.cpp` | The wagon's table and its sound (the audio thread), its page call and input function (the thread that handles keys), and the frame function that dresses the faceplate |
| `juneaux_art.h` | Its drawing on the train: line art in the train's colours, no ids and no arcs (an arc hangs the unit at the start) |
| `juneaux_card.s` | The card, made from the recipe by `tools/pack_build.py card`. Do not edit it |
| `juneaux_pictures.s` | The directory of its pictures, made from the recipe by `tools/pack_build.py pictures`. Do not edit it |
| `juneaux.svg.make` | The instructions for its faceplate: the head and the last tag of the unit's own tempo screen, by reference, around a drawing of ours: the panel, its buttons I and II with a light over each, the voices in a group for each of 5 depth steps and 8 phase frames, and for MIX, DEPTH and TONE a placeholder for the label and the value and a bar of 16 segments |

It uses the base's helpers `op1f_picture`, `op1f_node`, `op1f_show`, `op1f_text`, `op1f_page_open` and `op1f_log`, and needs the mark `op1f_level_3`.

Build it, from the top of the repository, into a file that is not there yet (the tool overwrites nothing):

```bash
python -I tools/pack_build.py build3 --recipe examples/juneaux/recipe.json --profile profiles/op1f-1.7.3.json --out out/juneaux.pack
```

`python -I tools/build_all.py` builds it too, into `out/examples/`, and holds it to the size and SHA-256 kept in `packs/built.json`.

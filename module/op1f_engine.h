// An engine for the OP-1 Field, offered by a pack and hosted by the pack engines (open-field, OS 1.7.3).
//
// An engine pack exports one Op1fEngine under the name op1f_engine_<name> and needs the pack engines. engines wants each
// such name as an optional one (ENGINES in module/build_n_engines.cpp, one line an engine); examples/drone is one. engines finds the engines that are
// there (optional names: an engine whose pack is away is simply not offered), and does everything TE needs:
//   - TE's engine list: a synth_types entry {"type": name, "id", "default_params"} is added when TE loads
//     content/op1_factory.json and left out when it saves it (the file on the disk stays stock);
//   - the engine's name resolves to its host engine type (the knobs, the screen, the voice it rides on), and each
//     synth slot whose patch names it is marked as that engine's;
//   - every synth note is handed to note() (from cherd.pack when it is there, from TE's note funnel when it is not);
//   - a voice of the host engine on such a slot renders through render() instead of TE's;
//   - a bus engine adds its sound to the synth bus once a block through bus(), before the T3 effect;
//   - face() draws the faceplate over the host engine's screen;
//   - version 2: its presets (TE patch files) are written into user/synth_presets/<name>/ at the start, each one only
//     when no file of that name is there, so a preset the owner changed or deleted-and-remade is never overwritten.
// Threads: note() may run on any thread, the audio thread included: no log, no file, nothing that waits. render()
// and bus() run on the audio thread. face() runs on the display thread.
#pragma once

struct Op1fPreset { const char* name; const char* json; };   // a preset's file name (no .json) and its text

enum { OP1F_ENGINE_MAGIC = 0x4e474e45 /* "ENGN" */, OP1F_ENGINE_VERSION = 2, OP1F_ENGINE_BUS = 1, OP1F_ENGINE_LIVE_KNOBS = 2 };

struct Op1fEngine {
    unsigned magic;              // OP1F_ENGINE_MAGIC
    unsigned version;            // OP1F_ENGINE_VERSION
    const char* name;            // its synth type name, as TE and the presets name it: "trumpet"
    unsigned host_type;          // the stock engine it rides on: its knob ranges, screen and voice (12 = dsynth, 13 = voltage: both
                                 // polyphonic, six voices; a mono engine renders once a block)
    short defaults[8];           // the default knob values, 0..32767 (a new patch's)
    unsigned flags;              // OP1F_ENGINE_BUS: its sound goes onto the synth bus (bus()), the host voice stays silent;
                                 // OP1F_ENGINE_LIVE_KNOBS: render() gets the knobs as set, without TE's LFO on them
    // a synth note, any thread: on 1 = down (vel 0..127), 0 = up
    void (*note)(int note, int on, int vel);
    // one block of its voice: out = 64 frames of interleaved stereo, Q31 (a bus engine writes silence here and keeps its
    // sound for bus()); params = the slot's 8 knobs (Q31), with TE's LFO applied unless OP1F_ENGINE_LIVE_KNOBS; pitch = TE's pitch word; block = the
    // host's block counter (one step per audio block); voice = which host voice this is (0..5, the host's pool): a
    // polyphonic engine keeps one state per voice, a mono one renders once per block and ignores it
    void (*render)(int* out, const volatile int* params, unsigned pitch, unsigned block, int voice);
    // a bus engine: add this block's sound to the synth bus (L, R: 64 samples each, Q31); not 0 when it added
    int (*bus)(int* L, int* R, unsigned block);
    // its faceplate over the host engine's screen (480 x 216, index y + ((479 - x) << 8)); knobs = the live knobs
    // (Q31), shift = shift held, frame = the display frame
    void (*face)(unsigned short* fb, const volatile int* knobs, int shift, unsigned frame);
    // version 2: its presets, or 0; the pack engines writes the missing ones (display thread, at its first frame)
    const Op1fPreset* presets;
    unsigned preset_count;
};

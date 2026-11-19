// The example of an engine (module/op1f_engine.h): DRONE, a synth engine of a pack's own, picked in the engine list
// like a stock one. Two saws a little apart, a third a fifth above, one low-pass and a slow drift: small enough to
// read in one go, and an engine to start from. It needs the pack engines, which hosts it; it changes no place of the
// firmware itself and names no address of it.
//
// What it shows a pack author:
//   the description op1f_engine_drone (exported by its recipe): its name, the stock engine it rides on (dsynth, type
//     12: its knob ranges, its voices, its screen), the knobs of a new patch, and the functions engines calls;
//   render(): one block of one voice. engines calls it for each sounding voice of the host on a slot whose patch is
//     DRONE, with that voice's number (0 to 5), so the engine keeps one state a voice. The firmware's own envelope,
//     mixer and effects follow, as for its own engines (the flags are 0: a voice engine, not a bus engine);
//   face(): its faceplate, drawn into the frame buffer over the host's screen, every display frame;
//   presets: two patch files, which engines writes into user/synth_presets/drone/ when they are not there.
//
// The knobs, 1st to 4th: DETUNE (the second saw up to about a sixth of a semitone sharp), TONE (the low-pass), FIFTH
// (how much of the third saw), DRIFT (a slow wobble of the pitch, each voice its own). The pitch word engines hands
// over is the phase step a sample (2^32 a cycle at 44.1 kHz). Nothing is divided: a pack has no copy of the
// compiler's division, and the loader leaves out a pack that wants it.
//
// Threads: render() on the audio thread, face() on the display thread. They share nothing but the knobs.

#include "op1f_engine.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;
enum { VOICES = 6, FRAMES = 64 };

struct Voice { u32 a, b, c, drift; i32 low; };
static Voice voices[VOICES];

static inline i32 q15(i32 knob) { return knob < 0 ? 0 : knob >> 16; }   // a knob (Q31) as 0 .. 32767

// Every product fits 32 bits: a pack has no copy of the compiler's 64-bit multiply (___muldi3) either.
static void render(int* out, const volatile int* knobs, unsigned pitch, unsigned, int voice) {
    Voice& v = voices[(unsigned)voice < VOICES ? voice : 0];
    const i32 detune = q15(knobs[0]), tone = q15(knobs[1]), fifth = q15(knobs[2]), drift = q15(knobs[3]);
    v.drift += 0x00010000u + (u32)voice * 0x1234u;                    // about 0.17 Hz, a little different a voice
    i32 t = (i32)(v.drift >> 15); if (t > 65535) t = 131071 - t;     // a triangle: 0 .. 65535 .. 0
    const i32 wobble = ((i32)(pitch >> 12) * ((t >> 1) - 16384)) >> 11;   // up to about 3 cents either way
    const u32 step = pitch + (u32)(((wobble >> 8) * drift) >> 7);
    const u32 step_b = step + (step >> 22) * (u32)detune;             // up to 1/128 sharp: about a sixth of a semitone
    const u32 step_c = step + (step >> 1);                            // a fifth above: 3/2
    const i32 cut = 1500 + ((30000 * tone) >> 15);                    // the low-pass coefficient, Q15
    const i32 c_gain = (0x4000 * fifth) >> 15;                        // the fifth's level, Q14
    for (int i = 0; i < FRAMES; ++i) {
        v.a += step; v.b += step_b; v.c += step_c;
        const i32 x = ((i32)v.a >> 3) + ((i32)v.b >> 3) + ((((i32)v.c >> 15) * c_gain) >> 2);
        v.low += ((x - v.low) >> 15) * cut;
        out[2 * i] = out[2 * i + 1] = v.low;                         // the voice's buffer: interleaved stereo, Q31
    }
}

// The faceplate: the host's screen painted black, then the first saw in light grey, the second in the 1st knob's
// colour and as far behind it as DETUNE, and the fifth in the 3rd knob's colour, as tall as FIFTH; they run along
// with the display frames. The frame buffer's index: y + ((479 - x) << 8).
static inline void dot(u16* fb, int x, int y, u16 c) { if ((unsigned)x < 480u && (unsigned)y < 216u) fb[y + ((479 - x) << 8)] = c; }
static void saw(u16* fb, int y0, int height, int speed, int shift, u16 c) {   // speed in halves: 2 one tooth in 64 pixels, 3 a fifth up
    for (int x = 40; x < 440; ++x) {
        const int ph = ((x * speed >> 1) + shift) & 63;
        const int y = y0 + height - (ph * height >> 6); dot(fb, x, y, c); dot(fb, x, y + 1, c);   // two pixels thick
        if (ph < (speed >> 1)) for (int y = y0; y <= y0 + height; ++y) { dot(fb, x, y, c); dot(fb, x + 1, y, c); }
    }
}
static void face(u16* fb, const volatile int* knobs, int, unsigned frame) {
    for (int i = 0; i < 480 * 256; ++i) fb[i] = 0;
    const u16 LIGHT = 0xce98, BLUE = 0x3b10, GREY = 0xa534;           // light grey, the 1st and the 3rd knob's colours (RGB565)
    const int apart = (knobs[0] >> 26) & 31, tall = 8 + ((knobs[2] >> 26) & 31);
    const int move = (int)(frame & 63);
    saw(fb, 40, 50, 2, move, LIGHT);
    saw(fb, 40, 50, 2, (move + apart) & 63, BLUE);
    saw(fb, 150 - tall, tall, 3, move, GREY);
}

static const Op1fPreset DRONE_PRESETS[2] = {
    { "low hum", "{\"adsr\": [3000, 8192, 32767, 16000, 5120, 6464, 4000, 4000], \"fx_active\": false, \"fx_params\": [500, 0, 10500, 16000, 0, 0, 0, 0], \"fx_type\": \"nitro\", \"lfo_active\": false, \"lfo_params\": [7680, 9225, 1024, 1520, 0, 0, 0, 0], \"lfo_type\": \"value\", \"octave\": -1, \"synth_version\": 3, \"type\": \"drone\", \"name\": \"low hum\", \"knobs\": [9000, 9000, 6000, 12000, 16384, 0, 0, 0]}" },
    { "organ", "{\"adsr\": [64, 8192, 32767, 9000, 5120, 6464, 4000, 4000], \"fx_active\": false, \"fx_params\": [500, 0, 10500, 16000, 0, 0, 0, 0], \"fx_type\": \"nitro\", \"lfo_active\": false, \"lfo_params\": [7680, 9225, 1024, 1520, 0, 0, 0, 0], \"lfo_type\": \"value\", \"octave\": 0, \"synth_version\": 3, \"type\": \"drone\", \"name\": \"organ\", \"knobs\": [4000, 20000, 24000, 2000, 16384, 0, 0, 0]}" },
};

extern "C" const Op1fEngine op1f_engine_drone asm("op1f_engine_drone");
extern "C" const Op1fEngine op1f_engine_drone = {
    OP1F_ENGINE_MAGIC, OP1F_ENGINE_VERSION, "drone", 12,
    { 12000, 16000, 8000, 8000, 16384, 0, 0, 0 },     // DETUNE, TONE, FIFTH, DRIFT; the rest unused
    0,                                                 // a voice engine: the firmware's envelope, mixer and effects follow
    0, render, 0, face, DRONE_PRESETS, 2 };

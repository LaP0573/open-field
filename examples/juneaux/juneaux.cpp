// The example of a wagon for fx train (module/op1f_wagon.h): JUNEAUX, the Juno-60 chorus, an effect of a pack's own
// that the train offers on every knob, with its own drawing on the train and its own faceplate as its page. It
// changes no place of the firmware and names no address of it; it needs the base (level 3) and fx train 11 or later.
//
// What it shows a pack author:
//   the table op1f_wagon_juneaux (exported by its recipe): its name, the state of one instance, its cost, its four
//     values' defaults, the function that runs a block, the one a tap calls, and its drawing (juneaux_art.h);
//   fx train wants the name as an optional one (OFFERED in module/build_h_chain.cpp): with this pack on the unit,
//     JUNEAUX is one more pick at the end of each knob's list, never under the lamp; with it away, the train runs as
//     before and a saved train loads with that wagon empty;
//   a tap on the wagon opens the pack's page (op1f_page_open) with that wagon's values: the four knobs turn them, the
//     page draws them, T3 goes back to the train;
//   the page is a faceplate: a picture of its own (juneaux.svg, instructions of the second kind around the unit's own
//     tempo.svg), the parts of which the frame shows and hides (op1f_show) and writes texts into (op1f_text).
//
// The sound: one bucket-brigade line, two taps on opposite phases of a slow triangle (left and right), as the Juno-60's
// chorus. The numbers are from pendragon-andyh/Juno60: mode I 0.513 Hz and mode II 0.863 Hz over 1.66 to 5.35 ms, mode
// I+II a 9.75 Hz sine over 3.3 to 3.7 ms, mono; the dry signal 1.62 dB under the wet; a ~9 kHz low-pass before the
// line and a two-pole one after it. The knobs: MODE (I, II, I+II), MIX (full: the original balance), DEPTH (none to
// twice the mode's sweep), TONE (the low-pass after the line, ~7.5 kHz in the middle).
//
// Threads: process() runs on the audio thread, page() and juneaux_input on the thread that handles keys,
// juneaux_frame on the display thread. They share single words. Nothing is divided: a pack has no copy of the
// compiler's division (___divsi3 and its kin), and the loader leaves out a pack that wants one.

#include "build_m3_card.h"
#include "op1f_wagon.h"
#include "juneaux_art.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef int i32;

enum { FRAME_LEVEL_3 = 36, INPUT_LEVEL_2 = 36, KEY_CLICK_1 = 54, KEY_CLICK_4 = 57, KEY_PRESS_1 = 61, KEY_PRESS_4 = 64 };
enum { SEGMENTS = 16, DEPTHS = 5, FRAMES = 8, LINE = 512 };

static const char LOG_FILE[] = "op1f/juneaux.cpp";            // the log shows "juneaux"

extern "C" const Op1fCard op1f_card_juneaux OP1F_NAMED("op1f_card_juneaux");

// ---------------------------------------------------------------- the sound (audio thread)
struct State { u16 p[4]; i32 s[5]; u32 pos, lfo; short line[LINE]; };   // the train clears it before the first block

static inline i32 sat_add(i32 a, i32 b) { const i32 s = (i32)((u32)a + (u32)b); return ((a ^ s) & (b ^ s)) < 0 ? (a < 0 ? (i32)0x80000000 : 0x7fffffff) : s; }
static inline i32 mul15(i32 x, i32 q) { return (x >> 15) * q + (((x & 0x7fff) * q) >> 15); }

static void process(void* state, int* L, int* R, const volatile unsigned* params) {
    State& f = *(State*)state;
    for (int k = 0; k < 4; ++k) f.p[k] = (u16)(params[k] >> 16);
    const int mode = f.p[0] < 10923 ? 0 : f.p[0] < 21845 ? 1 : 2;
    static const u32 inc[3] = { 49962u, 84049u, 949617u };                 // the LFO: 0.513, 0.863, 9.75 Hz (2^32 / 44100 a Hz)
    static const int lo[3] = { 4797628, 4797628, 9537454 }, hi[3] = { 15462236, 15462236, 10693509 };   // the delay, Q16 samples
    const int centre = (lo[mode] >> 1) + (hi[mode] >> 1), half0 = (hi[mode] >> 1) - (lo[mode] >> 1);
    const int half = (half0 >> 7) * (f.p[2] >> 7);                         // DEPTH: x0 .. x2 (16384 = x1)
    const int mix = f.p[1], lp = 12000 + (f.p[3] * 19000 >> 15);          // TONE: the low-pass after the line, Q15
    for (int i = 0; i < 64; ++i) {
        f.s[4] += mul15(((L[i] >> 2) + (R[i] >> 2)) - f.s[4], 23678);       // ~9 kHz before the line, mono, Q30
        f.line[f.pos & (LINE - 1)] = (short)(f.s[4] >> 15);
        f.lfo += inc[mode];
        const int ph = (int)(f.lfo >> 16), tri = ph < 32768 ? ph * 2 : (65535 - ph) * 2;
        int m = tri - 32767;
        if (mode == 2) m = (m * (98304 - ((m * m) >> 15))) >> 16;          // I+II: the triangle shaped to a sine
        for (int c = 0; c < 2; ++c) {
            const int mm = (c && mode != 2) ? -m : m;                      // I and II: the right tap on the other phase
            const int d = centre + (((half >> 8) * (mm >> 3)) >> 4);
            const int rp = (int)((f.pos << 16) - (u32)d);
            const int i0 = (rp >> 16) & (LINE - 1), fr = rp & 0xffff;
            const int a0 = f.line[i0], a1 = f.line[(i0 + 1) & (LINE - 1)];
            const int y = a0 + (((a1 - a0) * (fr >> 2)) >> 14);
            i32& p1 = f.s[c * 2]; i32& p2 = f.s[c * 2 + 1];
            p1 += mul15((y << 15) - p1, lp); p2 += mul15(p1 - p2, lp);     // two poles after the line, Q30
            i32* x = c ? &R[i] : &L[i];
            *x = sat_add(mul15(*x, 19035), mul15(mul15(p2 << 1, mix), 22938));   // dry 0.58, wet 0.7
        }
        ++f.pos;
    }
}

// ---------------------------------------------------------------- the page (key thread)
static void* volatile document;                       // the faceplate, once the frame made it
static volatile unsigned* volatile shown;             // the tapped wagon's values while the page shows

static void page(volatile unsigned* params) {
    if (document == 0) return;
    shown = params;
    op1f_page_open(&op1f_card_juneaux, document);
}

extern "C" const Op1fWagon op1f_wagon_juneaux OP1F_NAMED("op1f_wagon_juneaux") = {
    OP1F_WAGON_MAGIC, OP1F_WAGON_VERSION, "juneaux", sizeof(State), 20000,
    { 0, 32767, 16384, 16384, 0, 0, 0, 0 },           // mode I, the original mix, the mode's own sweep, the middle tone
    0, process, page, JUNEAUX_ART };                  // constants only: a pack runs no constructors

// A knob turns its value: MODE a setting a detent, the others a 32nd.
extern "C" int juneaux_input(const Op1fInput* e) OP1F_NAMED("juneaux_input");
extern "C" int juneaux_input(const Op1fInput* e) {
    if (e->bytes < (u32)INPUT_LEVEL_2 || !e->mine) return 0;
    if (e->what == (u32)OP1F_UP) return 1;
    volatile unsigned* p = shown;
    if (e->what == (u32)OP1F_TURN && e->key < 4 && p) {
        const int by = e->turn >> 10;                 // 1024 a detent
        if (!by) return 1;
        int v = (int)(p[e->key] >> 16);
        if (e->key == 0) { int s = (v < 10923 ? 0 : v < 21845 ? 1 : 2) + by; s = s < 0 ? 0 : s > 2 ? 2 : s; v = s == 0 ? 0 : s == 1 ? 16383 : 32767; }
        else { v += by * 1024; v = v < 0 ? 0 : v > 32767 ? 32767 : v; }
        p[e->key] = (u32)v << 16;
        return 1;
    }
    if ((e->what == (u32)OP1F_DOWN || e->what == (u32)OP1F_DOWN_SHIFT) &&
        ((e->key >= (u32)KEY_CLICK_1 && e->key <= (u32)KEY_CLICK_4) || (e->key >= (u32)KEY_PRESS_1 && e->key <= (u32)KEY_PRESS_4))) return 1;
    return 0;
}

// ---------------------------------------------------------------- the faceplate (display thread)
// The parts: the lights over the buttons I and II (jx_led1, jx_led2), the two voices weaving over the panel in a group
// for each depth step and phase frame (jx_w<depth>_<frame>, one shown at a time: the voices move at the mode's speed
// and swing wider with DEPTH), and for MIX, DEPTH and TONE a label, a value and a bar of segments (fx_lab<k>,
// fx_val<k>, fx_b<k>s<segment>).
static struct { void* seg[4][SEGMENTS]; void* led[2]; void* wave[DEPTHS][FRAMES]; int lit[4], mode, wave_depth; u32 asked, ticks, phase, wave_phase; } f;

static char* put(char* p, const char* s) { while (*s) *p++ = *s++; *p = 0; return p; }
static char* put_num(char* p, u32 v) {                // the digits by subtraction: nothing is divided
    static const u32 TEN[] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };
    int started = 0;
    for (int i = 0; i < 10; ++i) { char c = '0'; while (v >= TEN[i]) { v -= TEN[i]; ++c; } if (c != '0' || started || i == 9) { *p++ = c; started = 1; } }
    *p = 0; return p;
}

static void make_page() {
    void* d = op1f_picture(&op1f_card_juneaux, "juneaux.svg");
    if (d) {
        char name[16];
        for (int k = 1; k < 4; ++k) for (int s = 0; s < SEGMENTS; ++s) { char* q = put_num(put(name, "fx_b"), (u32)k); q = put(q, "s"); put_num(q, (u32)s); f.seg[k][s] = op1f_node(d, name); }
        f.led[0] = op1f_node(d, "jx_led1"); f.led[1] = op1f_node(d, "jx_led2");
        for (int a = 0; a < DEPTHS; ++a) for (int b = 0; b < FRAMES; ++b) {
            char* q = put_num(put(name, "jx_w"), (u32)a); q = put(q, "_"); put_num(q, (u32)b);
            f.wave[a][b] = op1f_node(d, name); op1f_show(f.wave[a][b], 0);
        }
    }
    for (int k = 0; k < 4; ++k) f.lit[k] = -1;
    f.mode = f.wave_depth = -1;
    document = d;
    op1f_log(3, LOG_FILE, 1, "juneaux: faceplate %s; the wagon offered to fx train", d ? "made" : "NOT made");
}

static void dress(void* d, volatile unsigned* p) {
    static const char* const LABEL[4] = { "mode", "mix", "depth", "tone" };
    static const char* const LAB[4] = { 0, "fx_lab1", "fx_lab2", "fx_lab3" }, * const VAL[4] = { 0, "fx_val1", "fx_val2", "fx_val3" };
    op1f_text(d, "fx_title", "juneaux", 200);
    const u32 mode_v = p[0] >> 16, depth_v = p[2] >> 16;
    const int m = mode_v < 10923 ? 0 : mode_v < 21845 ? 1 : 2;
    if (m != f.mode) { op1f_show(f.led[0], m != 1); op1f_show(f.led[1], m != 0); f.mode = m; }
    const u32 every = m == 0 ? 6u : m == 1 ? 3u : 1u;             // frames a phase step: I slow, II faster, I+II fastest
    if (++f.ticks >= every) { f.ticks = 0; f.phase = (f.phase + 1) % FRAMES; }
    const int depth = (int)((depth_v * (DEPTHS - 1) + 16384) >> 15);
    if (depth != f.wave_depth || f.phase != f.wave_phase) {
        if (f.wave_depth >= 0) op1f_show(f.wave[f.wave_depth][f.wave_phase], 0);
        op1f_show(f.wave[depth][f.phase], 1); f.wave_depth = depth; f.wave_phase = f.phase;
    }
    for (int k = 1; k < 4; ++k) {
        const u32 v = p[k] >> 16;
        char t[8]; put(put_num(t, (v * 100u + 16384u) >> 15), "%");
        op1f_text(d, LAB[k], LABEL[k], 120);
        op1f_text(d, VAL[k], t, 96);
        const int lit = (int)((v * SEGMENTS + 16384) >> 15);
        if (lit != f.lit[k]) { for (int s = 0; s < SEGMENTS; ++s) if (f.seg[k][s]) op1f_show(f.seg[k][s], s < lit); f.lit[k] = lit; }
    }
}

extern "C" void juneaux_frame(const Op1fFrame* now) OP1F_NAMED("juneaux_frame");
extern "C" void juneaux_frame(const Op1fFrame* now) {
    if (now->bytes < (u32)FRAME_LEVEL_3) return;
    if (!f.asked) { f.asked = 1; make_page(); }
    void* d = document;
    if (d && now->page == &op1f_card_juneaux && shown) dress(d, shown);
}

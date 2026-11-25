// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R2, the base pack (base.pack 21): what a frame costs, in a base that holds nothing made for one pack. A sibling
// of P3b's cost unit, which stays as it was for the bases before. The weak name it asked a feature pack for
// (whether the UFOs' scene of screen 77 is the open one) is gone, and with it the marking of its line: a period is
// about one screen by the GUI's id alone, and the line says that id and nothing behind it. Every other line is the
// elder's.
//
// The rest is the elder's, unchanged; its description follows and holds, but where it tells the train's and the
// UFOs' scenes of screen 77 apart.
//
// Pictures step P3b, the kit: what a frame costs.
//
// Why. On 2026-11-15 the UFOs' page drew 29 frames a second where it had drawn 58 the day before, and nothing on
// the unit says where a frame's time goes: the logs give a rate, and a rate comes in steps of the display's own
// (58.4, 29.2, 19.5 frames a second), so it only says between which two steps a frame's time lies. This unit
// measures the time itself, for whatever screen is shown.
//
// The display thread's frame loop, as far as this unit stands in it (0x8c0b110c to 0x8c0b118e). The loop waits
// until the display has taken the frame before. Then, for each frame, it
//
//   0x8c0b1904  calls the firmware's own work of a frame before the draw                     <- op1f_cost_tick
//   0x8c0b1914  calls the draw of the screens, with the frame buffer                         <- op1f_cost_draw
//   0x8c0b1936  calls the function that writes the frame buffer out of the cache: the kit's frame hook since M2
//               (module/m2_measure.s), which runs our own frame work first (op1f_on_frame: every unit's frame
//               function, the dressing of our pages among them) and then the firmware's function
//
// and hands the frame to the display.
//
// So a frame is: the wait, the tick, the draw, our own work, the flush and the hand-over. The two calls marked are
// retargeted to the two wrappers below (the profile's group build-p3b, in the kit's table), which take a time stamp,
// call stock's function with its arguments as they came, and take another. The kit's frame function calls
// op1f_cost_frame_in as its first statement and op1f_cost_frame_out as its last (the first frame hook's measuring unit, through weak
// references, so that a kit without this unit links as before).
//
// What is kept, for the screen that is shown ([0x848d2540], the GUI's current screen id), over a period of 128
// frames, each as the least, the mean and the most of the period, in microseconds:
//
//   work    from the loop's wake (the tick's start) to the end of our own frame work: everything a frame costs but
//           stock's flush and hand-over, which follow it, and the wait
//   tick    the GUI's work before the draw
//   draw    stock's draw of the screens: the picture's cost
//   ours    our own frame work: every unit's frame function, the dressing of the train's and the UFOs' page
//   every   from one wake to the next: the display's interval (17,123 microseconds) once, twice or three times
//
// and how many of the 128 frames took one, two, and three or more of the display's intervals. A screen that is left
// before its period is full starts a new period: a line is always about one screen.
//
// The effect train's page and the UFOs' page are one screen, 77, with two scenes in its one document
// (Build H's train page unit). The LFO unit says which of the two is open, by the name the screen's own unit asks it
// with (op1f_lfotrain_open, weak: the pack lfo may be away); the line says `(ufos)` or `(train)` behind the 77, and
// a change from one to the other starts a new period as another screen does.
//
// The line goes through the gate of build_q_quiet.h: it is printed for the first three periods of a start, when the
// screen is another or when most frames take another count of intervals than in the period before, for two periods
// after that, and otherwise once in 75 periods. A screen looked at for ten seconds has its line. And when the power
// popup comes up, the last full period's line is printed if the gate had held it back, so that the last state before
// the unit is off is on the log.
//
// Everything here runs on the display thread: the two wrappers and the frame function are called from the one loop.
// Nothing is divided (the module links no runtime): a period is 128 frames, a mean a shift by seven, and cycles
// become microseconds by a multiply and a shift (16384 / 41 is 399.6, where a microsecond is 400 cycles). Nothing is
// copied either: the period that runs and the last full one are two records that change places.
//
// A time stamp is pinned where it is written: the read of CYCLES carries a memory clobber, which nothing may be
// moved across. The listing is read for a stamp on each side of each
// stock call.

typedef unsigned int u32;
typedef unsigned char u8;

#include "build_q_quiet.h"

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef u32 (*Call3)(u32, u32, u32);                   // the two stock calls take one argument and give nothing back: the wrappers below take three and hand R0 back, so the registers go on as they came
static const char LOG_FILE[] = "../src/main/op1f_cost.cpp";

#ifdef OP1F_HOST
#include "mock_cost.h"                                 // the PC's stand-ins: a clock, the two stock calls, the screen id, the popup's byte, the logger
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_TICK ((Call3)0x8fca6940u)                // (the object 0x84d20f64): the frame loop's call at 0x8c0b1904
#define STOCK_DRAW ((Call3)0x8fc27678u)                // (the frame buffer): the frame loop's call at 0x8c0b1914
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
static inline u32 stamp() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
#endif

static const u32 CURRENT_SCREEN = 0x848d2540u;         // the id of the screen the GUI shows (0x8fc23a1c sets it)
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;      // the power popup's byte, 1 while the countdown popup is on the screen
enum { PERIOD_FRAMES = 128, PERIOD_SHIFT = 7 };
enum { ONE_AND_A_HALF = 10273972, TWO_AND_A_HALF = 17123287 };   // of the display's interval, 400,000,000 / 58.4 = 6,849,315 cycles
enum { HAVE_TICK = 1, HAVE_DRAW = 2, HAVE_IN = 4, HAVE_ALL = 7 };
enum { SCREEN_ID = 0xff };                                       // a period's screen: the GUI's id
static const u32 LONGEST = 0x0fffffffu;                // a stretch is counted as two thirds of a second at most: six digits of microseconds

struct Measure {
    u32 sum, least, most;
};

struct Period {
    u32 screen, frames;                                      // its screen and its frames so far
    Measure work, tick, draw, ours, every;
    u32 at[3];                                               // frames that took one, two, three or more of the display's intervals
};

static struct {
    u32 wake, tick_end, draw_start, draw_end, hook_in;       // this frame's stamps
    u32 have;                                                // which of them this frame has
    u32 last_wake;
    int have_last;
    Period periods[2];                                       // the period that runs and the last full one: they change places, nothing is copied
    u32 running;                                             // which of the two runs
    int full, said;                                          // there is a last full period; its line was printed
    QuietGate quiet;
    u32 lines;
} state;

extern "C" u32 op1f_cost_lines asm("op1f_cost_lines");
u32 op1f_cost_lines;                                         // for a read-out: the lines printed since the start

static inline u32 microseconds(u32 cycles) {
    if (cycles > LONGEST) cycles = LONGEST;
    return ((cycles >> 4) * 41u) >> 10;
}

static inline void add(Measure* m, u32 value) {
    m->sum += value;
    if (value < m->least) m->least = value;
    if (value > m->most) m->most = value;
}

static inline void clear(Measure* m) {
    m->sum = 0;
    m->least = 0xffffffffu;
    m->most = 0;
}

static __attribute__((noinline)) void new_period(Period* p, u32 screen) {
    p->screen = screen;
    p->frames = 0;
    clear(&p->work);
    clear(&p->tick);
    clear(&p->draw);
    clear(&p->ours);
    clear(&p->every);
    p->at[0] = p->at[1] = p->at[2] = 0;
}

static __attribute__((noinline)) void say(const Period* p) {
    u32 id = p->screen & (u32)SCREEN_ID;
    STOCK_LOG(3, LOG_FILE, 1, "frame cost: screen %u, %u frames at %u/%u/%u intervals: work %u/%u/%u us (tick %u/%u/%u, draw %u/%u/%u, ours %u/%u/%u), a frame every %u/%u/%u",
              id, p->frames, p->at[0], p->at[1], p->at[2],
              p->work.least, p->work.sum >> PERIOD_SHIFT, p->work.most,
              p->tick.least, p->tick.sum >> PERIOD_SHIFT, p->tick.most,
              p->draw.least, p->draw.sum >> PERIOD_SHIFT, p->draw.most,
              p->ours.least, p->ours.sum >> PERIOD_SHIFT, p->ours.most,
              p->every.least, p->every.sum >> PERIOD_SHIFT, p->every.most);
    state.said = 1;
    op1f_cost_lines = ++state.lines;
}

// The frame loop's call of the GUI's own work before the draw. The first thing the loop does after its wait.
extern "C" u32 op1f_cost_tick(u32 a, u32 b, u32 c) asm("op1f_cost_tick");
extern "C" u32 op1f_cost_tick(u32 a, u32 b, u32 c) {
    state.wake = stamp();
    u32 answer = STOCK_TICK(a, b, c);
    state.tick_end = stamp();
    state.have = HAVE_TICK;
    return answer;
}

// The frame loop's call of the draw.
extern "C" u32 op1f_cost_draw(u32 a, u32 b, u32 c) asm("op1f_cost_draw");
extern "C" u32 op1f_cost_draw(u32 a, u32 b, u32 c) {
    state.draw_start = stamp();
    u32 answer = STOCK_DRAW(a, b, c);
    state.draw_end = stamp();
    state.have |= HAVE_DRAW;
    return answer;
}

// The kit's frame function, at its first statement.
extern "C" void op1f_cost_frame_in() asm("op1f_cost_frame_in");
extern "C" void op1f_cost_frame_in() {
    state.hook_in = stamp();
    state.have |= HAVE_IN;
}

// The kit's frame function, at its last statement: the frame's numbers are taken, and a full period is spoken of.
extern "C" void op1f_cost_frame_out() asm("op1f_cost_frame_out");
extern "C" void op1f_cost_frame_out() {
    u32 out = stamp();
    u32 have = state.have;
    state.have = 0;
    if (quiet_popup(&state.quiet, RD8(POWER_POPUP_SHOWN) != 0) && state.full && !state.said)
        say(&state.periods[state.running ^ 1u]);             // the last full period, before the unit is off
    if (have != HAVE_ALL) return;                            // a frame that did not pass all four places is not measured
    u32 wake = state.wake;
    int had_last = state.have_last;
    u32 since = wake - state.last_wake;
    state.last_wake = wake;
    state.have_last = 1;
    u32 screen = RD32(CURRENT_SCREEN);
    Period* p = &state.periods[state.running];
    if (screen != p->screen || !had_last) {                  // another screen, or the first frame of all: a new period
        new_period(p, screen);
        return;
    }
    add(&p->work, microseconds(out - wake));
    add(&p->tick, microseconds(state.tick_end - wake));
    add(&p->draw, microseconds(state.draw_end - state.draw_start));
    add(&p->ours, microseconds(out - state.hook_in));
    add(&p->every, microseconds(since));
    ++p->at[since < (u32)ONE_AND_A_HALF ? 0 : since < (u32)TWO_AND_A_HALF ? 1 : 2];
    if (++p->frames < (u32)PERIOD_FRAMES) return;
    u32 most = p->at[0] >= p->at[1] ? (p->at[0] >= p->at[2] ? 0u : 2u) : (p->at[1] >= p->at[2] ? 1u : 2u);
    u32 s = quiet_mix(quiet_mix(quiet_start(), screen), most);       // what the line states: which screen, and at which of the display's rates
    state.full = 1;
    state.said = 0;
    if (quiet_period(&state.quiet, s, 0)) say(p);
    state.running ^= 1u;                                     // the full period is kept; the other begins anew
    new_period(&state.periods[state.running], screen);
}

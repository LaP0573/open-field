// The base pack: sound coming in starts a tape recording (OS 1.7.3+0).
//
// With REC held and the tape stopped, the firmware starts a recording on the first key of the keyboard, but not on
// sound coming in: gear on line in or USB (a sampler, a phone) cannot start it. This does: while the firmware waits
// on REC and the input goes over -40 dBFS, it presses PLAY, REC + PLAY being the firmware's own start. Once per press
// of REC.
//
// What it needs of the firmware:
//   0x848ce528  a word that is 2 while the firmware waits on REC (5 with shift held), and other values otherwise.
//   0x119025a0  the input's current block: 64 frames of stereo, interleaved, signed 32 bits, written by the audio
//               thread.
//   0x8400e3b4  the last key event the firmware read, 16 bytes; its third and fourth words (the clock at the press)
//               go into the PLAY handed on.
//   0x8c0b2794  the function the key thread hands each key event to, (int event[4]: key, down, clock, clock). It
//               puts the event in a queue.
//   0x8c0b2530  the function the key thread calls after it: it wakes the thread that takes keys from the queue.
//               Without it a queued key waits for the next real one.
// op1f_recstart_poll runs on the key thread, once per pass of its main loop (module/build_m3_recstart.s), so the
// PLAY comes on the thread every key comes on.
// Not used: the word 0x11902e44, which looks like the input's on/off, read 0 on a unit while a sampler's sound
// filled the input block. The level alone decides.

#include "build_m3_card.h"

typedef void (*KeyFn)(int*);
typedef void (*WakeFn)();

static const int RECSTART_LEVEL = 21474836;   // -40 dBFS: ~15 dB over the input's noise and ~35 dB under a sampler, on a unit
static volatile unsigned recstart_done;       // PLAY pressed in this wait

extern "C" void op1f_recstart_poll() OP1F_NAMED("op1f_recstart_poll");
extern "C" void op1f_recstart_poll(){
    const unsigned waiting = *(volatile unsigned*)0x848ce528u;
    if (waiting != 2 && waiting != 5) { recstart_done = 0; return; }
    if (recstart_done) return;
    const volatile int* in = (const volatile int*)0x119025a0u;
    for (int i = 0; i < 128; ++i) {
        const int s = in[i];
        if (s > RECSTART_LEVEL || s < -RECSTART_LEVEL) {
            recstart_done = 1;
            const volatile int* last = (const volatile int*)0x8400e3b4u;
            int event[4] = {34, 1, last[2], last[3]};                     // PLAY down, then up
            ((KeyFn)0x8c0b2794u)(event); event[1] = 0; ((KeyFn)0x8c0b2794u)(event);
            ((WakeFn)0x8c0b2530u)();
            return;
        }
    }
}

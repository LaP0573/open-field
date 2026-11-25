// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R9, base.pack 33: the rule by which a pack's page keeps its picture from frame to frame (the cards unit,
// module/build_r6d_cards.cpp, includes this file).
//
// A frame runs the firmware's render loop first and the base's frame call after it. The loop clears the whole
// picture to black once a frame, for the first screen it draws, and draws each screen; with a pack's page shown and no
// popup up, 77 is that first screen. The frame call decides for the next loop: 77's object in op1f_hold_screen asks
// the loop to leave out both the clearing (op1f_fill_place, module/build_r6d_popups.s) and 77's render
// (op1f_popups_loop); 0 lets it draw the page whole. The display's two pictures are drawn into in turn, so a frame
// whose clearing and render are left out holds the picture of two frames before, with what was painted into it then.
//
// The frame call asks the next loop to hold only when all three hold:
//   1. the card whose page is shown (a pack's page at 77, not under the base's top page) called op1f_page_hold in
//      this frame;
//   2. the same page and the same document under 77 were shown at this frame call and the two before, so that the
//      picture the held loop keeps (the one of the frame before this one) was drawn with this page's document;
//   3. nothing asked a whole picture in this frame or the two before: the pack's `redraw`, a change the base made to a
//      document, one of the unit's popups, the base's notice or its top page over the page, the power popup, or the
//      base painting on the page.
// So after any of those the next three loops draw the page whole: both pictures are drawn again before one is held.
//
// Plain integer code with nothing called and nothing divided: what the frame call saw goes in, the decision comes
// out. The rule's state is the cards unit's, kept from frame call to frame call.

#ifndef OP1F_BUILD_R9_HOLD_H
#define OP1F_BUILD_R9_HOLD_H

// What the frame call saw in this frame.
//   page      the slot + 1 of the pack's page the frame's `page` names (a pack's page at 77), 0 for none
//   document  the document under 77 now
//   asked     not 0: that page's card called op1f_page_hold in this frame
//   whole     not 0: something asked a whole picture in this frame (rule 3's list)
//   held      not 0: this frame's loop left out both the clearing and 77's render (the frame's `held`)
struct Op1fHoldSeen {
    unsigned page, document, asked, whole, held;
};

// The rule's state.
//   page, document  what the last frame call saw
//   same            frame calls in a row, up to 3, at which that page and document were shown
//   recent          bit 0 this frame, bit 1 the one before, bit 2 two before: a whole picture asked
//   on              1 from the frame call that first asks a loop to hold until the page stops asking or goes
//   held, drawn     since it began: frames whose loop held, frames drawn whole (the frame of its end included)
struct Op1fHoldRule {
    unsigned page, document, same, recent, on, held, drawn;
};

// What op1f_hold_decide answers, bits:
//   OP1F_HOLD_KEEP     ask the next loop to hold
//   OP1F_HOLD_BEGAN    a hold began in this frame call (its line)
//   OP1F_HOLD_ENDED    a hold ended in this frame call: the page stopped asking, went, or another document came
//                      under 77; `held` and `drawn` keep its counts until the next one begins (its line)
enum { OP1F_HOLD_KEEP = 1, OP1F_HOLD_BEGAN = 2, OP1F_HOLD_ENDED = 4, OP1F_HOLD_SAME = 3, OP1F_HOLD_RECENT = 7 };

static inline unsigned op1f_hold_decide(Op1fHoldRule* r, const Op1fHoldSeen* s) {
    unsigned out = 0;
    if (r->on != 0) {
        if (s->held != 0) ++r->held;
        else ++r->drawn;
    }
    if (s->page != 0 && s->page == r->page && s->document == r->document) {
        if (r->same < (unsigned)OP1F_HOLD_SAME) ++r->same;
    } else {
        r->same = s->page != 0 ? 1u : 0u;
    }
    r->page = s->page;
    r->document = s->document;
    r->recent = ((r->recent << 1) | (s->whole != 0 ? 1u : 0u)) & (unsigned)OP1F_HOLD_RECENT;
    unsigned asking = s->page != 0 && s->asked != 0 && r->same >= (unsigned)OP1F_HOLD_SAME ? 1u : 0u;
    if (r->on != 0 && asking == 0) {
        r->on = 0;
        out |= (unsigned)OP1F_HOLD_ENDED;
    }
    if (asking != 0 && r->recent == 0) {
        if (r->on == 0) {
            r->on = 1;
            r->held = 0;
            r->drawn = 0;
            out |= (unsigned)OP1F_HOLD_BEGAN;
        }
        out |= (unsigned)OP1F_HOLD_KEEP;
    }
    return out;
}

// A frame's mark against its count: the loop of the frame the frame call counts `frame` set `mark` to `frame` (the
// loop's own count + 1) when it left the clearing or 77's render out.
static inline unsigned op1f_hold_left(unsigned mark, unsigned frame) {
    return mark != 0 && mark == frame ? 1u : 0u;
}

// The frame's `held`: 1 only when this frame's loop left out both the clearing (`fill`, the stub's mark) and 77's
// render (`render`, op1f_popups_loop's mark).
static inline unsigned op1f_hold_held(unsigned fill, unsigned render, unsigned frame) {
    return op1f_hold_left(fill, frame) != 0 && op1f_hold_left(render, frame) != 0 ? 1u : 0u;
}

// The frame's `picture`: 0 for the first canvas the base saw (kept in `first`), 1 for the other. The display draws
// into its two pictures in turn, so it alternates frame by frame.
static inline unsigned op1f_hold_picture(unsigned* first, unsigned canvas) {
    if (*first == 0) *first = canvas;
    return canvas != 0 && canvas != *first ? 1u : 0u;
}

#endif

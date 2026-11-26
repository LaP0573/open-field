// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R3: what the units of the base share among themselves at level 5. Nothing here is offered to a pack: these are
// names between the base's own units (the cards, the sequencers, the notes heard, the folders), fixed in one place so
// that each unit can be written and tested by itself.

#ifndef OP1F_BUILD_R3_BASE_H
#define OP1F_BUILD_R3_BASE_H

#include "build_r3_card.h"

// ---- R20 (base 42): the second list of cards, and a set of its slots, for every unit of the base ----

// The loader N2 fills op1f_cards_2 beside op1f_cards: the same two words, then OP1F_SLOTS_MOST slots of three
// (1,544 bytes). The base serves from it alone: under N1 the cards unit fills it from op1f_cards (op1f_cards_lists).
// The public headers keep op1f_cards and its thirty-two (OP1F_CARDS_MOST): these names are the base's own, no pack's.
enum { OP1F_SLOTS_MOST = 128, OP1F_SLOT_WORDS = OP1F_SLOTS_MOST / 32 };
struct Op1fCards2 { unsigned count, more; Op1fCardSlot slots[OP1F_SLOTS_MOST]; };
extern "C" Op1fCards2 op1f_cards_2 OP1F_NAMED("op1f_cards_2");

// A set of slots of op1f_cards_2, a bit a slot (no heap; zeroed data or the stack). A slot at or past
// OP1F_SLOTS_MOST is in no set: setting or clearing it does nothing.
struct Op1fSlotSet { unsigned w[OP1F_SLOT_WORDS]; };
static inline void op1f_slot_set(Op1fSlotSet* s, unsigned slot) {
    if (slot < (unsigned)OP1F_SLOTS_MOST) s->w[slot >> 5] |= 1u << (slot & 31u);
}
static inline void op1f_slot_clear(Op1fSlotSet* s, unsigned slot) {
    if (slot < (unsigned)OP1F_SLOTS_MOST) s->w[slot >> 5] &= ~(1u << (slot & 31u));
}
static inline int op1f_slot_test(const Op1fSlotSet* s, unsigned slot) {
    return slot < (unsigned)OP1F_SLOTS_MOST && ((s->w[slot >> 5] >> (slot & 31u)) & 1u) != 0;
}
static inline int op1f_slot_any(const Op1fSlotSet* s) {
    return (s->w[0] | s->w[1] | s->w[2] | s->w[3]) != 0;
}
static inline void op1f_slot_none(Op1fSlotSet* s) {
    s->w[0] = s->w[1] = s->w[2] = s->w[3] = 0;
}

// Before a unit first reads op1f_cards_2: under N1 (the loader's word r18_loader_2_record 0) op1f_cards' count, more
// and slots are copied into it, once, in one masked instant; under N2 nothing is done. From any thread; no line. Every
// reader of op1f_cards_2 calls it first (or reads it only past op1f_cards_verdict, op1f_cards_slot or another name of
// the cards unit's, which call it).
extern "C" void op1f_cards_lists() OP1F_NAMED("op1f_cards_lists");

// 1 when the base uses the card of a slot of op1f_cards_2 (the check made, the card in order), 0 otherwise; any slot,
// up to OP1F_SLOTS_MOST. From any thread; no line. (op1f_cards_used answers the first thirty-two slots alone.)
extern "C" int op1f_cards_in_use(unsigned slot) OP1F_NAMED("op1f_cards_in_use");

// ---- from the cards unit (base 20's cards unit) ----

// Whether the base uses the card of a slot of op1f_cards_2 (R20; op1f_cards before base 42): 0 when it does, not 0
// when it does not (no card there, or a card that is not in order). It calls op1f_cards_lists first (under N1 its
// first call copies the first list into the second), writes nothing else, and says no line: from any thread.
extern "C" int op1f_cards_verdict(unsigned slot) OP1F_NAMED("op1f_cards_verdict");

// The calls the base makes into a pack outside its frame function. Each is recorded while it runs, so that a helper
// knows from where it was called: a pack may begin and end notes from the first five and from no other.
//   OP1F_CALL_TICK, OP1F_CALL_KEY, OP1F_CALL_RUN, OP1F_CALL_OCTAVE   a sequencer's functions
//   OP1F_CALL_INPUT    a card's input function
//   OP1F_CALL_LISTEN   a `note` of the notes heard
//   OP1F_CALL_SHOWN    an ask's `shown`, and a sequencer's `playing`
enum { OP1F_CALL_TICK = 0, OP1F_CALL_KEY = 1, OP1F_CALL_RUN = 2, OP1F_CALL_OCTAVE = 3, OP1F_CALL_INPUT = 4, OP1F_CALL_LISTEN = 5, OP1F_CALL_SHOWN = 6,
       OP1F_CALL_KINDS = 7 };

// What a record held before a call began: handed back when the call has ended.
struct Op1fCallKept { unsigned who, sp; };

// Around every call of the kinds above. op1f_cards_call_in, before it: the record of that kind is copied into `kept`
// and written anew (the card's slot, and the stack pointer here). op1f_cards_call_out, after it: the kept record is
// put back. Each of the two does its reading and writing in one masked instant, since a call of the same kind may
// come on another thread while this one runs. From any thread; neither says a line. The stack pointer written is the
// one inside op1f_cards_call_in, a few words below its caller's: op1f_cards_inside allows for that.
extern "C" void op1f_cards_call_in(unsigned kind, unsigned slot, Op1fCallKept* kept) OP1F_NAMED("op1f_cards_call_in");
extern "C" void op1f_cards_call_out(unsigned kind, const Op1fCallKept* kept) OP1F_NAMED("op1f_cards_call_out");

// 1 when, of all the base's calls into packs that are running around the caller on its own stack (those above, and a
// card's frame function), the nearest one is a tick, a key, a run, an octave or an input call of this card. 0
// otherwise: the card is not one in use, no call of the base's is around the caller on this stack, or the nearest one
// is another card's, a frame function, a `shown` or a `note` of the notes heard. A record is around the caller when
// its stack pointer lies at or above the caller's and within the reach the frame function's own test uses. From any
// thread; it says no line.
extern "C" int op1f_cards_inside(const Op1fCard* card) OP1F_NAMED("op1f_cards_inside");

// The kind of ask whose stand-in page is the page on the screen now (OP1F_ASK_SEQUENCER), 0 when none is. Written by
// the cards unit on the display thread each frame; read as one word from the display thread.
extern "C" unsigned op1f_cards_stand_in_shown OP1F_NAMED("op1f_cards_stand_in_shown");

// ---- from the sequencer unit (base 20's sequencer unit), for the cards unit ----

// A page that should stand in for a screen of the firmware now.
//   slot   the card's slot in op1f_cards
//   ask    the ask's number in that card's list
//   where  which of the thing's places (a sequencer: 0 the synth, 1 the drum)
//   how    OP1F_SHOWN_OFF and OP1F_SHOWN_COVERED as they are now; the cards unit adds OP1F_SHOWN_UP while the
//          stand-in is the page on the screen
struct Op1fStanding { unsigned slot, ask, where, how; };

// Asked by the cards unit once a frame, on the display thread: does a sequencer's page stand in now? It answers 1 and
// fills `out` while the sequencer's screen is up and the chosen sequencer of the instrument it shows is a pack's; 0
// otherwise. It says no line and calls nothing of a pack's.
extern "C" int op1f_seq_standing(Op1fStanding* out) OP1F_NAMED("op1f_seq_standing");

// Called by the cards unit once a frame, on the display thread, after its own check and before the cards' frame
// functions: the sequencer unit says what its other threads recorded, keeps the owner's choice when it changed, and
// says its period's lines.
extern "C" void op1f_seq_frame() OP1F_NAMED("op1f_seq_frame");

// ---- from the notes unit (module/build_r3_notes.cpp), for the cards unit ----

// Called by the cards unit once a frame, on the display thread, beside op1f_seq_frame: at its first call the asks to
// hear the notes are looked through, and each period it says what was heard.
extern "C" void op1f_notes_frame() OP1F_NAMED("op1f_notes_frame");

// ---- from the folders unit (base 20's folders unit), for the sequencer unit ----

// A file of the base's own written whole: `bytes` bytes of `data` to `path` (a path on the unit's disk under
// user/pf/, its name beginning with a dot, so that it is no pack's file and no folder's), by the steps of op1f_write:
// a temporary file of the base's own in tmp/, renamed over the file, its size held afterwards; user/pf/ is made first
// when it is not there. Answers `bytes` when the file landed, OP1F_NOT_NOW while the disk mode is open (nothing
// written), OP1F_FAILED when the disk did not do it (the line says which step). Display thread only.
extern "C" int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) OP1F_NAMED("op1f_folders_write_base");

#endif

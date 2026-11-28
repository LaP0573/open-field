// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R5: what the units of the base share among themselves at level 8 (effects). Nothing here is offered to a pack:
// these are names between the base's own units, fixed in one place so that each unit can be written and tested by
// itself. It adds to module/build_r3_base.h, which stands as it is (the cards unit, the sequencer unit, the notes
// unit), and includes it.
//
// The units of level 8 and their files:
//   the effects unit   base 28's effects unit, its stubs module/build_r5_effects.s: the registry of effects, the
//                      check of the asks of kind OP1F_ASK_EFFECT, the rows of the unit's lists, the picks, the rides,
//                      the effect objects the firmware makes wrapped, the run of an effect on the audio thread, the
//                      mapping of knob words and values, the helpers op1f_param_get, op1f_param_set, op1f_riding,
//                      op1f_rack_list, op1f_rack_ready, op1f_rack_run, and the mark op1f_level_8
//   the stages unit    module/build_r5_stages.cpp (a sibling of base 24's stages unit; its stubs stay
//                      module/build_r4_stages.s): calls op1f_effects_after at S1, S2 and S4 before level 6's stages
//   the memory unit    module/build_r5_memory.cpp (a sibling of base 24's memory unit): memory asked after the
//                      start, for the effects unit, by the same machinery and inside the same limits
//   the sequencer unit base 28's sequencer unit (a sibling of base 20's sequencer unit; its stubs stay
//                      module/build_r3_seq.s): at the catalog's read and write it calls the effects unit too
//   the values unit    base 28's values unit (a sibling of base 27's values unit; its stubs stay
//                      module/build_r4_keeps.s): the base's own member op1f_effect, and values kept for the master
//   the cards unit     base 28's cards unit (base 27's cards unit with what follows): kind 7
//                      known for a card of level 8, the effects unit's frame work, its
//                      stand-in page for an effect's screen
//   the screen stub    module/build_r5_input.s (base 27's screen stub with what follows): the four
//                      entries below, and the screen the controller asked
//
// The places of the firmware level 8 stands at (OS 1.7.0; 1.7.3 beside), named here once for the table and the
// profile:
//   W1  0x8d5053b4 (0x8d5053b4)  the builder's CALL.a of the factory: retargeted to op1f_fx_make (type in R0, index in
//                                R1; answers the object in R0), which calls the factory itself
//   W2  0x8d4f398c (0x8d4f398c)  the instruments' pick, its 8-byte head: a jump to op1f_fx_pick_entry, the trampoline in
//                                the room op1f_fx_pick_room
//   W3  0x8400f2cc (0x8400f2cc)  the mixer controller's word +0x28, the master's pick: op1f_fx_master_pick in its place
//                                (the firmware's function's arguments: the controller and the id)
//   W4  0x8d51750c (0x8d51750c)  the knob names' function, its 8-byte head: a jump to op1f_fx_names_entry, the room
//                                op1f_fx_names_room
//   W5  0x8d4f3c24 (0x8d4f3c24)  the setter of a record's fx_active, its 8-byte head: a jump to op1f_fx_active_entry,
//                                the room op1f_fx_active_room
//   W6  0x8d4f6468 (0x8d4f6468)  the synth record's apply, the eight bytes of its call of the on setter with the two
//                                moves of the call's arguments before it: a CALL.a to op1f_fx_apply_synth_at, which
//                                hands op1f_fx_apply_synth (slot in R0, on in R1) on (the table takes no 4-byte CALL)
//   W7  0x8d4f4f38 (0x8d4f4f38)  the drum record's apply, the same: op1f_fx_apply_drum_at, op1f_fx_apply_drum
//   W8  0x8c0babc6 (0x8c0babc6)  the master's save, its CALL.a of the master's on getter: retargeted to
//                                op1f_fx_master_saved (answers the byte the save writes as fx_active)

#ifndef OP1F_BUILD_R5_BASE_H
#define OP1F_BUILD_R5_BASE_H

#include "build_r5_card.h"
#ifndef OP1F_BUILD_R3_CARD_H
#define OP1F_BUILD_R3_CARD_H                           // so that build_r3_base.h brings what the base's units share and not level 5's words again
#endif
#include "build_r3_base.h"

// ---- from the effects unit, for the sequencer unit's sibling (the catalog) ----

// At the catalog's read, on the thread of the start, after the firmware has read the document and after the
// sequencers' rows: the first call checks every ask of kind OP1F_ASK_EFFECT of the cards in use (it says no line: the
// lines come at the first frame) and registers those in order; every call adds a row to the document's `fx_types` for
// each registered effect that has none of its name. Nothing when no effect is registered.
extern "C" void op1f_effects_catalog_read(void* document) OP1F_NAMED("op1f_effects_catalog_read");

// How many effects are registered: 0 before the catalog's read and when none is. One word, from any thread.
extern "C" volatile unsigned op1f_effects_registered OP1F_NAMED("op1f_effects_registered");

// 1 when an element of `fx_types` is a row the effects unit added (its `type` the name of a registered effect), else
// 0: the catalog's write leaves such rows out of the copy it writes. Under the database's lock: it calls only the
// JSON functions' readers.
extern "C" int op1f_effects_row_is(const void* row) OP1F_NAMED("op1f_effects_row_is");

// ---- from the effects unit, for the cards unit ----

// Once a frame on the display thread, after op1f_keeps_frame and before the cards' frame functions: the effects
// unit's frame work (its first frame's lines, the rides held against the type words, `reset` calls, the on states put
// back after a start or a load, the store written, the period's lines).
extern "C" void op1f_effects_frame() OP1F_NAMED("op1f_effects_frame");

// Does an effect's page stand in now? 1 and `out` filled while the screen the current controller asked for (the
// word op1f_screen_asked) is the effect screen of a place that rides an effect, in the synth's, the drum's or the
// mixer's mode; `out->where` is the place, `out->how` OP1F_SHOWN_OFF while that place's effect is switched off; and
// `*mode` and `*screen` the mode and the screen the stand-in takes. 0 otherwise. It says no line and calls nothing of a
// pack's. The cards unit asks it when op1f_seq_standing answers 0.
extern "C" int op1f_effects_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_effects_standing");

// As op1f_effects_standing, but whatever screen the current controller asked for: 1 and `out`, `*mode`, `*screen` filled
// while the place the owner sees in the mode the unit is in rides an effect whose type word still holds the type it
// stands on; 0 otherwise. It says no line and calls nothing of a pack's. The cards unit keeps that effect's page under
// 77 and its entry written while it answers, so that the firmware's own request of the effect's screen answers 77 at once
// (package A's report 6.1, taken by the lead: no frame of the unit's own effect page before the pack's).
extern "C" int op1f_effects_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_effects_ahead");

// The base's own bits of a standing's `how` for an effect, beside the published OP1F_SHOWN_ bits: the place's effect
// passes the sound dry because its memory was refused (OP1F_STAND_NO_ROOM) or because it was switched off for its
// cost (OP1F_STAND_OVER). The cards unit paints `no room` or `switched off` under the effect's name on the blank page
// for them, and never hands them to a pack's `shown`.
enum { OP1F_STAND_NO_ROOM = 0x100, OP1F_STAND_OVER = 0x200 };

// ---- the screen stub's words (module/build_r5_input.s), the cards unit's and the effects unit's ----

// The screen the current controller answered at the last screen request, before the stub changed it: written by the
// stub at every request, on the thread that makes it; read as one word by the display thread.
extern "C" volatile unsigned op1f_screen_asked OP1F_NAMED("op1f_screen_asked");

// The stand-in entries: a table of four pairs {mode + 1, screen}, {0, 0} for none (the coordinator's ruling of
// 2026-11-20, evening: a later level adds an entry, not a stub). Entry 0 is the instruments' effect (the synth's and
// the drum's modes), entry 1 the master's (the mixer's mode); entries 2 and 3 are free for later levels (step R6's
// engine takes one). While the mode the unit is in + 1 and the screen the controller answered are one pair, the stub
// answers 77. Written by the cards unit on the display thread (the screen before the mode; cleared mode first), after
// the stand-in's document is under 77, and by the effects unit at a pick, before it calls the firmware's pick, for the
// pick's own mode, under op1f_mask.
enum { OP1F_STAND_ENTRIES = 4, OP1F_STAND_EFFECT = 0, OP1F_STAND_MASTER = 1 };
extern "C" volatile unsigned op1f_stand_screens[2 * OP1F_STAND_ENTRIES] OP1F_NAMED("op1f_stand_screens");

// ---- from the effects unit, for the stages unit's sibling ----

// At S1 and S2 (point OP1F_AT_INSTRUMENT, host 0 the synth, 1 the drum) and S4 (OP1F_AT_MASTER_OUT, host 2), on the
// audio thread, before level 6's stages there: an effect with OP1F_EFFECT_TAILS whose place is switched off is called
// with `on` 0. Nothing when no effect is registered.
extern "C" void op1f_effects_after(unsigned point, unsigned host, int* left, int* right) OP1F_NAMED("op1f_effects_after");

// ---- from the values unit's sibling, for the effects unit ----

// The base's own member, op1f_effect, kept as a pack's values are (level 7): at the top of a sound's preset of the
// synth and the drum (read where the unit reads it, written where it writes it), for each slot in the base's store,
// and for the master in the store alone (`where` OP1F_KEEP_MASTER, `which` 0). The text is the member's JSON text.
// Get: answers the bytes written into `out` (ended with a zero), or OP1F_NO_FILE (none there, or before the first
// frame), OP1F_TOO_LARGE, OP1F_BAD_NAME (no place the member is kept at). Set: `text` 0 takes the member away; answers
// 0, OP1F_BAD_VALUE, OP1F_TOO_LARGE, OP1F_NOT_NOW (before the first frame; a sound loaded onto the slot not yet told;
// another thread wrote the slot meanwhile), OP1F_BAD_NAME. Both from the display thread only (package K's report 10.1).
extern "C" int op1f_keeps_base_get(unsigned where, unsigned which, char* out, unsigned room) OP1F_NAMED("op1f_keeps_base_get");
extern "C" int op1f_keeps_base_set(unsigned where, unsigned which, const char* text) OP1F_NAMED("op1f_keeps_base_set");

// What the values unit tells of the base's own member, as it tells a pack's `changed` (OP1F_KEPT_...): on the
// display thread, in op1f_keeps_frame, before the effects unit's frame work. A weak name in the values unit: away, it
// tells nothing.
extern "C" void op1f_effects_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_effects_changed");

// ---- from the memory unit's sibling, for the effects unit ----

// A block of working memory asked after the start, for the base's own use (an effect's instance), by the machinery
// and inside the limits of level 6 (OP1F_MEMORY_MOST, OP1F_MEMORY_KEPT, OP1F_MEMORY_ALIGN_MOST), counted with the
// packs' asks; `whose` names it on the log. Answers a handle above 0, or less than 0: OP1F_TOO_LARGE, OP1F_FAILED (a
// line says why). From the display thread.
extern "C" int op1f_memory_more(unsigned bytes, unsigned align, const char* whose) OP1F_NAMED("op1f_memory_more");

// The block of a handle op1f_memory_more answered: 0 while it is being cleared (a quarter of a megabyte a frame),
// then the cleared block, always the same until it is given back. From any thread.
extern "C" void* op1f_memory_block(int handle) OP1F_NAMED("op1f_memory_block");

// R23 (base 45): a block of the base's own given back, from the display thread, once nothing reads or writes it any
// more and no other thread can (the caller's to know). The handle is no longer the caller's: op1f_memory_block
// answers 0 for it, and a later op1f_memory_more may hand the same block out again, cleared, under the same handle.
// `whose` names it on the log. Answers 0, or OP1F_FAILED for a handle that is not a block of the base's own.
extern "C" int op1f_memory_less(int handle, const char* whose) OP1F_NAMED("op1f_memory_less");

// R23: why op1f_memory_more refused its last ask, the end of its line, for the line of the unit that asked. From the
// display thread, right after the refusal.
extern "C" const char* op1f_memory_refusal() OP1F_NAMED("op1f_memory_refusal");

// R23: how many blocks the base may hold for its own use at once: what the units that ask can hold together. The
// engines unit holds one block an engine (OP1F_ENGINES_MOST, 8); the effects unit one an effect at the instrument and
// one at the master (2 x OP1F_EFFECTS_MOST, 32) and one a run inside a pack (its 64 records). Each unit gives a
// block back before it asks another for the same holder, so the count of blocks held never passes 104. A block given
// back is handed out again; so the memory unit refuses for the count only when 104 are held.
enum { OP1F_MEMORY_MORE = 104 };

#endif

// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R6d, base 30 (the release's base): what the units of the base share among themselves at level 10, beside level
// 9's names. It adds to module/build_r6_base.h (base 29's), which stands as it is, and includes it. Nothing here is a
// pack's to call but the two names at its end, which the header for packs words the same way (a pack's header and
// this one may both be included: the declarations are the same).
//
// Level 10 adds one helper for packs, op1f_param_heard, and its mark op1f_level_10, both in the effects unit's sibling
// (module/build_r6d_effects.cpp), which holds level 8's helpers and their table of kinds; the engines unit's sibling
// (module/build_r6d_engines.cpp) answers it for an ask of kind 8 through op1f_engines_param_heard below. The value as
// heard is kept by the audio thread where the base already maps a block's knob words into a pack's numbers: an engine's
// render of a voice, an effect's call at a place of the unit's (never a run inside a pack's own sound work).

#ifndef OP1F_BUILD_R6D_BASE_H
#define OP1F_BUILD_R6D_BASE_H

#include "build_r6_base.h"

// ---- from the engines unit, for level 10's helper ----

// For an ask of kind OP1F_ASK_ENGINE of the card at `card_slot`: what op1f_param_heard answers, `place` a synth slot.
// The value `index` as the slot's voices last rendered it (its LFOs and any pack's offsets applied), or the value as
// set (op1f_engines_param_get's) where no voice of the slot rendered in the last few blocks. Answers as
// op1f_engines_param_get: 0, OP1F_ENGINE_NOT_MINE, OP1F_BAD_NAME, OP1F_NOT_HERE. It takes no mask and calls nothing.
extern "C" int op1f_engines_param_heard(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_engines_param_heard");

// ---- for packs (as the header for packs words them) ----

// Level 10. The value `index` (0 to count - 1) of this pack's effect or engine (its ask `ask`) at `place` as the sound
// plays it now, in the pack's own numbers, into `out`: the value the audio last used, with the unit's LFOs (and any
// pack's offsets on the unit's knob words) applied, where op1f_param_get answers the value as set. Where the place has
// not played in the last few blocks (no voice of a synth sound for an engine; for an effect, its sound not the one that
// plays), it answers the value as set, as op1f_param_get does. Answers 0, or less than 0: OP1F_NOT_HERE, OP1F_BAD_NAME,
// OP1F_NOT_NOW, as op1f_param_get. From the frame function and the input function. A page that shows its values draws
// these, and turns them with op1f_param_set.
extern "C" int op1f_param_heard(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_param_heard");

// The mark of a base of level 10 (a constant 10): a pack that calls op1f_param_heard names it.
extern "C" const unsigned op1f_level_10 OP1F_NAMED("op1f_level_10");

// ---- R22 (base 44): hosted pages, for packs (as module/build_r6e_card.h words them, "Base 44: hosted pages") ----

extern "C" int op1f_rack_place(const Op1fCard* card, unsigned which, unsigned place, const char* kept) OP1F_NAMED("op1f_rack_place");
extern "C" int op1f_rack_kept(const Op1fCard* card, unsigned which, char* out, unsigned room) OP1F_NAMED("op1f_rack_kept");
extern "C" int op1f_rack_page(const Op1fCard* card, unsigned ask, unsigned which, int* words) OP1F_NAMED("op1f_rack_page");
extern "C" const unsigned op1f_hosted_1 OP1F_NAMED("op1f_hosted_1");

// ---- R22 (base 44): hosted pages, from the effects unit for the cards unit and the values unit ----

// The hosted page for `place`, the place on the screen: 1 and `out` filled while a host wants it there (its
// op1f_rack_page at this frame or the one before, for that place, which still rides the host's layer, the instance
// current and ready); 0 otherwise. `out->slot` and `out->ask` are the effect's card and ask; `out->where` the place its
// pack is told (OP1F_PLACE_HOSTED + n for an effect that declares wagons, the lent place in the fallback, else
// OP1F_PLACE_HOSTED + which); `out->how` OP1F_SHOWN_OFF while the host's last run of it was off, with OP1F_STAND_OVER
// while it is switched off for its cost (an instance whose memory was refused is not ready: no page). The display
// thread (the cards unit's choice of the page); it says no line and calls nothing of a pack's.
extern "C" int op1f_effects_hosted(unsigned place, Op1fStanding* out) OP1F_NAMED("op1f_effects_hosted");

// The hosted page came on the screen (1) or left it (0): the cards unit says it where it tells the effect's `shown`
// (which it tells itself, with op1f_effects_hosted's `where`). While it is up, op1f_rack_page answers
// OP1F_HOSTED_PAGE_UP and op1f_param_set reaches the page's words. The display thread.
extern "C" void op1f_effects_hosted_up(unsigned up) OP1F_NAMED("op1f_effects_hosted_up");

// A knob's turn that no pack took while the hosted page is up: knob `knob` (0 blue to 3 red), `turn` as an
// Op1fInput's (1024 a step). Noted under a mask and moved at the next frame (op1f_effects_frame, or the host's
// op1f_rack_page first) as the unit's knob of the effect the pack's stands on: inside that knob's row, a 128th of the
// row a step; the word's bit then in the host's mask. Answers 1 when noted (the page up, the knob one of the effect's
// values that does not stand fixed), 0 when not (drop it all the same: it is never the unit's while the page is up).
// Any thread, the key thread's input among them; it says no line and calls nothing.
extern "C" int op1f_effects_hosted_turn(unsigned knob, int turn) OP1F_NAMED("op1f_effects_hosted_turn");

// For the values unit: op1f_value_get and op1f_value_set with `where` OP1F_KEEP_HOSTED (16), `which` the instance's
// number, handed here whole (the card and the caller's thread held here, as the values unit holds them): the answers
// are the two helpers', OP1F_BAD_NAME for no instance `which` of a declared effect of the card. The display thread.
extern "C" int op1f_effects_hosted_get(const Op1fCard* card, unsigned which, const char* key, char* out, unsigned room) OP1F_NAMED("op1f_effects_hosted_get");
extern "C" int op1f_effects_hosted_set(const Op1fCard* card, unsigned which, const char* key, const char* text) OP1F_NAMED("op1f_effects_hosted_set");

#endif

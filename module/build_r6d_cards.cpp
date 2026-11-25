// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Base 35 (R11), the base pack: base 33's cards, edited in place, with layers (module/build_r6e_card.h, "Base 35:
// layers"), for every pack alike. A card of level 8 or later may ask for a layer of a sound's list of LFOs (kind
// OP1F_ASK_LFO, its table the head of a kind): each such ask is held to its form at the check's lines (lfo_verdict:
// its size, its name, its one flag OP1F_LFO_DRUM, its `shown`, eight at most), kept by its number, or left aside with
// line 49; the line of the kinds this base does not know passes it by. Its page stands in as an effect's does
// (op1f_page_stand_in takes its number; else the base's blank page with its name; its `shown` told with the place),
// where neither the effects unit nor the engines unit says a page stands in and the LFO unit does (op1f_lfos_standing,
// op1f_lfos_ahead, weak: away, nothing of it), by entry 3 of the table of screens; line 48 when it is placed. A layer
// of effects (OP1F_EFFECT_LAYER) is the effects unit's to answer: while its pack has not chosen its own page, the
// effects unit answers that no page stands in, and nothing here puts a page of the base's under 77 for it.
//
// What base 33 said follows and holds.
//
// Base 33 (R9), the base pack: base 32's cards, edited in place (base 32's text is open-field 0.14's), with one rule added for
// every pack alike: a page that holds its picture. A pack's frame function, while its page is shown, may ask the base
// to keep the page's picture in the next frame (op1f_page_hold); the frame call then asks the next render loop to leave
// out its clearing of the picture and 77's render (op1f_hold_screen: module/build_r6d_popups.s leaves the clearing out,
// op1f_popups_loop the render), by the rule of module/build_r9_hold.h. What a frame function is handed grows by two
// words behind base 32's forty bytes: `held` (the loop left both out in this frame) and `picture` (which of the display's
// two pictures the canvas is). The base marks every change it makes to a document (op1f_show and op1f_move where they
// change a part, a picture made, op1f_cards_changed for the text unit), and every popup, notice, top page or painting
// of its own over a page: each asks the next three frames whole. A page that does not ask is drawn as on base 32.
// Lines 46 and 47: a page begins and ends to hold, for the first sixteen times.
//
// What base 32 said follows and holds.
//
// Base 32 (R8), the base pack: base 31's cards, edited in place (base 31's text is release 0.13's, in this repository's history), with one kind
// of ask more for a card of level 6 or later, a pack's options (OP1F_ASK_OPTIONS): the options unit's
// (module/build_r8_options.cpp), whose frame work this unit calls once a frame behind the values' unit's, a weak name
// (away: nothing of options is done), and which the line of the kinds this base does not know passes by.
//
// What base 31 said follows and holds.
//
// Base 31 (R6h), the base pack: base 30's cards, edited in place (base 30's text is release 0.12's, in this repository's history), with one rule added for
// every pack alike:
//
//   Two packs that carry a picture under one screen's name. A card of level 6 may carry two asks of kind
//   OP1F_ASK_PLACES: its first, as on base 30 but that its flags may be 0 (its list is firm) or PLACES_GIVES_WAY (its
//   list gives way), and a second one, taken only when the first was taken firm and the second is in order with the
//   flag PLACES_GIVES_WAY; any other later ask of the kind is left aside with base 30's line. A screen's picture is
//   served by the first card, in the list's order, that has it and does not give way (base 30's pass, line for line:
//   the same `pick` calls up to the same stop, the second firm one said by `twice`); only when none does, by the first
//   card that gives way for it and has it, kept with a record of its own while the pass goes on. A firm card served
//   where a card that gives way had the picture: noted in one masked instant, once a name, for sixteen names, and said
//   at the next frame (line 45). `twice` raises its count in one masked instant, and the places are worked out into
//   locals and only their final values stored (two threads may look at once).
//
// What base 30 said follows and holds.
//
// Base 30 (R6d), the base pack: the cards of base 29's second build (base 29's cards unit, which stays as it
// was) with one thing changed, for every pack alike:
//
//   The base's page for an engine away, in the unit's own letters. Where base 29 put its blank page under 77 for a
//   synth sound that names an engine whose pack is not on the unit, and painted on it the engine's name and the words
//   '<name>' is not on this unit in the base's own small font, base 30 puts there a page of a picture of its own,
//   away.svg: the blank page with two text placeholders, the name's at the top and the words' under it, each centred
//   on the page, in the light grey the letters take. The page is made once, at the first entry kept ready for an
//   engine away (away_page). The name and the words are set into its placeholders by the text unit's
//   op1f_text_base_set, centred, whenever the name the entry stands for is not the one last set, while that entry is
//   kept ready (away_letters): so they stand before the page is first drawn, and the firmware's own labels draw them as
//   they draw every word of the unit's own pages. The entry, its guards and line 41 are base 29's. Where away.svg
//   cannot be made (a line says so, once), the blank page stands in, and where a placeholder takes no text (the text
//   unit's own line says why), the base's own font stands in for both words: base 29's painting, kept for that case.
//
// What base 29's second build said follows and holds, but where it speaks of the base's own font on the page for an
// engine away.
//
// Base 29, its second build (R6b): the cards of base 29 (the cards unit of base 29's first build, which stays as it was for the
// first build) with three things added before base 29's try on a unit, each for every pack alike:
//
//   The base's page for an engine away. A synth sound that names an engine whose pack is not on the unit has no pack
//   whose page could stand in for its engine page (T1): the engines unit answers it with no card and OP1F_STAND_AWAY.
//   Such an answer is kept ready as an engine's page of a card is (ahead_of, ready_frame): while the synth sound the
//   owner sees names it, the base's blank page is put under 77 while 77 is free and not on the screen, and entry 2 of
//   the table of screens written after it, so that T1 on that sound answers 77 at once; it is taken back as an engine's
//   entry kept ready is. While that page is the one on the screen (77 shown, its document under 77, no pack's page shown,
//   no stand-in, not under the base's top page), the base paints on it the engine's name at the top, in its own font as
//   it paints an ask's name on the blank page, and under it the words '<name>' is not on this unit, as it paints `no
//   room` or `switched off` (op1f_paint_text_base of the paint unit, module/build_r6b_paint.cpp: the base's own
//   painting, with no pack's frame function running); line 41 says when it comes on the screen.
//
//   The unit's own popups over a page at 77. The firmware's render loop draws the registered screens in the order of
//   their numbers, and a page at 77 (a pack's, the base's) hid every one of the unit's popups from 60 to 76 that
//   answers on a flag of its own (the octave, the count-in, undo, save, help and the others: base 27's package A), and
//   the packs paint after the loop. At the loop's call of a screen's render (P1, base 29's popups stub), while the
//   screen the GUI shows is 77, op1f_popups_loop keeps the render of each screen from 60 to 85 but 77 (each popup, and
//   the ones above 77, the countdown to switch off among them), with the time the loop worked out for the frame, and
//   the frame call makes them after every card's frame function and the notice, in the loop's order (held_draw): each
//   once a frame, with the loop's own time, as the loop would have made it. So the order of a frame with a page at 77
//   is the loop's with 77 and the packs' painting moved before 59. The memory's notice (65), which base 27 drew a
//   second time over a pack's page, is one of them: its second call goes. Line 36 says each popup's first times.
//
//   A page set aside comes back before a ride's page (M5, found at base 28's try on a unit). A page that waits for its mode
//   (the train) over an effect's page, set aside when the sequencer's page took 77,
//   came back after the effect's page had been placed at the top again, and so never while it stood. Back in the mode
//   of the page set aside with the instrument it was put under 77 with, where an effect's or an engine's page is to be
//   placed, the page set aside goes back first (away_first, as away_frame puts it back) and the ride's page is put
//   under it (put_lowest), as the two lay before the visit. And the entry of a ride's screen is kept across a visit to
//   another mode: a ride's page that leaves for another mode, at the top or under another page, keeps its entry held
//   by its mode (ready), and an entry held for another mode than the unit is in is kept whatever document that mode puts
//   under 77 (ready_frame); so the return to the ride's screen answers 77 at once. Taken back as before when the unit
//   is back in its mode and the place no longer rides, or when the mode the unit is in has a ride of its own.
//
// What base 29's first build said follows and holds.
//
// Step R6, the base pack (base.pack 29): the cards at level 9 (module/build_r6_card.h, and module/build_r6_base.h for
// what the base's units share at level 9). A sibling of base 28's cards unit, which stays as it was:
// everything of base 28's is here as it was, and level 9's part of the cards is added, for every pack alike:
//
//   The kind. A card of level 9 (it says level 9 or later and is of level 4's size) may ask for a synth engine, kind
//   OP1F_ASK_ENGINE. The engines unit (base 29's engines unit) holds those asks to their form and says their lines,
//   so the line of the kinds this base does not know passes kind 8 for such a card. The mark op1f_level_9 is the
//   engines unit's.
//
//   The engines unit's frame work. Once a frame, behind the effects unit's: op1f_engines_frame(), a weak name.
//
//   An engine's page standing in. Where neither the sequencer unit nor the effects unit says a page stands in, the
//   engines unit is asked (op1f_engines_standing): while the screen the synth's controller asked for is the engine page
//   (T1) of a synth sound that rides a pack's engine, that engine's page stands in for that one screen of the synth's
//   mode, placed, kept, kept ready (op1f_engines_ahead) and closed as an effect's page is, by its own entry of the table
//   of screens (entry 2, OP1F_STAND_ENGINE). An engine switched off for its cost or refused its memory shows the base's
//   blank page with its name and `switched off` or `no room`, as an effect's. A sound that names an engine whose pack is
//   away has no pack whose page could stand in: the engines unit answers it with no card, and no page stands in for it
//   (the firmware's own engine page shows; the sound stays silent). One entry is kept ready at a time: where a synth
//   sound rides both a pack's effect and a pack's engine, the effect's page is the one kept ready.
//
// The engines unit's names are weak: away, this unit is base 28's in all it does. What base 28's unit said follows and
// holds.
//
// Step R5, the base pack (base.pack 28): the cards at level 8 (module/build_r5_card.h, and module/build_r5_base.h for
// what the base's units share at level 8). A sibling of base 27's cards unit, which stays as it was for base.pack
// 27: everything of base 27's is here as it was, and level 8's part of the cards is added, for every pack alike:
//
//   The kind. A card of level 8 (it says level 8 or later and is of level 4's size) may ask for an effect, kind
//   OP1F_ASK_EFFECT. The effects unit (base 28's effects unit) holds those asks to their form and says their
//   lines, so the line of the kinds this base does not know passes kind 7 for such a card; for a card below level 8 it
//   is counted there as before. The mark op1f_level_8 is the effects unit's.
//
//   The effects unit's frame work. Once a frame, behind the values' unit's (and so behind the memory unit's), and
//   before the early return for "no card asks for anything": op1f_effects_frame(), a weak name.
//
//   An effect's page standing in. Where the sequencer unit says no page stands in, the effects unit is asked
//   (op1f_effects_standing): while the screen the current controller asked for is the effect screen of a place that
//   rides a pack's effect, in the synth's, the drum's or the mixer's mode, that effect's page stands in for that one
//   screen of that mode. It is placed, kept and closed as the sequencer's stand-in is (its document, else the base's
//   blank page with the effect's name; its `shown`, told with the place; a page of the same mode over it), but where the
//   sequencer's stand-in writes the word our call in the screen request reads for the whole mode, the effect's writes
//   one entry of the table of screens that call reads (op1f_stand_screens: the instruments' entry for the synth and the
//   drum, the master's for the mixer): the screen, then the mode + 1, in one masked instant, after its document is
//   under 77; and takes it back, the mode first, in one masked instant, before the document of before goes back, and
//   only where the entry still holds what this unit wrote: an entry the effects unit wrote at a pick meanwhile is the
//   pick's, and stays. From one effect's page to another's in the same mode (a pick in the list, another sound that
//   rides one), the new page takes the old one's place in the same frame, with no request and no entry taken back
//   between. A place whose effect passes the sound dry for lack of memory or for its cost shows the base's blank page
//   with the effect's name and, under it, the base's words `no room` or `switched off`; those two bits of the base's
//   (OP1F_STAND_NO_ROOM, OP1F_STAND_OVER) are never handed to the pack's `shown`. A page of the firmware's own that
//   stands over 77 (the unit's notices) is drawn over it as over any pack's page.
//
//   The entry kept ready. Where the effects unit offers op1f_effects_ahead (a weak name: it answers as
//   op1f_effects_standing, whatever screen the controller asked), the entry of the mode the unit is in is kept while
//   the place the owner sees there rides an effect, also while its page is not up: an effect's page left for another
//   screen of the mode keeps its document under 77, and the page of a place that rides is put under 77 while 77 is free
//   and not on the screen, its entry written after it. So the firmware's own request of the effect's screen (T3, or
//   the mode's key back to that screen) answers 77 at once, with the page's own document under it, and no frame of
//   the firmware's own effect page is drawn first. The entry is taken back as soon as that no longer holds in its mode,
//   or as soon as another document stands under 77. One entry is kept ready at a time. Without that name the entry
//   lives with the stand-in alone.
//
// Compiled for the PC with base 27's harness as a sibling, with the table of screens and the effects unit's names in the
// model of the screen request, and base 27's tests run on it as well (for tests this tree does not hold).
//
// What follows is base 27's description, which holds.
//
// Step R4c, the base pack (base.pack 27): the cards of base 26 with no new level. A sibling of
// base 26's cards unit, which stays as it was for base.pack 26: everything of base 26's is here as it was, but
// for two things, each for every pack alike:
//
//   The unit's own popups over a pack's page. While the page standing in for the sequencer's screen is the page on
//   the screen and the sequencer unit says OP1F_SHOWN_COVERED (the firmware's "sequencer off" notice wants to be
//   drawn), the base, after every card's frame function, with the canvas withdrawn, calls the notice's own render
//   (screen 59) on the notice's own object, as the firmware's render loop calls it: the render darkens every pixel
//   already drawn in the frame, the page under 77 and what the packs painted on it, and draws its picture over them,
//   as over the unit's own sequencers. The same for the firmware's popups between 59 and 77 that answer yes on a flag
//   of their own whatever the screen and whose render is of the notice's form (the screen's own object, document and
//   canvas, no queue, no lock, no failure path, the loop's time not read): of them only screen 65, the memory's
//   notice, which the base draws while any pack's page is on the screen and the loop drew it in this frame, under 77.
//   Each is drawn only when the base's top page is not up and no screen of the firmware's above 77 was drawn in the
//   frame (the power popup's byte, and the loop's own record of what it drew at 78 to 85), so that those stay on top;
//   and only when its object is the one read: its table, the render in it, a document. Otherwise it is said once and
//   nothing is drawn. The word the sequencer unit reads for the notice (op1f_cards_stand_in_shown) stays 0, so the
//   loop never draws the notice under 77, and it is drawn once a frame. Base 26's aside page, its picture aside.svg
//   and the frame's canvas withheld under the notice are gone: a pack paints as usual, and its painting is darkened
//   with the page. What the drawing costs is timed and said at the period, through the gate.
//
//   The page set aside. A page that waits for its mode (OP1F_PAGE_STAYS) is no longer closed when another page takes
//   77 in another mode (a stand-in placed, another card's page asked in the mode the unit is in): it is set aside,
//   one at a time, with its document, how it was asked, its mode and the instrument it was put under 77 with, and it
//   waits for its mode again as soon as 77 is free (away_frame). A second word that our call in the firmware's screen
//   request reads, op1f_page_aside_mode (its mode + 1; base 27's screen stub), makes the request answer 77 in that
//   mode too, so a return straight from the other page to the page's mode shows no frame of the mode's own screen;
//   the base's next frame puts the page back under 77. It is closed, with its line, by its own card's close (the word
//   cleared in the same masked instant), when the instrument changed, and when another page that stays is set aside
//   in its place. Where the unit comes back to its mode while another page that stays waits in its place, the two
//   change places. A request of its own card takes it out of aside: the card's page is the one it asks for.
//
// Compiled for the PC with base 26's harness as a sibling, with the popups' objects and renders and the second word in
// the model of the screen request, and base 26's tests run on it as well (for tests this tree does not hold).
//
// What follows is base 26's description, which holds but where it speaks of the aside page, of the canvas withheld
// under the notice, of the word of the notice and of a waiting page closed when another takes the screen.
//
// Step R4b, the base pack (base.pack 26): the cards at level 7 (module/build_r4b_card.h). A sibling of
// base 24's cards unit, which stays as it was for base.pack 24: everything of base 24's is here as it was, and
// four things are added, each for every pack alike:
//
//   The mark op1f_level_7 beside the six earlier ones and op1f_bare_1. A card of level 7 is read as a card of level 4
//   is: the same 64 bytes. The line of the kinds this base does not know passes kind OP1F_ASK_KEEPS for a card of level
//   7 (it says level 7 or later and is of level 4's size): the values' unit (base 24's values unit) holds those
//   asks to their form and says its own lines. For a card below level 7 kind 6 is counted there as before.
//
//   The values' unit's frame work. Once a frame, behind the memory unit's and the stages unit's and before the early
//   return for "no card asks for anything": op1f_keeps_frame(), so that what the firmware's threads took from a file
//   is told to the packs (their `changed`) before any frame function of that frame, and the base's own store of the
//   slots' values is read and written whatever the packs ask for.
//
//   The page that stays with its mode. A page shown with the flag OP1F_PAGE_STAYS (op1f_page_show) is not closed when
//   the mode changes: where page_frame would close the page shown because its request is of another mode, it leaves
//   the page as it stands (its document under 77, the word our call in the firmware's screen request reads, the
//   request), so that the firmware's own request when the unit comes back to the mode answers 77 at once. The flag is
//   written with the request, in the same masked instant (`page_stays`). Only the page shown waits; the pages kept
//   under it close by the mode as before. A page waits only while the instrument the unit is on, read from the
//   firmware's word [0x84d21088] when the page was put under 77, is the same: when it is not, the page is closed for
//   it. While it waits, a request of another card, or a stand-in placed (stand_frame), closes it; its own card's close
//   closes it as any page; a request of its own card in the mode the unit is in makes it that mode's page. The base's
//   top page lies over it and puts back what it found. The lines: one when a page begins to wait, one when it is back
//   in its mode, and two more reasons for a close.
//
//   The stand-in gives way to the unit's own notice. While the stand-in is the page shown and the sequencer unit says
//   OP1F_SHOWN_COVERED (the firmware's notice is drawn at 59, before 77), the document under 77 is the base's aside
//   page, a picture that draws nothing (aside.svg: the head of the unit's tempo.svg, its own hidden backdrop line and
//   its end, all three read from the unit's own file), made once at the first frame that needs it, as the blank page
//   is; at the first frame without the notice the stand-in's own document goes back. The stand-in stays the page shown
//   meanwhile: its `shown`, the word of the notice and the frame's `page` are as before. And in that frame no card is
//   handed a canvas (the frame's `canvas` 0), so that neither a pack nor the base's own name of the ask paints over
//   the notice.
//
// Compiled for the PC with base 24's harness as a sibling, with the instrument's word, the aside page and the values'
// unit's frame work, and base 24's tests run on it as well (for tests this tree does not hold).
//
// What follows is base 24's description, which holds.
//
// Step R4, the base pack (base.pack 24): the cards at level 6 (base 24's header for pack authors). A sibling of
// base 21's cards unit, which stays as it was for base.pack 21: everything of base 21's is here as it was, and
// level 6 is added:
//
//   The mark op1f_level_6 beside the five earlier ones and op1f_bare_1. A card of level 6 is read as a card of level 4
//   is: the same 64 bytes.
//
//   The kinds. For a card that says level 6 or later and is of level 6's size, the asks of kinds OP1F_ASK_STAGE and
//   OP1F_ASK_MEMORY are the stages unit's and the memory unit's (base 24's stages unit, base 24's memory unit),
//   which hold them to their form and say their own lines; the asks of kind OP1F_ASK_PLACES are this unit's (below).
//   The line of the kinds this base does not know counts every other kind, and, for a card below level 6, these
//   three as well: nothing serves them there, as nothing did under base 21.
//
//   The units' frame work. Once a frame, beside the notes unit's, and before the early return for "no card asks for
//   anything": op1f_memory_frame(), then op1f_stages_frame().
//
//   Pictures that take a place. A card of level 6 names, in one ask of kind OP1F_ASK_PLACES (an Op1fPlaces), the
//   pictures of the unit whose place its pictures take. When a screen of the firmware asks for a picture
//   (op1f_cards_picture with no card), such a card is looked into, and its `pick` asked, only for a name its list
//   holds; for any other name it is passed by, whatever it carries. Where no card serves the name and such a card
//   carries a picture of it unlisted, the name is noted, from whatever thread asked, in one masked instant (sixteen names, once a
//   name), and the frame call says it at its next frame. The pack's own request (op1f_cards_picture with its card,
//   which op1f_picture makes) finds any picture of its directory by its name, as before. A card of levels 1 to 5 is
//   asked as level 1 says, exactly as under base 21. The rule binds every card that says level 6 or later, whatever
//   its size: a card too short for a list of asks has no list of names, and so serves no screen at all, while its own
//   requests find its pictures as before. The asks of this kind are looked at once, on the first request
//   for a picture or at the first frame, whichever comes first, after the cards' check: what is found is stored
//   before the word that says it was made, as the check's is; their lines follow the check's.
//
//   An ask of kind OP1F_ASK_PLACES is held to its form: its table's bytes at least an Op1fPlaces' and the same as its
//   head says, its name one to fifteen characters of a-z, 0-9, '-' and '_' with its end, no flag, no `shown`, one such
//   ask a card, 1 to OP1F_PLACES_MOST names, the list in the pack on a word with all its names, and each name plain
//   text that ends within its OP1F_PLACE_NAME bytes and ends in .svg. An ask that is not is left aside with a line,
//   its card is used, and the card then takes the place of no picture of the unit.
//
// Compiled for the PC with base 20's harness as a sibling, with the lines of level 6, and base 21's tests run on it as
// well (for tests this tree does not hold).
//
// What follows is base 21's description, which holds.
//
// Step R2, the base pack (base.pack 21): the cards of a base that holds nothing made for one pack. A sibling of
// base 20's cards unit, which stays as it was for base.pack 20. What base 20 knew of the effect train and the
// UFOs is gone: the two weak names it asked them by, the close by the train and its reason, what the train's unit
// showed at the last frame, the refusal of a page asked from a frame function while the train's or the UFOs' page was
// shown and the count of those refusals, the wait for the second frame before the base made its own screen 77, and
// op1f_cards_also, which only the picture loader of the bases before asked. A page asked from a frame function is
// taken as one asked from a key. Screen 77 is the one registered there, else the base's own, made at the first frame
// that needs it, the base's first frame among them; where its lines said "the train's" they say "another unit's".
// One mark more, beside the five of the levels: op1f_bare_1, the mark of a base that holds nothing made for one pack
// (base 21's header for pack authors).
//
// The rest is the elder's, unchanged; its description follows and holds, but where it speaks of the train's and the
// UFOs' page, of the second frame and of the four packs the picture loader asked by name.
//
// Step R3, the base pack (base.pack 20): the cards at level 5 (module/build_r3_card.h, and module/build_r3_base.h for
// what the base's units share). A sibling of base 19's cards unit, which stays as it was (base.pack 19 is its
// file): everything of base 19's is here as it was, what it knows of the effect train and the UFOs included, and
// level 5 is added:
//
//   The mark op1f_level_5 beside the four earlier ones. A card of level 5 is read as a card of level 4 is: the same
//   64 bytes. The asks' line tells the kinds this base knows (a sequencer, the notes heard), which the sequencer unit
//   and the notes unit look at and speak of, from the kinds it does not, which it leaves aside with one line.
//
//   The verdict. op1f_cards_verdict(slot) answers the check's verdict on one slot, from any thread, with no line: the
//   sequencer unit looks at the asks of the cards in use only.
//
//   The records of the calls. Around every call the base makes into a pack outside its frame function, a record of
//   that kind of call holds the card's slot and where the stack stood, in a table of the calls running now, eight
//   places, with no order assumed (op1f_cards_call_in takes a free place and notes which in what it keeps,
//   op1f_cards_call_out gives exactly that place back, each in one masked instant), so that calls on several threads
//   may end in any order. A call that finds the table full gets no record, and is counted. Every call of a card's
//   input function made here is inside a record of its kind. op1f_cards_inside(card) answers by the record nearest above the caller on its own
//   stack, the frame call's own among them: 1 for a tick, a key, a run, an octave or an input call of that card, 0
//   for anything else. A call that comes on the stack of a thread it interrupts finds its own record nearer than the
//   one it interrupted.
//
//   The stand-in. Each frame the sequencer unit is asked whether a pack's page stands in for the sequencer's screen
//   now (op1f_seq_standing). While it answers so, page_frame's neighbour stand_frame places that page itself, never
//   through the request word: shown when no page is shown, else put in as the lowest of the pages kept under the one
//   shown. It is an entry of its own, so that the same card's own page lies over it and the card's close never
//   closes it; it is never the one dropped when four lie under the page shown; it is closed only when the answer
//   stops or names another ask, or when the mode changes. Its document is the one its card set with
//   op1f_page_stand_in for that ask, from its frame function; or, while the card has set none, the base's blank page
//   with the ask's name painted by the base at the top centre, in its own font at scale 3 and light grey. The base
//   paints it at each frame before the cards' frame functions, as the paint helpers allow it from a frame call: the
//   frame call's record names the card for the instant of the painting, and no pack's code runs inside it. The
//   ask's `shown` is called on the display thread, outside any frame function and inside a record of its own, at
//   each frame at which what it would be told changed: OP1F_SHOWN_UP with the bits the sequencer unit gave while the
//   stand-in is the page on the screen, 0 when it no longer is. op1f_cards_stand_in_shown is written each frame:
//   OP1F_ASK_SEQUENCER while a sequencer's stand-in is the page on the screen, else 0.
//
//   The frame call. The sequencer unit's and the notes unit's frame work once a frame, after the check and before the
//   cards' frame functions; the early return for "no card has a frame or an input function" is passed by a frame with
//   a stand-in.
//
// Compiled for the PC, with base 19's tests run on it as well (for tests this tree does not hold).
//
// What follows is R1's description, which holds.
//
// Step R1, the base pack (base.pack 19): the cards at level 4 (module/build_r1_card.h). A sibling of
// base 18's cards unit, which stays as it was (base.pack 18 is its file): everything of base 18's is here as it
// was, the door's top page and its lines included, and level 4 is added:
//
//   The card. A card of level 4 (it says level 4 or later and is of level 4's 64 bytes) may list asks. The check
//   holds the list to be in order (sixteen at most, a list with its count) and to lie in its pack on a word, each
//   ask's table with its bytes too (4,096 at most). Level 4 defines no kind of ask: a card whose asks are in order is
//   used, and its asks are left aside with one line. The mark op1f_level_4 beside the three earlier ones.
//
//   The frame. 40 bytes: `canvas` behind level 3's 36, this frame's picture, read once a frame from the firmware's
//   word for this frame's buffer, after the draw and before the cards' frame functions. op1f_cards_canvas() answers
//   it while a card's frame function is being called on the display thread, 0 otherwise (the paint unit asks it).
//   With CANVAS_ON_BLANK_ONLY set to 1 the canvas is handed only while the base's blank page is the one shown.
//
//   The input. 40 bytes: `thread` behind level 2's 36, OP1F_KEY_THREAD for every turn and for every key outside 13 to
//   53, 0 for the keys 13 to 53 (the keys that play notes, which may come on the audio thread). And a key watched: an
//   input function of a card of level 4 may answer OP1F_SEEN. The event goes on to the other cards and to the
//   firmware as if it had answered 0, and the card is told that key's up, before the up goes where its down went. A
//   bit a card for each key, set with interrupts masked; several cards may watch one key. A turn answered OP1F_SEEN
//   is not taken and has no up. A card of level 3 or less answering 2 takes the event, as it always did.
//
//   Pages that stack. op1f_page_show(card, document, flags) shows the card's page over whatever page is there in
//   that mode. The page under it is kept when it was itself shown with op1f_page_show, and closed, as since level 2,
//   when it was opened with op1f_page_open. Up to four pages are kept under the one shown, from the lowest; a fifth
//   drops the lowest, with a line. A card has one page in the stack: a page it shows again comes to the top with its
//   new document. When the page on top closes, the page kept under it shows again, its document put under 77 at the
//   frame the close is taken, with the firmware's request still answered 77 throughout. A covered page closed by its
//   pack is taken out from under; a mode change and the train's or the UFOs' page opened over them close them all.
//   The display thread keeps the stack (`pages`); the other threads ask through the request word, as before, and
//   through a bit a card that says the card closed its page (`pages.closing`), each written with interrupts masked.
//   Document 0 is the base's own blank page: made once, at the first frame that needs it, from the base's own picture
//   blank.svg, and the same for every pack. flags must be 0.
//
//   A close seen at once. op1f_page_close, when the card's page is the one shown and nothing is kept under it and no
//   other request waits, clears the word our call in the firmware's screen request reads in the same masked instant
//   in which it withdraws the request, so that the request the firmware makes after the key answers the mode's own
//   screen. The base's top page is above all of this: while it is asked for or up, a card's close leaves the word.
//
//   The paint unit's line. At each period, for a pack whose paint helpers refused a call, one line with the counts
//   op1f_paint_take hands (a weak name: the unit links without the paint unit).
//
// What follows is base 18's description, which holds.
//
// Step S1 of piece 11, the base pack (base.pack 18): the cards unit of M3 with the base's top page added. A sibling of
// M3's cards unit, which stays as it was (base.pack 17 is its file). What is added, each in a block of its
// own and reached from one line:
//
//   The top page. One page of the base's own, which no card can open (its two functions are names inside the base,
//   not offered to packs): op1f_top_open(document, keys) asks for it from any thread, op1f_top_close() takes it off.
//   At the next frame top_frame puts the document under 77 (the train's screen, else the base's own, as a pack's
//   page), keeps the document and the word op1f_page_screen_mode that stood there, and keeps that word at the mode
//   the unit is in + 1 while the page is up, so that our call in the screen request answers 77 in any mode; when it
//   is taken off, the document and the word of before go back and the firmware is asked again. While it is up a
//   pack's page waits under it (page_frame returns at its first line: what a pack asked for is kept). From the moment
//   it is asked for until it is taken off, every key down and every turn goes to its key function and to nothing
//   else (top_input, the first line of the three input functions after their counts); an up goes where its down
//   went, and the up of a down the top page took is taken too. The door (the door's unit of base 18) is its first user.
//   The door's frame work. op1f_door_frame(ms), once a frame, before the top page is placed and before the early
//   return for "no card asks for anything".
//
// The three call sites, each one line, so that a sibling of this unit from another session takes them in one line
// each: in op1f_cards_frame, `op1f_door_frame(state.ms);` and `top_frame(...)` after the folders are settled; in
// page_frame, `if (top.shown) return;` first; in op1f_input_down, op1f_input_up and op1f_input_turn, the top page's
// line right after the count.
//
// What follows is M3's description, which holds.
//
// Step M3, the base pack: the cards at level 3 (OS 1.7.0+0). A sibling
// of M2's cards unit, which stays as it was (base.pack 15 is its file): levels 1 and 2 are that unit's
// text, with what M2 left for this base mended, and level 3's part of the cards is added:
//
//   A card of level 3 and of its size may list folders (module/build_m3_card.h): the check holds the list to lie in
//   the pack on a word, whole, with eight folders at most. What the folders are, their names and places, is the
//   folders unit's (M3's folders unit), which the frame call asks once, at the first frame, before any
//   card's frame function, and which asks this unit which cards are in use (op1f_cards_used) and whose frame function
//   is being called (op1f_cards_calling). A frame function is handed 36 bytes: level 2's 32 and `disk`, not 0 while
//   the firmware's word [0x84d25bcc] says the disk mode is open (mtp::init writes it, mtp::shutdown clears it).
//   The documents a card made through op1f_picture (M3's picture loader tells this unit, op1f_cards_made) are
//   kept, eight a card: op1f_page_open takes only one of the card's own, and the text unit (M3's text unit)
//   only one of a card's (op1f_cards_document). An open asked from a frame function while the train's or the UFOs'
//   page is shown is refused; one from a key is taken as before. With the base's own screen 77, a close leaves the
//   pack's document where it stands rather than putting back none. And the input's counts are made with interrupts
//   masked for the instant of each count, so that a count made on one thread is not lost to one made on another.
//
// What follows is M2's description, which holds.
//
// A pack that wants something of the base carries a card: a small table in its own body (module/build_m2_card.h).
// The loader, our piece in the firmware file, knows where each running pack's card stands and nothing of what a card
// holds. Once it has joined the packs and cleared their zeroed data it writes the cards of the packs that run, in the
// order of their files' names, into the list this unit offers under the name op1f_cards: each card's address, and
// its pack's first address and the one behind its last. From there on the base serves each such pack through its
// card and knows none of them by name. The marks op1f_level_1 and op1f_level_2 say what this unit serves:
//
//   The check (level 1, and three things more). Each card is looked at once, before the first use of any (check):
//   it lies in its pack on a word; it begins with a card's mark; its size is at least a card of level 1, in whole
//   words, inside the pack; its name is plain text with an end; its frame function, its choice among its pictures
//   and, for a card of level 2 and of its size, its input function are 0 or an even address inside the pack; its directory
//   of pictures is 0, or lies in the pack on a word with a directory's mark and all its entries, and every entry and
//   the directory's two texts end inside their room and no picture is larger than the picture loader's largest, a
//   megabyte. A card that fails is not used, and one line says which pack's and why. What a card says of its level
//   is printed and not judged: a pack that uses a later level names that level's mark, which this base does not
//   offer, and the loader leaves such a pack out before its card comes here. The list holds thirty-two cards, as
//   many as the loader takes pack files: a count above that is read as thirty-two, and the first line counts the
//   rest with the cards the loader says it found no slot for. The first use is the first picture stock asks for
//   while it makes its screens, on the main thread, before the first frame. What the check finds is written before
//   the word that says it was made, and the source forces that order (a barrier the compiler moves nothing across),
//   so a second caller, and the threads that hand keys in, read either nothing or all of it.
//
//   The frame call (level 1). Once a frame, from the measuring unit's frame function (M1's measuring unit),
//   on the display thread: each used card's frame function, in the list's order, handed the frame's number, the
//   clock, the mode, the number of the screen that is shown, whether shift is held, whether the power popup is up,
//   and (level 2) the card whose page is shown. Each call is timed and printed by the pack's name through the gate
//   of build_q_quiet.h, as M1's unit did.
//
//   Pictures (level 1). The picture loader (M3's picture loader) asks its four packs first, then the cards in
//   the list's order (op1f_cards_picture, op1f_cards_also), as M1's unit did. Where a second pack carries a name,
//   one line says so, once a name, for sixteen names; a name is remembered by its first 23 characters and compared
//   by them, so the line for a longer name comes once.
//
//   The page (level 2). Only one page is on the screen at a time, so the base lends the one it has, screen 77: the
//   train's and the UFOs' (Build H's train page unit) when that screen is made, else one the base makes itself, a
//   screen of the base class with no document, as the train's unit makes its own. A pack opens its page with a
//   document of its own (op1f_page_open, from its frame function or its input function), and the base puts that
//   document under 77 between two frames, on the display thread, keeps the document that stood there, and asks the
//   firmware for the screen (with interrupts masked, as our units ask from this thread since J5i). From then on the
//   firmware's screen request answers 77 while the unit stays in the mode the page was opened in: a call of ours
//   stands in it (M2's input stub, op1f_page_screen) and reads one word this unit writes. The page opened
//   last is the one shown. It is closed by op1f_page_close, when the mode changes, and when the train's or the
//   UFOs' page is opened over it; then the document of before goes back under 77 and the firmware is asked again.
//   The base makes its own screen 77 only from the second frame on: the train's unit makes its screen at the first
//   frame, after this unit's frame work, and the firmware stops the unit when a second screen takes a number.
//
//   Keys and knobs (level 2). Our calls stand at the four functions every key and knob event passes before the
//   mode's own code (M2's input stub) and ask op1f_input_down, op1f_input_up and op1f_input_turn here. An
//   event is offered first to the pack whose page is shown, then to every card that has an input function, in the
//   list's order, until one takes it; a taken event does not reach the firmware. An up goes where its down went:
//   to the pack that took the down, which is told whatever it answers, or to the firmware. These run on the thread
//   that handles keys, and for the keys of the keyboard maybe on the audio thread: they write no line, they wait
//   for nothing, and they share single words with the display thread. What they count is said by the frame's lines.
//
//   The helpers. op1f_log (module/build_m1_log.s), op1f_ms, op1f_screen, op1f_node, op1f_show, op1f_move as M1's
//   unit has them, and level 2's op1f_page_open and op1f_page_close. Each wraps a call or a fact our own units have
//   used on the unit since H2 and I2:
//     the registry 0x8fc261fc() answers the table of 86 screens by their numbers (0x84c960f4), 0 for a number no
//       screen has; stock's own registration holds a number against no bound, so this unit takes 1 to 85 and
//       answers 0 for any other. A screen's document is the word at its +4, 0 until stock has loaded it. The
//       firmware reads that word at every draw, on the display thread.
//     the lookup 0x8fc25cc0(screen, id, optional) reads the document at the screen's +4 and nothing else of the
//       screen, and stops the unit for a screen without a document or for no id, so
//       both are held here before it is called. It is handed the screen of the 85 that holds the document, which is
//       the call our units make; for a document no screen holds (a pack's own picture) it is handed four words of
//       which the second is the document.
//     a part's display byte stands at its +0x5c; 0x8c03dbbe(part, matrix) sets its matrix from nine words.
//     the base constructor 0x8fc26398(object, number, name) registers a screen of the base class (the train's unit
//       makes its own the same way); the screen request 0x8c0b2554() asks the current controller and our call.
//
// The clock is the core's cycle counter summed up frame by frame, 400,000 cycles a millisecond: it counts from the
// base's first frame and moves once a frame. Nothing is divided (the module links no runtime). Nothing here runs on
// the audio thread but the three input functions, which count and call the packs' input functions and nothing of
// stock's but the shift key's getter, which stock's own dispatcher calls on the same paths. Compiled for the PC as it
// is, with OP1F_HOST defined: the stand-ins for stock come in through a header of the PC's own, and every earlier
// base's tests run on this unit as well (for tests this tree does not hold).

#include "build_r6_card.h"                             // R6: base 28's words, every one, with level 9's
#include "build_r6_base.h"                             // R6: what the base's units share, base 28's (build_r5_base.h) and level 9's
#include "build_q_quiet.h"
#include "build_r9_hold.h"                             // R9 (base 33): the rule of a page that holds its picture
#include "build_r12_parts.h"                           // R12 (base 36): the parts to hide while a held page's rectangles are drawn again

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef u32* (*ScreensFn)();                                                // 0x8fc261fc: the table of the screens by their numbers
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);    // 0x8fc25cc0: 0 when optional and missing
typedef void (*ApplyMatrixFn)(void* node, const int* matrix);               // 0x8c03dbbe
typedef int (*ShiftFn)();                                                   // 0x8fc24ca4: non-zero while shift is held
typedef void (*ScreenCtorFn)(void* object, int number, const char* name);  // 0x8fc26398: Screen::Screen, registers the object by its number
typedef void (*RequestScreenFn)();                                          // 0x8c0b2554: the GUI asks the current controller which screen, and queues it

static const char LOG_FILE[] = "../src/main/op1f_cards.cpp";

#ifdef OP1F_HOST
#include "mock_cards_r5.h"                             // the PC's stand-ins: a clock, the logger, the registry of screens, the lookup, the matrix, the words read, the constructor, the request
#include <intrin.h>
static inline u32 stack_here() { return (u32)_AddressOfReturnAddress(); }   // M3: where the stack of the thread that runs this stands
#define OP1F_APART
#define BARRIER() ((void)0)                            // the PC's run is one thread
#define PINNED(pointer) ((void)0)                      // R6: the PC's compiler needs no pinning
#define MASK_OFF(mask) ((void)(mask = 0))
#define MASK_ON(mask) ((void)(mask))
extern "C" { u32 op1f_hold_screen; u32 op1f_fill_left; }    // R9 (base 33): the stub's two words (module/build_r6d_popups.s), which no stub keeps on the PC
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_SCREENS ((ScreensFn)0x8fc261fcu)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_APPLY_MATRIX ((ApplyMatrixFn)0x8c03dbbeu)
#define STOCK_SHIFT ((ShiftFn)0x8fc24ca4u)
#define STOCK_SCREEN_CTOR ((ScreenCtorFn)0x8fc26398u)
#define STOCK_REQUEST_SCREEN ((RequestScreenFn)0x8c0b2554u)
// R4c: the unit's own popups the base draws over a pack's page: each one's table, the word at its object's +0, and its
// render, the table's +0xc, called as the firmware's render loop calls it.
#define STOCK_NOTICE_TABLE 0x848d8660u                 // R4c: the sequencer's "off" notice, seq_popup.svg, screen 59
#define STOCK_NOTICE_RENDER 0x8fc681ccu
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define OP1F_APART __attribute__((noinline))
// The compiler moves no load or store across this: what the check found is stored before the word that says so.
#define BARRIER() asm volatile("" : : : "memory")
// R6: a pointer held in one P register from here on, so that two stores through it address the same words in the
// listing (the pair's two stores, step M2's rule).
#define PINNED(pointer) asm volatile("" : "+a"(pointer))
// J5i: a request from the display thread is made with interrupts masked, so that no thread that hands keys in is
// inside the request's queue meanwhile (Build H's chain unit); a word cleared after a look, the same.
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
// A time stamp is pinned where it is written: the read carries a memory clobber, which nothing is moved across.
static inline u32 stamp() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
// M3: where the stack of the thread that runs this stands.
static inline u32 stack_here() {
    u32 sp;
    asm volatile("%0 = SP;" : "=d"(sp));
    return sp;
}
#endif

static const u32 CURRENT_MODE = 0x8400f978u;           // the mode: 0 synth, 1 drum, 2 tape, 3 mixer (the mode switch 0x8c0bce20 keeps it)
static const u32 CURRENT_SCREEN = 0x848d2540u;         // the id of the screen the GUI shows (0x8fc23a1c sets it)
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;      // the power popup's byte, 1 while the countdown popup is on the screen
static const u32 DISK_OPEN = 0x84d25bccu;              // M3: not 0 while the disk mode is open (mtp::init 0x8fcba168 writes it, mtp::shutdown 0x8fcba1c8 clears it, through 0x8fcba5f8)
enum { CARD_3_BYTES = OP1F_CARD_LEVEL_3_BYTES, FOLDER_BYTES = 36, FOLDERS_MOST = OP1F_FOLDERS_MOST, DOCS = 8 };   // M3
enum { CALLING_REACH = 8192 };                         // M3: how far below the frame call's stack a call from inside a frame function may stand (a task's stack is 32 KB)
enum { FIRST_SCREEN = 1, LAST_SCREEN = 85, SCREEN_DOCUMENT = 1, NODE_DISPLAY = 0x5c, PAGE_SCREEN = 77, SCREEN_WORDS = 16 };
enum { PERIOD_FRAMES = 256, CYCLES_PER_MS = 400000, SLOW_US = 1000 };
enum { CARD_BYTES = OP1F_CARD_LEVEL_1_BYTES, CARD_2_BYTES = OP1F_CARD_LEVEL_2_BYTES, CARD_NAME = 16, MOST = OP1F_CARDS_MOST };
enum { ART_MAGIC = 0x31545241, ART_NAME = 24, ART_PACK = 16, ART_PATH = 48, ART_HEAD_BYTES = 80, ART_ENTRY_BYTES = 36, ART_MOST = 0xffff, NAMES_SAID = 16 };
enum { ART_LARGEST = 1 << 20 };                         // the picture loader's largest picture (M3's picture loader, LARGEST)
enum { NAME_KEPT = ART_NAME - 1 };                      // a name said is remembered by its first 23 characters
enum { FOUND = 1, FOUND_PICKED = 2 };
enum { HELD_KEYS = 128 };                               // keys whose down a pack took, by the key's number: the firmware's keys are 0 to 64
enum { CLOSED_BY_PACK = 1, CLOSED_BY_MODE = 2, CLOSED_BY_OTHER = 4 };
// R1: level 4.
static const u32 FRAME_BUFFER = 0x0801b804u;           // R1: the word that holds this frame's buffer, read once a frame in the frame call
enum { CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, ASK_BYTES = 12, ASKS_MOST = OP1F_ASKS_MOST, ASK_TABLE_MOST = OP1F_ASK_BYTES_MOST };
enum { CANVAS_ON_BLANK_ONLY = 0 };                      // R1: 1 hands the canvas only while the base's blank page is the one shown
enum { UNDER_MOST = 4 };                                // R1: pages kept under the one shown
enum { HOW_OPEN = 0, HOW_SHOW = 1, HOW_BLANK = 2 };     // R1: how a page was asked: op1f_page_open, op1f_page_show with a document, op1f_page_show with none (the blank page)
enum { KEYS_OF_NOTES_FIRST = 13, KEYS_OF_NOTES_LAST = 53 };   // R1: the keys that may come on another thread than the key thread
static const char BLANK_PICTURE[] = "blank.svg";
// R6d: the base's own page for an engine away: its picture, its two placeholders, and the width their words may take
// before the text unit shortens them with a mark (the longest name and its words take some 300 pixels).
static const char AWAY_PICTURE[] = "away.svg";
static const char AWAY_NAME_PART[] = "away_name";
static const char AWAY_WORDS_PART[] = "away_words";
enum { AWAY_WIDTH = 440 };
// R3: level 5.
enum { STAND_NONE = 0, STAND_TOP = 1, STAND_UNDER = 2 };  // R3: where the stand-in lies: nowhere, the page shown, the lowest kept under it (pages.slot[0])
enum { STOOD_BY_MODE = 1, STOOD_BY_OTHER = 2 };         // R3: why it no longer stands in
enum { STAND_SAID = 16 };                               // R3: each of its two lines said for the first sixteen times
enum { CALL_SLACK = 64 };                               // R3: how far below its caller's stack op1f_cards_call_in may write the place it records
enum { PICTURE_WIDE = 480, NAME_Y = 12, NAME_SCALE = 3 };   // R3: the ask's name on the blank page: top centre, scale 3
static const unsigned NAME_COLOUR = OP1F_COLOUR(0xcd, 0xd0, 0xc2);   // R3: light grey, #CDD0C2
static const u32 LONGEST = 0x0fffffffu;                // a call is counted as two thirds of a second at most: six digits of microseconds
// R4b: level 7.
static const u32 INSTRUMENT = 0x84d21088u;             // R4b: the instrument the unit is on: 0 the synth, 1 the drum (the instrument change writes it; it stays in the tape, the mixer and the disk mode)
enum { CLOSED_BY_INSTRUMENT = 8, CLOSED_BY_TAKEN = 16 };   // R4b: why a page that waited for its mode was closed
// R4c: the unit's own popups over a pack's page.
enum { NOTICE_SCREEN = 59, MEMORY_SCREEN = 65, ABOVE_FIRST = 78, SCREEN_DRAWN = 0x10, POPUP_SAID = 16 };
// R6b: 65, the memory's notice, is no longer drawn by the base a second time: the loop hands its render over as any
// popup's (held_draw). R6b: the screens whose renders the loop hands over while 77 is shown: 60 to 85 (but 77), at most 26; each one's line
// said for its first eight times.
enum { HELD_FIRST = 60, HELD_MOST = 26, HELD_SAID = 8 };
typedef void (*RenderFn)(void* screen, u32 time);                          // R4c: a screen's render, the table's +0xc
// R5: level 8.
enum { STOOD_BY_SCREEN = 3 };                           // R5: why an effect's page no longer stands in: its screen was left, or its ride ended
enum { MASTER_MODE = 3 };                               // R5: the mixer's mode, where the master's effect page is
enum { WORDS_Y = 44, WORDS_SCALE = 2 };                 // R5: the base's words under an effect's name on the blank page
static const u32 DRY = (u32)OP1F_STAND_NO_ROOM | (u32)OP1F_STAND_OVER;   // R5: the base's own bits: the place passes the sound dry

// A pack's directory of pictures, as the picture loader has it (M3's picture loader) and tools/pack_build.py
// writes it: a head of 80 bytes, then an entry of 36 bytes a picture.
struct ArtEntry {
    char name[ART_NAME];
    u32 offset, bytes, crc;
};

struct ArtDir {
    u32 magic, count, total, version;
    char pack[ART_PACK];
    char path[ART_PATH];
    ArtEntry entries[1];
};

struct Measure {
    u32 calls, sum, least, most;                       // of a period: the calls, and what they took in microseconds
};

// The list the loader fills. Zeroed data: the loader clears it with the rest of the pack's and then writes it.
extern "C" Op1fCards op1f_cards OP1F_NAMED("op1f_cards");
Op1fCards op1f_cards;

// The marks of a base that reads cards (level 1) and lends a page with keys and knobs (level 2). A pack with a card
// needs the mark of its card's level; no base of before M1 offers the first, none of before M2 the second.
extern "C" const u32 op1f_level_1 OP1F_NAMED("op1f_level_1");
const u32 op1f_level_1 = 1;
extern "C" const u32 op1f_level_2 OP1F_NAMED("op1f_level_2");
const u32 op1f_level_2 = 2;
// M3: and folders, files and text (level 3). No base before M3 offers it.
extern "C" const u32 op1f_level_3 OP1F_NAMED("op1f_level_3");
const u32 op1f_level_3 = 3;
// R1: and what a page needs (level 4). No base before R1 offers it.
extern "C" const u32 op1f_level_4 OP1F_NAMED("op1f_level_4");
const u32 op1f_level_4 = 4;
// R3: and sequencers and the notes heard (level 5). No base before R3 offers it.
extern "C" const u32 op1f_level_5 OP1F_NAMED("op1f_level_5");
const u32 op1f_level_5 = 5;
// R2: and nothing made for one pack: no name of a pack is asked and no page of a pack's is known here. No base before
// R2 offers it.
extern "C" const u32 op1f_bare_1 OP1F_NAMED("op1f_bare_1");
const u32 op1f_bare_1 = 1;
// R4: and a place in the sound's path, working memory and pictures that take a place (level 6). No base before R4
// offers it.
extern "C" const u32 op1f_level_6 OP1F_NAMED("op1f_level_6");
const u32 op1f_level_6 = 6;
// R4b: and values kept in the unit's files (level 7), served by the values' unit. No base before R4b offers it.
extern "C" const u32 op1f_level_7 OP1F_NAMED("op1f_level_7");
const u32 op1f_level_7 = 7;

// R4b: the values' unit's frame work (base 24's values unit), once a frame on the display thread, before the frame
// functions: it tells the packs what the firmware's threads took from a file, and keeps the base's store of the slots.
extern "C" void op1f_keeps_frame() OP1F_NAMED("op1f_keeps_frame");

// R8 (base 32): the options unit's (module/build_r8_options.cpp): its frame work, once a frame on the display thread
// behind the values' unit's and before any frame function: the file read, the settings' lines filled, `changed` and
// `open` told. And its key down, first in op1f_input_down behind the top page's: not 0 when it took the key (T1, while a
// pack's options show under PACKS in the settings, goes back to the list of packs; the firmware's own T1 there leaves
// the settings at its press). Weak: away, nothing of options is done. The kind of its ask is the header's
// (module/build_r6e_card.h); this unit reads base 29's header, so it names the same number here where that header is
// not read.
extern "C" void __attribute__((weak)) op1f_options_frame() OP1F_NAMED("op1f_options_frame");
extern "C" int __attribute__((weak)) op1f_options_down(unsigned key) OP1F_NAMED("op1f_options_down");
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_ASK_OPTIONS = 10 };
#endif

// R5: the effects unit's (base 28's effects unit): its frame work, once a frame on the display thread behind the
// values' unit's and before the frame functions; and whether an effect's page stands in for a screen now. Weak: away,
// nothing of level 8 is done here.
extern "C" void __attribute__((weak)) op1f_effects_frame() OP1F_NAMED("op1f_effects_frame");
extern "C" int __attribute__((weak)) op1f_effects_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_effects_standing");
// R5: asked of the lead (module/build_r5_base.h does not name it yet): as op1f_effects_standing, but whatever screen the
// current controller asked for: 1 and `out`, `*mode`, `*screen` filled while the place the owner sees in the mode the
// unit is in rides an effect whose type word still holds the type it stands on; 0 otherwise. No line, nothing of a
// pack's called. Weak: away, the entry of an effect's screen lives with its stand-in alone.
extern "C" int __attribute__((weak)) op1f_effects_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_effects_ahead");

// R6: the engines unit's (base 29's engines unit): its frame work, once a frame behind the effects unit's; whether
// an engine's page stands in now; the same whatever screen is asked. Weak: away, nothing of level 9 is done here.
#if defined(OP1F_HOST)
#ifdef OP1F_R6_HOST_ENGINES
extern "C" void (*mock_engines_frame)();
#define op1f_engines_frame (*mock_engines_frame)
extern "C" int (*mock_engines_standing)(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name);
#define op1f_engines_standing (*mock_engines_standing)
extern "C" int (*mock_engines_ahead)(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name);
#define op1f_engines_ahead (*mock_engines_ahead)
#else
static void (*const r6_none_engines_frame)() = 0;
#define op1f_engines_frame (*r6_none_engines_frame)
static int (*const r6_none_engines_standing)(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) = 0;
#define op1f_engines_standing (*r6_none_engines_standing)
static int (*const r6_none_engines_ahead)(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) = 0;
#define op1f_engines_ahead (*r6_none_engines_ahead)
#endif
#else
extern "C" void op1f_engines_frame() OP1F_NAMED("op1f_engines_frame") __attribute__((weak));
extern "C" int op1f_engines_standing(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) OP1F_NAMED("op1f_engines_standing") __attribute__((weak));
extern "C" int op1f_engines_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) OP1F_NAMED("op1f_engines_ahead") __attribute__((weak));
#endif

// R11 (base 35): the kind of ask of a layer of a sound's list of LFOs, its one flag, the size of its table and how many
// the packs may register, as the header of base 35 (module/build_r6e_card.h) names them: this unit reads base 29's
// header, so it names the same numbers here where that header is not read. The entry of the table of screens an LFO's
// page takes is the fourth (OP1F_STAND_LFO in the design). The words fx.asked_engine, fx.placed_engine and
// fx.ahead_engine say from base 35 on which unit answered: the effects unit (0), the engines unit (1), the LFO unit (2).
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_ASK_LFO = 11, OP1F_LFO_DRUM = 1, OP1F_LFOS_MOST = 8 };
#endif
enum { LFO_BYTES = 28, STAND_LFO = 3, LFO_KINDS = 6 };
enum { BY_EFFECTS = 0, BY_ENGINES = 1, BY_LFOS = 2 };

// R11 (base 35): the LFO unit's (module/build_r11_lfos.cpp): whether an LFO layer's page stands in now, while the screen
// the current controller asked for is the LFO page of a place that rides one (op1f_lfos_standing), and the same
// whatever screen is asked (op1f_lfos_ahead), as the effects unit answers for an effect: `out` names the card's ask of
// kind OP1F_ASK_LFO and the place, `*mode` and `*screen` the mode and the screen. Asked where neither the effects unit
// nor the engines unit says a page stands in. Weak: away, entry 3 stands for nothing.
#if defined(OP1F_HOST)
#ifdef OP1F_R11_HOST_LFOS
extern "C" int (*mock_lfos_standing)(Op1fStanding* out, unsigned* mode, unsigned* screen);
#define op1f_lfos_standing (*mock_lfos_standing)
extern "C" int (*mock_lfos_ahead)(Op1fStanding* out, unsigned* mode, unsigned* screen);
#define op1f_lfos_ahead (*mock_lfos_ahead)
#else
static int (*const r11_none_lfos_standing)(Op1fStanding* out, unsigned* mode, unsigned* screen) = 0;
#define op1f_lfos_standing (*r11_none_lfos_standing)
static int (*const r11_none_lfos_ahead)(Op1fStanding* out, unsigned* mode, unsigned* screen) = 0;
#define op1f_lfos_ahead (*r11_none_lfos_ahead)
#endif
#else
extern "C" int op1f_lfos_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_lfos_standing") __attribute__((weak));
extern "C" int op1f_lfos_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_lfos_ahead") __attribute__((weak));
#endif

// R4: the memory unit's and the stages unit's frame work (base 24's memory unit, base 24's stages unit),
// once a frame on the display thread, the memory's first: at its first call each looks through the asks of its kind.
extern "C" void op1f_memory_frame() OP1F_NAMED("op1f_memory_frame");
extern "C" void op1f_stages_frame() OP1F_NAMED("op1f_stages_frame");

// R1: a picture of the base's own directory made into a document (base 18's picture loader): the blank page's.
extern "C" void* op1f_base_picture(const char* name) OP1F_NAMED("op1f_base_picture");

// R1: the paint unit's counts of a slot since the last time they were taken (base 19's paint unit), weak: away,
// no line is said of painting.
extern "C" void __attribute__((weak)) op1f_paint_take(u32 slot, u32* painted, u32* refused) OP1F_NAMED("op1f_paint_take");

// R6b: the paint unit's text for the base's own words on a page of its own (module/build_r6b_paint.cpp), painted from
// this unit's frame work while no pack's frame function runs.
extern "C" int op1f_paint_text_base(void* canvas, int x, int y, const char* text, unsigned colour, int scale) OP1F_NAMED("op1f_paint_text_base");

// R6d: the text unit's text for the base's own words on a page of its own, with an alignment
// (module/build_r6d_text.cpp), set from this unit's frame work on the display thread.
extern "C" int op1f_text_base_set(void* document, const char* part, const char* text, int width, unsigned align) OP1F_NAMED("op1f_text_base_set");

// M3: the folders unit (M3's folders unit), asked once at the first frame, after the check and before any
// card's frame function: it settles the cards' folders and makes them on the disk.
extern "C" void op1f_folders_settle() OP1F_NAMED("op1f_folders_settle");

extern "C" u32 op1f_frame_counter OP1F_NAMED("op1f_frame_counter");         // the measuring unit's count of frames

// S1: the door's frame work (the door's unit of base 18), once a frame. On the PC, where the cards' tests run without
// the door, a stand-in that does nothing.
#if defined(OP1F_HOST) && !defined(OP1F_DOOR)
static void op1f_door_frame(u32) {}
#else
extern "C" void op1f_door_frame(u32 ms) OP1F_NAMED("op1f_door_frame");
#endif

// The one word our call in the firmware's screen request reads (M2's input stub): 0, or the mode + 1 in
// which a pack's page stands under 77 now. Written on the display thread, after the document is in place and before
// the firmware is asked; cleared before the document of before goes back.
extern "C" u32 op1f_page_screen_mode OP1F_NAMED("op1f_page_screen_mode");
u32 op1f_page_screen_mode;

// What a pack asked for (op1f_page_open, op1f_page_close), from any thread but the display thread's own frame work:
// one word, 0 or the slot + 1 in its low byte and the mode in the next, and the document of each slot, written
// before that word.
static volatile u32 page_asked;
static void* volatile page_document[MOST];

// M3: the documents each card made through op1f_picture, eight a card, written on the display thread only (where
// op1f_picture runs) and read from any thread, a word at a time.
static void* volatile made_document[MOST][DOCS];

// R1: how each slot's page was asked (HOW_OPEN, HOW_SHOW, HOW_BLANK), written with its document and the request word,
// all three in one masked instant.
static volatile u32 page_how[MOST];

// R4b: whether each slot's page was asked with OP1F_PAGE_STAYS, written with its document, how it was asked and the
// request word, all four in one masked instant.
static volatile u32 page_stays[MOST];

// R4b: the page that stays with its mode. The display thread's.
static struct {
    u32 waiting;                                        // the slot + 1 of the page shown that waits for its mode now, 0 for none
    u32 lost;                                           // at this frame: the instrument changed under the page that would wait
    u32 instrument;                                     // the instrument's word when the page shown was put under 77
    u32 kept, back;                                     // since the start: pages that began to wait, and came back, for their lines
} stay;

// R4c: the page set aside: a page that waited for its mode when another page took 77 in another mode. The display
// thread's, but for `slot1` and the word op1f_page_aside_mode, which the display thread writes together in one masked
// instant and a card's close, from any thread, looks at and clears in one.
struct Away {
    u32 slot1;                                          // the slot + 1 of the page set aside, 0 for none
    u32 mode;                                           // the mode it waits for
    u32 instrument;                                     // the instrument's word it was put under 77 with
    void* document;                                     // its document, and how it was asked
    u32 how;
};
static Away away;
static u32 away_set;                                    // R4c: pages set aside since the start, for their line

// R4c: the second word our call in the firmware's screen request reads (base 27's screen stub): 0, or the mode + 1
// of the page set aside, so that the request answers 77 in that mode too. Written with `away.slot1`.
extern "C" u32 op1f_page_aside_mode OP1F_NAMED("op1f_page_aside_mode");
u32 op1f_page_aside_mode;

// R4c: the stand-in is the page on the screen (not under another page, not under the base's top page): the base's own
// flag, where base 26 wrote the word the sequencer unit reads, which stays 0 from base 27 on.
static u32 stand_on_screen;

// R4c: the unit's own popups the base drew over a pack's page: whether each was drawn at the last frame, how many
// times it began to be, its line said so many times, and its refusal said; the cost, a period at a time.
// R6b: [1], the memory's notice's, no longer used: the loop's renders are kept and made by `held` below.
static struct {
    u32 on[2], began[2], said[2], refused[2];          // [0] the notice (59), [1] the memory's notice (65)
    u32 calls, sum, least, most;                        // frames in which the base drew one, and what they took in microseconds
} popups;

// R6b: the renders the firmware's render loop handed over while 77 was shown (op1f_popups_loop), kept for the frame
// call to make after the pages are painted (held_draw). The display thread's: the loop and the frame call run on it,
// one after the other, in every frame.
struct HeldRender {
    void* screen;                                       // the screen's object, as the loop handed it
    u32 time;                                           // the time the loop worked out for this frame
    u32 render;                                         // the render the loop read from the screen's table
    u32 number;                                         // the screen's number, 60 to 85
};
static struct {
    const u32* screens;                                 // the table of the screens by their numbers, read at the first frame
    u32 frame;                                          // the frame counter when the loop began to keep the list
    u32 count;                                          // renders kept for this frame
    HeldRender list[HELD_MOST];
    u32 on;                                             // bit n - 60: screen n was made by the base at the last frame
    u32 began[HELD_MOST], said[HELD_MOST];              // since the start: the times each began to be, its lines said
    u32 lost;                                           // renders kept by a loop whose frame call never came (none expected)
} held;

// R9 (base 33): a page that holds its picture (the rule: module/build_r9_hold.h). The two words the render loop's
// places read and write, the stub's (module/build_r6d_popups.s): op1f_hold_screen, the frame call's ask for the next
// loop (77's object, or 0); op1f_fill_left, the loop's frame counter + 1 when the stub left the clearing out. On the PC
// no stub runs, and the words are this unit's own (above, with the PC's stand-ins).
extern "C" u32 op1f_hold_screen OP1F_NAMED("op1f_hold_screen");
extern "C" u32 op1f_fill_left OP1F_NAMED("op1f_fill_left");
enum { HOLD_SAID = 16, HOLD_MOVES = 16, MATRIX_WORDS = 9 };
struct HoldMove {                                       // a part op1f_move set, and the matrix it set
    void* part;
    int matrix[MATRIX_WORDS];
};
static struct {
    Op1fHoldRule rule;                                  // the rule's state, from frame call to frame call
    u32 page;                                           // the slot + 1 of the pack's page this frame names, while the frame functions are called; else 0
    u32 asked;                                          // the slot + 1 that called op1f_page_hold in this frame, 0 none
    u32 redraw;                                         // a redraw asked in this frame
    u32 changes, changes_seen;                          // a count of the base's changes to a document (any thread), and the count at the last frame call
    u32 covered;                                        // this frame: one of the unit's popups or the base's notice drawn over the page (held_frame)
    u32 render_left;                                    // the loop's frame counter + 1 when op1f_popups_loop left 77's render out
    u32 first;                                          // the first canvas the frame call saw: `picture` 0; the other 1
    u32 slot;                                           // the slot + 1 of the page in a hold, for its lines
    u32 began, said;                                    // holds begun since the start, and the lines said of them
    HoldMove moves[HOLD_MOVES];                         // the parts op1f_move set last, a ring
    u32 move_next;
} hold;

// R9: a change the base made to a document (a part shown or hidden, moved, a text set, a picture made): the next three
// frames are drawn whole. From any thread: a word counted on, which the frame call compares.
static inline void hold_changed() {
    hold.changes = hold.changes + 1u;
}

// R12 (base 36): a held page whose parts the firmware draws again (module/build_r6e_card.h, "Base 36"). The frame's ask
// (the rectangles and parts op1f_page_redraw named, in the plan's inputs) and, once the frame call has made it, the plan
// the next loop draws by (module/build_r12_parts.h); static, about 17 KB. Around each drawing op1f_popups_loop sets the
// rasteriser's clip (its struct, the same address in 1.7.0 and 1.7.3: +0x10 the first row, +0x14 the first column,
// +0x18 the row past, +0x1c the column past, column = 479 - x) and puts back its normal state 0, 0, 224, 480.
#if defined(OP1F_HOST)                                 // R12: written as R6's are, so the PC's block above stays the one #ifdef
static u32 parts_raster[8];                             // R12: the PC's stand-in for the rasteriser's struct
#define PARTS_RASTER ((u32)parts_raster)
#else
#define PARTS_RASTER 0x08011514u
#endif
enum { PARTS_SAID = 16, PARTS_SECOND = 1000, PARTS_SAYING = 30000, PARTS_TALL = 224, PARTS_WIDE = 480 };
static Op1fPlan parts_plan;
struct PartsDraw {                                      // R12: a rectangle as the loop draws it, laid out by the frame call
    u32 clip[4];                                        // the clip's +0x10 to +0x1c for it
    u8* const* hide;                                    // the nodes to hide while it is drawn, and how many
    u32 hides;
};
static struct {
    u32 asked;                                          // the slot + 1 that called op1f_page_parts in this frame, 0 none
    u32 refused;                                        // the redraws refused in this frame
    u32 own;                                            // of the base's changes counted in this frame, those the card of the page this frame names made from its frame function
    u32 ready, document;                                // the loop's frame counter + 1 the plan is for (0 none), and the document under 77 it was made over
    u32 screen, time, render, at, before;               // in the loop: 77's object, the time and the render it handed, the rectangle drawn, the cycles at its start
    u32 raster;                                         // in the loop: the rasteriser's struct, read again after each drawing
    u32 draws;                                          // the rectangles of the plan made, as the loop draws them
    PartsDraw draw[OP1F_REDRAWS_MOST];
    u32 page, start, next, second, saying, runs, said;  // the page asking (slot + 1, 0 none), state.ms when its run began, the next line's ms after it and its second; its lines go on; runs begun, runs said
    u32 frames, held, drawn, hides, sum, most, refusals, failures;   // this second: frames, frames held, rectangles drawn, parts hidden, their us summed and largest, refusals, plans failed
    u8 saved[OP1F_PLAN_HIDES_MOST];                     // the display bytes of the parts hidden while a rectangle is drawn
} parts;

// R12: a change of op1f_show, op1f_move, op1f_text or op1f_text_set: base 33's, and the page's own (parts.own) when the
// card whose page this frame names makes it from its frame function. Below, behind op1f_cards_calling.
static inline void hold_changed_by();

// R5: an effect's page standing in, and the entry of the table of screens this unit holds. The display thread's; the
// table itself (op1f_stand_screens) is written by this unit and by the effects unit's picks, each in masked instants.
static struct {
    u32 asked, asked_mode, asked_screen;                // this frame: the effects unit says an effect's page stands in, for this mode and screen
    u32 placed;                                         // the stand-in placed (stand.on) is an effect's
    u32 handing;                                        // it is being handed over to another effect's page in the same frame
    u32 held, held_mode1, held_screen;                  // the entry this unit wrote and holds: its pair + 1 (0: none), the mode + 1, the screen
    u32 ready;                                          // it is held with no stand-in placed: the page of the place the owner sees kept under 77
    void* ready_document;                               // that page's document, as it was under 77
    u32 ready_slot, ready_ask;                          // whose page it is
    u32 dry;                                            // the page asked for now is the base's blank page with its words
    u32 readied;                                        // pages kept ready since the start
    u32 asked_engine;                                   // R6: this frame's answer is the engines unit's (an engine's page)
    u32 placed_engine;                                  // R6: the stand-in placed is an engine's
    u32 ahead_engine;                                   // R6: the last answer ahead was the engines unit's
    u32 ahead_away;                                     // R6b: the last answer ahead was an engine away (no card)
    const char* ahead_name;                             // R6b: its name, as the engines unit keeps it
    u32 ahead_where;                                    // R6b: the synth sound it was for
    u32 ready_away;                                     // R6b: the entry kept ready is an engine away's: the base's own page
    u32 away_on, away_said;                             // R6b: its page was on the screen at the last frame; its line said so many times
} fx;

// R6d: the base's own page for an engine away, of its picture away.svg. The display thread's.
static struct {
    void* document;                                     // made from away.svg, once; 0 when it could not be
    u32 tried;                                          // it was asked for: made, or said once that it could not be
    u32 set;                                            // a name was set into its placeholders
    u32 lettered;                                       // both placeholders took the words last set
    char name[OP1F_KIND_NAME];                          // the name last set
} awaypage;

// R1: pages that stack. The display thread's, but for `closing`, which any thread sets (masked) and the display thread
// takes (masked), and `count`, which any thread reads as a word.
static struct {
    volatile u32 count;                                 // pages kept under the one shown, 0 to UNDER_MOST
    u32 slot[UNDER_MOST];                               // from the lowest: the slot of each
    void* document[UNDER_MOST];                         // its document
    u32 how[UNDER_MOST];                                // how it was asked
    u32 mode;                                           // the mode the shown page and the pages under it stand in
    void* shown_document;                               // the document of the page shown, and how it was asked
    u32 shown_how;
    volatile u32 closing;                               // a bit a slot: its pack closed its page since the last frame
    void* blank;                                        // the base's blank page, once made
    u32 blank_tried;                                    // it was asked for: made, or said once that it could not be
    u32 covered, uncovered, dropped;                    // since the start
    void* volatile canvas;                              // this frame's canvas while the cards' frame functions are called, 0 otherwise
} pages;

// What the threads that hand keys in count, as totals that only grow: the display thread keeps what it saw at the
// last period and says the difference. Single words. The key thread writes most of them; a key of the keyboard can
// come on another thread, and where two meet in one count a count can be lost: they are for the log, nothing
// decides on them. The bytes of `held` are each one key's, written by the thread that hands that key in.
static struct {
    volatile u32 down, up, turns, taken;               // offered downs, ups seen, offered turns, events taken by packs
    volatile u32 took_down[MOST], took_up[MOST], took_turn[MOST];
    volatile u32 last_down, last_down_ms, last_up, last_up_ms;   // the last key down and up the firmware handed in, and when
    volatile u8 held[HELD_KEYS];                       // the slot + 1 of the pack that took a key's down, 0 for the firmware
} input;

static struct {
    u32 checked, said;                                 // the cards were looked at; the check's lines were printed
    u32 seen, in_use, more;                            // slots read; cards in use; cards that found no slot
    u32 used, framed;                                  // a bit a slot: its card is in use; and has a frame function
    u32 started;                                       // the first frame has come
    u32 last_cycles, cycle_acc, ms;                    // the clock
    int countdown;                                     // frames until the period ends
    QuietGate quiet;
    Measure measure[MOST];
    u32 names;                                         // names said to be carried by two packs
    char name_said[NAMES_SAID][ART_NAME];
    // the page: the display thread's own
    u32 shown;                                         // the slot + 1 whose document stands under 77 now, 0 for none
    u32* screen;                                       // the screen object of 77 the page is lent on
    void* kept;                                        // the document that stood under 77 before a page's
    u32 own[SCREEN_WORDS];                             // the base's own screen 77, where no other unit made one
    u32 own_made, opened, closes;
    u32 last_screen;                                   // the screen shown at the last frame, for the line when it changes
    u32 seen_down, seen_up, seen_turns, seen_taken;    // the input's totals at the last period
    u32 seen_took[MOST];
    u32 inputs;                                        // a bit a slot: its card is in use and has an input function (behind M1's fields, which keep their places)
    u32 calling;                                       // M3: the slot + 1 whose frame function is being called now, 0 between them (the display thread's; stored before and after an outside call, which the compiler keeps)
    u32 calling_sp;                                    // M3: where the display thread's stack stood at that call
    u32 settled;                                       // M3: the folders unit was asked to settle the folders
} state;

// R1: keys watched. A bit a slot for each key: the card answered OP1F_SEEN to the key's down and is told its up. Set
// on the thread that hands the down in and taken by the one that hands the up in, each with interrupts masked. The
// counts are for the log, as the input's.
static struct {
    volatile u32 keys[HELD_KEYS];
    volatile u32 down[MOST], up[MOST];                  // downs watched, ups told, since the start
    u32 seen[MOST];                                     // the display thread's: down + up at the last period
} watch;

// R3: the stand-in. The display thread's: what op1f_page_stand_in sets comes from a frame function, on that thread.
static struct {
    u32 on;                                             // STAND_NONE, STAND_TOP or STAND_UNDER
    u32 slot, ask, where, how;                          // whose page it is, for which of its places, and the bits the sequencer unit gave
    u32 mode;                                           // the mode it was placed in
    void* document;                                     // its document, under 77 or kept under the page shown
    u32 named;                                          // its card set no document for the ask: the base paints the ask's name on the blank page
    u32 told, told_slot, told_ask, told_where;          // what its `shown` was last handed (0: not on the screen), and whose
    u32 placed, said_up, said_off;                      // placed since the start; lines said
    void* document_of[MOST][ASKS_MOST];                 // what each card set with op1f_page_stand_in, for each of its asks
    u32 set[MOST];                                      // a bit an ask: its card set a document for it (0 too)
} stand;

// R11 (base 35): the asks of kind OP1F_ASK_LFO of the cards in use, held to their form once, at the check's lines, on
// the display thread: a bit an ask of each card's for one in order, and how many are.
static struct {
    u32 good[MOST];
    u32 count;
} lfos;

// R3: the word the sequencer unit reads for the firmware's "sequencer off" notice: OP1F_ASK_SEQUENCER while a
// sequencer's stand-in is the page on the screen, 0 otherwise. Written each frame on the display thread.
unsigned op1f_cards_stand_in_shown;

// R3: the calls the base is making into packs outside their frame functions now, a place each: the kind, the slot + 1
// whose call runs (0: the place is free) and where the stack stood. Taken, given back and read in masked instants,
// from any thread, in no order. `missed`: calls that found every place taken, since the start.
enum { RECORDS_MOST = 8 };
static struct {
    volatile u32 kind, who, sp;
} records[RECORDS_MOST];
static volatile u32 records_missed;

// M3: a count made with interrupts masked for its instant, so that two threads that count at once lose nothing.
static inline void count(volatile u32* c) {
    u32 mask;
    MASK_OFF(mask);
    *c = *c + 1u;
    MASK_ON(mask);
}

static inline u32 microseconds(u32 cycles) {
    if (cycles > LONGEST) cycles = LONGEST;
    return ((cycles >> 4) * 41u) >> 10;
}

// ---------------------------------------------------------------- the check

// A text that ends inside its room.
static int ended(const char* text, u32 room) {
    for (u32 i = 0; i < room; ++i) {
        if (text[i] == 0) return 1;
    }
    return 0;
}

// A directory's head and entries walked once: 0, or the entry (from 1; 0 for the head's two texts) that is not in
// order, with what it says (the picture's bytes, or the room of the text that has no end).
static OP1F_APART u32 walk(const ArtDir* dir, u32* what) {
    if (!ended(dir->pack, ART_PACK)) { *what = ART_PACK; return 0xffffffffu; }
    if (!ended(dir->path, ART_PATH)) { *what = ART_PATH; return 0xffffffffu; }
    for (u32 i = 0; i < dir->count; ++i) {
        const ArtEntry* e = &dir->entries[i];
        if (!ended(e->name, ART_NAME)) { *what = ART_NAME; return i + 1; }
        if (e->bytes > (u32)ART_LARGEST) { *what = e->bytes; return i + 1; }
    }
    return 0;
}

// The card's input function, for a card that says level 2 or later and is of level 2's size; 0 for any other: a card
// of level 1 that says more bytes has no input function the base would call.
static inline u32 input_of(const Op1fCard* card) {
    return card->bytes >= (u32)CARD_2_BYTES && card->level >= 2u ? (u32)card->input : 0;
}

// M3: the card's list of folders and their count, for a card that says level 3 or later and is of level 3's size;
// none for any other.
static inline u32 folders_of(const Op1fCard* card, u32* count) {
    int level_3 = card->bytes >= (u32)CARD_3_BYTES && card->level >= 3u;
    *count = level_3 ? card->folder_count : 0u;
    return level_3 ? (u32)card->folders : 0u;
}

// R1: is the card one of level 4 (it says level 4 or later and is of level 4's size)? Only such a card has asks the
// base reads and may answer OP1F_SEEN.
static inline int level_4(const Op1fCard* card) {
    return card->bytes >= (u32)CARD_4_BYTES && card->level >= 4u;
}

// R4: is the card one of level 6 (it says level 6 or later and is of level 6's size, which is level 4's)? Only such a
// card is held to the rule of pictures that take a place.
static inline int level_6(const Op1fCard* card) {
    return card->bytes >= (u32)OP1F_CARD_LEVEL_6_BYTES && card->level >= 6u;
}

// R4b: is the card one of level 7 (it says level 7 or later and is of level 7's size, which is level 4's)? Only such a
// card's asks of kind OP1F_ASK_KEEPS are the values' unit's.
static inline int level_7(const Op1fCard* card) {
    return card->bytes >= (u32)OP1F_CARD_LEVEL_7_BYTES && card->level >= 7u;
}

// R5: is the card one of level 8 (it says level 8 or later and is of level 8's size, which is level 4's)? Only such a
// card's asks of kind OP1F_ASK_EFFECT are the effects unit's, and only such an ask's page stands in for an effect's
// screen.
static inline int level_8(const Op1fCard* card) {
    return card->bytes >= (u32)OP1F_CARD_LEVEL_8_BYTES && card->level >= 8u;
}

// R6: is the card one of level 9? Only such a card's asks of kind OP1F_ASK_ENGINE are the engines unit's, and only such
// an ask's page stands in for an engine's page.
static inline int level_9(const Op1fCard* card) {
    return card->bytes >= (u32)OP1F_CARD_LEVEL_9_BYTES && card->level >= 9u;
}

// R1: the card's list of asks and their count, for a card of level 4; none for any other.
static inline u32 asks_of(const Op1fCard* card, u32* count) {
    int four = level_4(card);
    *count = four ? card->ask_count : 0u;
    return four ? (u32)card->asks : 0u;
}

// R1: the first ask of a list in order whose table is not 0 and does not lie in its pack on a word with its bytes, or
// whose bytes are more than an ask may have: its number from 1, or 0 when every ask is in order.
static u32 ask_out_of_place(const Op1fAsk* asks, u32 count, u32 first, u32 end) {
    for (u32 i = 0; i < count; ++i) {
        u32 what = (u32)asks[i].what, bytes = asks[i].bytes;
        if (bytes > (u32)ASK_TABLE_MOST) return i + 1;
        if (what != 0 && (what < first || what >= end || (what & 3u) != 0 || end - what < bytes)) return i + 1;
    }
    return 0;
}

// One card held against the pack it came with: 0, or why it is not used, with two numbers for the line.
static OP1F_APART const char* verdict(const Op1fCardSlot* slot, u32* a, u32* b) {
    u32 first = slot->first, end = slot->end, at = (u32)slot->card;
    u32 size = end - first;
    *a = at - first;
    *b = size;
    if (end < first || size < (u32)CARD_BYTES || at < first || at - first > size - (u32)CARD_BYTES || (at & 3u) != 0)
        return "it does not lie in its pack on a word";                             // (its place from the pack's first byte, the pack's bytes)
    const Op1fCard* card = slot->card;
    u32 room = end - at;
    if (card->mark != (u32)OP1F_CARD_MARK) {
        *a = card->mark;
        *b = 0;
        return "it does not begin with a card's mark";                              // (its first word, 0)
    }
    if (card->bytes < (u32)CARD_BYTES || (card->bytes & 3u) != 0 || card->bytes > room) {
        *a = card->bytes;
        *b = room;
        return "its size is not in order";                                          // (the size it says, the bytes from it to the pack's end)
    }
    u32 n = 0;
    while (n < (u32)CARD_NAME && card->name[n] >= 0x20 && card->name[n] < 0x7f) ++n;
    if (n == 0 || n == (u32)CARD_NAME || card->name[n] != 0) {
        *a = n;
        *b = n < (u32)CARD_NAME ? (u32)(u8)card->name[n] : 0;
        return "its name is not plain text with an end";                            // (the plain characters it begins with, the byte behind them)
    }
    u32 frame = (u32)card->frame, pick = (u32)card->pick, pictures = (u32)card->pictures, input = input_of(card);
    if (frame != 0 && (frame < first || frame >= end || (frame & 1u) != 0)) {
        *a = frame - first;
        *b = size;
        return "its frame function does not lie in its pack on an even address";    // (its place from the pack's first byte, the pack's bytes)
    }
    if (pick != 0 && (pick < first || pick >= end || (pick & 1u) != 0)) {
        *a = pick - first;
        *b = size;
        return "its choice among its pictures does not lie in its pack on an even address";
    }
    if (input != 0 && (input < first || input >= end || (input & 1u) != 0)) {
        *a = input - first;
        *b = size;
        return "its input function does not lie in its pack on an even address";
    }
    if (pictures != 0) {
        if (pictures < first || pictures >= end || end - pictures < (u32)ART_HEAD_BYTES || (pictures & 3u) != 0) {
            *a = pictures - first;
            *b = size;
            return "its directory of pictures does not lie in its pack on a word";
        }
        const ArtDir* dir = (const ArtDir*)card->pictures;
        if (dir->magic != (u32)ART_MAGIC) {
            *a = dir->magic;
            *b = 0;
            return "its directory of pictures does not begin with a directory's mark";  // (its first word, 0)
        }
        u32 behind = end - pictures - (u32)ART_HEAD_BYTES;
        if (dir->count > (u32)ART_MOST || dir->count * (u32)ART_ENTRY_BYTES > behind) {
            *a = dir->count;
            *b = behind;
            return "its directory of pictures does not end in its pack";            // (the pictures it counts, the bytes behind its head)
        }
        u32 what = 0, which = walk(dir, &what);
        if (which != 0) {
            *a = which;
            *b = what;
            return "its directory of pictures has a text with no end or a picture larger than a megabyte";   // (the picture from 1, or 4294967295 for the head; its bytes, or the text's room)
        }
    }
    u32 folder_count = 0, folders = folders_of(card, &folder_count);                  // M3: the list of folders, behind M2's reasons
    if (folder_count > (u32)FOLDERS_MOST || (folders == 0) != (folder_count == 0)) {
        *a = folder_count;
        *b = (u32)FOLDERS_MOST;
        return "its list of folders is not in order";                               // (the folders it counts, the most a card may list)
    }
    if (folders != 0 && (folders < first || folders >= end || (folders & 3u) != 0 || end - folders < folder_count * (u32)FOLDER_BYTES)) {
        *a = folders - first;
        *b = size;
        return "its list of folders does not lie in its pack on a word";            // (its place from the pack's first byte, the pack's bytes)
    }
    u32 ask_count = 0, asks = asks_of(card, &ask_count);                             // R1: the list of asks, behind M3's reasons
    if (ask_count > (u32)ASKS_MOST || (asks == 0) != (ask_count == 0)) {
        *a = ask_count;
        *b = (u32)ASKS_MOST;
        return "its list of asks is not in order";                                  // (the asks it counts, the most a card may list)
    }
    if (asks != 0 && (asks < first || asks >= end || (asks & 3u) != 0 || end - asks < ask_count * (u32)ASK_BYTES ||
                      ask_out_of_place((const Op1fAsk*)asks, ask_count, first, end) != 0)) {
        *a = asks - first;
        *b = size;
        return "its list of asks does not lie in its pack on a word";               // (its place from the pack's first byte, the pack's bytes)
    }
    return 0;
}

// R3: the check's verdict on one slot of the list: 0 when the base uses its card, 1 when it does not (no card there,
// or a card not in order). It reads and writes nothing else and says no line: from any thread.
extern "C" int op1f_cards_verdict(unsigned slot) {
    u32 count = op1f_cards.count, a = 0, b = 0;
    u32 seen = count > (u32)MOST ? (u32)MOST : count;
    if (slot >= seen) return 1;
    return verdict(&op1f_cards.slots[slot], &a, &b) != 0 ? 1 : 0;
}

static u32 pictures_of(const Op1fCard* card) {
    return card->pictures ? ((const ArtDir*)card->pictures)->count : 0;
}

// R1: level 4 defines no kind of ask: a card's asks, in order, are left aside with one line. Out of line, so that the
// check's own order of stores and its first line stay as the compiler made them for base 18.
// R3: level 5 knows two kinds, a sequencer and the notes heard: the units that serve them look at those asks and say
// their own lines. The line here counts the asks of the kinds this base does not know, and names the first of them.
// R4: level 6 knows three kinds more, for a card of level 6: a stage, memory, pictures that take a place.
// R4b: level 7 knows one kind more, for a card of level 7: values kept in the unit's files.
// R11 (base 35): the names of the unit's own LFO kinds, which no layer takes.
static const char LFO_KIND_NAMES[LFO_KINDS][9] = {"tremolo", "value", "random", "element", "midi", "velocity"};

// R11 (base 35): two names of a kind's head the same, up to their end.
static int same_kind_name(const char* a, const char* b) {
    for (u32 i = 0; i < (u32)OP1F_KIND_NAME; ++i) {
        if (a[i] != b[i]) return 0;
        if (a[i] == 0) return 1;
    }
    return 1;
}

// R11 (base 35): is `name` one of the unit's own LFO kinds? Out of line, as are lfo_named and lfo_taken, so that each
// walk holds what it reads in plain registers (no index register in any of them).
static OP1F_APART int lfo_stock(const char* name) {
    for (u32 k = 0; k < (u32)LFO_KINDS; ++k)
        if (same_kind_name(name, LFO_KIND_NAMES[k])) return 1;
    return 0;
}

// R11 (base 35): is `name` the name of one of the asks of a card in use whose bits are set in `good` (its LFO layers
// kept)? Its list of asks the check held in order.
static OP1F_APART int lfo_named(const Op1fCard* card, u32 good, const char* name) {
    u32 count = 0;
    const Op1fAsk* asks = (const Op1fAsk*)asks_of(card, &count);
    for (u32 k = 0; k < count; ++k)
        if (((good >> k) & 1u) != 0 && same_kind_name(((const Op1fKindHead*)asks[k].what)->name, name)) return 1;
    return 0;
}

// R11 (base 35): has an LFO layer kept at `slot` or before it `name` already? Only the cards the base uses are read, and
// of them only those with a layer kept: a slot with no card there, or a card not in order, is passed by unread (its
// address may be anything, its count of asks too).
static OP1F_APART int lfo_taken(u32 slot, const char* name) {
    for (u32 s = 0; s <= slot; ++s) {
        u32 good = lfos.good[s];
        if (((state.used >> s) & 1u) != 0 && good != 0 && lfo_named(op1f_cards.slots[s].card, good, name)) return 1;
    }
    return 0;
}

// R11 (base 35): why an ask of kind OP1F_ASK_LFO of the card in use at `slot` (of level 8 or later) is left aside, or 0.
// Its table (the check held it to lie in its pack on a word with its bytes) at least an Op1fLfo, 28 bytes, its head
// saying the ask's bytes; its name 1 to 15 characters of a-z, 0-9, ' ', '-' and '_', none of the unit's LFO kinds nor an
// LFO layer's taken before it; no flag but OP1F_LFO_DRUM; its `shown` 0 or a function of its pack on an even address; and
// no more than OP1F_LFOS_MOST in all.
static OP1F_APART const char* lfo_verdict(u32 slot, const Op1fAsk* ask) {
    const Op1fCardSlot* in = &op1f_cards.slots[slot];
    const Op1fKindHead* head = (const Op1fKindHead*)ask->what;
    if (head == 0 || ask->bytes < (u32)LFO_BYTES || head->bytes != ask->bytes) return "its table is not in order";
    u32 n = 0;
    for (; n < (u32)OP1F_KIND_NAME && head->name[n] != 0; ++n) {
        char c = head->name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_')) return "its name is not in order";
    }
    if (n == 0 || n == (u32)OP1F_KIND_NAME) return "its name is not in order";
    if (lfo_stock(head->name)) return "its name is one of the unit's own LFOs";
    if (lfo_taken(slot, head->name)) return "another LFO layer has its name";
    if ((head->flags & ~(u32)OP1F_LFO_DRUM) != 0) return "it has a flag this base does not know";
    u32 shown = (u32)head->shown;
    if (shown != 0 && (shown < in->first || shown >= in->end || (shown & 1u) != 0)) return "its shown does not lie in its pack on an even address";
    if (lfos.count >= (u32)OP1F_LFOS_MOST) return "the packs ask for more LFO layers than the base takes";
    return 0;
}

// R11 (base 35): an ask of kind OP1F_ASK_LFO held to its form, from the line of the kinds this base knows: kept, a bit
// of its card's, or left aside with line 49 (no page of it ever stands in).
static int slot_of(const Op1fCard* card);
static OP1F_APART void lfo_look(const Op1fCard* card, u32 ask) {
    int slot = slot_of(card);
    if (slot < 0) return;
    const char* why = lfo_verdict((u32)slot, &card->asks[ask]);
    if (why == 0) {
        lfos.good[slot] |= 1u << ask;
        ++lfos.count;
        return;
    }
    STOCK_LOG(2, LOG_FILE, 49, "cards: '%s' asks for an LFO layer (its ask %u) that is left aside: %s", card->name, ask, why);
}

// R5: level 8 knows one kind more, for a card of level 8: an effect, whose lines the effects unit says.
// R11 (base 35): and a layer of a sound's list of LFOs, for a card of level 8, held to its form here (lfo_look).
static OP1F_APART void said_asks(const Op1fCard* card) {
    u32 asked = 0, unknown = 0, first = 0;
    const Op1fAsk* asks = (const Op1fAsk*)asks_of(card, &asked);
    int six = level_6(card), seven = level_7(card);
    for (u32 i = 0; i < asked; ++i) {
        u32 kind = asks[i].kind;
        if (kind == (u32)OP1F_ASK_SEQUENCER || kind == (u32)OP1F_ASK_NOTES) continue;
        if (six && (kind == (u32)OP1F_ASK_STAGE || kind == (u32)OP1F_ASK_MEMORY || kind == (u32)OP1F_ASK_PLACES)) continue;   // R4
        if (seven && kind == (u32)OP1F_ASK_KEEPS) continue;                         // R4b
        if (six && kind == (u32)OP1F_ASK_OPTIONS) continue;                         // R8 (base 32): the options unit's
        if (kind == (u32)OP1F_ASK_EFFECT && level_8(card)) continue;               // R5
        if (kind == (u32)OP1F_ASK_ENGINE && level_9(card)) continue;               // R6
        if (kind == (u32)OP1F_ASK_LFO && level_8(card)) {                          // R11 (base 35): held to its form here
            lfo_look(card, i);
            continue;
        }
        if (unknown == 0) first = kind;
        ++unknown;
    }
    if (unknown != 0)
        STOCK_LOG(3, LOG_FILE, 15, "cards: '%s' asks %u thing(s) of kind(s) this base does not know (the first of kind %u): left aside",
                  card->name, unknown, first);
}

// Every card of the list looked at, once. What is found is worked out first and written then, the same whoever
// looks; then, past a barrier, the word that says the check was made; the lines follow, each card's in the list's
// order.
static OP1F_APART void check() {
    u32 count = op1f_cards.count;
    u32 seen = count > (u32)MOST ? (u32)MOST : count;                               // the list holds thirty-two: a count above that names slots that are not there
    u32 more = op1f_cards.more + (count - seen);
    u32 used = 0, framed = 0, inputs = 0, in_use = 0, a = 0, b = 0;
    for (u32 i = 0; i < seen; ++i) {
        if (verdict(&op1f_cards.slots[i], &a, &b) != 0) continue;
        used |= 1u << i;
        ++in_use;
        if (op1f_cards.slots[i].card->frame != 0) framed |= 1u << i;
        if (input_of(op1f_cards.slots[i].card) != 0) inputs |= 1u << i;
    }
    state.seen = seen;
    state.more = more;
    state.in_use = in_use;
    state.used = used;
    state.framed = framed;
    state.inputs = inputs;
    BARRIER();                                                                      // all of the above is stored before the word that says it was made
    state.checked = 1;
    BARRIER();                                                                      // and that word before the first line
    if (state.said) return;
    state.said = 1;
    STOCK_LOG(3, LOG_FILE, 1, "cards: %u card(s) from the loader, %u in use, %u not used, %u more than the list holds", seen, in_use, seen - in_use, more);
    for (u32 i = 0; i < seen; ++i) {
        const Op1fCardSlot* slot = &op1f_cards.slots[i];
        const char* why = verdict(slot, &a, &b);
        if (why != 0) {
            STOCK_LOG(1, LOG_FILE, 2, "cards: the card of the pack at 0x%08x is not used: %s (%u, %u)", slot->first, why, a, b);
            continue;
        }
        const Op1fCard* card = slot->card;
        STOCK_LOG(3, LOG_FILE, 3, "cards: '%s' version %u, level %u: a frame call %s, %u picture(s), a choice among them %s, an input function %s",
                  card->name, card->version, card->level, card->frame ? "yes" : "no", pictures_of(card), card->pick ? "yes" : "no", input_of(card) ? "yes" : "no");
        said_asks(card);                                                            // R1
    }
}

static inline void ready() {
    if (!state.checked) check();
}

// Has the check been made? For the threads that hand keys in, which never make it (it writes lines): past this,
// what it found is read whole.
static inline int checked() {
    u32 done = state.checked;
    BARRIER();
    return done != 0;
}

// ---------------------------------------------------------------- the page

// The slot of a card the base has in use, or -1.
static int slot_of(const Op1fCard* card) {
    if (card == 0 || !checked()) return -1;
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.used >> i) & 1u) != 0 && op1f_cards.slots[i].card == card) return (int)i;
    }
    return -1;
}

// M3: is this one of the documents the card of this slot made?
static int made_by(u32 slot, const void* document) {
    for (u32 d = 0; d < (u32)DOCS; ++d) {
        if (made_document[slot][d] == document) return 1;
    }
    return 0;
}

// M3: from op1f_picture (M3's picture loader), on the display thread: a document a card made. The first eight of
// each card are kept; the ninth and later are not, and neither op1f_page_open nor op1f_text takes them.
extern "C" void op1f_cards_made(const Op1fCard* card, void* document) OP1F_NAMED("op1f_cards_made");
extern "C" void op1f_cards_made(const Op1fCard* card, void* document) {
    int slot = slot_of(card);
    hold_changed();                                         // R9 (base 33): a picture made
    if (slot < 0 || document == 0 || made_by((u32)slot, document)) return;
    for (u32 d = 0; d < (u32)DOCS; ++d) {
        if (made_document[slot][d] == 0) {
            made_document[slot][d] = document;
            return;
        }
    }
}

// R9 (base 33): from the text unit (module/build_r6d_text.cpp), where it lays a text out into a document or makes a
// part a text: the document under 77 changed, so the next three frames are drawn whole. A document that is not under
// 77 changes nothing on the screen and is passed by; 0 counts for any.
extern "C" void op1f_cards_changed(const void* document) OP1F_NAMED("op1f_cards_changed");
extern "C" void op1f_cards_changed(const void* document) {
    const u32* screens = held.screens;
    const u32* s77 = screens != 0 ? (const u32*)screens[PAGE_SCREEN] : 0;
    if (document == 0 || s77 == 0 || (const void*)s77[SCREEN_DOCUMENT] == document) hold_changed_by();   // R12: or the page's own
}

// M3: the slot of the card that made this document, or -1: for the text unit.
extern "C" int op1f_cards_document(const void* document) OP1F_NAMED("op1f_cards_document");
extern "C" int op1f_cards_document(const void* document) {
    if (document == 0 || !checked()) return -1;
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.used >> i) & 1u) != 0 && made_by(i, document)) return (int)i;
    }
    return -1;
}

// M3: for the folders and text units. The cards in use, a bit a slot (0 before the check); the slot of a card in
// use, or -1; the slot + 1 whose frame function the base is calling now, 0 between the calls.
extern "C" u32 op1f_cards_used() OP1F_NAMED("op1f_cards_used");
extern "C" u32 op1f_cards_used() {
    return checked() ? state.used : 0u;
}

extern "C" int op1f_cards_slot(const Op1fCard* card) OP1F_NAMED("op1f_cards_slot");
extern "C" int op1f_cards_slot(const Op1fCard* card) {
    return slot_of(card);
}

// The slot + 1 is said only to a caller on the display thread's own stack, within CALLING_REACH below where it stood
// at the frame call: a call from an input function that runs, on the thread that handles keys or on the audio
// thread, while the display thread is inside a frame function stands on another stack and is told 0 (the review of
// M3). Another task's stack lies in another allocation of 32 KB [inferred: the tasks' stacks were not read for
// their places].
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
extern "C" u32 op1f_cards_calling() {
    u32 calling = state.calling, at = state.calling_sp;
    if (calling == 0) return 0;
    u32 here = stack_here();
    return here <= at && at - here < (u32)CALLING_REACH ? calling : 0u;
}

// R12 (base 36): above. hold.page is the page this frame names only while the frame functions are called.
static inline void hold_changed_by() {
    hold_changed();
    u32 page = hold.page;
    if (page != 0 && op1f_cards_calling() == page) parts.own = parts.own + 1u;
}

// R3: the records of the calls into packs. Around each call: a free place of the table taken, then given back, each in
// one masked instant; what is kept is the place + 1 (0: none taken) and the stack pointer written. Calls on other
// threads may begin and end in any order between the two. Out of line: the place recorded is the one inside
// op1f_cards_call_in.
extern "C" OP1F_APART void op1f_cards_call_in(unsigned kind, unsigned slot, Op1fCallKept* kept) {
    u32 sp = stack_here(), place = 0, mask;
    kept->who = 0;
    kept->sp = sp;
    if (kind >= (u32)OP1F_CALL_KINDS) return;
    MASK_OFF(mask);
    for (u32 i = 0; i < (u32)RECORDS_MOST; ++i) {
        if (records[i].who != 0) continue;
        records[i].kind = kind;
        records[i].sp = sp;
        records[i].who = slot + 1u;
        place = i + 1u;
        break;
    }
    if (place == 0) records_missed = records_missed + 1u;                          // every place taken: no record, so the call's notes are refused
    MASK_ON(mask);
    kept->who = place;
}

extern "C" OP1F_APART void op1f_cards_call_out(unsigned kind, const Op1fCallKept* kept) {
    u32 place = kept->who, mask;
    (void)kind;
    if (place == 0 || place > (u32)RECORDS_MOST) return;
    MASK_OFF(mask);
    records[place - 1u].who = 0;                                                    // exactly the place this call took
    MASK_ON(mask);
}

// R3: a record around the caller: its place at or above the caller's stack, CALL_SLACK allowed for the few words
// op1f_cards_call_in stands below its own caller, and within the frame call's reach. A record of a call that stood
// deeper on this stack, or on another thread's stack, is not around it.
static inline int around(u32 at, u32 here) {
    return at + (u32)CALL_SLACK - here < (u32)CALLING_REACH;                          // a caller above the place wraps to far more than the reach
}

// R3: 1 when the nearest of the base's calls into packs around the caller is a tick, a key, a run, an octave or an
// input call of this card; 0 otherwise. The frame call's own record counts among them, as a frame function's. The
// records are read each in one masked instant, so that none is read half written.
// The nearest record around a caller whose stack stands at `here`: its kind + 1 in the high half (OP1F_CALL_KINDS + 1
// for the frame call's own), its slot + 1 in the low half; 0 for none.
static OP1F_APART u32 nearest_call(u32 here) {
    u32 nearest = 0xffffffffu, best = 0;
    for (u32 i = 0; i < (u32)RECORDS_MOST; ++i) {
        u32 mask, who, at, kind;
        MASK_OFF(mask);                                                             // a record's words read in one masked instant
        who = records[i].who;
        at = records[i].sp;
        kind = records[i].kind;
        MASK_ON(mask);
        if (who == 0 || !around(at, here) || at >= nearest) continue;
        nearest = at;
        best = (kind + 1u) << 16 | who;
    }
    u32 calling = state.calling, at = state.calling_sp;
    if (calling != 0 && around(at, here) && at < nearest) best = ((u32)OP1F_CALL_KINDS + 1u) << 16 | calling;
    return best;
}

extern "C" int op1f_cards_inside(const Op1fCard* card) {
    int slot = slot_of(card);
    if (slot < 0) return 0;
    u32 best = nearest_call(stack_here());
    return best != 0 && (best >> 16) <= (u32)OP1F_CALL_INPUT + 1u && (best & 0xffffu) == (u32)slot + 1u ? 1 : 0;
}

// R1: a page asked for, by op1f_page_open and op1f_page_show alike, from any thread: the document, how it was asked
// and the word that asks, written in one masked instant, so that the display thread, which writes them back when a
// page under another shows again, never finds half of a request.
// R4b: and whether it stays with its mode, in the same instant.
static int ask(u32 slot, void* document, u32 how, u32 stays) {
    u32 mode = RD32(CURRENT_MODE);
    if (mode > 0xffu) return 0;                                                     // the word holds a mode of a byte: the firmware's are below 32
    u32 mask;
    MASK_OFF(mask);
    page_document[slot] = document;
    page_how[slot] = how;
    page_stays[slot] = stays;                                                       // R4b
    page_asked = (slot + 1u) | (mode << 8);
    MASK_ON(mask);
    return 1;
}

extern "C" int op1f_page_open(const Op1fCard* card, void* document) {
    int slot = slot_of(card);
    if (slot < 0 || document == 0) return 0;
    if (!made_by((u32)slot, document)) return 0;                                   // M3: a document this card made, and no other
    return ask((u32)slot, document, HOW_OPEN, 0);                                   // R1: a page another covers is closed, as since level 2 (R4b: and it does not stay)
}

// R1: the pack's page shown over the page that stands in the mode, kept under it. Document 0 is the base's blank page.
// R4b: one flag, OP1F_PAGE_STAYS: the page stays with its mode.
extern "C" int op1f_page_show(const Op1fCard* card, void* document, unsigned flags) {
    if ((flags & ~(unsigned)OP1F_PAGE_STAYS) != 0) return 0;                        // R4b: a flag this base does not know
    u32 stays = flags != 0 ? 1u : 0u;
    int slot = slot_of(card);
    if (slot < 0) return 0;
    if (document == 0) {
        if (pages.blank_tried && pages.blank == 0) return 0;                        // the blank page could not be made: said once, at the frame that tried
        return ask((u32)slot, 0, HOW_BLANK, stays);
    }
    if (made_by((u32)slot, document) == 0) return 0;                                // a document this card made, and no other
    return ask((u32)slot, document, HOW_SHOW, stays);
}

// The word cleared only when it is still the one looked at: a page opened meanwhile on another thread stays.
static void close_asked(u32 seen) {
    u32 mask;
    MASK_OFF(mask);
    if (page_asked == seen) page_asked = 0;
    MASK_ON(mask);
}

// The screen request, from the display thread: with interrupts masked (J5i).
static OP1F_APART void request_screen() {
    u32 mask;
    MASK_OFF(mask);
    STOCK_REQUEST_SCREEN();
    MASK_ON(mask);
}

// The screen object of 77: the one registered, else the base's own, made once, at the first frame that needs it.
static OP1F_APART u32* page_screen(u32 frame) {
    u32* screens = STOCK_SCREENS();
    if (screens != 0) held.screens = screens;               // R6b: the table, for the loop's renders: read here before 77 is ever shown
    u32* screen = screens ? (u32*)screens[PAGE_SCREEN] : 0;
    if (screen != 0 || state.own_made) return screen;
    (void)frame;
    for (u32 i = 0; i < (u32)SCREEN_WORDS; ++i) state.own[i] = 0;
    STOCK_SCREEN_CTOR(state.own, PAGE_SCREEN, 0);
    state.own_made = 1;
    STOCK_LOG(3, LOG_FILE, 9, "page: screen %u made by the base for the packs' pages: no other unit made it", (u32)PAGE_SCREEN);
    return state.own;
}

static const char* closed_why(u32 why) {
    if (why == CLOSED_BY_MODE) return "the mode changed";
    if (why == CLOSED_BY_OTHER) return "another pack's page was opened";
    if (why == CLOSED_BY_INSTRUMENT) return "it waited for its mode and the instrument changed";   // R4b
    if (why == CLOSED_BY_TAKEN) return "it waited for its mode and another page took the screen";   // R4b
    return "the pack closed it";
}

// ---------------------------------------------------------------- S1: the top page of the base's own

enum { TOP_HELD = 0xff };                               // in input.held: the top page took this key's down

static struct {
    void* volatile document;                            // asked for, from any thread
    int (* volatile keys)(const Op1fInput* e);
    volatile u32 wanted;                                // 1 from op1f_top_open to op1f_top_close
    u32 shown;                                          // the display thread's: the document stands under 77
    u32* screen;
    void* kept;                                         // the document that stood under 77 before
    u32 kept_mode;                                      // op1f_page_screen_mode before
    u32 opened, closed;
} top;

extern "C" void op1f_top_open(void* document, int (*keys)(const Op1fInput* e)) OP1F_NAMED("op1f_top_open");
extern "C" void op1f_top_open(void* document, int (*keys)(const Op1fInput* e)) {
    if (document == 0) return;
    top.document = document;
    top.keys = keys;
    BARRIER();                                          // the document and the function before the word that asks
    top.wanted = 1;
}

extern "C" void op1f_top_close() OP1F_NAMED("op1f_top_close");
extern "C" void op1f_top_close() {
    top.wanted = 0;
}

// Once a frame on the display thread, before page_frame and before the cards' frame functions.
static OP1F_APART void top_frame(u32 frame, u32 mode) {
    if (top.wanted) {
        if (!top.shown) {
            u32* screen = page_screen(frame);
            if (screen == 0) return;                                                // the next frame
            top.screen = screen;
            top.kept = (void*)screen[SCREEN_DOCUMENT];
            top.kept_mode = op1f_page_screen_mode;
            screen[SCREEN_DOCUMENT] = (u32)top.document;
            top.shown = 1;
            ++top.opened;
            BARRIER();                                                              // the document is in place before our call answers 77
            op1f_page_screen_mode = mode + 1u;
            request_screen();
            STOCK_LOG(3, LOG_FILE, 13, "page: the base's top page is up in mode %u, its document at 0x%08x under screen %u, %s; %u opened since the start",
                      mode, (u32)top.document, (u32)PAGE_SCREEN, state.own_made ? "the base's own" : "another unit's", top.opened);
            return;
        }
        if ((void*)top.screen[SCREEN_DOCUMENT] != top.document) top.screen[SCREEN_DOCUMENT] = (u32)top.document;
        if (op1f_page_screen_mode != mode + 1u) {                                   // the mode changed under it: the page stays
            op1f_page_screen_mode = mode + 1u;
            request_screen();
        }
        return;
    }
    if (!top.shown) return;
    op1f_page_screen_mode = top.kept_mode;                                          // our call answers what it answered before
    BARRIER();
    if (top.kept != 0) top.screen[SCREEN_DOCUMENT] = (u32)top.kept;               // with none kept (the base's own 77) the document stays: 77 is never left without one (M3's rule)
    top.shown = 0;
    ++top.closed;
    request_screen();
    STOCK_LOG(3, LOG_FILE, 14, "page: the base's top page is taken off; the document of before is back under screen %u", (u32)PAGE_SCREEN);
}

static void event_of(Op1fInput* e, u32 what, u32 key, int turn, const u32* record);

// From the three input functions, right after their counts: 1 when the top page takes the event.
static OP1F_APART int top_input(u32 what, u32 key, int turn, const u32* record) {
    if (what == OP1F_UP) {
        if (key >= (u32)HELD_KEYS || input.held[key] != (u8)TOP_HELD) return 0;    // the up goes where its down went
        input.held[key] = (u8)0;
    } else {
        if (!top.wanted && !top.shown) return 0;
        if ((what == OP1F_DOWN || what == OP1F_DOWN_SHIFT) && key < (u32)HELD_KEYS) input.held[key] = (u8)TOP_HELD;
    }
    int (*keys)(const Op1fInput*) = top.keys;
    if (keys != 0) {
        Op1fInput e;
        event_of(&e, what, key, turn, record);
        keys(&e);
    }
    return 1;
}

// ---------------------------------------------------------------- R1: pages that stack

// The pack's page closed, from any thread: the request withdrawn when it is the card's, and the card's page taken out
// from under another at the next frame. R1: when the card's page is the one shown, nothing is kept under it and no
// other request waits, the word our call in the screen request reads is cleared in the same masked instant, so that
// the request the firmware makes after the key answers the mode's own screen; the document goes back under 77 at the
// next frame, as before. While the base's top page is asked for or up, the word is the top page's and stays.
extern "C" void op1f_page_close(const Op1fCard* card) {
    int slot = slot_of(card);
    if (slot < 0) return;
    u32 mask;
    MASK_OFF(mask);                                                                 // looked at and cleared with nothing between
    if ((page_asked & 0xffu) == (u32)(slot + 1)) page_asked = 0;
    pages.closing |= 1u << slot;                                                    // R1: out from under another page too
    if (away.slot1 == (u32)slot + 1u) op1f_page_aside_mode = 0;                     // R4c: its page set aside, the word cleared at once
    if (stand.on != (u32)STAND_TOP) {                                               // R3: a stand-in shown is the base's: the card's close leaves it
        if (page_asked == 0 && state.shown == (u32)slot + 1u && pages.count == 0 && !top.wanted && !top.shown)
            op1f_page_screen_mode = 0;                                                  // R1: the close seen at once
    }
    MASK_ON(mask);
}

// A page closed, said by its pack's name.
static OP1F_APART void said_closed(u32 slot, u32 why) {
    STOCK_LOG(3, LOG_FILE, 8, "page: '%s' closed: %s; %u opened since the start", op1f_cards.slots[slot].card->name, closed_why(why), state.opened);
}

// The page kept at `at` under the shown one taken out; those above it move down. Out of line, as the loops of the
// unit's other small helpers: put into page_frame, the compiler kept its count in an index register.
static OP1F_APART void take_out(u32 at) {
    u32 n = pages.count;
    for (u32 i = at; i + 1u < n; ++i) {
        pages.slot[i] = pages.slot[i + 1u];
        pages.document[i] = pages.document[i + 1u];
        pages.how[i] = pages.how[i + 1u];
    }
    pages.count = n - 1u;
}

// ---------------------------------------------------------------- R5: the entries of the table of screens

// The entry of a mode in the table our call in the screen request reads: the instruments' for the synth and the
// drum, the master's for the mixer.
static inline u32 pair_of(u32 mode) {
    return mode == (u32)MASTER_MODE ? (u32)OP1F_STAND_MASTER : (u32)OP1F_STAND_EFFECT;
}

// R6: the entry of a page: an engine's its own (OP1F_STAND_ENGINE), an effect's its mode's.
// R11 (base 35): an LFO layer's its own too (entry 3).
static inline u32 pair_for(u32 mode, u32 engine) {
    if (engine == (u32)BY_LFOS) return (u32)STAND_LFO;                              // R11 (base 35)
    return engine ? (u32)OP1F_STAND_ENGINE : pair_of(mode);
}

// The entry this unit holds taken back, the mode first, in one masked instant, before the document of before goes back,
// and only where it still holds what this unit wrote: an entry the effects unit wrote at a pick meanwhile is the
// pick's, and stays.
static OP1F_APART void entry_drop() {
    u32 held = fx.held, mask;
    fx.held = 0;
    fx.ready = 0;
    if (held == 0) return;
    volatile unsigned* at = &op1f_stand_screens[2u * (held - 1u)];                 // the pair: {mode + 1, screen}
    MASK_OFF(mask);
    if (at[0] == fx.held_mode1 && at[1] == fx.held_screen) {
        at[0] = 0;                                                                  // the mode first
        at[1] = 0;
    }
    MASK_ON(mask);
}

// The entry of `mode` written for `screen`, from the display thread, once the page's document is under 77: the screen,
// then the mode + 1, in one masked instant; from then on our call answers 77 for that screen of that mode. An entry
// this unit held for the other pair is taken back first: one document stands under 77.
static OP1F_APART void entry_write(u32 mode, u32 screen, u32 engine) {
    u32 pair = pair_for(mode, engine), mask;                                        // R6: an engine's page has its own entry
    if (fx.held != 0 && fx.held != pair + 1u) entry_drop();
    volatile unsigned* at = &op1f_stand_screens[2u * pair];                          // the pair: {mode + 1, screen}
    PINNED(at);                                                                     // R6: one register holds the pair for both stores
    MASK_OFF(mask);
    at[1] = screen;                                                                 // the screen first
    at[0] = mode + 1u;                                                              // then the mode + 1
    MASK_ON(mask);
    fx.held = pair + 1u;
    fx.held_mode1 = mode + 1u;
    fx.held_screen = screen;
}

// Why a page no longer stands in, as line 24 says it.
static const char* gone_why(u32 why, u32 effect) {
    if (why == STOOD_BY_MODE) return "the mode changed";
    if (effect == 0) return "another sequencer was chosen";
    return why == STOOD_BY_OTHER ? "another effect's page stands in its place" : "its screen was left, or its ride ended";
}

// R3: the stand-in no longer stands in, said by its pack's name for the first sixteen times; where it lay is the
// caller's to clear.
// R5: an effect's: its entry taken back, the mode first, unless it is kept ready or handed to the page that takes its
// place in the same frame.
static OP1F_APART void stand_said_gone(u32 why) {
    stand.on = STAND_NONE;
    ++state.closes;
    u32 effect = fx.placed;                                                         // R5
    if (effect != 0) {
        if (!fx.ready && !fx.handing) entry_drop();
        fx.placed = 0;
        fx.handing = 0;
    }
    if (stand.said_off >= (u32)STAND_SAID) return;
    ++stand.said_off;
    STOCK_LOG(3, LOG_FILE, 24, "page: '%s' no longer stands in: %s", op1f_cards.slots[stand.slot].card->name, gone_why(why, effect));
}

// The card's page taken out from under the shown one, where it lies there.
// R3: never the stand-in, which is an entry of its own.
static void take_out_of(u32 slot) {
    for (u32 i = stand.on == (u32)STAND_UNDER ? 1u : 0u; i < pages.count; ++i) {
        if (pages.slot[i] == slot) {
            take_out(i);
            return;
        }
    }
}

// R6b: a ride's page under another page, taken out because the unit went to another mode: its entry, still held, is
// kept ready for its mode as effect_off_top keeps it for a ride's page at the top, with the page's document as the one
// it stands with; ready_frame keeps it across the visit and takes it back as any entry kept ready.
static OP1F_APART void under_ready() {
    fx.ready = 1;
    fx.ready_document = stand.document;
    fx.ready_slot = stand.slot;
    fx.ready_ask = stand.ask;
    fx.ready_away = 0;
}

// The pages kept under the shown one closed: all of them for `all` (CLOSED_BY_MODE), else those
// whose packs closed them since the last frame (a bit a slot in `closing`). One line each, the lowest first.
// R3: the stand-in, lowest, is closed here only with all of them: its card's close is not its own.
static OP1F_APART void under_closed(u32 closing, u32 all) {
    u32 i = 0;
    while (i < pages.count) {
        u32 slot = pages.slot[i];
        u32 standing = i == 0 && stand.on == (u32)STAND_UNDER ? 1u : 0u;
        if (all == 0 && (standing || ((closing >> slot) & 1u) == 0)) {
            ++i;
            continue;
        }
        take_out(i);
        if (standing) {
            if (fx.placed && fx.held != 0 && &op1f_effects_ahead != 0) under_ready();   // R6b: its entry held by its mode across the visit
            stand_said_gone(STOOD_BY_MODE);                                         // R3: a mode change
            continue;
        }
        ++state.closes;
        said_closed(slot, all != 0 ? all : (u32)CLOSED_BY_PACK);
    }
}

// The page shown kept under the page that comes over it (`over`, the slot + 1), with what it showed. Four lie under
// the shown one at most: the lowest is dropped, with a line, to make room.
// R3: the lowest page kept dropped to make room, with a line: never the stand-in.
static OP1F_APART void drop_lowest(u32 mode) {
    u32 at = stand.on == (u32)STAND_UNDER ? 1u : 0u;
    u32 lowest = pages.slot[at];
    take_out(at);
    ++pages.dropped;
    ++state.closes;
    STOCK_LOG(3, LOG_FILE, 18, "page: '%s' dropped from under the pages of mode %u: more than %u lie over one another", op1f_cards.slots[lowest].card->name, mode,
              (u32)UNDER_MOST);
}

// R3: a stand-in shown is covered as any page shown with op1f_page_show, and lies under from then on.
static OP1F_APART void cover(u32 mode, u32 over) {
    u32 slot = state.shown - 1u;
    if (pages.count >= (u32)UNDER_MOST) drop_lowest(mode);
    u32 n = pages.count;
    pages.slot[n] = slot;
    pages.document[n] = pages.shown_document;
    pages.how[n] = pages.shown_how;
    pages.count = n + 1u;
    if (stand.on == (u32)STAND_TOP) stand.on = STAND_UNDER;                         // R3: nothing lay under it: it is pages.slot[0]
    ++pages.covered;
    STOCK_LOG(3, LOG_FILE, 16, "page: '%s' covered by '%s' in mode %u: it shows again when that page closes", op1f_cards.slots[slot].card->name,
              op1f_cards.slots[over - 1u].card->name, mode);
}

// The page on top closed by its pack, with pages kept under it: the highest of them shows again in its place at this
// frame, its document put under 77 before the next draw; the word our call reads stays, so the firmware is answered
// 77 throughout, and no request is made. Its request is written back as if its pack had asked again, in one masked
// instant, only while no other request waits: when one does, nothing moves now, the page on top closes as before and
// the request waiting is taken at the next frame over the pages kept. 1 when the page under shows again.
// R3: the stand-in shows again the same way, with no request written for it: it stands by its condition alone.
static OP1F_APART int uncover(u32 mode) {
    u32 n = pages.count - 1u, slot = pages.slot[n], how = pages.how[n], put = 0, mask;
    u32 standing = n == 0 && stand.on == (u32)STAND_UNDER ? 1u : 0u;
    void* document = pages.document[n];
    MASK_OFF(mask);
    if (page_asked == 0) {
        if (!standing) {
            page_document[slot] = document;
            page_how[slot] = how;
            page_asked = (slot + 1u) | (mode << 8);
        }
        put = 1;
    }
    MASK_ON(mask);
    if (!put) return 0;
    pages.count = n;
    if (standing) stand.on = STAND_TOP;
    u32 was = state.shown;
    state.screen[SCREEN_DOCUMENT] = (u32)document;
    state.shown = slot + 1u;
    pages.shown_document = document;
    pages.shown_how = how;
    ++state.closes;
    ++pages.uncovered;
    said_closed(was - 1u, CLOSED_BY_PACK);
    STOCK_LOG(3, LOG_FILE, 17, "page: '%s' shown again in mode %u, its document at 0x%08x under screen %u", op1f_cards.slots[slot].card->name, mode, (u32)document,
              (u32)PAGE_SCREEN);
    return 1;
}

// The document a slot's request stands for: its own, or the base's blank page, made here once, at the first request
// that needs it, from the base's own picture. 0 when the blank page could not be made.
static OP1F_APART void* blank_page();

static OP1F_APART void* asked_document(u32 slot) {
    if (page_how[slot] != (u32)HOW_BLANK) return page_document[slot];
    return blank_page();
}

// R3: the base's blank page, made once, at the first page that needs it: a request's or a stand-in's.
static OP1F_APART void* blank_page() {
    if (!pages.blank_tried) {
        pages.blank_tried = 1;
        pages.blank = op1f_base_picture(BLANK_PICTURE);
        if (pages.blank != 0)
            STOCK_LOG(3, LOG_FILE, 19, "page: the base's blank page made from blank.svg, its document at 0x%08x", (u32)pages.blank);
        else
            STOCK_LOG(1, LOG_FILE, 20, "page: the base's blank page could not be made: a page asked with no document is not shown");
    }
    return pages.blank;
}

// A request the frame cannot take (the blank page could not be made): withdrawn, and the page shown, if any, asked
// again as it stood, so that it stays; in one masked instant, and only while the request is still the one looked at.
// R3: a stand-in shown has no request: the word is cleared, and it stays by its condition.
static OP1F_APART void ask_again_shown(u32 seen) {
    u32 shown = stand.on == (u32)STAND_TOP ? 0u : state.shown, mask;
    MASK_OFF(mask);
    if (page_asked == seen) {
        if (shown != 0) {
            page_document[shown - 1u] = pages.shown_document;
            page_how[shown - 1u] = pages.shown_how;
        }
        page_asked = shown != 0 ? shown | (pages.mode << 8) : 0u;
    }
    MASK_ON(mask);
}

// ---------------------------------------------------------------- R4b: the page that stays with its mode

// The instrument the unit is on, read when a page is put under 77 in a mode (opened, or opened again in another mode):
// a page that then waits for its mode waits only while the instrument is the same. A page kept under another and shown
// again lies in the mode of the page that covered it, whose instrument was read when it was opened.
static inline void stay_shown() {
    stay.instrument = RD32(INSTRUMENT);
}

// Once a frame from page_frame, before the request is taken, with the request word as page_frame read it: 1 when the
// page shown waits for its mode now, and page_frame leaves everything as it stands; 0 otherwise. It waits when the
// request is its own card's, asked with OP1F_PAGE_STAYS and stamped with a mode other than the unit's, and the
// instrument is the one it was put under 77 with; where only the instrument is not, `stay.lost` says so, and the page
// is closed for it. The stand-in shown has no request and never waits. Its lines: when a page begins to wait, and when
// it is back in its mode as it stood (the word our call reads still that mode + 1: the firmware's request answered 77).
static OP1F_APART int stay_look(u32 asked, u32 mode) {
    u32 slot1 = asked & 0xffu, was = stay.waiting;
    stay.waiting = 0;
    stay.lost = 0;
    if (asked == 0 || slot1 == 0 || slot1 > (u32)MOST || slot1 != state.shown || stand.on == (u32)STAND_TOP || page_stays[slot1 - 1u] == 0) return 0;
    if ((asked >> 8) == mode) {
        if (was == slot1 && op1f_page_screen_mode == mode + 1u) {
            ++stay.back;
            STOCK_LOG(3, LOG_FILE, 30, "page: '%s' shown again in mode %u, as it stayed; %u time(s) since the start", op1f_cards.slots[slot1 - 1u].card->name, mode,
                      stay.back);
        }
        return 0;
    }
    if (RD32(INSTRUMENT) != stay.instrument) {
        stay.lost = 1;
        return 0;
    }
    stay.waiting = slot1;
    if (was == slot1) return 1;
    ++stay.kept;
    STOCK_LOG(3, LOG_FILE, 29, "page: '%s' stays with mode %u while the unit is in mode %u: shown again when the unit comes back; %u time(s) since the start",
              op1f_cards.slots[slot1 - 1u].card->name, asked >> 8, mode, stay.kept);
    return 1;
}

// ---------------------------------------------------------------- R4c: the page set aside

// The slot set aside and the word our call reads for it, written in one masked instant: a card's close, from any
// thread, looks at the slot and clears the word in one.
static inline void away_word(u32 slot1, u32 word) {
    u32 mask;
    MASK_OFF(mask);
    away.slot1 = slot1;
    op1f_page_aside_mode = word;
    MASK_ON(mask);
}

// The page set aside closed, with its line; where the unit is in its mode the firmware is asked again, since a
// request may have answered 77 for it.
static OP1F_APART void away_close(u32 why, u32 mode) {
    u32 slot1 = away.slot1;
    if (slot1 == 0) return;
    away_word(0, 0);
    ++state.closes;
    said_closed(slot1 - 1u, why);
    if (away.mode == mode) request_screen();
}

// Its own card asked for its page again: the request is the card's page from then on, and nothing is said.
static inline void away_drop() {
    away_word(0, 0);
}

// A page kept aside, with its line. The word our call reads for it is written before the caller clears the page's
// own, so that the firmware's request answers 77 in its mode throughout.
static OP1F_APART void away_store(const Away* y, u32 mode) {
    away.mode = y->mode;
    away.instrument = y->instrument;
    away.document = y->document;
    away.how = y->how;
    away_word(y->slot1, y->mode + 1u);
    ++away_set;
    STOCK_LOG(3, LOG_FILE, 34, "page: '%s' set aside while another page takes the screen in mode %u: it waits for mode %u again when that page goes; %u time(s) since the start",
              op1f_cards.slots[y->slot1 - 1u].card->name, mode, y->mode, away_set);
}

// The page shown, which waits for its mode, as it would be kept aside.
static inline void away_of_shown(Away* y) {
    y->slot1 = state.shown;
    y->mode = pages.mode;
    y->instrument = stay.instrument;
    y->document = pages.shown_document;
    y->how = pages.shown_how;
}

// The page shown, which waits for its mode, kept aside because another card's page takes 77 in the mode the unit is
// in (page_frame, which then puts that page under 77); an older page set aside is closed for it.
static OP1F_APART void away_keep(u32 mode) {
    if (away.slot1 != 0) away_close(CLOSED_BY_TAKEN, mode);
    Away y;
    away_of_shown(&y);
    away_store(&y, mode);
}

// The page that waits set aside because a stand-in takes 77 in another mode: what base 26 did to close it (its
// request withdrawn, the word our call reads cleared, the document of before back under 77; with none kept, the base's
// own 77, the page's document stays until the stand-in's takes its place), but kept aside and not closed. From
// stand_frame, right before the stand-in is placed.
static OP1F_APART void stay_aside(u32 mode) {
    u32 was = state.shown, seen = page_asked;
    void* before = state.kept;
    away_keep(mode);
    if ((seen & 0xffu) == was) close_asked(seen);
    op1f_page_screen_mode = 0;
    BARRIER();
    if (before != 0) state.screen[SCREEN_DOCUMENT] = (u32)before;
    state.shown = 0;
    stay.waiting = 0;
}

// The page set aside asked for again in its card's name as its card had asked it, with the flag, in one masked
// instant and only while the request word is still `seen`: 1 when written.
static OP1F_APART u32 away_ask(const Away* x, u32 seen) {
    u32 slot = x->slot1 - 1u, mask, put = 0;
    MASK_OFF(mask);
    if (page_asked == seen) {
        page_document[slot] = x->document;
        page_how[slot] = x->how;
        page_stays[slot] = 1u;
        page_asked = x->slot1 | (x->mode << 8);
        put = 1;
    }
    MASK_ON(mask);
    return put;
}

// The page set aside put back under 77 as the page shown, its request written back already: the pages' record, the
// instrument it was put under 77 with, and the word our call reads, its mode + 1. In its mode the firmware is asked at
// once, with line 30; in another the page waits for it, with line 29. Its record is read, not copied: a copy of the
// record is a loop the compiler runs through an index register.
static OP1F_APART void away_put(const Away* x, u32 mode) {
    state.screen[SCREEN_DOCUMENT] = (u32)x->document;
    state.shown = x->slot1;
    pages.shown_document = x->document;
    pages.shown_how = x->how;
    pages.mode = x->mode;
    stay.instrument = x->instrument;
    BARRIER();                                                                      // the document is in place before our call answers 77 by this word
    op1f_page_screen_mode = x->mode + 1u;
    const char* name = op1f_cards.slots[x->slot1 - 1u].card->name;
    if (x->mode == mode) {
        stay.waiting = 0;
        ++stay.back;
        request_screen();
        STOCK_LOG(3, LOG_FILE, 30, "page: '%s' shown again in mode %u, as it stayed; %u time(s) since the start", name, mode, stay.back);
        return;
    }
    stay.waiting = x->slot1;
    ++stay.kept;
    STOCK_LOG(3, LOG_FILE, 29, "page: '%s' stays with mode %u while the unit is in mode %u: shown again when the unit comes back; %u time(s) since the start",
              name, x->mode, mode, stay.kept);
}

// The unit came back to the mode of the page set aside while another page that stays waits in its place (from
// page_frame, where stay_look said so): the two change places. The page set aside is asked for again in the place of
// the request of the one that waits, in one masked instant (nothing moves when another request came meanwhile: the
// next frame takes it), and put under 77; the one that waited is kept aside in its turn.
static OP1F_APART void away_swap(u32 asked, u32 mode) {
    if (RD32(INSTRUMENT) != away.instrument) return;                               // away_frame closes it
    if (!away_ask(&away, asked)) return;
    Away y;
    away_of_shown(&y);
    away_put(&away, mode);                                                          // read before the one that waited takes its slot
    away_store(&y, mode);
}

// Once a frame on the display thread, after stand_frame: the page set aside closed when the instrument changed, and put
// back as soon as 77 is free (no page shown, nothing kept under, no stand-in, not under the base's top page) and no
// request waits; a request that waits is taken first, at the next frame.
static OP1F_APART void away_frame(u32 frame, u32 mode) {
    if (away.slot1 == 0 || top.shown) return;
    if (RD32(INSTRUMENT) != away.instrument) {
        away_close(CLOSED_BY_INSTRUMENT, mode);
        return;
    }
    if (state.shown != 0 || pages.count != 0 || stand.on != (u32)STAND_NONE) return;   // another page holds 77
    u32* screen = page_screen(frame);
    if (screen == 0) return;
    if (!away_ask(&away, 0)) return;
    state.screen = screen;
    state.kept = (void*)screen[SCREEN_DOCUMENT];
    away_put(&away, mode);
    away_word(0, 0);                                                                // after the page's own word: our call answers 77 in its mode throughout
}

// R6b: back in the mode of the page set aside, with the instrument it was put under 77 with, and before a ride's page
// (an effect's or an engine's) of that mode is placed at the top (from stand_frame, with nothing shown and nothing kept
// under): the page set aside goes back first, as away_frame puts it back, so that stand_frame puts the ride's page
// under it, as the two lay before it was set aside. A request that waits is taken first, as away_frame leaves it.
static OP1F_APART void away_first(u32 frame, u32 mode) {
    if (away.slot1 == 0 || away.mode != mode || top.shown || RD32(INSTRUMENT) != away.instrument) return;
    u32* screen = page_screen(frame);
    if (screen == 0) return;
    if (!away_ask(&away, 0)) return;
    state.screen = screen;
    state.kept = (void*)screen[SCREEN_DOCUMENT];
    away_put(&away, mode);
    away_word(0, 0);                                    // R6b: as away_frame does, behind the page's own word
}


// Once a frame on the display thread, before the cards' frame functions: the page asked for against the page shown.
// R1: and the pages kept under it.
// R4b: and the page that stays with its mode: while it waits nothing moves; another's request in its place, a request
// dropped for its mode in its place, and the instrument changed close it with their own reasons.
static OP1F_APART void page_frame(u32 frame, u32 mode) {
    if (top.shown) return;                                                          // S1: a pack's page waits under the base's top page
    u32 mask, closing;
    MASK_OFF(mask);                                                                 // R1: the closes since the last frame, taken
    closing = pages.closing;
    pages.closing = 0;
    MASK_ON(mask);
    if (pages.count != 0) under_closed(closing, pages.mode != mode ? (u32)CLOSED_BY_MODE : 0u);   // R1: pages kept in another mode, or closed by their packs
    if (away.slot1 != 0 && ((closing >> (away.slot1 - 1u)) & 1u) != 0) away_close(CLOSED_BY_PACK, mode);   // R4c: the page set aside closed by its pack
    u32 asked = page_asked;
    u32 want = 0, why = CLOSED_BY_PACK;
    void* document = 0;
    u32 waited = stay.waiting;                                                      // R4b: the page that waited at the last frame
    u32 taken = waited != 0 && waited == state.shown && asked != 0 && (asked & 0xffu) != waited ? 1u : 0u;   // R4b: another's request in its place
    if (stay_look(asked, mode)) {                                                   // R4b: the page shown waits for its mode: nothing moves
        if (away.slot1 != 0 && away.mode == mode) away_swap(asked, mode);          // R4c: but where the page set aside is of this mode, the two change places
        return;
    }
    if (asked != 0) {
        u32 slot = (asked & 0xffu) - 1u;
        if ((asked >> 8) != mode) {
            close_asked(asked);
            why = stay.lost ? (u32)CLOSED_BY_INSTRUMENT : taken ? (u32)CLOSED_BY_TAKEN : (u32)CLOSED_BY_MODE;   // R4b
        } else if (slot < state.seen && ((state.used >> slot) & 1u) != 0) {
            document = asked_document(slot);                                        // R1: the blank page made here, once
            if (document == 0) {
                ask_again_shown(asked);                                             // R1: no blank page: the request dropped, what was shown stays
                return;
            }
            want = slot + 1u;
        }
    }
    if (want == 0 && stand.on == (u32)STAND_TOP) return;                            // R3: the stand-in shown has no request: stand_frame keeps it or closes it
    if (want == state.shown && stand.on != (u32)STAND_TOP) {                        // R3: the stand-in's own card asks its page over it
        if (want == 0 || state.screen == 0) return;
        pages.shown_document = document;                                            // R1
        pages.shown_how = page_how[want - 1];
        if ((void*)state.screen[SCREEN_DOCUMENT] != document)
            state.screen[SCREEN_DOCUMENT] = (u32)document;                          // the pack opened its page again with another document
        if (op1f_page_screen_mode == mode + 1u) return;
        BARRIER();                                                                  // opened again in another mode before this frame saw the change
        op1f_page_screen_mode = mode + 1u;
        pages.mode = mode;                                                          // R1
        stay_shown();                                                               // R4b
        request_screen();
        STOCK_LOG(3, LOG_FILE, 7, "page: '%s' opened in mode %u, its document at 0x%08x under screen %u, %s", op1f_cards.slots[want - 1].card->name, mode,
                  (u32)document, (u32)PAGE_SCREEN, state.own_made ? "the base's own" : "another unit's");
        return;
    }
    if (want != 0) {
        u32* screen = page_screen(frame);
        if (screen == 0) return;                                                    // the next frame
        if (away.slot1 == want) away_drop();                                        // R4c: its own card asks for its page: the request is the page now
        take_out_of(want - 1u);                                                     // R1: the card's page, where it lay under, comes to the top
        if (state.shown == 0) {
            state.screen = screen;
            state.kept = (void*)screen[SCREEN_DOCUMENT];
        } else if (pages.shown_how != (u32)HOW_OPEN && pages.mode == mode && (stand.on == (u32)STAND_TOP || ((closing >> (state.shown - 1u)) & 1u) == 0)) {
            cover(mode, want);                                                      // R1: a page shown with op1f_page_show is kept under the one that comes over it
        } else if (stand.on == (u32)STAND_TOP) {
            if (fx.placed && fx.held != 0 && &op1f_effects_ahead != 0) under_ready();   // R6b: a ride's page: its entry held by its mode across the visit
            stand_said_gone(STOOD_BY_MODE);                                         // R3: a page of another mode takes the stand-in's place
        } else if (taken) {
            away_keep(mode);                                                        // R4c: the page that waited is set aside, not closed
        } else {
            u32 how = pages.shown_how == (u32)HOW_OPEN ? (u32)CLOSED_BY_OTHER : pages.mode != mode ? (u32)CLOSED_BY_MODE : (u32)CLOSED_BY_PACK;
            STOCK_LOG(3, LOG_FILE, 8, "page: '%s' closed: %s; %u opened since the start", op1f_cards.slots[state.shown - 1].card->name, closed_why(how), state.opened);
        }
        screen[SCREEN_DOCUMENT] = (u32)document;
        state.shown = want;
        pages.shown_document = document;                                            // R1
        pages.shown_how = page_how[want - 1];
        pages.mode = mode;
        stay_shown();                                                               // R4b
        ++state.opened;
        BARRIER();                                                                  // the document is in place before our call in the request answers 77
        op1f_page_screen_mode = mode + 1u;
        request_screen();
        STOCK_LOG(3, LOG_FILE, 7, "page: '%s' opened in mode %u, its document at 0x%08x under screen %u, %s", op1f_cards.slots[want - 1].card->name, mode,
                  (u32)document, (u32)PAGE_SCREEN, state.own_made ? "the base's own" : "another unit's");
        return;
    }
    if (pages.count != 0) {                                                         // R1: the pages kept under the one that closes
        if (why == CLOSED_BY_PACK && uncover(mode)) return;
        if (why != CLOSED_BY_PACK) under_closed(0, why);
    }
    op1f_page_screen_mode = 0;                                                      // our call answers the firmware's screen again
    BARRIER();
    if (state.kept != 0) state.screen[SCREEN_DOCUMENT] = (u32)state.kept;          // M3: with none kept (the base's own 77) the pack's document stays: 77 is never left without one
    u32 was = state.shown;
    state.shown = 0;
    ++state.closes;
    request_screen();
    STOCK_LOG(3, LOG_FILE, 8, "page: '%s' closed: %s; %u opened since the start", op1f_cards.slots[was - 1].card->name, closed_why(why), state.opened);
}

// The slot + 1 whose page the GUI shows now, 0 for none.
static inline u32 page_on_screen(u32 screen) {
    return screen == (u32)PAGE_SCREEN ? state.shown : 0u;
}

// ---------------------------------------------------------------- R3: a pack's page standing in for a screen

// The head of a card's ask whose page may stand in for a screen (a sequencer's), or 0: the card in use, the ask one of
// its list, of that kind, with a table at least a head's size (the check held it to lie in its pack with its bytes).
// R5: or an effect's, of a card of level 8.
static const Op1fKindHead* head_of(u32 slot, u32 ask) {
    if (slot >= state.seen || ((state.used >> slot) & 1u) == 0) return 0;
    u32 count = 0;
    const Op1fAsk* asks = (const Op1fAsk*)asks_of(op1f_cards.slots[slot].card, &count);
    if (ask < count && asks[ask].kind == (u32)OP1F_ASK_EFFECT && level_8(op1f_cards.slots[slot].card) && asks[ask].what != 0 &&
        asks[ask].bytes >= (u32)sizeof(Op1fKindHead))
        return (const Op1fKindHead*)asks[ask].what;                                 // R5
    if (ask < count && asks[ask].kind == (u32)OP1F_ASK_ENGINE && level_9(op1f_cards.slots[slot].card) && asks[ask].what != 0 &&
        asks[ask].bytes >= (u32)sizeof(Op1fKindHead))
        return (const Op1fKindHead*)asks[ask].what;                                 // R6
    if (ask < count && asks[ask].kind == (u32)OP1F_ASK_LFO && level_8(op1f_cards.slots[slot].card) && ((lfos.good[slot] >> ask) & 1u) != 0)
        return (const Op1fKindHead*)asks[ask].what;                                 // R11 (base 35): an LFO layer's, held to its form
    if (ask >= count || asks[ask].kind != (u32)OP1F_ASK_SEQUENCER || asks[ask].what == 0 || asks[ask].bytes < (u32)sizeof(Op1fKindHead)) return 0;
    return (const Op1fKindHead*)asks[ask].what;
}

// The document that stands in for one of a card's asks, from its frame function. 0 is the base's blank page, on which
// the pack paints; a document must be one this card made. It is taken at the next frame.
extern "C" int op1f_page_stand_in(const Op1fCard* card, unsigned ask, void* document) {
    int slot = slot_of(card);
    if (slot < 0 || op1f_cards_calling() != (u32)slot + 1u) return 0;              // from the card's own frame function, and no other call
    if (head_of((u32)slot, ask) == 0) return 0;
    if (document != 0 && !made_by((u32)slot, document)) return 0;
    stand.document_of[slot][ask] = document;
    stand.set[slot] |= 1u << ask;
    return 1;
}

// What stands in for an ask: the document its card set, else the base's blank page (0 when that cannot be made).
// `named`: the card set nothing, and the base paints the ask's name on it.
// R5: the base's blank page, with the ask's name and the base's words, while an effect's place passes the sound dry.
static OP1F_APART void* stand_document(u32 slot, u32 ask, u32* named) {
    if (fx.dry) {                                                                   // R5
        *named = 1;
        return blank_page();
    }
    u32 set = (stand.set[slot] >> ask) & 1u;
    void* document = set ? stand.document_of[slot][ask] : 0;
    *named = set ? 0u : 1u;
    return document != 0 ? document : blank_page();
}

// The stand-in put in as the lowest of the pages kept under the one shown, the lowest of them dropped to make room.
static OP1F_APART void put_lowest(u32 mode, u32 slot, void* document, u32 how) {
    if (pages.count >= (u32)UNDER_MOST) drop_lowest(mode);
    for (u32 i = pages.count; i > 0; --i) {
        pages.slot[i] = pages.slot[i - 1u];
        pages.document[i] = pages.document[i - 1u];
        pages.how[i] = pages.how[i - 1u];
    }
    pages.slot[0] = slot;
    pages.document[0] = document;
    pages.how[0] = how;
    pages.count = pages.count + 1u;
}

// The stand-in shown taken off: the document of before back under 77 and the firmware's screen asked again.
static OP1F_APART void stand_off_top() {
    op1f_page_screen_mode = 0;
    BARRIER();
    void* kept = state.kept;
    if (kept != 0) state.screen[SCREEN_DOCUMENT] = (u32)kept;                       // with none kept (the base's own 77) the document stays, as for any page
    state.shown = 0;
    request_screen();
}

// ---------------------------------------------------------------- R5: an effect's page standing in

// R11 (base 35): is the LFO unit's answer an ask of kind OP1F_ASK_LFO held to its form (a card of level 8 or later)?
static inline int lfo_ask(u32 slot, u32 ask) {
    return slot < (u32)MOST && ask < (u32)ASKS_MOST && ((lfos.good[slot] >> ask) & 1u) != 0;
}

// R11 (base 35): else an LFO layer's page, of a card: what the LFO unit says of the mode and the screen is kept for
// stand_frame, as an effect's is, with entry 3 for it.
static OP1F_APART int lfo_standing(Op1fStanding* s) {
    u32 mode = 0, screen = 0;
    if (&op1f_lfos_standing == 0 || !op1f_lfos_standing(s, &mode, &screen) || !lfo_ask(s->slot, s->ask)) return 0;
    fx.asked_engine = BY_LFOS;
    fx.asked = 1;
    fx.asked_mode = mode;
    fx.asked_screen = screen;
    return 1;
}

// R11 (base 35): else an LFO layer's page whatever screen is asked, as ahead_of asks the effects unit.
static OP1F_APART int lfo_ahead(u32 mode, Op1fStanding* out, u32* screen) {
    u32 m = 0;
    if (&op1f_lfos_ahead == 0 || !op1f_lfos_ahead(out, &m, screen) || !lfo_ask(out->slot, out->ask)) return 0;
    fx.ahead_engine = BY_LFOS;
    return m == mode ? 1 : 0;
}

// Does an effect's page stand in now? Asked where the sequencer unit says no page stands in: what the effects unit
// says of the mode and the screen is kept for stand_frame.
// R11 (base 35): where neither the effects unit nor the engines unit says so, the LFO unit is asked.
static OP1F_APART int effect_standing(Op1fStanding* s) {
    u32 mode = 0, screen = 0;
    fx.asked_engine = 0;                                                            // R6
    if (&op1f_effects_standing == 0 || !op1f_effects_standing(s, &mode, &screen)) {
        const char* name = 0;                                                       // R6: else an engine's page, of a card (an engine away has none)
        if (&op1f_engines_standing == 0 || !op1f_engines_standing(s, &mode, &screen, &name) || s->slot >= (u32)OP1F_CARDS_MOST) return lfo_standing(s);   // R11 (base 35)
        fx.asked_engine = 1;
    }
    fx.asked = 1;
    fx.asked_mode = mode;
    fx.asked_screen = screen;
    return 1;
}

// Does the place the owner sees in `mode` still ride the effect of the card's ask `ask`, whatever screen is asked for?
// The effects unit's op1f_effects_ahead, where it offers it; `out` is its answer.
static OP1F_APART int ahead_of(u32 mode, Op1fStanding* out, u32* screen) {
    u32 m = 0;
    fx.ahead_engine = 0;                                                            // R6
    fx.ahead_away = 0;                                                              // R6b
    if (&op1f_effects_ahead == 0 || !op1f_effects_ahead(out, &m, screen)) {
        const char* name = 0;                                                       // R6: else an engine's page, of a card
        if (&op1f_engines_ahead == 0 || !op1f_engines_ahead(out, &m, screen, &name)) return lfo_ahead(mode, out, screen);   // R11 (base 35): else an LFO layer's
        if (out->slot == OP1F_STAND_NO_CARD && (out->how & (u32)OP1F_STAND_AWAY) != 0 && name != 0) {   // R6b: an engine away: the base's own page
            fx.ahead_engine = 1;
            fx.ahead_away = 1;
            fx.ahead_name = name;
            fx.ahead_where = out->where;
            out->ask = 0;
            return m == mode ? 1 : 0;
        }
        if (out->slot >= (u32)OP1F_CARDS_MOST) return 0;
        fx.ahead_engine = 1;
    }
    return m == mode && head_of(out->slot, out->ask) != 0 ? 1 : 0;
}

// The effect's page shown taken off. Where only the screen moved (the place the owner sees still rides that effect,
// op1f_effects_ahead says), or the unit went to another mode, where it cannot be asked (with op1f_effects_ahead
// offered), its entry and its document under 77 stay, kept ready for the screen asked again; else the entry is taken
// back, then the document of before goes back. The firmware's screen asked again either way.
static OP1F_APART void effect_off_top(u32 mode) {
    Op1fStanding a;
    u32 screen = 0;
    op1f_page_screen_mode = 0;                                                      // left by a page over it that went in this frame
    u32 keep = mode != stand.mode && &op1f_effects_ahead != 0 ? 1u : 0u;            // left for another mode: kept as it is until the unit is back
    if (!keep && ahead_of(mode, &a, &screen) && a.slot == stand.slot && a.ask == stand.ask && fx.held_mode1 == mode + 1u && fx.held_screen == screen &&
        fx.ahead_engine == fx.placed_engine) keep = 1;                             // R6: and of the same kind
    if (keep && fx.held != 0) {
        fx.ready = 1;
        fx.ready_document = (void*)state.screen[SCREEN_DOCUMENT];
        fx.ready_slot = stand.slot;
        fx.ready_ask = stand.ask;
        state.shown = 0;
        request_screen();
        return;
    }
    entry_drop();
    BARRIER();                                                                      // the entry taken back before the document of before goes back
    void* prior = state.kept;
    if (prior != 0) state.screen[SCREEN_DOCUMENT] = (u32)prior;                    // with none kept (the base's own 77) the document stays, as for any page
    state.shown = 0;
    request_screen();
}

// The effect's page shown handed over to another effect's page in the same mode, which stand_frame puts under 77 below
// in the same frame: the document of before back under 77 for it to keep, no request, and the entry left as it is, so
// that our call answers 77 for the effect's screen throughout; the new page writes its own entry over it.
static OP1F_APART void effect_handed() {
    void* prior = state.kept;
    if (prior != 0) state.screen[SCREEN_DOCUMENT] = (u32)prior;
    state.shown = 0;
    fx.handing = 1;
}

// The effect's page shown again from under a page of its mode that closed (uncover): the word of the whole mode, which
// the page over it held, gives way to the entry of the effect's one screen: the entry written first, then the word
// cleared, so that our call answers 77 for that screen throughout.
static OP1F_APART void effect_uncovered(u32 mode) {
    entry_write(mode, fx.asked_screen, fx.asked_engine);                            // R6: its kind's entry
    BARRIER();
    op1f_page_screen_mode = 0;
}

// A place said as the owner meets it, for line 39.
static const char PLACES_SAID[(u32)OP1F_PLACE_MASTER + 1u][12] = {"synth 1", "synth 2", "synth 3", "synth 4", "synth 5", "synth 6", "synth 7", "synth 8",
                                                                  "drum 1", "drum 2", "drum 3", "drum 4", "drum 5", "drum 6", "drum 7", "drum 8", "the master"};

static OP1F_APART void said_effect(u32 slot, u32 where, const void* document) {
    STOCK_LOG(3, LOG_FILE, 39, "page: '%s' stands in for the effect's page of %s, its document at 0x%08x", op1f_cards.slots[slot].card->name,
              where <= (u32)OP1F_PLACE_MASTER ? PLACES_SAID[where] : "a place", (u32)document);
}

// R11 (base 35): an LFO layer's page placed, line 48.
static OP1F_APART void said_lfo(u32 slot, u32 where, const void* document) {
    STOCK_LOG(3, LOG_FILE, 48, "page: '%s' stands in for the LFO's page of %s, its document at 0x%08x", op1f_cards.slots[slot].card->name,
              where < (u32)OP1F_PLACE_MASTER ? PLACES_SAID[where] : "a place", (u32)document);
}

// R6d: the base's own page for an engine away, made once, at the first entry kept ready for one, from the base's own
// picture away.svg: the blank page with the two placeholders of its words. Where it cannot be made, the blank page
// stands in for it (and the base's own font for its words, away_paint), as in base 29; one line says which, once.
static OP1F_APART void* away_page() {
    if (!awaypage.tried) {
        awaypage.tried = 1;
        awaypage.document = op1f_base_picture(AWAY_PICTURE);
        if (awaypage.document != 0)
            STOCK_LOG(3, LOG_FILE, 42, "page: the page for an engine away made from away.svg, its document at 0x%08x", (u32)awaypage.document);
        else
            STOCK_LOG(2, LOG_FILE, 43, "page: away.svg could not be made: an engine away shows the blank page with the base font");
    }
    return awaypage.document != 0 ? awaypage.document : blank_page();
}

// Once a frame on the display thread, after the stand-in's work (and in a frame with nothing else to do): the entry
// kept ready while no effect's page is up. An entry held with no stand-in placed and not ready is taken back. One held
// ready stays while its page's document stands under 77 and, in its own mode, while the place the owner sees there
// still rides that effect; in another mode it stays as it is, but where a place of the mode the unit is in rides an
// effect (one document under 77). Where the last request of its mode was answered 77 by it, the firmware is asked again
// once it is taken back, so that its own screen comes back. With none held, where the place the owner sees in the mode
// rides an effect, its page is put
// under 77 while 77 is free (no page shown, kept, set aside or standing in, no word of a mode, not under the base's top
// page) and not on the screen, and its entry written after it.
static OP1F_APART void ready_frame(u32 mode, u32 screen) {
    if (fx.placed || top.shown) return;
    Op1fStanding a;
    u32 at = 0;
    int ahead = ahead_of(mode, &a, &at);
    if (fx.held != 0) {
        u32 other = fx.ready && state.screen != 0 && fx.held_mode1 != mode + 1u ? 1u : 0u;   // R6b: held for another mode than the unit is in
        u32 mine = fx.ready && state.screen != 0 && (void*)state.screen[SCREEN_DOCUMENT] == fx.ready_document ? 1u : 0u;
        if (other) mine = 1u;                                                       // R6b: kept whatever document that mode puts under 77
        if (mine && fx.held_mode1 == mode + 1u) mine = ahead && at == fx.held_screen && a.slot == fx.ready_slot && a.ask == fx.ready_ask ? 1u : 0u;
        else if (mine && ahead) mine = 0;                                           // the mode the unit is in has a ride of its own
        if (mine) return;
        u32 answered = fx.held_mode1 == mode + 1u && op1f_screen_asked == fx.held_screen ? 1u : 0u;   // the last request answered 77 by it
        entry_drop();
        if (answered && state.shown == 0 && stand.on == (u32)STAND_NONE) request_screen();   // the firmware's own screen back
    }
    if (!ahead || screen == (u32)PAGE_SCREEN) return;
    if (state.shown != 0 || pages.count != 0 || stand.on != (u32)STAND_NONE || away.slot1 != 0 || op1f_page_screen_mode != 0 || op1f_page_aside_mode != 0) return;
    u32 named = 0;
    fx.dry = (a.how & DRY) != 0 ? 1u : 0u;
    void* document = fx.ahead_away ? away_page() : stand_document(a.slot, a.ask, &named);    // R6d: an engine away: the base's own page (away.svg)
    fx.dry = 0;
    if (document == 0) return;
    u32* s77 = page_screen(op1f_frame_counter);
    if (s77 == 0) return;
    state.screen = s77;
    if ((void*)s77[SCREEN_DOCUMENT] != document) s77[SCREEN_DOCUMENT] = (u32)document;
    BARRIER();                                                                      // the document is in place before our call answers 77 for the entry
    entry_write(mode, at, fx.ahead_engine);                                         // R6: its kind's entry
    fx.ready = 1;
    fx.ready_document = document;
    fx.ready_slot = a.slot;
    fx.ready_ask = a.ask;
    fx.ready_away = fx.ahead_away;                                                  // R6b
    ++fx.readied;
}

// Once a frame on the display thread, after page_frame: the stand-in placed, kept, changed or closed by what the
// sequencer unit answered (`holds`, `s`). It waits under the base's top page, as every page.
// R5: or the effects unit (fx.asked): an effect's page stands in for one screen of a mode, by an entry of the table.
static OP1F_APART void stand_frame(u32 frame, u32 mode, int holds, const Op1fStanding* s) {
    if (top.shown) return;
    int fits = holds && head_of(s->slot, s->ask) != 0;
    if (fx.asked && fx.asked_mode != mode) fits = 0;                                // R5: the mode moved under the effects unit's answer: the next frame
    fx.dry = fits && fx.asked && (s->how & DRY) != 0 ? 1u : 0u;                     // R5
    if (stand.on != (u32)STAND_NONE) {
        if (fits && s->slot == stand.slot && s->ask == stand.ask && mode == stand.mode) {
            stand.where = s->where;
            stand.how = s->how;
            if (fx.placed && stand.on == (u32)STAND_TOP && op1f_page_screen_mode == mode + 1u) effect_uncovered(mode);   // R5
            u32 named = 0;
            void* document = stand_document(stand.slot, stand.ask, &named);
            stand.named = named;
            if (document == 0 || document == stand.document) return;
            u32 how = document == pages.blank ? (u32)HOW_BLANK : (u32)HOW_SHOW;    // the card set another document
            if (stand.on == (u32)STAND_TOP) {
                state.screen[SCREEN_DOCUMENT] = (u32)document;
                pages.shown_document = document;
                pages.shown_how = how;
            } else {
                pages.document[0] = document;
                pages.how[0] = how;
            }
            stand.document = document;
            return;
        }
        u32 why = mode != stand.mode ? (u32)STOOD_BY_MODE : (u32)STOOD_BY_OTHER;
        if (fx.placed && why == (u32)STOOD_BY_OTHER && !fits) why = STOOD_BY_SCREEN;   // R5
        if (stand.on == (u32)STAND_UNDER) take_out(0);
        else if (fx.placed && fits && fx.asked && mode == stand.mode) effect_handed();   // R5: another effect's page takes its place below
        else if (fx.placed) effect_off_top(mode);                                   // R5
        else stand_off_top();
        stand_said_gone(why);
    }
    if (!fits) return;
    u32 named = 0;
    void* document = stand_document(s->slot, s->ask, &named);
    if (document == 0) return;                                                      // no blank page: said once, where it was tried
    u32 how = document == pages.blank ? (u32)HOW_BLANK : (u32)HOW_SHOW;
    if (state.shown != 0 && stay.waiting == state.shown) stay_aside(mode);          // R4c: a page that waits for its mode is set aside for the stand-in
    if (state.shown == 0 && fx.asked && pages.count == 0) away_first(frame, mode);  // R6b: a page set aside of this mode first, the ride's page under it
    if (state.shown == 0) {
        if (pages.count != 0) return;                                               // pages kept while a request waits: the next frame, over them
        u32* screen = page_screen(frame);
        if (screen == 0) return;                                                    // the next frame
        state.screen = screen;
        state.kept = (void*)screen[SCREEN_DOCUMENT];
        screen[SCREEN_DOCUMENT] = (u32)document;
        state.shown = s->slot + 1u;
        pages.shown_document = document;
        pages.shown_how = how;
        pages.mode = mode;
        stand.on = STAND_TOP;
        BARRIER();                                                                  // the document is in place before our call in the request answers 77
        if (fx.asked) entry_write(mode, fx.asked_screen, fx.asked_engine);          // R5: an effect's page: the entry of its one screen (R6: or an engine's)
        else op1f_page_screen_mode = mode + 1u;
        request_screen();
    } else {
        if (pages.mode != mode) return;                                             // the page shown is of another mode: the next frame
        put_lowest(mode, s->slot, document, how);
        stand.on = STAND_UNDER;
    }
    stand.slot = s->slot;
    stand.ask = s->ask;
    stand.where = s->where;
    stand.how = s->how;
    stand.mode = mode;
    stand.document = document;
    stand.named = named;
    ++stand.placed;
    fx.placed = fx.asked;                                                           // R5
    fx.placed_engine = fx.asked ? fx.asked_engine : 0u;                             // R6
    if (fx.asked) fx.ready = 0;                                                     // R5: an entry kept ready is the stand-in's now
    if (stand.said_up >= (u32)STAND_SAID) return;
    ++stand.said_up;
    if (fx.placed && fx.placed_engine == (u32)BY_LFOS) {                            // R11 (base 35): an LFO layer's
        said_lfo(s->slot, s->where, document);
        return;
    }
    if (fx.placed && fx.placed_engine) {                                            // R6
        STOCK_LOG(3, LOG_FILE, 40, "page: '%s' stands in for the engine's page of synth %u, its document at 0x%08x", op1f_cards.slots[s->slot].card->name,
                  (s->where & 7u) + 1u, (u32)document);
        return;
    }
    if (fx.placed) {                                                                // R5
        said_effect(s->slot, s->where, document);
        return;
    }
    STOCK_LOG(3, LOG_FILE, 23, "page: '%s' stands in for the sequencer's screen of the %s, its document at 0x%08x", op1f_cards.slots[s->slot].card->name,
              s->where == 1u ? "drum" : "synth", (u32)document);
}

// An ask's `shown` called, inside a record of its own: the function read through the card's ask, and held to lie in
// its pack on an even address.
static OP1F_APART void tell_shown(u32 slot, u32 ask, u32 where, u32 how) {
    const Op1fKindHead* head = head_of(slot, ask);
    if (head == 0) return;
    u32 shown = (u32)head->shown;
    const Op1fCardSlot* in = &op1f_cards.slots[slot];
    if (shown == 0 || shown < in->first || shown >= in->end || (shown & 1u) != 0) return;
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_SHOWN, slot, &kept);
    head->shown(where, how);
    op1f_cards_call_out(OP1F_CALL_SHOWN, &kept);
}

// Once a frame, after the page's work and before the cards' frame functions: what the stand-in's `shown` would be
// told now (OP1F_SHOWN_UP and the sequencer unit's bits while the stand-in is the page on the screen, that is while
// the frame's `page` names its card for it; 0 otherwise), told where it changed; and the word of the sequencer's
// notice.
// R4c: the word stays 0, so that the firmware's render loop never draws the notice under 77 (the base draws it over
// the page, popups_frame); whether the stand-in is the page on the screen is the base's own flag, stand_on_screen.
static OP1F_APART void stand_told(u32 screen) {
    u32 up = stand.on == (u32)STAND_TOP && !top.shown && page_on_screen(screen) == stand.slot + 1u ? 1u : 0u;
    u32 how = up ? (u32)OP1F_SHOWN_UP | (stand.how & (u32)(OP1F_SHOWN_OFF | OP1F_SHOWN_COVERED)) : 0u;
    stand_on_screen = up;
    op1f_cards_stand_in_shown = 0;
    u32 same = stand.told_slot == stand.slot && stand.told_ask == stand.ask && stand.told_where == stand.where ? 1u : 0u;
    if (stand.told != 0 && (how == 0 || !same)) {
        stand.told = 0;
        tell_shown(stand.told_slot, stand.told_ask, stand.told_where, 0);
    }
    if (how == 0 || how == stand.told) return;
    stand.told = how;
    stand.told_slot = stand.slot;
    stand.told_ask = stand.ask;
    stand.told_where = stand.where;
    tell_shown(stand.slot, stand.ask, stand.where, how);
}

// R5: the base's words under an effect's name on the blank page, while its place passes the sound dry: for lack of
// memory, or for its cost. Inside the frame call's record paint_name holds; measured first, as the name is.
static OP1F_APART void paint_words(void* canvas) {
    const char* words = (stand.how & (u32)OP1F_STAND_NO_ROOM) != 0 ? "no room" : (stand.how & (u32)OP1F_STAND_OVER) != 0 ? "switched off" : 0;
    if (words == 0) return;
    int w = op1f_paint_text(0, 0, 0, words, NAME_COLOUR, (int)WORDS_SCALE);
    op1f_paint_text(canvas, ((int)PICTURE_WIDE - w) >> 1, (int)WORDS_Y, words, NAME_COLOUR, (int)WORDS_SCALE);
}

// The ask's name on the blank page, painted by the base at the top centre in its own font, while the card has set no
// document for it. The paint helpers paint only for a card's frame call, into this frame's canvas: the frame call's
// record names the card for the instant of the painting, and nothing of the pack's runs inside it.
static OP1F_APART void paint_name(void* canvas) {
    const Op1fKindHead* head = head_of(stand.slot, stand.ask);
    if (canvas == 0 || head == 0) return;
    char name[OP1F_KIND_NAME];
    u32 n = 0;
    for (; n + 1u < (u32)OP1F_KIND_NAME && head->name[n] != 0; ++n) name[n] = head->name[n];
    name[n] = 0;
    int wide = op1f_paint_text(0, 0, 0, name, NAME_COLOUR, (int)NAME_SCALE);         // measured: a canvas of 0 paints nothing and is not counted
    state.calling_sp = stack_here();
    state.calling = stand.slot + 1u;
    op1f_paint_text(canvas, ((int)PICTURE_WIDE - wide) >> 1, (int)NAME_Y, name, NAME_COLOUR, (int)NAME_SCALE);
    if (fx.placed) paint_words(canvas);                                             // R5
    state.calling = 0;
}

// ---------------------------------------------------------------- keys and knobs

// An event offered: first to the pack whose page is shown, then to every card with an input function in the list's
// order, until one takes it. The slot + 1 of the pack that took it, or 0.
// R1: what a card's input function answered, as the offer takes it: 1 taken. 0 not taken: an answer of 0, and
// OP1F_SEEN from a card of level 4, which for a down records the card as a watcher of the key (told its up), with
// interrupts masked for the instant; a turn has no up. From a card of level 3 or less any answer not 0 takes it.
// R3: a card's input function called inside a record of its kind, for op1f_cards_inside.
static inline __attribute__((always_inline)) int input_call(u32 i, const Op1fCard* card, Op1fInput* e) {
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_INPUT, i, &kept);
    int answer = card->input(e);
    op1f_cards_call_out(OP1F_CALL_INPUT, &kept);
    return answer;
}

static inline __attribute__((always_inline)) int taken(u32 i, Op1fInput* e) {
    const Op1fCard* card = op1f_cards.slots[i].card;
    int answer = input_call(i, card, e);                                            // R3: inside a record
    if (answer == 0) return 0;
    if (answer != (int)OP1F_SEEN || !level_4(card)) return 1;
    if (e->what != (u32)OP1F_TURN && e->key < (u32)HELD_KEYS) {
        u32 mask;
        MASK_OFF(mask);
        watch.keys[e->key] = watch.keys[e->key] | (1u << i);
        watch.down[i] = watch.down[i] + 1u;
        MASK_ON(mask);
    }
    return 0;
}

static OP1F_APART u32 offer(Op1fInput* e) {
    u32 page = page_on_screen(e->screen);
    if (page != 0 && ((state.inputs >> (page - 1)) & 1u) != 0) {
        e->mine = 1;
        if (taken(page - 1, e)) return page;
    }
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.inputs >> i) & 1u) == 0 || i + 1 == page) continue;
        e->mine = 0;
        if (taken(i, e)) return i + 1;
    }
    return 0;
}

// R1: a key's up told to every card that watched its down, in the list's order, what it answers not looked at; the
// bits taken with interrupts masked. The card that took the down, if one did, is told as before, after these.
static inline __attribute__((always_inline)) void tell_watchers(u32 key, u32 who) {
    u32 mask, bits;
    MASK_OFF(mask);
    bits = watch.keys[key];
    watch.keys[key] = 0;
    MASK_ON(mask);
    for (u32 i = 0; bits != 0 && i < state.seen; ++i) {
        if (((bits >> i) & 1u) == 0 || i + 1 == who || ((state.inputs >> i) & 1u) == 0) continue;
        Op1fInput e;
        event_of(&e, OP1F_UP, key, 0, 0);
        e.mine = page_on_screen(e.screen) == i + 1 ? 1u : 0u;
        input_call(i, op1f_cards.slots[i].card, &e);                                // R3: inside a record
        count(&watch.up[i]);
    }
}

static void event_of(Op1fInput* e, u32 what, u32 key, int turn, const u32* record) {
    e->bytes = (u32)sizeof(Op1fInput);
    e->what = what;
    e->key = key;
    e->turn = turn;
    e->mode = RD32(CURRENT_MODE);
    e->screen = RD32(CURRENT_SCREEN);
    e->shift = STOCK_SHIFT() != 0 ? 1u : 0u;
    e->mine = 0;
    e->record = record;
    e->thread = what == (u32)OP1F_TURN || key < (u32)KEYS_OF_NOTES_FIRST || key > (u32)KEYS_OF_NOTES_LAST ? (u32)OP1F_KEY_THREAD : 0u;   // R1: by the key's number
}

// From our call at the two key-down functions (M2's input stub): the firmware's record, and whether it came
// through the shifted one. Not 0: a pack took the key, and the firmware never sees it.
extern "C" int op1f_input_down(const u32* record, int shifted) OP1F_NAMED("op1f_input_down");
extern "C" int op1f_input_down(const u32* record, int shifted) {
    if (record == 0) return 0;
    u32 key = record[0];
    input.last_down = key;
    input.last_down_ms = state.ms;
    count(&input.down);                                                             // M3: masked for the instant
    if (top_input(shifted ? (u32)OP1F_DOWN_SHIFT : (u32)OP1F_DOWN, key, 0, record)) return 1;   // S1: the base's top page
    if (&op1f_options_down != 0 && op1f_options_down(key)) return 1;               // R8 (base 32): T1 back to the list of packs
    if (!checked() || state.inputs == 0) return 0;
    Op1fInput e;
    event_of(&e, shifted ? (u32)OP1F_DOWN_SHIFT : (u32)OP1F_DOWN, key, 0, record);
    u32 who = offer(&e);
    if (who == 0) return 0;
    if (key < (u32)HELD_KEYS) input.held[key] = (u8)who;
    count(&input.took_down[who - 1]);
    count(&input.taken);
    return 1;
}

// From our call at the key-up function: the key's number. An up goes where its down went.
extern "C" int op1f_input_up(int key) OP1F_NAMED("op1f_input_up");
extern "C" int op1f_input_up(int key) {
    input.last_up = (u32)key;
    input.last_up_ms = state.ms;
    count(&input.up);                                                               // M3: masked for the instant
    if (top_input(OP1F_UP, (u32)key, 0, 0)) return 1;                               // S1: the base's top page took its down
    if ((u32)key >= (u32)HELD_KEYS || !checked()) return 0;
    u32 who = input.held[key];
    tell_watchers((u32)key, who);                                                   // R1: those that watched its down first
    if (who == 0) return 0;                                                         // the firmware had the down
    input.held[key] = 0;
    if (who <= state.seen && ((state.inputs >> (who - 1)) & 1u) != 0) {
        Op1fInput e;
        event_of(&e, OP1F_UP, (u32)key, 0, 0);
        e.mine = page_on_screen(e.screen) == who ? 1u : 0u;
        input_call(who - 1, op1f_cards.slots[who - 1].card, &e);                    // told; what it answers does not matter (R3: inside a record)
    }
    count(&input.took_up[who - 1]);
    count(&input.taken);
    return 1;
}

// From our call in the turn function, after the firmware's own bookkeeping: the knob (0 to 3) and the turn.
extern "C" int op1f_input_turn(int knob, int turn) OP1F_NAMED("op1f_input_turn");
extern "C" int op1f_input_turn(int knob, int turn) {
    count(&input.turns);                                                            // M3: masked for the instant
    if (top_input(OP1F_TURN, (u32)knob, turn, 0)) return 1;                         // S1: the base's top page
    if (!checked() || state.inputs == 0) return 0;
    Op1fInput e;
    event_of(&e, OP1F_TURN, (u32)knob, turn, 0);
    u32 who = offer(&e);
    if (who == 0) return 0;
    count(&input.took_turn[who - 1]);
    count(&input.taken);
    return 1;
}

// ---------------------------------------------------------------- R4c: the unit's own popups over a pack's page

// A byte of one of the firmware's screens, as the firmware's own loop left it in this frame.
static inline u32 byte_of(const u32* object, u32 at) {
    return ((const volatile u8*)object)[at];
}

// The first screen above 77 the firmware's render loop drew in this frame, or 0: the loop sets every screen's byte
// +0x10 to its answer of the screen's `is`, every frame, whether it draws it or not.
static OP1F_APART u32 drawn_above(const u32* screens) {
    for (u32 n = (u32)ABOVE_FIRST; n <= (u32)LAST_SCREEN; ++n) {
        const u32* object = (const u32*)screens[n];
        if (object != 0 && byte_of(object, SCREEN_DRAWN) != 0) return n;
    }
    return 0;
}

// One of the unit's own popups drawn by its own render on its own object, as the loop calls it (the screen, and a
// time the render does not read: 0), once it is held to be what was read: the object there, its table, the render in
// the table, a document. 1 when drawn; else 0, and said once for that popup (`which`: 0 the notice, 1 the memory's).
static OP1F_APART u32 popup_draw(const u32* screens, u32 number, u32 table, u32 render, u32 which) {
    u32* object = (u32*)screens[number];
    const char* why = 0;
    if (object == 0) why = "no screen of the firmware's is there";
    else if (object[0] != table) why = "its table is not the one read";
    else if (((const u32*)table)[3] != render) why = "its render is not the one read";
    else if (object[SCREEN_DOCUMENT] == 0) why = "it has no document";
    if (why == 0) {
        ((RenderFn)render)(object, 0u);
        return 1;
    }
    if (!popups.refused[which]) {
        popups.refused[which] = 1;
        STOCK_LOG(1, LOG_FILE, 37, "page: the unit's own popup of screen %u is not drawn over a pack's page: %s", number, why);
    }
    return 0;
}

// The notice's line when it begins to be drawn over a pack's page, for the first sixteen times.
// R6b: the notice alone; the popups the loop hands over say theirs in held_draw.
static OP1F_APART void popup_said(u32 which, u32 on, u32 page) {
    u32 was = popups.on[which];
    popups.on[which] = on;
    if (on == 0 || was != 0) return;
    ++popups.began[which];
    if (popups.said[which] >= (u32)POPUP_SAID) return;
    ++popups.said[which];
    STOCK_LOG(3, LOG_FILE, 35, "page: the unit's own notice drawn over the page of '%s', the page darkened under it; %u time(s) since the start",
              op1f_cards.slots[page - 1u].card->name, popups.began[0]);
}

// R6b: the render of one of the unit's own screens, handed over by the firmware's render loop at its call of the render
// (base 29's popups stub), which calls this only while the screen the GUI shows is 77: a page of the base's or of
// a pack's, which the loop draws after every screen up to 76 and the packs paint after the loop. The render of a
// screen from 60 to 85 but 77 is kept, with the time the loop handed it, for the frame call of this frame to make after
// the pages are painted (held_draw); any other render (77, or anything before the base's first frame) is made at once,
// as the loop would. On the display thread, inside the loop: nothing is said, nothing waits, nothing of a pack's runs.
// A list that a loop kept and no frame call made (not expected: the frame call follows the loop in every frame) is
// dropped and counted at the next loop.
// R12 (base 36): in a held frame whose plan the frame call made for this loop, over the same document: each rectangle
// drawn by 77's render as the loop handed it, cut to the rectangle by the rasteriser's clip, with the plan's parts
// hidden; the clip and the display bytes put back right after each drawing, each drawing timed. Inlined, so that the
// loop's function calls nothing but the render; nothing is kept in a register across a render (the rectangle, the
// render and its arguments are the unit's state, each rectangle's clip and hides laid out by the frame call in
// parts_lay, the rasteriser's struct read again after each drawing), so that its frame stays near base 33's.
static inline __attribute__((always_inline)) u32 parts_before() {
    u32 at = parts.at;
    if (at >= parts.draws) return 0;
    const PartsDraw* d = &parts.draw[at];
    u8* const* hide = d->hide;
    u32 hides = d->hides;
    for (u32 i = 0; i < hides; ++i) {
        u8* node = hide[i];
        parts.saved[i] = node[NODE_DISPLAY];
        node[NODE_DISPLAY] = 0;
    }
    volatile u32* clip = (volatile u32*)parts.raster;
    for (u32 k = 0; k < 4u; ++k) clip[4u + k] = d->clip[k];
    parts.before = stamp();
    return 1;
}
static inline __attribute__((always_inline)) void parts_after() {
    u32 took = stamp() - parts.before;
    volatile u32* clip = (volatile u32*)parts.raster;
    clip[4] = 0u;
    clip[5] = 0u;
    clip[6] = (u32)PARTS_TALL;
    clip[7] = (u32)PARTS_WIDE;
    u32 at = parts.at;
    const PartsDraw* d = &parts.draw[at];
    u8* const* hide = d->hide;
    u32 hides = d->hides;
    for (u32 i = hides; i-- > 0;) hide[i][NODE_DISPLAY] = parts.saved[i];   // the other way: a part twice gets its first byte back
    if (took > LONGEST) took = LONGEST;
    took = ((took >> 4) * 41u) >> 10;                   // microseconds, as microseconds() counts them
    ++parts.drawn;
    parts.hides += hides;
    parts.sum += took;
    if (took > parts.most) parts.most = took;
    parts.at = at + 1u;
}
static inline __attribute__((always_inline)) void parts_loop(void* screen, u32 time, u32 render, u32 frame) {
    if (parts.ready != frame + 1u || ((const u32*)screen)[SCREEN_DOCUMENT] != parts.document) return;
    parts.ready = 0;                                    // once
    parts.screen = (u32)screen;
    parts.time = time;
    parts.render = render;
    parts.raster = PARTS_RASTER;
    parts.at = 0;
    while (parts_before()) {
        ((RenderFn)parts.render)((void*)parts.screen, parts.time);
        parts_after();
    }
}

extern "C" void op1f_popups_loop(void* screen, u32 time, u32 render) OP1F_NAMED("op1f_popups_loop");
// R9 (base 33): 77's render is left out in a frame whose clearing op1f_fill_place left out (op1f_fill_left = this
// loop's frame counter + 1): the page holds its picture, and the frame call is told (render_left).
extern "C" void op1f_popups_loop(void* screen, u32 time, u32 render) {
    u32 frame = op1f_frame_counter;
    if (held.frame != frame) {
        held.lost += held.count;
        held.count = 0;
        held.frame = frame;
    }
    const u32* screens = held.screens;
    if (op1f_hold_left(op1f_fill_left, frame + 1u) && screens != 0 && screens[PAGE_SCREEN] == (u32)screen) {
        hold.render_left = frame + 1u;                      // R9 (base 33): the page holds its picture in this frame
        parts_loop(screen, time, render, frame);            // R12 (base 36): the rectangles the page named, drawn again
        return;
    }
    u32 n = held.count;
    if (screens != 0 && n < (u32)HELD_MOST) {
        for (u32 number = (u32)HELD_FIRST; number <= (u32)LAST_SCREEN; ++number) {
            if (screens[number] != (u32)screen) continue;
            if (number == (u32)PAGE_SCREEN) break;
            HeldRender* h = &held.list[n];
            h->screen = screen;
            h->time = time;
            h->render = render;
            h->number = number;
            held.count = n + 1u;
            return;
        }
    }
    ((RenderFn)render)(screen, time);
}

// R6b: the renders the loop handed over in this frame, made now, after every card's frame function and the notice, in
// the loop's order, each with the time the loop handed it: once a frame, as the loop would have made them. Each
// popup's line when it begins to be made over a page (`page`: the slot + 1 of the pack's page on the screen, 0 for a
// page of the base's own), for its first eight times. 2 when one was made.
static OP1F_APART u32 held_draw(u32 page) {
    u32 n = held.count, on = 0;
    held.count = 0;
    if (n > (u32)HELD_MOST) n = HELD_MOST;
    for (u32 i = 0; i < n; ++i) {
        const HeldRender* h = &held.list[i];
        ((RenderFn)h->render)(h->screen, h->time);
        on |= 1u << (h->number - (u32)HELD_FIRST);
    }
    u32 began = on & ~held.on;
    held.on = on;
    for (u32 k = 0; began != 0 && k < (u32)HELD_MOST; ++k) {
        if (((began >> k) & 1u) == 0) continue;
        ++held.began[k];
        if (held.said[k] >= (u32)HELD_SAID) continue;
        ++held.said[k];
        STOCK_LOG(3, LOG_FILE, 36, "page: the unit's own popup of screen %u drawn over the page of '%s'; %u time(s) since the start", k + (u32)HELD_FIRST,
                  page != 0 ? op1f_cards.slots[page - 1u].card->name : "base", held.began[k]);
    }
    return n != 0 ? 2u : 0u;
}

// R6b: the loop's renders kept for after the pages, made (held_draw) and timed: from popups_frame, last, with `counted`
// not 0 when the notice was drawn in this frame and counted already, or from the frame call's early return with 0. A
// frame is counted once in the popups' cost; the time of the renders is added to it.
// R9 (base 33): a popup made or the notice drawn in this frame asks the next three frames whole.
static OP1F_APART void held_frame(u32 page, u32 counted) {
    u32 start = stamp();
    u32 made = held_draw(page);
    if (made != 0 || counted != 0) hold.covered = 1;
    if (made == 0) return;
    u32 spent = microseconds(stamp() - start);
    popups.sum = popups.sum + spent;
    if (counted != 0) return;
    if (popups.calls == 0 || spent < popups.least) popups.least = spent;
    if (spent > popups.most) popups.most = spent;
    popups.calls = popups.calls + 1u;
}

// R6d: the name the entry kept ready for an engine away stands for, and under it the words '<name>' is not on this
// unit, as base 29's page said them: `name` holds OP1F_KIND_NAME bytes, `said` sizeof AWAY_SAID.
static const char AWAY_TAIL[] = "' is not on this unit";
enum { AWAY_SAID = OP1F_KIND_NAME + sizeof AWAY_TAIL + 1 };
static OP1F_APART void away_words(char* name, char* said) {
    u32 n = 0;
    for (; n + 1u < (u32)OP1F_KIND_NAME && fx.ahead_name[n] != 0; ++n) name[n] = fx.ahead_name[n];
    name[n] = 0;
    u32 w = 0;
    said[w++] = '\'';
    for (u32 i = 0; i < n; ++i) said[w++] = name[i];
    for (u32 i = 0; AWAY_TAIL[i] != 0; ++i) said[w++] = AWAY_TAIL[i];
    said[w] = 0;
}

// R6d: the name and the words set into the two placeholders of the base's own page for an engine away, centred, in
// the unit's own letters, whenever the name the entry kept ready stands for is not the one last set, while that entry
// is the engines unit's answer for an engine away and its document the page of away.svg. Called each frame before the
// page could be drawn, so the words stand before its first frame, and follow another synth sound whose engine is
// away. On the display thread, in the frame work, where the page is drawn; the text unit lays a text out only when it
// changed. `lettered`: both placeholders took their words; where one did not, the base's own font stands in.
static OP1F_APART void away_letters() {
    if (!fx.ready || !fx.ready_away || !fx.ahead_away || fx.ahead_name == 0) return;
    if (awaypage.document == 0 || fx.ready_document != awaypage.document) return;
    char name[OP1F_KIND_NAME];
    char said[AWAY_SAID];
    away_words(name, said);
    u32 k = 0;
    while (name[k] != 0 && name[k] == awaypage.name[k]) ++k;
    if (awaypage.set && name[k] == awaypage.name[k]) return;                       // the same name: its words stand
    int took_name = op1f_text_base_set(awaypage.document, AWAY_NAME_PART, name, (int)AWAY_WIDTH, OP1F_CENTRE);
    int took_words = op1f_text_base_set(awaypage.document, AWAY_WORDS_PART, said, (int)AWAY_WIDTH, OP1F_CENTRE);
    awaypage.lettered = took_name != 0 && took_words != 0 ? 1u : 0u;
    for (u32 i = 0; i < (u32)OP1F_KIND_NAME; ++i) awaypage.name[i] = name[i];
    awaypage.set = 1;
}

// R6b: the base's own page for a synth sound whose engine's pack is not on the unit (an entry kept ready of an engine
// away: ahead_of, ready_frame), while it is the page on the screen: 77 shown with its document under 77, no pack's
// page shown, no stand-in, not under the base's top page. The engine's name at the top as an ask's name is painted on
// the blank page, and under it the words '<name>' is not on this unit, as `no room` is painted; line 41 when it comes
// on the screen, for the first sixteen times. Painted by the base itself (op1f_paint_text_base) into this frame's
// canvas, before the cards' frame functions, as the stand-in's name is.
static OP1F_APART void away_paint(u32 screen) {
    away_letters();                                                                 // R6d: the words in the unit's own letters, where they changed
    u32 up = fx.ready && fx.ready_away && fx.ahead_away && fx.held == (u32)OP1F_STAND_ENGINE + 1u && screen == (u32)PAGE_SCREEN && state.shown == 0 &&
             stand.on == (u32)STAND_NONE && !top.shown && state.screen != 0 && (void*)state.screen[SCREEN_DOCUMENT] == fx.ready_document && fx.ahead_name != 0 ? 1u : 0u;
    u32 was = fx.away_on;
    fx.away_on = up;
    if (!up) return;
    char name[OP1F_KIND_NAME];
    char said[AWAY_SAID];
    away_words(name, said);                                                         // R6d: one helper makes them, for the letters and for this
    if (awaypage.lettered == 0 || fx.ready_document != awaypage.document) {         // R6d: the base's own font only where the unit's letters are not set
        void* canvas = (void*)RD32(FRAME_BUFFER);
        int span = op1f_paint_text_base(0, 0, 0, name, NAME_COLOUR, (int)NAME_SCALE);    // measured: a canvas of 0 paints nothing
        op1f_paint_text_base(canvas, ((int)PICTURE_WIDE - span) >> 1, (int)NAME_Y, name, NAME_COLOUR, (int)NAME_SCALE);
        span = op1f_paint_text_base(0, 0, 0, said, NAME_COLOUR, (int)WORDS_SCALE);
        op1f_paint_text_base(canvas, ((int)PICTURE_WIDE - span) >> 1, (int)WORDS_Y, said, NAME_COLOUR, (int)WORDS_SCALE);
    }
    if (was != 0 || fx.away_said >= (u32)STAND_SAID) return;
    ++fx.away_said;
    STOCK_LOG(3, LOG_FILE, 41, "page: the base's own page says '%s' is not on this unit, for synth %u", name, (fx.ahead_where & 7u) + 1u);
}

// Once a frame, after every card's frame function, with the canvas withdrawn (`page`: the slot + 1 whose page is on
// the screen, as the frame's `page` says): the notice (59) while the stand-in is the page on the screen and the
// sequencer unit says the notice lies over it; then the memory's notice (65) while any pack's page is on the screen
// and the loop drew that popup in this frame, under 77. In the order of their numbers, as the loop draws them; never
// under the base's top page, nor in a frame in which the firmware drew a screen of its own above 77 (the countdown to
// switch off first). Timed: the frames in which the base drew one, and what the drawing took.
// R6b: the memory's notice is the loop's now; then every render the loop handed over in this frame, whatever the page
// (held_frame): the loop's own, each made once, in the order of their numbers, after the notice.
static OP1F_APART void popups_frame(const Op1fFrame* now, u32 page) {
    u32 notice = stand_on_screen != 0 && (stand.how & (u32)OP1F_SHOWN_COVERED) != 0 ? 1u : 0u;
    u32 drawn = 0;
    if ((notice != 0 || page != 0) && !top.shown && now->popup == 0) {
        u32* screens = STOCK_SCREENS();
        if (screens != 0 && drawn_above(screens) == 0) {
            u32 before = stamp();
            if (notice != 0 && popup_draw(screens, NOTICE_SCREEN, STOCK_NOTICE_TABLE, STOCK_NOTICE_RENDER, 0)) drawn |= 1u;
            if (drawn != 0) {
                u32 took = microseconds(stamp() - before);
                if (popups.calls == 0 || took < popups.least) popups.least = took;
                if (took > popups.most) popups.most = took;
                ++popups.calls;
                popups.sum += took;
            }
        }
    }
    popup_said(0, drawn & 1u, page);
    held_frame(page, drawn);                                // R6b: the loop's renders kept for after the pages
}

// ---------------------------------------------------------------- R9 (base 33): a page that holds its picture

// What a frame function is handed on base 33: base 32's forty bytes and two words behind them. A pack reads what its
// `bytes` covers (module/build_r6e_card.h): `held` 1 when this frame's loop left out both the clearing and 77's
// render, the canvas then holding the picture of two frames before; `picture` 0 or 1, which of the display's two
// pictures the canvas is (0 the first the base saw in the frame's word, 1 the other: they alternate frame by frame).
struct Handed {
    Op1fFrame now;                                      // +0, base 32's
    u32 held, picture;                                  // +40, +44
};
static_assert(sizeof(Op1fFrame) == 40 && sizeof(Handed) == 48, "base 33 hands a frame function 48 bytes, base 32's forty and two words");

// R9: the words of this frame for its frame functions: `held` from the loop's two marks, `picture` from the canvas
// (the frame's word, as read for `canvas`).
static OP1F_APART void hold_words(Handed* h, u32 frame, u32 canvas) {
    h->held = op1f_hold_held(op1f_fill_left, hold.render_left, frame);
    h->picture = op1f_hold_picture(&hold.first, canvas);
}

// R9: from the frame call, before any frame function: what this frame's calls may ask is cleared, and the page they
// may ask for is known (`page`: the slot + 1 of the pack's page this frame names, 0 none).
static inline void hold_open(u32 page) {
    hold.page = page;
    hold.asked = 0;
    hold.redraw = 0;
    parts.asked = 0;                                    // R12 (base 36): and the parts it may name
    parts.refused = 0;
    parts.own = 0;
    parts_plan.rects = 0;
    parts_plan.parts = 0;
}

// R12 (base 36): a plan made, laid out as the next loop draws it: each rectangle's clip words and its run of nodes to
// hide, so that the loop reads nothing of the plan and keeps nothing in a register across a render.
static OP1F_APART void parts_lay() {
    u32 n = parts_plan.rects;
    for (u32 i = 0; i < n; ++i) {
        const Op1fPlanRect* r = &parts_plan.rect[i];
        PartsDraw* d = &parts.draw[i];
        d->clip[0] = r->y0;
        d->clip[1] = (u32)PARTS_WIDE - r->x1;
        d->clip[2] = r->y1;
        d->clip[3] = (u32)PARTS_WIDE - r->x0;
        d->hide = (u8* const*)&parts_plan.hide[r->hide];
        d->hides = r->hides;
    }
    parts.draws = n;
}

// R12 (base 36): from hold_frame, behind the rule's decision `out` for the next loop (`page`, `held_now` as there;
// `asks`: the page asked op1f_page_parts in this frame; `document`: the one under 77). When the rule keeps the picture
// and the page asked with a rectangle named, the plan for the next loop, made here and never in the loop; a plan that
// fails holds nothing (op1f_hold_screen 0: the next picture whole), counted. Line 50 once a second while a page asks,
// for the first 30 seconds of each of the first sixteen runs of asking.
static OP1F_APART void parts_frame(u32 page, u32 asks, u32 out, u32 document, u32 held_now) {
    parts.ready = 0;
    u32 failed = 0;
    if (asks != 0 && (out & (u32)OP1F_HOLD_KEEP) != 0 && parts_plan.rects != 0) {
        if (op1f_plan_parts(&parts_plan, (Op1fPlanNode)document) == (unsigned)OP1F_PLAN_MADE) {
            parts_lay();
            parts.document = document;
            parts.ready = op1f_frame_counter + 1u;      // the next loop's counter + 1
        } else {
            op1f_hold_screen = 0;
            failed = 1;
        }
    }
    if (asks == 0) {
        parts.page = 0;
        return;
    }
    if (page != parts.page) {                           // a run of asking begins
        parts.page = page;
        parts.start = state.ms;
        parts.next = (u32)PARTS_SECOND;
        parts.second = 1;
        ++parts.runs;
        parts.saying = parts.said < (u32)PARTS_SAID ? 1u : 0u;
        if (parts.saying != 0) ++parts.said;
        parts.frames = parts.held = parts.drawn = parts.hides = parts.sum = parts.most = parts.refusals = parts.failures = 0;
    }
    ++parts.frames;
    if (held_now != 0) ++parts.held;
    parts.refusals += parts.refused;
    parts.failures += failed;
    if (parts.saying == 0 || state.ms - parts.start < parts.next) return;
    STOCK_LOG(3, LOG_FILE, 50, "page: '%s' parts, second %u: %u frames, %u held, %u rectangles drawn, %u hidden, %u us in all, %u most; %u refused, %u plans failed",
              op1f_cards.slots[page - 1u].card->name, parts.second, parts.frames, parts.held, parts.drawn, parts.hides, parts.sum, parts.most,
              parts.refusals, parts.failures);
    parts.frames = parts.held = parts.drawn = parts.hides = parts.sum = parts.most = parts.refusals = parts.failures = 0;
    parts.next += (u32)PARTS_SECOND;
    ++parts.second;
    if (parts.next > (u32)PARTS_SAYING) parts.saying = 0;
}

// R9: the frame call's decision for the next loop, after every frame function, the popups and the notice (`page`: the
// slot + 1 of the pack's page this frame named, 0 none; `held`: this frame's `held`; `over`: something of the base's
// own lay over the page in this frame: its top page, the power popup, the ask's name it paints on a stand-in's blank
// page). 77's object goes into op1f_hold_screen when the rule keeps the picture, else 0. Lines 46 and 47 when a hold
// begins and ends, for the first sixteen holds.
static OP1F_APART void hold_frame(u32 page, u32 held_now, u32 over) {
    const u32* screens = held.screens;
    u32 s77 = screens != 0 ? screens[PAGE_SCREEN] : 0u;
    Op1fHoldSeen seen;
    seen.page = page;
    seen.document = s77 != 0 ? ((const u32*)s77)[SCREEN_DOCUMENT] : 0u;
    seen.asked = page != 0 && hold.asked == page ? 1u : 0u;
    u32 changes = hold.changes;
    u32 asks = page != 0 && parts.asked == page ? 1u : 0u;                      // R12 (base 36): the page asked its parts
    u32 other = changes - hold.changes_seen - (asks != 0 ? parts.own : 0u);    // R12: then its own changes are not counted
    u32 refused = parts.refused + (asks == 0 && parts_plan.rects != 0 ? 1u : 0u);   // R12: rectangles refused, or named with no ask
    parts.refused = refused;
    seen.whole = hold.redraw != 0 || hold.covered != 0 || over != 0 || other != 0 || refused != 0 ? 1u : 0u;
    seen.held = held_now;
    hold.changes_seen = changes;
    hold.covered = 0;
    hold.page = 0;
    u32 out = op1f_hold_decide(&hold.rule, &seen);
    op1f_hold_screen = (out & (u32)OP1F_HOLD_KEEP) != 0 ? s77 : 0u;
    parts_frame(page, asks, out, seen.document, held_now);                     // R12: the plan for the next loop, and line 50
    if ((out & (u32)OP1F_HOLD_ENDED) != 0 && hold.slot != 0) {
        STOCK_LOG(3, LOG_FILE, 47, "page: '%s' stopped holding its picture: %u frame(s) held, %u drawn whole since it began",
                  op1f_cards.slots[hold.slot - 1u].card->name, hold.rule.held, hold.rule.drawn);
        hold.slot = 0;
    }
    if ((out & (u32)OP1F_HOLD_BEGAN) != 0) {
        ++hold.began;
        if (hold.said >= (u32)HOLD_SAID) return;
        ++hold.said;
        hold.slot = page;
        STOCK_LOG(3, LOG_FILE, 46, "page: '%s' holds its picture from frame to frame; %u time(s) since the start",
                  op1f_cards.slots[page - 1u].card->name, hold.began);
    }
}

// ---------------------------------------------------------------- the frame call

static OP1F_APART void clear_measures() {
    for (u32 i = 0; i < (u32)MOST; ++i) {
        state.measure[i].calls = 0;
        state.measure[i].sum = 0;
        state.measure[i].least = 0xffffffffu;
        state.measure[i].most = 0;
    }
}

// The lines of the frame calls: one a card with a frame function, by its pack's name; the input's, where a card has
// an input function; and how long the gate was shut.
static OP1F_APART void say(int why) {
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.framed >> i) & 1u) == 0) continue;
        const Measure* m = &state.measure[i];
        STOCK_LOG(3, LOG_FILE, 4, "cards: '%s' frame call: %u calls, %u us in all, %u least, %u most", op1f_cards.slots[i].card->name, m->calls, m->sum, m->least, m->most);
    }
    if (state.inputs != 0) {
        u32 down = input.down, up = input.up, turns = input.turns, taken = input.taken;
        STOCK_LOG(3, LOG_FILE, 10, "input: %u down, %u up, %u turns this period, %u taken by packs; since the start %u down, %u up, %u turns, %u taken; %u pages opened",
                  down - state.seen_down, up - state.seen_up, turns - state.seen_turns, taken - state.seen_taken, down, up, turns, taken, state.opened);
        for (u32 i = 0; i < state.seen; ++i) {
            if (((state.inputs >> i) & 1u) == 0) continue;
            u32 all = input.took_down[i] + input.took_up[i] + input.took_turn[i];
            if (all == state.seen_took[i]) continue;                                // nothing taken by this pack this period
            STOCK_LOG(3, LOG_FILE, 11, "input: '%s' took %u down, %u up, %u turns since the start",
                      op1f_cards.slots[i].card->name, input.took_down[i], input.took_up[i], input.took_turn[i]);
        }
    }
    if (state.quiet.silent_before)
        STOCK_LOG(3, LOG_FILE, 5, "cards quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  state.quiet.silent_before, quiet_why(why), state.quiet.spoken, state.quiet.skipped);
    if (popups.calls != 0)                                                          // R4c: what drawing the unit's own popups cost
        STOCK_LOG(3, LOG_FILE, 38, "page: the unit's own popups drawn by the base in %u frame(s) this period: %u us in all, %u least, %u most",
                  popups.calls, popups.sum, popups.least, popups.most);
}

// R1: the paint unit's counts of each card with a frame function, taken at the end of a period and kept for its line;
// whether one of them refused a call. Nothing is taken where the paint unit is away.
static struct {
    u32 painted[MOST], refused[MOST];
} paint;

static OP1F_APART u32 paint_taken() {
    u32 any = 0;
    for (u32 i = 0; i < state.seen; ++i) {
        u32 painted = 0, refused = 0;
        if (((state.framed >> i) & 1u) != 0 && &op1f_paint_take != 0) op1f_paint_take(i, &painted, &refused);
        paint.painted[i] = painted;
        paint.refused[i] = refused;
        any |= refused;
    }
    return any;
}

// R1: the lines of level 4, behind the others: a card that watched keys in the period, and, at the end of a period,
// a card whose painting was refused in it.
static OP1F_APART void say_level_4(int period) {
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.used >> i) & 1u) == 0) continue;
        if (watch.down[i] + watch.up[i] != watch.seen[i])
            STOCK_LOG(3, LOG_FILE, 21, "input: '%s' watched %u down, told %u up since the start", op1f_cards.slots[i].card->name, watch.down[i], watch.up[i]);
        if (period && paint.refused[i] != 0)
            STOCK_LOG(3, LOG_FILE, 22, "paint: '%s' painted %u time(s) this period, %u refused (not from its frame function, or not this frame's picture)",
                      op1f_cards.slots[i].card->name, paint.painted[i], paint.refused[i]);
    }
}

// What the input's lines say was said: kept at every period, printed or not.
static OP1F_APART void input_seen() {
    state.seen_down = input.down;
    state.seen_up = input.up;
    state.seen_turns = input.turns;
    state.seen_taken = input.taken;
    for (u32 i = 0; i < state.seen; ++i) state.seen_took[i] = input.took_down[i] + input.took_up[i] + input.took_turn[i];
}

// From the measuring unit's frame function, once a frame, after our own features' frame work and before the shared
// screens are dressed: the clock, the page, then each used card's frame function in the list's order.
extern "C" void op1f_cards_frame() OP1F_NAMED("op1f_cards_frame");
static inline void places_ready();                          // R4: pictures that take a place, below
static OP1F_APART void said_passed();
extern "C" void op1f_cards_frame() {
    places_ready();                                         // R4: the check, and the places after it
    u32 cycles = stamp();
    if (state.started) {
        state.cycle_acc += cycles - state.last_cycles;
        while (state.cycle_acc >= (u32)CYCLES_PER_MS) {
            state.cycle_acc -= (u32)CYCLES_PER_MS;
            ++state.ms;
        }
    } else {
        state.started = 1;
        clear_measures();
    }
    state.last_cycles = cycles;
    if (!state.settled) {                                   // M3: the cards' folders settled and made, once, before any frame function
        state.settled = 1;
        op1f_folders_settle();
    }
    op1f_door_frame(state.ms);                              // S1: the door's frame work
    top_frame(op1f_frame_counter, RD32(CURRENT_MODE));      // S1: the base's top page put up, kept, taken off
    op1f_seq_frame();                                       // R3: the sequencer unit's frame work, then the notes unit's
    op1f_notes_frame();
    op1f_memory_frame();                                    // R4: the memory unit's frame work, then the stages unit's
    op1f_stages_frame();
    op1f_keeps_frame();                                     // R4b: the values' unit's, before any frame function
    if (&op1f_options_frame != 0) op1f_options_frame();     // R8 (base 32): the options unit's, before any frame function
    if (&op1f_effects_frame != 0) op1f_effects_frame();     // R5: the effects unit's, behind the values' unit's and the memory unit's
    if (&op1f_engines_frame != 0) op1f_engines_frame();     // R6: the engines unit's, behind the effects unit's
    said_passed();                                          // R4: the pictures a card of level 6 carries and a screen was not served
    Op1fStanding standing;
    int stands = op1f_seq_standing(&standing);              // R3: does a pack's page stand in for the sequencer's screen now
    fx.asked = 0;                                           // R5
    if (!stands) stands = effect_standing(&standing);       // R5: else for an effect's screen
    if (state.framed == 0 && state.inputs == 0 && !stands && stand.on == (u32)STAND_NONE && stand.told == 0 && away.slot1 == 0) {   // R3: and no stand-in to place, keep or close (R4c: nor a page set aside)
        op1f_cards_stand_in_shown = 0;
        stand_on_screen = 0;                                                        // R4c
        if (fx.held != 0 || &op1f_effects_ahead != 0 || &op1f_engines_ahead != 0) ready_frame(RD32(CURRENT_MODE), RD32(CURRENT_SCREEN));   // R5: the entry kept ready, in such a frame too (R6: an engine's too)
        away_paint(RD32(CURRENT_SCREEN));                   // R6b: the base's page for an engine away, in such a frame too
        held_frame(0, 0);                                   // R6b: the loop's renders kept for after the pages, in such a frame too
        hold_frame(0, 0, 0);                                // R9 (base 33): no pack's page, nothing held
        return;                                             // no card asks for a call or for keys: nothing of stock is asked, nothing is said
    }
    static Handed handed;                                   // R9 (base 33): base 32's frame and two words behind it; kept out
    Op1fFrame& now = handed.now;                            // of the stack (the display thread's only, once a frame), whose depth to a kept render stays within the firmware's own
    now.bytes = (u32)sizeof(Handed);
    now.frame = op1f_frame_counter;
    now.ms = state.ms;
    now.mode = RD32(CURRENT_MODE);
    now.screen = RD32(CURRENT_SCREEN);
    now.shift = STOCK_SHIFT() != 0 ? 1u : 0u;
    now.popup = RD8(POWER_POPUP_SHOWN) != 0 ? 1u : 0u;
    page_frame(now.frame, now.mode);
    stand_frame(now.frame, now.mode, stands, &standing);    // R3: the stand-in placed, kept or closed
    away_frame(now.frame, now.mode);                        // R4c: the page set aside back where 77 is free, or closed
    ready_frame(now.mode, now.screen);                      // R5: the entry kept ready
    stand_told(now.screen);                                 // R3: its `shown`, and the word of the sequencer's notice
    u32 page = page_on_screen(now.screen);
    if (page != 0 && top.shown && stand.on == (u32)STAND_TOP && page == stand.slot + 1u) page = 0;   // R3: the stand-in under the base's top page: `shown` said 0, and `page` says it too
    now.page =page != 0 ? op1f_cards.slots[page - 1].card : 0;
    now.disk = RD32(DISK_OPEN) != 0 ? 1u : 0u;              // M3
    now.canvas = (void*)RD32(FRAME_BUFFER);                 // R1: this frame's picture, already drawn
    if (CANVAS_ON_BLANK_ONLY && (page == 0 || pages.shown_how != (u32)HOW_BLANK)) now.canvas = 0;
    if (state.inputs != 0 && now.screen != state.last_screen && state.started > 1) {   // the line that places a screen among the keys
        u32 down_ms = input.last_down_ms, up_ms = input.last_up_ms;
        STOCK_LOG(3, LOG_FILE, 12, "input: screen %u is shown; %u keys down so far, the last %u, %u ms before; %u up, the last %u, %u ms before",
                  now.screen, input.down, input.last_down, state.ms - down_ms, input.up, input.last_up, state.ms - up_ms);
    }
    state.last_screen = now.screen;
    state.started = 2;
    hold_words(&handed, now.frame, (u32)now.canvas);        // R9 (base 33): `held` and `picture`
    hold_open(page);                                        // R9: the page this frame's calls may ask to hold
    u32 over = top.shown || now.popup != 0 || (stand.named && stand_on_screen) ? 1u : 0u;   // R9: the base's top page, the power popup, the ask's name below
    pages.canvas = now.canvas;                              // R1: for the paint helpers, while the frame functions are called
    if (stand.named && stand_on_screen) paint_name(now.canvas);                 // R3: the ask's name on the blank page, while the stand-in is on the screen (R4c: the base's flag)
    away_paint(now.screen);                                                   // R6b: the base's page for an engine away
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.framed >> i) & 1u) == 0) continue;
        const Op1fCard* card = op1f_cards.slots[i].card;
        Measure* m = &state.measure[i];
        u32 before = stamp();
        state.calling_sp = stack_here();                    // M3: where this thread's stack stands at the call
        state.calling = i + 1;                              // M3: whose frame function runs, for the files and the text
        card->frame(&now);
        state.calling = 0;
        u32 took = microseconds(stamp() - before);
        ++m->calls;
        m->sum += took;
        if (took < m->least) m->least = took;
        if (took > m->most) m->most = took;
    }
    pages.canvas = 0;                                       // R1
    popups_frame(&now, page);                               // R4c: the unit's own popups over a pack's page, after every frame function
    hold_frame(page, handed.held, over);                    // R9 (base 33): the next loop's hold, after everything drawn in this frame
    int why = quiet_popup(&state.quiet, now.popup != 0);    // the lines once more before the unit is off
    int period = --state.countdown <= 0;                    // a countdown, not a modulo: nothing is divided
    if (!period && !why) return;
    if (period) {
        state.countdown = PERIOD_FRAMES;
        u32 s = quiet_mix(quiet_start(), state.in_use);     // what the lines state: how many cards, and which of them had a call of a millisecond
        for (u32 i = 0; i < state.seen; ++i) {
            if ((state.used >> i) & 1u) s = quiet_mix(s, state.measure[i].most >= (u32)SLOW_US ? 1u : 0u);
        }
        s = quiet_mix(s, state.opened);                     // and how many pages were opened
        s = quiet_mix(s, popups.calls != 0 ? 1u : 0u);      // R4c: and whether the base drew one of the unit's own popups
        u32 refused = paint_taken();                        // R1: a refused painting is said
        why |= quiet_period(&state.quiet, s, input.down + input.up + input.turns != state.seen_down + state.seen_up + state.seen_turns || refused != 0);
    }
    if (why) say(why);
    if (why) say_level_4(period);                           // R1
    if (period) {
        clear_measures();                                   // read and cleared at every period, printed or not: a line is about one period
        popups.calls = popups.sum = popups.least = popups.most = 0;   // R4c: the same for the popups' cost
        input_seen();
        for (u32 i = 0; i < state.seen; ++i) watch.seen[i] = watch.down[i] + watch.up[i];   // R1
    }
}

// ---------------------------------------------------------------- pictures

// A name against a directory's entry, whose name ends inside its 24 bytes.
static int same_name(const char* name, const char* entry) {
    for (u32 i = 0; i < (u32)ART_NAME; ++i) {
        if (name[i] != entry[i]) return 0;
        if (name[i] == 0) return 1;
    }
    return 0;
}

// A name against one remembered by its first 23 characters: the same when those are.
static int same_kept(const char* name, const char* kept) {
    for (u32 i = 0; i < (u32)NAME_KEPT; ++i) {
        if (name[i] != kept[i]) return 0;
        if (name[i] == 0) return 1;
    }
    return 1;
}

// The entry of a directory under a name, or 0. Kept a function of its own: put into its callers the compiler held
// its answer in an index register, and out of line nothing in this unit touches one.
static OP1F_APART const ArtEntry* entry_of(const ArtDir* dir, const char* name) {
    for (u32 i = 0; i < dir->count; ++i) {
        if (same_name(name, dir->entries[i].name)) return &dir->entries[i];
    }
    return 0;
}

// What a used card's pack has for a name a screen asks for, the pack's own choice asked first: the entry, or 0.
static const ArtEntry* carried(const Op1fCard* card, const char* name, int* picked) {
    if (card->pictures == 0) return 0;
    const char* wanted = name;
    if (card->pick != 0) {
        wanted = card->pick(name);
        if (wanted == 0) return 0;
    }
    *picked = wanted != name;
    return entry_of((const ArtDir*)card->pictures, wanted);
}

// Two packs carry one name: said once a name, for sixteen names. The first of the two serves it.
// R6h (base 31): the look, the slot and the count in one masked instant (a request may come on any thread: two at fifteen
// names could take the count past sixteen); the line said after it, at the same request, as before.
static OP1F_APART void twice(const char* name, const char* first, const char* second) {
    u32 mask, say = 0;
    MASK_OFF(mask);
    u32 n = state.names, known = 0;
    for (u32 k = 0; k < n && !known; ++k) known = (u32)same_kept(name, state.name_said[k]);
    if (!known && n < (u32)NAMES_SAID) {                    // sixteen names were said: the log has what it needs
        char* kept = state.name_said[n];
        u32 i = 0;
        for (; i < (u32)NAME_KEPT && name[i]; ++i) kept[i] = name[i];
        kept[i] = 0;
        state.names = n + 1u;
        say = 1;
    }
    MASK_ON(mask);
    if (say) STOCK_LOG(2, LOG_FILE, 6, "cards: the picture %s is carried by '%s' and by '%s': '%s' serves it", name, first, second, first);
}

// ---------------------------------------------------------------- R4: pictures that take a place

enum { PLACES_BYTES = 36, PLACES_MOST = OP1F_PLACES_MOST, PLACE_NAME = OP1F_PLACE_NAME, ASK_NAME = OP1F_KIND_NAME, PASSED_SAID = 16 };
static_assert(sizeof(Op1fPlaces) == PLACES_BYTES, "the table of the places is 36 bytes at level 6");
static_assert((u32)PLACE_NAME == (u32)ART_NAME,"a name of the list has a directory entry's room");

// What the asks of this kind were found to say, once, from whichever thread asks first: stored whole before the word
// that says so, and read only past it.
static struct {
    u32 made, said;                                     // looked at; the lines said
    u32 bound;                                          // a bit a slot: its card is of level 6, held to the rule
    const char (*names[MOST])[PLACE_NAME];              // the list each such card's ask names, 0 for none (no ask, or one left aside)
    u32 count[MOST];                                    // and how many names it holds
    const char (*yields[MOST])[PLACE_NAME];             // R6h (base 31): the list each such card gives way for, 0 for none
    u32 yield_count[MOST];                              // R6h (base 31): and how many names it holds
} places;

// R6h (base 31): the flag of an ask of kind places whose list gives way (the header's OP1F_PLACES_GIVES_WAY), and what a
// card makes of one of its asks of the kind.
enum { PLACES_GIVES_WAY = 1 };
enum { ASK_ASIDE = 0, ASK_FIRM = 1, ASK_GIVES_WAY = 2 };

// R6h (base 31): the names a firm card was served for where a card that gives way had the picture: noted from any thread,
// each in one masked instant, and said by the display thread at its next frame. Sixteen names, once a name.
static struct {
    volatile u32 count;                                 // names noted: an entry is whole before the count that holds it
    u32 said;                                           // the display thread's: names said
    u32 slot[PASSED_SAID], over[PASSED_SAID];           // the card that gave way, the card that served
    char name[PASSED_SAID][ART_NAME];
} gave;

// The names a screen asked for that a card of level 6 carries and does not list: noted from any thread, each in one
// masked instant, and said by the display thread at its next frame. Sixteen names, once a name.
static struct {
    volatile u32 count;                                 // names noted: an entry is whole before the count that holds it
    u32 said;                                           // the display thread's: names said
    u32 slot[PASSED_SAID];
    char name[PASSED_SAID][ART_NAME];
} passed;

// An ask's name: one to fifteen characters of a-z, 0-9, '-' and '_', ended within its sixteen bytes.
static int plain_ask_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)ASK_NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)ASK_NAME;
}

// A name of the list: plain text with no space and no slash, ended within its room, at least one character and .svg.
static int picture_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)PLACE_NAME && name[n]; ++n) {
        char c = name[n];
        if (c <= ' ' || c > '~' || c == '/') return 0;
    }
    if (n == (u32)PLACE_NAME || n < 5u) return 0;
    return name[n - 4] == '.' && name[n - 3] == 's' && name[n - 2] == 'v' && name[n - 1] == 'g';
}

// Why an ask of this kind is left aside, or 0; `a` and `b` for the line:
//   PLACES_TABLE  (the ask's bytes, an Op1fPlaces' bytes): the table is too small or there is none; (the bytes its head
//                 says, the ask's bytes): the two differ; (its flags, 0): a flag (R6h: but PLACES_GIVES_WAY); (its `shown`'s place from the pack's
//                 first byte, 0): a `shown`; (its count, OP1F_PLACES_MOST): no name, or more than a list may hold
//   PLACES_LIST   (the list's place from the pack's first byte, the pack's bytes): the list does not lie in the pack on
//                 a word with all its names
//   PLACES_NAME   (the name's number from 1, 0): a name of the list is not a picture's name
// `plain` says whether its name may be shown.
enum { PLACES_IN_ORDER = 0, PLACES_TABLE = 1, PLACES_NOT_A_NAME = 2, PLACES_LIST = 3, PLACES_NAME = 4 };
static int places_refused(const Op1fAsk* ask, const Op1fCardSlot* slot, u32* a, u32* b, int* plain) {
    *plain = 0;
    const Op1fPlaces* table = (const Op1fPlaces*)ask->what;
    if (table == 0 || ask->bytes < (u32)PLACES_BYTES) {                            // the card's check holds the table in its pack, with its bytes
        *a = ask->bytes;
        *b = PLACES_BYTES;
        return PLACES_TABLE;
    }
    if (!plain_ask_name(table->head.name)) return PLACES_NOT_A_NAME;
    *plain = 1;
    if (table->head.bytes != ask->bytes) {
        *a = table->head.bytes;
        *b = ask->bytes;
        return PLACES_TABLE;
    }
    if ((table->head.flags & ~(u32)PLACES_GIVES_WAY) != 0) {                      // R6h (base 31): a bit this base does not know
        *a = table->head.flags;
        *b = 0;
        return PLACES_TABLE;
    }
    if (table->head.shown != 0) {
        *a = (u32)table->head.shown - slot->first;
        *b = 0;
        return PLACES_TABLE;
    }
    u32 count = table->count;
    if (count < 1u || count > (u32)PLACES_MOST) {
        *a = count;
        *b = PLACES_MOST;
        return PLACES_TABLE;
    }
    u32 list = (u32)table->names, first = slot->first, end = slot->end;
    if (list < first || list >= end || (list & 3u) != 0 || end - list < count * (u32)PLACE_NAME) {   // count * 24: at most 6,144, no overflow
        *a = list - first;
        *b = end - first;
        return PLACES_LIST;
    }
    for (u32 k = 0; k < count; ++k) {
        if (!picture_name(table->names[k])) {
            *a = k + 1u;
            *b = 0;
            return PLACES_NAME;
        }
    }
    return PLACES_IN_ORDER;
}

// R6h (base 31): what a card makes of its ask of kind places that comes `kinds`-th (from 0) among them, `why` what
// places_refused said of it and `firm` whether its first was taken firm: the first is the card's, firm or giving way by
// its flag; a second is taken only behind a firm first, in order and giving way; every other is left aside.
static int places_take(u32 kinds, int why, const Op1fAsk* ask, int firm) {
    if (why != PLACES_IN_ORDER || kinds > 1u) return ASK_ASIDE;
    u32 flags = ((const Op1fPlaces*)ask->what)->head.flags;
    if (kinds == 0) return flags == 0 ? ASK_FIRM : ASK_GIVES_WAY;
    return firm && flags == (u32)PLACES_GIVES_WAY ? ASK_GIVES_WAY : ASK_ASIDE;
}

// The asks of this kind of every card of level 6 in use, looked at once, after the check: worked out first and stored
// then, past a barrier the word that says so; then the lines, in the list's order, behind the check's. A card of
// level 6 with no such ask in order takes the place of no picture of the unit.
// R6h (base 31): each slot's lists worked out into locals and only their final values stored, so that a second thread
// that looks as well writes again what a reader may be reading, never a 0 before it; the list that gives way beside.
static OP1F_APART void places_look() {
    u32 bound = 0;
    for (u32 i = 0; i < state.seen; ++i) {
        const char (*names)[PLACE_NAME] = 0;
        const char (*yields)[PLACE_NAME] = 0;
        u32 count = 0, yield_count = 0;
        const Op1fCard* card = op1f_cards.slots[i].card;
        if (((state.used >> i) & 1u) != 0 && card->level >= 6u) {                  // the rule binds a card by its level alone, whatever its size
            bound |= 1u << i;
            u32 asked = 0, kinds = 0;
            int firm = 0;
            const Op1fAsk* asks = (const Op1fAsk*)asks_of(card, &asked);
            for (u32 k = 0; k < asked && kinds < 2u; ++k) {                         // the first ask of the kind is the card's; a second may give way; a later one is left aside
                if (asks[k].kind != (u32)OP1F_ASK_PLACES) continue;
                u32 a = 0, b = 0;
                int plain = 0, took = places_take(kinds, places_refused(&asks[k], &op1f_cards.slots[i], &a, &b, &plain), &asks[k], firm);
                const Op1fPlaces* table = (const Op1fPlaces*)asks[k].what;
                if (took == ASK_FIRM) {
                    names = table->names;
                    count = table->count;
                    firm = 1;
                } else if (took == ASK_GIVES_WAY) {
                    yields = table->names;
                    yield_count = table->count;
                }
                ++kinds;
            }
        }
        places.names[i] = names;
        places.count[i] = count;
        places.yields[i] = yields;
        places.yield_count[i] = yield_count;
    }
    places.bound = bound;
    BARRIER();                                                                      // all of the above is stored before the word that says it was made
    places.made = 1;
    BARRIER();
    if (places.said) return;
    places.said = 1;
    for (u32 i = 0; i < state.seen; ++i) {
        if (((bound >> i) & 1u) == 0) continue;
        const Op1fCard* card = op1f_cards.slots[i].card;
        u32 asked = 0, kinds = 0;
        int firm = 0;                                                               // R6h (base 31)
        const Op1fAsk* asks = (const Op1fAsk*)asks_of(card, &asked);
        for (u32 k = 0; k < asked; ++k) {
            if (asks[k].kind != (u32)OP1F_ASK_PLACES) continue;
            u32 a = 0, b = 0;
            int plain = 0, why = places_refused(&asks[k], &op1f_cards.slots[i], &a, &b, &plain);
            const char* name = plain ? ((const Op1fPlaces*)asks[k].what)->head.name : "?";
            int took = places_take(kinds, why, &asks[k], firm);                    // R6h (base 31)
            if (kinds == 0) firm = took == ASK_FIRM;
            if (took == ASK_GIVES_WAY)                                              // R6h (base 31): a list that gives way
                STOCK_LOG(3, LOG_FILE, 44, "places: '%s' of '%s' takes the place of %u picture(s) of the unit where no other pack's does", name, card->name,
                          places.yield_count[i]);
            else if (kinds != 0)
                STOCK_LOG(2, LOG_FILE, 26, "places: '%s' of '%s' is left aside: %s", name, card->name, "an earlier ask of the card names the pictures it takes the place of");
            else if (why == PLACES_NOT_A_NAME)
                STOCK_LOG(2, LOG_FILE, 26, "places: '%s' of '%s' is left aside: %s", name, card->name, "its name is not one the log can show");
            else if (why != PLACES_IN_ORDER)
                STOCK_LOG(2, LOG_FILE, 27, "places: '%s' of '%s' is left aside: %s (%u, %u)", name, card->name,
                          why == PLACES_TABLE ? "its table is not in order" : why == PLACES_LIST ? "its list of names does not lie in its pack on a word"
                                                                                       : "a name of its list is not a picture's name ending in .svg", a, b);
            else
                STOCK_LOG(3, LOG_FILE, 25, "places: '%s' of '%s' takes the place of %u picture(s) of the unit", name, card->name, places.count[i]);
            ++kinds;                                                                // R6h (base 31): counted after its line
        }
    }
}

// The check, and the asks of this kind after it: from any thread, as the check.
static inline void places_ready() {
    ready();
    u32 made = places.made;
    BARRIER();
    if (!made) places_look();
}

// Does the list of the card of this slot hold the name a screen asks for?
static int listed(u32 slot, const char* name) {
    const char (*names)[PLACE_NAME] = places.names[slot];
    u32 count = places.count[slot];
    for (u32 k = 0; k < count; ++k) {
        if (same_name(name, names[k])) return 1;
    }
    return 0;
}

// R6h (base 31): does the list the card of this slot gives way for hold the name a screen asks for?
static int gives_way(u32 slot, const char* name) {
    const char (*yields)[PLACE_NAME] = places.yields[slot];
    u32 count = places.yield_count[slot];
    for (u32 k = 0; k < count; ++k) {
        if (same_name(name, yields[k])) return 1;
    }
    return 0;
}

// R6h (base 31): a name a firm card was served for where the card of `slot` gives way and had the picture, `over` the
// card that served: noted once a name, for sixteen names, in one masked instant (a request may come on any thread).
static OP1F_APART void note_gave_way(u32 slot, u32 over, const char* name) {
    u32 mask;
    MASK_OFF(mask);
    u32 n = gave.count, known = 0;
    for (u32 k = 0; k < n && !known; ++k) known = (u32)same_kept(name, gave.name[k]);
    if (!known && n < (u32)PASSED_SAID) {
        char* kept = gave.name[n];
        u32 i = 0;
        for (; i < (u32)NAME_KEPT && name[i]; ++i) kept[i] = name[i];
        kept[i] = 0;
        gave.slot[n] = slot;
        gave.over[n] = over;
        BARRIER();                                                                  // the entry is whole before the count that holds it
        gave.count = n + 1u;
    }
    MASK_ON(mask);
}

// A name a card of level 6 carries and does not list, asked for by a screen: noted once a name, for sixteen names, in
// one masked instant (a request may come on any thread).
static OP1F_APART void note_passed(u32 slot, const char* name) {
    u32 mask;
    MASK_OFF(mask);
    u32 n = passed.count, known = 0;
    for (u32 k = 0; k < n && !known; ++k) known = (u32)same_kept(name, passed.name[k]);
    if (!known && n < (u32)PASSED_SAID) {
        char* kept = passed.name[n];
        u32 i = 0;
        for (; i < (u32)NAME_KEPT && name[i]; ++i) kept[i] = name[i];
        kept[i] = 0;
        passed.slot[n] = slot;
        BARRIER();                                                                  // the entry is whole before the count that holds it
        passed.count = n + 1u;
    }
    MASK_ON(mask);
}

// The names noted since the last frame, said on the display thread.
// R6h (base 31): and behind them the names a card gave way for.
static OP1F_APART void said_passed() {
    u32 n = passed.count;
    u32 g = gave.count;                                                             // R6h (base 31)
    BARRIER();
    for (; passed.said < n; ++passed.said) {
        u32 k = passed.said;
        STOCK_LOG(3, LOG_FILE, 28, "cards: '%s' carries a picture named %s and does not take its place: the unit's own is shown",
                  op1f_cards.slots[passed.slot[k]].card->name, passed.name[k]);
    }
    for (; gave.said < g; ++gave.said) {                                            // R6h (base 31)
        u32 k = gave.said;
        STOCK_LOG(3, LOG_FILE, 45, "cards: '%s' gives way to '%s' for the picture %s", op1f_cards.slots[gave.slot[k]].card->name,
                  op1f_cards.slots[gave.over[k]].card->name, gave.name[k]);
    }
}

// R6h (base 31): what a screen's pass keeps of the cards that give way for its name: the stand-in, the first such card
// that has the picture, with its own record; a second such card that has it; and the firm card that served. Each a
// slot + 1, 0 for none.
struct Stand {
    u32 slot, second, served;
    int found;                                              // the stand-in's FOUND or FOUND_PICKED
    const ArtEntry* entry;                                  // and its entry
};

// R6h (base 31): does the card of slot `i` give way for the name? Then it is asked while it could stand in, be said
// beside the stand-in, or be the line's (`open`: no firm card served yet), and kept as the stand-in or as the second.
static OP1F_APART int stand_by(Stand* stand, u32 i, const char* name, int open) {
    if (!gives_way(i, name)) return 0;
    if (stand->slot == 0 || (open && stand->second == 0)) {
        int own = 0;                                        // its own: `carried` writes it even when it finds nothing
        const ArtEntry* e = carried(op1f_cards.slots[i].card, name, &own);
        if (e != 0 && stand->slot == 0) {
            stand->slot = i + 1u;
            stand->entry = e;
            stand->found = own ? FOUND_PICKED : FOUND;
        } else if (e != 0) {
            stand->second = i + 1u;
        }
    }
    return 1;
}

// R6h (base 31): after the pass, what it answers: the firm card's `found` where one served (noted for line 45 where
// the stand-in had the picture), else the stand-in's (a second card that gives way and has it: said by `twice`).
static OP1F_APART int stand_in(const Stand* stand, int found, const char* name, const ArtDir** dir, const ArtEntry** entry) {
    if (stand->slot == 0) return found;
    const Op1fCard* card = op1f_cards.slots[stand->slot - 1u].card;
    if (found != 0) {
        note_gave_way(stand->slot - 1u, stand->served - 1u, name);                 // a firm card served it: the line at the next frame
        return found;
    }
    *dir = (const ArtDir*)card->pictures;
    *entry = stand->entry;
    if (stand->second != 0) twice(name, card->name, op1f_cards.slots[stand->second - 1u].card->name);
    return stand->found;
}

// The picture loader, for a name none of its four packs has: the directory and the entry of the first card that has
// it, in the list's order (FOUND, or FOUND_PICKED where the pack chose another of its pictures for the name), or 0.
// With `only`, a pack's own card: that card's directory alone, by the name itself. No file is touched here.
// R4: without `only` (a screen's request), a card of level 6 is looked into, and its `pick` asked, only for a name
// its ask of kind OP1F_ASK_PLACES lists; it is passed by. Where no card serves the name and one of level 6 carries it
// unlisted, the name is noted for the line.
// R6h (base 31): the cards whose name is firm (listed in the firm list, or any name of a card below level 6) go as on
// base 30. A card that gives way for the name and has the picture is kept as the stand-in, the first such, with its
// own record (`carried` writes its `picked` even when it finds nothing); it never ends the pass and never serves while
// a firm card does. After the pass: the firm card's, else the stand-in's (a second card that gives way and has it: said
// by `twice`); a firm card's where the stand-in had it: noted for line 45. The stand-in's work is done apart
// (stand_by, stand_in), its record kept in memory: the pass itself holds no more than base 30's in its registers.
extern "C" int op1f_cards_picture(const char* name, const Op1fCard* only, const ArtDir** dir, const ArtEntry** entry) OP1F_NAMED("op1f_cards_picture");
extern "C" int op1f_cards_picture(const char* name, const Op1fCard* only, const ArtDir** dir, const ArtEntry** entry) {
    places_ready();                                         // R4: the check, and the places after it
    int found = 0, picked = 0;
    u32 passer = 0;                                         // R4: the slot + 1 of the first card of level 6 that carries the name and does not list it
    const Op1fCard* server = 0;
    Stand stand = {0, 0, 0, 0, 0};                          // R6h (base 31)
    if (name == 0) return 0;
    for (u32 i = 0; i < state.seen; ++i) {
        if (((state.used >> i) & 1u) == 0) continue;
        const Op1fCard* card = op1f_cards.slots[i].card;
        if (only != 0) {
            if (card != only) continue;
            const ArtEntry* own = card->pictures ? entry_of((const ArtDir*)card->pictures, name) : 0;
            if (own == 0) return 0;
            *dir = (const ArtDir*)card->pictures;
            *entry = own;
            return FOUND;
        }
        if (((places.bound >> i) & 1u) != 0 && !listed(i, name)) {                // R4: a card of level 6 serves a screen only the names it lists
            if (stand_by(&stand, i, name, server == 0)) continue;                  // R6h (base 31): a card that gives way for the name
            if (passer == 0 && card->pictures != 0 && entry_of((const ArtDir*)card->pictures, name) != 0) passer = i + 1u;
            continue;
        }
        const ArtEntry* e = carried(card, name, &picked);
        if (e == 0) continue;
        if (server != 0) {                                  // a second pack has it too: said, and the first serves it
            twice(name, server->name, card->name);
            break;
        }
        server = card;
        stand.served = i + 1u;                              // R6h (base 31)
        found = picked ? FOUND_PICKED : FOUND;
        *dir = (const ArtDir*)card->pictures;
        *entry = e;
    }
    found = stand_in(&stand, found, name, dir, entry);     // R6h (base 31): a card that gives way had the picture
    if (found == 0 && passer != 0) note_passed(passer - 1u, name);                  // R4: the unit's own is shown, and a card of level 6 carries the name
    return found;
}

// ---------------------------------------------------------------- the helpers

// R1: this frame's canvas while a card's frame function is being called on the display thread, 0 otherwise: for the
// paint unit, which paints only into it.
extern "C" void* op1f_cards_canvas() OP1F_NAMED("op1f_cards_canvas");
extern "C" void* op1f_cards_canvas() {
    void* canvas = pages.canvas;
    return canvas != 0 && op1f_cards_calling() != 0 ? canvas : 0;
}

// The clock: milliseconds since the base's first frame. It moves once a frame.
extern "C" unsigned op1f_ms() {
    return state.ms;
}

// The document of one of the firmware's screens by its number: 0 for a number outside 1 to 85, for a number no
// screen has, and for a screen whose document stock has not loaded yet.
extern "C" void* op1f_screen(int number) {
    if (number < (int)FIRST_SCREEN || number > (int)LAST_SCREEN) return 0;
    u32* screens = STOCK_SCREENS();
    u32* screen = screens ? (u32*)screens[number] : 0;
    return screen ? (void*)screen[SCREEN_DOCUMENT] : 0;
}

// A part of a document by its name: 0 when the document has none of that name, and for no document or no name.
extern "C" void* op1f_node(void* document, const char* name) {
    if (document == 0 || name == 0) return 0;               // stock's lookup stops the unit on either
    u32* screens = STOCK_SCREENS();
    if (screens) {
        for (int number = FIRST_SCREEN; number <= (int)LAST_SCREEN; ++number) {
            u32* screen = (u32*)screens[number];
            if (screen != 0 && (void*)screen[SCREEN_DOCUMENT] == document) return STOCK_FIND_NODE(screen, name, 1);
        }
    }
    u32 held[4] = { 0, (u32)document, 0, 0 };               // no screen holds it: the lookup reads the word at +4 and nothing else
    return STOCK_FIND_NODE(held, name, 1);
}

// A part shown or hidden, with what is inside it. R9 (base 33): a part whose display byte changes is a change of its
// document's.
extern "C" void op1f_show(void* part, int on) {
    if (part == 0) return;
    u8 want = (u8)(on ? 1 : 0);
    if (((u8*)part)[NODE_DISPLAY] != want) hold_changed_by();     // R12: or the page's own
    ((u8*)part)[NODE_DISPLAY] = want;
}

// R9 (base 33): whether op1f_move sets a part another matrix than the one it set it last: the last sixteen parts moved
// are kept with their matrices, a ring. 1 for a part not kept (a change, to be sure). Out of line, nothing inlined.
static OP1F_APART u32 moved_other(void* part, const int* matrix) {
    u32 at = (u32)HOLD_MOVES;
    for (u32 i = 0; i < (u32)HOLD_MOVES; ++i) {
        if (hold.moves[i].part == part) at = i;
    }
    u32 other = 0;
    if (at == (u32)HOLD_MOVES) {
        at = hold.move_next;
        hold.move_next = (at + 1u) & (u32)(HOLD_MOVES - 1);
        hold.moves[at].part = part;
        other = 1;
    }
    int* kept = hold.moves[at].matrix;
    for (u32 k = 0; k < (u32)MATRIX_WORDS; ++k) {
        if (kept[k] != matrix[k]) other = 1;
        kept[k] = matrix[k];
    }
    return other;
}

// A part's matrix set: nine words in 1024ths, [a, b, 0, c, d, 0, e, f, 1024]. R9 (base 33): another matrix than the
// one this set the part last is a change of its document's.
extern "C" void op1f_move(void* part, const int* matrix) {
    if (part == 0 || matrix == 0) return;
    if (moved_other(part, matrix)) hold_changed_by();             // R12: or the page's own
    STOCK_APPLY_MATRIX(part, matrix);
}

// R9 (base 33): a pack's ask to keep its page's picture in the next frame (module/build_r6e_card.h, "Base 33"). Taken
// from the card's own frame function while its page is the one this frame names: 1. `redraw` not 0: the next frames
// are drawn whole (three, by the rule's count). 0 for any other call.
extern "C" int op1f_page_hold(const Op1fCard* card, unsigned redraw) OP1F_NAMED("op1f_page_hold");
extern "C" int op1f_page_hold(const Op1fCard* card, unsigned redraw) {
    int slot = slot_of(card);
    if (slot < 0 || hold.page != (u32)slot + 1u || state.calling != (u32)slot + 1u) return 0;
    hold.asked = (u32)slot + 1u;
    if (redraw != 0) hold.redraw = 1;
    return 1;
}

// R9 (base 33): the mark of a base whose page may hold its picture.
extern "C" const u32 op1f_hold_1 OP1F_NAMED("op1f_hold_1");
const u32 op1f_hold_1 = 1;

// R12 (base 36): a pack's ask to hold its page's picture and draw again the rectangles it names in this frame
// (module/build_r6e_card.h, "Base 36"): op1f_page_hold's checks and answer, and the frame marked as asking parts.
extern "C" int op1f_page_parts(const Op1fCard* card) OP1F_NAMED("op1f_page_parts");
extern "C" int op1f_page_parts(const Op1fCard* card) {
    if (op1f_page_hold(card, 0) == 0) return 0;
    parts.asked = hold.asked;
    return 1;
}

// R12: a cut of [at, at + size) to [0, most): 0 when nothing is left. No sum overflows.
static OP1F_APART u32 parts_cut(int at, int size, u32 most, u32* from, u32* past) {
    if (size <= 0 || at >= (int)most) return 0;
    if (at < 0) {
        size += at;                                     // the signs differ: no overflow
        at = 0;
        if (size <= 0) return 0;
    }
    *from = (u32)at;
    *past = (u32)size > most - (u32)at ? most : (u32)at + (u32)size;
    return 1;
}

// R12: a rectangle and its parts named for the next frame, into the frame's ask. From the card's own frame function
// while its page is the one this frame names, before or after op1f_page_parts (a frame that names one and does not ask
// is drawn whole: hold_frame). A refusal (a rectangle of nothing, no parts, too many, a part 0) is counted and makes
// the next picture whole; a call from elsewhere answers 0 and changes nothing.
extern "C" int op1f_page_redraw(const Op1fCard* card, int x, int y, int w, int h, void* const* named, unsigned count) OP1F_NAMED("op1f_page_redraw");
extern "C" int op1f_page_redraw(const Op1fCard* card, int x, int y, int w, int h, void* const* named, unsigned count) {
    int slot = slot_of(card);
    if (slot < 0 || hold.page != (u32)slot + 1u || state.calling != (u32)slot + 1u) return 0;
    Op1fPlan* p = &parts_plan;
    u32 x0, x1, y0, y1;
    u32 ok = parts_cut(x, w, (u32)PARTS_WIDE, &x0, &x1) != 0 && parts_cut(y, h, (u32)PARTS_TALL, &y0, &y1) != 0 && named != 0 && count != 0 &&
             p->rects < (u32)OP1F_REDRAWS_MOST && count <= (u32)OP1F_REDRAW_PARTS_MOST - p->parts ? 1u : 0u;
    for (u32 i = 0; ok != 0 && i < count; ++i) {
        if (named[i] == 0) ok = 0;
    }
    if (ok == 0) {
        ++parts.refused;
        return 0;
    }
    Op1fPlanRect* r = &p->rect[p->rects];
    r->x0 = x0;
    r->x1 = x1;
    r->y0 = y0;
    r->y1 = y1;
    r->first = p->parts;
    r->count = count;
    for (u32 i = 0; i < count; ++i) p->part[p->parts + i] = named[i];
    p->parts += count;
    ++p->rects;
    return 1;
}

// R12: the mark of a base whose held page's parts the firmware draws again.
extern "C" const u32 op1f_parts_1 OP1F_NAMED("op1f_parts_1");
const u32 op1f_parts_1 = 1;

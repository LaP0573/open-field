// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R6e, the release's words for base 30 (2026-11-21): base 30's header as it was built, which stays as it was for base
// 30's reviewed build, with the words on where the unit's lists show a pack's engine and a pack's effect mended. The
// unit sorts its engine list and its lists of effects by name, so a pack's engine or effect stands among the unit's own
// where its name falls (fold after fm, crush before cwo), not at the end; and the base's list of effects
// (op1f_rack_list) is said in its own order, the order of the effects' numbers. Every declaration stands here at the
// same place with the same meaning and nothing is added: the guard is base 30's, so a unit that names both headers gets
// one of them. What base 30's header said follows and holds, but where these words say otherwise.
//
// Step R6d: what a pack shows the base, and what the base offers a pack, at level 10, for base.pack 30: the value as
// heard. A sibling of module/build_r6_card.h, which stays as it was for base 29: every declaration of it stands here at
// the same place with the same meaning, and level 10 adds behind it one helper, op1f_param_heard, and the mark
// op1f_level_10; no kind of ask and no field of the card.
//
// Level 10: the value as heard. op1f_param_get answers a value of a pack's effect or engine as it is set: the knob's
// place. While one of the unit's LFOs moves that knob (or another pack's offset on the unit's knob words), the sound
// plays another value, block by block. op1f_param_heard answers that one: the value the audio last used, in the pack's
// own numbers, kept by the base where it maps the block's knob words into the pack's values (an engine's render of a
// voice, an effect's call at a place of the unit's). Where the place has not played in the last few blocks it answers
// the value as set, as op1f_param_get does. A page that shows its values draws these, so that the owner sees an LFO
// move them, and turns them with op1f_param_set as before. The firmware's own pages are not touched: the base keeps
// its own copy and takes nothing the firmware's own pages read.
//
// The mark. A pack that calls op1f_param_heard asks for level 10 the way a pack asks for every earlier level, through
// its card: its recipe says level 10 (a card of level 10 is a card of level 9's 64 bytes), and its card's source, which
// tools/pack_build.py writes from the recipe, names op1f_level_10, so nothing is written by hand. Under a base below
// level 10 the loader leaves such a pack out, as it does for any mark it lacks. A base of level 10 offers op1f_level_1
// to op1f_level_9 too, and op1f_bare_1.
//
// And from base 30 on, for level 9's engines: the base writes an engine's sounds only where the unit has room for all
// of them and 20 more of the owner's own (the unit takes 500 user sounds, synth and drum together; 100 on a unit set to
// the classic content): else none, with a line, and again at the next start.
//
// What base 29's header said follows and holds.
//
// Step R6: what a pack shows the base, and what the base offers a pack, at level 9, for base.pack 29: synth engines. A
// sibling of module/build_r5_card.h, which stays as it was for base 28: every declaration of it stands here at the same
// place with the same meaning, and level 9 adds behind it. Level 9 adds one kind of ask, OP1F_ASK_ENGINE, and the mark
// op1f_level_9; no helper and no field of the card: the helpers of level 8, op1f_param_get, op1f_param_set and
// op1f_riding, answer for an ask of this kind too, `place` then being a synth sound's slot (0 to 7).
//
// Level 9: synth engines. A pack adds an engine to the unit's list of engines (shift and T1 on a synth sound) with
// one ask of kind OP1F_ASK_ENGINE. The owner meets it where he meets every engine:
//   - the engine list shows its name among the unit's own engines, in the order of their names (a row with no sound
//     of its own is passed over by a walk from sound to sound: shift and T1 shows it); picking it puts it on the
//     sound, as any engine;
//   - the sound list shows a folder of the engine's own sounds, written the first time the pack runs (each sound of
//     the ask's list, once for each version of the pack; a file is never written over, and a folder of the owner's of
//     the same name is never touched). The folder shows after the next start;
//   - T1 shows the engine's page: level 5's page that stands in (op1f_page_stand_in, this ask's number), or the base's
//     blank page with the engine's name until the pack gives one;
//   - the four knobs turn values 1 to 4, and with shift values 5 to 8; the unit's LFOs (and any pack's offsets on the
//     unit's knob words) move them, and the LFO pages name them;
//   - saving the sound keeps it. A sound saved with a pack's engine is a sound of the unit's own engine `dsynth`, its
//     eight knobs inside dsynth's own ranges, with the engine's name added at the top: the unit without the pack plays
//     it as dsynth, with its knobs where the engine left them; with open-field and without the pack it is silent.
//     Nothing is ever reset.
//
// The unit keeps the voices: which voice plays which note, stealing, the play modes, glide, the envelope, velocity,
// pan and the mix are the unit's own, before and after the engine. An engine fills one voice's block of sound from
// its values and its pitch; the unit then applies its envelope, the velocity, the pan and the mix as for any engine.
//
// The threads. `render` is called on the audio thread, once a block for each voice that sounds, one voice after the
// other (never two at once): it may not wait, allocate, log or call the base, uses at most OP1F_STAGE_STACK_MOST bytes
// of stack with what it calls, and comes back within what the table declares. A note's start and its release reach no
// function of the pack: the next block of that voice says them in `events`. `reset` is called on the display thread,
// as a frame function is. What `render` shares with the pack's frame function is single words.
//
// The mark. A pack with an ask of this kind needs op1f_level_9, the mark of a base of level 9 (its card's source
// names it, so nothing is written by hand). A base of level 9 offers op1f_level_1 to op1f_level_8 too, and op1f_bare_1.
// The name sorts before op1f_param_get, so a pack that names both is still left out under a base below level 9, as the
// loader does for any mark it lacks. What base 28's header said follows and holds.
//
// Step R5: what a pack shows the base, and what the base offers a pack, at level 8, for base.pack 28: effects. A
// sibling of base 27's header for pack authors, which stays as it was for base 27: every declaration of it stands here at the
// same place with the same meaning, and level 8 adds behind it. A pack adds an effect to the unit's own lists of
// effects (a sound's, shift and T3, and the master's, on the mixer) with one ask of a new kind, OP1F_ASK_EFFECT: the
// owner picks it there as any effect, switches it on and off with the unit's own key, turns its four values with the
// four knobs, aims an LFO at them, and saves it with the sound. Each effect stands on one of eight of the unit's own
// effects, which the unit keeps in the sound's file with its four knobs inside that effect's own ranges and switched
// off: on a unit without the pack, and on the original firmware, such a sound plays with no effect. The base keeps one
// list of every effect the packs registered, and any pack may run one of them inside its own sound work (a chain of
// effects, a lane of a mixer). Level 7 gains the master as a place for values (OP1F_KEEP_MASTER). What base 27's header
// said follows and holds.
//
// Step R4c: what a pack shows the base, and what the base offers a pack, at levels 6 and 7, for base.pack 27. A sibling
// of module/build_r4b_card.h, which stays as it was for base 26: every declaration of it stands here at the same place
// with the same meaning; no word, kind or mark is added, and op1f_level_7 is the newest mark. What changes is what a
// pack meets, for every pack alike, and the words that say it. While the unit's own notice lies over a page that
// stands in for a screen of the firmware, a pack paints as usual; the base then darkens the whole frame and draws the
// unit's notice over it, as the unit does over its own sequencers. A page that stays with its mode is set aside, not
// closed, while another page takes the lent screen in another mode, and waits for its mode again when that page goes.
// And op1f_value_set answers OP1F_NOT_NOW for the tape while a tape the unit loaded has not been told yet. What base
// 26's header said follows and holds, but where these words say otherwise.
//
// Step R4b: what a pack shows the base, and what the base offers a pack, at levels 6 and 7, for base.pack 26, the first
// base that serves level 7. A sibling of base 24's header for pack authors, which stays as it was for base 24: every word of it
// stands here at the same place with the same meaning, and three things are added, for every pack alike. A flag of
// op1f_page_show, OP1F_PAGE_STAYS, by which a pack's page stays with its mode while the unit visits another mode and
// shows again at once when the unit comes back. A page that stands in for a screen of the firmware gives way while
// the unit's own notice lies over it: the notice shows alone, and no pack is handed a canvas meanwhile. Neither needs
// a mark: a base before 26 refuses the flag, and a pack paints only with a canvas. And, for level 7, what `changed`
// tells of a tape the unit has just made, new or copied (OP1F_KEPT_MADE). What base 24's header said follows and
// holds.
//
// Step R4: what a pack shows the base, and what the base offers a pack, at levels 6 and 7. A sibling of
// module/build_r3_card.h, which stays as it was for the packs built against level 5: every word of levels 1 to 5
// stands here at the same place with the same meaning, and levels 6 and 7 add behind it. It holds, too, what base
// 21's header (base 21's header for pack authors) says of a base that holds nothing made for one pack: no folder name is the
// base's, no page made for one pack is opened over a pack's page, no pack is asked first for a picture by its name;
// and its two additions, the mark op1f_bare_1 and the count op1f_svg_served.
//
// The words. The loader is our piece inside the firmware file: at the start it reads the pack files, places them
// and joins them by their names. The base is the shared pack, the file base.pack. A pack that wants something of
// the base carries a card: a small table in its own body that says what it wants. The pack file's header names the
// card's place in its last word; tools/pack_build.py writes the card's source from the pack's recipe (`card`) and
// the header's word (`build3`). The loader hands the base the cards of the packs that run, in the order of their
// files' names, and the base serves each pack through its card and knows none of them by name.
//
// Level 1: a call once a frame, pictures the pack carries for screens of the firmware, a picture of the pack's own,
// and six helpers, so that a pack with a card names no address of the firmware. A picture such a pack carries is a
// picture as it is, or instructions of either kind by which the unit makes it from its own pictures, up to eight of
// them in a set.
//
// Level 2: a page of the pack's own, with keys and knobs. Only one page is on the screen at a time, so the base
// lends the one it has: a pack makes a document from a picture of its own (op1f_picture), finds its parts by their
// names (op1f_node), opens and closes its page through the base (op1f_page_open, op1f_page_close) and dresses it
// from its frame function. A pack never names a screen's number. And every key and knob event is offered to the
// packs before the firmware's own code for the mode: first to the pack whose page is shown, then to every card that
// has an input function, in the order of the files' names, until one takes it. A taken event does not reach the
// firmware; one that nobody takes goes on as it always did. A pack picks its own keys, anywhere: a combination in a
// mode, or a key on another screen. Nobody keeps a list of them: where two packs pick the same keys, the pack of the
// earlier file gets them. The base offers a key to the packs before the mode's own code, where our own features
// stand, so a pack that takes a key one of them uses has it first. And while a pack's page is shown, every key and
// knob it does not take still goes to the mode's own code, which acts on the screen the page hides: in the mixer
// the other knobs move levels and pans nobody sees, over the effect train they move effects one hears. A pack that
// wants its page to own the knobs takes all four while its page is shown.
//
// Level 3: folders of the pack's own on the unit's disk, files read and written there through the base, each folder
// shown to the computer in the disk mode if the pack asks, and text on the pack's page.
//
//   Folders: none, one or several. Most packs need none: a pack whose card says no folder gets none, and nothing of
//   the disk is touched for it. A pack that keeps something (its settings, files the computer brings) lists its
//   folders in its card, up to eight. Each has
//     a name     what the computer shows in the disk mode, beside synth, drum, tapes, mixdown, packs and logs;
//     a place    where the folder lies on the unit's disk: user/pf/ and the place, a folder of its own;
//     shown      whether the computer sees it. A folder that is not shown is a pack's private folder (its settings).
//   tools/pack_build.py writes the list from the recipe and fills what the recipe leaves out: a folder's name is the
//   pack's own when the pack has one folder and does not name it; its place is its name; it is shown unless the
//   recipe says not. A pack names a folder to the helpers by its number in that list, from 0.
//   The base makes each folder at its first frame (user/pf/ and the place, with the slash behind: the folder maker
//   makes the part before each slash), and says one line a folder. A name or a place is one to fifteen characters
//   of A-Z, a-z, 0-9, space, '_', '-' and '.', not beginning with a space, a dot or a dash and not ending with a
//   space or a dot; the tool refuses any other and the base does not use it. Where two folders want one thing, the
//   earlier keeps it, in the order of the packs' file names and then of each pack's list, capitals counted as small
//   letters (the computer's disk cannot keep two names that differ only in capitals):
//     a name the firmware's (synth, drum, tapes, mixdown), the loader's (packs), the base's (logs, since base 34) or an earlier
//       folder's: the folder is not shown, and its line says so; the pack still has it on the disk;
//     a place an earlier folder has: the folder is not the pack's, its line says so, and the helpers answer
//       OP1F_NO_FOLDER for it.
//   In the disk mode the computer sees a shown folder's files, every name the disk holds there but those beginning
//   with a dot and the folders inside it, which are not shown; it may read them, bring new ones and delete them
//   (a delete takes the file off the disk). The firmware builds what the computer sees once, when the disk mode
//   starts: a file a pack writes while the disk mode is open is seen at the next disk mode.
//
//   Files: op1f_list, op1f_read and op1f_write work on the files of a pack's own folder, by their names. A pack's
//   file is bytes: the base reads and writes them as they are. A write goes to a file of the base's in tmp/ first
//   and is then renamed over the pack's file, as the firmware writes its own files, so a file is the old one or the
//   new one, never half of each; and it is held by the file's size afterwards, since the firmware does not say when
//   a rename fails. Every write that fails, and the first ones that land, are said on the log by the pack's name.
//   While the disk mode is open (the frame's `disk`), the computer owns the folders: a write is refused with
//   OP1F_NOT_NOW, and a read sees what the disk holds. A pack that keeps its settings in memory reads its file again
//   when `disk` goes back to 0, since the computer may have brought a new one.
//
//   Text: op1f_text writes a text into a part of a document op1f_picture made, in the firmware's own letters, as the
//   firmware's lists show theirs. The part is a placeholder in the pack's picture: a group (<g id="...">) holding a
//   path with a stroke, whose top-left is where the text begins and whose stroke colour the letters take. The
//   letters are small letters only, the firmware's font having no capitals of its own: a capital is shown as its
//   small letter, and a character the font has not as a question mark.
//
// Level 4: what a page needs. Pages that stack: a page shown over another in the same mode lies over it, and the one
// under it shows again when it closes; a page of the base's own, black, for a pack that paints its page itself; text
// set left, centred or to the right, its width measured, and the text moved as a part; painting on the frame's
// picture with boxes, lines, blocks of pixels and text in the base's own small font; a file of a pack's folder
// removed; and, for an input function, the thread an event came on, and a key watched: a pack may see a key's down
// and up and still let the key go on to the others and to the firmware.
//
//   Painting. Each frame the screen is drawn whole, and then the base calls the packs' frame functions: the frame's
//   `canvas` is that frame's picture, already drawn. What a pack paints on it lies over everything for
//   that frame and is gone at the next, so a pack paints again at every frame. Only from the pack's own frame
//   function, with the `canvas` it was handed in that call: any other call paints nothing and is counted, and the
//   base says the count on the log by the pack's name. A pack that paints its whole page opens the base's blank page
//   (op1f_page_show with no document) and paints on it: the screen behind is black, every frame. While the unit's own
//   notice lies over a page that stands in for a screen of the firmware (level 5, OP1F_SHOWN_COVERED), a pack paints
//   as usual: from base 27 on the base then darkens the frame, what the pack painted included, and draws the unit's
//   notice over it, after every frame function. (Base 26 handed no canvas in such a frame.)
//
//   The asks. A card of level 4 may carry a list of things it asks of the base. Level 4 defines no kind of ask yet:
//   a later level adds kinds (a sequencer, an effect, an engine, a point of the audio path, a sound's file) with their
//   helpers, and never a field of the card. A base that does not know a kind uses the card and leaves that ask aside,
//   with one line.
//
// Level 5: sequencers, and notes heard. The card's list of asks gets its first two kinds; the card itself, and what a
// frame function and an input function are handed, stay as level 4 has them.
//
//   A sequencer. A pack adds a sequencer of its own to the unit's list: its ask hands over a name and up to five
//   functions, and the pack changes no place of the firmware. The base puts the name at the end of the list of
//   sequencers, for the synth and the drum or for the synth alone. While the owner has chosen it for an instrument
//   and switched it on, the base calls the pack at every pulse of the clock and for every key of the keyboard and
//   every MIDI note that goes down (and for the up of each one the pack took: a key it lets the instrument play is
//   ended by the instrument, and the pack is not told), plays the notes the pack asks for, ends the ones it left
//   sounding when the sequencer stops, and shows a page of the pack's in place of the sequencer's screen. The unit
//   keeps the owner's choice by the sequencer's name, and the base keeps it across a start without the pack. Several
//   packs may each add sequencers, sixteen in all; one is chosen for the synth and one for the drum at a time, as
//   with the firmware's own seven. Where two packs ask for one name, the earlier keeps it, in the order of the files'
//   names; the later pack still runs, without that sequencer.
//
//   The unit's own notice. When the owner switches the sequencer off while its screen is up, the unit shows its own
//   notice ("sequencer off") for a moment. From base 27 on, while that notice is up over a page that stands in for the
//   sequencer's screen, the page stays and the pack paints as usual; after every frame function the base darkens the
//   whole frame and draws the unit's own notice over it, as the unit does over its own sequencers: the page shows
//   dark under the notice. The ask's `shown` says OP1F_SHOWN_COVERED meanwhile, and the frame's `page` still names
//   the pack. A pack has nothing to do for it. (Under bases 20 to 25 the page lay over the notice and hid it; base 26
//   put a page of its own in the page's place and handed no pack a canvas, so the notice showed on black.)
//
//   Notes heard. Any pack may ask to be told every note the instrument is asked to begin and to end, whoever asks:
//   the keyboard, MIDI, a sequencer of the firmware's or of a pack's. It listens only: a pack that wants to change
//   what is played is a sequencer. What it hears are the asks, not the sound. The instrument also ends notes by
//   itself (when the unit changes instrument or a sound is loaded, when a seventh note takes a voice), and no call
//   comes for those; an end may come twice, or for a note that never began; and a begin comes also where the
//   instrument then plays nothing (a note already sounding, a sound being loaded). A pack that keeps a list of
//   sounding notes takes it as an upper bound, and clears it when the frame's `mode` changes.
//
// Level 6: a place in the sound's path, working memory, and pictures that take the place of the unit's own. Three
// kinds of ask more; the card, and what a frame function and an input function are handed, stay as level 4 has them.
//
//   A sound stage. A pack's function is handed the sound, once a block of 64 frames, at one of three points: an
//   instrument after its effect (the synth's and the drum's sound, each with a call of its own that says which), the
//   unit's sum before the master effect, and the unit's sum after it. The function may change the sound in place.
//   At a point any number of packs: first the stages that change the sound, in the order of the files' names and then
//   of each card's asks, then those that only listen, in the same order. A stage that only listens hears what the
//   others made, and is handed a copy of it, so that nothing it does to the copy is heard. Each stage says what one
//   call costs it at most, in the processor's cycles, and the base holds it to that: a stage whose calls go well over
//   is switched off until the next start, and the sound then goes past it untouched.
//
//   Working memory. A pack asks for a block of memory, up to 32 MiB from base 41 (16 MiB before). The base takes it
//   at its first frame, from the unit's spare memory first and then from its heap, clears it to 0 over the frames that
//   follow, and only then hands it over; it is never given back while the unit runs. Where it cannot be had, the base says so on the log by the pack's name, and the pack runs without it.
//   A stage may wait for a memory of its own card: it is not called before that memory is handed over, and never
//   when it was refused.
//
//   Pictures that take a place. A card of level 6 or later takes the place of a picture of the unit only for the names
//   its ask of that kind lists: any other picture it carries is its own alone, whatever its name, and a screen of the
//   unit that asks for that name gets the unit's own. A card of levels 1 to 5 is asked as level 1 says, as before.
//
// Level 7: values a pack keeps in the unit's own files, with a sound (in its preset) and with a tape, each as a member
// of its own at the top of the file, under a name the pack's ask lists, read and set as JSON text (OP1F_ASK_KEEPS
// below). Base 26, the first base of level 7, adds two things that any pack may use and that need no mark: the flag
// OP1F_PAGE_STAYS of op1f_page_show, by which a page stays with its mode (below), and the page standing in for the
// sequencer's screen that gives way to the unit's own notice (level 5, "The unit's own notice": from base 27 on the
// base darkens the page and draws the notice over it).
//
// Level 8: effects. One kind of ask more; the card, and what a frame function and an input function are handed, stay
// as level 4 has them.
//
//   An effect in the unit's lists. A pack's effect shows in the unit's own list of effects, among the unit's own
//   effects in the order of their names, for every sound of the synth and the drum (shift and T3) and for the master
//   (the mixer's list), and the owner picks it there as he picks any effect. It then plays where the unit plays its
//   own: on the sound that plays, or on the unit's sum; the unit's own key switches it on and off, and T3 shows its
//   page (level 5's page that stands in, or the base's blank page with its name). Each effect stands on one of eight
//   effects of the unit's (`rides`): the unit keeps that effect's name in the sound's file, with its four knobs inside
//   that effect's ranges, switched off, and the base keeps which pack's effect plays and whether it is on, beside the
//   sound in its preset and for the slot in its own store. So a sound saved with a pack's effect plays dry on a unit
//   without the pack and on the original firmware, and plays the pack's effect again when the pack is back. The four
//   knobs, the unit's LFOs and its random key move the effect's values as they move the unit's own effect's knobs, by
//   that effect's knob travel and steps; the base maps each knob onto the value's range in the pack's numbers, and the
//   LFO pages name the values by the pack's names. One effect may play at several places; only one sound plays at a
//   time, so at most two instances run at once: the sound's and the master's, each with working memory of its own if
//   the effect asks for it.
//
//   The list, and an effect run inside another pack. The base keeps one list of every effect the packs registered.
//   Any pack may read it (op1f_rack_list), ask which one rides a sound or the master (op1f_rack_at), make an effect
//   ready at a place of its own (op1f_rack_ready) and run it
//   there from its own sound work (op1f_rack_run): a chain of effects offers a pack's effects beside the unit's own on
//   each of its links, a mixer's lane beside its other choices. The effect cannot tell a run inside from a place of
//   the unit's but by its block's `place`.
//
//   Values for the master. Level 7's values may be kept with the master's effect too (OP1F_KEEP_MASTER), in the
//   base's store: a setting an effect keeps beyond its four values (a mode, a pattern) for the master.
//
// A pack that uses level 3 needs the name op1f_level_3, the mark of a base that offers it, a pack that uses level 4
// the name op1f_level_4, a pack that uses level 5 the name op1f_level_5, a pack that uses level 6 the name
// op1f_level_6, a pack that uses level 7 the name op1f_level_7, and a pack that uses level 8 the name op1f_level_8;
// the card's source names it, so nothing is written by hand. A base of level 6 offers op1f_level_1 to op1f_level_5
// too, a base of level 7 op1f_level_1 to op1f_level_6, and a base of level 8 op1f_level_1 to op1f_level_7: a pack
// built for an earlier level runs on it as it is. Where no pack offers the mark the loader leaves the pack out and
// says so.
//
// A base that holds nothing made for one pack offers one mark more, op1f_bare_1: it knows no pack by its name, asks
// no pack's name, keeps no folder name for a pack and opens no page made for one pack over a pack's (its own top
// page, which no pack opens, is the base's and the same for every pack: see the frame's `page`). A pack that does by
// itself what an older base did for it by name (its own page, its own folders) needs op1f_bare_1, so that it is
// left out under such a base and the work is never done twice. The name sorts before op1f_level_1. A base of level 6
// or 7 or 8 offers it too.
//
// A card only grows. A later level adds fields behind the last one and raises `bytes`; a base reads the part it
// knows and takes a field the card is too short for as 0. What a frame function is handed grows the same way.
//
// The threads. A frame function runs on the display thread. An input function runs on the thread that handles keys,
// never on the display thread; for the keys of the keyboard (the ones that play notes) the firmware may hand a key
// in at once on the thread that brought it, which can be the audio thread. So an input function does what the audio
// thread may do: it writes no line on the log, it waits for nothing, it calls nothing of the firmware's, and it comes
// back quickly. It counts, and the frame function says it. The two share single words, never a structure: either can
// find the other half way through its work. Level 3's files and text are the frame function's alone: the base
// answers OP1F_NOT_NOW (op1f_text: 0) to a call it gets while it is not calling that pack's frame function, and to
// a call from any other thread while it is (it knows the display thread by its stack), so that nothing of the disk
// and no text is touched from the thread that handles keys or from the audio thread. A file is
// read or written while the frame function waits: the picture stands still for as long as it takes, some
// milliseconds for a small file. Read a file once and keep what it says; write it when something changed, not at
// every frame. Level 4's text and painting are the frame function's too (the display thread), and so is
// op1f_remove; op1f_page_show is taken from the frame function and from the input function, as op1f_page_open is.
// From level 4 an input function is told the thread (the input's `thread`): every turn and every key outside 13 to
// 53 come on the firmware's key thread, where a handler may do what the mode's own code does there; the keys 13 to
// 53 (the keys that play notes, and shift among them) may come on the audio thread, and for them the rules above
// stand.
//
// Level 5's calls come on other threads still. A sequencer's `tick` and `octave` come on the firmware's sequencer
// thread; its `key` on the thread that brings the key (the key thread, the audio thread, a MIDI handler's); its `run`
// on the key thread, a MIDI handler's or the thread of the start; a `note` of the notes heard on whatever thread asks
// for the note. For all of these the rules of an input function stand: no line on the log, no wait, nothing of the
// firmware's, quickly back; they count, and the frame function says it. One of them may come while another, or the
// frame function, stands interrupted: what they share is single words, or anything inside a section of op1f_mask.
// `shown` and `playing` come on the display thread, outside the pack's frame function: they keep what they are told
// in single words and do nothing else (no painting, no text, no file, no page: those are the frame function's).
//
// Level 6's `process` comes on the audio thread, once a block, while the unit's sound waits for it. For it the rules of
// an input function stand, and more: it allocates nothing, it calls nothing of the base's but op1f_memory, op1f_stage,
// op1f_mask and op1f_unmask, and it comes back within what it said it costs. It runs on the audio thread's own stack,
// below the unit's own frames: it uses at most OP1F_STAGE_STACK_MOST bytes of it, its own frame and those of what it
// calls together, so no large array is a local of it: a delay line, a table or a buffer of the pack's own lies in its
// working memory (OP1F_ASK_MEMORY) or in its data. It shares with the frame function single
// words, or what it reads and writes inside a section of op1f_mask of a few microseconds. Level 7's `changed` comes on
// the display thread, outside the pack's frame function, as `shown` does: it notes what it is told and does nothing
// else, and the frame function then reads and sets the values.
//
// Level 8's `process` (an effect's) comes on the audio thread, once a block, where the unit runs its own effect; the
// rules of a stage's `process` stand for it whole, with op1f_rack_run beside the four helpers it may call, and with its
// own limit of cycles (OP1F_EFFECT_CYCLES_MOST). Its `reset` comes on the display thread, outside the pack's frame
// function, as `shown` does: it clears the pack's working memory for a place and does nothing else. op1f_rack_run comes
// from a pack's own sound work on the audio thread (a stage's `process`, or an effect's) and runs another pack's effect
// there, on that thread.
//
// Nothing here names an address of the firmware. The file is compiled for the unit and, with OP1F_HOST defined, by
// the PC's compiler for the tests. The PC's build must be one with addresses of four bytes, as the unit's are: the
// sizes and places below are held when the file is compiled, and a build they do not hold for stops.

#ifndef OP1F_BUILD_R6D_CARD_H                          // R6e: base 30's guard, for the same declarations
#define OP1F_BUILD_R6D_CARD_H
#define OP1F_BUILD_R6E_CARD_H
#define OP1F_BUILD_R6_CARD_H                           // R6d: so that module/build_r6_base.h brings the base's shared names and not level 9's words again
#define OP1F_BUILD_R5_CARD_H                           // R6: so that module/build_r5_base.h brings the base's shared names and not level 8's words again

// The name a function or an object has in the linked pack. The module's compiler takes it as written here; the
// PC's compiler needs none, and the tests' stand-ins are found by their C names.
#ifdef OP1F_HOST
#define OP1F_NAMED(x)
#else
#define OP1F_NAMED(x) asm(x)
#endif

// OP1F_CARD_MARK is a card's first word. OP1F_CARD_LEVEL_1_BYTES, OP1F_CARD_LEVEL_2_BYTES and OP1F_CARD_LEVEL_3_BYTES
// are the sizes of a card of level 1, 2 and 3. OP1F_CARDS_MOST is how many cards the base's list holds: as many as
// the loader takes pack files. OP1F_RELEASE_BYTES is the room of the release text a base carries under the name
// op1f_release: the text and zeros behind it. OP1F_FOLDERS_MOST is how many folders a card may list,
// OP1F_FOLDER_TEXT the room of a folder's name and of its place (fifteen characters and the end), OP1F_FILE_NAME
// the room of a file's name in a pack's folder (thirty-nine characters and the end).
enum { OP1F_CARD_MARK = 0x44524143 /* "CARD" */, OP1F_CARD_LEVEL_1_BYTES = 44, OP1F_CARD_LEVEL_2_BYTES = 48, OP1F_CARD_LEVEL_3_BYTES = 56, OP1F_CARDS_MOST = 32,
       OP1F_RELEASE_BYTES = 16, OP1F_FOLDERS_MOST = 8, OP1F_FOLDER_TEXT = 16, OP1F_FILE_NAME = 40 };

// Level 4. OP1F_CARD_LEVEL_4_BYTES is the size of a card of level 4; OP1F_ASKS_MOST is how many asks a card may list,
// OP1F_ASK_BYTES_MOST the most bytes one ask's table may have.
enum { OP1F_CARD_LEVEL_4_BYTES = 64, OP1F_ASKS_MOST = 16, OP1F_ASK_BYTES_MOST = 4096 };

// Level 5. A card of level 5 is a card of level 4's size: OP1F_CARD_LEVEL_5_BYTES. OP1F_SEQUENCERS_MOST is how many
// sequencers the packs may add in all, OP1F_NOTES_MOST how many asks may hear the notes, OP1F_KIND_NAME the room of an
// ask's name (fifteen characters and the end).
enum { OP1F_CARD_LEVEL_5_BYTES = 64, OP1F_SEQUENCERS_MOST = 16, OP1F_NOTES_MOST = 8, OP1F_KIND_NAME = 16 };

// What level 3's helpers answer when they do not do what was asked (each is less than 0):
//   OP1F_NO_FILE     op1f_read, op1f_remove: the folder holds no file of that name
//   OP1F_TOO_LARGE   op1f_read: the file is larger than the room handed in (nothing is read); op1f_path: the room
//                    handed in is too small for the path
//   OP1F_NOT_NOW     the call did not come from this pack's frame function, or (op1f_write, op1f_remove) the disk mode
//                    is open
//   OP1F_NO_FOLDER   the card is not one the base has in use, or the folder's number is not one of its folders the
//                    base made
//   OP1F_BAD_NAME    a file's name is empty, longer than thirty-nine characters, holds a slash or a character that
//                    is not plain, or begins with a dot
//   OP1F_FAILED      the disk did not do it: the line on the log says which step
enum { OP1F_NO_FILE = -1, OP1F_TOO_LARGE = -2, OP1F_NOT_NOW = -3, OP1F_NO_FOLDER = -4, OP1F_BAD_NAME = -5, OP1F_FAILED = -6 };

struct Op1fCard;

// What a card's frame function is handed, the same for every card of a frame.
//   bytes    the size of what is handed: 28 at level 1, 32 at level 2, 36 at level 3, 40 at level 4, 48 on base 33
//            (an Op1fFrameHeld, at the end of this file). A later base may hand more; read what you know
//   frame    the number of the frame, counted by the base
//   ms       milliseconds since the base's first frame: what op1f_ms answers. It moves once a frame
//   mode     the mode the unit is in, by the firmware's own number: 0 synth, 1 drum, 2 tape, 3 mixer. A pack with a
//            sequencer (level 5) also sees 7 here: the sequencer's screen is up
//   screen   the number of the screen that is shown
//   shift    not 0 while shift is held
//   popup    not 0 while the power popup is on the screen: the last moment to say something on the log
//   page     level 2: the card whose page the GUI shows now, 0 when it shows none. Compare it with your own card's
//            address: it is yours when they are the same. The base takes a page asked for at its next frame and the
//            GUI shows it at the frame after, from which on `page` names it; it stops when the page is closed or when
//            the mode changes. Level 4: while a page shown with op1f_page_show lies over another, `page` names the one on top.
//            A page of the base's own (its top page, which asks the owner to confirm something) may lie over a pack's
//            page for a moment: the pack's page is still the one `page` names, and no key or knob reaches any pack
//            until it is gone; from base 38 `canvas` is 0 from the frame it is asked for until it is gone, so that
//            nothing a pack paints hides it. From base 26 on, a page shown with OP1F_PAGE_STAYS is not closed when the mode changes:
//            while the unit is in another mode `page` is 0, and it names the page again once the unit is back in
//            the page's mode
//   disk     level 3: not 0 while the disk mode is open (the COM key, the computer connected): the computer owns the
//            folders and op1f_write answers OP1F_NOT_NOW. When it goes back to 0 the computer may have brought,
//            changed or deleted files in a shown folder
//   canvas   level 4: this frame's picture as the display will show it, already drawn, for the op1f_paint_ helpers,
//            which take it; 0 when the base offers none (base 26 only: also for a frame in which the unit's own
//            notice lies over a page standing in; see OP1F_SHOWN_COVERED. From base 38 also while the base's own top
//            page is asked for or up; see `page`). It lives for the call only: never keep
//            it, and never write it yourself (its layout is the base's to know). What a pack paints lies over
//            everything for this frame and is gone at the next: paint again at every frame (base 33: unless the
//            page holds its picture; see "Base 33" at the end)
struct Op1fFrame { unsigned bytes, frame, ms, mode, screen, shift, popup; const Op1fCard* page; unsigned disk; void* canvas; };   // 40 bytes at level 4

// What happened, for an input function (level 2).
//   OP1F_DOWN        a key went down
//   OP1F_DOWN_SHIFT  a key went down while shift was held. The firmware hands these on apart, and so does the base
//   OP1F_UP          a key came up. An up goes where its down went: to the pack that took the down, whatever its
//                    input function answers, and never to another; to the firmware where the firmware had the down.
//                    So a pack sees the up of every key whose down it took, and of no other
//   OP1F_TURN        a knob was turned
enum { OP1F_DOWN = 1, OP1F_DOWN_SHIFT = 2, OP1F_UP = 3, OP1F_TURN = 4 };

// What an input function answers, besides 0 (let the event go on) and any other number (take it):
//   OP1F_SEEN        level 4: the event goes on to the other packs and to the firmware as if the answer had been 0,
//                    and the pack is handed that key's up too, before the up goes where its down went. For a turn it
//                    is the same as 0. A base of level 3 or earlier takes it as any answer not 0: the event is taken.
//                    So a pack that answers it needs op1f_level_4
enum { OP1F_SEEN = 2 };

// The thread an event came on (the input's `thread`, level 4).
//   OP1F_KEY_THREAD  the firmware's key thread: every turn, and every key outside 13 to 53 (the keys under the
//                    screen, the mode keys, the help and metronome keys, the knobs' presses and clicks). There a
//                    handler may do what the mode's own code does there
//   0                the keys 13 to 53: the keys that play notes, and shift (38) among them. They may come on the
//                    audio thread: a handler of such a key does only what the audio thread may do (see "The threads"
//                    above)
enum { OP1F_KEY_THREAD = 1 };

// What an input function is handed (level 2), one event at a time.
//   bytes    the size of what is handed: 36 at level 2, 40 at level 4. A later base may hand more; read what you know
//   what     OP1F_DOWN, OP1F_DOWN_SHIFT, OP1F_UP or OP1F_TURN
//   key      the key's number as the firmware counts its keys (0 the help key, 1 the metronome key, 2 to 5 the
//            synth, drum, tape and mixer keys, 6 to 9 the four keys under the screen, 38 shift, 54 to 57 the knobs'
//            clicks, 61 to 64 the knobs pressed down and let go); for a turn, the knob: 0 blue, 1 the second, 2 the
//            third, 3 red. The firmware makes a click (54 to 57, a down and then an up) when a knob that was pressed
//            comes up without having been turned
//   turn     for a turn: 1024 a step of the knob, more than 0 one way and less the other, as the firmware's own pages
//            take it: a value goes up for more than 0. 0 for a key
//   mode     the mode the unit is in: 0 synth, 1 drum, 2 tape, 3 mixer; and, as in the frame, 7 while the sequencer's
//            screen is up
//   screen   the number of the screen that is shown
//   shift    not 0 while shift is held
//   mine     not 0 when this pack's page is the one the base shows
//   record   the firmware's own record of a key that went down, sixteen bytes: the key's number, the value (127, or
//            how hard a key of the keyboard was struck), and two words of the clock at the press. 0 for an up and a
//            turn, which the firmware hands on as numbers. It lives only for the call (for a knob's click it stands
//            in the firmware's own stack): copy what you need, never keep the pointer
//   thread   level 4: OP1F_KEY_THREAD or 0, by the key's number, as above
struct Op1fInput { unsigned bytes, what, key; int turn; unsigned mode, screen, shift, mine; const unsigned* record; unsigned thread; };   // 40 bytes at level 4

// One of a pack's folders (level 3), constant data in the pack's body, written by tools/pack_build.py from the
// recipe. Each text is zero padded and ended within its sixteen bytes.
//   name     what the computer shows in the disk mode
//   place    where the folder lies on the unit's disk, inside user/pf/
//   shown    not 0 for a folder the computer sees in the disk mode
struct Op1fFolder { char name[OP1F_FOLDER_TEXT]; char place[OP1F_FOLDER_TEXT]; unsigned shown; };   // 36 bytes

// One thing a pack asks of the base (level 4), constant data in the pack's body, written by tools/pack_build.py from
// the recipe's `asks`. Level 4 defines no kind; level 5 defines two, a sequencer and the notes heard; level 6 three
// more, a sound stage, working memory and pictures that take a place; level 7 one, values kept in the unit's files
// (below). A later level adds kinds (an effect, an engine), and their helpers, and never a field of the card.
//   kind     what is asked; 0 is no kind
//   bytes    the size of what `what` points at, at most OP1F_ASK_BYTES_MOST
//   what     a table of that kind's form in the pack's body, or 0
struct Op1fAsk { unsigned kind, bytes; const void* what; };   // 12 bytes

// A pack's card: constant data in the pack's body, written by tools/pack_build.py from the recipe under the name
// op1f_card_ and the pack's name in small letters, digits and underscores (op1f_card_wink). Its addresses are the
// pack's own and move with it; each is 0 where the pack has no such thing.
//   frame     called once a frame on the display thread, in the order of the packs' file names, before the base
//             dresses its own screens. It is handed the frame (above) and answers nothing. It must come back
//             quickly, every time: the picture stands still for as long as it runs. The base times every call and
//             prints the time on the log by the pack's name. A function that never comes back stops the display
//             for good, and no line names the pack. A function written in assembly keeps the registers the
//             calling convention says a function keeps
//   pictures  the pack's directory of pictures (tools/pack_build.py `pictures`): pictures for screens of the
//             firmware, by the file names the firmware asks for, and the pack's own. The cards are asked in their
//             order, each pack's pick first: the first pack that has a name serves it. A name the
//             firmware never asks for is the pack's alone. No picture may be larger than a megabyte: the base does
//             not use a card whose directory holds one
//   pick      asked before the pack's directory is looked into: handed the name a screen asks for, it answers the
//             name to look for in this pack's directory, or 0 for nothing of this pack's now. With no such function
//             the directory is looked into by the name itself. It is asked every time the firmware asks for a
//             picture: while the firmware makes its screens at the start, before the first frame, and again later,
//             when a tape is loaded and when a page of the base is opened. A later call is not always on the
//             display thread, and may come while the pack's own frame function stands interrupted. So it only
//             reads; what it shares with the frame function is single words, never a structure, which it could
//             find half written; and it takes no time.
//             Level 6: for a card that says level 6 or later and is of level 6's size, a screen of the firmware is
//             served from the pack's directory, or `pick` is asked, only for a name its ask of kind OP1F_ASK_PLACES
//             lists; a name it does not list is never served to a screen by this pack, whatever the pack carries.
//             The pack's own request (op1f_picture) still finds any picture of its directory, by any name
//   input     level 2, read only from a card that says level 2 or later: offered every key and knob event, on the
//             thread that handles keys (see "The threads" above),
//             before the firmware's code for the mode. It answers not 0 to take the event: then the firmware never
//             sees it, and no pack after this one is asked. It answers 0 to let it go on. It is asked first when the
//             pack's page is the one shown, else in the order of the files' names. An up is not offered: it goes where
//             its down went. Level 4, from a card that says level 4 or later and is of level 4's size: it may answer
//             OP1F_SEEN (above) to see a key and let it go on
//   folders   level 3, read only from a card that says level 3 or later and is of level 3's size: the pack's list of
//             folders (above), 0 for none
//   folder_count  level 3: how many folders the list holds, 0 to OP1F_FOLDERS_MOST; 0 for none
//   asks      level 4, read only from a card that says level 4 or later and is of level 4's size: the pack's list of
//             asks (above), 0 for none. The base does not use a card whose list is not in order (more than
//             OP1F_ASKS_MOST, a count with no list or a list with no count) or does not lie in its pack (the list,
//             or an ask's table with its bytes, outside the pack's body, or not on a word). Level 5: an ask whose
//             table lies in its place but is not in order for its kind (its size, its name, a flag the base does not
//             know, a function that is not the pack's) is left aside with a line, and the card is used
//   ask_count level 4: how many asks the list holds, 0 to OP1F_ASKS_MOST; 0 for none
struct Op1fCard {
    unsigned mark, bytes, level, version;        // the mark; 44 at level 1, 48 at level 2, 56 at level 3, 64 at levels 4 to 7; the highest level used; the pack's version
    char name[16];                               // the pack's name, zero padded
    void (*frame)(const Op1fFrame* now);         // +32: once a frame, on the display thread; 0 for none
    const void* pictures;                        // +36: the pack's directory of pictures; 0 for none
    const char* (*pick)(const char* name);       // +40: which of its pictures a name stands for now; 0: the name itself
    int (*input)(const Op1fInput* event);        // +44, level 2: a key or a knob; not 0 when the pack took it; 0 for none
    const Op1fFolder* folders;                   // +48, level 3: the pack's folders; 0 for none
    unsigned folder_count;                       // +52, level 3: how many
    const Op1fAsk* asks;                         // +56, level 4: the pack's asks; 0 for none
    unsigned ask_count;                          // +60, level 4: how many, 0 to OP1F_ASKS_MOST
};

// The list the base offers under the name op1f_cards, in its zeroed data, and the loader fills once the packs are
// joined: one slot for each pack that runs and has a card, in the order of the files' names. `first` is the pack's
// first address and `end` the one behind its last (its body and its zeroed data), so that the base can hold every
// address of a card to the pack it came with. `count` is the slots filled; `more` is the cards that found no slot.
// A pack with a card has no business with the list: it is between the loader and the base.
struct Op1fCardSlot { const Op1fCard* card; unsigned first, end; };
struct Op1fCards { unsigned count, more; Op1fCardSlot slots[OP1F_CARDS_MOST]; };   // 392 bytes

// Level 5. The kinds of ask a base of level 5 knows.
//   OP1F_ASK_SEQUENCER  a sequencer: what `what` points at is an Op1fSequencer
//   OP1F_ASK_NOTES      the notes heard: what `what` points at is an Op1fNotes
// Kinds to come keep the same form: a head (below), then the functions the base calls for that kind.
enum { OP1F_ASK_SEQUENCER = 1, OP1F_ASK_NOTES = 2 };

// What `shown` is told (the head's function, below): bits, 0 when the page is not on the screen.
//   OP1F_SHOWN_UP       the thing's page is the one on the screen
//   OP1F_SHOWN_OFF      the thing is switched off (a sequencer: its instrument's sequencer is off)
//   OP1F_SHOWN_COVERED  the unit's own notice lies over the page (a sequencer: the firmware's "sequencer off"): paint
//                       as usual. From base 27 on the base darkens the whole frame after every frame function and
//                       draws the unit's notice over it, as over the unit's own sequencers (base 26 put a page of its
//                       own that draws nothing in the page's place and handed no pack a canvas)
enum { OP1F_SHOWN_UP = 1, OP1F_SHOWN_OFF = 2, OP1F_SHOWN_COVERED = 4 };

// The head of every kind's table (level 5): constant data in the pack's body, like the card, written by
// tools/pack_build.py from the recipe's `asks`.
//   bytes  the table's size as the pack was built: a later level grows a kind's table behind, never inside
//   name   the thing's name: 1 to 15 characters of a-z, 0-9, '-' and '_', zero padded. For a sequencer it is what
//          the owner sees in the list of sequencers and what the unit saves as the choice; for a kind with nothing
//          to show, what the base's lines on the log call it
//   flags  the kind's flags; a bit the base does not know makes it leave the ask aside
//   shown  for a kind with a page: called on the display thread, outside the pack's frame function, at each frame
//          at which what it says changed: `how` is OP1F_SHOWN_UP and the bits beside it while the thing's page is
//          the one on the screen, 0 when it no longer is. `where` says which of the thing's places (a sequencer: its
//          instrument, 0 the synth, 1 the drum). It comes before the frame functions of the same frame: from the
//          frame at which it says OP1F_SHOWN_UP, the frame's `page` is the pack's card. It keeps what it is told and
//          does no more. 0 for none; 0 for every kind of levels 6 and 7, which have no page
struct Op1fKindHead { unsigned bytes; char name[OP1F_KIND_NAME]; unsigned flags; void (*shown)(unsigned where, unsigned how); };   // 28 bytes

// A sequencer's flags.
//   OP1F_SEQ_SYNTH_ONLY  not in the drum's list
//   OP1F_SEQ_ALL_MIDI    MIDI notes beyond the keyboard's two octaves come to `key` too. Without it such a note is
//                        dropped while this sequencer is on, as with those of the firmware's own that do not take them
enum { OP1F_SEQ_SYNTH_ONLY = 1, OP1F_SEQ_ALL_MIDI = 2 };

// Which clock leads (the tick's `clock`): the unit's own, the tape's while it plays, or one from outside over MIDI.
enum { OP1F_CLOCK_OWN = 0, OP1F_CLOCK_TAPE = 1, OP1F_CLOCK_MIDI = 2 };

// What `tick` is handed, once a pulse of the clock, 96 pulses a quarter note.
//   bytes    the size of what is handed: 32 at level 5. A later base may hand more; read what you know
//   host     0 the synth, 1 the drum: the instrument the unit is on, the only one whose sequencer ticks
//   pulse    the firmware's own count of pulses, 0 to 47
//   count    the base's count of this sequencer's ticks since it was switched on or last stopped, from 0. It is not
//            set back when the firmware's count is
//   tempo    beats a minute in hundredths (12000 is 120)
//   us       microseconds a pulse at that tempo
//   clock    OP1F_CLOCK_OWN, OP1F_CLOCK_TAPE or OP1F_CLOCK_MIDI
//   jumped   not 0 when the firmware set its count back since the last tick (the tape started, a key restarted it)
struct Op1fTick { unsigned bytes, host, pulse, count, tempo, us, clock, jumped; };   // 32 bytes

// Where a key came from (the key's `from`).
enum { OP1F_FROM_KEYS = 0, OP1F_FROM_MIDI = 1 };

// What `key` is handed: a key of the keyboard or a MIDI note, before the instrument plays it: every down, and the up
// of each down this sequencer took. Where its `key` answered 0 for a down, the instrument plays the note and ends it
// itself when the key comes up, and the sequencer is not told of that up: a sequencer that wants to know how long a
// key is held takes the key and plays the note itself (op1f_note_on at the down, op1f_note_off at the up), as the
// firmware's own sequencers do; or it lets the key play and asks to hear the notes as well.
//   bytes     the size of what is handed: 32 at level 5
//   what      OP1F_DOWN, or OP1F_UP for a key or note whose down it took
//   host      0 the synth, 1 the drum
//   key       the keyboard's key, 0 to 23 from the left; for a MIDI note, the note - 53 (with OP1F_SEQ_ALL_MIDI it
//             may be below 0 or above 23). An up is matched to its down by `key`: the octave may have moved between
//             the two, and `note` with it
//   note      the note the instrument would play for it now, the octave counted, 0 to 127
//   velocity  1 to 127 for a down, 0 for an up
//   shift     not 0 when the key went down while shift was held; 0 for an up
//   from      OP1F_FROM_KEYS or OP1F_FROM_MIDI
struct Op1fKey { unsigned bytes, what, host; int key; unsigned note, velocity, shift, from; };   // 32 bytes

// `run`'s `how`.
//   OP1F_RUN_ON    the sequencer was switched on
//   OP1F_RUN_STOP  stop sounding now: it was switched off, another sequencer was chosen for its instrument, or the
//                  unit changes instrument. A STOP may come twice, or with nothing sounding. Once the pack's `run`
//                  has come back the base ends every note the pack left sounding. A sequencer that is still on is
//                  ticked again when its instrument is the unit's again
enum { OP1F_RUN_STOP = 0, OP1F_RUN_ON = 1 };

// A sequencer: what its ask points at. `tick` or `key` must be there; the other three may be 0. What a sequencer
// keeps (its settings, its patterns) is its own, in the pack's folders.
//   tick     called each pulse while the sequencer is on, its instrument is the one the unit is on and no sound is
//            being loaded
//   key      called for a key of the keyboard or a MIDI note while the sequencer is on. For a down it answers not 0 to
//            take the key: the instrument does not play it, and the key's up comes to `key` too. It answers 0 to let
//            the instrument play it as usual: then no up comes for it (above). What it answers for an up is not used
//   run      called when the sequencer is switched on and when it must stop sounding (above)
//   octave   called at the pulse after an octave key, while on, with the steps the octave moved. It answers not 0 (or
//            is not there) to have the notes of the keys held on the keyboard follow the move now, 0 to leave them.
//            The notes the pack began never move by themselves. The octave keys are 36 and 37 as the firmware counts
//            its keys: a pack that wants to see them go down itself watches them from its input function (OP1F_SEEN)
//   playing  asked when the unit draws its "playing" mark: not 0 shows it. Not there: 1
struct Op1fSequencer {
    Op1fKindHead head;                                // +0
    void (*tick)(const Op1fTick* now);                // +28
    int  (*key)(const Op1fKey* event);                // +32
    void (*run)(unsigned host, unsigned how);         // +36
    int  (*octave)(unsigned host, int steps);         // +40
    int  (*playing)(unsigned host);                   // +44
};   // 48 bytes at level 5

// What a pack that hears the notes is told (the note's `what`).
enum { OP1F_NOTE_ENDS = 0, OP1F_NOTE_BEGINS = 1 };

// A note's flags: for op1f_note_on and op1f_note_off, and in what a pack that hears the notes is handed.
//   OP1F_NOTE_NO_MIDI  not sent out over MIDI as well
enum { OP1F_NOTE_NO_MIDI = 1 };

// What a pack that hears the notes is handed, one note at a time, as the instrument is asked and before it plays.
//   bytes     the size of what is handed: 24 at level 5
//   what      OP1F_NOTE_BEGINS or OP1F_NOTE_ENDS
//   host      0 the synth, 1 the drum: the instrument the unit is on, which is asked. Another number while the unit
//             is on neither: the note then goes out over MIDI only
//   note      0 to 127
//   velocity  1 to 127 for a note that begins, 0 for one that ends
//   flags     OP1F_NOTE_NO_MIDI when the note is not sent out over MIDI as well
struct Op1fNote { unsigned bytes, what, host, note, velocity, flags; };   // 24 bytes

// The notes heard: what its ask points at. The head's `flags` is 0 and its `shown` is 0: there is nothing to show.
//   note     called for each note, on whatever thread asks for it. It answers nothing and changes nothing: it may
//            not begin or end a note itself
struct Op1fNotes {
    Op1fKindHead head;                                // +0
    void (*note)(const Op1fNote* event);              // +28
};   // 32 bytes at level 5

// Levels 6 and 7. The kinds of ask a base of level 6 knows, beside level 5's two, and the kind level 7 adds. Each table
// begins with level 5's head: its `name` is what the base's lines call the ask, its `shown` is 0 (none of these has a
// page), and its `flags` is 0 but where a kind says otherwise.
//   OP1F_ASK_STAGE   level 6: a function in the sound's path: what `what` points at is an Op1fStage
//   OP1F_ASK_MEMORY  level 6: working memory: an Op1fMemory
//   OP1F_ASK_PLACES  level 6: the pictures of the unit this pack's pictures take the place of: an Op1fPlaces
//   OP1F_ASK_KEEPS   level 7: values kept in the unit's files: an Op1fKeeps
// A card of level 6 or 7 is a card of level 4's size: OP1F_CARD_LEVEL_6_BYTES, OP1F_CARD_LEVEL_7_BYTES.
enum { OP1F_ASK_STAGE = 3, OP1F_ASK_MEMORY = 4, OP1F_ASK_PLACES = 5, OP1F_ASK_KEEPS = 6 };
enum { OP1F_CARD_LEVEL_6_BYTES = 64, OP1F_CARD_LEVEL_7_BYTES = 64 };

// Level 6: a sound stage.
//
// The points of the sound's path (a stage's `point`, and the block's):
//   OP1F_AT_INSTRUMENT  an instrument after its effect. The synth's sound and the drum's each come to the stage in a
//                       call of their own, the block's `host` saying which
//   OP1F_AT_MASTER_IN   the unit's whole sum, before the master effect
//   OP1F_AT_MASTER_OUT  the unit's whole sum, after the master effect
// At each point the unit's own code still works on the buffers after the stages, as it did before them.
enum { OP1F_AT_INSTRUMENT = 1, OP1F_AT_MASTER_IN = 2, OP1F_AT_MASTER_OUT = 3 };

// A stage's flag, in its head's `flags`.
//   OP1F_STAGE_LISTENS  it reads the sound and never changes it: it is called after every stage at its point that
//                       changes the sound, and is handed a copy of the buffers, so that what it writes there is never
//                       heard. A pack that wants to change the sound leaves this flag out
enum { OP1F_STAGE_LISTENS = 1 };

// The budget, in the processor's cycles (a block of 64 frames has some 580,000):
//   OP1F_STAGE_CYCLES_MOST  the most one stage may declare for one call
//   OP1F_STAGE_CYCLES_ALL   the most all stages together may declare for a block. A stage at the instrument is called
//                           twice a block, for the synth and for the drum, and counts twice
//   OP1F_STAGE_CYCLES_OFF   a call that takes more than this switches its stage off at once
//   OP1F_STAGE_OVER_CALLS   a stage is switched off when, within one second, this many of its calls went over: took
//                           more than twice what it declared and more than what it declared and OP1F_STAGE_CYCLES_SPARE
//   OP1F_STAGE_CYCLES_SPARE the allowance a call has beyond what it declared before it counts as over. A call is timed
//                           from its start to its end, and the time measured includes whatever interrupts the unit
//                           served meanwhile: the allowance keeps a light stage that keeps to its cost from being
//                           switched off for them
//   OP1F_STAGES_MOST        how many stages the packs may ask in all
//   OP1F_STAGE_STACK_MOST   the most bytes of stack one call of `process` may use, with what it calls (see "The threads")
// An ask beyond the first two limits, or beyond the last, is left aside at the start with a line, and its card is used.
// A stage switched off stays off until the next start, with a line on the log by its pack's name.
enum { OP1F_STAGE_CYCLES_MOST = 116000, OP1F_STAGE_CYCLES_ALL = 174000, OP1F_STAGE_CYCLES_OFF = 290000, OP1F_STAGE_OVER_CALLS = 8,
       OP1F_STAGES_MOST = 16, OP1F_STAGE_CYCLES_SPARE = 20000, OP1F_STAGE_STACK_MOST = 1024 };

// What `process` is handed, once a block, at its point (for a stage at the instrument, once for each instrument).
//   bytes     the size of what is handed: 36 at level 6. A later base may hand more; read what you know
//   point     the stage's point, OP1F_AT_...
//   host      0 the synth, 1 the drum, 2 the unit's sum (both master points)
//   frames    64, the frames of the block
//   left      the left buffer, `frames` words of 32 bits, signed, each a Q1.31 fraction: full scale is -2^31 to
//             2^31 - 1. A stage that changes the sound writes its sound here, in place, and holds it at full scale
//             (a sum that wraps is a loud click); a stage that listens is handed a copy
//   right     the right buffer, the same
//   period    the length of a 96th of a beat at the unit's tempo, in samples at 44,100 a second times 2^18 (about
//             60,211,200 at 120 beats a minute). The word is large: times 96 it no longer fits in 32 bits. A beat is
//             (period x 3) >> 13 samples, which fits above 5 beats a minute; a sixteenth (period >> 6) x 24 >> 12
//             samples. No division
//   playing   not 0 while the tape plays
//   block     how many times this stage was called before at this point and host: 0 at its first call
// It lives for the call only: never keep it, nor the buffers' addresses.
struct Op1fBlock { unsigned bytes, point, host, frames; int* left; int* right; unsigned period, playing, block; };   // 36 bytes at level 6

// A sound stage: what its ask points at.
//   point    OP1F_AT_INSTRUMENT, OP1F_AT_MASTER_IN or OP1F_AT_MASTER_OUT
//   cycles   what one call takes at most, in the processor's cycles: 1 to OP1F_STAGE_CYCLES_MOST. The base times every
//            call, interrupts served during it included: a stage that, within one second, takes more than twice this
//            and more than this and OP1F_STAGE_CYCLES_SPARE in OP1F_STAGE_OVER_CALLS calls, or more than
//            OP1F_STAGE_CYCLES_OFF in one, is switched off until the next start
//   memory   the number of the card's ask of kind OP1F_ASK_MEMORY that this stage needs, from 0, plus 1; 0 for none.
//            The stage is not called before that memory was handed over, and never when it was refused
//   process  called once a block on the audio thread (see "The threads" above); it answers nothing. Not 0. It uses at
//            most OP1F_STAGE_STACK_MOST bytes of stack: a buffer larger than a few words is no local of it
struct Op1fStage {
    Op1fKindHead head;                                // +0
    unsigned point, cycles, memory;                   // +28
    void (*process)(const Op1fBlock* now);            // +40
};   // 44 bytes at level 6

// Level 6: working memory. From base 41 the base hands out the unit's spare memory first, and the unit's heap only for
// an ask that no longer fits there; a base before 41 gave 16 MiB for one ask and for all asks together, all from the
// heap, and 16 asks (a pack that counts on more names the mark op1f_memory_2).
//   OP1F_MEMORY_MOST        the most one ask may have (before base 41: 16 MiB, for one ask and all asks together)
//   OP1F_MEMORY_SPARE       base 41: the unit's spare memory in bytes, 95.75 MiB that the base hands out first for packs' working memory
//   OP1F_MEMORY_HEAP_MOST   base 41: the most all asks together take from the heap
//   OP1F_MEMORY_KEPT        what the unit keeps free of its heap: an ask after which less would stay free there is
//                           refused
//   OP1F_MEMORY_ALIGN_MOST  the most an ask's alignment may be
//   OP1F_MEMORIES_MOST      how many asks of this kind the packs may make in all (before base 41: 16)
enum { OP1F_MEMORY_MOST = 32 << 20, OP1F_MEMORY_KEPT = 8 << 20, OP1F_MEMORY_ALIGN_MOST = 4096, OP1F_MEMORIES_MOST = 64,
       OP1F_MEMORY_SPARE = 100401152, OP1F_MEMORY_HEAP_MOST = 16 << 20 };

// Working memory: what its ask points at. The head's `flags` is 0.
//   bytes  1 to OP1F_MEMORY_MOST
//   align  where the block begins: on a multiple of `align`, a power of two from 4 to OP1F_MEMORY_ALIGN_MOST
// The base takes the memory at its first frame, in the order of the files' names and then of each card's asks, and
// clears it to 0 over the frames that follow (a quarter of a megabyte a frame); op1f_memory answers it from then on.
struct Op1fMemory {
    Op1fKindHead head;                                // +0
    unsigned bytes, align;                            // +28
};   // 36 bytes at level 6

// Level 6: pictures that take a place. OP1F_PLACES_MOST is how many names one ask may list, OP1F_PLACE_NAME the room
// of one (twenty-three characters and the end).
enum { OP1F_PLACES_MOST = 256, OP1F_PLACE_NAME = 24 };

// From base 31: pictures that give way. The bit of a places ask's head `flags`:
//   OP1F_PLACES_GIVES_WAY  this ask's pictures give way: a screen is handed one of them only when no pack serves that
//                          name firmly (from an ask of places without the bit). A screen's picture is served by the
//                          first pack, in the order of the files' names, that has it and does not give way; only when
//                          none does, by the first that gives way. A card may carry one firm ask of places and then
//                          one that gives way, in that order, with no name in both lists. Bases 24 to 30 take the
//                          first ask of places and stop: they leave the giving-way ask aside with a line on the log,
//                          the card runs, and its giving-way pictures are not served there
enum { OP1F_PLACES_GIVES_WAY = 1 };

// Pictures that take a place: what its ask points at; at most one firm such ask a card and, from base 31, one that
// gives way after it. The head's `flags` is 0, or OP1F_PLACES_GIVES_WAY on the ask that gives way.
//   count  how many names the list holds, 1 to OP1F_PLACES_MOST
//   names  the pack's own list of `count` names, constant data in the pack's body, on a word: the names of the unit's
//          own pictures, as a screen of the unit asks for them (each ending in .svg, zero padded within its
//          OP1F_PLACE_NAME bytes), whose place this pack's pictures of the same names take. A name a screen asks for
//          that is not in the list is never served by this pack
struct Op1fPlaces {
    Op1fKindHead head;                                // +0
    unsigned count;                                   // +28: how many names, 1 to OP1F_PLACES_MOST
    const char (*names)[OP1F_PLACE_NAME];             // +32: the pack's own list of `count` names, each zero padded
};   // 36 bytes at level 6

// Level 7: values kept in the unit's files. The kinds of value a base of level 7 keeps: whole numbers from -2147483648
// to 2147483647, true and false, null, texts (with no zero character in them), and lists and objects of these, nested
// up to four deep, an object naming each of its members once. A number with a fraction or an exponent is not one of
// them: op1f_value_set answers OP1F_BAD_VALUE for it, and a member of a file that holds one is not taken from the file
// (the file keeps it as the unit alone keeps it). An object's members come back in the order of their names once the
// value has been through a file. The values of one slot, or of the tape, take at most 4 KiB in the base (about as much
// as their text; a list of small numbers, two bytes a number): op1f_value_set answers OP1F_TOO_LARGE beyond.
//
// Where a value is kept: a key's `files` (bits), and `where` in the helpers and in `changed`.
//   OP1F_KEEP_SYNTH  with the synth's sounds: in a preset of the synth, and with each of its slots
//   OP1F_KEEP_DRUM   with the drum's sounds, the same
//   OP1F_KEEP_TAPE   with the tape, in its own file
enum { OP1F_KEEP_SYNTH = 1, OP1F_KEEP_DRUM = 2, OP1F_KEEP_TAPE = 4 };

// What `changed` says (its `how`):
//   OP1F_KEPT_LOADED   a sound was loaded onto the slot from a preset, or a tape was loaded (from base 26 on, a tape
//                      switched to: one the unit has just made is OP1F_KEPT_MADE below)
//   OP1F_KEPT_ENGINE   the slot's engine changed: the values stay the slot's
//   OP1F_KEPT_SAVED    the values were written: into a preset of the slot's sound, or into the tape's file
//   OP1F_KEPT_START    the start: the slot's values as the base kept them, or none
//   OP1F_KEPT_DROPPED  the start: the values the base kept for the slot did not fit the sound on it, and were let go
//   OP1F_KEPT_MADE     (from base 26 on; for the tape alone) a tape the unit has just made was loaded: a new tape (the
//                      tape list's T3), which holds no value, or a copy of one (shift and T3), which holds the
//                      original's values as its file does. It is told in place of OP1F_KEPT_LOADED for such a load: read
//                      the values as after OP1F_KEPT_LOADED, and set what the pack keeps of the tape's own state as the
//                      unit sets its own for a new tape or a copy (the levels and pans of its tracks back to their start,
//                      for instance), where a tape switched to (OP1F_KEPT_LOADED) is taken as it was left
enum { OP1F_KEPT_LOADED = 1, OP1F_KEPT_ENGINE = 2, OP1F_KEPT_SAVED = 3, OP1F_KEPT_START = 4, OP1F_KEPT_DROPPED = 5 };
enum { OP1F_KEPT_MADE = 6 };

// OP1F_KEY_BYTES is the room of a member's name (thirty-one characters and the end), OP1F_KEYS_MOST how many members an
// ask may list, OP1F_VALUE_MOST the most bytes of a value's text. OP1F_BAD_VALUE is what op1f_value_set answers for a
// text that is not JSON or a kind of value the base cannot write.
enum { OP1F_KEY_BYTES = 32, OP1F_KEYS_MOST = 8, OP1F_VALUE_MOST = 2048, OP1F_BAD_VALUE = -7 };

// One member a pack keeps.
//   name   its name at the top of the file: 1 to 31 characters of a-z, 0-9 and '_', zero padded. A name the unit's own
//          file already has is never written: choose one of the pack's own (its name and a word)
//   files  where it is kept: OP1F_KEEP_ bits, not 0
struct Op1fKeptKey { char name[OP1F_KEY_BYTES]; unsigned files; };   // 36 bytes

// Values kept in the unit's files: what its ask points at; at most one such ask a card. The head's `flags` is 0.
//   changed  called on the display thread, before the frame functions of the frame, outside the pack's frame function:
//            `where` OP1F_KEEP_SYNTH, OP1F_KEEP_DRUM or OP1F_KEEP_TAPE; `which` the slot, 0 to 7 (0 for the tape);
//            `how` above. It notes and does nothing else: the frame function then reads the values with
//            op1f_value_get. 0 for none
//   count    how many keys follow, 1 to OP1F_KEYS_MOST
//   keys     the members
// A pack cannot make the unit save: its values are written when the unit writes the file, the preset when the owner
// saves it, the tape when the unit writes the tape, as with the unit's own members.
struct Op1fKeeps {
    Op1fKindHead head;                                                   // +0
    void (*changed)(unsigned where, unsigned which, unsigned how);       // +28
    unsigned count;                                                      // +32
    Op1fKeptKey keys[OP1F_KEYS_MOST];                                    // +36
};   // 324 bytes at level 7

// Level 7, from level 8: the master as a place for values. A card of level 8 may name OP1F_KEEP_MASTER in a key's
// `files`: the value is kept with the master's effect, in the base's own store on the unit's disk (the unit keeps no
// file of the master's own a pack may write in), across a start; `which` is 0 for it. A card of a level before 8
// takes the bit as no place, and a key with nothing else is left aside; a base before 28 leaves aside any key that
// names it.
enum { OP1F_KEEP_MASTER = 8 };

// Level 8: effects. The kind of ask a base of level 8 knows, beside those of levels 5 to 7. A card of level 8 is a
// card of level 4's size: OP1F_CARD_LEVEL_8_BYTES.
//   OP1F_ASK_EFFECT  an effect in the unit's lists of effects: what `what` points at is an Op1fEffect
enum { OP1F_ASK_EFFECT = 7 };
enum { OP1F_CARD_LEVEL_8_BYTES = 64 };

// Where an effect plays: a place (`place` in the block and in the helpers; `where` in the head's `shown`).
//   0 to 7                   a sound of the synth, by its slot
//   OP1F_PLACE_DRUM + slot   a sound of the drum, by its slot (8 to 15)
//   OP1F_PLACE_MASTER        the master (16)
//   OP1F_PLACE_HOSTED + n    run inside a pack's own sound work, at that pack's place `n` (op1f_rack_run; 32 to 39)
enum { OP1F_PLACE_DRUM = 8, OP1F_PLACE_MASTER = 16, OP1F_PLACE_HOSTED = 32 };

// An effect's flags, in its head's `flags`.
//   OP1F_EFFECT_MONO   the effect works on one buffer: the base hands it the mean of the two sides in `left`, and
//                      copies `left` to both sides after the call
//   OP1F_EFFECT_TAILS  while the place's effect is switched off, the effect is still called every block, with `on` 0,
//                      after the unit's own sound and before level 6's stages there, so that what it holds (an echo, a
//                      hall) can ring out; without the flag, an effect switched off is not called and the sound passes
//                      dry
enum { OP1F_EFFECT_MONO = 1, OP1F_EFFECT_TAILS = 2 };

// The limits:
//   OP1F_EFFECT_VALUES_MOST  how many values an effect has: one for each of the four knobs at most
//   OP1F_EFFECT_CYCLES_MOST  the most one call of an effect's `process` may declare, in the processor's cycles: about
//                            what the costliest of the unit's own effects takes. It is not counted in level 6's sum
//                            (an effect takes the place of the unit's own effect there, which costs as much)
//   OP1F_EFFECTS_MOST        how many effects the packs may register in all
//   OP1F_EFFECT_STATE_MOST   the most working memory one instance of an effect may ask
//   OP1F_RACK_PLACES         how many places of its own a pack may run effects at (op1f_rack_run's `which`)
//   OP1F_NOT_HERE            what the helpers answer when the place does not ride this effect, or when no instance is
//                            ready there
// An ask beyond a limit is left aside at the start with a line, and its card is used.
enum { OP1F_EFFECT_VALUES_MOST = 4, OP1F_EFFECT_CYCLES_MOST = 84000, OP1F_EFFECTS_MOST = 16, OP1F_EFFECT_STATE_MOST = 8 << 20,
       OP1F_RACK_PLACES = 8, OP1F_NOT_HERE = -8 };

// One value of an effect: a knob of the unit's, in the pack's own numbers.
//   name   what the LFO pages and the pack's page show: 1 to 15 characters of a-z, 0-9, ' ', '-' and '_', zero padded
//   low, high  the value's range in the pack's numbers: low < high, high - low at most 65535. The base maps it onto the
//          unit's own knob of the effect the pack's stands on, both ways: the knob's whole travel is the whole range
//   start  its value when the owner picks the effect, inside the range
struct Op1fParam { char name[16]; int low, high, start; };   // 28 bytes

// What an effect's `process` is handed, once a block.
//   bytes    the size of what is handed: 44 at level 8. A later base may hand more; read what you know
//   place    where it plays now: a place above (0 to 16), or OP1F_PLACE_HOSTED + n when a pack runs it
//   frames   64, the frames of the block
//   left     the left buffer, `frames` words of 32 bits, signed, Q1.31 as a stage's (level 6), changed in place and held
//            to full scale; with OP1F_EFFECT_MONO the mean of the two sides
//   right    the right buffer, the same; with OP1F_EFFECT_MONO the same buffer as `left`
//   values   the effect's values in the pack's own numbers, `count` of them, as the knobs, the LFOs and the unit's
//            random key left them for this block (the unit's knob words mapped onto each value's range)
//   on       1; 0 only for an effect with OP1F_EFFECT_TAILS while the place's effect is switched off
//   fresh    1 at the first call after the place's effect was made anew: picked, a sound loaded, another slot
//            played, the unit started; or, for a run inside a pack, when that pack says so. Clear what the call keeps
//            (or let `reset` have done it)
//   period   the length of a 96th of a beat, as a stage's block has it (level 6): no division
//   playing  not 0 while the tape plays
//   state    this place's working memory (its `state_bytes`, cleared to 0 when it was given), or 0 when the effect
//            asks none
// It lives for the call only: never keep it, nor the buffers' addresses.
struct Op1fEffectBlock { unsigned bytes, place, frames; int* left; int* right; const int* values;
                         unsigned on, fresh, period, playing; void* state; };   // 44 bytes at level 8

// An effect: what its ask points at.
//   head         level 5's head: `name` is what the unit's lists show (1 to 15 characters of a-z, 0-9, ' ', '-' and
//                '_': no name of the unit's own effects, and none an earlier pack registered); `flags` OP1F_EFFECT_...;
//                `shown(where, how)` is told when the effect's page comes on the screen and goes: `where` the place
//                whose page is up, `how` OP1F_SHOWN_UP, with OP1F_SHOWN_OFF while that place's effect is switched off,
//                and 0 when the page leaves the screen. The page is level 5's page that stands in
//                (op1f_page_stand_in, this ask's number), or the base's blank page with the effect's name
//   rides        the unit's own effect this one stands on, zero padded: "delay", "grid", "punch", "phone",
//                "spring", "nitro", "cwo" or "terminal". The unit keeps that effect's name in the sound's file, its four
//                knobs inside that effect's ranges, switched off; the knobs' travel and steps are that effect's. Pick
//                the one nearest in character
//   cycles       what one call of `process` takes at most, 1 to OP1F_EFFECT_CYCLES_MOST. The base times every call
//                as it times a stage's (level 6), with the same allowance: an effect that goes over is switched off at
//                that place, and the place then passes the sound dry, until an effect is picked there again (base 38;
//                before, until the next start; inside a pack, until the pack's place picks it again after another)
//   state_bytes  the working memory one instance needs, 0 to OP1F_EFFECT_STATE_MOST: 0 for none. The base gives up
//                to two instances for the unit's places (one for the sound that plays, one for the master), and one
//                for each place a pack runs the effect at, from level 6's memory and inside its limits, each the
//                first time it is needed, cleared, never given back to the heap (base 38: a pack's place that has not
//                run it for two frames leaves it to the next place that asks, the same effect first, cleared again over
//                the frames). A place whose memory is refused passes the sound
//                dry, with a line, and its page says so
//   state_align  where the memory begins: on a multiple of it, a power of two from 4 to OP1F_MEMORY_ALIGN_MOST
//   process      the effect, called once a block on the audio thread (see "The threads" above). Not 0
//   reset        called on the display thread at the frame after a place's effect was made anew, before the next
//                call of `process` there: `place` and that place's working memory. Until it has returned, the place
//                passes the sound dry. 0 for none: `fresh` then says it in the next block
//   count        how many values follow, 1 to OP1F_EFFECT_VALUES_MOST: value n is turned by knob n + 1 (blue, ochre,
//                grey, red)
//   params       the values
// The effect names no address of the unit's and changes no place of the firmware. It needs op1f_level_8.
struct Op1fEffect {
    Op1fKindHead head;                                              // +0
    char rides[16];                                                 // +28
    unsigned cycles;                                                // +44
    unsigned state_bytes, state_align;                              // +48, +52
    void (*process)(const Op1fEffectBlock* now);                    // +56
    void (*reset)(unsigned place, void* state);                     // +60
    unsigned count;                                                 // +64
    Op1fParam params[OP1F_EFFECT_VALUES_MOST];                      // +68
};   // 180 bytes at level 8

// The base's list of every effect the packs registered (op1f_rack_list), one entry for each, in the order of their
// numbers: the order of the packs' file names, then of each card's asks (the unit's lists show them by name).
//   bytes   the size of an entry: 20 at level 8
//   id      the effect's number in the unit's lists (the unit's own are below it)
//   pack    the name of the pack that registered it
//   effect  its table, as its pack wrote it: its name, flags, the effect it stands on, its values, its cost
//   rides   the number of the unit's own effect it stands on, in the unit's own order of its effects
struct Op1fRackEntry { unsigned bytes, id; const char* pack; const Op1fEffect* effect; unsigned rides; };   // 20 bytes at level 8

// How a pack hands the values of an effect it runs (op1f_rack_run's `form`):
//   OP1F_RACK_WORDS   `values` holds four words as the unit keeps a knob of the effect it stands on (a value shifted
//                     left by sixteen, inside that knob's range): a pack that keeps the unit's own words (a chain of
//                     the unit's effects, with the unit's LFOs on them) hands them as they are, and the base maps them
//   OP1F_RACK_VALUES  `values` holds the effect's values in its own numbers, `count` of them, each held to its range
enum { OP1F_RACK_WORDS = 0, OP1F_RACK_VALUES = 1 };

// What a pack hands op1f_rack_run.
//   bytes   the size of what is handed: 36 at level 8
//   id      the effect's number in the list (an entry's `id`)
//   form    OP1F_RACK_WORDS or OP1F_RACK_VALUES
//   frames  64
//   left, right  the pack's two buffers, Q1.31, changed in place
//   values  as `form` says
//   on      1, or 0 for an effect with OP1F_EFFECT_TAILS that the pack has switched off and lets ring out
//   fresh   1 when the pack starts the effect anew at this place (a chain rebuilt, a sound changed): the effect
//           clears what it keeps. The base hands 1 by itself at the first run after op1f_rack_ready made the instance
struct Op1fRackRun { unsigned bytes, id, form, frames; int* left; int* right; const int* values; unsigned on, fresh; };   // 36 bytes at level 8

// Level 9: synth engines.
// The kind of ask a base of level 9 knows, beside those of levels 5 to 8. A card of level 9 is a card of level 4's
// size: OP1F_CARD_LEVEL_9_BYTES.
//   OP1F_ASK_ENGINE  a synth engine in the unit's list of engines: what `what` points at is an Op1fEngine
enum { OP1F_ASK_ENGINE = 8 };
enum { OP1F_CARD_LEVEL_9_BYTES = 64 };

// An engine's flags, in its head's `flags`.
//   OP1F_ENGINE_STEREO  the engine renders two channels: `out` then holds 2 x `frames` words, left and right in turn
//                       for each frame (left first); without it, `frames` words, one channel, which the unit pans
enum { OP1F_ENGINE_STEREO = 1 };

// A voice's events since its last block (`events`, bits).
//   OP1F_VOICE_BEGINS  a note began on this voice
//   OP1F_VOICE_ENDS    the note was let go: the unit's envelope goes on, and the voice keeps being rendered until it
//                      has faded
//   OP1F_VOICE_FRESH   the voice has just been handed to this engine (picked, a sound loaded, another sound played,
//                      the unit started): forget what it held. A voice may be rendered before any OP1F_VOICE_BEGINS
//                      (a note that began just before the engine was picked): render silence until one comes
// More than one may come in one block (a short note: BEGINS and ENDS together).
enum { OP1F_VOICE_BEGINS = 1, OP1F_VOICE_ENDS = 2, OP1F_VOICE_FRESH = 4 };

// The limits:
//   OP1F_ENGINE_VALUES_MOST  how many values an engine has: one for each of the four knobs, with and without shift
//   OP1F_ENGINE_CYCLES_MOST  the most one call of `render` may declare, for one voice, in the processor's cycles. It is
//                            not counted in level 6's sum (the engine takes the place of the unit's own engine, which
//                            costs as much)
//   OP1F_ENGINES_MOST        how many engines the packs may register in all
//   OP1F_ENGINE_SOUNDS_MOST  how many sounds one engine's list may hold
//   OP1F_ENGINE_SOUND_MOST   the most bytes one sound's text may have
//   OP1F_ENGINE_VOICE_MOST   the most working memory one voice may ask (the base gives it six times, one a voice)
//   OP1F_ENGINE_OFF          a render that takes more than this switches the engine off at once
// An ask beyond a limit is left aside at the start with a line, and its card is used.
enum { OP1F_ENGINE_VALUES_MOST = 8, OP1F_ENGINE_CYCLES_MOST = 40000, OP1F_ENGINES_MOST = 8, OP1F_ENGINE_SOUNDS_MOST = 32,
       OP1F_ENGINE_SOUND_MOST = 2048, OP1F_ENGINE_VOICE_MOST = 1 << 20, OP1F_ENGINE_OFF = 120000 };

// What `render` is handed, once a block, for one voice that sounds.
//   bytes     the size of what is handed: 52 at level 9. A later base may hand more; read what you know
//   slot      the synth sound the voice plays, 0 to 7
//   voice     the voice, 0 to 5
//   frames    128, the frames of one render
//   rate      the frames a second they stand for: 88,200
//   out       the voice's sound, written whole by `render`: `frames` words of 32 bits, signed, each a Q1.31 fraction
//             (full scale is -2^31 to 2^31 - 1), or with OP1F_ENGINE_STEREO 2 x `frames` words, left and right in turn.
//             The unit applies its own envelope, the velocity, the pan and the mix afterwards: never apply them here
//   values    the engine's values in the pack's own numbers, `count` of them, as the knobs, the LFOs, MIDI and the
//             unit's random key left them for this block
//   step      the pitch: how far one frame advances the phase of a wave at the note's frequency, a whole cycle being
//             2^32 (the frequency is step x 88,200 / 2^32), glide and pitch bend applied; one value for the block. The
//             note A4 (440 Hz) with no bend is 21,426,141
//   velocity  the note's velocity, 0 (softest) to about 0x7f000000 (hardest): MIDI's 0 to 127 in the top byte
//   events    OP1F_VOICE_... since the voice's last block
//   period    the length of a 96th of a beat, as a stage's block has it (level 6): no division
//   playing   not 0 while the tape plays
//   state     this voice's working memory (its `voice_bytes`, cleared to 0 when it was given), or 0 when the engine
//             asks none
// It lives for the call only: never keep it, nor `out`'s address.
struct Op1fVoiceBlock { unsigned bytes, slot, voice, frames, rate; int* out; const int* values;
                        unsigned step; int velocity; unsigned events, period, playing; void* state; };   // 52 bytes at level 9

// One of the engine's own sounds, written once into the unit's sound list.
//   name   the file's name without ".json": 1 to 23 characters of a-z, 0-9, ' ', '-' and '_', zero padded; no two of
//          one engine alike
//   text   the sound's file, `bytes` long: one JSON object as the unit writes a synth sound, with "type": "dsynth",
//          "knobs" eight whole numbers from 0 to 32767 (the knob words as the unit keeps them: value 1 to 8 of the
//          engine at the top of each knob's travel is 32767), and only the members the unit's own sounds hold; the
//          base adds "op1f_engine" with the engine's name at the top when it is missing. At most OP1F_ENGINE_SOUND_MOST
//          bytes. A text the base cannot hold to this is not written, with a line, and the engine runs
struct Op1fSound { char name[24]; const char* text; unsigned bytes; };   // 32 bytes

// A synth engine: what its ask points at.
//   head         level 5's head: `name` is what the engine list and the sound list's folder show (1 to 15 characters
//                of a-z, 0-9, ' ', '-' and '_': no name of the unit's own engines, nor "sampler" or "fazer", and none
//                an earlier pack registered); `flags` OP1F_ENGINE_...; `shown(where, how)` is told when the engine's
//                page comes on the screen and goes: `where` the slot whose page is up, `how` OP1F_SHOWN_UP, with
//                OP1F_SHOWN_OFF while the engine is switched off for its cost, and 0 when the page leaves the screen
//   cycles       what one call of `render` takes at most, for one voice, 1 to OP1F_ENGINE_CYCLES_MOST. The base times
//                every call, interrupts served during it included: an engine that, within one second, takes more than
//                twice this and more than this and OP1F_STAGE_CYCLES_SPARE in OP1F_STAGE_OVER_CALLS calls, or more
//                than OP1F_ENGINE_OFF in one, is switched off until the next start, and its sounds are then silent
//   voice_bytes  the working memory one voice needs, 0 to OP1F_ENGINE_VOICE_MOST: 0 for none. The base gives six, one a
//                voice, at its first frame, from level 6's memory and inside its limits (the six together count as one
//                ask), cleared, never given back. Refused, the engine's sounds are silent, with a line, and its page
//                says so. A table every voice shares (a wavetable) is the pack's own data, or an ask of kind
//                OP1F_ASK_MEMORY
//   voice_align  where each voice's memory begins: on a multiple of it, a power of two from 4 to OP1F_MEMORY_ALIGN_MOST
//   render       the engine, called once a block for each voice that sounds, on the audio thread. Not 0
//   reset        called on the display thread at the frame after a voice was handed to the engine, before that voice's
//                next `render`: the slot, the voice and its working memory. Until it has returned, the voice is
//                silent. 0 for none: OP1F_VOICE_FRESH then says it in the voice's next block, which renders at once.
//                An engine whose voice holds much (a string's delay line) clears it here
//   count        how many values follow, 1 to OP1F_ENGINE_VALUES_MOST: values 1 to 4 are turned by knobs 1 to 4 (blue,
//                ochre, grey, red) on T1, values 5 to 8 by the same knobs with shift
//   params       the values (level 8's Op1fParam): each value's name, its range in the pack's own numbers, and its value
//                when the owner picks the engine. The base maps a value onto the unit's own knob both ways: the
//                knob's whole travel is the value's whole range
//   sounds       how many of the engine's own sounds follow, 0 to OP1F_ENGINE_SOUNDS_MOST
//   sound        the sounds, or 0 for none
// The engine names no address of the unit's and changes no place of the firmware. It needs op1f_level_9.
struct Op1fEngine {
    Op1fKindHead head;                                              // +0
    unsigned cycles;                                                // +28
    unsigned voice_bytes, voice_align;                              // +32, +36
    void (*render)(const Op1fVoiceBlock* now);                      // +40
    void (*reset)(unsigned slot, unsigned voice, void* state);      // +44
    unsigned count;                                                 // +48
    Op1fParam params[OP1F_ENGINE_VALUES_MOST];                      // +52
    unsigned sounds;                                                // +276
    const Op1fSound* sound;                                         // +280
};   // 284 bytes at level 9

#ifdef __GNUC__
#define OP1F_PLACE_IN(type, field) __builtin_offsetof(type, field)
#else
#include <stddef.h>
#define OP1F_PLACE_IN(type, field) offsetof(type, field)
#endif
static_assert(sizeof(unsigned) == 4 && sizeof(void*) == 4, "a card is laid out for words and addresses of four bytes, as the unit's are");
// Level 3's sizes of what a frame function is handed and of a card are held by the places of level 4's words behind
// them: `canvas` at +36 is level 3's 36 bytes, `asks` at +56 a card of level 3's 56.
static_assert(sizeof(Op1fFrame) == 40, "what a frame function is handed is 40 bytes at level 4");
static_assert(OP1F_PLACE_IN(Op1fFrame, popup) == 24 && OP1F_PLACE_IN(Op1fFrame, page) == 28 && OP1F_PLACE_IN(Op1fFrame, disk) == 32,
              "levels 1 and 2's 32 bytes stand as they were, and the disk behind them");
static_assert(OP1F_PLACE_IN(Op1fFrame, canvas) == 36, "level 3's 36 bytes stand as they were, and the canvas behind them");
// Level 2's size of what an input function is handed is held by the place of level 4's `thread` behind it.
static_assert(sizeof(Op1fInput) == 40 && OP1F_PLACE_IN(Op1fInput, record) == 32, "what an input function is handed is 40 bytes at level 4");
static_assert(OP1F_PLACE_IN(Op1fInput, thread) == 36, "level 2's 36 bytes stand as they were, and the thread behind them");
static_assert(sizeof(Op1fFolder) == 36, "a folder of a card's list is 36 bytes");
static_assert(sizeof(Op1fAsk) == 12, "an ask is 12 bytes");
static_assert(sizeof(Op1fCard) == OP1F_CARD_LEVEL_4_BYTES, "a card of level 4 is 64 bytes");
static_assert(OP1F_PLACE_IN(Op1fCard, frame) == 32, "a card's frame function stands at +32");
static_assert(OP1F_PLACE_IN(Op1fCard, pictures) == 36, "a card's directory of pictures stands at +36");
static_assert(OP1F_PLACE_IN(Op1fCard, pick) == 40, "a card's choice among its pictures stands at +40");
static_assert(OP1F_PLACE_IN(Op1fCard, input) == OP1F_CARD_LEVEL_1_BYTES, "a card's input function stands at +44, behind level 1");
static_assert(OP1F_PLACE_IN(Op1fCard, folders) == OP1F_CARD_LEVEL_2_BYTES && OP1F_PLACE_IN(Op1fCard, folder_count) == 52, "a card's folders stand at +48, behind level 2");
static_assert(OP1F_PLACE_IN(Op1fCard, asks) == OP1F_CARD_LEVEL_3_BYTES && OP1F_PLACE_IN(Op1fCard, ask_count) == 60, "a card's asks stand at +56, behind level 3");
static_assert(sizeof(Op1fCardSlot) == 12, "a slot of the list of cards is 12 bytes");
static_assert(sizeof(Op1fCards) == 8 + 12 * OP1F_CARDS_MOST && sizeof(Op1fCards) == 392, "the list of cards is 392 bytes");
static_assert(sizeof(Op1fCard) == OP1F_CARD_LEVEL_5_BYTES, "a card of level 5 is a card of level 4's size, 64 bytes");
static_assert(sizeof(Op1fKindHead) == 28 && OP1F_PLACE_IN(Op1fKindHead, name) == 4 && OP1F_PLACE_IN(Op1fKindHead, flags) == 20 && OP1F_PLACE_IN(Op1fKindHead, shown) == 24,
              "the head of an ask's table is 28 bytes");
static_assert(sizeof(Op1fTick) == 32 && sizeof(Op1fKey) == 32, "a tick and a key are 32 bytes each at level 5");
static_assert(sizeof(Op1fSequencer) == 48 && OP1F_PLACE_IN(Op1fSequencer, tick) == 28 && OP1F_PLACE_IN(Op1fSequencer, playing) == 44,
              "a sequencer's table is 48 bytes at level 5, its functions behind the head");
static_assert(sizeof(Op1fNote) == 24, "a note heard is 24 bytes at level 5");
static_assert(sizeof(Op1fNotes) == 32 && OP1F_PLACE_IN(Op1fNotes, note) == 28, "the table of the notes heard is 32 bytes at level 5");
static_assert(sizeof(Op1fCard) == OP1F_CARD_LEVEL_6_BYTES && sizeof(Op1fCard) == OP1F_CARD_LEVEL_7_BYTES, "a card of level 6 or 7 is a card of level 4's size, 64 bytes");
static_assert(sizeof(Op1fBlock) == 36 && OP1F_PLACE_IN(Op1fBlock, left) == 16 && OP1F_PLACE_IN(Op1fBlock, right) == 20 && OP1F_PLACE_IN(Op1fBlock, period) == 24 &&
              OP1F_PLACE_IN(Op1fBlock, block) == 32, "what a stage is handed is 36 bytes at level 6");
static_assert(sizeof(Op1fStage) == 44 && OP1F_PLACE_IN(Op1fStage, point) == 28 && OP1F_PLACE_IN(Op1fStage, cycles) == 32 && OP1F_PLACE_IN(Op1fStage, memory) == 36 &&
              OP1F_PLACE_IN(Op1fStage, process) == 40, "a stage's table is 44 bytes at level 6, behind the head");
static_assert(sizeof(Op1fMemory) == 36 && OP1F_PLACE_IN(Op1fMemory, bytes) == 28 && OP1F_PLACE_IN(Op1fMemory, align) == 32, "a memory's table is 36 bytes at level 6");
static_assert(sizeof(Op1fPlaces) == 36 && OP1F_PLACE_IN(Op1fPlaces, count) == 28 && OP1F_PLACE_IN(Op1fPlaces, names) == 32, "the table of the places is 36 bytes at level 6");
static_assert(sizeof(Op1fKeptKey) == 36 && OP1F_PLACE_IN(Op1fKeptKey, files) == 32, "a kept member is 36 bytes");
static_assert(sizeof(Op1fKeeps) == 324 && OP1F_PLACE_IN(Op1fKeeps, changed) == 28 && OP1F_PLACE_IN(Op1fKeeps, count) == 32 && OP1F_PLACE_IN(Op1fKeeps, keys) == 36,
              "the table of the values kept is 324 bytes at level 7");
static_assert(OP1F_STAGE_CYCLES_MOST * 3 <= OP1F_STAGE_CYCLES_ALL * 2 && OP1F_STAGE_CYCLES_ALL < OP1F_STAGE_CYCLES_OFF, "a fifth of a block, three tenths, a half");
static_assert(sizeof(Op1fCard) == OP1F_CARD_LEVEL_8_BYTES, "a card of level 8 is a card of level 4's size, 64 bytes");
static_assert(sizeof(Op1fParam) == 28 && OP1F_PLACE_IN(Op1fParam, low) == 16 && OP1F_PLACE_IN(Op1fParam, start) == 24, "a value of an effect is 28 bytes");
static_assert(sizeof(Op1fEffectBlock) == 44 && OP1F_PLACE_IN(Op1fEffectBlock, left) == 12 && OP1F_PLACE_IN(Op1fEffectBlock, values) == 20 &&
              OP1F_PLACE_IN(Op1fEffectBlock, on) == 24 && OP1F_PLACE_IN(Op1fEffectBlock, state) == 40, "what an effect is handed is 44 bytes at level 8");
static_assert(sizeof(Op1fEffect) == 180 && OP1F_PLACE_IN(Op1fEffect, rides) == 28 && OP1F_PLACE_IN(Op1fEffect, cycles) == 44 &&
              OP1F_PLACE_IN(Op1fEffect, state_bytes) == 48 && OP1F_PLACE_IN(Op1fEffect, process) == 56 && OP1F_PLACE_IN(Op1fEffect, reset) == 60 &&
              OP1F_PLACE_IN(Op1fEffect, count) == 64 && OP1F_PLACE_IN(Op1fEffect, params) == 68, "an effect's table is 180 bytes at level 8, behind the head");
static_assert(sizeof(Op1fRackEntry) == 20 && OP1F_PLACE_IN(Op1fRackEntry, effect) == 12, "an entry of the list of effects is 20 bytes at level 8");
static_assert(sizeof(Op1fRackRun) == 36 && OP1F_PLACE_IN(Op1fRackRun, left) == 16 && OP1F_PLACE_IN(Op1fRackRun, values) == 24 &&
              OP1F_PLACE_IN(Op1fRackRun, fresh) == 32, "what a pack hands op1f_rack_run is 36 bytes at level 8");
static_assert((int)OP1F_EFFECT_CYCLES_MOST < (int)OP1F_STAGE_CYCLES_OFF && (int)OP1F_EFFECT_STATE_MOST <= (int)OP1F_MEMORY_MOST, "an effect's limits lie inside level 6's");
static_assert(sizeof(Op1fCard) == OP1F_CARD_LEVEL_9_BYTES, "a card of level 9 is a card of level 4's size, 64 bytes");
static_assert(sizeof(Op1fVoiceBlock) == 52 && OP1F_PLACE_IN(Op1fVoiceBlock, out) == 20 && OP1F_PLACE_IN(Op1fVoiceBlock, state) == 48,
              "what render is handed is 52 bytes at level 9");
static_assert(sizeof(Op1fSound) == 32 && OP1F_PLACE_IN(Op1fSound, text) == 24, "a sound of an engine is 32 bytes");
static_assert(sizeof(Op1fEngine) == 284 && OP1F_PLACE_IN(Op1fEngine, cycles) == 28 && OP1F_PLACE_IN(Op1fEngine, render) == 40 &&
              OP1F_PLACE_IN(Op1fEngine, count) == 48 && OP1F_PLACE_IN(Op1fEngine, params) == 52 && OP1F_PLACE_IN(Op1fEngine, sounds) == 276 &&
              OP1F_PLACE_IN(Op1fEngine, sound) == 280,
              "an engine's table is 284 bytes at level 9");
static_assert((int)OP1F_ENGINE_CYCLES_MOST < (int)OP1F_ENGINE_OFF && (int)OP1F_ENGINE_VOICE_MOST * 6 <= (int)OP1F_MEMORY_MOST, "an engine's limits lie inside level 6's");

// op1f_text_set's alignment (level 4): the text begins at the placeholder's left (OP1F_LEFT), is centred on it
// (OP1F_CENTRE), or ends at its right (OP1F_RIGHT).
enum { OP1F_LEFT = 0, OP1F_CENTRE = 1, OP1F_RIGHT = 2 };

// A colour for the op1f_paint_ helpers, from its red, green and blue, each 0 to 255: the display's sixteen bits a
// pixel, five of red, six of green, five of blue. The unit's own colours are the field palette's; its knobs, for
// instance, blue OP1F_COLOUR(0x3c, 0x60, 0x87), ochre OP1F_COLOUR(0x8a, 0x65, 0x3c), grey OP1F_COLOUR(0xa2, 0xa4, 0xa6)
// and red OP1F_COLOUR(0xe5, 0x28, 0x17): take them, and their light and dark ones, from the unit's own palette.svg.
#define OP1F_COLOUR(r, g, b) ((((unsigned)(r) & 0xf8u) << 8) | (((unsigned)(g) & 0xfcu) << 3) | (((unsigned)(b) & 0xffu) >> 3))

// What the base offers a pack with a card. Each is a name the pack needs once it calls it, and the loader joins it
// to the base's function. Every one of these names sorts behind op1f_level_1 to op1f_level_8, and the loader looks
// for a pack's needed names in their order: a pack whose base is too old is left out for the mark, which says what is
// missing, not for a helper.

// A line on the unit's log. `level`: 1 an error, 2 a warning, 3 for information. `file`: a source path; the log
// shows the name between its last slash and its dot in front of the line, and a bare word shows nothing. `line`: a
// number shown with that name. `format` and what follows it: as printf takes them. Answers nothing. From the frame
// function, never from an input function: an input function may run on the audio thread. A line every frame fills
// the log.
extern "C" void op1f_log(int level, const char* file, int line, const char* format, ...) OP1F_NAMED("op1f_log");

// The clock: milliseconds since the base's first frame. It moves once a frame. From any thread.
extern "C" unsigned op1f_ms() OP1F_NAMED("op1f_ms");

// The document of one of the firmware's screens, by the screen's number: the picture the firmware made for that
// screen, whose parts the next three work on. Answers 0 for a number outside 1 to 85, for a number no screen has,
// and for a screen that has no document yet: ask again at a later frame. Display thread.
extern "C" void* op1f_screen(int number) OP1F_NAMED("op1f_screen");

// A part of a document, by the name it has in the picture (its id). Answers the part, or 0 when the document has
// none of that name; 0 too for no document or no name. Display thread.
extern "C" void* op1f_node(void* document, const char* name) OP1F_NAMED("op1f_node");

// A part shown (`on` not 0) or hidden (0), with everything inside it. `part` is one op1f_node answered, never 0.
// Answers nothing. Display thread.
extern "C" void op1f_show(void* part, int on) OP1F_NAMED("op1f_show");

// A part moved, turned or scaled: its matrix set. `matrix` is nine whole numbers in 1024ths, [a, b, 0, c, d, 0,
// e, f, 1024]: the six numbers of a picture's matrix(a b c d e f), so [1024, 0, 0, 0, 1024, 0, 1024 x, 1024 y,
// 1024] is a move of x and y pixels. `part` is one op1f_node answered, never 0. Answers nothing. Display thread.
extern "C" void op1f_move(void* part, const int* matrix) OP1F_NAMED("op1f_move");

// One of the pack's own pictures, by its name in the pack's own directory and nobody else's, made into a document
// as the firmware makes one of a file. `card` is the pack's own card, one the base has in use. Answers the
// document, or 0 when the card is not in use, the pack carries no picture of that name, or it could not be made.
// Ask once for a picture and keep the answer, also where the answer was 0: each call for a picture the pack
// carries reads the pack's file and takes memory, and one that cannot be made is an error line on the log at
// every call. The base keeps the first eight documents a card made: those are the ones op1f_page_open and
// op1f_text take. Display thread.
extern "C" void* op1f_picture(const Op1fCard* card, const char* name) OP1F_NAMED("op1f_picture");

// The count of the pictures the base has served from the packs' files since the start: each picture a screen of the
// firmware or a pack asked for that a pack's file answered and the firmware's parser took. It only grows. A word a
// pack may read, from any thread, for a line of its own; never write it. A pack that reads it may ask for it weakly,
// so that it runs under a base that does not offer it.
extern "C" unsigned op1f_svg_served OP1F_NAMED("op1f_svg_served");

// Level 2. The pack's page opened, in the mode the unit is in, with a document op1f_picture made: the base shows it
// from the next frame on, in place of whatever the mode would show, until it is closed. `card` is the pack's own
// card, one the base has in use; a pack with no input function may open a page too, from its frame function, and
// the keys then go on to the firmware as they always did. The page opened last is the one shown: another
// pack's page that was open is closed. The page is closed when the mode changes and by op1f_page_close. Answers
// not 0 when the base took the request, 0 when it did not (no such card, no document, the base not ready yet). A
// request taken can still be dropped before the page is ever shown, with no line: when the mode changes before the
// base's next frame. The frame's `page` is what says the page shows. `document` must be one op1f_picture made for
// this card: since
// level 3 the base answers 0 for any other. From the frame function or the input function. The document stays the
// pack's: the base never frees it, and the pack may dress it at every frame, shown or not.
extern "C" int op1f_page_open(const Op1fCard* card, void* document) OP1F_NAMED("op1f_page_open");

// Level 2. The pack's page closed, when it is the one open; nothing otherwise. The mode's own screen comes back.
// From the frame function or the input function. From level 4's base on, a close is seen by the firmware's next
// request for a screen, so the mode's own screen is back at the next frame; and where the page lay over another shown
// with op1f_page_show, that page shows again in its place. From base 26 on, a page that stays with its mode
// (OP1F_PAGE_STAYS) is closed by it in any mode, the one it waits for or another.
extern "C" void op1f_page_close(const Op1fCard* card) OP1F_NAMED("op1f_page_close");

// Level 4. The pack's page shown, in the mode the unit is in, over whatever page was there: pages stack. When the
// page on top closes (op1f_page_close by its pack), the page under it shows again; a mode change closes them all (but
// from base 26 on the page on top when it was asked with OP1F_PAGE_STAYS, below).
// A page opened with
// op1f_page_open behaves as it always did: a page shown over it closes it. `document` is one this card made with
// op1f_picture, or 0 for the base's own blank page: black, the whole screen, for a pack that paints its page itself
// on the frame's `canvas`; the same page for every pack, and its text is not a pack's to set. `flags` is 0, or from
// base 26 on OP1F_PAGE_STAYS (below); a base before 26 knows no flag. Answers 1 when the base took the request, 0 when
// it did not: the card not in use, a document this card did not make, a flag the base does not know, or the blank
// page not to be made (the log says so once). Taken is not shown: the frame's `page` says it. From the frame function
// or the input function.
extern "C" int op1f_page_show(const Op1fCard* card, void* document, unsigned flags) OP1F_NAMED("op1f_page_show");

// A flag of op1f_page_show (from base 26 on; any pack may ask it, and it needs no mark).
//   OP1F_PAGE_STAYS  the page stays with its mode. When the unit goes to another mode the page is not closed: its
//                    document stays under the screen the base lends, and when the unit comes back to the page's mode
//                    the firmware's own request for a screen shows the page at once, with no frame of the mode's own
//                    screen between (the synth key back from the mixer shows the page, not the synth's first page).
//                    While the unit is in another mode the frame's `page` is 0, and keys and knobs go as they go with
//                    no page. While it waits in another mode and anything else takes the lent screen (another
//                    pack's page, or a page standing in for a screen of the firmware, level 5), from base 27 on it is
//                    set aside, not closed: when that page goes it waits for its mode again, and the unit coming back
//                    to its mode shows it at once, straight from the other page too. (Base 26 closed it then.) It is
//                    closed, with a line on the log:
//                      by its pack, op1f_page_close, in any mode, waiting or set aside;
//                      when the instrument the unit is on (the synth or the drum) has changed while it waited: a page
//                        of the synth's does not come back after the drum;
//                      from base 27 on, when it is set aside and another page that stays is set aside in its place.
//                    One page stays at a time: the page shown when the mode changes, if it was asked with the flag.
//                    The pages kept under it close then, as always; a page shown over it in its own mode lies over it
//                    as over any page, and it is the page on top when the mode changes that stays. The base's top page
//                    (which asks the owner to confirm something) lies over it and leaves it as it found it. A page its
//                    own pack asks for while it waits (with this function or op1f_page_open) is the pack's page from
//                    then on, in the mode the unit is in, and stays with that mode only if asked with the flag. A page
//                    asked without the flag closes when the mode changes, as it always did.
// A base before 26 answers 0 for any flag and takes nothing: a pack that may run on such a base asks again without the
// flag when it is answered 0; its page then closes when the mode changes, and the pack asks for it again when the unit
// is back in the mode (at its first frame there, not after some frames, so that the mode's own screen shows for one
// frame and no more).
enum { OP1F_PAGE_STAYS = 1 };

// Level 3. The path on the unit's disk of a file of one of the pack's folders (`name`), or of the folder itself
// (`name` 0: with the slash behind), written into `out` with its end. Answers the path's characters, or less than
// 0: OP1F_NO_FOLDER, OP1F_BAD_NAME, OP1F_TOO_LARGE (`room` too small; nothing written). For a line on the log or a
// page; the helpers below take the folder and the name, never a path. From any thread, once the base's first frame
// has made the folders.
extern "C" int op1f_path(const Op1fCard* card, unsigned folder, const char* name, char* out, unsigned room) OP1F_NAMED("op1f_path");

// Level 3. The names of the files in one of the pack's folders, in the order the disk lists them: each name and a
// zero behind it, one after another in `names`, as many as fit whole in `room`. Folders inside the folder and names
// that begin with a dot are left out, and so is a name of more than thirty-nine characters. Answers how many names
// it wrote, or less than 0: OP1F_NOT_NOW, OP1F_NO_FOLDER, OP1F_FAILED. From the pack's own frame function only.
extern "C" int op1f_list(const Op1fCard* card, unsigned folder, char* names, unsigned room) OP1F_NAMED("op1f_list");

// Level 3. A file of one of the pack's folders read whole into `buffer`. Answers the bytes read (0 for an empty
// file), or less than 0: OP1F_NO_FILE (no line), OP1F_TOO_LARGE (the file has more than `room` bytes: nothing is
// read), OP1F_NOT_NOW, OP1F_NO_FOLDER, OP1F_BAD_NAME, OP1F_FAILED. From the pack's own frame function only.
extern "C" int op1f_read(const Op1fCard* card, unsigned folder, const char* name, void* buffer, unsigned room) OP1F_NAMED("op1f_read");

// Level 3. `bytes` bytes of `data` written as a file of one of the pack's folders, in place of what the file held,
// or as a new file. Answers `bytes` when the file landed (its size held afterwards), or less than 0: OP1F_NOT_NOW
// (also while the disk mode is open), OP1F_NO_FOLDER, OP1F_BAD_NAME, OP1F_FAILED (the line says which step: the
// temporary file opened, written, renamed over the pack's file, or the size afterwards). From the pack's own frame
// function only. At most a megabyte.
extern "C" int op1f_write(const Op1fCard* card, unsigned folder, const char* name, const void* data, unsigned bytes) OP1F_NAMED("op1f_write");

// Level 4. A file of one of the pack's folders taken off the disk. Answers 0 when the file is gone, or less than 0:
// OP1F_NO_FILE (the folder held no file of that name), OP1F_NOT_NOW (not from the pack's own frame function, or the
// disk mode is open), OP1F_NO_FOLDER, OP1F_BAD_NAME, OP1F_FAILED (the disk did not take it off; the line says so).
// From the pack's own frame function only.
extern "C" int op1f_remove(const Op1fCard* card, unsigned folder, const char* name) OP1F_NAMED("op1f_remove");

// Level 3. A text written into a part of a document op1f_picture made (any card's, one the base keeps): the part, a
// placeholder group of the pack's picture (above), becomes a text at its first use and stays one; the text is set
// again only when it changed. `width`: the text is shortened with a mark where it is wider than so many pixels, 0
// for the screen's width. An empty text shows nothing. Answers 1 when the text is in place, 0 when it is not: no
// such document or part, a part that is no group with a stroke, the base's room for texts full (sixty-four on a base
// of level 3; on a base of level 4, 256 for one pack and 512 for all), or a call that did not come from a frame
// function. From a frame function only. On a base of level 4 it is op1f_text_set with OP1F_LEFT.
extern "C" int op1f_text(void* document, const char* part, const char* text, int width) OP1F_NAMED("op1f_text");

// Level 4. op1f_text with an alignment: `align` OP1F_LEFT (the text begins at the placeholder's left), OP1F_CENTRE
// (centred on it) or OP1F_RIGHT (it ends at its right). A part's alignment is the one of its first use: a later call
// with another alignment sets the text and keeps the first alignment, and the log says so; a pack that wants a part
// in two alignments uses two placeholders. Answers as op1f_text, and 0 for any other `align`. From a frame function
// only.
extern "C" int op1f_text_set(void* document, const char* part, const char* text, int width, unsigned align) OP1F_NAMED("op1f_text_set");

// Level 4. The width in pixels of the text a part holds, as it was laid out: 0 for an empty text, -1 when no text of
// that part was made (by op1f_text or op1f_text_set). The width is the letters' own, without their stroke: a text
// of one upright stroke measures 0. From a frame function only.
extern "C" int op1f_text_width(void* document, const char* part) OP1F_NAMED("op1f_text_width");

// Level 4. The text a part became, as a part: op1f_move moves it, op1f_show shows and hides it. Answers 0 when no
// text of that part was made. A text moved by a matrix needs nothing more. From a frame function only.
extern "C" void* op1f_text_node(void* document, const char* part) OP1F_NAMED("op1f_text_node");

// Level 4: painting on the frame's picture. `canvas` is the frame's `canvas`, handed in the same call; the picture is
// 480 pixels wide and 224 tall, x from the left and y from the top, and everything is cut where it leaves it. A
// colour is OP1F_COLOUR's, its low sixteen bits used. Each helper paints only from the pack's own frame function and
// with this frame's canvas: any other call paints nothing and is counted, and the base says the count on the log by
// the pack's name. A `canvas` of 0 (the frame offers none) paints nothing and is not counted.

// A filled box, `w` pixels wide and `h` tall, its top-left at (x, y). Nothing for a `w` or an `h` of 0 or less.
extern "C" void op1f_paint_box(void* canvas, int x, int y, int w, int h, unsigned colour) OP1F_NAMED("op1f_paint_box");

// A line from (x0, y0) to (x1, y1), both ends painted, drawn with a square pen `width` pixels wide (1 to 16) whose
// middle follows the line. Nothing for another width, and nothing for an end outside -32,768 to 32,767.
extern "C" void op1f_paint_line(void* canvas, int x0, int y0, int x1, int y1, int width, unsigned colour) OP1F_NAMED("op1f_paint_line");

// A block of pixels put at (x, y): `w` by `h` pixels of sixteen bits, row by row from the top, `stride` pixels from
// the start of one row to the start of the next. A pixel equal to `key` (0 to 65,535) is left out, so that what lies
// under it shows; a `key` of -1 leaves none out. Nothing for a `w` or an `h` of 0 or less or more than 480, for
// `pixels` 0, or for another key.
extern "C" void op1f_paint_block(void* canvas, int x, int y, int w, int h, const unsigned short* pixels, int stride, int key) OP1F_NAMED("op1f_paint_block");

// A colour laid over what is there through a block of cover, its top-left at (x, y): `w` by `h` bytes, row by row from
// the top, `stride` bytes from the start of one row to the start of the next, each saying how much of `colour` lies
// over its pixel, 0 none and 255 all, so that a pack paints letters and shapes with soft edges of its own. Each of the
// pixel's three parts is mixed by itself: the part + (((the colour's part - the part) x weight) >> 8), the weight being
// the cover + (cover >> 7), 0 to 256; 0 leaves the pixel as it is and 255 gives the colour exactly. Nothing for a `w`
// or an `h` of 0 or less or more than 480, or for `cover` 0.
extern "C" void op1f_paint_mask(void* canvas, int x, int y, int w, int h, const unsigned char* cover, int stride, unsigned colour) OP1F_NAMED("op1f_paint_mask");

// A text in the base's own font, its top-left at (x, y): seven pixels tall and five wide a character, with one
// pixel of space after each but the last, every pixel of the font a square of `scale` by `scale` (1 to 8). The
// characters are the plain ones, capitals and small letters, digits and signs; any other is a question mark. At most
// 128 characters; the rest is not painted. Answers the width in pixels the text takes, (6 x characters - 1) x scale,
// the same whether it was painted, cut or not painted at all, so that a pack measures a text with it; 0 for no text,
// an empty one or a scale outside 1 to 8.
extern "C" int op1f_paint_text(void* canvas, int x, int y, const char* text, unsigned colour, int scale) OP1F_NAMED("op1f_paint_text");

// Level 5: notes out, for a pack with a sequencer. `card` is the pack's own card.

// A note begun on the instrument the unit is on, as a key does, and sent out over MIDI on the unit's own MIDI
// settings unless `flags` says OP1F_NOTE_NO_MIDI. `note` 0 to 127, `velocity` 1 to 127. Only from the pack's `tick`,
// `key`, `run`, `octave` and its input function, while the base is calling them, and only while the instrument the
// unit is on is one whose chosen sequencer is this card's: any other call plays nothing and is counted, and the base
// says the count on the log by the pack's name. Never from the frame function, and never from a `note` of the notes
// heard. The base keeps a list of the notes a pack began on each instrument; when the sequencer stops (switched off,
// another chosen, the unit changing instrument) it ends every one the pack left, once the pack's `run` has come back.
// Answers 1 when the note was handed on, 0 when not: the card not in use, a note above 127, a velocity of 0 or above
// 127, a note this pack already has sounding (end it first), or a call from elsewhere. Handed on is not heard: the
// synth has six voices and a seventh note takes one of them, and while a sound is being loaded the instrument plays
// nothing. A note a pack began is not moved by the octave keys: a pack that wants its notes to follow ends and begins
// them in `octave`.
extern "C" int op1f_note_on(const Op1fCard* card, unsigned note, unsigned velocity, unsigned flags) OP1F_NAMED("op1f_note_on");

// A note this pack began ended. From the same calls as op1f_note_on. Answers 1 when the end was handed on, 0 when
// not: the card not in use, a note this pack has not sounding, or a call from elsewhere. While a sustain pedal is
// held the instrument may let the note ring until the pedal comes up; for the base it has ended.
extern "C" int op1f_note_off(const Op1fCard* card, unsigned note, unsigned flags) OP1F_NAMED("op1f_note_off");

// A sounding note of this pack's moved in pitch, on the synth: `cents` hundredths of a semitone, up for more than 0
// and down for less, -4800 to 4800 (beyond: the nearer of the two), 0 back to the note itself. The note keeps its
// number: it is ended with op1f_note_off as it was begun. Where the instrument plays the same note for a key as well,
// that one moves with it. From the same calls as op1f_note_on. Answers 1 when the pitch was set, 0 when not: the unit
// is not on the synth, a note this pack has not sounding, or a call from elsewhere.
extern "C" int op1f_note_bend(const Op1fCard* card, unsigned note, int cents) OP1F_NAMED("op1f_note_bend");

// Level 5. The clock's count set back to its start, as the firmware's own sequencers do when a first key goes down,
// where the firmware allows it (not while a clock from outside leads). From the same calls as op1f_note_on, with the
// same refusals. Answers 1 when it was set back, 0 when not. The next tick's `pulse` starts again and its `jumped`
// says so; its `count` goes on.
extern "C" int op1f_seq_restart(const Op1fCard* card) OP1F_NAMED("op1f_seq_restart");

// Level 5. A short section in which nothing else of the unit runs: for what a pack shares between its calls on
// different threads (a queue its `key` fills and its `tick` empties). op1f_mask answers what op1f_unmask must be
// handed. A few microseconds between the two, and never a call of the base's or a wait inside. From any thread.
extern "C" unsigned op1f_mask() OP1F_NAMED("op1f_mask");
extern "C" void op1f_unmask(unsigned was) OP1F_NAMED("op1f_unmask");

// Level 5. The document that stands in for the firmware's screen of one of the pack's asks while that screen would be
// up (a sequencer: the sequencer's screen of an instrument whose chosen sequencer it is): one this card made with
// op1f_picture, or 0 for the base's blank page, on which the pack paints from its frame function. `ask` is the ask's
// number in the card's list, from 0. It takes effect at the next frame and stands until it is called again with
// another document, whatever the page does in between; which instrument's screen the page stands in for at a moment
// is what the ask's `shown` says in `where`. Until a pack calls it, the base's blank page stands in with the ask's
// name on it. The pack does not open
// or close this page: the base does, and the ask's `shown` says when it is on the screen. A page a pack shows over it
// (op1f_page_show) lies over it, and it shows again when that one closes. While it is on the screen the frame's `page`
// is this card, and every key and knob is offered to this pack's input function first, as for any page. While the
// unit's own notice lies over it (the ask's `shown` says OP1F_SHOWN_COVERED) the page stays and its pack paints as
// usual; from base 27 on the base darkens the frame and draws the notice over it (base 26 put a page of its own that
// draws nothing in its place). Answers 1, or 0: the card not in use, no such ask of a kind with a page, a document
// this card did not make, or a call that did not come from the pack's frame function. From the frame function.
extern "C" int op1f_page_stand_in(const Op1fCard* card, unsigned ask, void* document) OP1F_NAMED("op1f_page_stand_in");

// Level 6. Whether the base calls one of the pack's stages: 1 when it does, 0 when it does not: the ask was left aside
// at the start, it waits for its memory, its memory was refused, it was switched off, or it is no stage of a card the
// base has in use. `ask` is the ask's number in the card's list, from 0. From any thread.
extern "C" int op1f_stage(const Op1fCard* card, unsigned ask) OP1F_NAMED("op1f_stage");

// Level 6. The memory an ask of kind OP1F_ASK_MEMORY was given, cleared to 0 and on its alignment; or 0: not handed
// over yet (the base clears it over the frames after its first), refused, or no such ask of a card the base has in
// use. `ask` is the ask's number in the card's list, from 0. Once it answers a block it always answers the same
// block. From any thread, the audio thread among them. Each call looks through the cards: a stage asks until it is
// answered and then keeps the block, rather than asking at every block.
extern "C" void* op1f_memory(const Op1fCard* card, unsigned ask) OP1F_NAMED("op1f_memory");

// Level 7. The text of a value the pack keeps: `where` OP1F_KEEP_SYNTH, OP1F_KEEP_DRUM or
// OP1F_KEEP_TAPE, `which` the slot (0 for the tape), `key` one of its ask's keys. Writes the value's JSON text, ended
// with a zero, into `out`, and answers its bytes; or less than 0: OP1F_NO_FILE (no value there), OP1F_TOO_LARGE (`room`
// is too small; nothing written), OP1F_NOT_NOW (not from the pack's frame function), OP1F_BAD_NAME (a key the pack's
// ask does not list for that kind of file). From the pack's own frame function only.
extern "C" int op1f_value_get(const Op1fCard* card, unsigned where, unsigned which, const char* key, char* out, unsigned room) OP1F_NAMED("op1f_value_get");

// Level 7. A value set (`text`, a JSON text of at most OP1F_VALUE_MOST bytes), or taken away (`text` 0). It is written
// at the unit's next save of that sound or tape, and kept for the slot across a start. A value taken away is not
// written again; a preset of the drum or the synth sampler that held it keeps what it held. Answers 0, or less than 0:
// OP1F_BAD_VALUE (not JSON, or a kind of value the base cannot write), OP1F_TOO_LARGE (a text longer than
// OP1F_VALUE_MOST, or the slot's or the tape's room for values, 4 KiB, would be passed), OP1F_NOT_NOW (not from the
// pack's frame function; or (from base 27 on) for the tape, a tape was loaded and the pack has not been told yet (try
// again at the next frame: `changed` tells the load first); or another thread wrote the slot meanwhile), OP1F_BAD_NAME.
// From the pack's own frame function only.
extern "C" int op1f_value_set(const Op1fCard* card, unsigned where, unsigned which, const char* key, const char* text) OP1F_NAMED("op1f_value_set");

// Level 8. The value `index` (0 to count - 1) of this pack's effect (its ask `ask`) at `place`, in the pack's own
// numbers, into `out`. Answers 0, or less than 0: OP1F_NOT_HERE (that place does not ride this effect now),
// OP1F_BAD_NAME (no such ask of kind OP1F_ASK_EFFECT, or no such value), OP1F_NOT_NOW (not from the pack's frame
// function or its input function). From the frame function and the input function.
extern "C" int op1f_param_get(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_param_get");

// Level 8. The value set: held to its range, mapped onto the unit's own knob and written as the knob would write it,
// so that the sound plays it at the next block and the unit's own save keeps it. A place of a sound also keeps it in
// the sound itself; the master, in the base's store. Answers 0, OP1F_NOT_HERE, OP1F_BAD_NAME, or OP1F_NOT_NOW (not
// from the pack's frame function). From the frame function only: an input function notes the turn it takes and the
// frame function sets the value.
extern "C" int op1f_param_set(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int value) OP1F_NAMED("op1f_param_set");

// Level 8. The effect a place of the unit's rides now (a sound's slot, 0 to 15, or OP1F_PLACE_MASTER): its id in the
// list (an entry's `id`), or 0 when the place plays one of the unit's own effects or none. Any pack may ask it: a page
// that shows a sound's effect names it. From the frame function and the input function.
extern "C" unsigned op1f_rack_at(const Op1fCard* card, unsigned place) OP1F_NAMED("op1f_rack_at");

// Level 8. The list of every effect the packs registered, the base's, the same from the first frame to the end:
// `count` set to how many entries it holds (0 to OP1F_EFFECTS_MOST). Answers the first entry, or 0 when no effect is
// registered (or `card` is not one the base uses). From the frame function and the input function. Any pack may read
// it: a pack that offers effects of its own (a chain, a lane) offers these beside them.
extern "C" const Op1fRackEntry* op1f_rack_list(const Op1fCard* card, unsigned* count) OP1F_NAMED("op1f_rack_list");

// Level 8. An instance of the effect `id` made ready at the pack's own place `which` (0 to OP1F_RACK_PLACES - 1, a
// place of the pack's own: a wagon, a lane), so that op1f_rack_run can run it there. The first time a place asks for an
// effect, the effect's working memory is taken (level 6's machinery and limits) and cleared over the frames that
// follow; it is kept for that place and that effect (base 38: until no place of any pack has run it for two frames;
// then the next place that asks may take it over, the same effect first, its memory cleared again over the frames, and
// OP1F_NOT_NOW meanwhile; 64 at most for all packs), and taken again when the place asks for it again. When the place
// held another effect, or none, the effect's `reset` is called for it first (the
// display thread). Answers 0 when the instance is ready, OP1F_NOT_NOW while its memory is being made ready (ask again
// at the next frame), OP1F_BAD_NAME (no such effect), OP1F_TOO_LARGE or OP1F_FAILED (its memory was refused: a line
// says why; the place stays dry), or OP1F_NOT_HERE (the effect was switched off for its cost at that place; from base
// 38 the place gets another go when it asks for another effect and then this one again, or asks for it again after
// neither asking for it nor running anything there for more than two frames). From the
// frame function only. Asking again with the same effect each frame costs a few comparisons.
extern "C" int op1f_rack_ready(const Op1fCard* card, unsigned which, unsigned id) OP1F_NAMED("op1f_rack_ready");

// Level 8. The effect `run->id` run once at the pack's place `which`, on the pack's buffers, from the pack's own sound
// work on the audio thread: the effect's `process` is called with a block whose `place` is OP1F_PLACE_HOSTED + `which`
// and whose `state` is that place's instance. Timed and held to the effect's declared cost as a place of the unit's
// is; switched off at that place when it goes over (a line at the next frame), until it is picked there again after
// another or after a pause in the place's asks (base 38; before, until the next start). Answers 0 when it
// ran, or less than 0, the buffers then untouched: OP1F_NOT_HERE (no instance of that effect is ready at that place,
// or it was switched off), OP1F_BAD_NAME (no such effect, or a run that is not in order). It writes no line and waits
// for nothing.
extern "C" int op1f_rack_run(const Op1fCard* card, unsigned which, const Op1fRackRun* run) OP1F_NAMED("op1f_rack_run");

// Level 8. The places that ride this pack's effect (its ask `ask`) now: bit n for place n (bit 16 the master). From
// the frame function and the input function.
extern "C" unsigned op1f_riding(const Op1fCard* card, unsigned ask) OP1F_NAMED("op1f_riding");

// Level 9. The three helpers above answer for an ask of kind OP1F_ASK_ENGINE as for one of kind OP1F_ASK_EFFECT, the
// place being a synth sound's slot (0 to 7) and the values the engine's (up to eight): op1f_riding answers bit n for
// slot n.

// The mark of a base of level 9 (a constant 9): a pack with an ask of this kind names it, as its card's source does.
extern "C" const unsigned op1f_level_9 OP1F_NAMED("op1f_level_9");

// Level 10. The value `index` (0 to count - 1) of this pack's effect or engine (its ask `ask`) at `place` as the sound
// plays it now, in the pack's own numbers, into `out`: the value the audio last used, with the unit's LFOs (and any
// pack's offsets on the unit's knob words) applied, where op1f_param_get answers the value as set. Where the place has
// not played in the last few blocks (eight; about 12 ms: no voice of a synth sound for an engine; for an effect, its
// sound not the one that plays, or its effect not called), it answers the value as set, as op1f_param_get does.
// Answers 0, or less than 0: OP1F_NOT_HERE (that place does not ride it now), OP1F_BAD_NAME (no such ask of kind
// OP1F_ASK_EFFECT or OP1F_ASK_ENGINE, or no such value), OP1F_NOT_NOW (not from the pack's frame function or its input
// function). From the frame function and the input function. A page that shows its values draws these, and turns them
// with op1f_param_set as before. It waits for nothing and takes no mask.
extern "C" int op1f_param_heard(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_param_heard");

// The mark of a base of level 10 (a constant 10): a pack that calls op1f_param_heard names it.
extern "C" const unsigned op1f_level_10 OP1F_NAMED("op1f_level_10");

// Base 32: options. Up to here every word is base 31's; this part only adds behind it, and a pack built for base 31 or
// earlier is not touched by it.
//
// A pack that has things the owner may want to set (a full-screen drawing on or off, a speed, a number of steps)
// declares them in one ask of kind OP1F_ASK_OPTIONS, on a card of level 6 or later: up to eight options, each a switch
// (off or on), a list of up to eight words or a number, with the one it starts at, and the place it shows in. The base
// keeps the values for the whole unit in a file of its own (user/pf/.options, a line `<pack>/<option>=<value>` each),
// never in the unit's own settings, and hands them to the pack: op1f_option(card, i) from any thread, and `changed` when
// the owner changes one. A pack with a page of its own (`open`) sets its options from its own page with
// op1f_option_set(card, i, position).
//
// Where the owner sees them: in the unit's own settings (shift + COM, then T1), drawn and turned as the unit's own items
// are (the ochre knob picks a line, the red knob changes it; the names ochre, the chosen one light grey, the values
// red):
//   KEYBOARD, SYSTEM, MIDI  an option of that category, after the unit's own lines. The right side shows seven lines in
//                           all, so KEYBOARD takes four of the packs', SYSTEM one, MIDI two, in the order of the packs'
//                           file names; an option that finds its category full shows on its pack's page under PACKS
//                           instead (the log says so)
//   PACKS                   the row the base adds behind the unit's six: its right side lists the packs that have an
//                           option on their page or a page of their own (`open`), a name a line. The ochre knob picks a
//                           pack; its press opens the pack's own page (`open`), or, for a pack without one, shows that
//                           pack's options in the same right side, where the ochre knob picks and the red knob changes.
//                           More than seven lines: the seven around the chosen one
//
// The kind of ask a base 32 knows beside levels 5 to 9's, for a card of level 6 or later. A base that does not know it
// (base 31 or earlier) leaves the ask aside, with its line for kinds it does not know, and uses the card: a pack reads its
// options through op1f_option imported as optional, and takes the first of each where the name is 0.
//   OP1F_ASK_OPTIONS  the pack's options: what `what` points at is an Op1fOptions
enum { OP1F_ASK_OPTIONS = 10 };   // not 9: packs out there ask for options as 10, and 9 is proposed for files per tape

// An option's kind (Op1fOption.kind).
//   OP1F_OPTION_SWITCH  off (0) or on (1); shown "off" and "on"; `count` 2
//   OP1F_OPTION_LIST    one of `count` words (2 to 8), from 0; shown as the word
//   OP1F_OPTION_NUMBER  a whole number from low to high: low and high are signed bytes (-128 to 127, high above low)
//                       in choices[0][0] and choices[0][1]; `count` 0; `first` the starting number's distance from low;
//                       shown as the number
enum { OP1F_OPTION_SWITCH = 0, OP1F_OPTION_LIST = 1, OP1F_OPTION_NUMBER = 2 };
// Where it is kept and where it shows (Op1fOption.where, one byte: the first of each list OR the second).
//   bits 0 to 3, where it is kept: OP1F_OPTION_UNIT the same value whatever the tape (the only place base 32 keeps);
//     OP1F_OPTION_TAPE reserved for a value kept with each tape: base 32 keeps it for the unit
//   bits 4 to 7, its category: OP1F_OPTION_IN_PACKS its pack's page under PACKS, OP1F_OPTION_IN_KEYBOARD,
//     OP1F_OPTION_IN_SYSTEM, OP1F_OPTION_IN_MIDI after the unit's own lines there. A category base 32 does not know
//     shows under PACKS
enum { OP1F_OPTION_UNIT = 0, OP1F_OPTION_TAPE = 1, OP1F_OPTION_KEPT = 0x0f,
       OP1F_OPTION_IN_PACKS = 0x00, OP1F_OPTION_IN_KEYBOARD = 0x10, OP1F_OPTION_IN_SYSTEM = 0x20, OP1F_OPTION_IN_MIDI = 0x30,
       OP1F_OPTION_CATEGORY = 0xf0 };
enum { OP1F_OPTIONS_MOST = 8, OP1F_CHOICES_MOST = 8, OP1F_OPTION_NAME = 16, OP1F_CHOICE_NAME = 8 };
// The two sizes of the table: without `open` (the first packs that asked for options) and with it.
enum { OP1F_OPTIONS_BYTES_FIRST = 708, OP1F_OPTIONS_BYTES = 712 };

// One option: its name (1 to 15 characters, zero padded; the unit shows it in its own capitals), its kind, how many
// choices (2 for a switch, 2 to 8 for a list, 0 for a number), the one it starts at, where it is kept and shows, and
// for a list the words of its choices (1 to 7 characters each), for a number its low and high.
struct Op1fOption {
    char name[OP1F_OPTION_NAME];                        // +0
    unsigned char kind, count, first, where;            // +16
    char choices[OP1F_CHOICES_MOST][OP1F_CHOICE_NAME];  // +20: a list's words; a number's low and high; a switch's zeros
};   // 84 bytes

// The options: what its ask points at; one such ask a card, its `bytes` OP1F_OPTIONS_BYTES (or
// OP1F_OPTIONS_BYTES_FIRST: no `open`). The head's `flags` is 0 and its `shown` 0.
//   changed  called on the display thread, outside the pack's frame function, before the frame functions of the frame
//            after the owner changed option `option`; `value` is what op1f_option answers for it from then on (a
//            position: a number's distance from its low). Like level 7's `changed`, it notes what it is told and
//            does no more. Not called for a value the pack set itself (op1f_option_set). 0 for none
//   count    how many options follow, 1 to OP1F_OPTIONS_MOST
//   open     0, or the pack's own page of settings: called on the display thread, outside the pack's frame function,
//            before the frame functions of the frame after the owner pressed the ochre knob on the pack's name under
//            PACKS; the pack opens its page then (op1f_page_open) and closes it when it is done. With `open`, the base
//            shows none of the pack's options under PACKS: the page is the pack's, and it sets them with
//            op1f_option_set. Its options of KEYBOARD, SYSTEM and MIDI show there as ever
// The base leaves the ask aside, with a line, when an option is not in order (a kind it does not know, a count or a
// first outside its kind's, a name not 1 to 15 characters) or `changed` or `open` is not a function of the pack.
struct Op1fOptions {
    Op1fKindHead head;                                  // +0
    void (*changed)(unsigned option, unsigned value);   // +28
    unsigned count;                                     // +32
    Op1fOption options[OP1F_OPTIONS_MOST];              // +36
    void (*open)();                                     // +708: in a table of OP1F_OPTIONS_BYTES only
};   // 712 bytes
static_assert(sizeof(Op1fOption) == 84 && sizeof(Op1fOptions) == (unsigned)OP1F_OPTIONS_BYTES &&
              OP1F_PLACE_IN(Op1fOptions, options) == 36 && OP1F_PLACE_IN(Op1fOptions, open) == (unsigned)OP1F_OPTIONS_BYTES_FIRST,
              "the table of options is 712 bytes, `open` at +708");

// The value of the card's option `option` (from 0, in the ask's order) as a position, 0 to 255 for every kind and never
// below 0 for a real option: a switch 0 or 1, a list its word's number from 0, a number its distance from its low (the
// pack adds its own low: the number is low + the position). Before the base has read its file it answers the option's
// start (`first`); for a card the base does not use, a card with no such ask (or one left aside), or an option it does
// not list, OP1F_BAD_NAME. From any thread: it reads one byte.
extern "C" int op1f_option(const Op1fCard* card, unsigned option) OP1F_NAMED("op1f_option");

// Sets the card's option `option` to `value`, a position as op1f_option answers it (a number's distance from its low).
// A pack with `open` sets its options from its own page with it: the base shows none of them under PACKS. From the
// pack's own frame function only (the display thread): elsewhere OP1F_NOT_NOW. A card the base does not use, a card
// with no such ask (or one left aside), an option it does not list, or a value past the option's last (a switch 1, a
// list its count - 1, a number high - low): OP1F_BAD_NAME. Else 0: the value is kept, the base's file written as after
// a change in the settings (about a second and a half later), a line of the settings that shows it written again;
// `changed` is not called (the pack set it).
extern "C" int op1f_option_set(const Op1fCard* card, unsigned option, unsigned value) OP1F_NAMED("op1f_option_set");

// The mark of a base that carries out an ask of kind OP1F_ASK_OPTIONS (a constant 1).
extern "C" const unsigned op1f_options_1 OP1F_NAMED("op1f_options_1");

// Base 33: a page that holds its picture. Up to here every word is base 32's; this part only adds behind it, and a pack
// built for base 32 or earlier is drawn as before. Op1fFrame stays as it was (so that a pack that compares `bytes` with
// its size keeps its meaning): base 33 hands an Op1fFrameHeld, the frame and two words behind it, and says 48 in
// `bytes`. Read the two words with op1f_frame_held and op1f_frame_picture, which answer for an older base too.
//
// In every frame the firmware clears the picture to black and draws the page's document at 77 into it, and the packs
// paint over that. A page that changes little from frame to frame (a few numbers, a moving dot) may ask the base to
// leave that out: then the firmware draws nothing of the page in the frame, and the frame costs the page only what the
// pack repaints. The display has two pictures and draws into them in turn, so a held picture is two frames old: it is
// the picture of the last frame with the same `picture`, with everything painted into it then. What a pack painted
// stays until it is painted over.
//
// How a pack holds its picture:
//   - its frame function calls op1f_page_hold in every frame the hold should go on, while its page is the one shown.
//     A frame without the ask is followed by a whole picture;
//   - in each frame it reads `held` (op1f_frame_held, below). 0: the page was drawn whole, so it paints all of its own, as on base 32. 1: it
//     repaints what changed since the frame with the same `picture` (two frames before): what it painted into the
//     other picture in the frame between is not in this one;
//   - the base draws the page whole by itself, for the next three frames, whenever the picture could be other than
//     the one kept: the page comes on the screen, or another document comes under 77; the base changes the document
//     (op1f_show and op1f_move where they change a part, op1f_text and op1f_text_set, a picture made with op1f_picture);
//     one of the unit's popups, the base's notice or its top page lies over the page, or the power popup is up; the
//     base paints the ask's name on a stand-in's blank page; the pack asks it (`redraw`). The pack needs to know none
//     of these: it reads `held`;
//   - a change the base cannot see (anything a pack changes in its picture by other means than those helpers) is said
//     with `redraw`.
// Something that moves over a held picture leaves its old place painted: keep what lies under it with op1f_paint_copy
// before painting it, and paste that back with op1f_paint_block (a `key` of -1) in the frame with the same `picture`.

// What base 33 hands a frame function: the frame, and behind it
//   held     1 when the firmware drew nothing of the page in this frame (the page asked to hold its picture with
//            op1f_page_hold): the canvas holds the picture of two frames before, with everything painted into it then;
//            0 when the page was drawn whole
//   picture  which of the display's two pictures this frame's canvas is, 0 or 1. It alternates frame by frame: a held
//            canvas is the one of the last frame with the same `picture`
struct Op1fFrameHeld { Op1fFrame frame; unsigned held, picture; };   // 48 bytes on base 33
static_assert(sizeof(Op1fFrameHeld) == 48 && OP1F_PLACE_IN(Op1fFrameHeld, held) == 40 && OP1F_PLACE_IN(Op1fFrameHeld, picture) == 44,
              "base 33 hands a frame function 48 bytes: level 4's 40 and two words behind them");

// `held` of the frame a frame function is handed: 0 on a base older than 33 (every picture is drawn whole there).
static inline unsigned op1f_frame_held(const Op1fFrame* now) {
    return now && now->bytes >= (unsigned)sizeof(Op1fFrameHeld) ? ((const Op1fFrameHeld*)now)->held : 0u;
}

// `picture` of the frame a frame function is handed: 0 or 1; 0 on a base older than 33.
static inline unsigned op1f_frame_picture(const Op1fFrame* now) {
    return now && now->bytes >= (unsigned)sizeof(Op1fFrameHeld) ? ((const Op1fFrameHeld*)now)->picture : 0u;
}

// Asks the base to keep the page's picture in the next frame: the firmware's clearing and drawing of the page are left
// out, and that frame's `held` says 1 (unless the base draws whole for a reason above). From the card's frame function,
// while its page is the one shown (the frame's `page` is `card`); ask it in every frame the hold should go on.
// `redraw` not 0: the next two frames at the least are drawn whole, for a change the base cannot see. Answers 1 when
// the ask is taken, 0 when not: another page shown, or a call from elsewhere than the card's frame function.
extern "C" int op1f_page_hold(const Op1fCard* card, unsigned redraw) OP1F_NAMED("op1f_page_hold");

// Copies `w` by `h` pixels of this frame's picture from its (x, y) into `pixels`, row by row from the top, `stride`
// pixels from one row's start to the next: the layout op1f_paint_block takes back (with a `key` of -1). The pixels of
// the block that lie outside the picture are not written. The paint helpers' rules (above): only from the pack's own
// frame function and with this frame's canvas, else nothing and counted; nothing for a `w` or an `h` of 0 or less or
// more than 480, or for `pixels` 0. Answers 1 when anything was copied, 0 when not.
extern "C" int op1f_paint_copy(void* canvas, int x, int y, int w, int h, unsigned short* pixels, int stride) OP1F_NAMED("op1f_paint_copy");

// The mark of a base whose page may hold its picture (a constant 1): a pack that calls op1f_page_hold or op1f_paint_copy
// names it.
extern "C" const unsigned op1f_hold_1 OP1F_NAMED("op1f_hold_1");

// Base 35: layers (step R11, 2026-11-24). Up to here every word is the earlier bases'; this part only adds behind it,
// and a pack built for base 34 or earlier runs as before: an effect without the flag below rides as level 8 says, and
// nothing here is offered unasked.
//
// What base 35 adds. A pack's effect may be a layer (OP1F_EFFECT_LAYER): it shows in the unit's lists of effects as
// any pack's effect, but where it rides, the unit's own effect stays under it and plays as always, whatever the owner,
// a pack or MIDI puts there, and the layer's `process` (if it has one) comes after it. A new kind of ask, OP1F_ASK_LFO,
// is the same for a sound's list of LFOs: a row that rides over the unit's own LFO and never reaches the firmware.
// Three helpers start and end a layer's ride and choose its page (op1f_layer_start, op1f_layer_end, op1f_layer_page),
// and the mark op1f_layer_1 says them; no new level (level stays 10). And for every ride, layer or not, the unit's
// list opens with its cursor on the pack's row.
//
// For every ride, from base 35 on: the unit's list of effects (shift and T3, and the mixer's) opens with its cursor on
// the row of the pack's effect the place rides, where an older base opened it on the row of the unit's effect it stands
// on. Nothing to ask for: it is the base's.
//
// A layer. An effect whose head's `flags` holds OP1F_EFFECT_LAYER does not stand on one of the unit's effects: it lies
// over whichever the place holds. The owner meets it so:
//   - the list: its row shows among the unit's own and the packs' effects, by its name, for every sound of the synth and
//     the drum and for the master, and the list opens with its cursor on it while the place rides it;
//   - the pick: the unit's list picks as its cursor moves, so a walk to the layer's row passes other rows and puts each
//     on the place. When the cursor stops on the layer's row, the place rides it, and the base puts back under it the
//     unit's own effect as the place held it when the list opened: its type, its eight knob words and its switch. A
//     walk off the row ends the ride; coming back to it starts it again over the effect of the list's opening;
//   - the sound: the unit's own effect plays as always, switched by the unit's own key, its knobs the unit's, moved by
//     the unit's LFOs; then the layer's `process`, once a block where the place plays, after the unit's effect when that
//     is on and where it would have played when it is off (`on` in the block says which). A layer may have no
//     `process` (0): it is then a row, a ride, a member and a page, and its sound work, if any, is the pack's own (a
//     stage of level 6, or an effect it runs through op1f_rack_run);
//   - the unit's effect under it may change while it rides: another of the unit's effects put there by a pack, by the
//     owner's own pages or by MIDI. The ride goes on over the new one. It ends by a pick of another row in the list, by a
//     sound loaded onto the place without the base's member naming it (or with another), or by op1f_layer_end;
//   - the file: the unit's own effect is saved as the owner left it, its switch too (never forced off), and the base's
//     member op1f_effect names the layer. Without the pack, under an older base, or on the original firmware, the sound
//     plays the unit's own effect as saved. The master the same, its ride in the base's store;
//   - the page: T3 on a place that rides a layer shows the unit's own effect's page, as if no layer were there, until
//     the pack chooses its own (op1f_layer_page): then level 5's page that stands in (op1f_page_stand_in, the ask's
//     number), or the base's blank page with the layer's name until the pack gives one. The pack may switch between the
//     two from its frame function, as often as it likes.
// A layer's table is level 8's Op1fEffect, read so: `rides` all zeros; `count` 0 (the knobs are the unit's effect's: no
// value of the layer's is on them, the helpers op1f_param_get, op1f_param_set and op1f_param_heard answer OP1F_BAD_NAME
// for it, and the LFO pages name the unit's effect's knobs); `process` 0 or a function, and `cycles` 0 with no
// `process`, else as level 8 has it; `reset`, `state_bytes` and `state_align` as level 8 has them. op1f_riding and
// op1f_rack_at answer for a layer as for any effect; op1f_rack_ready answers OP1F_BAD_NAME for it (a layer runs only
// over a place of the unit's). Its name follows level 8's rule, and is no LFO's of the unit's nor any pack's LFO's.
//
// An effect's flag, beside OP1F_EFFECT_MONO and OP1F_EFFECT_TAILS (a base before 35 leaves aside an effect that has it,
// with its line for a table not in order, and uses the card).
//   OP1F_EFFECT_LAYER  the effect lies over the unit's own effect where it rides, as above. OP1F_EFFECT_TAILS means
//                      nothing for it (its `process` is called every block where the place plays); OP1F_EFFECT_MONO
//                      holds
enum { OP1F_EFFECT_LAYER = 4 };
//
// For a layer's `process`, the block's `on` is 1 when the unit's own effect under it ran in this block (it is switched
// on), 0 when it did not; `values` is 0. Everything else of Op1fEffectBlock is as level 8 has it. Its `fresh` is 1 at
// the first call after a ride began (a pick, a load, op1f_layer_start, a start) and after the unit's effect under it was
// made anew (another of its effects put there, another sound played).
//
// LFOs. A layer of a sound's list of LFOs (shift and T4 on the synth), one ask of kind OP1F_ASK_LFO. Everything said of
// an effect's layer above holds for it, with the LFO in the place of the effect: the row in the list of LFO kinds, its
// cursor, the pick that puts back under it the unit's own LFO as the place held it when the list opened (its kind, its
// eight words, its switch), the unit's own LFO playing under it as always and changing without ending the ride, the
// three ways it ends, the file (the unit's own LFO saved as left, and the base's member op1f_lfo, beside op1f_effect
// and op1f_engine, naming the layer), and the page on T4 (the unit's own LFO's page until op1f_layer_page chooses the
// pack's). An LFO layer has no function on the audio thread: the unit's LFO acts in many places of a voice, and a pack
// that moves the sound itself does it through level 6's stages. The row never reaches the firmware: no pick, no
// file, no catalog the unit writes ever holds its name (the unit resets a whole sound whose LFO it does not know).
// The places: a synth sound's slot, 0 to 7; with OP1F_LFO_DRUM also a drum sound's, OP1F_PLACE_DRUM + slot (8 to 15),
// from base 37 on. Never the master: the master has no LFO.
//
// The kind of ask a base 35 knows beside levels 5 to 9's and OP1F_ASK_OPTIONS, for a card of level 8 or later. A base
// that does not know it (34 or earlier) leaves the ask aside, with its line for kinds it does not know, and uses the
// card.
//   OP1F_ASK_LFO  a layer of a sound's list of LFOs: what `what` points at is an Op1fLfo
enum { OP1F_ASK_LFO = 11 };   // not 9 (proposed for files per tape) nor 10 (options)
//
// An LFO layer's flags, in its head's `flags`.
//   OP1F_LFO_DRUM  a drum sound may ride the layer too (from base 37 on; bases 35 and 36 let synth sounds alone
//                  ride and accept the flag). The row shows in the drum's list of LFOs either way (the unit builds the
//                  drum's list from the same rows as the synth's); without the flag, a pick of it on a drum sound starts
//                  no ride, hands the unit nothing, and the drum's LFO stays as the list's cursor left it
// The limits:
//   OP1F_LFOS_MOST  how many LFO layers the packs may register in all; an ask beyond it is left aside with a line
enum { OP1F_LFO_DRUM = 1 };
enum { OP1F_LFOS_MOST = 8 };

// An LFO layer: what its ask points at.
//   head  level 5's head: `name` is what the list of LFO kinds shows (1 to 15 characters of a-z, 0-9, ' ', '-' and '_':
//         none of the unit's LFO kinds, "tremolo", "value", "random", "element", "midi", "velocity", nor a name an
//         earlier pack registered for an LFO; the lists of effects and of LFOs are separate, so an effect's name may be
//         an LFO's, though the pack tool refuses one name for an effect and an LFO layer of the same card); `bytes` 28 at base 35 (a later base may read more); `flags` OP1F_LFO_...;
//         `shown(where, how)` is told when the layer's own page comes on the screen and goes, as an effect's: `where`
//         the place, `how` OP1F_SHOWN_UP, with OP1F_SHOWN_OFF while the unit's LFO under it is switched off, and 0 when
//         the page leaves the screen. Not told while the unit's own LFO page shows
// The base leaves the ask aside, with a line, when it is not in order. The layer names no address of the unit's and
// changes no place of the firmware. It needs op1f_layer_1.
struct Op1fLfo { Op1fKindHead head; };   // 28 bytes at base 35
static_assert(sizeof(Op1fLfo) == 28, "an LFO layer's table is the head, 28 bytes at base 35");

// The helpers. Each takes the card, the ask's number in its list (from 0: an ask of kind OP1F_ASK_EFFECT with
// OP1F_EFFECT_LAYER, or of kind OP1F_ASK_LFO) and a place. op1f_riding, op1f_param_get and the others of level 8 serve
// an ask of kind OP1F_ASK_LFO too: op1f_riding answers bit n for place n, the value helpers OP1F_BAD_NAME.

// Starts the layer's ride at `place` (0 to 15, or OP1F_PLACE_MASTER for an effect), as a pick of its row would, but over
// the unit's own effect or LFO as the place holds it now: nothing of the unit's is put back or changed. A ride of
// another pack's effect or layer there ends first (as a pick of another row would); the base's member is written, and
// `fresh` says it at the next block. For what a pack brings back of its own (a sound it kept before the layer was
// there). Answers 0 when the place rides it, also when it rode it already; or less than 0: OP1F_BAD_NAME (no such
// layer of the card, or a place it cannot ride: the master for an LFO, a drum's place for an LFO without
// OP1F_LFO_DRUM or under base 35 or 36), OP1F_NOT_NOW (not from the pack's frame function; or a sound loaded onto the place is not told yet: try
// again at the next frame). From the frame function only. Base 37's LFO layers: a load in the very frame of the call
// may not be counted yet, so the start answers 0 and the ride ends at the next frame as "a sound without it was
// loaded"; start an LFO layer's ride a few frames after a load (lfo 12 waits 4).
extern "C" int op1f_layer_start(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_layer_start");

// Ends the layer's ride at `place`: the base's member taken away, the unit's own effect or LFO left as it is, playing
// as it plays (its switch, its knobs, its type). Answers 0, OP1F_NOT_HERE (the place does not ride it), OP1F_BAD_NAME,
// OP1F_NOT_NOW (not from the pack's frame function). From the frame function only.
extern "C" int op1f_layer_end(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_layer_end");

// Which page T3 (an effect) or T4 (an LFO) shows for a place that rides the layer: OP1F_LAYER_PAGE_UNIT, the unit's own
// page of the effect or LFO under it, as if no layer were there (where every ride of the layer starts); or
// OP1F_LAYER_PAGE_OWN, the layer's own: the document op1f_page_stand_in gave for the ask, or the base's blank page with
// its name. It holds for every place that rides the layer, until it is called again. While one of the two pages is on
// the screen the other takes its place when the unit next asks which screen to show. While the layer's own page is up,
// every key and knob is offered to the pack's input function first, as for any page; while the unit's page is up, the
// pack's input function is offered them as a card's without a page. Answers 1, or 0: no such layer of the card,
// `which` not one of the two, or a call from elsewhere than the pack's frame function.
enum { OP1F_LAYER_PAGE_UNIT = 0, OP1F_LAYER_PAGE_OWN = 1 };
extern "C" int op1f_layer_page(const Op1fCard* card, unsigned ask, unsigned which) OP1F_NAMED("op1f_layer_page");

// The mark of a base that carries out layers (a constant 1): a pack with an effect that has OP1F_EFFECT_LAYER, an ask
// of kind OP1F_ASK_LFO, or a call of the three helpers above names it, so that a base before 35 leaves it out.
extern "C" const unsigned op1f_layer_1 OP1F_NAMED("op1f_layer_1");

// Base 36: a held page whose parts the firmware draws again. Up to here every word is the earlier bases' (base 35's
// layers last); this part only adds behind it, and a pack that does not call it is drawn as before.
//
// A page held with op1f_page_hold may not change its document: any change makes the base draw it whole. A page whose
// parts keep moving asks instead with op1f_page_parts, and names in the same frame each rectangle of its picture that
// changed and the parts of its document that lie there (op1f_page_redraw). In the next frame, when the base holds the
// page, the firmware draws, for each rectangle in turn, the page's document cut to that rectangle with only the named
// parts shown: each named part with everything under it as its own display says, and the groups that hold it; every
// other part of the document is left out of that drawing. Outside the rectangles the picture stays as it was two
// frames before (the frame with the same `picture`). When the base draws whole instead (base 33's list), the names are
// not used. So the page's look is the firmware's own, and a frame costs what the named parts cost to draw (about the
// same as in a whole picture, part for part: the cut saves little), plus a little a rectangle.
//   - name what changed since the frame with the same `picture`: where a part was and where it is now;
//   - name everything that lies in a rectangle, its ground first among them: the base does not clear a rectangle, and a
//     part not named is missing there for that frame;
//   - only parts of the page's own document, as op1f_node or op1f_text_node answered them, and not one twice in one
//     rectangle;
//   - in a frame where the page asks op1f_page_parts, the changes its card makes from its frame function (op1f_show,
//     op1f_move, and op1f_text and op1f_text_set on the document under 77) no longer make the base draw whole;
//     everything else in base 33's list still does, and so does a frame with more rectangles or parts than below, a
//     rectangle refused, or rectangles named in a frame that then does not ask;
//   - a page that asks changes its document only from its frame function: around each drawing the base hides and
//     shows parts itself and puts back what it found.

// The most a frame may name: rectangles, and parts in all its rectangles together.
enum { OP1F_REDRAWS_MOST = 8, OP1F_REDRAW_PARTS_MOST = 160 };

// Asks the base to hold the page's picture in the next frame and to draw again only the rectangles named in this frame
// with op1f_page_redraw (none named: the picture is held as it is). In place of op1f_page_hold, from the card's frame
// function while its page is the one shown, in every frame the asking should go on. Answers 1 when the ask is taken,
// 0 when not (another page shown, or a call from elsewhere).
extern "C" int op1f_page_parts(const Op1fCard* card) OP1F_NAMED("op1f_page_parts");

// Names a rectangle to draw again in the next frame: `w` by `h` pixels from (x, y), cut to the picture, and the
// `count` parts of the page's document drawn in it. From the card's frame function while its page is the one shown,
// before or after op1f_page_parts in the same frame (named without the ask, the frame is drawn whole). Answers 1 when
// taken; 0 when not (another page shown, a call from elsewhere, a rectangle of nothing once cut to the picture, no
// parts, more than OP1F_REDRAWS_MOST rectangles or OP1F_REDRAW_PARTS_MOST parts in this frame, a part that is 0): the
// next frame is then drawn whole.
extern "C" int op1f_page_redraw(const Op1fCard* card, int x, int y, int w, int h, void* const* parts, unsigned count) OP1F_NAMED("op1f_page_redraw");

// The mark of a base whose held page's parts the firmware draws again (a constant 1). A pack that must have it names it;
// a pack that also runs without it asks for op1f_page_parts and op1f_page_redraw weakly and calls them only when their
// address is not 0.
extern "C" const unsigned op1f_parts_1 OP1F_NAMED("op1f_parts_1");

// Base 39: a sound's sample (step R17, 2026-11-25). Up to here every word is the earlier bases' (base 36's parts last);
// this part only adds behind it, and a pack that does not call it runs as before. No new level, kind of ask or field of
// the card: level stays 10.
//
// Every synth sound and every drum sound has a sample of its own, whatever it plays: the unit's sampler plays it, and
// the unit's drop key on the sampler's page puts what the tape has lifted into it. From base 39 a pack's engine can have
// the same. op1f_sample_drop asks the unit to put what is lifted into the sample of a synth sound that plays one of the
// card's own engines; the unit does the work in its own turn, as for its sampler, and keeps it over a restart as it
// keeps the sampler's. op1f_sample_get reads any sound's sample, its engine or not.
//
// How, in order: on the key's down, fade your voices on that sound (the unit mutes nothing); on its up, ask, from the
// input function; in the frame function read op1f_sample_get until `done` is your ticket; read `answer` and `dropped`,
// play again. What the unit never lets happen: two drops at once (one for the whole unit, any pack's); a cut that would
// not fit (it is refused, never cut by the base); a drop into a sound that does not play the card's engine; anything of
// the pack's running in the unit's turn. Several lifted tracks are mixed as the unit mixes them for its sampler, and a
// cut longer than the sound's `capacity` is cut there by the unit, as for its sampler: `answer` says how much was put in.
//
// Two answers more, beside level 3's and level 8's (the header had none below -8).
//   OP1F_BUSY   a drop is under way (any pack's), or the unit has clipboard work waiting: ask again later
//   OP1F_EMPTY  no cut of the tape is lifted (nothing, or a copied sound)
enum { OP1F_BUSY = -9, OP1F_EMPTY = -10 };

// A sound's sample, as op1f_sample_get hands it (36 bytes at base 39; a later base may hand more: read `bytes`).
//   bytes     what the base wrote
//   data      the sample, 16-bit, left and right in turn when stereo. It stays where it is for the run: hand it to your
//             `render` through your own words. A sample read while the unit writes it sounds wrong, never faults
//   stereo    1 or 0, as the unit holds it for the sound
//   room      the frames its memory holds at that channel count (264,600 stereo, 529,200 mono for a synth sound):
//             a drop never passes it
//   capacity  the unit's own frames for the sound (what a sound's load sets; a sound never loaded says a little more
//             than `room`): a drop puts in at most this many
//   dropped   the frames of the last drop here, or put back at the start; 0 after a sound is loaded there
//   ticket    the drop into this place under way, queued or running; 0 when none
//   done      the ticket of the last drop into this place that ended; 0 when none since the start
//   answer    how `done` ended: the frames put in (0 or more), or a refusal in the unit's turn (less than 0)
struct Op1fSample {
    unsigned bytes;
    const short* data;
    unsigned stereo, room, capacity, dropped, ticket, done;
    int answer;
};
static_assert(sizeof(Op1fSample) == 36 && OP1F_PLACE_IN(Op1fSample, dropped) == 20 && OP1F_PLACE_IN(Op1fSample, answer) == 32,
              "a sound's sample, 36 bytes at base 39");

// Asks the unit to put what the tape has lifted into the sample of synth sound `place` (0 to 7), which must play the
// card's engine `ask` (its ask of kind OP1F_ASK_ENGINE, by its number in the card's list). Only inside the card's own
// input function, for an event on the key thread (the event's `thread`, level 4); any key or knob of the pack's own.
// It waits for nothing. Answers the ticket, 1 or more: the unit does it in its own turn; watch `done`. Or less than 0,
// and nothing is asked of the unit:
//   OP1F_NOT_NOW    not in this card's input function for an event on the key thread; the disk mode is open; the
//                   sound's sample is being put back after a start
//   OP1F_BAD_NAME   no ask of kind engine with that number; a place that is not a synth sound's (a drum sound's place,
//                   OP1F_PLACE_DRUM + slot, on base 39). From base 43 a drum sound's place is taken when the ask is the
//                   card's drum engine (OP1F_ASK_DRUM_ENGINE, below): the drum sampler's own drop, in the unit's turn
//   OP1F_NOT_HERE   the sound does not play that engine now, or a sound loaded there is not told yet
//   OP1F_BUSY       a drop is under way (any pack's), or the unit has clipboard work waiting
//   OP1F_EMPTY      no cut of the tape is lifted (nothing, or a copied sound)
//   OP1F_TOO_LARGE  the cut, as the sound takes it, would pass its memory (`room`)
//   OP1F_FAILED     the sound's sample is not where it should be; a lifted track the unit cannot render; the unit did
//                   not take the job. The log says which
// In its own turn the unit checks again: `answer` is then the frames put in, or OP1F_NOT_HERE, OP1F_EMPTY,
// OP1F_TOO_LARGE, OP1F_NOT_NOW or OP1F_FAILED, with nothing put in and nothing written.
extern "C" int op1f_sample_drop(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_sample_drop");

// Fills `out` for any synth sound's place (0 to 7) or drum sound's (OP1F_PLACE_DRUM + slot). Answers 0, or less than 0:
// OP1F_BAD_NAME (another place, the card not in use), OP1F_NOT_NOW (not from the card's frame function), OP1F_FAILED
// (the sample is not where it should be: only `bytes` written). From the frame function only. It calls nothing of the
// unit's: it reads the base's words and three of the sound's.
extern "C" int op1f_sample_get(const Op1fCard* card, unsigned place, Op1fSample* out) OP1F_NAMED("op1f_sample_get");

// The mark of a base whose packs may drop a lifted cut into their engine's sound and read any sound's sample (a
// constant 1). A pack that must have it names it; under a base before 39 the loader then leaves the pack out. A pack
// that also runs without it asks for op1f_sample_drop and op1f_sample_get weakly and calls them only when their address
// is not 0.
extern "C" const unsigned op1f_sample_1 OP1F_NAMED("op1f_sample_1");

// Base 43: a drum engine. Up to here every word is the earlier bases' (base 39's sample last); this part only adds
// behind it, and a pack that does not ask for it is not touched by it.
//
// A pack may add an engine of its own to the drum's engine list (shift and T1 in the drum's mode), as level 9 lets a
// pack add a synth engine that rides dsynth. A drum engine rides the unit's drum sampler and makes no sound of its own:
// the drum sampler plays on under it (the kit, its sample, the keys' points, the saving and loading of the kit stay the
// unit's), and the engine is the pack's page on T1 and a name in the list and in the sound's file. One ask of kind
// OP1F_ASK_DRUM_ENGINE, on a card of level 8 or later. The owner meets it so:
//   - the engine's name stands in the drum's engine list among the unit's own (dbox, the drum sampler), where the unit
//     sorts it by its name, as a pack's synth engine stands in the synth's;
//   - picked on a drum sound that plays the drum sampler, the sound goes on playing the drum sampler as it was, and T1
//     shows the pack's page: the document op1f_page_stand_in gave for the ask, or the base's blank page with the ask's
//     name until it gives one. Picked on a sound that plays another engine (dbox), nothing changes and the base says a
//     line: pick the drum sampler there first;
//   - every key and knob (the four knobs, shift, the keyboard's keys) goes to the pack's input function first, as for any
//     page; what it does not take goes to the unit as it would on the drum sampler's own page;
//   - another screen of the drum (T2 to T4, shift and T1, another mode) takes the page off as for any page that stands
//     in, and T1 brings it back; the drum sampler (one of its kits), dbox, or a sound loaded that does not name the
//     engine, picked on that drum sound, ends the ride and shows the unit's own page;
//   - the sound's file keeps the engine's name in the base's member op1f_engine, as a synth sound's does; loaded again,
//     the sound rides the engine again. Without the pack, or on a stock unit, the same file loads as the drum sampler
//     with its kit and its keys' points.
// Only one pack holds the drum engine: the first card in the list's order (the packs' file names) with such an ask. The
// ask of any other card is left aside with a line, and that pack runs without it.
//
// The kind of ask a base 43 knows beside the earlier ones, for a card of level 8 or later. A base that does not know it
// (42 or earlier) leaves the ask aside, with its line for kinds it does not know, and uses the card.
//   OP1F_ASK_DRUM_ENGINE  a drum engine: what `what` points at is an Op1fDrumEngine
#define OP1F_BUILD_R21_DRUM_H
enum { OP1F_ASK_DRUM_ENGINE = 12 };   // after OP1F_ASK_LFO (11); 9 stays proposed for files per tape

// A drum engine: what its ask points at; its `bytes` 28 at base 43 (a later base may read more).
//   head  level 5's head: `name` is the engine's name in the drum's engine list and in the sound's file, and what the
//         base's blank page shows until the pack gives a document (1 to 15 characters of a-z, 0-9, space, - and _;
//         never one of the unit's own: dbox, drum, drums, sampler, drum sampler, drum_sampler, synth); `flags` 0;
//         `shown(where, how)` is told when the page comes on the screen and goes: `where` the drum sound's place
//         (OP1F_PLACE_DRUM + slot, 8 to 15), `how` OP1F_SHOWN_UP, and 0 when the page leaves the screen. Another drum
//         sound that rides the engine, picked while the page is up, is told with its own `where`
// The ask names no address of the unit's and changes no place of the firmware. It needs op1f_drum_engine_1.
// The drop (base 39's op1f_sample_drop) takes a drum sound's place, OP1F_PLACE_DRUM + slot, with this ask's number,
// while that drum sound rides the engine: the unit puts the lifted cut into the drum sampler's sample in its own turn,
// as the drum sampler's page does, with the synth's checks and answers; `dropped`, `ticket`, `done` and `answer` of
// op1f_sample_get are a drum place's too. The unit keeps the drum sampler's sample over a restart itself. A drop does
// what the drum sampler page's own drop does, no more.
//
// A pack that writes into its drum sound's sample itself (a cut pasted behind the kit's material, say) asks the unit
// to keep it, as the unit keeps a dropped cut: op1f_sample_changed(card, ask, place), with the drum engine's ask and
// the drum place, from the card's input function for an event on the key thread only, as a drop. A pack that wrote
// the sample in its frame function asks at the next key event of its page. In the unit's turn the base marks the drum
// sound changed (what the drum's own drop does after its import); nothing is imported.
// It answers a ticket, 1 or more (watch op1f_sample_get's `done`; `answer` 0 when marked), or less than 0, with nothing
// asked of the unit: OP1F_NOT_NOW (not in the card's input function for an event on the key thread; the disk mode
// open), OP1F_BAD_NAME (not a drum place, or
// not the card's drum engine ask), OP1F_NOT_HERE (the sound does not ride it now), OP1F_BUSY (a drop or mark under
// way, or clipboard work waiting), OP1F_FAILED (the unit took nothing).
extern "C" int op1f_sample_changed(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_sample_changed");
struct Op1fDrumEngine { Op1fKindHead head; };   // 28 bytes at base 43
static_assert(sizeof(Op1fDrumEngine) == 28, "a drum engine's table is the head, 28 bytes at base 43");

// The mark of a base that adds a pack's drum engine to the drum's engine list (a constant 1). A pack with an ask of
// kind OP1F_ASK_DRUM_ENGINE names it, so that a base before 43 leaves it out.
extern "C" const unsigned op1f_drum_engine_1 OP1F_NAMED("op1f_drum_engine_1");

// Base 44 (hasis, on top of base 43): lists whose words the pack names while it runs. Up to here every word is the
// earlier bases'; this part only adds behind it, and a pack built for base 43 or earlier is not touched by it.
//
// A list of fixed words lives in the pack's card, which nothing changes while the unit runs: it cannot list what is
// only known then (the looks other packs hand the pack at their first frame, say). An option of kind
// OP1F_OPTION_WORDS asks the pack for its words instead: a function of the pack, word(choice), answers the word of each
// choice from 0 (1 to 15 characters the unit's letters draw), and 0 past the last; at most OP1F_WORDS_MOST choices
// are asked for. The base calls it on the display thread only (its frame work, the settings' draw, a turn, and
// op1f_option_set from the pack's frame function); it answers from what the pack holds and does no more.
//
// Where it stands: behind the table's `count` options, in the places a base 43 or earlier leaves unread
// (options[count] and on, one after another, as long as their kind is OP1F_OPTION_WORDS). So an older base sees only
// the `count` options before them and runs the ask as it was; for it the pack's op1f_option(card, i) of such an option
// answers OP1F_BAD_NAME (the pack does without it there); an ask of such options only has `count` 0, which an older
// base leaves aside. In order, base 44 takes `count` + these options together,
// at most OP1F_OPTIONS_MOST; their numbers for op1f_option, op1f_option_set and `changed` follow `count`'s.
//
// Such an option: its name, kind OP1F_OPTION_WORDS, `count` 0, `first` the choice it starts at, `where` 0 (it shows on
// its pack's page under PACKS, kept for the unit), and in its choices' first four bytes the function word (the rest
// zero). Its value is a choice's number, as a list's; the 4th knob steps through the words the function gives at that
// moment. The file keeps the chosen WORD (`<pack>/<option>=<word>`), not its number: at the first frame, before the
// packs hand over their parts, the base takes `first`; at every frame after, while the word kept is not the word of
// the value, it looks for it among the words and, found, takes its number and tells `changed`. A word that is not
// there (its pack taken away) keeps its line in the file until the owner chooses another.
enum { OP1F_OPTION_WORDS = 3, OP1F_WORDS_MOST = 32 };
typedef const char* (*Op1fOptionWord)(unsigned choice);

// Base 45: hosted pages. Up to here every word is the earlier bases' (base 44's lists of words last); this part only
// adds behind it, and a pack built for base 44 or earlier runs as before: a host that does not call the three helpers
// below, and an effect that does not declare wagons, are not touched by it.
//
// Words. A host is a pack that runs another pack's effect at a place of its own, `which` (op1f_rack_ready,
// op1f_rack_run): a chain's wagon, a mixer's lane. An instance is one of the base's 64 records of such runs: a host, its
// `which` and an effect. Each instance has a number n, 0 to 63, which no other instance of any host has while it lives.
//
// What base 45 adds. A host may say which of the unit's places a run plays for (op1f_rack_place), keep for each wagon
// the settings the effect's pack writes for it (op1f_rack_kept), and show the effect's own page for a wagon in the place
// of its own page (op1f_rack_page). An effect's pack may declare that it knows wagons: each instance then gets its own
// place number and its own kept settings. The mark op1f_hosted_1 says these helpers; no new level (level stays 10).
//
// For a host (from its frame function only).
//
// The effect at the host's `which` plays for the unit's `place` (0 to 15, or OP1F_PLACE_MASTER; OP1F_PLACE_NONE for
// none, as before base 45), and `kept` is that wagon's kept text for its effect (what op1f_rack_kept gave; 0 or "" for
// none; a JSON object of at most OP1F_HOSTED_KEPT_MOST bytes). Call it before op1f_rack_ready whenever the wagon at
// `which` changes: a start, another sound played, a wagon moved there, a train loaded. The next op1f_rack_ready at
// `which` takes the text for its effect; a ready of another effect at `which` with no call between drops it. Answers 0,
// or less than 0: OP1F_BAD_NAME (no such card, `which` or place, or a text that is not a JSON object), OP1F_TOO_LARGE
// (a longer text, or 64 places of all hosts hold a place or a text already), OP1F_NOT_NOW (not from the pack's frame
// function). What the effect's pack is told of its place: "Declared or not", below.
enum : unsigned { OP1F_PLACE_NONE = 0xffffffffu };
enum { OP1F_HOSTED_KEPT_MOST = 48 };
extern "C" int op1f_rack_place(const Op1fCard* card, unsigned which, unsigned place, const char* kept) OP1F_NAMED("op1f_rack_place");

// The kept text the effect's pack wrote for the instance at `which`, when it changed since the last call: written into
// `out` with a zero behind it, and its bytes answered (2 or more: an object, "{}" when it holds no member any more). 0:
// unchanged. Or less than 0: OP1F_NOT_HERE (no instance at `which`, or its effect does not declare wagons),
// OP1F_TOO_LARGE (`room` too small for it and its zero: nothing written, and the next call answers it again),
// OP1F_BAD_NAME, OP1F_NOT_NOW (not from the pack's frame function). The host saves it with the wagon, and hands it back
// by op1f_rack_place. Ask it once a frame.
extern "C" int op1f_rack_kept(const Op1fCard* card, unsigned which, char* out, unsigned room) OP1F_NAMED("op1f_rack_kept");

// The page of the effect at `which`, for the place on the screen while it rides the host's layer `ask` (an effect of
// the card with OP1F_EFFECT_LAYER): T3's page becomes the effect's own page for that instance (its page that stands
// in, or the base's blank page with its name), a third choice beside OP1F_LAYER_PAGE_UNIT and _OWN. Call it once a
// frame while the host wants that page; a frame without the call, or op1f_layer_page, ends it. `words` is four ints, in
// and out, as OP1F_RACK_WORDS has them (the unit's knob words of the effect the pack's stands on): in, the instance's
// four words as the host keeps them (as set, not moved by an LFO); out, the words the page changed since the last
// call (the effect's op1f_param_set at the instance's place, or a knob turn no pack took, moved as the unit's knob of
// the effect it stands on would move it), the others as they came in. The host applies them to its wagon and keeps them
// as it keeps a turn. Answers a mask: bits 0 to 3 the words changed, with OP1F_HOSTED_PAGE_UP while the page is on the
// screen; or less than 0: OP1F_NOT_HERE (no ready instance at `which`, or the place on the screen does not ride the
// layer), OP1F_BAD_NAME (no such layer of the card, `which`, or `words` 0), OP1F_NOT_NOW (not from the pack's frame
// function). While the page is up, a knob's turn, click or press that no pack took is not handed to the unit (it would
// move the unit's effect under the layer): a turn moves the instance's word, a click or a press is dropped.
enum { OP1F_HOSTED_PAGE_UP = 256 };
extern "C" int op1f_rack_page(const Op1fCard* card, unsigned ask, unsigned which, int* words) OP1F_NAMED("op1f_rack_page");

// For an effect's pack: the declaration of wagons. An effect knows wagons when its table is OP1F_EFFECT_HOSTED_BYTES
// long (or more) and the word at OP1F_EFFECT_HOSTED_AT holds OP1F_HOSTED_WAGONS. It is a field behind level 8's 180
// bytes and the picture's word at +180 (op1f_wagon_art.h; 0 for no picture), not a flag: a base before 45 reads level
// 8's table and runs the effect as a plain effect, so a pack built for base 45 runs on base 43 and 44 too. The pack tool
// writes it: "wagons": true in the effect's ask. A shorter table, or 0 there, is the fallback below. A layer's is read
// as 0.
enum { OP1F_EFFECT_HOSTED_AT = 184, OP1F_EFFECT_HOSTED_BYTES = 188, OP1F_HOSTED_WAGONS = 1 };
//
// Declared or not.
//   - Declared. Every instance of the effect plays at its own place, OP1F_PLACE_HOSTED + n (32 to 95): `place` in
//     `process` and `reset`, `where` in `shown`, and `place` in op1f_param_get, op1f_param_heard and op1f_param_set,
//     which answer that instance's values (op1f_param_set while its page is up; OP1F_NOT_HERE otherwise). Its kept text:
//     op1f_value_get and op1f_value_set with `where` OP1F_KEEP_HOSTED and `which` n read and write a member of the
//     instance's object, its name `key` (1 to 31 characters of a-z, 0-9 and '_', which the pack's ask of kind
//     OP1F_ASK_KEEPS need not list), its value a JSON text of the kinds level 7 keeps; the whole object at most
//     OP1F_HOSTED_KEPT_MOST bytes (op1f_value_set answers OP1F_TOO_LARGE beyond). The host keeps it with the wagon, in
//     its files and its saved trains. When the host hands another text, a pack whose card has an ask of kind
//     OP1F_ASK_KEEPS with `changed` is told changed(OP1F_KEEP_HOSTED, n, 0) at the next frame, before its frame function.
//     Without a host that calls op1f_rack_place the text is still the instance's, until another effect takes the place.
//   - Not declared (every effect pack written before base 45): the fallback, only meant for packs written before base
//     45. Do not rely on it in new work: it will eventually be deprecated or removed, and a pack that relies on it then
//     loses its page on wagons, nothing worse. While the place a host lent the instance (op1f_rack_place) rides a layer
//     of the host's own card (checked at each use: never the master under such a train, and a pick of another effect
//     there ends it), the base answers the effect's pack as if that place rode the effect: `process` and `reset` get the
//     place (not OP1F_PLACE_HOSTED + which); op1f_riding has its bit for as long as the wagon is lent; `shown` its
//     `where`; op1f_param_get, op1f_param_heard and op1f_param_set at the place answer the instance whose page is up,
//     else the first lent instance of that effect there (op1f_param_set answers OP1F_NOT_HERE while no hosted page is
//     up; op1f_param_get without a page answers the values of its last run). op1f_value_get and op1f_value_set at that
//     place stay the sound's own: two wagons of one sound with the same effect share them. op1f_rack_at(place) still
//     answers the layer. Without op1f_rack_place, every run is as base 44 has it: OP1F_PLACE_HOSTED + which, no page.
enum { OP1F_KEEP_HOSTED = 16 };

// The mark of a base that carries out hosted pages (a constant 1): a host that calls the three helpers above names it,
// so that a base before 45 leaves it out. An effect that declares wagons needs no mark.
extern "C" const unsigned op1f_hosted_1 OP1F_NAMED("op1f_hosted_1");

#endif

// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Base 29's mends M3 and M4 (2026-11-21, after base 28's try on a unit: a sound saved and loaded): a
// sibling of the values unit of base 29's first build, which stays as it was, as do its elders; its stubs are module/build_r4_keeps.s
// as before and four more in module/build_r6b_keeps.s. Nothing a pack sees changes.
//
// M3, a sound loaded from the unit's own list of sounds. The unit's list (shift and the instrument's key, the list of
// sounds) loads the sound under the cursor by a job of its own, on the firmware's background worker, the queue the
// other loads use: the synth's job reads the preset with the plain reader and puts the document on the slot's record
// through the synth's put (record, slot, document, 0), which assigns it and runs the firmware's pass that keeps only the
// members it knows; the drum's job is the same with the drum's put. The base watched only the load of a sound sent over
// MIDI (the places of I9, F1 and F4), so a sound picked in the list told nobody: no ride, no train, no pack's value came
// back with it. Here both jobs are taken at their read (L1 the synth's, L3 the drum's: the members taken into the
// waiting holder at once, as F2 and F3 take them) and at their put (L2 and L4: the firmware's put, then the slot's
// values become the sound's, marked with its name and type, and OP1F_KEPT_LOADED is told at the next frame, as F4 does
// for a sound sent over MIDI); a synth sound is handed to the engines unit before the put, as F4 hands it. The packs,
// the effects unit (its ride and switch) and the engines unit (its ride) then read the sound's values at their next
// frame, as at a load over MIDI.
//
// M4, the mark of a slot's sound. The store user/pf/.sounds keeps a slot's values with the mark of the sound they belong
// to, its `name` and `type`; at a start a slot whose record shows another mark lets its values go ("their sound
// changed"). The mark was taken at a load the base was told of and when a value was first set on a slot that had none,
// so the unit's own save, which names the sound anew and puts it back on the slot's record, left the old mark: the next
// start let the slot's values go. Here the mark follows every sound the base knows was put on the slot: at a list load
// (M3) from the loaded document; at a save from the document written (its new name, set before the base's place of the
// save, and its type), held as waiting until the record shows it (the save puts the document on the record only once
// its file is written: a save whose file the disk refused leaves the record and the mark as they were) and then the
// slot's mark, the store written again when the slot holds values. A load or a change of engine on the slot meanwhile
// ends the wait (its own mark is the newer).
//
// Step R6, the base pack 29: values a pack keeps in the unit's own files (level 7), with what levels 8 and 9 add. A
// sibling of base 28's values unit, which stays as it was: its stubs are still module/build_r4_keeps.s,
// everything of it is here as it was, and one thing is added for level 9 (synth engines): the base's second own member,
// op1f_engine (the name of the engine a synth sound rides, a JSON string), kept as op1f_effect is: at the top of a synth
// sound's preset (taken where the unit reads it, written where it writes it) and for each synth slot in the base's
// store, never with the drum or the master. It is the second name of the base's list, its owner the base: a pack's key
// of that name is left aside as op1f_effect's is, and packs keep sixty-four names beside the two. In the store it is
// listed only while a slot holds it, so a store with no engine member is base 28's byte for byte. The engines unit
// reads and sets it from the display thread (op1f_keeps_engine_get, op1f_keeps_engine_set) and is told what changed
// through op1f_engines_changed (weak), at the moments op1f_effects_changed is told. And at the place where a sound is
// put on a synth slot's record, before the firmware's own assign, the engines unit is handed the sound: for a synth
// preset (or a synth sampler's) loaded from a file, with the member as this unit took it at the read (op1f_engines_assign
// with OP1F_KEPT_LOADED and the name, or 0), so that the slot's ride is set before the unit builds the sound; for the
// engine job (the engine list or MIDI picked an engine), with OP1F_KEPT_ENGINE, so that the engines unit names the sound
// after a pack's engine. The engines unit's names are weak: away, this unit is base 28's in all it does but the member's
// name kept (and a store that never holds it). What base 28's unit said follows and holds.
//
// Step R5, the base pack (base.pack 28): values a pack keeps in the unit's own files, level 7's kind of ask, with what
// level 8 adds to it. A sibling of base 27's values unit, which stays as it was, as do its elders;
// the stubs are still module/build_r4_keeps.s (OS 1.7.0+0), and the unit names no address base 27's did not. What a
// pack sees is in module/build_r5_card.h, what the base's units share in module/build_r5_base.h. A pack built for base
// 26 or 27 runs under it unchanged.
//
// What changed from base 27's unit, and why. Two things, both for level 8 (effects).
//
// The base's own member, op1f_effect. A sound whose effect is a pack's is saved with the unit's own switch of the
// effect off: the unit's files never say such an effect is on. Which pack's effect a place rides, and whether it is
// switched on, is kept by the base, as a member of its own under the name op1f_effect, kept as a pack's values are: at
// the top of a sound's preset of the synth and of the drum (taken where the unit reads the preset, written where it
// writes it), for each slot in the base's store user/pf/.sounds, and for the master in the store alone. Its text is
// the effects unit's ({"name": "crush", "on": 1}); this unit keeps a JSON text, as for a pack, and a member whose
// effect is away is kept as it is. It is the first name of the base's list, its owner the base: a pack's key of that
// name is left aside (line 2), as a name of the unit's own files is, and packs may still keep sixty-four names beside
// it. The effects unit reads and sets it from the display thread (op1f_keeps_base_get, op1f_keeps_base_set) and is
// told what changed through op1f_effects_changed, a weak name (away, nothing is told), at the moments and with the
// words a pack's `changed` is told (OP1F_KEPT_LOADED, _ENGINE, _SAVED, _START, _DROPPED), in op1f_keeps_frame, before
// the packs' `changed` of the same place: so before the effects unit's frame work and every frame function, which the
// cards unit calls after op1f_keeps_frame. At the start both ways hold: the store is read in the first call of
// op1f_keeps_frame, which the cards unit makes before its first call of op1f_effects_frame, and OP1F_KEPT_START is told
// for each place whose values the store kept. Beside level 7's cases, a set of the base's member answers OP1F_NOT_NOW
// while a sound loaded onto that slot waits to be told (base 27's look for the tape, made here for the base's own
// member at every place): the effects unit sets it again at the next frame, after it has read what the sound holds.
//
// Values with the master (OP1F_KEEP_MASTER). A key of a card of level 8 may name the master in its `files`: its value
// is kept in the base's store alone (the unit keeps no file of the master's that a pack may write in), across a start,
// and told at the start; `which` is 0. A card of a level before 8 takes the bit as no place, and a key with nothing
// else is left aside as one with no place (line 2). The master's values have a holder of their own, with a slot's room,
// and carry no mark: the master is no sound loaded from a file.
//
// The store grows by a section a base before 28 reads past: `master`, {"names":[...],"values":{...}}, the master's
// values and their names, written only while the master holds a value. The list `names` holds what base 27's held,
// each name with its places among the slots and the tape (a name kept with the master alone is named in `master`
// only), and op1f_effect only while a slot holds it: a store with no ride and nothing with the master is base 27's,
// byte for byte, and `version` stays 1. Base 27 run on such a store (the way back) reads every slot, op1f_effect among
// the values as the name of a pack that is away, and leaves `master` aside; the first store it writes then has no
// `master`, and what was kept with the master is gone (a base before 28 has no master to keep values for). This unit
// reads base 27's store as base 27 does. A `master` that is not as this unit writes it makes the whole store read as
// not written (line 5, its reason 10), as any other part does.
//
// The room. The master's holder is a slot's; to stay under the zeroed data's limit the first frame's mark of a record
// is a mark alone (Mark), no longer a whole holder.
//
// What base 27's unit said follows and holds, but where these words say otherwise.
//
// Step R4c, the base pack (base.pack 27): values a pack keeps in the unit's own files, level 7's kind of ask
// (OP1F_ASK_KEEPS, its table an Op1fKeeps, module/build_r4b_card.h). A sibling of base 26's unit,
// base 26's values unit, which stays as it was, as does its elder base 24's values unit; the stubs are still
// module/build_r4_keeps.s (OS 1.7.0+0). No new level: a pack built for base 26 runs under it unchanged.
//
// What changed from base 26's unit, and why. Two things, both about a tape load that comes while a pack or the tape's
// save is at work on the tape before it.
//
// A pack's tape value set in the frame of a load. The firmware's tape load runs on the key's thread, above the display
// thread, so it can come between the base's word to the packs (op1f_keeps_frame) and a pack's frame function; the base
// replaces the tape's values at once and tells the packs only at the next frame. Base 26 took a pack's set of a tape
// value in that window into the new tape's values: a pack that sets what it sees of the unit each frame (the selected
// track, say) set the old tape's state, or what the firmware's load had just made of it, into the new tape, and read it
// back at the tell as if the new tape's file had held it. Here op1f_value_set answers OP1F_NOT_NOW for a tape value
// while a tape load waits to be told: the bit OP1F_KEPT_LOADED or OP1F_KEPT_MADE in told[TAPE], read inside the same
// mask as the write (put_in), so no load can come between the look and the write. Nothing is set; at the next frame the
// pack is told the load before its frame function runs, and may set again. A sound's slot is not refused so.
//
// The tape's save cut by a tape load. The firmware's save of the tape runs on its background worker, the lowest of its
// threads, and the base puts the tape's values into the tape's document there (op1f_keeps_tape_saving), from a copy it
// takes once at the start. A tape switch on the key's thread can come in the middle: it writes the document as it stands
// into the tape before, reads the next tape's file into the document and loads it. Base 26's worker then went on with
// its old copy and set the tape before's remaining values into the next tape's document. Here the base counts the
// tape's loads (tape_loads, moved inside the mask that puts a load's values in) and the save reads the count with its
// copy, inside one mask: before each value it looks again, and the moment the count has moved, the rest of the copy is
// left out (line 15). The look right before the firmware's setter cannot see a load that comes while the setter itself
// waits: the setter waits for a switch that is writing or reading a tape's file, so a set that reaches it then lands
// in the next tape's document once the switch is done. The base looks a third time after
// each set: when the count moved during it, the value may be in the next tape's document, and the next tape's own
// value of that name is set again over it; when the next tape holds none of that name and its document now has one,
// the member stays (the base has no way to take a member out), and line 16 says so. A save that was cut tells no
// pack OP1F_KEPT_SAVED.
//
// Base 26's own change from base 24's unit, kept here. The tape's load hands the base the flag its caller hands the firmware's load.
// The firmware gives it 1 for a tape the owner makes in the tape list (T3) and for a copy of a tape (shift + T3), and 0
// for every other load; it reads the tape's file into the tape's document before the load in every case, and for a
// copy that file is the original's, copied as a file. The elder took the flag for "no file read" and cleared the
// tape's values, so a copy's document still held the original's members while the base held none of them as its own:
// at the copy's next write every value a pack set there was refused as a member of the firmware's own file, and the
// next load of the copy with the flag 0 handed the original's values back. Here the tape's values are taken from the
// tape's document at every load, whatever the flag: a copy keeps the original's values, as its file does, and a pack's
// value set there is written over them; a tape made anew is the template's file and holds none. A pack is told such a
// load (the flag not 0) as OP1F_KEPT_MADE in place of OP1F_KEPT_LOADED, so that it can set what it keeps of the tape's
// own state as the firmware sets its own for a new tape or a copy (its load sets tracks 1 to 4's levels and pans to
// their start then); a tape switched to (the flag 0) stays OP1F_KEPT_LOADED. When both come between two frames, both
// are told, the earlier load first and the later last, in OP1F_KEPT_LOADED's place before the other words of the tape.
// A pack's value of the tape reaches the tape's document only in the firmware's save of the tape (the place T1), which
// runs when a value of the tape's asks for it; the elder never asked, so a value set since the last save was not in the
// file a tape switch or a copy writes. Here a tape value set (and changed) asks for the save as the firmware's own tape
// values do when they change (0x8c0c1dd0, then 0x8fca1ed2), from the display thread, outside every mask.
// A load with the flag set says so in a line of its own (line 14), with the number of values its file held. The words
// of two lines name the rooms this base has (4 KiB a slot or the tape, 48 KiB the store), where the elder's named 8
// and 128.
//
// A pack lists, in an ask of this kind, up to eight members it keeps, each with the synth's sounds, the drum's or the
// tape. Each is a member of its own at the top of the file, under its name: in a preset of a sound when the owner
// saves it, and in the tape's file when the unit writes the tape. The pack reads and sets its values as JSON text
// from its frame function (op1f_value_get, op1f_value_set), and is told at its next frame when a sound was loaded
// onto a slot, when a slot's engine changed, when the values were written, and at the start.
//
// Eight places of the firmware, each a call or an entry under the base's table, go to the stubs of
// module/build_r4_keeps.s and from them to the functions below:
//   the synth's preset read, before the firmware keeps only the members it knows    op1f_keeps_read_preset
//   the synth sampler's preset read                                                 op1f_keeps_read_sampler
//   the drum's preset read                                                          op1f_keeps_read_drum
//   a sound put on a slot's record (a preset loaded, or the engine changed)        op1f_keeps_assign
//   the synth's preset and the drum's, about to be written                         op1f_keeps_saving
//   the tape about to be written, after the firmware's last member                 op1f_keeps_tape_saving
//   the tape read, after the firmware's own load                                   op1f_keeps_tape_loaded
// None of them runs on the audio thread. The reads, the assign and the tape's save run on the firmware's background
// worker, the preset's save and the tape's first load on its main task, a tape picked from the list on the thread of
// the key: each only takes values off the document in hand or puts values into it, and no text is parsed there.
//
// What a file is never left with. The base never sets, changes or removes a member the firmware's own document holds:
// before it puts a member into a document it asks whether the document has one of that name, and a name the
// document holds that is not one the base took from that file itself is refused, with a line. Names the unit's own
// files use are refused at the start (the list NAMES_OF_THE_UNIT below). A value is put into a document only whole:
// it is made beside the document first, from the base's own checked form, and only then joined. A member whose value
// the base cannot keep whole (a number with a fraction, or one beyond the 32 bits of a whole number, or a text longer
// than 2,048 bytes) is not taken from a file, so a file that goes through the base with no value set comes out as the
// firmware alone writes it, and with values set it differs only by the members added.
//
// The checked form. A value is kept between the threads as a row of tokens (a byte for null, false, true, the start
// and end of a list or of an object; a byte and four for a whole number; a byte, two and the bytes and a zero for a
// text or a member's name), made either from a document's value, by its kind, walked with nothing allocated, or from
// the pack's text, parsed on the display thread. A value is nested at most four deep. The tokens of a slot or of the
// tape take at most 4 KiB.
//
// The threads. What the firmware's threads and the display thread share is held in single words and in holders
// that are read and written whole inside short masked sections: a thread copies a holder out, or a whole new one in,
// with interrupts masked, and does its work on its own copy. A copy is at most 4 KiB, a few microseconds.
//
// The slots' values across a start. The firmware drops every member it does not know from its slots at every start,
// so the base keeps the slots' values in a file of its own, user/pf/.sounds, a JSON document written by the base's
// own writer, half a second after the last change, never while the disk mode is open. It holds the names the base
// knew (so that a pack that is away keeps its members of the sounds loaded meanwhile) and, for each slot with
// values, the mark of its sound: the record's `name` and `type`. At the first frame each slot's record is read
// again; a slot whose mark differs lets its values go.
//
// Compiled for the PC as it is, with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for
// tests this tree does not hold). Nothing is divided by a number that is not a power of two (the module links no
// runtime).

#include "build_r6_base.h"                             // R6: base 28's shared names, and level 9's

// R6b: the four new places (module/build_r6b_keeps.s), OS 1.7.0+0, 1.7.3 by the map:
//   L1  0x8d4ecb42 (0x8d4ecb42)  the synth's list job, its CALL.a of the plain reader      -> op1f_keeps_read_list_synth
//   L2  0x8d4ecc2c (0x8d4ecc2c)  the synth's list job, its call of the synth's put with the move of its third
//                                argument before it and of the job's document after it -> op1f_keeps_put_synth
//   L3  0x8d4ec1f4 (0x8d4ec1f4)  the drum's list job, its CALL.a of the plain reader       -> op1f_keeps_read_list_drum
//   L4  0x8d4ec2c6 (0x8d4ec2c6)  the drum's list job, its call of the drum's put with a store of the job's own to its
//                                stack before it -> op1f_keeps_put_drum

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

struct Json { u32 words[4]; };                       // a value of the firmware's JSON library is 12 bytes; 16 here

typedef int (*JsonHasFn)(const void* value, const char* key);
typedef const void* (*JsonMemberFn)(const void* value, const char* key);
typedef void* (*JsonPlaceFn)(void* value, const char* key);
typedef u32 (*JsonAssignFn)(void* to, const void* from);
typedef void (*JsonMakeFn)(void* value, int kind);
typedef void (*JsonNumberFn)(void* value, int number);
typedef void (*JsonTextFn)(void* value, const char* text);
typedef void* (*JsonAppendFn)(void* list, const void* value);
typedef void (*JsonDestroyFn)(void* value);
typedef u32 (*JsonSizeFn)(const void* value);
typedef const void* (*JsonElementFn)(const void* value, u32 index);
typedef const char* (*JsonCStringFn)(const void* value);
typedef int (*JsonReadFileFn)(const char* path, void* value);
typedef int (*JsonReadRawFn)(void* a, void* text, void* value, int flag);
typedef void* (*TapeRootFn)();
typedef int (*TapeSetFn)(int db, const char* key, const void* value);
typedef int (*FileSizeFn)(const char* path);
typedef void* (*SaverFn)();
typedef void (*NotifyFn)(void* saver);
typedef void (*PutFn)(void* record, int slot, void* json, int flag);   // R6b: a slot's put (its answer is not read)

#ifdef OP1F_HOST
#include "mock_keeps_r6b.h"                            // the PC's stand-ins: base 28's (for tests this tree does not hold) and the two puts (R6b)
#define MASK_OFF(mask) ((void)((mask) = mock_mask()))
#define MASK_ON(mask) mock_unmask(mask)
#define BARRIER() ((void)0)
#define OUT_OF_LINE
static inline u32 cycles_now() { return mock_cycles(); }
#else
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)          // whether an object (or null) has a member of that name
#define STOCK_JSON_MEMBER ((JsonMemberFn)0x8c084f74u)    // an object's member, the library's shared null when none
#define STOCK_JSON_PLACE ((JsonPlaceFn)0x8c085e84u)      // an object's member, made null when there is none
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)    // a copy of a value assigned
#define STOCK_JSON_MAKE ((JsonMakeFn)0x8c085ba8u)        // a value of a kind: 0 null, 6 a list, 7 an object
#define STOCK_JSON_NUMBER ((JsonNumberFn)0x8c083b1cu)    // a whole number
#define STOCK_JSON_TRUTH ((JsonNumberFn)0x8c083aecu)     // true or false
#define STOCK_JSON_TEXT ((JsonTextFn)0x8c084430u)        // a text, from a C text
#define STOCK_JSON_APPEND ((JsonAppendFn)0x8c085d54u)    // a copy of a value put at a list's end
#define STOCK_JSON_DESTROY ((JsonDestroyFn)0x8c0844e0u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_ELEMENT ((JsonElementFn)0x8c084ee0u)  // a list's element, the shared null when none
#define STOCK_JSON_CSTRING ((JsonCStringFn)0x8c084748u)  // a text's characters: only for a value of the kind of a text
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)   // a file read into a value: not 0 when it was read
#define STOCK_JSON_READ_RAW ((JsonReadRawFn)0x8c0828e0u)     // the read under it, as the synth's preset read calls it
#define STOCK_TAPE_ROOT ((TapeRootFn)0x8d4f2b80u)        // the tape's document
#define STOCK_TAPE_SET ((TapeSetFn)0x8d4f24dcu)          // a member of the tape's document set, under its own lock
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)        // a file's bytes, -1 when there is none
#define STOCK_TAPE_SAVER ((SaverFn)0x8c0c1dd0u)          // the tape's saver, whose save (0x8c0c116c) sets the tape's members
#define STOCK_NOTIFY ((NotifyFn)0x8fca1ed2u)             // a saver marked to save, the background worker woken
#define RECORDS_OF(host) ((host) ? 0x0802377cu : 0x080235dcu)   // the slots' records, eight of 52 bytes, the document at +8
// R6b: a slot's put, (record, slot, document, flag): the slot word set, the document assigned to the record, and with
// the flag 0 the firmware's pass that keeps only the members it knows, then the record's name for the screen. Called by
// the list jobs (L2, L4), the unit's save, the start's load of the slots files.
#define STOCK_SYNTH_PUT ((PutFn)0x8d4f6c3cu)
#define STOCK_DRUM_PUT ((PutFn)0x8d4f6748u)
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
#define OUT_OF_LINE __attribute__((noinline))
static inline u32 cycles_now() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}
// R5: the effects unit's word of what changed for the base's own member; weak, so that a base without that unit links
// and tells nothing (module/build_r5_base.h).
extern "C" void op1f_effects_changed(unsigned where, unsigned which, unsigned how) __attribute__((weak));
#define EFFECTS_CHANGED op1f_effects_changed
#endif


// R6: the engines unit's two names (module/build_r6_base.h): what changed of the base's second member, and the sound
// handed at the assign. Weak: away, nothing is told.
#if defined(OP1F_HOST)
#ifdef OP1F_R6_HOST_ENGINES
extern "C" void (*mock_engines_changed)(unsigned where, unsigned which, unsigned how);
#define op1f_engines_changed (*mock_engines_changed)
extern "C" void (*mock_engines_assign)(unsigned slot, unsigned how, void* json, const char* name);
#define op1f_engines_assign (*mock_engines_assign)
#else
static void (*const r6_none_engines_changed)(unsigned where, unsigned which, unsigned how) = 0;
#define op1f_engines_changed (*r6_none_engines_changed)
static void (*const r6_none_engines_assign)(unsigned slot, unsigned how, void* json, const char* name) = 0;
#define op1f_engines_assign (*r6_none_engines_assign)
#endif
#else
extern "C" void op1f_engines_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_engines_changed") __attribute__((weak));
extern "C" void op1f_engines_assign(unsigned slot, unsigned how, void* json, const char* name) OP1F_NAMED("op1f_engines_assign") __attribute__((weak));
#endif

// R11: the LFO unit's word of what changed of the base's third member, op1f_lfo; weak: away, nothing is told.
#if defined(OP1F_HOST)
#ifdef OP1F_R11_HOST_LFOS
extern "C" void (*mock_lfos_changed)(unsigned where, unsigned which, unsigned how);
#define op1f_lfos_changed (*mock_lfos_changed)
#else
static void (*const r11_none_lfos_changed)(unsigned where, unsigned which, unsigned how) = 0;
#define op1f_lfos_changed (*r11_none_lfos_changed)
#endif
#else
extern "C" void op1f_lfos_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_lfos_changed") __attribute__((weak));
#endif

// R21 (base 43): the drum unit's (module/build_r21_drum.cpp), told what changed of the member op1f_engine at a drum
// slot, as the engines unit is at a synth slot. Weak: away, nothing is told.
#if defined(OP1F_HOST)
static void (*const r18_none_drum_changed)(unsigned where, unsigned which, unsigned how) = 0;
#define op1f_drum_changed (*r18_none_drum_changed)
#else
extern "C" void op1f_drum_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_drum_changed") __attribute__((weak));
#endif

// The return addresses into the five jobs that put a sound on a slot's record (each calls it with a call of four
// bytes): three loads from a file, two changes of the engine.
static const u32 CALLER_SYNTH_PRESET = 0x8d4ea95au, CALLER_DRUM = 0x8d4ebf9au, CALLER_SYNTH_SAMPLER = 0x8d4ec968u;
static const u32 CALLER_SYNTH_TYPE = 0x8d4ec6f0u, CALLER_DRUM_TYPE = 0x8d4eba60u;

// ---- from the cards unit and the folders unit ----

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
extern "C" int op1f_cards_verdict(unsigned slot) OP1F_NAMED("op1f_cards_verdict");
// The card's slot + 1 while the base calls that card's frame function on the caller's stack, else 0.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
enum { CALL_SHOWN = 6 };                              // the record of a call such as an ask's `shown` (Op1fCallKept: module/build_r3_base.h)
extern "C" void op1f_cards_call_in(unsigned kind, unsigned slot, Op1fCallKept* kept) OP1F_NAMED("op1f_cards_call_in");
extern "C" void op1f_cards_call_out(unsigned kind, const Op1fCallKept* kept) OP1F_NAMED("op1f_cards_call_out");
extern "C" int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) OP1F_NAMED("op1f_folders_write_base");

static const char LOG_FILE[] = "../src/main/op1f_keeps.cpp";
static const char STORE_PATH[] = "user/pf/.sounds";

enum {
    KIND_NULL = 0, KIND_INT = 1, KIND_UINT = 2, KIND_REAL = 3, KIND_STRING = 4, KIND_BOOL = 5, KIND_LIST = 6, KIND_OBJECT = 7,
    KIND_AT = 4,                                       // a value's kind byte
    RECORD_BYTES = 52, RECORD_DOCUMENT = 8, SLOTS = 8, HOSTS = 2, TAPE = HOSTS * SLOTS, MASTER = TAPE + 1, HOLDERS = TAPE + 2,
    HOLDER_BYTES = 4096, VALUE_MOST = OP1F_VALUE_MOST, DEPTH_MOST = 4, NAMES_MOST = 64, NAME = OP1F_KEY_BYTES,
    // R5: the base's own name first in the list, beside the sixty-four the packs may keep; its owner, which no card's
    // slot + 1 is; the places a key of a card of level 8 may name; a holder's bits of the names, one for each
    NAMES_ROOM = NAMES_MOST + 3, BASE_NAME = 0, BASE_ENGINE = 1, BASE_LFO = 2, OWNER_BASE = OP1F_SLOTS_MOST + 1,   // R6: two names of the base's own
    KEEP_PLACES = OP1F_KEEP_SYNTH | OP1F_KEEP_DRUM | OP1F_KEEP_TAPE,
    KEEP_ALL = KEEP_PLACES | OP1F_KEEP_MASTER, BITS = (NAMES_ROOM + 31) >> 5,
    KEYS = OP1F_KEYS_MOST, TABLE_BYTES = 324, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, CARDS = OP1F_SLOTS_MOST,
    MARK = 48, STORE_MOST = 48 << 10, STORE_ROOM = STORE_MOST + 16, KIND_NAME = OP1F_KIND_NAME, TAPE_DB = 2,
    WRITTEN_SAID = 16, QUEUE = 16, KEEPS_MOST = OP1F_SLOTS_MOST,
    // the walk of an object's members, as the library lays them out: the object's word -> its tree; the tree's head
    // at +4; a node's left +0, parent +4, right +8, its name's characters at +0xc, its value at +0x14, its "is the
    // head" byte at +0x21
    TREE_HEAD = 4, NODE_LEFT = 0, NODE_PARENT = 4, NODE_RIGHT = 8, NODE_NAME = 0xc, NODE_VALUE = 0x14, NODE_IS_HEAD = 0x21
};
static const u32 HALF_SECOND = 200000000u;            // the processor's cycles, at 400,000,000 a second
static_assert(sizeof(Op1fKeeps) == TABLE_BYTES && sizeof(Op1fKeptKey) == 36, "the table of the values kept is 324 bytes at level 7");

// The tokens of the checked form.
// A whole number takes two bytes from -128 to 127, three from -32768 to 32767, else five: a value's tokens are never
// much more than its text.
enum { T_NULL = 1, T_FALSE = 2, T_TRUE = 3, T_INT = 4, T_TEXT = 5, T_LIST = 6, T_LIST_END = 7, T_OBJECT = 8, T_OBJECT_END = 9, T_KEY = 10,
       T_INT8 = 11, T_INT16 = 12 };

// Names the unit's own preset, slots and tape files hold at their top: no pack may keep a member of one of them. Each
// name stands in a field of 24 bytes (the longest is 19, so at least five zero bytes follow each), so that no run of
// names lies in the base as the firmware lays its own out; the empty name ends the list.
static const char NAMES_OF_THE_UNIT[][24] = {
    "adsr", "attack", "base_freq", "clips", "dbox_data", "drum_version", "dyna_env", "end", "fade", "fademode", "fx_active",
    "fx_params", "fx_type", "knobs", "lfo_active", "lfo_params", "lfo_type", "loop", "loop_in", "loop_out", "mixer_params",
    "mtime", "name", "octave", "original_folder", "pan", "pan_ab", "pitch", "playmode", "pos", "reverse", "start", "stereo",
    "synth_version", "tape_type", "tempobpmdecimal", "tempometronomelevel", "tempometronometype", "temposyncout",
    "temposynctype", "track", "type", "version", "volume", ""
};

// R5: the base's own member, kept as a pack's values are: which pack's effect a place rides, and its switch.
static const char BASE_KEY[] = "op1f_effect";
static const char ENGINE_KEY[] = "op1f_engine";        // R6: the base's second own member
static const char LFO_KEY[] = "op1f_lfo";              // R11: the third, the layer an LFO of a sound rides ({"name": "<layer>"})

// ---------------------------------------------------------------- what is kept

// One member of the base's list of names: the base's own (the first), a pack's key of this start, or a name remembered
// from the store.
struct Name {
    char name[NAME];
    u32 files;                                          // OP1F_KEEP_ bits
    u32 owner;                                          // the card's slot + 1; 0 for a name remembered for a pack that is away; OWNER_BASE for the base's own
};

// The values of a slot, of the tape or of the master. `bytes` of `data`: entries of [the name's number][two bytes of
// length][the tokens]. `gen` grows at every change, so that a copy taken out and put back finds whether someone else
// wrote it meanwhile. `carried` is the names the file in hand held when the values were taken from it (they may be
// written over at the save, being the base's own there).
struct Holder {
    u32 gen, bytes;
    u32 carried[BITS];
    u32 marked;                                         // the mark below was taken
    char mark_name[MARK], mark_type[MARK];
    u8 data[HOLDER_BYTES];
};
static_assert(sizeof(Holder::carried) * 8 >= NAMES_ROOM, "a bit for every name of the list");

// The mark of a sound alone, as a holder keeps it (the first frame's, of a record read again).
struct Mark {
    u32 marked;
    char mark_name[MARK], mark_type[MARK];
};

static Name names[NAMES_ROOM];
static u32 names_count;
static volatile u32 ready;                              // the list of names is whole: the threads may take values
static Holder holders[HOLDERS];                         // the synth's slots, the drum's, the tape, the master (R5)
static volatile u32 told[HOLDERS];                      // 1 << OP1F_KEPT_..., set by any thread inside a mask, taken by the display's
enum { TOLD_STORE = 0 };                                // and this bit: a slot's values changed (or the master's), the store is to be written

// Each thread's own copy. The background worker: the values of the preset in hand (its kind, its values), and its
// copy for the tape's save; the main task: its copy for the preset's save; the tape's load: its own; the display: its
// own.
enum { FROM_NONE = 0, FROM_SYNTH_PRESET = 1, FROM_DRUM = 2, FROM_SYNTH_SAMPLER = 3,
       FROM_LIST_SYNTH = 4, FROM_LIST_DRUM = 5 };      // R6b: the unit's list of sounds, the synth's and the drum's (M3)
static u32 waiting_kind;
static Holder waiting;
static Holder save_copy, tape_save_copy, tape_load_copy, display_copy;
static u32 tape_written[BITS];                          // the names the base itself wrote into the tape since its load
static volatile u32 tape_early;                         // a tape was loaded before the list of names was whole
static volatile u32 tape_early_flag;                    // the flag of that load
static volatile u32 tape_last;                          // the tape's last load told: OP1F_KEPT_LOADED or OP1F_KEPT_MADE
static volatile u32 tape_seen;                          // a tape was loaded since the start: its saver may be asked to save
// The tape's loads, counted. Moved by the thread that loads a tape (the key's thread for a tape picked, made or copied
// in the tape list; the main task for the start's load; the display thread for a load taken at the first frame), only
// inside put_in's mask, in the same mask as the load's values go in. Read by the background worker in the tape's save:
// inside one mask with its copy of the tape's values, then as a single word before and after each value it sets.
static volatile u32 tape_loads;
// The two bits of told[] that a tape load sets: while either is in told[TAPE], a load waits to be told to the packs.
static const u32 LOADS_UNTOLD = 1u << OP1F_KEPT_LOADED | 1u << OP1F_KEPT_MADE;
static char store_text[STORE_ROOM];

// R17 (base 39): the kept cut of a drop into a synth slot's sample (module/build_r17_restore.cpp puts it back at the
// next start), the slot's member beside its ride: the frames (0: none) and the channels (1 stereo). In the store it is
// the slot's row's member "sample", [frames, stereo], written only while the slot holds the ride (op1f_engine), so a
// store with no kept cut is base 38's byte for byte. Base 38's reader takes a row's host, name, slot, type and values by
// name and nothing else: it reads such a store whole, the ride with it, passes the member by and leaves it out of the
// next store it writes. The display thread's alone: set by op1f_keeps_sample_set, read at the first frame and by the
// store's writer. The pool's halfwords, as module/build_r17_sample.h's OP1F_SAMPLE_POOL.
static const char SAMPLE_KEY[] = "sample";
enum { SAMPLE_POOL = 529200 };
static u32 sample_frames[SLOTS], sample_stereo[SLOTS];

template <typename Marked> static void mark_copy(Marked* to, const Mark* from);

// R6b (M4): the mark a save gave a slot's sound, waiting until the slot's record shows it. Written by the save's
// thread (the main task) and read by the display thread, each inside a mask; `due` is the save's count (0: none waits),
// so that the display thread takes the mark only of the save it read. A load or a change of engine on the slot ends the
// wait inside put_in's mask; past REMARK_WAIT it ends by itself (the save's file was not written: no put came).
struct Remark { u32 due, since; Mark mark; };
static Remark remarks[TAPE];
static u32 remark_count;                                // the saves' count, on the main task (never 0 in `due`)
static Remark remark_copy;                              // the display thread's copy
static volatile u32 remarks_any;                        // a save's mark may wait: set by the save, cleared by the frame
static Mark save_mark;                                  // the main task's: the mark of the document a save writes
static const u32 REMARK_WAIT = 800000000u;              // two seconds of the processor's cycles

// The asks of this kind in use: for `changed`.
struct Keeps { u32 slot; const Op1fKeeps* table; u32 files; };
static Keeps keeps[KEEPS_MOST];
static u32 keeps_count;

// What the firmware's threads found, said at the next frame. Q_TAPE_LOADED: a tape loaded with the firmware's flag set
// (a tape made anew or a copy), its `name` the values taken, its `bytes` the flag. Q_CUT: the tape's save cut by a
// tape load, its `bytes` the values of the tape before left out. Q_STRAY: a value set while a tape load came, its `why`
// STRAY_PUT_BACK (the next tape's own value set again over it) or STRAY_STAYS (the next tape holds none of that name).
enum { Q_WRITTEN = 1, Q_REFUSED = 2, Q_NOT_TAKEN = 3, Q_TAPE_LOADED = 4, Q_CUT = 5, Q_STRAY = 6 };
enum { STRAY_PUT_BACK = 1, STRAY_STAYS = 2 };
enum { WHY_FIRMWARE = 1, WHY_MADE = 2, WHY_ROOM = 3, WHY_KIND = 4, WHY_LONG = 5 };
struct Said { u32 what, name, holder, bytes, why; };
static Said queue[QUEUE];
static volatile u32 queue_in, queue_out, queue_lost;

static struct {
    u32 started;                                        // the first frame was made
    u32 store_due, store_since;                         // the store changed, at that cycle
    u32 store_known, store_sum;                         // what is on the disk is the text whose sum the base wrote last
    u32 written_said, kept_said, loads_said, cuts_said;
    u32 early_loads, early_saves, bad_slots;
} unit;

// ---------------------------------------------------------------- small helpers

static int same(const char* a, const char* b) {
    for (u32 i = 0;; ++i) {
        if (a[i] != b[i]) return 0;
        if (a[i] == 0) return 1;
    }
}

static u32 length_of(const char* text, u32 most) {
    u32 n = 0;
    while (n <= most && text[n]) ++n;
    return n;
}

static void copy_text(char* to, const char* from, u32 room) {
    u32 n = 0;
    if (from)
        for (; n + 1 < room && from[n]; ++n) to[n] = from[n];
    for (; n < room; ++n) to[n] = 0;
}

static u32 kind_of(const void* value) {
    return ((const volatile u8*)value)[KIND_AT];
}

static void copy_bytes(u8* to, const u8* from, u32 n) {
    for (u32 i = 0; i < n; ++i) to[i] = from[i];
}

// A holder copied whole, its data only as far as it is used.
static void holder_copy(Holder* to, const Holder* from) {
    to->gen = from->gen;
    to->bytes = from->bytes > (u32)HOLDER_BYTES ? 0u : from->bytes;
    for (u32 i = 0; i < (u32)BITS; ++i) to->carried[i] = from->carried[i];
    to->marked = from->marked;
    for (u32 i = 0; i < (u32)MARK; ++i) {
        to->mark_name[i] = from->mark_name[i];
        to->mark_type[i] = from->mark_type[i];
    }
    copy_bytes(to->data, from->data, to->bytes);
}

static void holder_clear(Holder* h) {
    h->bytes = 0;
    for (u32 i = 0; i < (u32)BITS; ++i) h->carried[i] = 0;
    h->marked = 0;
    h->mark_name[0] = h->mark_type[0] = 0;
}

// Out of a holder into a copy of the thread's own, inside one mask.
static void take_out(u32 index, Holder* copy) {
    u32 mask;
    MASK_OFF(mask);
    holder_copy(copy, &holders[index]);
    MASK_ON(mask);
}

// The same, with the count of the tape's loads read in the same mask (the tape's save, on the background worker).
static u32 take_out_counted(u32 index, Holder* copy) {
    u32 mask;
    MASK_OFF(mask);
    holder_copy(copy, &holders[index]);
    u32 loads = tape_loads;
    MASK_ON(mask);
    return loads;
}

// A whole new holder put in, inside one mask, and the change told. `when` 0: whatever was there; else only when the
// holder is still at the generation the copy was taken at (answers 0 when it was not) and, for the tape, only while no
// tape load waits to be told to the packs (answers -1 when one waits: the set of a pack in the frame of a load). A load
// (`how` OP1F_KEPT_LOADED or OP1F_KEPT_MADE of the tape) moves the count of the tape's loads in the same mask.
// R5: `when` SET_BASE, a set of the base's own member: the same, and refused (-1) while a load of any holder waits to
// be told (a sound loaded onto the slot after this frame's op1f_keeps_frame): the effects unit has not read it yet.
enum { SET_BASE = 2 };
static int put_in(u32 index, const Holder* copy, u32 how, int when) {
    u32 mask;
    MASK_OFF(mask);
    Holder* h = &holders[index];
    if (when && index == (u32)TAPE && (told[index] & LOADS_UNTOLD) != 0) {
        MASK_ON(mask);
        return -1;
    }
    if (when == SET_BASE && (told[index] & LOADS_UNTOLD) != 0) {
        MASK_ON(mask);
        return -1;
    }
    if (when && h->gen != copy->gen) {
        MASK_ON(mask);
        return 0;
    }
    u32 gen = h->gen + 1u;
    u32 store = h->bytes != 0 || copy->bytes != 0 ? 1u << TOLD_STORE : 0u;           // the store is to be written only when values were or are there
    holder_copy(h, copy);
    h->gen = gen;
    if (how) told[index] |= 1u << how | store;
    if (index == (u32)TAPE && (how == (u32)OP1F_KEPT_LOADED || how == (u32)OP1F_KEPT_MADE)) tape_last = how;
    if (index == (u32)TAPE && (1u << how & LOADS_UNTOLD) != 0) tape_loads = tape_loads + 1u;
    if (index < (u32)TAPE && (how == (u32)OP1F_KEPT_LOADED || how == (u32)OP1F_KEPT_ENGINE)) remarks[index].due = 0;   // R6b: its own mark is the newer
    MASK_ON(mask);
    return 1;
}

static void tell(u32 index, u32 how) {
    u32 mask;
    MASK_OFF(mask);
    told[index] |= 1u << how;
    MASK_ON(mask);
}

// A line for the display thread to say, from any thread.
static void say_later(u32 what, u32 name, u32 holder, u32 bytes, u32 why) {
    u32 mask;
    MASK_OFF(mask);
    u32 in = queue_in;
    if (in - queue_out >= (u32)QUEUE) {
        ++queue_lost;
    } else {
        Said* s = &queue[in & (u32)(QUEUE - 1)];
        s->what = what;
        s->name = name;
        s->holder = holder;
        s->bytes = bytes;
        s->why = why;
        queue_in = in + 1u;
    }
    MASK_ON(mask);
}

static u32 bit_of(const u32* bits, u32 n) {
    return (bits[n >> 5] >> (n & 31u)) & 1u;
}

static void set_bit(u32* bits, u32 n) {
    bits[n >> 5] |= 1u << (n & 31u);
}

static u32 files_of_holder(u32 index) {
    return index == (u32)MASTER ? (u32)OP1F_KEEP_MASTER
         : index == (u32)TAPE ? (u32)OP1F_KEEP_TAPE : index >= (u32)SLOTS ? (u32)OP1F_KEEP_DRUM : (u32)OP1F_KEEP_SYNTH;
}

// R5: the `which` a holder is told and named by: the slot, or 0 for the tape and the master.
static u32 which_of_holder(u32 index) {
    return index >= (u32)TAPE ? 0u : index & (u32)(SLOTS - 1);
}

// ---------------------------------------------------------------- the tokens: written, read

struct Out { u8* at; u32 used, room; int full; };

static void put_byte(Out* o, u32 b) {
    if (o->used >= o->room) { o->full = 1; return; }
    o->at[o->used++] = (u8)b;
}

static void put_word(Out* o, u32 w) {
    for (u32 i = 0; i < 4u; ++i) put_byte(o, (w >> (8u * i)) & 0xffu);
}

// A text or a key: its token, two bytes of length, the characters and a zero. 0 when it cannot be (too long).
static int put_text(Out* o, u32 token, const char* text, u32 n) {
    if (n > 0xffffu) return 0;
    put_byte(o, token);
    put_byte(o, n & 0xffu);
    put_byte(o, n >> 8);
    for (u32 i = 0; i < n; ++i) put_byte(o, (u8)text[i]);
    put_byte(o, 0);
    return 1;
}

static void put_whole(Out* o, i32 v) {
    if (v >= -128 && v <= 127) { put_byte(o, T_INT8); put_byte(o, (u32)v & 0xffu); }
    else if (v >= -32768 && v <= 32767) { put_byte(o, T_INT16); put_byte(o, (u32)v & 0xffu); put_byte(o, ((u32)v >> 8) & 0xffu); }
    else { put_byte(o, T_INT); put_word(o, (u32)v); }
}

static u32 get_word(const u8* at) {
    return (u32)at[0] | (u32)at[1] << 8 | (u32)at[2] << 16 | (u32)at[3] << 24;
}

static u32 get_half(const u8* at) {
    return (u32)at[0] | (u32)at[1] << 8;
}

// The whole number of a token of T_INT, T_INT8 or T_INT16.
static i32 whole_at(const u8* at) {
    if (at[0] == (u32)T_INT8) return (i32)(signed char)at[1];
    if (at[0] == (u32)T_INT16) return (i32)(short)(u16)get_half(at + 1);
    return (i32)get_word(at + 1);
}

// How many bytes the one value at `at` takes (its tokens), within `n`; 0 when it is not whole.
static u32 value_span(const u8* at, u32 n) {
    u32 i = 0, depth = 0;
    do {
        if (i >= n) return 0;
        u32 t = at[i];
        if (t == (u32)T_NULL || t == (u32)T_FALSE || t == (u32)T_TRUE) {
            i += 1;
        } else if (t == (u32)T_INT) {
            i += 5;
        } else if (t == (u32)T_INT8) {
            i += 2;
        } else if (t == (u32)T_INT16) {
            i += 3;
        } else if (t == (u32)T_TEXT || t == (u32)T_KEY) {
            if (i + 3 > n) return 0;
            if (t == (u32)T_KEY && depth == 0) return 0;
            i += 4 + get_half(at + i + 1);
            if (t == (u32)T_KEY) continue;                                          // a key's value follows
        } else if (t == (u32)T_LIST || t == (u32)T_OBJECT) {
            ++depth;
            i += 1;
            continue;
        } else if (t == (u32)T_LIST_END || t == (u32)T_OBJECT_END) {
            if (depth == 0) return 0;
            --depth;
            i += 1;
        } else {
            return 0;
        }
    } while (depth != 0);
    return i <= n ? i : 0;
}

// ---------------------------------------------------------------- a document's value into tokens (any thread)

// 0 when the value was put whole; WHY_KIND for a kind the base cannot keep; WHY_ROOM when the room is short.
static u32 from_value(const void* value, Out* o, u32 depth) {
    u32 kind = kind_of(value);
    if (kind == (u32)KIND_NULL) { put_byte(o, T_NULL); }
    else if (kind == (u32)KIND_BOOL) { put_byte(o, *(const volatile u8*)value ? T_TRUE : T_FALSE); }
    else if (kind == (u32)KIND_INT) { put_whole(o, *(const volatile i32*)value); }
    else if (kind == (u32)KIND_UINT) {                                              // a whole number written without a sign
        u32 n = *(const volatile u32*)value;
        if (n > 0x7fffffffu) return WHY_KIND;
        put_whole(o, (i32)n);
    } else if (kind == (u32)KIND_STRING) {
        const char* text = STOCK_JSON_CSTRING(value);                               // the kind was tested first: the call stops the unit on any other
        u32 n = text ? length_of(text, 0xffffu) : 0;
        if (n > 0xffffu || !put_text(o, T_TEXT, text ? text : "", n)) return WHY_ROOM;
    } else if (kind == (u32)KIND_LIST) {
        if (depth >= (u32)DEPTH_MOST) return WHY_KIND;
        put_byte(o, T_LIST);
        u32 count = STOCK_JSON_SIZE(value);
        for (u32 i = 0; i < count && !o->full; ++i) {
            u32 why = from_value(STOCK_JSON_ELEMENT(value, i), o, depth + 1u);
            if (why) return why;
        }
        put_byte(o, T_LIST_END);
    } else if (kind == (u32)KIND_OBJECT) {
        if (depth >= (u32)DEPTH_MOST) return WHY_KIND;
        put_byte(o, T_OBJECT);
        // The members walked in the order of their names, as the library keeps them, with nothing allocated.
        u32 tree = *(const volatile u32*)value;
        u32 head = tree ? *(const volatile u32*)(tree + (u32)TREE_HEAD) : 0;
        if (head) {
            u32 node = *(const volatile u32*)(head + (u32)NODE_LEFT);
            u32 steps = 0;
            while (node != head && !o->full) {
                if (++steps > 0x10000u) return WHY_KIND;                            // a tree that does not end: never made by the library
                const char* key = *(const char* const volatile*)(node + (u32)NODE_NAME);
                if (key == 0 || !put_text(o, T_KEY, key, length_of(key, 0xffffu))) return WHY_ROOM;
                u32 why = from_value((const void*)(node + (u32)NODE_VALUE), o, depth + 1u);
                if (why) return why;
                u32 right = *(const volatile u32*)(node + (u32)NODE_RIGHT);
                if (*(const volatile u8*)(right + (u32)NODE_IS_HEAD) == 0) {      // down the right, then all the way left
                    node = right;
                    for (;;) {
                        u32 left = *(const volatile u32*)(node + (u32)NODE_LEFT);
                        if (*(const volatile u8*)(left + (u32)NODE_IS_HEAD) != 0) break;
                        node = left;
                    }
                } else {                                                            // up while the node is its parent's right
                    u32 parent = *(const volatile u32*)(node + (u32)NODE_PARENT);
                    while (*(const volatile u8*)(parent + (u32)NODE_IS_HEAD) == 0 && *(const volatile u32*)(parent + (u32)NODE_RIGHT) == node) {
                        node = parent;
                        parent = *(const volatile u32*)(node + (u32)NODE_PARENT);
                    }
                    node = parent;
                }
            }
        }
        put_byte(o, T_OBJECT_END);
    } else {
        return WHY_KIND;                                                            // a number with a fraction, or a kind the library does not have
    }
    return o->full ? (u32)WHY_ROOM : 0u;
}

// ---------------------------------------------------------------- tokens into a document's value (any thread)

// The value at `at` made into `out` (a value not yet made). Answers the tokens' bytes, 0 when it could not be made
// (nothing is left made then).
static u32 to_value(const u8* at, u32 n, void* out, u32 depth) {
    if (n == 0) return 0;
    u32 t = at[0];
    if (t == (u32)T_NULL) { STOCK_JSON_MAKE(out, KIND_NULL); return 1; }
    if (t == (u32)T_FALSE || t == (u32)T_TRUE) { STOCK_JSON_TRUTH(out, t == (u32)T_TRUE ? 1 : 0); return 1; }
    if (t == (u32)T_INT || t == (u32)T_INT8 || t == (u32)T_INT16) {
        u32 used = t == (u32)T_INT ? 5u : t == (u32)T_INT16 ? 3u : 2u;
        if (n < used) return 0;
        STOCK_JSON_NUMBER(out, whole_at(at));
        return used;
    }
    if (t == (u32)T_TEXT) {
        if (n < 4) return 0;
        u32 len = get_half(at + 1);
        if (4 + len > n || at[3 + len] != 0) return 0;
        STOCK_JSON_TEXT(out, (const char*)(at + 3));
        return 4 + len;
    }
    if ((t != (u32)T_LIST && t != (u32)T_OBJECT) || depth >= (u32)DEPTH_MOST) return 0;
    STOCK_JSON_MAKE(out, t == (u32)T_LIST ? KIND_LIST : KIND_OBJECT);
    u32 i = 1;
    for (;;) {
        if (i >= n) { STOCK_JSON_DESTROY(out); return 0; }
        u32 u = at[i];
        if (t == (u32)T_LIST && u == (u32)T_LIST_END) return i + 1;
        if (t == (u32)T_OBJECT && u == (u32)T_OBJECT_END) return i + 1;
        const char* key = 0;
        if (t == (u32)T_OBJECT) {
            if (u != (u32)T_KEY || i + 4 > n) { STOCK_JSON_DESTROY(out); return 0; }
            u32 len = get_half(at + i + 1);
            if (i + 4 + len > n || at[i + 3 + len] != 0) { STOCK_JSON_DESTROY(out); return 0; }
            key = (const char*)(at + i + 3);
            i += 4 + len;
        }
        Json item;
        u32 used = to_value(at + i, n - i, &item, depth + 1u);
        if (used == 0) { STOCK_JSON_DESTROY(out); return 0; }
        if (key) {
            void* place = STOCK_JSON_PLACE(out, key);
            if (place) STOCK_JSON_ASSIGN(place, &item);
        } else {
            STOCK_JSON_APPEND(out, &item);
        }
        STOCK_JSON_DESTROY(&item);
        i += used;
    }
}

// ---------------------------------------------------------------- tokens into text, text into tokens (the display thread)

struct Text { char* at; u32 used, room; int full; };

static void put_char(Text* t, char c) {
    if (t->used + 1 >= t->room) { t->full = 1; return; }
    if (t->at) t->at[t->used] = c;                                                  // no place: the bytes only counted
    ++t->used;
}

static void put_chars(Text* t, const char* s) {
    while (*s) put_char(t, *s++);
}

static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };

static void put_number(Text* t, i32 v) {
    u32 n = (u32)v;
    if (v < 0) { put_char(t, '-'); n = 0u - n; }
    int begun = 0;
    for (u32 k = 0; k < 10u; ++k) {                                                 // by subtraction: nothing divided
        u32 digit = 0;
        while (n >= TENS[k]) { n -= TENS[k]; ++digit; }
        if (digit || begun || k == 9u) { put_char(t, (char)('0' + digit)); begun = 1; }
    }
}

static void put_quoted(Text* t, const char* s, u32 n) {
    static const char HEX[] = "0123456789abcdef";
    put_char(t, '"');
    for (u32 i = 0; i < n; ++i) {
        u8 c = (u8)s[i];
        if (c == '"' || c == '\\') { put_char(t, '\\'); put_char(t, (char)c); }
        else if (c == '\b') put_chars(t, "\\b");
        else if (c == '\f') put_chars(t, "\\f");
        else if (c == '\n') put_chars(t, "\\n");
        else if (c == '\r') put_chars(t, "\\r");
        else if (c == '\t') put_chars(t, "\\t");
        else if (c < 0x20u) { put_chars(t, "\\u00"); put_char(t, HEX[c >> 4]); put_char(t, HEX[c & 15u]); }
        else put_char(t, (char)c);
    }
    put_char(t, '"');
}

// The JSON text of the value at `at`, compact. Answers the tokens' bytes, 0 when they are not whole.
static u32 to_text(const u8* at, u32 n, Text* t) {
    u32 span = value_span(at, n);
    if (span == 0) return 0;
    u32 i = 0;
    u32 first[DEPTH_MOST + 2];
    u32 depth = 0;
    first[0] = 1;
    while (i < span) {
        u32 tok = at[i];
        if (tok == (u32)T_LIST_END || tok == (u32)T_OBJECT_END) {
            put_char(t, tok == (u32)T_LIST_END ? ']' : '}');
            if (depth) --depth;
            ++i;
            continue;
        }
        if (tok == (u32)T_KEY) {
            if (!first[depth]) put_char(t, ',');
            first[depth] = 0;
            u32 len = get_half(at + i + 1);
            put_quoted(t, (const char*)(at + i + 3), len);
            put_char(t, ':');
            i += 4 + len;
            tok = at[i];
            if (tok == (u32)T_LIST_END || tok == (u32)T_OBJECT_END) return 0;
        } else if (depth > 0) {
            if (!first[depth]) put_char(t, ',');
            first[depth] = 0;
        }
        if (tok == (u32)T_NULL) { put_chars(t, "null"); ++i; }
        else if (tok == (u32)T_FALSE) { put_chars(t, "false"); ++i; }
        else if (tok == (u32)T_TRUE) { put_chars(t, "true"); ++i; }
        else if (tok == (u32)T_INT || tok == (u32)T_INT8 || tok == (u32)T_INT16) {
            put_number(t, whole_at(at + i));
            i += tok == (u32)T_INT ? 5u : tok == (u32)T_INT16 ? 3u : 2u;
        }
        else if (tok == (u32)T_TEXT) { u32 len = get_half(at + i + 1); put_quoted(t, (const char*)(at + i + 3), len); i += 4 + len; }
        else if (tok == (u32)T_LIST || tok == (u32)T_OBJECT) {
            put_char(t, tok == (u32)T_LIST ? '[' : '{');
            if (depth + 1u > (u32)DEPTH_MOST) return 0;
            ++depth;
            first[depth] = 1;
            ++i;
        } else {
            return 0;
        }
    }
    return span;
}

// A JSON text parsed into tokens: what op1f_value_set and the store hand in. Only the kinds the base can write: null,
// true, false, whole numbers of 32 bits, texts (no zero character in them), lists and objects nested at most four deep
// (no member named twice in one object).
struct Parse { const char* s; u32 i, n; Out* o; };

static void skip_space(Parse* p) {
    while (p->i < p->n && (p->s[p->i] == ' ' || p->s[p->i] == '\t' || p->s[p->i] == '\n' || p->s[p->i] == '\r')) ++p->i;
}

static int hex_digit(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

// A quoted text: its characters (UTF-8) after the token's head, written in place. 0 when it is not one.
static int parse_text(Parse* p, u32 token) {
    if (p->i >= p->n || p->s[p->i] != '"') return 0;
    ++p->i;
    Out* o = p->o;
    u32 head = o->used;
    put_byte(o, token);
    put_byte(o, 0);
    put_byte(o, 0);
    u32 begin = o->used;
    for (;;) {
        if (p->i >= p->n) return 0;
        u8 c = (u8)p->s[p->i++];
        if (c == '"') break;
        if (c < 0x20u) return 0;
        if (c != '\\') { put_byte(o, c); continue; }
        if (p->i >= p->n) return 0;
        char e = p->s[p->i++];
        if (e == '"' || e == '\\' || e == '/') put_byte(o, (u8)e);
        else if (e == 'b') put_byte(o, 8);
        else if (e == 'f') put_byte(o, 12);
        else if (e == 'n') put_byte(o, 10);
        else if (e == 'r') put_byte(o, 13);
        else if (e == 't') put_byte(o, 9);
        else if (e == 'u') {
            u32 code = 0;
            for (u32 k = 0; k < 4u; ++k) {
                int d = p->i < p->n ? hex_digit(p->s[p->i]) : -1;
                if (d < 0) return 0;
                code = code << 4 | (u32)d;
                ++p->i;
            }
            if (code >= 0xd800u && code < 0xdc00u) {                                // a pair of halves
                if (p->i + 6 > p->n || p->s[p->i] != '\\' || p->s[p->i + 1] != 'u') return 0;
                p->i += 2;
                u32 low = 0;
                for (u32 k = 0; k < 4u; ++k) {
                    int d = hex_digit(p->s[p->i]);
                    if (d < 0) return 0;
                    low = low << 4 | (u32)d;
                    ++p->i;
                }
                if (low < 0xdc00u || low >= 0xe000u) return 0;
                code = 0x10000u + ((code - 0xd800u) << 10) + (low - 0xdc00u);
            } else if (code >= 0xdc00u && code < 0xe000u) {
                return 0;
            }
            if (code == 0) return 0;                                                // a zero character: the firmware's texts end at one
            if (code < 0x80u) put_byte(o, code);
            else if (code < 0x800u) { put_byte(o, 0xc0u | code >> 6); put_byte(o, 0x80u | (code & 0x3fu)); }
            else if (code < 0x10000u) { put_byte(o, 0xe0u | code >> 12); put_byte(o, 0x80u | (code >> 6 & 0x3fu)); put_byte(o, 0x80u | (code & 0x3fu)); }
            else { put_byte(o, 0xf0u | code >> 18); put_byte(o, 0x80u | (code >> 12 & 0x3fu)); put_byte(o, 0x80u | (code >> 6 & 0x3fu)); put_byte(o, 0x80u | (code & 0x3fu)); }
        } else {
            return 0;
        }
    }
    if (o->full) return 0;
    u32 len = o->used - begin;
    if (len > 0xffffu) return 0;
    o->at[head + 1] = (u8)(len & 0xffu);
    o->at[head + 2] = (u8)(len >> 8);
    put_byte(o, 0);
    return !o->full;
}

static int same_key(const u8* a, const u8* b) {
    u32 la = get_half(a + 1), lb = get_half(b + 1);
    if (la != lb) return 0;
    for (u32 k = 0; k < la; ++k)
        if (a[3 + k] != b[3 + k]) return 0;
    return 1;
}

static int parse_value(Parse* p, u32 depth) {
    skip_space(p);
    if (p->i >= p->n) return 0;
    char c = p->s[p->i];
    Out* o = p->o;
    if (c == 'n' || c == 't' || c == 'f') {
        const char* word = c == 'n' ? "null" : c == 't' ? "true" : "false";
        u32 k = 0;
        for (; word[k]; ++k)
            if (p->i + k >= p->n || p->s[p->i + k] != word[k]) return 0;
        p->i += k;
        put_byte(o, c == 'n' ? T_NULL : c == 't' ? T_TRUE : T_FALSE);
        return !o->full;
    }
    if (c == '"') return parse_text(p, T_TEXT);
    if (c == '-' || (c >= '0' && c <= '9')) {
        int negative = c == '-';
        if (negative) ++p->i;
        if (p->i >= p->n || p->s[p->i] < '0' || p->s[p->i] > '9') return 0;
        if (p->s[p->i] == '0' && p->i + 1 < p->n && p->s[p->i + 1] >= '0' && p->s[p->i + 1] <= '9') return 0;
        u32 v = 0;
        while (p->i < p->n && p->s[p->i] >= '0' && p->s[p->i] <= '9') {
            u32 d = (u32)(p->s[p->i] - '0');
            if (v > 214748364u || (v == 214748364u && d > (negative ? 8u : 7u))) return 0;   // beyond 32 bits
            v = v * 10u + d;
            ++p->i;
        }
        if (p->i < p->n && (p->s[p->i] == '.' || p->s[p->i] == 'e' || p->s[p->i] == 'E')) return 0;   // a fraction: a kind the base does not write
        put_whole(o, (i32)(negative ? 0u - v : v));
        return !o->full;
    }
    if (c == '[' || c == '{') {
        if (depth >= (u32)DEPTH_MOST) return 0;
        int list = c == '[';
        ++p->i;
        put_byte(o, list ? T_LIST : T_OBJECT);
        u32 keys_from = o->used;
        skip_space(p);
        if (p->i < p->n && p->s[p->i] == (list ? ']' : '}')) {
            ++p->i;
            put_byte(o, list ? T_LIST_END : T_OBJECT_END);
            return !o->full;
        }
        for (;;) {
            if (!list) {
                skip_space(p);
                u32 key_at = o->used;
                if (!parse_text(p, T_KEY)) return 0;
                // the same name twice in one object: the firmware would keep one of the two; refused
                u32 k = keys_from;
                while (k < key_at) {
                    if (o->at[k] != (u32)T_KEY) return 0;
                    u32 len = get_half(o->at + k + 1);
                    if (same_key(o->at + k, o->at + key_at)) return 0;
                    u32 span = value_span(o->at + k + 4 + len, key_at - (k + 4 + len));
                    if (span == 0) return 0;
                    k += 4 + len + span;
                }
                skip_space(p);
                if (p->i >= p->n || p->s[p->i] != ':') return 0;
                ++p->i;
            }
            if (!parse_value(p, depth + 1u)) return 0;
            skip_space(p);
            if (p->i >= p->n) return 0;
            char e = p->s[p->i++];
            if (e == ',') continue;
            if (e == (list ? ']' : '}')) break;
            return 0;
        }
        put_byte(o, list ? T_LIST_END : T_OBJECT_END);
        return !o->full;
    }
    return 0;
}

// Answers the tokens' bytes, 0 when the text is not one value of the kinds above, wholly.
static u32 parse(const char* text, u32 n, Out* o) {
    Parse p = { text, 0, n, o };
    if (!parse_value(&p, 0)) return 0;
    skip_space(&p);
    if (p.i != n || o->full) return 0;
    return o->used;
}

// ---------------------------------------------------------------- the entries of a holder

// The entry of a name: its place in the data, or -1.
static int entry_of(const Holder* h, u32 name) {
    u32 i = 0;
    while (i + 3 <= h->bytes) {
        u32 len = get_half(h->data + i + 1);
        if (h->data[i] == name) return (int)i;
        i += 3 + len;
    }
    return -1;
}

// R5: kept out of line: inlined into the set's body (now shared by a pack's set and the base's), its loop was counted
// by the compiler with an index register, where base 27's unit has none.
static OUT_OF_LINE void entry_remove(Holder* h, u32 name) {
    int at = entry_of(h, name);
    if (at < 0) return;
    u32 len = 3 + get_half(h->data + at + 1);
    for (u32 i = (u32)at; i + len < h->bytes; ++i) h->data[i] = h->data[i + len];
    h->bytes -= len;
}

// An entry made in place at the holder's end: the room behind its three bytes of head, then the entry closed with
// what was written there (0 when it did not fit: nothing of it stays).
static Out entry_room(Holder* h) {
    u32 room = h->bytes + 3u <= (u32)HOLDER_BYTES ? (u32)HOLDER_BYTES - h->bytes - 3u : 0u;
    Out o = { h->data + (room ? h->bytes + 3u : 0u), 0, room, 0 };
    return o;
}

static int entry_close(Holder* h, u32 name, const Out* o) {
    if (o->full || o->used == 0 || o->used > 0xffffu || o->room == 0) return 0;
    u8* at = h->data + h->bytes;
    at[0] = (u8)name;
    at[1] = (u8)(o->used & 0xffu);
    at[2] = (u8)(o->used >> 8);
    h->bytes += 3u + o->used;
    return 1;
}

// ---------------------------------------------------------------- the list of names

static int plain_key(const char* name) {
    u32 n = 0;
    for (; n < (u32)NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return 0;
    }
    if (n == 0 || n >= (u32)NAME) return 0;
    for (u32 k = n; k < (u32)NAME; ++k)
        if (name[k] != 0) return 0;
    return 1;
}

static int name_of_the_unit(const char* name) {
    for (u32 i = 0; NAMES_OF_THE_UNIT[i][0]; ++i)
        if (same(NAMES_OF_THE_UNIT[i], name)) return 1;
    return 0;
}

static int name_index(const char* name) {
    for (u32 i = 0; i < names_count; ++i)
        if (same(names[i].name, name)) return (int)i;
    return -1;
}

static int plain_ask_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)KIND_NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)KIND_NAME;
}

static const char* where_words(u32 files) {
    static const char* const WORDS[16] = { "nothing", "synth sounds", "drum sounds", "synth sounds and drum sounds", "tapes", "synth sounds and tapes",
                                           "drum sounds and tapes", "synth sounds and drum sounds and tapes", "the master", "synth sounds and the master",
                                           "drum sounds and the master", "synth sounds and drum sounds and the master", "tapes and the master",
                                           "synth sounds and tapes and the master", "drum sounds and tapes and the master",
                                           "synth sounds and drum sounds and tapes and the master" };
    return WORDS[files & (u32)KEEP_ALL];
}

static const char* pack_name(u32 owner) {
    return owner == (u32)OWNER_BASE ? "the base" : owner ? op1f_cards_2.slots[owner - 1u].card->name : "a pack that is away";
}

// The asks of this kind of every card in use, at the first frame, in the cards' order and each card's. R5: the base's
// own name first, kept with the sounds of both instruments and with the master.
static void enrol() {
    Name* own = &names[BASE_NAME];
    copy_text(own->name, BASE_KEY, NAME);
    own->files = OP1F_KEEP_SYNTH | OP1F_KEEP_DRUM | OP1F_KEEP_MASTER;
    own->owner = OWNER_BASE;
    Name* engine = &names[BASE_ENGINE];                                            // R6: the engine a synth sound rides
    copy_text(engine->name, ENGINE_KEY, NAME);
    engine->files = OP1F_KEEP_SYNTH | OP1F_KEEP_DRUM;                              // R21 (base 43): and the drum engine a drum sound rides
    engine->owner = OWNER_BASE;
    Name* lfo = &names[BASE_LFO];                                                  // R11: the LFO layer of a synth or drum sound
    copy_text(lfo->name, LFO_KEY, NAME);
    lfo->files = OP1F_KEEP_SYNTH | OP1F_KEEP_DRUM;                                 // never the master
    lfo->owner = OWNER_BASE;
    names_count = BASE_LFO + 1u;
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;
        const Op1fCardSlot* slot = &op1f_cards_2.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 7u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        int had = 0;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_KEEPS) continue;
            const Op1fKeeps* t = (const Op1fKeeps*)ask->what;
            const char* ask_name = t && ask->bytes >= (u32)TABLE_BYTES && plain_ask_name(t->head.name) ? t->head.name : "?";
            u32 a = 0, b = 0;
            int bad = 1;
            if (t == 0 || ask->bytes < (u32)TABLE_BYTES) { a = ask->bytes; b = TABLE_BYTES; }
            else if (!plain_ask_name(t->head.name)) { a = 0; b = 0; }
            else if (t->head.bytes != ask->bytes) { a = t->head.bytes; b = ask->bytes; }
            else if (t->head.flags != 0) { a = t->head.flags; b = 0; }
            else if (t->head.shown != 0) { a = (u32)t->head.shown - slot->first; b = 0; }
            else if (t->changed != 0 && ((u32)t->changed < slot->first || (u32)t->changed >= slot->end || ((u32)t->changed & 1u))) {
                a = (u32)t->changed - slot->first;
                b = slot->end - slot->first;
            } else if (t->count < 1u || t->count > (u32)KEYS) { a = t->count; b = KEYS; }
            else if (had) { a = k; b = 1; }
            else bad = 0;
            if (bad) {
                op1f_log(2, LOG_FILE, 3, "keeps: '%s' asks to keep '%s', which its table is not in order (%u, %u): left aside", card->name, ask_name, a, b);
                continue;
            }
            had = 1;
            u32 files = 0, kept = 0;
            for (u32 j = 0; j < t->count; ++j) {
                const Op1fKeptKey* key = &t->keys[j];
                const char* shown = plain_key(key->name) ? key->name : "?";
                const char* why = 0;
                int twice = 0;
                for (u32 m = 0; m < j; ++m)
                    if (plain_key(t->keys[m].name) && same(t->keys[m].name, key->name)) twice = 1;
                // R5: the master is a place for a card of level 8; before it the bit is no place
                u32 places = card->level >= 8u ? key->files : key->files & ~(u32)OP1F_KEEP_MASTER;
                if (!plain_key(key->name) || places == 0 || (places & ~(u32)KEEP_ALL) != 0 || twice) why = "is not a name a file can hold";
                else if (name_of_the_unit(key->name)) why = "is a name of the unit's own files";
                else if (same(key->name, BASE_KEY) || same(key->name, ENGINE_KEY) || same(key->name, LFO_KEY)) why = "is a name of the base's own";   // R6: the second too; R11: the third
                else if (name_index(key->name) >= 0) why = "an earlier pack keeps";
                else if (names_count >= (u32)NAMES_ROOM) why = "would be the sixty-fifth name";
                if (why) {
                    op1f_log(2, LOG_FILE, 2, "keeps: '%s' asks to keep '%s', which %s: left aside", card->name, shown, why);
                    continue;
                }
                Name* n = &names[names_count++];
                copy_text(n->name, key->name, NAME);
                n->files = places;
                n->owner = i + 1u;
                files |= places;
                ++kept;
            }
            if (kept == 0) continue;
            if (keeps_count < (u32)KEEPS_MOST) {
                keeps[keeps_count].slot = i;
                keeps[keeps_count].table = t;
                keeps[keeps_count].files = files;
                ++keeps_count;
            }
            op1f_log(3, LOG_FILE, 1, "keeps: '%s' keeps %u value(s) in %s", card->name, kept, where_words(files));
        }
    }
}

// ---------------------------------------------------------------- the reads (the background worker)

// The values of a document just read: every member whose name the list holds for that kind of file. `carry`: the
// document is the one the slot will hold, so the names it holds are the base's to write over at the save.
static void take_from(Holder* h, const void* document, u32 files, int carry) {
    holder_clear(h);
    if (!ready || document == 0 || kind_of(document) != (u32)KIND_OBJECT) return;
    u32 count = names_count;
    for (u32 i = 0; i < count; ++i) {
        if ((names[i].files & files) == 0 || !STOCK_JSON_HAS(document, names[i].name)) continue;
        if (carry) set_bit(h->carried, i);
        Out o = entry_room(h);
        u32 why = from_value(STOCK_JSON_MEMBER(document, names[i].name), &o, 0);
        if (why == 0) {
            Text t = { 0, 0, VALUE_MOST + 2, 0 };                                   // its text's bytes, counted
            to_text(o.at, o.used, &t);
            if (t.full || t.used > (u32)VALUE_MOST) why = WHY_LONG;
        }
        if (why == 0 && !entry_close(h, i, &o)) why = WHY_ROOM;
        if (why) say_later(Q_NOT_TAKEN, i, 0, 0, why);
    }
}

// R6b: the list's two kinds beside the elder three. A list job's put runs the firmware's pass on the record's document
// (the synth's 0x8d4f787c(record + 8, 0) through 0x8d4f5544, the drum's 0x8d4f787c(record + 8, 1) through 0x8d4f45d0,
// both with the put's flag 0), as the preset's load over MIDI does: the slot does not hold the document read, so its
// names are not carried.
static void waiting_take(u32 kind, const void* json) {
    waiting_kind = kind;
    if (!ready) ++unit.early_loads;
    u32 drum = kind == (u32)FROM_DRUM || kind == (u32)FROM_LIST_DRUM;
    u32 carry = kind == (u32)FROM_DRUM || kind == (u32)FROM_SYNTH_SAMPLER;
    take_from(&waiting, json, drum ? (u32)OP1F_KEEP_DRUM : (u32)OP1F_KEEP_SYNTH, (int)carry);
}

// Stands in for the synth's preset read, before the firmware keeps only the members it knows.
extern "C" int op1f_keeps_read_preset(void* a, void* text, void* json, int flag) OP1F_NAMED("op1f_keeps_read_preset");
extern "C" int op1f_keeps_read_preset(void* a, void* text, void* json, int flag) {
    int ok = STOCK_JSON_READ_RAW(a, text, json, flag);
    waiting_take(FROM_SYNTH_PRESET, ok ? json : 0);
    return ok;
}

// Stands in for the synth sampler's preset read.
extern "C" int op1f_keeps_read_sampler(const char* path, void* json) OP1F_NAMED("op1f_keeps_read_sampler");
extern "C" int op1f_keeps_read_sampler(const char* path, void* json) {
    int ok = STOCK_JSON_READ_FILE(path, json);
    waiting_take(FROM_SYNTH_SAMPLER, ok ? json : 0);
    return ok;
}

// Stands in for the drum's preset read.
extern "C" int op1f_keeps_read_drum(const char* path, void* json) OP1F_NAMED("op1f_keeps_read_drum");
extern "C" int op1f_keeps_read_drum(const char* path, void* json) {
    int ok = STOCK_JSON_READ_FILE(path, json);
    waiting_take(FROM_DRUM, ok ? json : 0);
    return ok;
}

// R6b (M3): stands in for the synth's list job's read of the sound picked (L1), and the drum's (L3): the plain reader
// (path, document), then the members taken at once, before anything else touches the document.
extern "C" int op1f_keeps_read_list_synth(const char* path, void* json) OP1F_NAMED("op1f_keeps_read_list_synth");
extern "C" int op1f_keeps_read_list_synth(const char* path, void* json) {
    int ok = STOCK_JSON_READ_FILE(path, json);
    waiting_take(FROM_LIST_SYNTH, ok ? json : 0);
    return ok;
}

extern "C" int op1f_keeps_read_list_drum(const char* path, void* json) OP1F_NAMED("op1f_keeps_read_list_drum");
extern "C" int op1f_keeps_read_list_drum(const char* path, void* json) {
    int ok = STOCK_JSON_READ_FILE(path, json);
    waiting_take(FROM_LIST_DRUM, ok ? json : 0);
    return ok;
}

// The mark of a sound: its `name` and `type`, from a document that is an object, each only when it is a text. Into a
// holder, or a mark alone (R5).
template <typename Marked> static void mark_from(Marked* h, const void* document) {
    h->marked = 1;
    h->mark_name[0] = h->mark_type[0] = 0;
    if (document == 0 || kind_of(document) != (u32)KIND_OBJECT) return;
    static const char* const WHICH[2] = { "name", "type" };
    for (u32 k = 0; k < 2u; ++k) {
        char* to = k ? h->mark_type : h->mark_name;
        if (!STOCK_JSON_HAS(document, WHICH[k])) continue;
        const void* v = STOCK_JSON_MEMBER(document, WHICH[k]);
        if (kind_of(v) != (u32)KIND_STRING) continue;
        copy_text(to, STOCK_JSON_CSTRING(v), MARK);
    }
}

static int record_slot(u32 record, u32* host) {
    for (u32 h = 0; h < (u32)HOSTS; ++h) {
        u32 base = RECORDS_OF(h);
        for (u32 s = 0; s < (u32)SLOTS; ++s)
            if (record == base + (u32)RECORD_BYTES * s) { *host = h; return (int)s; }
    }
    return -1;
}

// R6: the engine member of a holder as a name, when it holds a JSON string of one to fifteen characters; else 0. The
// text lives in the holder: the engines unit copies it at once. Kept out of line, as R5's entry_remove: inlined into the
// assign's body, its walk was counted by the compiler with an index register, where base 28's unit has none.
static OUT_OF_LINE const char* engine_member_of(const Holder* h) {
    int e = entry_of(h, BASE_ENGINE);
    if (e < 0) return 0;
    const u8* at = h->data + e + 3;
    if (at[0] != (u8)T_TEXT) return 0;
    u32 len = get_half(at + 1);
    if (len == 0 || len > 15u) return 0;
    return (const char*)(at + 3);
}

// R6: a sound about to be put on a synth slot's record, handed to the engines unit: a preset or a sampler's sound
// loaded from a file, with the member this unit took at the read (or none); the engine job's sound. Any thread the
// assign runs on; the engines unit reads the member's text and sets single words in a mask.
static void engine_assign(void* document, const void* json, u32 kind, u32 engine) {
    if (&op1f_engines_assign == 0) return;
    u32 host = 0;
    int slot = record_slot((u32)document - (u32)RECORD_DOCUMENT, &host);
    if (slot < 0 || host != 0) return;
    if (kind == (u32)FROM_SYNTH_PRESET || kind == (u32)FROM_SYNTH_SAMPLER || kind == (u32)FROM_LIST_SYNTH)   // R6b: the list's too
        op1f_engines_assign((u32)slot, (u32)OP1F_KEPT_LOADED, (void*)json, engine_member_of(&waiting));
    else if (engine == 1u)
        op1f_engines_assign((u32)slot, (u32)OP1F_KEPT_ENGINE, (void*)json, 0);
}

// From the stub at the place where a sound is put on a slot's record: the firmware's own assign first, then the
// values of the sound become the slot's (a load), or stay the slot's (a change of the engine).
extern "C" u32 op1f_keeps_assign(void* document, const void* json, u32 caller) OP1F_NAMED("op1f_keeps_assign");
extern "C" u32 op1f_keeps_assign(void* document, const void* json, u32 caller) {
    u32 kind = caller == CALLER_SYNTH_PRESET ? (u32)FROM_SYNTH_PRESET : caller == CALLER_DRUM ? (u32)FROM_DRUM
             : caller == CALLER_SYNTH_SAMPLER ? (u32)FROM_SYNTH_SAMPLER : (u32)FROM_NONE;
    u32 engine = caller == CALLER_SYNTH_TYPE ? 1u : caller == CALLER_DRUM_TYPE ? 2u : 0u;
    if (kind != (u32)FROM_NONE && waiting_kind != kind) waiting_take(kind, json);  // no read of this kind came before: what the document itself carries
    engine_assign(document, json, kind, engine);                                    // R6: before the firmware's own assign
    u32 result = STOCK_JSON_ASSIGN(document, json);
    u32 host = 0;
    int slot = record_slot((u32)document - (u32)RECORD_DOCUMENT, &host);
    if (slot >= 0 && ready) {
        u32 index = host * (u32)SLOTS + (u32)slot;
        if (kind != (u32)FROM_NONE && host == (kind == (u32)FROM_DRUM ? 1u : 0u)) {
            mark_from(&waiting, json);
            put_in(index, &waiting, OP1F_KEPT_LOADED, 0);
        } else if (engine && host == engine - 1u) {
            Holder* c = &waiting;                                                   // the slot's values stay; its mark takes the new engine
            take_out(index, c);
            mark_from(c, json);
            put_in(index, c, OP1F_KEPT_ENGINE, 0);
        }
    }
    waiting_kind = FROM_NONE;
    holder_clear(&waiting);
    return result;
}

// R6b (M3): a list job's put (the background worker). The sound handed to the engines unit first (a synth slot), the
// firmware's put (record, slot, document, flag) as the job called it, then the slot's values are the sound's, marked
// with its name and type, and the load told at the next frame: as op1f_keeps_assign does for a sound loaded over MIDI.
// The members were taken at the job's read (L1, L3); a put with no read of its kind before takes what the document
// itself carries.
static void list_put(u32 host, void* record, int slot, void* json, int flag) {
    u32 kind = host ? (u32)FROM_LIST_DRUM : (u32)FROM_LIST_SYNTH;
    if (waiting_kind != kind) waiting_take(kind, json);
    if (host == 0) engine_assign((u8*)record + RECORD_DOCUMENT, json, kind, 0);    // R6: before the firmware builds the sound
    (host ? STOCK_DRUM_PUT : STOCK_SYNTH_PUT)(record, slot, json, flag);
    u32 on = 0;
    int at = record_slot((u32)record, &on);
    if (at >= 0 && at == slot && on == host && ready) {
        mark_from(&waiting, json);
        put_in(host * (u32)SLOTS + (u32)at, &waiting, OP1F_KEPT_LOADED, 0);
    }
    waiting_kind = FROM_NONE;
    holder_clear(&waiting);
}

// From the stub at L2: answers the document, which the job's R0 = R6 under the stub's CALL.a was (R2 = R6 before the
// call, both from the eight bytes the stub stands for).
extern "C" void* op1f_keeps_put_synth(void* record, int slot, void* json, int flag) OP1F_NAMED("op1f_keeps_put_synth");
extern "C" void* op1f_keeps_put_synth(void* record, int slot, void* json, int flag) {
    list_put(0, record, slot, json, flag);
    return json;
}

// From the stub at L4 (the job sets R0 itself after the call).
extern "C" void op1f_keeps_put_drum(void* record, int slot, void* json, int flag) OP1F_NAMED("op1f_keeps_put_drum");
extern "C" void op1f_keeps_put_drum(void* record, int slot, void* json, int flag) {
    list_put(1, record, slot, json, flag);
}

// ---------------------------------------------------------------- the saves

// After a set of the tape's save (the background worker): whether the count of the tape's loads is still `loads`, the
// count read with the copy the value came from (the value went into that copy's tape), and then the name marked as one
// the base wrote into the tape since its load; both in one mask, so that no load's clearing of the marks comes between.
static int tape_mark_written(u32 name, u32 loads) {
    u32 mask;
    MASK_OFF(mask);
    int same_tape = tape_loads == loads;
    if (same_tape) set_bit(tape_written, name);
    MASK_ON(mask);
    return same_tape;
}

// The tape's save cut by a tape load, before the entry at `at` of its copy: that entry and the rest are left out (they
// belong to the tape before, whose file the switch has written already), and said. Answers 0: a cut save tells no pack
// OP1F_KEPT_SAVED.
static u32 tape_cut(const Holder* copy, u32 at) {
    u32 left = 0;
    while (at + 3 <= copy->bytes) {
        u32 name = copy->data[at];
        at += 3 + get_half(copy->data + at + 1);
        if (name < names_count) ++left;
    }
    say_later(Q_CUT, 0, TAPE, left, 0);
    return 0;
}

// A set of the tape's save during which the count of the tape's loads moved (the background worker): the value may have
// reached the document of the tape loaded meanwhile, for the firmware's setter waits while a switch writes or reads a
// tape's file. The tape loaded's own value of that name is set again over it, from a copy of its values taken anew into
// `copy` (the worker's own); when it holds none and its document has a member of that name now, the member stays (the
// base cannot take a member out), marked as the base's so that a pack's value of that tape is written over it later.
static void tape_stray(u32 name, const char* key, Holder* copy) {
    u32 loads = take_out_counted(TAPE, copy);
    int e = entry_of(copy, name);
    if (e >= 0) {
        u32 len = get_half(copy->data + e + 1);
        Json value;
        u32 made = to_value(copy->data + e + 3, len, &value, 0);
        if (made != len) {
            if (made) STOCK_JSON_DESTROY(&value);
            return;
        }
        if (tape_loads == loads) {                                                  // no further load since the copy
            STOCK_TAPE_SET(TAPE_DB, key, &value);
            if (tape_mark_written(name, loads)) say_later(Q_STRAY, name, TAPE, 0, STRAY_PUT_BACK);
        }
        STOCK_JSON_DESTROY(&value);
        return;
    }
    if (STOCK_JSON_HAS(STOCK_TAPE_ROOT(), key) && tape_mark_written(name, loads)) say_later(Q_STRAY, name, TAPE, 0, STRAY_STAYS);
}

// The values of a holder put into a document about to be written: as members of the document itself (a preset), or
// through the tape's setter (`tape`). Beside the names the holder carried, `own` are names the base may write over.
// For the tape, the count of the tape's loads is read with the copy and looked at again before each value, right
// before the setter and right after it: a tape load since the copy was taken ends the save (tape_cut, tape_stray).
static u32 put_values(u32 index, Holder* copy, void* document, const u32* own, int tape) {
    u32 loads = take_out_counted(index, copy);
    u32 put = 0;
    if (document == 0 || kind_of(document) != (u32)KIND_OBJECT) return 0;
    u32 i = 0;
    while (i + 3 <= copy->bytes) {
        u32 at = i;
        u32 name = copy->data[i];
        u32 len = get_half(copy->data + i + 1);
        const u8* tokens = copy->data + i + 3;
        i += 3 + len;
        if (name >= names_count) continue;
        if (tape && tape_loads != loads) return tape_cut(copy, at);                // the document is the next tape's now
        const char* key = names[name].name;
        if (STOCK_JSON_HAS(document, key) && !bit_of(copy->carried, name) && !(own && bit_of(own, name))) {
            if (tape && tape_loads != loads) return tape_cut(copy, at);            // the look was at the next tape's document: no refusal said
            say_later(Q_REFUSED, name, index, 0, WHY_FIRMWARE);
            continue;
        }
        Json value;
        u32 made = to_value(tokens, len, &value, 0);
        if (made != len) {                                                          // never half a member
            if (made) STOCK_JSON_DESTROY(&value);
            say_later(Q_REFUSED, name, index, 0, WHY_MADE);
            continue;
        }
        Text text = { 0, 0, 0x7fffffffu, 0 };                                       // the member's bytes in the file, counted
        to_text(tokens, len, &text);
        if (tape) {
            if (tape_loads != loads) {                                              // right before the setter
                STOCK_JSON_DESTROY(&value);
                return tape_cut(copy, at);
            }
            STOCK_TAPE_SET(TAPE_DB, key, &value);
            STOCK_JSON_DESTROY(&value);
            if (!tape_mark_written(name, loads)) {                                  // a load came while the setter ran
                tape_stray(name, key, copy);
                return 0;
            }
        } else {
            void* place = STOCK_JSON_PLACE(document, key);
            if (place) STOCK_JSON_ASSIGN(place, &value);
            STOCK_JSON_DESTROY(&value);
        }
        say_later(Q_WRITTEN, name, index, text.used, 0);
        ++put;
    }
    if (tape && tape_loads != loads) return 0;                                      // whole, but the tape changed since: no pack told
    return put;
}

// From the stubs at the preset's save (the main task): the synth's (host 0) or the drum's (1), `slot` where the save
// keeps it, `json` the document about to be written, a copy of the slot's own.
extern "C" void op1f_keeps_saving(int host, int slot, void* json) OP1F_NAMED("op1f_keeps_saving");
extern "C" void op1f_keeps_saving(int host, int slot, void* json) {
    if (!ready) { ++unit.early_saves; return; }
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS) { ++unit.bad_slots; return; }   // not where the save keeps it: nothing added
    u32 index = (u32)host * (u32)SLOTS + (u32)slot;
    if (put_values(index, &save_copy, json, 0, 0)) tell(index, OP1F_KEPT_SAVED);
    // R6b (M4): the document's mark (the save named it before this place) waits for the record to show it.
    mark_from(&save_mark, json);
    u32 count = remark_count + 1u;
    remark_count = count ? count : 1u;
    u32 mask;
    MASK_OFF(mask);
    Remark* r = &remarks[index];
    r->due = remark_count;
    r->since = cycles_now();
    mark_copy(&r->mark, &save_mark);
    remarks_any = 1;
    MASK_ON(mask);
}


// Stands in for the tape's save's last set of a member (the background worker): that set, then the tape's values.
extern "C" int op1f_keeps_tape_saving(int db, const char* key, const void* value) OP1F_NAMED("op1f_keeps_tape_saving");
extern "C" int op1f_keeps_tape_saving(int db, const char* key, const void* value) {
    int answer = STOCK_TAPE_SET(db, key, value);
    if (!ready) { ++unit.early_saves; return answer; }
    if (put_values(TAPE, &tape_save_copy, STOCK_TAPE_ROOT(), tape_written, 1)) tell(TAPE, OP1F_KEPT_SAVED);
    return answer;
}

// After the firmware's tape load (the stub comes here with the flag the load was handed). Whatever the flag, the
// firmware has read the tape's file into the tape's document before its load: the file of the tape picked, of a tape
// made anew (the template, written by the format), or of a copy (the original's, copied as a file). So the values are
// taken from that document every time, and every name it holds is the base's to write over at the tape's next write:
// a copy keeps the original's values as its file does, a tape made anew holds none. The flag only says which line.
static u32 entries_of(const Holder* h) {
    u32 i = 0, n = 0;
    while (i + 3 <= h->bytes) {
        i += 3 + get_half(h->data + i + 1);
        ++n;
    }
    return n;
}

static void tape_take(Holder* c, u32 flag) {
    take_from(c, STOCK_TAPE_ROOT(), OP1F_KEEP_TAPE, 1);
    for (u32 i = 0; i < (u32)BITS; ++i) tape_written[i] = 0;
    if (flag != 0) say_later(Q_TAPE_LOADED, entries_of(c), TAPE, flag, 0);
    put_in(TAPE, c, flag != 0 ? (u32)OP1F_KEPT_MADE : (u32)OP1F_KEPT_LOADED, 0);
}

extern "C" void op1f_keeps_tape_loaded(int flag) OP1F_NAMED("op1f_keeps_tape_loaded");
extern "C" void op1f_keeps_tape_loaded(int flag) {
    tape_seen = 1;
    if (!ready) {
        tape_early_flag = (u32)flag;
        tape_early = 1;
        return;
    }
    tape_take(&tape_load_copy, (u32)flag);
}

// ---------------------------------------------------------------- the store (the display thread)

static void text_holder_values(Text* t, const Holder* h) {
    put_char(t, '{');
    u32 i = 0, first = 1;
    while (i + 3 <= h->bytes) {
        u32 name = h->data[i];
        u32 len = get_half(h->data + i + 1);
        if (name < names_count) {
            if (!first) put_char(t, ',');
            first = 0;
            put_quoted(t, names[name].name, length_of(names[name].name, NAME));
            put_char(t, ':');
            to_text(h->data + i + 3, len, t);
        }
        i += 3 + len;
    }
    put_char(t, '}');
}

// R5: whether a slot of the synth or the drum holds a value of `name` now (read inside one mask, nothing copied).
static int a_slot_holds(u32 name) {
    int found = 0;
    for (u32 index = 0; index < (u32)TAPE && !found; ++index) {
        u32 mask;
        MASK_OFF(mask);
        found = entry_of(&holders[index], name) >= 0;
        MASK_ON(mask);
    }
    return found;
}

// R5: the store's section of the master, {"names":[name,...],"values":{..}}, from the master's values in `m`.
static void text_master(Text* t, const Holder* m) {
    put_chars(t, "\"master\":{\"names\":[");
    u32 i = 0, first = 1;
    while (i + 3 <= m->bytes) {
        u32 name = m->data[i];
        if (name < names_count) {
            if (!first) put_char(t, ',');
            first = 0;
            put_quoted(t, names[name].name, length_of(names[name].name, NAME));
        }
        i += 3 + get_half(m->data + i + 1);
    }
    put_chars(t, "],\"values\":");
    text_holder_values(t, m);
    put_chars(t, "},");
}

// The store's text: {"names":[[name,files],...],"slots":[{"host":h,"name":..,"slot":s,"type":..,"values":{..}},...],"version":1}
// R5: with {"master":{..}} first while the master holds a value; in `names` each name with its places among the slots
// and the tape (a name kept with the master alone is not there), and the base's own name only while a slot holds it:
// with neither, the text is base 27's.
static int store_compose(u32* slots_kept, u32* bytes) {
    Text t = { store_text, 0, STORE_MOST, 0 };
    put_char(&t, '{');
    take_out(MASTER, &display_copy);
    if (display_copy.bytes != 0) text_master(&t, &display_copy);
    put_chars(&t, "\"names\":[");
    int base_listed = a_slot_holds(BASE_NAME);
    int engine_listed = a_slot_holds(BASE_ENGINE);                                 // R6
    int lfo_listed = a_slot_holds(BASE_LFO);                                       // R11
    for (u32 i = 0, listed = 0; i < names_count; ++i) {
        u32 places = names[i].files & (u32)KEEP_PLACES;
        if (places == 0 || (i == (u32)BASE_NAME && !base_listed) || (i == (u32)BASE_ENGINE && !engine_listed) || (i == (u32)BASE_LFO && !lfo_listed)) continue;
        if (listed++) put_char(&t, ',');
        put_char(&t, '[');
        put_quoted(&t, names[i].name, length_of(names[i].name, NAME));
        put_char(&t, ',');
        put_number(&t, (i32)places);
        put_char(&t, ']');
    }
    put_chars(&t, "],\"slots\":[");
    u32 kept = 0;
    for (u32 index = 0; index < (u32)TAPE; ++index) {
        Holder* c = &display_copy;
        take_out(index, c);
        if (c->bytes == 0) continue;
        if (kept) put_char(&t, ',');
        ++kept;
        put_chars(&t, "{\"host\":");
        put_number(&t, (i32)(index >= (u32)SLOTS ? 1 : 0));
        put_chars(&t, ",\"name\":");
        put_quoted(&t, c->mark_name, length_of(c->mark_name, MARK));
        if (index < (u32)SLOTS && sample_frames[index] != 0 && entry_of(c, BASE_ENGINE) >= 0) {   // R17: the kept cut, beside the ride
            put_chars(&t, ",\"sample\":[");
            put_number(&t, (i32)sample_frames[index]);
            put_char(&t, ',');
            put_number(&t, (i32)sample_stereo[index]);
            put_char(&t, ']');
        }
        put_chars(&t, ",\"slot\":");
        put_number(&t, (i32)(index & (u32)(SLOTS - 1)));
        put_chars(&t, ",\"type\":");
        put_quoted(&t, c->mark_type, length_of(c->mark_type, MARK));
        put_chars(&t, ",\"values\":");
        text_holder_values(&t, c);
        put_char(&t, '}');
    }
    put_chars(&t, "],\"version\":1}\n");
    *slots_kept = kept;
    *bytes = t.used;
    return !t.full;
}

static void store_write() {
    u32 kept = 0, bytes = 0;
    if (!store_compose(&kept, &bytes)) {
        unit.store_due = 0;
        op1f_log(1, LOG_FILE, 9, "keeps: the slots' values could not be kept: %s", "they would take more than 48 KiB");
        return;
    }
    u32 sum = 2166136261u;                                                          // FNV-1a of the text: the same text is not written again
    for (u32 i = 0; i < bytes; ++i) sum = (sum ^ (u8)store_text[i]) * 16777619u;
    if (unit.store_known && sum == unit.store_sum) {
        unit.store_due = 0;
        return;
    }
    int answer = op1f_folders_write_base(STORE_PATH, store_text, bytes);
    if (answer == OP1F_NOT_NOW) return;                                             // the disk mode is open: at a later frame
    unit.store_due = 0;
    if (answer != (int)bytes) {
        unit.store_known = 0;
        op1f_log(1, LOG_FILE, 9, "keeps: the slots' values could not be kept: %s", "the disk did not take the file");
        return;
    }
    unit.store_known = 1;
    unit.store_sum = sum;
    if (unit.kept_said < (u32)WRITTEN_SAID) {
        ++unit.kept_said;
        op1f_log(3, LOG_FILE, 8, "keeps: the slots' values kept (%u slot(s), %u bytes)", kept, bytes);
    }
}

static void store_changed() {
    unit.store_due = 1;
    unit.store_since = cycles_now();
}

// A member of an object read from the store, by name; 0 when there is none or the document is not an object.
static const void* member(const void* object, const char* key) {
    if (object == 0 || kind_of(object) != (u32)KIND_OBJECT || !STOCK_JSON_HAS(object, key)) return 0;
    return STOCK_JSON_MEMBER(object, key);
}

static int whole_of(const void* v, i32* out) {
    if (v == 0 || (kind_of(v) != (u32)KIND_INT && kind_of(v) != (u32)KIND_UINT)) return 0;
    *out = *(const volatile i32*)v;
    return 1;
}

static const char* text_of(const void* v) {
    if (v == 0 || kind_of(v) != (u32)KIND_STRING) return 0;
    const char* t = STOCK_JSON_CSTRING(v);
    return t ? t : "";
}

// The mark of the sound on a slot's record now, read inside one mask with the record's other reads.
template <typename Marked> static void mark_of_record(u32 host, u32 slot, Marked* h) {
    u32 mask;
    MASK_OFF(mask);
    mark_from(h, (const void*)(RECORDS_OF(host) + (u32)RECORD_BYTES * slot + (u32)RECORD_DOCUMENT));
    MASK_ON(mask);
}

enum { STORE_READ = 0, STORE_NONE = 1, STORE_LONG = 2, STORE_UNREAD = 3, STORE_NOT_AN_OBJECT = 4, STORE_VERSION = 5, STORE_NAMES = 6, STORE_SLOTS = 7,
       STORE_SLOT = 8, STORE_VALUE = 9, STORE_MASTER = 10 };

// The store's document read into `document` (made by the caller), its remembered names added to the list. 0, or why
// it is not one the base wrote.
static u32 store_names(void* document) {
    int size = STOCK_FILE_SIZE(STORE_PATH);
    if (size < 0) return STORE_NONE;
    if (size > (int)STORE_MOST) return STORE_LONG;
    if (!STOCK_JSON_READ_FILE(STORE_PATH, document)) return STORE_UNREAD;
    if (kind_of(document) != (u32)KIND_OBJECT) return STORE_NOT_AN_OBJECT;
    i32 version = 0;
    if (!whole_of(member(document, "version"), &version) || version != 1) return STORE_VERSION;
    const void* list = member(document, "names");
    if (list == 0 || kind_of(list) != (u32)KIND_LIST) return STORE_NAMES;
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i) {
        const void* row = STOCK_JSON_ELEMENT(list, i);
        if (kind_of(row) != (u32)KIND_LIST || STOCK_JSON_SIZE(row) != 2u) return STORE_NAMES;
        const char* name = text_of(STOCK_JSON_ELEMENT(row, 0));
        i32 files = 0;
        if (name == 0 || !whole_of(STOCK_JSON_ELEMENT(row, 1), &files)) return STORE_NAMES;
        char key[NAME];
        if (length_of(name, NAME) >= (u32)NAME) return STORE_NAMES;
        copy_text(key, name, NAME);
        if (!plain_key(key) || files <= 0 || (files & ~7) != 0) return STORE_NAMES;
        if (name_of_the_unit(key) || name_index(key) >= 0 || names_count >= (u32)NAMES_ROOM) continue;   // a name a pack of this start keeps, or no room
        Name* n = &names[names_count++];
        copy_text(n->name, key, NAME);
        n->files = (u32)files;
        n->owner = 0;
    }
    const void* slots = member(document, "slots");
    if (slots == 0 || kind_of(slots) != (u32)KIND_LIST || STOCK_JSON_SIZE(slots) > (u32)TAPE) return STORE_SLOTS;
    return STORE_READ;
}

// The store's slot `i` made into `h` (its values and the mark it was kept with); 0, or why not.
static u32 store_slot(const void* document, u32 i, Holder* h, u32* index) {
    const void* row = STOCK_JSON_ELEMENT(member(document, "slots"), i);
    i32 host = -1, slot = -1;
    const char* mark_name = text_of(member(row, "name"));
    const char* mark_type = text_of(member(row, "type"));
    const void* values = member(row, "values");
    if (!whole_of(member(row, "host"), &host) || !whole_of(member(row, "slot"), &slot) || host < 0 || host > 1 || slot < 0 || slot >= SLOTS ||
        mark_name == 0 || mark_type == 0 || values == 0 || kind_of(values) != (u32)KIND_OBJECT)
        return STORE_SLOT;
    *index = (u32)host * (u32)SLOTS + (u32)slot;
    holder_clear(h);
    h->marked = 1;
    copy_text(h->mark_name, mark_name, MARK);
    copy_text(h->mark_type, mark_type, MARK);
    u32 files = host ? (u32)OP1F_KEEP_DRUM : (u32)OP1F_KEEP_SYNTH;
    for (u32 n = 0; n < names_count; ++n) {
        if ((names[n].files & files) == 0) continue;
        const void* v = member(values, names[n].name);
        if (v == 0) continue;
        Out o = entry_room(h);
        if (from_value(v, &o, 0) != 0 || !entry_close(h, n, &o)) return STORE_VALUE;
    }
    return STORE_READ;
}

// R5: the store's section of the master's values, {"names":[name,...],"values":{..}}, when there is one (none in a
// store of a base before 28, nor while the master held no value). Its names join the list as names kept with the
// master: a name remembered for a pack that is away gains the master, a name of this start's asks keeps what its ask
// says (the base's own among them), a name not yet listed is remembered for the master alone. 0, or why it is not as
// this unit writes it.
static u32 store_master_names(const void* document) {
    const void* section = member(document, "master");
    if (section == 0) return STORE_READ;
    const void* list = member(section, "names");
    const void* values = member(section, "values");
    if (list == 0 || kind_of(list) != (u32)KIND_LIST || values == 0 || kind_of(values) != (u32)KIND_OBJECT) return STORE_MASTER;
    u32 count = STOCK_JSON_SIZE(list);
    if (count > (u32)NAMES_ROOM) return STORE_MASTER;
    for (u32 i = 0; i < count; ++i) {
        const char* name = text_of(STOCK_JSON_ELEMENT(list, i));
        if (name == 0 || length_of(name, NAME) >= (u32)NAME) return STORE_MASTER;
        char key[NAME];
        copy_text(key, name, NAME);
        if (!plain_key(key) || name_of_the_unit(key)) return STORE_MASTER;
        int at = name_index(key);
        if (at >= 0) {
            if (names[at].owner == 0) names[at].files |= OP1F_KEEP_MASTER;
            continue;
        }
        if (names_count >= (u32)NAMES_ROOM) continue;                               // no room: as for the slots' names
        Name* n = &names[names_count++];
        copy_text(n->name, key, NAME);
        n->files = OP1F_KEEP_MASTER;
        n->owner = 0;
    }
    return STORE_READ;
}

// R5: the master's values of the store made into `h` (no mark: the master is no sound loaded from a file); 0, or why
// not.
static u32 store_master(const void* document, Holder* h) {
    holder_clear(h);
    const void* section = member(document, "master");
    if (section == 0) return STORE_READ;
    const void* values = member(section, "values");
    if (values == 0 || kind_of(values) != (u32)KIND_OBJECT) return STORE_MASTER;
    for (u32 n = 0; n < names_count; ++n) {
        if ((names[n].files & (u32)OP1F_KEEP_MASTER) == 0) continue;
        const void* v = member(values, names[n].name);
        if (v == 0) continue;
        Out o = entry_room(h);
        if (from_value(v, &o, 0) != 0 || !entry_close(h, n, &o)) return STORE_MASTER;
    }
    return STORE_READ;
}

// R17 (base 39): the kept cut of the store's slot row `i`, synth slot `index`, whose values `h` were just kept: 1 taken,
// 0 none there, 2 one not as this unit writes it (or with no ride beside it), left aside: the slot's values stay kept.
static u32 sample_take(const void* document, u32 i, u32 index, const Holder* h) {
    sample_frames[index] = 0;
    sample_stereo[index] = 0;
    const void* v = member(STOCK_JSON_ELEMENT(member(document, "slots"), i), SAMPLE_KEY);
    if (v == 0) return 0;
    i32 frames = 0, stereo = -1;
    if (entry_of(h, BASE_ENGINE) < 0 || kind_of(v) != (u32)KIND_LIST || STOCK_JSON_SIZE(v) != 2u || !whole_of(STOCK_JSON_ELEMENT(v, 0), &frames) ||
        !whole_of(STOCK_JSON_ELEMENT(v, 1), &stereo) || (stereo != 0 && stereo != 1) || frames <= 0 || (u32)frames > ((u32)SAMPLE_POOL >> (u32)stereo))
        return 2;
    sample_frames[index] = (u32)frames;
    sample_stereo[index] = (u32)stereo;
    return 1;
}

// ---------------------------------------------------------------- once a frame (the display thread)

static Mark now_mark;                                                              // the display's: the mark of a record read at the first frame

static void first_frame() {
    enrol();
    u32 asked = names_count;
    Json document;
    STOCK_JSON_MAKE(&document, KIND_NULL);
    u32 state = store_names(&document);
    u32 count = state == (u32)STORE_READ ? STOCK_JSON_SIZE(member(&document, "slots")) : 0u;
    u32 seen = 0;                                                                   // the slots the file names: each once
    for (u32 i = 0; i < count && state == (u32)STORE_READ; ++i) {                   // first the whole file held to its form, nothing kept
        u32 index = 0;
        state = store_slot(&document, i, &display_copy, &index);
        if (state == (u32)STORE_READ && ((seen >> index) & 1u)) state = STORE_SLOT;
        seen |= 1u << index;
    }
    if (state == (u32)STORE_READ) state = store_master_names(&document);             // R5: the master's section held to its form too
    if (state == (u32)STORE_READ) state = store_master(&document, &display_copy);
    u32 kept = 0, dropped = 0, cuts_taken = 0, cuts_aside = 0;
    if (state == (u32)STORE_READ) {
        for (u32 i = 0; i < count; ++i) {                                           // then each slot, held to the sound on it now
            u32 index = 0;
            Holder* s = &display_copy;
            store_slot(&document, i, s, &index);
            if (s->bytes == 0) continue;
            mark_of_record(index >= (u32)SLOTS ? 1u : 0u, index & (u32)(SLOTS - 1), &now_mark);
            if (same(now_mark.mark_name, s->mark_name) && same(now_mark.mark_type, s->mark_type)) {
                ++kept;
                put_in(index, s, OP1F_KEPT_START, 0);
                u32 cut = index < (u32)SLOTS ? sample_take(&document, i, index, s) : 0u;   // R17: the kept cut beside the ride
                if (cut) ++(cut == 1u ? cuts_taken : cuts_aside);
            } else {
                ++dropped;
                tell(index, OP1F_KEPT_DROPPED);
                store_changed();
            }
        }
        if (kept || dropped)
            op1f_log(3, LOG_FILE, 4, "keeps: %u slot(s) of sounds with values read from the base's file, %u let go: their sound changed", kept, dropped);
        if (cuts_taken || cuts_aside)                                               // R17
            op1f_log(3, LOG_FILE, 17, "keeps: the kept cut of %u synth sound(s) read beside its ride, %u not as written: left aside", cuts_taken, cuts_aside);
        store_master(&document, &display_copy);                                    // R5: the master's values, kept whatever sound plays
        if (display_copy.bytes != 0) put_in(MASTER, &display_copy, OP1F_KEPT_START, 0);
        unit.store_known = 1;
    } else if (state != (u32)STORE_NONE) {
        names_count = asked;                                                        // the names of this start's asks stand, none remembered
        op1f_log(1, LOG_FILE, 5, "keeps: the base's file of the slots' values could not be read as written (%u): none kept", state);
    }
    STOCK_JSON_DESTROY(&document);
    BARRIER();
    ready = 1;
    BARRIER();
    if (tape_early) {                                                               // the tape the start loaded, before the names were whole
        tape_early = 0;
        tape_take(&display_copy, tape_early_flag);
    }
}

static const char* holder_words(u32 index) {
    return index == (u32)MASTER ? "the master" : index == (u32)TAPE ? "the tape" : index >= (u32)SLOTS ? "drum sound" : "synth sound";
}

static void say_queue() {
    for (;;) {
        u32 mask;
        Said s = { 0, 0, 0, 0, 0 };
        MASK_OFF(mask);
        u32 out = queue_out;
        int none = out == queue_in;
        if (!none) {
            s = queue[out & (u32)(QUEUE - 1)];
            queue_out = out + 1u;
        }
        MASK_ON(mask);
        if (none) break;
        if (s.what == (u32)Q_TAPE_LOADED) {
            if (unit.loads_said >= (u32)WRITTEN_SAID) continue;
            ++unit.loads_said;
            op1f_log(3, LOG_FILE, 14, "keeps: the tape loaded as a new tape or a copy (the firmware's flag %u): %u value(s) of packs taken from its file", s.bytes,
                     s.name);
            continue;
        }
        if (s.what == (u32)Q_CUT) {
            if (unit.cuts_said >= (u32)WRITTEN_SAID) continue;
            ++unit.cuts_said;
            op1f_log(3, LOG_FILE, 15, "keeps: a tape was loaded during the tape's save: %u value(s) of the tape before not written, into either tape", s.bytes);
            continue;
        }
        if (s.name >= names_count) continue;
        const char* key = names[s.name].name;
        const char* pack = pack_name(names[s.name].owner);
        u32 slot = (s.holder & (u32)(SLOTS - 1)) + 1u;
        if (s.what == (u32)Q_STRAY) {
            if (unit.cuts_said >= (u32)WRITTEN_SAID) continue;
            ++unit.cuts_said;
            op1f_log(2, LOG_FILE, 16, "keeps: '%s' of '%s' of the tape before may have reached the tape loaded during its save: %s", key, pack,
                     s.why == (u32)STRAY_PUT_BACK ? "that tape's own value set again over it" : "that tape holds no value of that name, so it stays there");
            continue;
        }
        if (s.what == (u32)Q_WRITTEN) {
            if (unit.written_said >= (u32)WRITTEN_SAID) continue;
            ++unit.written_said;
            if (s.holder == (u32)TAPE) op1f_log(3, LOG_FILE, 12, "keeps: '%s' of '%s' written into the tape (%u bytes)", key, pack, s.bytes);
            else op1f_log(3, LOG_FILE, 6, "keeps: '%s' of '%s' written into %s %u (%u bytes)", key, pack, holder_words(s.holder), slot, s.bytes);
        } else if (s.what == (u32)Q_REFUSED) {
            const char* why = s.why == (u32)WHY_FIRMWARE ? "the firmware's own file has a member of that name" : "it could not be made";
            if (s.holder == (u32)TAPE) op1f_log(1, LOG_FILE, 13, "keeps: '%s' of '%s' not written into the tape: %s", key, pack, why);
            else op1f_log(1, LOG_FILE, 7, "keeps: '%s' of '%s' not written into %s %u: %s", key, pack, holder_words(s.holder), slot, why);
        } else {
            const char* why = s.why == (u32)WHY_KIND ? "it holds a kind of value the base cannot keep" : s.why == (u32)WHY_LONG ? "its text would be longer than 2048 bytes"
                                                                                                  : "the values would take more than 4 KiB";
            op1f_log(2, LOG_FILE, 10, "keeps: '%s' of '%s' was not taken from a file: %s", key, pack, why);
        }
    }
    u32 lost = queue_lost;
    if (lost) {
        u32 mask;
        MASK_OFF(mask);
        queue_lost = 0;
        MASK_ON(mask);
        op1f_log(2, LOG_FILE, 11, "keeps: %u more line(s) of the files' threads left unsaid", lost);
    }
}

// The cards told what changed, each whose ask keeps a value there, in the cards' order. R5: the effects unit first, of
// its member at the slots and the master (op1f_effects_changed, the base's own code: no record of a pack's call).
static void tell_cards(u32 index, u32 how) {
    u32 where = files_of_holder(index);
    u32 which = which_of_holder(index);
    if ((names[BASE_NAME].files & where) != 0 && EFFECTS_CHANGED) EFFECTS_CHANGED(where, which, how);
    if ((names[BASE_ENGINE].files & where) != 0 && &op1f_engines_changed != 0) op1f_engines_changed(where, which, how);   // R6
    if (where == (u32)OP1F_KEEP_DRUM && (names[BASE_ENGINE].files & where) != 0 && &op1f_drum_changed != 0) op1f_drum_changed(where, which, how);   // R21 (base 43)
    if ((names[BASE_LFO].files & where) != 0 && &op1f_lfos_changed != 0) op1f_lfos_changed(where, which, how);   // R11
    for (u32 k = 0; k < keeps_count; ++k) {
        if ((keeps[k].files & where) == 0 || keeps[k].table->changed == 0) continue;
        Op1fCallKept kept;
        op1f_cards_call_in(CALL_SHOWN, keeps[k].slot, &kept);
        keeps[k].table->changed(where, which, how);
        op1f_cards_call_out(CALL_SHOWN, &kept);
    }
}

// R6b: a mark copied into a holder or a mark alone, field by field.
template <typename Marked> static void mark_copy(Marked* to, const Mark* from) {
    to->marked = from->marked;
    for (u32 i = 0; i < (u32)MARK; ++i) {
        to->mark_name[i] = from->mark_name[i];
        to->mark_type[i] = from->mark_type[i];
    }
}

// R6b (M4): from op1f_keeps_frame (the display thread): each slot whose save's mark waits, held to the slot's record;
// the mark taken when the record shows it (the store written again when the slot holds values), dropped past the wait.
static void remarks_frame() {
    if (!remarks_any) return;
    u32 waiting_still = 0;
    for (u32 index = 0; index < (u32)TAPE; ++index) {
        u32 mask;
        MASK_OFF(mask);
        u32 due = remarks[index].due;
        if (due) {
            remark_copy.due = due;
            remark_copy.since = remarks[index].since;
            mark_copy(&remark_copy.mark, &remarks[index].mark);
        }
        MASK_ON(mask);
        if (!due) continue;
        mark_of_record(index >= (u32)SLOTS ? 1u : 0u, index & (u32)(SLOTS - 1), &now_mark);
        int shown = same(now_mark.mark_name, remark_copy.mark.mark_name) && same(now_mark.mark_type, remark_copy.mark.mark_type);
        int late = cycles_now() - remark_copy.since >= REMARK_WAIT;
        if (!shown && !late) {
            ++waiting_still;
            continue;
        }
        u32 changed = 0;
        MASK_OFF(mask);
        if (remarks[index].due == due) {                                            // no save, load or change of engine since the look
            remarks[index].due = 0;
            Holder* h = &holders[index];
            if (shown) {
                changed = !h->marked || !same(h->mark_name, remark_copy.mark.mark_name) || !same(h->mark_type, remark_copy.mark.mark_type);
                if (changed) {
                    mark_copy(h, &remark_copy.mark);
                    h->gen = h->gen + 1u;
                }
                changed = changed && h->bytes != 0;
            }
        }
        MASK_ON(mask);
        if (changed) store_changed();
    }
    if (waiting_still) return;
    u32 mask;
    MASK_OFF(mask);
    u32 any = 0;
    for (u32 index = 0; index < (u32)TAPE; ++index) any |= remarks[index].due;
    remarks_any = any ? 1u : 0u;                                                    // a save since the loop keeps it set
    MASK_ON(mask);
}

// Called by the cards unit once a frame, on the display thread, before the cards' frame functions.
extern "C" void op1f_keeps_frame() OP1F_NAMED("op1f_keeps_frame");
extern "C" void op1f_keeps_frame() {
    if (!unit.started) {
        unit.started = 1;
        first_frame();
    }
    say_queue();
    for (u32 index = 0; index < (u32)HOLDERS; ++index) {
        u32 mask;
        MASK_OFF(mask);
        u32 bits = told[index];
        u32 last = tape_last;
        told[index] = 0;
        MASK_ON(mask);
        if (!bits) continue;
        if (index != (u32)TAPE && (bits & 1u << TOLD_STORE)) store_changed();
        // The tape's loads in OP1F_KEPT_LOADED's place: a tape switched to, or one just made (OP1F_KEPT_MADE), or both
        // since the last frame, the earlier first and the later, whose values the base holds now, last.
        const u32 BOTH = 1u << OP1F_KEPT_LOADED | 1u << OP1F_KEPT_MADE;
        for (u32 how = OP1F_KEPT_LOADED; how <= (u32)OP1F_KEPT_DROPPED; ++how) {
            if (how == (u32)OP1F_KEPT_LOADED) {
                u32 loads = bits & BOTH;
                if (loads == BOTH) {
                    tell_cards(index, last == (u32)OP1F_KEPT_MADE ? (u32)OP1F_KEPT_LOADED : (u32)OP1F_KEPT_MADE);
                    tell_cards(index, last == (u32)OP1F_KEPT_MADE ? (u32)OP1F_KEPT_MADE : (u32)OP1F_KEPT_LOADED);
                } else if (loads) {
                    tell_cards(index, loads == (1u << OP1F_KEPT_MADE) ? (u32)OP1F_KEPT_MADE : (u32)OP1F_KEPT_LOADED);
                }
                continue;
            }
            if (bits & (1u << how)) tell_cards(index, how);
        }
    }
    remarks_frame();                                                                // R6b (M4)
    if (unit.store_due && cycles_now() - unit.store_since >= HALF_SECOND) store_write();
}

// ---------------------------------------------------------------- the helpers, from a pack's frame function

static int slot_of(const Op1fCard* card) {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    if (card == 0) return -1;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards_2.slots[i].card == card) return op1f_cards_verdict(i) == 0 ? (int)i : -1;
    return -1;
}

// The holder and the name a call names, or less than 0: OP1F_NOT_NOW, OP1F_BAD_NAME.
static int named(const Op1fCard* card, unsigned where, unsigned which, const char* key, u32* index, u32* name) {
    int slot = slot_of(card);
    if (slot < 0 || !ready || op1f_cards_calling() != (u32)slot + 1u) return OP1F_NOT_NOW;
    if (where == (unsigned)OP1F_KEEP_TAPE) {
        if (which != 0) return OP1F_BAD_NAME;
        *index = TAPE;
    } else if (where == (unsigned)OP1F_KEEP_SYNTH || where == (unsigned)OP1F_KEEP_DRUM) {
        if (which >= (unsigned)SLOTS) return OP1F_BAD_NAME;
        *index = (where == (unsigned)OP1F_KEEP_DRUM ? (u32)SLOTS : 0u) + which;
    } else if (where == (unsigned)OP1F_KEEP_MASTER) {                               // R5: only a key of a card of level 8 has the master
        if (which != 0) return OP1F_BAD_NAME;
        *index = MASTER;
    } else {
        return OP1F_BAD_NAME;
    }
    if (key == 0) return OP1F_BAD_NAME;
    u32 n = length_of(key, NAME);
    if (n >= (u32)NAME) return OP1F_BAD_NAME;
    for (u32 i = 0; i < names_count; ++i) {
        if (names[i].owner == (u32)slot + 1u && (names[i].files & where) != 0 && same(names[i].name, key)) {
            *name = i;
            return 0;
        }
    }
    return OP1F_BAD_NAME;
}

// The text of the value of `name` in holder `index` (the display thread): a pack's, or the base's own (R5).
static int value_get_at(u32 index, u32 name, char* out, unsigned room) {
    Holder* c = &display_copy;
    take_out(index, c);
    int at = entry_of(c, name);
    if (at < 0) return OP1F_NO_FILE;
    u32 len = get_half(c->data + at + 1);
    Text count = { 0, 0, room, 0 };                                                 // its bytes counted first: nothing written when the room is short
    to_text(c->data + at + 3, len, &count);
    if (out == 0 || room == 0 || count.full) return OP1F_TOO_LARGE;
    Text t = { out, 0, room, 0 };
    to_text(c->data + at + 3, len, &t);
    out[t.used] = 0;
    return (int)t.used;
}

extern "C" int op1f_value_get(const Op1fCard* card, unsigned where, unsigned which, const char* key, char* out, unsigned room) {
    u32 index = 0, name = 0;
    int why = named(card, where, which, key, &index, &name);
    if (why) return why;
    return value_get_at(index, name, out, room);
}

// Whether the shared holder `index` already holds, under `name`, exactly the tokens at `at` (`len` bytes): read inside
// one mask, so that a value set again unchanged asks for no save.
static int holds_already(u32 index, u32 name, const u8* at, u32 len) {
    u32 mask;
    MASK_OFF(mask);
    const Holder* h = &holders[index];
    int e = entry_of(h, name);
    int same_value = e >= 0 && get_half(h->data + e + 1) == len;
    for (u32 i = 0; same_value && i < len; ++i)
        if (h->data[e + 3 + i] != at[i]) same_value = 0;
    MASK_ON(mask);
    return same_value;
}

// A value of the tape set: the firmware writes the tape's members only in its save (0x8c0c116c, on the background
// worker), and each of its own tape values asks for that save when it changes (the saver's getter, then the notify:
// the saver marked, the worker woken). The base asks the same way for a pack's value, after it is in the holder and
// outside every mask, once a tape has been loaded; the save then puts it into the document through the place T1, so
// that it is there whenever the firmware writes the tape's file (the switch's eject, a copy). A value set again
// unchanged asks for nothing, and a value taken away writes nothing (the base never removes a member).
// While a tape load waits to be told to the packs (it came after this frame's op1f_keeps_frame, on the key's thread), a
// value of the tape is refused with OP1F_NOT_NOW, by put_in, inside the mask of the write: nothing is set, nothing is
// asked of the tape's save, and the pack is told the load at the next frame before its frame function.
// R5: the body of a pack's set (`when` 1) and of the base's own member's (SET_BASE), the holder and the name found.
static int value_set_at(u32 index, u32 name, const char* text, int when) {
    u32 len = text ? length_of(text, VALUE_MOST) : 0u;
    if (len > (u32)VALUE_MOST) return OP1F_TOO_LARGE;
    for (u32 tries = 0; tries < 4u; ++tries) {
        Holder* c = &display_copy;
        take_out(index, c);
        entry_remove(c, name);
        int ask_save = 0;
        if (text) {
            Out o = entry_room(c);
            u32 n = parse(text, len, &o);
            if (n == 0) return o.full ? (int)OP1F_TOO_LARGE : (int)OP1F_BAD_VALUE;
            if (!entry_close(c, name, &o)) return OP1F_TOO_LARGE;
            ask_save = index == (u32)TAPE && !holds_already(index, name, o.at, o.used);
        }
        if (!c->marked && index < (u32)TAPE) mark_of_record(index >= (u32)SLOTS ? 1u : 0u, index & (u32)(SLOTS - 1), c);   // R5: the master has no record
        int in = put_in(index, c, 0, when);
        if (in < 0) return OP1F_NOT_NOW;                                            // a tape load not yet told: at the next frame
        if (in > 0) {
            if (index != (u32)TAPE) store_changed();
            else if (ask_save && tape_seen) STOCK_NOTIFY(STOCK_TAPE_SAVER());
            return 0;
        }
    }
    return OP1F_NOT_NOW;                                                            // another thread wrote the slot four times meanwhile
}

extern "C" int op1f_value_set(const Op1fCard* card, unsigned where, unsigned which, const char* key, const char* text) {
    u32 index = 0, name = 0;
    int why = named(card, where, which, key, &index, &name);
    if (why) return why;
    return value_set_at(index, name, text, 1);
}

// ---------------------------------------------------------------- R5: the base's own member, for the effects unit

// The holder of the base's own member at a place: a slot of the synth or of the drum, or the master. 0 for no place.
static int base_holder(unsigned where, unsigned which, u32* index) {
    if (where == (unsigned)OP1F_KEEP_SYNTH || where == (unsigned)OP1F_KEEP_DRUM) {
        if (which >= (unsigned)SLOTS) return 0;
        *index = (where == (unsigned)OP1F_KEEP_DRUM ? (u32)SLOTS : 0u) + which;
        return 1;
    }
    if (where == (unsigned)OP1F_KEEP_MASTER && which == 0) {
        *index = MASTER;
        return 1;
    }
    return 0;
}

// The member's JSON text at a place (module/build_r5_base.h), from the display thread: its bytes, or OP1F_NO_FILE (none
// there, or the first frame not made yet), OP1F_TOO_LARGE (`room` too small: nothing written), OP1F_BAD_NAME (no place
// the member is kept at).
extern "C" int op1f_keeps_base_get(unsigned where, unsigned which, char* out, unsigned room) {
    u32 index = 0;
    if (!base_holder(where, which, &index)) return OP1F_BAD_NAME;
    if (!ready) return OP1F_NO_FILE;
    return value_get_at(index, BASE_NAME, out, room);
}

// The member set, or taken away (`text` 0), from the display thread: 0, OP1F_BAD_VALUE, OP1F_TOO_LARGE, OP1F_NOT_NOW
// (the first frame not made yet; a sound loaded onto the slot waits to be told; another thread wrote the slot four
// times meanwhile), OP1F_BAD_NAME (no place the member is kept at). Written at the unit's next save of the sound and
// kept in the store, as a pack's value is; the master's in the store alone.
extern "C" int op1f_keeps_base_set(unsigned where, unsigned which, const char* text) {
    u32 index = 0;
    if (!base_holder(where, which, &index)) return OP1F_BAD_NAME;
    if (!ready) return OP1F_NOT_NOW;
    return value_set_at(index, BASE_NAME, text, SET_BASE);
}

// R6: the base's second own member, op1f_engine, at a synth slot (module/build_r6_base.h), from the display thread: as
// op1f_keeps_base_get and op1f_keeps_base_set for the synth's sounds.
extern "C" int op1f_keeps_engine_get(unsigned slot, char* out, unsigned room) {
    u32 index = 0;
    if (!base_holder((unsigned)OP1F_KEEP_SYNTH, slot, &index)) return OP1F_BAD_NAME;
    if (!ready) return OP1F_NO_FILE;
    return value_get_at(index, BASE_ENGINE, out, room);
}

extern "C" int op1f_keeps_engine_set(unsigned slot, const char* text) {
    u32 index = 0;
    if (!base_holder((unsigned)OP1F_KEEP_SYNTH, slot, &index)) return OP1F_BAD_NAME;
    if (ready == 0) return OP1F_NOT_NOW;                                            // before the first frame, as the effects' member
    return value_set_at(index, BASE_ENGINE, text, SET_BASE);
}

// R21 (base 43): the same member at a drum slot, the drum engine a drum sound rides (module/build_r21_drum.cpp), from
// the display thread: as op1f_keeps_engine_get and op1f_keeps_engine_set, with the drum's holder.
extern "C" int op1f_keeps_drum_engine_get(unsigned slot, char* out, unsigned room) OP1F_NAMED("op1f_keeps_drum_engine_get");
extern "C" int op1f_keeps_drum_engine_set(unsigned slot, const char* text) OP1F_NAMED("op1f_keeps_drum_engine_set");
extern "C" int op1f_keeps_drum_engine_get(unsigned slot, char* out, unsigned room) {
    u32 index = 0;
    if (!base_holder((unsigned)OP1F_KEEP_DRUM, slot, &index)) return OP1F_BAD_NAME;
    if (!ready) return OP1F_NO_FILE;
    return value_get_at(index, BASE_ENGINE, out, room);
}

extern "C" int op1f_keeps_drum_engine_set(unsigned slot, const char* text) {
    u32 index = 0;
    if (!base_holder((unsigned)OP1F_KEEP_DRUM, slot, &index)) return OP1F_BAD_NAME;
    if (ready == 0) return OP1F_NOT_NOW;
    return value_set_at(index, BASE_ENGINE, text, SET_BASE);
}

// R11: the base's third own member, op1f_lfo, at a synth slot (where OP1F_KEEP_SYNTH) or a drum slot (OP1F_KEEP_DRUM),
// from the display thread: as op1f_keeps_base_get and op1f_keeps_base_set, but never the master (OP1F_BAD_NAME).
// Declared with the names the LFO unit calls them by (module/build_r11_lfos.cpp), as build_r6_base.h declares the
// engine's pair: without it the unit's compiler names them with a leading underscore and the base does not link.
extern "C" int op1f_keeps_lfo_get(unsigned where, unsigned which, char* out, unsigned room) OP1F_NAMED("op1f_keeps_lfo_get");
extern "C" int op1f_keeps_lfo_set(unsigned where, unsigned which, const char* text) OP1F_NAMED("op1f_keeps_lfo_set");
extern "C" int op1f_keeps_lfo_get(unsigned where, unsigned which, char* out, unsigned room) {
    u32 index = 0;
    if (where == (unsigned)OP1F_KEEP_MASTER || !base_holder(where, which, &index)) return OP1F_BAD_NAME;
    if (!ready) return OP1F_NO_FILE;
    return value_get_at(index, BASE_LFO, out, room);
}

extern "C" int op1f_keeps_lfo_set(unsigned where, unsigned which, const char* text) {
    u32 index = 0;
    if (where == (unsigned)OP1F_KEEP_MASTER || !base_holder(where, which, &index)) return OP1F_BAD_NAME;
    if (ready == 0) return OP1F_NOT_NOW;
    return value_set_at(index, BASE_LFO, text, SET_BASE);
}

// ---------------------------------------------------------------- R17: the kept cut, for the restore unit

// From the display thread (module/build_r17_restore.cpp). The kept cut of synth slot `slot`: 1 with its frames and
// channels, 0 none, -1 before the first frame read the store.
extern "C" int op1f_keeps_sample_get(unsigned slot, unsigned* frames, unsigned* stereo) OP1F_NAMED("op1f_keeps_sample_get");
extern "C" int op1f_keeps_sample_get(unsigned slot, unsigned* frames, unsigned* stereo) {
    if (slot >= (unsigned)SLOTS) return 0;
    if (!ready) return -1;
    if (frames) *frames = sample_frames[slot];
    if (stereo) *stereo = sample_stereo[slot];
    return sample_frames[slot] != 0 ? 1 : 0;
}

// The kept cut set (a drop's end), or ended (`frames` 0: a sound loaded there): the store written as for a value, half a
// second on, never while the disk mode is open; a cut that passes the pool at its channels is kept as none.
extern "C" void op1f_keeps_sample_set(unsigned slot, unsigned frames, unsigned stereo) OP1F_NAMED("op1f_keeps_sample_set");
extern "C" void op1f_keeps_sample_set(unsigned slot, unsigned frames, unsigned stereo) {
    if (slot >= (unsigned)SLOTS) return;
    u32 s = stereo ? 1u : 0u;
    u32 f = frames > ((u32)SAMPLE_POOL >> s) ? 0u : frames;
    if (f == 0) s = 0;
    if (sample_frames[slot] == f && sample_stereo[slot] == s) return;
    sample_frames[slot] = f;
    sample_stereo[slot] = s;
    store_changed();
}

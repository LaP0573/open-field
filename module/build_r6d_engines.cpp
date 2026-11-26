// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R6d, base 30, the release's base: the engines unit's sibling. A sibling of base 29's engines unit,
// which stays as it was: everything of it is here as it was, its stubs are still module/build_r6_engines.s, and these
// are added or changed:
//   the sounds within the firmware's limit: before an engine's sounds are written into the unit's sound list, the
//       unit's user sounds are counted by the firmware's own count, the one its save asks before it saves (synth and
//       drum together; 500 at most, 100 on a unit that holds content/op1.classic); the engine's missing sounds are
//       written only when at least 20 stay free for the owner's own saves after them, all of them or none, with a line
//       either way. The count is said at each start an engine is registered. The base never takes a sound off the disk
//   a value as heard (level 10): each synth slot's values as its voices last rendered them, the LFOs applied, kept by
//       the audio thread between two steps of a sequence word, for op1f_param_heard (module/build_r6d_effects.cpp)
//       through op1f_engines_param_heard; the value as set where no voice of the slot rendered in the last few blocks
//   a sound loaded onto a slot: its member is read once the slot's build has come (while `settling` holds it), so that
//       a ride is never let go on the type word of the sound that was there before (base 29's review, F-1)
//   the lines count synth sounds from 1, as every other unit and the checklists do (base 29's review, F-2)
// What base 29's unit said follows and holds.
//
// Step R6, the base pack of level 9: synth engines (OS 1.7.0+0). A unit of its own, new at R6.
//
// A pack adds a synth engine through an ask of its card (OP1F_ASK_ENGINE, its table an Op1fEngine,
// module/build_r6_card.h). Every engine rides the unit's own engine dsynth: the unit keeps dsynth in the sound and in
// its files (its name, its eight knob words), and the base makes the pack's engine sound in its place. The unit's
// voices, envelope, velocity, play mode, glide, mix, effect, LFOs, knobs, MIDI and files stay the unit's own. What the
// units of the base share is in module/build_r6_base.h; the two places of the firmware this unit stands at are named
// there, E1 and E2, each read in both images.
//
// What this unit does, by where it runs:
//   the catalog's read (the start's thread, op1f_engines_catalog_read, called by the catalog's unit): the first call
//       checks every ask of kind OP1F_ASK_ENGINE of the cards in use and registers those in order, ids 32 and on; every
//       call adds the registered engines' rows to the document's `synth_types`. No line: the lines come at the first
//       frame
//   a synth engine built (E1, op1f_engine_built, from the stub after the firmware's factory has returned; the audio
//       thread, the key thread or the start's): when the sound the owner sees rides an engine and the six words hold the
//       firmware's dsynth, the base's six voice objects take their places
//   a block (the audio thread, the base's voice objects): the pack's `render` for a voice whose sound rides a
//       registered engine with its memory, silence for every other; a note's start and release only counted
//   a pick in the engine list or by MIDI (E2, op1f_engine_pick_do, the key thread or MIDI's): a ride begun or ended,
//       then the firmware's own pick (an engine of a pack's is picked as the firmware's dsynth)
//   a sound put on a slot (op1f_engines_assign, from the values unit at F4, the background worker): a loaded sound's
//       ride set from its member before the unit builds it; the engine job's sound named after the engine
//   a frame (the display thread, op1f_engines_frame, from the cards unit): the lines, the start values after a pick,
//       the rides held against the type words, the member written and read through the values unit, the voices'
//       memory, `reset` calls, the swap from the display thread, the engines' own sounds written, the period's lines
//   the helpers op1f_param_get, op1f_param_set and op1f_riding for kind 8, through op1f_engines_param_get and its two
//       neighbours; the names the LFO pages show for kind 0 (op1f_engines_names, which level 8's W4 asks); the page's
//       standing and its answer ahead (op1f_engines_standing, op1f_engines_ahead)
//
// What the firmware needs of these, where the code meets it (the readings name each address in both firmwares):
//   the synth's six voice objects are six words; the factory fills them for the type it is handed and the slot the
//       owner sees; an object's first word is its table, and the firmware calls through it only: +0x8 render (object,
//       out, the eight knob words of the block), +0xc start and +0x14 release (object, the slot's knob words), +0x1c
//       the stereo answer (object), +0x20, +0x24, +0x28 and +0x10, +0x18 answered by dsynth's table words, which the
//       base copies, and two destructors no code calls
//   a slot's type word (dsynth is 12), its eight knob words (a value shifted left by sixteen), the slot the owner sees,
//       the slot built, the instrument whose block runs, the pitch and the velocity words of a voice's block
//   dsynth's knob rows: eight bytes a knob, the lowest (signed halfword), the highest (unsigned halfword)
//   the knob setter (record, words) puts the eight words into the record's document as the knobs do
//   the catalog's rows are jsoncpp values of twelve bytes, the kind byte at +4 (1 an integer, 2 an unsigned one, 4 a
//       string, 6 an array, 7 an object); each function is handed only the kinds it takes
//
// The threads. The registration on the thread of the start, before the first build; a build on the audio thread, the
// key thread or the start's; a block on the audio thread; a note's start on the note's thread or inside the block; a
// pick on the key thread or MIDI's; a sound's assign on the background worker; the frame and the helpers' work on the
// display thread. What other threads write is single words, or a few words inside a short mask. Nothing here says a
// line but from the display thread; nothing waits; nothing is divided by a number that is not a power of two (the
// module links no runtime) and nothing is multiplied in 64 bits. With no engine pack and no sound naming one, nothing of
// this unit runs on the audio thread, and the frame tests one word. Compiled for the PC as it is, with OP1F_HOST
// defined: the stand-ins come in through a header of the PC's own (for tests this tree does not hold).

#include "build_r6d_base.h"                             // R6d: level 9's names, and level 10's
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"     // the gate's half for the power popup: this unit is handed no frame to see it by
#endif
#include "build_q_quiet.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef void (*PickFn)(u32 id, u32 slot);
typedef void (*KnobSetFn)(u32 record, const u32* words);
typedef u32 (*ScreenOfFn)(u32 type);
typedef const void* (*JsonMemberFn)(const void* value, const char* key);
typedef u32 (*JsonSizeFn)(const void* value);
typedef const void* (*JsonElementFn)(const void* value, u32 index);
typedef void (*JsonMakeFn)(void* value, int kind);
typedef void* (*JsonPlaceFn)(void* value, const char* key);
typedef void* (*JsonAppendFn)(void* array, const void* value);
typedef void* (*JsonAssignFn)(void* to, const void* from);
typedef void (*JsonDestroyFn)(void* value);
typedef void (*JsonTextFn)(void* value, const char* text);
typedef void (*JsonNumberFn)(void* value, int number);
typedef int (*JsonIntFn)(const void* value);
typedef int (*FileSizeFn)(const char* path);
typedef int (*IsFolderFn)(const char* path);
typedef int (*MakeFoldersFn)(const char* path);
typedef int (*OpenFn)(const char* path, const char* mode);
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);
typedef int (*WriteFn)(const void* data, u32 size, u32 count, int handle);
typedef void (*CloseFn)(int handle);
typedef int (*RenameFn)(const char* from, const char* to);
typedef int (*CountFn)();                              // R6d
typedef u32 (*ClassicFn)();

// The room of the pick's entry (module/build_r6_engines.s): the firmware's pick as it always was. The factory's room is
// the stub's alone.
extern "C" u8 op1f_engine_pick_room[] OP1F_NAMED("op1f_engine_pick_room");

#ifdef OP1F_HOST
#define OP1F_ENGINES_R6_UNIT
#include "mock_engines_r6d.h"                           // the PC's stand-ins: the firmware's memory and calls, the masks, the clock (R6d: and the count)
#define ENGINES_APART
#else
#define RD32(a) (*(volatile u32*)(a))
#define RD16(a) (*(volatile u16*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
// The processor's cycle counter, read with a memory clobber so that the compiler moves no load or store of the call it
// times across the stamp.
static inline u32 cycles_now() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
#define STOCK_KNOB_SET ((KnobSetFn)0x8d4f689cu)
#define STOCK_ENGINE_SCREEN ((ScreenOfFn)0x8fc274acu)
#define STOCK_JSON_MEMBER ((JsonMemberFn)0x8c084f74u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_ELEMENT ((JsonElementFn)0x8c084ee0u)
#define STOCK_JSON_MAKE ((JsonMakeFn)0x8c085ba8u)
#define STOCK_JSON_PLACE ((JsonPlaceFn)0x8c085e84u)
#define STOCK_JSON_APPEND ((JsonAppendFn)0x8c085d54u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DESTROY ((JsonDestroyFn)0x8c0844e0u)
#define STOCK_JSON_TEXT ((JsonTextFn)0x8c084430u)
#define STOCK_JSON_NUMBER ((JsonNumberFn)0x8c083b1cu)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)             // -1 when there is no file
#define STOCK_IS_FOLDER ((IsFolderFn)0x8c052054u)             // 1 when the path opens as a folder
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)                      // a handle, -1 when it cannot be opened
#define STOCK_READ ((ReadFn)0x8fcba954u)
#define STOCK_WRITE ((WriteFn)0x8fcba8c0u)
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)
#define STOCK_RENAME ((RenameFn)0x8c051f88u)                  // 1 when it was made
// R6d: the firmware's count of the user sounds, synth and drum together (each total less its factory count), the one
// its own save asks (its caller, the check before a save, holds its answer against the limit); it stops the unit when
// either total is 0, which is why the base reads the two totals first. And whether the unit holds content/op1.classic
// (the same caller takes 100 sounds then, else 500), a byte the firmware keeps once it has looked.
#define STOCK_USER_SOUNDS ((CountFn)0x8d4fee88u)
#define STOCK_CLASSIC ((ClassicFn)0x8fca2220u)
#define ROOM_PICK ((PickFn)(void*)op1f_engine_pick_room)
#define ENGINES_APART __attribute__((noinline))
#endif

// The firmware's words (an enum of their own: they do not fit a signed number).
enum : u32 {
    GUI_SLOT = 0x11904704u,             // the synth slot the owner sees
    TYPE_WORDS = 0x11904714u,           // a synth slot's engine type, a word a slot
    BUILT_SLOT = 0x84a01524u,           // the slot the synth's voices were built for
    SIX_WORDS = 0x84a03c4cu,            // the synth's six voice objects
    KNOB_WORDS = 0x119035e0u,           // a slot's eight knob words, 32 bytes a slot
    VELOCITY = 0x11903c00u,             // the velocity of the voice whose block it is, written before its render
    PITCH = 0x11903c04u,                // the phase step of that voice, 128 times the same word: the first is read
    PLAYING = 0x11902da4u,              // the instrument whose block runs: 0 the synth
    CURRENT_MODE = 0x8400f978u,         // the mode: 0 synth, 1 drum, 2 tape, 3 mixer
    DSYNTH_TABLE = 0x848cc9acu,         // dsynth's voice objects' table
    KNOB_ROWS = 0x840afa60u,            // the engines' knob rows: 64 bytes a type, 8 a knob
    SYNTH_RECORDS = 0x080235dcu,        // a synth slot's record, 52 bytes a slot
    PERIOD = 0x84d20e80u,               // samples a 96th of a beat at 44,100 a second, times 2^18
    TRANSPORT = 0x11905a04u,            // not 0 while the tape plays
    DISK_OPEN = 0x84d25bccu,            // not 0 while the disk mode is open
    SYNTH_TOTAL = 0x08023a84u,          // R6d: the synth's presets counted, factory and user; 0 before the first count
    DRUM_TOTAL = 0x08023a88u,           // the drum's
    SYNTH_COUNTING = 0x08023aacu,       // a byte, not 0 while the firmware counts the synth's presets (its counter's +0x14)
    DRUM_COUNTING = 0x08023ac4u         // the drum's
};

enum {
    ENGINES = OP1F_ENGINES_MOST, VALUES = OP1F_ENGINE_VALUES_MOST, SLOTS = 8, VOICES = 6, TABLE_WORDS = 11, ID_FIRST = 32,
    DSYNTH = 12, NAME = OP1F_KIND_NAME, SOUND_NAME = 24, TABLE_BYTES = 284, BLOCK_BYTES = 52, FRAMES = 128, RATE = 88200,
    CARDS = OP1F_SLOTS_MOST, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, ROW_BYTES = 64, KNOB_BYTES = 8, RECORD_BYTES = 52,
    KIND_INT = 1, KIND_UINT = 2, KIND_STRING = 4, KIND_ARRAY = 6, KIND_OBJECT = 7, KIND_AT = 4, MEMBER_ROOM = 64,
    ASIDES_KEPT = 16, RIDE_LINES = 32, PERIOD_FRAMES = 256, ALIGN_LEAST = 4, CALL_SHOWN = OP1F_CALL_SHOWN,
    RIDE_AWAY = 0x80, STORE_MOST = 2048, STORE_LINES = 16, SOUND_ROOM = OP1F_ENGINE_SOUND_MOST + 64, PATH = 96, SOUNDS = OP1F_ENGINE_SOUNDS_MOST,
    DEPTH_MOST = 4, WRITTEN_SAID = 32,
    USER_MOST = 500, USER_CLASSIC = 100, USER_FREE = 20,  // R6d: the firmware's limits of user sounds; what stays free for the owner's saves
    ROOM_GIVEN = 1, ROOM_REFUSED = 2, HEARD_TRIES = 4
};
static const u32 CYCLES_MOST = (u32)OP1F_ENGINE_CYCLES_MOST, CYCLES_OFF = (u32)OP1F_ENGINE_OFF, OVER_CALLS = (u32)OP1F_STAGE_OVER_CALLS,
                 SPARE = (u32)OP1F_STAGE_CYCLES_SPARE, VOICE_MOST = (u32)OP1F_ENGINE_VOICE_MOST, ALIGN_MOST = (u32)OP1F_MEMORY_ALIGN_MOST,
                 MEMORY_MOST = (u32)OP1F_MEMORY_MOST, BYTES_MOST = (u32)OP1F_ASK_BYTES_MOST, SOUND_MOST = (u32)OP1F_ENGINE_SOUND_MOST;
static const u32 SECOND = 400000000u;                  // the processor's cycles in a second
static const u32 HEARD_CYCLES = 4643991u;             // R6d: eight blocks of 128 frames at 88,200 a second, in the processor's cycles
static_assert(sizeof(Op1fEngine) == TABLE_BYTES && sizeof(Op1fVoiceBlock) == BLOCK_BYTES, "level 9's tables as the header has them");

static const char LOG_FILE[] = "../src/main/op1f_engines.cpp";
static const char KEY_LIST[] = "synth_types";
static const char KEY_TYPE[] = "type";
static const char KEY_ID[] = "id";
static const char KEY_NAME[] = "name";
static const char KEY_DEFAULTS[] = "default_params";
static const char DSYNTH_NAME[] = "dsynth";
static const char SOUNDS_ROOT[] = "user/synth_presets/";
static const char STORE_PATH[] = "user/pf/.engines";
static const char TEMPORARY_FOLDER[] = "tmp/";
static const char TEMPORARY[] = "tmp/op1f_engine.part";
static const char READ_MODE[] = "rb";
static const char WRITE_MODE[] = "w+b";                // texts the open compares with its own table: a new file, written

// Names no pack's engine may take: the unit's own engines (as the catalog names them, and the list's spelling of one),
// the sampler, and fazer, which the engine list hides while it is locked. Fixed fields of 16 bytes; the empty name ends
// the list. An engine's name is also held against every row of the catalog's `synth_types` at the registration.
static const char REFUSED_NAMES[][16] = {"cluster", "digital", "drwave", "dr.wave", "fm", "phase", "pulse", "dna", "string", "dsynth",
                                         "voltage", "dimension", "amp", "vocoder", "sampler", "fazer", ""};

// The members an engine's sound may hold: those the unit's own synth sounds hold, and the base's own; and the effects
// and LFOs the unit's sounds name.
static const char SOUND_MEMBERS[][16] = {"adsr", "fx_active", "fx_params", "fx_type", "knobs", "lfo_active", "lfo_params", "lfo_type",
                                         "mtime", "name", "octave", "original_folder", "synth_version", "type", "op1f_engine", ""};
// Each member by its place in the table above: the check tells them by their place, never by a name written again (a
// row of literals the compiler lays out one after the other is a row of the firmware's own bytes).
enum { M_ADSR, M_FX_ACTIVE, M_FX_PARAMS, M_FX_TYPE, M_KNOBS, M_LFO_ACTIVE, M_LFO_PARAMS, M_LFO_TYPE, M_MTIME, M_NAME, M_OCTAVE, M_ORIGINAL_FOLDER,
       M_SYNTH_VERSION, M_TYPE, M_ENGINE, M_COUNT };
static_assert(sizeof SOUND_MEMBERS / sizeof SOUND_MEMBERS[0] == (unsigned)M_COUNT + 1u, "a place in the table for each member, and the end");
static const char SOUND_EFFECTS[][16] = {"delay", "grid", "punch", "phone", "spring", "mother", "nitro", "cwo", "terminal", ""};
static const char SOUND_LFOS[][16] = {"tremolo", "value", "random", "element", "midi", "velocity", "crank", ""};

// The mark of a base of level 9.
const unsigned op1f_level_9 = 9;

// R6d, level 10: a synth slot's values as its voices last rendered them, the LFOs applied. The audio thread writes
// them at each render between two steps of `seq` (odd while it writes); the display thread reads them between two
// reads of it (op1f_engines_param_heard). `ride` is the ride they are of, `stamp` the cycle counter at the render;
// `live` is set by each render and cleared by the frame once the stamp is older than HEARD_CYCLES, so that a count
// of cycles that came round (every ten seconds) never passes for a fresh one.
struct Heard {
    volatile u32 seq, ride, stamp, live;
    volatile int values[VALUES];
};
static Heard heard[SLOTS];

// ---- from the cards unit ----

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
// The card's slot + 1 while the base calls that card's frame function on the caller's stack, else 0.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");

// ---- the words other units read ----

volatile unsigned op1f_engines_awake;
volatile unsigned op1f_engines_registered;

// ---------------------------------------------------------------- the registry

struct Budget {                                        // an engine's: the audio thread writes, the display thread reads
    u32 window_start, window_over, window_most;
    volatile u32 off;                                  // switched off until the next start
    volatile u32 off_cycles, off_overs;
    u32 off_said;
};

enum { NONE = 0, CLEARING = 1, READY = 2, REFUSED = 3 };

struct Engine {
    const Op1fEngine* table;
    u32 number;                                        // its place in the registry: its id less 32
    u32 slot, ask, version;                            // the card's slot in op1f_cards_2, the ask's number in its list, the pack's version
    u32 flags, cycles, count, voice_bytes, voice_align, stride;
    char name[NAME];
    char value_names[VALUES][NAME];
    int low[VALUES], high[VALUES], start[VALUES];      // the pack's ranges
    int row_low[VALUES], row_high[VALUES];             // dsynth's knob rows, read at the first frame
    u32 q_in[VALUES], f_in[VALUES];                    // word to value: the whole part and the fraction (16 bits, rounded up) of the ratio
    u32 q_out[VALUES], f_out[VALUES];                  // value to word, the same
    u32 fixed[VALUES];                                 // a row with no span: the value stands at its start
    int handle;                                        // its six voices' memory
    volatile u32 memory;                               // NONE, CLEARING, READY, REFUSED
    u8* volatile state;                                // voice 0's memory; voice v's is `stride` x v further
    volatile u32 renders, sum, most, over;             // this period's: the audio thread writes, the display thread reads and clears masked
    Budget budget;
    u32 sounds_checked;                                // 1 its sounds may be written, 2 one of them is not one the base can write
};
static Engine engines[ENGINES];

// What the check found, said at the first frame.
enum { WHY_ORDER = 1, WHY_NAME = 2, WHY_TAKEN = 3, WHY_COST = 4, WHY_NINTH = 5, WHY_NO_DSYNTH = 6 };
struct Aside { u32 slot, why, a, b; char name[NAME]; };
static Aside asides[ASIDES_KEPT];
static u32 aside_count, aside_more;

struct Candidate { const Op1fEngine* table; u32 slot, ask; };
static Candidate candidates[ENGINES];
static u32 candidate_count;
static u32 checked;                                    // the asks were looked through (once a start)
enum { LIST_UNREAD = 1, LIST_NOT_OBJECT = 2, LIST_NOT_ARRAY = 3 };
static u32 list_state;                                 // 0 when the engines were registered, else why not
static u32 dsynth_id;                                  // dsynth's catalog id, as the catalog holds it
static volatile u32 rows_added;                        // the rows added at the catalog's reads

// ---------------------------------------------------------------- the rides and the voice objects

// Which engine a synth slot rides: 0 none, an engine's number + 1, or RIDE_AWAY (the slot names an engine whose pack is
// not here, `away_name[slot]`). Written by the pick, the assign and the frame, inside a mask; read by every thread.
static volatile u32 rides[SLOTS];
static char away_name[SLOTS][NAME];
// R17 (base 39): the sounds loaded onto each synth slot since the start (the assign's OP1F_KEPT_LOADED, whatever the
// sound names), for the sample unit's `dropped` (op1f_engines_ride), which asks only whether the count moved: a plain
// increment, no mask (two assigns at once still move it), so a load at rest takes no mask, as before base 39.
static volatile u32 loads_counted[SLOTS];

struct Voice;
static u32 voice_destroy(Voice* self, u32 a, u32 b);
static u32 voice_render(Voice* self, int* out, const u32* words);
static u32 voice_start(Voice* self, const u32* words);
static u32 voice_release(Voice* self, const u32* words);
static u32 voice_stereo(Voice* self);

// One of the base's six voice objects. The firmware reads its first word only, as the table it calls through.
struct Voice {
    const void* const* table;
    u32 stock;                                         // dsynth's object it stands in for: kept for the record, never called
    u32 voice;                                         // 0 to 5
    volatile u32 slot;                                 // the slot it was swapped in for
    volatile u32 begins, ends;                         // counted by start and release, inside a mask
    u32 seen_begins, seen_ends;                        // what its last render saw
    volatile u32 fresh;                                // set by a swap, taken by the next render
    volatile u32 due;                                  // a `reset` is owed before the next render
    volatile u32 swap;                                 // the swap that put it there, counted
    volatile u32 stereo;                               // the answer of +0x1c: how its last render filled `out`
};
static const void* voice_table[TABLE_WORDS];          // made at the first frame: the base's five, dsynth's other six
static volatile u32 table_ready;
static Voice voices[VOICES];

// What other threads tell the display thread: a bit a slot, set and taken under a mask.
static volatile u32 picked;                            // a ride begun by a pick, its start values still to be written
static volatile u32 ended;                             // a ride ended by a pick
static volatile u32 loaded;                            // a ride set or ended by a sound loaded (its F4)
static volatile u32 job_seen;                          // the engine job's F4 came for the slot since its pick
static volatile u32 told;                              // the values unit told a load, a start, a drop
static volatile u32 settling;                          // a pick or a load whose build has not come yet: no type check
static volatile u32 settle_build[SLOTS];               // the build count at the slot's last F4
static volatile u32 builds;                            // E1's returns counted while level 9 is awake
static volatile u32 swaps;                             // the swaps made
static volatile u32 unknown_builds;                    // builds into another place than the six words: no swap

static u32 engine_screen;                              // the screen T1 asks for dsynth, read at the first frame
static int dsynth_low[VALUES], dsynth_high[VALUES];   // dsynth's eight knob rows, read at the first frame an engine is registered
static u32 resting;                                    // 1: level 9 at rest, the frame returns at once

static struct {
    u32 first_done;
    u32 frames;
    int countdown;
    u32 ride_lines;
    u32 unknown_said;
    u32 away_said;                                     // a bit a slot: its "not on this unit" line was said
    u32 count_said;                                    // R6d: the unit's count of user sounds was said this start
    QuietGate gate;
} unit;

// ---------------------------------------------------------------- small helpers

static int same_text(const char* a, const char* b, u32 most) {
    for (u32 i = 0; i < most; ++i) {
        if (a[i] != b[i]) return 0;
        if (a[i] == 0) return 1;
    }
    return 1;
}

static void copy_name(char* to, const char* from) {
    u32 i = 0;
    for (; i + 1 < (u32)NAME && from[i]; ++i) to[i] = from[i];
    for (; i < (u32)NAME; ++i) to[i] = 0;
}

static u32 put(char* out, u32 at, u32 room, const char* text) {
    while (*text && at + 1u < room) out[at++] = *text++;
    out[at] = 0;
    return at;
}

// n / d by shifts and subtractions, d not 0 (the module links no runtime).
static u32 divided(u32 n, u32 d) {
    u32 q = 0, r = 0;
    for (int bit = 31; bit >= 0; --bit) {
        r = (r << 1) | ((n >> bit) & 1u);
        if (r >= d) {
            r -= d;
            q |= 1u << bit;
        }
    }
    return q;
}

static int plain_name(const char* name, u32 field) {
    u32 n = 0;
    for (; n < field && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < field;
}

static int in_list(const char* name, const char (*list)[16]) {
    for (u32 i = 0; list[i][0]; ++i)
        if (same_text(name, list[i], 16)) return 1;
    return 0;
}

static const Engine* engine_of_ride(u32 r) {
    u32 n = op1f_engines_registered;
    if (r == 0 || r > n) return 0;
    return &engines[r - 1u];
}

static u32 slot_words(u32 slot) { return KNOB_WORDS + 32u * slot; }
static u32 slot_record(u32 slot) { return SYNTH_RECORDS + (u32)RECORD_BYTES * slot; }
static u32 slot_type(u32 slot) { return RD32(TYPE_WORDS + 4u * slot); }

// ---------------------------------------------------------------- the mapping of knob words and values

// A knob word as the pack's value: the word's upper half, held to dsynth's row, carried onto the value's range by a
// subtraction, a multiply and a shift.
static int value_of(const Engine* e, u32 i, u32 word) {
    if (e->fixed[i]) return e->start[i];
    int v = e->row_low[i] < 0 ? ((int)word >> 16) : (int)(word >> 16);
    if (v < e->row_low[i]) v = e->row_low[i];
    if (v > e->row_high[i]) v = e->row_high[i];
    u32 d = (u32)(v - e->row_low[i]);
    u32 p = d * e->q_in[i] + ((d * e->f_in[i]) >> 16);
    int out = e->low[i] + (int)p;
    return out > e->high[i] ? e->high[i] : out;
}

// A value as the knob's word, the other way: held to the value's range first.
static u32 word_of(const Engine* e, u32 i, int value) {
    if (value < e->low[i]) value = e->low[i];
    if (value > e->high[i]) value = e->high[i];
    if (e->fixed[i]) return (u32)e->row_low[i] << 16;
    u32 d = (u32)(value - e->low[i]);
    u32 s = d * e->q_out[i] + ((d * e->f_out[i]) >> 16);
    int v = e->row_low[i] + (int)s;
    if (v > e->row_high[i]) v = e->row_high[i];
    return (u32)v << 16;
}

// The ratio `to` / `from` as a whole part and a fraction of sixteen bits rounded up, so that the far end of one range
// lands on the far end of the other; worked out on the display thread.
static void ratio(u32 to, u32 from, u32* q, u32* f) {
    *q = divided(to, from);
    u32 rest = to - *q * from;
    *f = divided((rest << 16) + from - 1u, from);
}

// ---------------------------------------------------------------- the voice objects, on the firmware's threads

static void account(Engine* e, u32 began, u32 took) {
    Budget* b = &e->budget;
    u32 sum = e->sum + took;
    e->sum = sum < took ? 0xffffffffu : sum;
    ++e->renders;
    if (took > e->most) e->most = took;
    if (took > e->cycles) ++e->over;
    if (began - b->window_start >= SECOND) {
        b->window_start = began;
        b->window_over = 0;
        b->window_most = 0;
    }
    if (took > CYCLES_OFF) {
        b->off_overs = 0;
        b->off_cycles = took;
        b->off = 1;
        return;
    }
    if (took > (e->cycles << 1) && took - e->cycles > SPARE) {                       // over: twice its cost, and its allowance
        if (took > b->window_most) b->window_most = took;
        if (++b->window_over >= OVER_CALLS) {
            b->off_overs = b->window_over;
            b->off_cycles = b->window_most;
            b->off = 1;
        }
    }
}

static void silence(int* out, u32 words) {
    for (u32 k = 0; k < words; ++k) out[k] = 0;
}

// +0x8, the audio thread: (the object, the voice's out, the eight knob words of the block, the LFOs applied).
static u32 voice_render(Voice* self, int* out, const u32* words) {
    u32 slot = self->slot & 7u;
    u32 ride = rides[slot];
    Engine* e = (Engine*)engine_of_ride(ride);
    if (e == 0 || e->budget.off || e->memory != (u32)READY || self->due) {           // away, switched off, its memory not here, or a reset owed
        self->stereo = 0;
        silence(out, FRAMES);
        return 0;
    }
    u32 stereo = (e->flags & (u32)OP1F_ENGINE_STEREO) ? 1u : 0u;
    int values[VALUES];
    Heard* h = &heard[slot];                                                        // R6d: the values as heard, level 10's
    u32 seq = h->seq + 1u;
    h->seq = seq;                                                                   // odd: being written
    h->ride = ride;
    for (u32 i = 0; i < e->count; ++i) {
        int v = value_of(e, i, words[i]);
        values[i] = v;
        h->values[i] = v;
    }
    h->stamp = cycles_now();
    h->live = 1;
    h->seq = seq + 1u;                                                              // even: whole
    u32 b = self->begins, n = self->ends, events = 0;
    if (b != self->seen_begins) events |= (u32)OP1F_VOICE_BEGINS;
    if (n != self->seen_ends) events |= (u32)OP1F_VOICE_ENDS;
    self->seen_begins = b;
    self->seen_ends = n;
    if (self->fresh) {
        events |= (u32)OP1F_VOICE_FRESH;
        self->fresh = 0;
    }
    Op1fVoiceBlock block;
    block.bytes = BLOCK_BYTES;
    block.slot = slot;
    block.voice = self->voice;
    block.frames = FRAMES;
    block.rate = RATE;
    block.out = out;
    block.values = values;
    block.step = RD32(PITCH);
    block.velocity = (int)RD32(VELOCITY);
    block.events = events;
    block.period = RD32(PERIOD);
    block.playing = RD32(TRANSPORT) != 0 ? 1u : 0u;
    block.state = e->voice_bytes ? (void*)(e->state + e->stride * self->voice) : 0;
    self->stereo = stereo;
    u32 began = cycles_now();
    e->table->render(&block);
    account(e, began, cycles_now() - began);
    return 0;
}

// +0xc and +0x14: a note begins or is let go on the voice (the note's thread, or the audio thread inside its block).
static u32 voice_start(Voice* self, const u32* words) {
    (void)words;
    u32 mask;
    MASK_OFF(mask);
    self->begins = self->begins + 1u;
    MASK_ON(mask);
    return 0;
}

static u32 voice_release(Voice* self, const u32* words) {
    (void)words;
    u32 mask;
    MASK_OFF(mask);
    self->ends = self->ends + 1u;
    MASK_ON(mask);
    return 0;
}

// +0x1c, asked after each render: 1 when the render filled `out` with two channels.
static u32 voice_stereo(Voice* self) {
    return self->stereo;
}

// +0x0 and +0x4: no code of the firmware's calls them; they return.
static u32 voice_destroy(Voice* self, u32 a, u32 b) {
    (void)a;
    (void)b;
    return (u32)self;
}

// The six words hold dsynth's objects, every one: the firmware built dsynth there.
static int six_are_dsynth() {
    for (u32 v = 0; v < (u32)VOICES; ++v)
        if (RD32(RD32(SIX_WORDS + 4u * v)) != DSYNTH_TABLE) return 0;
    return 1;
}

// The base's six objects put into the six words for `slot`, each taking the dsynth object it replaces, fresh; only when
// the six hold dsynth's objects, all inside one mask. No call, no line.
static u32 swap_in(u32 slot) {
    const Engine* e = engine_of_ride(rides[slot]);
    u32 due = e != 0 && e->table->reset != 0 ? 1u : 0u;
    u32 done = 0, mask;
    MASK_OFF(mask);
    if (table_ready && six_are_dsynth()) {
        for (u32 v = 0; v < (u32)VOICES; ++v) {
            Voice* o = &voices[v];
            o->table = voice_table;
            o->stock = RD32(SIX_WORDS + 4u * v);
            o->voice = v;
            o->slot = slot;
            o->seen_begins = o->begins;
            o->seen_ends = o->ends;
            o->fresh = 1;
            o->due = due;
            o->swap = o->swap + 1u;
            o->stereo = 0;
            WR32(SIX_WORDS + 4u * v, (u32)o);
        }
        swaps = swaps + 1u;
        done = 1;
    }
    MASK_ON(mask);
    return done;
}

// The six words hold the base's objects.
static int six_are_ours() {
    for (u32 v = 0; v < (u32)VOICES; ++v)
        if (RD32(SIX_WORDS + 4u * v) != (u32)&voices[v]) return 0;
    return 1;
}

// ---------------------------------------------------------------- E1: a synth engine built

// From the factory's entry (module/build_r6_engines.s) after the firmware's factory has returned, while level 9 is
// awake: the type it built and where it put the six objects. The build is for the slot the owner sees (the readings'
// (a): every one of the factory's four callers builds for it); the base's objects go in when that slot rides an engine,
// its type word still reads dsynth and the six words hold dsynth's objects.
extern "C" void op1f_engine_built(unsigned type, unsigned out) OP1F_NAMED("op1f_engine_built");
extern "C" void op1f_engine_built(unsigned type, unsigned out) {
    builds = builds + 1u;
    if (type != (u32)DSYNTH) return;
    u32 g = RD32(GUI_SLOT);
    if (g >= (u32)SLOTS || rides[g] == 0 || slot_type(g) != (u32)DSYNTH) return;
    if (out != SIX_WORDS) {
        unknown_builds = unknown_builds + 1u;
        return;
    }
    swap_in(g);
}

// ---------------------------------------------------------------- E2: a pick in the engine list, or by MIDI

static void clear_entry() {
    op1f_stand_screens[2u * (u32)OP1F_STAND_ENGINE] = 0;                            // the mode first
    op1f_stand_screens[2u * (u32)OP1F_STAND_ENGINE + 1u] = 0;
}

// The engine pick, from its entry once level 9 is awake: R0 the catalog id picked, R1 the synth slot. The key thread,
// or MIDI's.
extern "C" void op1f_engine_pick_do(unsigned id, unsigned slot) OP1F_NAMED("op1f_engine_pick_do");
extern "C" void op1f_engine_pick_do(unsigned id, unsigned slot) {
    u32 n = op1f_engines_registered, mask;
    if (slot >= (u32)SLOTS) {
        ROOM_PICK(id, slot);
        return;
    }
    u32 bit = 1u << slot;
    if (n != 0 && id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n) {
        MASK_OFF(mask);
        rides[slot] = id - (u32)ID_FIRST + 1u;                                      // the mark first: the job's build finds it
        picked |= bit;
        ended &= ~bit;
        job_seen &= ~bit;
        settling |= bit;
        op1f_stand_screens[2u * (u32)OP1F_STAND_ENGINE + 1u] = engine_screen;       // the screen before the mode
        op1f_stand_screens[2u * (u32)OP1F_STAND_ENGINE] = 0u + 1u;                  // the synth's mode + 1
        MASK_ON(mask);
        ROOM_PICK(dsynth_id, slot);                                                 // the firmware makes the sound dsynth and applies it
        return;
    }
    if (rides[slot] != 0) {
        MASK_OFF(mask);
        rides[slot] = 0;
        ended |= bit;
        picked &= ~bit;
        settling &= ~bit;
        if (op1f_stand_screens[2u * (u32)OP1F_STAND_ENGINE] == 1u) clear_entry();
        MASK_ON(mask);
    }
    ROOM_PICK(id, slot);
}

// ---------------------------------------------------------------- a sound put on a slot (F4)

static u32 kind(const void* value) {
    return ((const volatile u8*)value)[KIND_AT];
}

static u32 registered_named(const char* name) {
    u32 n = op1f_engines_registered;
    for (u32 i = 0; i < n; ++i)
        if (same_text(name, engines[i].name, NAME)) return i + 1u;
    return 0;
}

static void wake() {
    op1f_engines_awake = 1;
    resting = 0;
}

extern "C" void op1f_engines_assign(unsigned slot, unsigned how, void* json, const char* name) {
    if (slot >= (u32)SLOTS) return;
    u32 bit = 1u << slot, mask;
    if (how == (u32)OP1F_KEPT_ENGINE) {                                            // the engine job: dsynth with its defaults is put on the slot
        if (op1f_engines_awake == 0) return;
        const Engine* e = engine_of_ride(rides[slot]);
        if (e && json && kind(json) == (u32)KIND_OBJECT) {                         // named after the engine, as the unit names a sound after any engine
            u32 value[4];
            STOCK_JSON_TEXT(value, e->name);
            STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(json, KEY_NAME), value);
            STOCK_JSON_DESTROY(value);
        }
        MASK_OFF(mask);
        job_seen |= bit;
        settle_build[slot] = builds;
        MASK_ON(mask);
        return;
    }
    if (how != (u32)OP1F_KEPT_LOADED) return;
    loads_counted[slot] = loads_counted[slot] + 1u;                                 // R17: every load counted, an engine named or not
    if (name == 0 && rides[slot] == 0) return;                                      // at rest, and the sound names no engine: nothing
    u32 r = 0;
    char held[NAME];
    if (name) {
        copy_name(held, name);
        r = registered_named(held);
        if (r == 0) r = plain_name(held, NAME) ? (u32)RIDE_AWAY : 0u;
    }
    MASK_OFF(mask);
    if (r == (u32)RIDE_AWAY)
        for (u32 c = 0; c < (u32)NAME; ++c) away_name[slot][c] = held[c];
    rides[slot] = r;
    loaded |= bit;
    picked &= ~bit;
    settling |= bit;
    settle_build[slot] = builds;
    if (r) wake();
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the registration, at the catalog's read

static void set_aside(u32 slot, u32 why, u32 a, u32 b, const char* name) {
    if (aside_count >= (u32)ASIDES_KEPT) {
        ++aside_more;
        return;
    }
    Aside* s = &asides[aside_count++];
    s->slot = slot;
    s->why = why;
    s->a = a;
    s->b = b;
    copy_name(s->name, name ? name : "?");
}

static int inside(u32 at, const Op1fCardSlot* slot, u32 bytes) {
    return at >= slot->first && at < slot->end && slot->end - at >= bytes;
}

static int function_in(u32 f, const Op1fCardSlot* slot) {
    return inside(f, slot, 2u) && (f & 1u) == 0;
}

// Why an ask of this kind is left aside (WHY_), or 0. For WHY_ORDER, (a, b):
//   (the ask's bytes, 284)                 its table is too small or too large, or there is none
//   (the bytes its head says, the ask's)   the two differ
//   (its place from the pack's first byte, the pack's bytes)   `what` not inside the pack on a word
//   (its flags, 1)                         a flag this base does not know
//   (`shown`'s place, the pack's bytes)    `shown` not a function of the pack
//   (`render`'s place, the pack's bytes)   `render` 0 or not a function of the pack
//   (`reset`'s place, the pack's bytes)    `reset` not a function of the pack
//   (its working memory, 1048576)          more than 1 MiB a voice
//   (its alignment, 4096)                  not a power of two from 4 to 4096
//   (six voices' memory, 16777216)         more than level 6's 16 MiB for the six
//   (its count, 8)                         not 1 to 8 values
//   (0x100 + the value's number, 0)        a value's name, or its range, or its start
//   (its sounds, 32)                       more than 32 sounds, or a list that is not inside the pack
//   (0x200 + the sound's number, 0)        a sound's name (or one an earlier sound has), or its text not inside the pack
//                                          or longer than 2048 bytes
static u32 check(const Op1fAsk* ask, const Op1fCardSlot* slot, u32* a, u32* b) {
    const Op1fEngine* t = (const Op1fEngine*)ask->what;
    u32 span = slot->end - slot->first;
    if (t == 0 || ask->bytes < (u32)TABLE_BYTES || ask->bytes > BYTES_MOST) { *a = ask->bytes; *b = TABLE_BYTES; return WHY_ORDER; }
    if (!inside((u32)t, slot, (u32)TABLE_BYTES) || ((u32)t & 3u) != 0) { *a = (u32)t - slot->first; *b = span; return WHY_ORDER; }
    if (!plain_name(t->head.name, NAME)) return WHY_NAME;
    if (t->head.bytes != ask->bytes) { *a = t->head.bytes; *b = ask->bytes; return WHY_ORDER; }
    if ((t->head.flags & ~(u32)OP1F_ENGINE_STEREO) != 0) { *a = t->head.flags; *b = OP1F_ENGINE_STEREO; return WHY_ORDER; }
    u32 f = (u32)t->head.shown;
    if (f != 0 && !function_in(f, slot)) { *a = f - slot->first; *b = span; return WHY_ORDER; }
    f = (u32)t->render;
    if (!function_in(f, slot)) { *a = f - slot->first; *b = span; return WHY_ORDER; }
    f = (u32)t->reset;
    if (f != 0 && !function_in(f, slot)) { *a = f - slot->first; *b = span; return WHY_ORDER; }
    if (t->voice_bytes > VOICE_MOST) { *a = t->voice_bytes; *b = VOICE_MOST; return WHY_ORDER; }
    u32 align = t->voice_align;
    if (t->voice_bytes != 0 && (align < (u32)ALIGN_LEAST || align > ALIGN_MOST || (align & (align - 1u)) != 0)) { *a = align; *b = ALIGN_MOST; return WHY_ORDER; }
    if (t->count < 1u || t->count > (u32)VALUES) { *a = t->count; *b = VALUES; return WHY_ORDER; }
    for (u32 i = 0; i < t->count; ++i) {
        const Op1fParam* p = &t->params[i];
        if (!plain_name(p->name, NAME) || p->low >= p->high || (u32)(p->high - p->low) > 65535u || p->start < p->low || p->start > p->high) {
            *a = 0x100u + i;
            *b = 0;
            return WHY_ORDER;
        }
    }
    if (t->sounds > (u32)SOUNDS || (t->sounds != 0 && (t->sound == 0 || !inside((u32)t->sound, slot, 32u * t->sounds) || ((u32)t->sound & 3u) != 0))) {
        *a = t->sounds;
        *b = SOUNDS;
        return WHY_ORDER;
    }
    for (u32 i = 0; i < t->sounds; ++i) {
        const Op1fSound* s = &t->sound[i];
        int named = plain_name(s->name, SOUND_NAME);
        for (u32 k = 0; k < i && named; ++k)
            if (same_text(s->name, t->sound[k].name, SOUND_NAME)) named = 0;
        if (!named || s->bytes == 0 || s->bytes > SOUND_MOST || s->text == 0 || !inside((u32)s->text, slot, s->bytes)) {
            *a = 0x200u + i;
            *b = 0;
            return WHY_ORDER;
        }
    }
    if (t->cycles < 1u || t->cycles > CYCLES_MOST) return WHY_COST;
    return 0;
}

// The asks of kind OP1F_ASK_ENGINE of the cards the base uses, in the list's order and each card's, held to their form.
static void look_through() {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;                                   // a card the base does not use
        const Op1fCardSlot* slot = &op1f_cards_2.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 9u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_ENGINE) continue;
            u32 a = 0, b = 0;
            u32 why = check(ask, slot, &a, &b);
            const Op1fEngine* t = (const Op1fEngine*)ask->what;
            if (why == WHY_ORDER) {
                int named = t != 0 && inside((u32)t, slot, (u32)TABLE_BYTES) && plain_name(t->head.name, NAME);
                set_aside(i, why, a, b, named ? t->head.name : "?");
                continue;
            }
            if (why == WHY_NAME) {
                set_aside(i, why, 0, 0, "?");
                continue;
            }
            int taken = in_list(t->head.name, REFUSED_NAMES);
            for (u32 c = 0; c < candidate_count && !taken; ++c)
                if (same_text(candidates[c].table->head.name, t->head.name, NAME)) taken = 1;
            if (taken) {
                set_aside(i, WHY_TAKEN, 0, 0, t->head.name);
                continue;
            }
            if (why) {
                set_aside(i, why, 0, 0, t->head.name);
                continue;
            }
            if (candidate_count == (u32)ENGINES) {
                set_aside(i, WHY_NINTH, 0, 0, t->head.name);
                continue;
            }
            Candidate* c = &candidates[candidate_count++];
            c->table = t;
            c->slot = i;
            c->ask = k;
        }
    }
}

static const char* text_of(const void* value) {
    if (kind(value) != (u32)KIND_STRING) return 0;
    const char* text = *(const char* const*)value;
    return text ? text : "";
}

// The row of `list` whose `type` is the text `name`, or 0.
static const void* row_named(const void* list, const char* name) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i) {
        const void* row = STOCK_JSON_ELEMENT(list, i);
        if (kind(row) != (u32)KIND_OBJECT) continue;
        const char* text = text_of(STOCK_JSON_MEMBER(row, KEY_TYPE));
        if (text && same_text(text, name, NAME)) return row;
    }
    return 0;
}

// The candidates registered in order, each with its id; a name a row of the catalog already holds is left aside, and
// all of them when the catalog holds no row for dsynth.
static void enrol(const void* list) {
    const void* dsynth = row_named(list, DSYNTH_NAME);
    const void* id = dsynth ? STOCK_JSON_MEMBER(dsynth, KEY_ID) : 0;
    if (id == 0 || (kind(id) != (u32)KIND_INT && kind(id) != (u32)KIND_UINT)) {
        for (u32 c = 0; c < candidate_count; ++c) set_aside(candidates[c].slot, WHY_NO_DSYNTH, 0, 0, candidates[c].table->head.name);
        return;
    }
    dsynth_id = (u32)STOCK_JSON_INT(id);
    u32 n = 0;
    for (u32 c = 0; c < candidate_count; ++c) {
        const Op1fEngine* t = candidates[c].table;
        if (row_named(list, t->head.name)) {
            set_aside(candidates[c].slot, WHY_TAKEN, 0, 0, t->head.name);
            continue;
        }
        const Op1fCard* card = op1f_cards_2.slots[candidates[c].slot].card;
        Engine* e = &engines[n];
        e->table = t;
        e->number = n;
        e->slot = candidates[c].slot;
        e->ask = candidates[c].ask;
        e->version = card->version;
        e->flags = t->head.flags;
        e->cycles = t->cycles;
        e->count = t->count;
        e->voice_bytes = t->voice_bytes;
        e->voice_align = t->voice_align < (u32)ALIGN_LEAST ? (u32)ALIGN_LEAST : t->voice_align;
        e->stride = (t->voice_bytes + e->voice_align - 1u) & ~(e->voice_align - 1u);
        copy_name(e->name, t->head.name);
        for (u32 i = 0; i < t->count; ++i) {
            copy_name(e->value_names[i], t->params[i].name);
            e->low[i] = t->params[i].low;
            e->high[i] = t->params[i].high;
            e->start[i] = t->params[i].start;
        }
        if ((u32)VOICES * e->stride > MEMORY_MOST) {
            set_aside(candidates[c].slot, WHY_ORDER, (u32)VOICES * e->stride, MEMORY_MOST, t->head.name);
            continue;
        }
        ++n;
    }
    BARRIER();                                                                      // every record whole before the word that counts them
    op1f_engines_registered = n;
    if (n) op1f_engines_awake = 1;
}

static void add_row(void* list, u32 n, const void* dsynth) {
    u32 row[3], value[3];
    STOCK_JSON_MAKE(row, KIND_OBJECT);
    STOCK_JSON_TEXT(value, engines[n].name);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_TYPE), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_NUMBER(value, (int)((u32)ID_FIRST + n));
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_ID), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_DEFAULTS), STOCK_JSON_MEMBER(dsynth, KEY_DEFAULTS));
    STOCK_JSON_APPEND(list, row);
    STOCK_JSON_DESTROY(row);
}

extern "C" void op1f_engines_catalog_read(void* document) {
    if (!checked) {
        checked = 1;
        look_through();
        if (candidate_count == 0) return;
        if (kind(document) != (u32)KIND_OBJECT) list_state = LIST_NOT_OBJECT;
        else if (kind(STOCK_JSON_MEMBER(document, KEY_LIST)) != (u32)KIND_ARRAY) list_state = LIST_NOT_ARRAY;
        if (list_state) return;                                                     // no row in the list: no engine of ours either
        enrol(STOCK_JSON_MEMBER(document, KEY_LIST));
    }
    u32 n = op1f_engines_registered;
    if (n == 0 || kind(document) != (u32)KIND_OBJECT) return;
    const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
    if (kind(list) != (u32)KIND_ARRAY) return;
    const void* dsynth = row_named(list, DSYNTH_NAME);
    if (dsynth == 0) return;
    for (u32 i = 0; i < n; ++i) {
        if (row_named(list, engines[i].name)) continue;
        add_row(STOCK_JSON_PLACE(document, KEY_LIST), i, dsynth);
        rows_added = rows_added + 1u;
    }
}

extern "C" int op1f_engines_row_is(const void* row) {
    u32 n = op1f_engines_registered;
    if (n == 0 || kind(row) != (u32)KIND_OBJECT) return 0;
    const char* text = text_of(STOCK_JSON_MEMBER(row, KEY_TYPE));
    if (text == 0) return 0;
    for (u32 i = 0; i < n; ++i)
        if (same_text(text, engines[i].name, NAME)) return 1;
    return 0;
}

// ---------------------------------------------------------------- the base's member, op1f_engine

// The member's text read: 1 with the name (one to fifteen characters of the allowed ones) from a JSON string, or from an
// array of the name's character codes (the form a base that reads no string back keeps); 0 otherwise.
static int member_name(const char* text, char* name) {
    u32 at = 0, len = 0;
    char got[NAME];
    while (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r') ++at;
    if (text[at] == '"') {
        ++at;
        while (text[at + len] && text[at + len] != '"' && len < (u32)NAME) ++len;
        if (len == 0 || len >= (u32)NAME || text[at + len] != '"') return 0;
        for (u32 c = 0; c < len; ++c) got[c] = text[at + c];
    } else if (text[at] == '[') {
        ++at;
        for (;;) {
            while (text[at] == ' ' || text[at] == ',') ++at;
            if (text[at] == ']') break;
            u32 code = 0, digits = 0;
            while (text[at] >= '0' && text[at] <= '9' && digits < 4u) code = code * 10u + (u32)(text[at++] - '0'), ++digits;
            if (digits == 0 || code > 127u || len + 1u >= (u32)NAME) return 0;
            got[len++] = (char)code;
        }
        if (len == 0) return 0;
    } else {
        return 0;
    }
    for (u32 c = len; c < (u32)NAME; ++c) got[c] = 0;
    if (!plain_name(got, NAME)) return 0;
    copy_name(name, got);
    return 1;
}

// The member written for a slot that rides, or taken away for one that rides nothing; again at the next frame when the
// values unit cannot take it now. A slot that names an engine that is away keeps its member as it is.
static u32 member_due;                                 // a bit a slot: to be written at a frame

static void write_member(u32 slot) {
    u32 r = rides[slot];
    if (r == (u32)RIDE_AWAY) {
        member_due &= ~(1u << slot);
        return;
    }
    const Engine* e = engine_of_ride(r);
    int answer;
    if (e) {
        char text[MEMBER_ROOM];
        u32 at = put(text, 0, MEMBER_ROOM, "\"");
        at = put(text, at, MEMBER_ROOM, e->name);
        put(text, at, MEMBER_ROOM, "\"");
        answer = op1f_keeps_engine_set(slot, text);
    } else {
        answer = op1f_keeps_engine_set(slot, 0);
    }
    if (answer == OP1F_NOT_NOW) member_due |= 1u << slot;
    else member_due &= ~(1u << slot);
}

extern "C" void op1f_engines_changed(unsigned where, unsigned which, unsigned how) {
    if (where != (u32)OP1F_KEEP_SYNTH || which >= (u32)SLOTS) return;
    if (how == (u32)OP1F_KEPT_LOADED && op1f_engines_awake == 0) return;            // the assign said it already: nothing named an engine
    if (how != (u32)OP1F_KEPT_LOADED && how != (u32)OP1F_KEPT_START && how != (u32)OP1F_KEPT_DROPPED) return;
    told |= 1u << which;
    resting = 0;
}

// ---------------------------------------------------------------- the frame's work

static void say_ride(u32 slot, const char* what, const char* why, u32 built) {
    if (unit.ride_lines >= (u32)RIDE_LINES) return;
    ++unit.ride_lines;
    u32 sound = slot + 1u;                                                          // R6d (F-2): synth sounds from 1, as the unit shows them
    if (what) op1f_log(3, LOG_FILE, 3, "engine: synth %u rides '%s'", sound, what);
    else if (why) op1f_log(3, LOG_FILE, 4, "engine: synth %u rides nothing now: %s", sound, why);
    else op1f_log(3, LOG_FILE, 5, "engine: synth %u rides nothing now: the unit built engine %u there", sound, built);
}

static void say_away(u32 slot) {
    if (unit.away_said & (1u << slot)) return;
    unit.away_said |= 1u << slot;
    op1f_log(3, LOG_FILE, 6, "engine: synth %u names '%s', which is not on this unit: it is silent", slot + 1u, away_name[slot]);   // R6d (F-2)
}

// The member of a slot read after a start, a load or a drop: the slot rides its engine when it is registered and the
// unit keeps dsynth there; a name whose pack is away keeps the slot silent; a slot whose type is no longer dsynth lets
// its member go.
static void read_member(u32 slot) {
    char text[MEMBER_ROOM];
    char name[NAME];
    int got = op1f_keeps_engine_get(slot, text, (u32)MEMBER_ROOM);
    int named = got >= 0 && member_name(text, name);
    u32 was = rides[slot], now = 0, mask;
    if (named) {
        now = registered_named(name);
        if (now == 0) now = RIDE_AWAY;
    }
    if (now && slot_type(slot) != (u32)DSYNTH) {                                     // the unit keeps another engine there: never a ride
        MASK_OFF(mask);
        rides[slot] = 0;
        MASK_ON(mask);
        if (was) say_ride(slot, 0, 0, slot_type(slot));
        write_member(slot);
        return;
    }
    MASK_OFF(mask);
    if (now == (u32)RIDE_AWAY)
        for (u32 c = 0; c < (u32)NAME; ++c) away_name[slot][c] = name[c];
    rides[slot] = now;
    MASK_ON(mask);
    if (now == (u32)RIDE_AWAY) say_away(slot);
    else if (now && now != was) say_ride(slot, engines[now - 1u].name, 0, 0);
    else if (now == 0 && was) say_ride(slot, 0, "a sound without it was loaded", 0);
}

// dsynth's knob rows and the factors each way, at the first frame.
static void map_all() {
    u32 n = op1f_engines_registered;
    int lo[VALUES], hi[VALUES];
    for (u32 i = 0; i < (u32)VALUES; ++i) {
        u32 row = KNOB_ROWS + (u32)ROW_BYTES * (u32)DSYNTH + (u32)KNOB_BYTES * i;
        lo[i] = (int)(short)RD16(row);
        hi[i] = (int)RD16(row + 2u);
    }
    for (u32 i = 0; i < (u32)VALUES; ++i) {
        dsynth_low[i] = lo[i];
        dsynth_high[i] = hi[i];
    }
    op1f_log(3, LOG_FILE, 7, "engine: the knob rows of dsynth: %d..%d, %d..%d, %d..%d, %d..%d, %d..%d, %d..%d, %d..%d, %d..%d", lo[0], hi[0], lo[1], hi[1], lo[2], hi[2],
             lo[3], hi[3], lo[4], hi[4], lo[5], hi[5], lo[6], hi[6], lo[7], hi[7]);
    for (u32 k = 0; k < n; ++k) {
        Engine* e = &engines[k];
        for (u32 i = 0; i < e->count; ++i) {
            e->row_low[i] = lo[i];
            e->row_high[i] = hi[i];
            if (hi[i] <= lo[i]) {
                e->fixed[i] = 1;
                op1f_log(2, LOG_FILE, 8, "engine: '%s' value %u '%s' stands at %d: the knob row of dsynth has no span", e->name, i + 1u, e->value_names[i], e->start[i]);
                continue;
            }
            u32 stock = (u32)(hi[i] - lo[i]), own = (u32)(e->high[i] - e->low[i]);
            ratio(own, stock, &e->q_in[i], &e->f_in[i]);
            ratio(stock, own, &e->q_out[i], &e->f_out[i]);
        }
    }
}

static const char* aside_reason(const Aside* s) {
    switch (s->why) {
    case WHY_NAME: return "its name is not one the engine list can show";
    case WHY_TAKEN: return "its name is a stock engine's or an earlier pack's";
    case WHY_COST: return "its cost is not between 1 and 40000";
    case WHY_NINTH: return "it would be the ninth";
    case WHY_NO_DSYNTH: return "the unit's catalog holds no row for dsynth";
    default: return 0;
    }
}

// The base's six objects' table: its own five words, and dsynth's other six read from dsynth's table in memory, so that
// every call the base does not take is answered by the firmware's own code.
static void make_table() {
    for (u32 w = 0; w < (u32)TABLE_WORDS; ++w) voice_table[w] = (const void*)RD32(DSYNTH_TABLE + 4u * w);
    voice_table[0] = (const void*)&voice_destroy;
    voice_table[1] = (const void*)&voice_destroy;
    voice_table[2] = (const void*)&voice_render;
    voice_table[3] = (const void*)&voice_start;
    voice_table[5] = (const void*)&voice_release;
    voice_table[7] = (const void*)&voice_stereo;
    BARRIER();
    table_ready = 1;
}

// The display thread's swap: the slot the owner sees, when it is the slot built and the synth's block runs, rides (or
// names) an engine, its type word reads dsynth and the six words hold dsynth's objects (a ride kept from before the
// start, or one a load set after its build).
static void swap_if_due() {
    if (RD32(PLAYING) != 0) return;
    u32 g = RD32(GUI_SLOT);
    if (g >= (u32)SLOTS || RD32(BUILT_SLOT) != g || rides[g] == 0 || slot_type(g) != (u32)DSYNTH) return;
    if (settling & (1u << g)) return;                                               // a pick or a load whose build is coming: E1 swaps at it
    swap_in(g);
}

static void give_memory(Engine* e) {
    if (e->voice_bytes == 0) {
        e->state = 0;
        BARRIER();
        e->memory = READY;
        return;
    }
    char whose[48];
    u32 at = put(whose, 0, 48, "the six voices of '");
    at = put(whose, at, 48, e->name);
    put(whose, at, 48, "'");
    e->handle = op1f_memory_more((u32)VOICES * e->stride, e->voice_align, whose);
    if (e->handle <= 0) {
        e->memory = REFUSED;
        op1f_log(1, LOG_FILE, 10, "engine: '%s' has no room for its six voices: %s", e->name,
                 e->handle == OP1F_TOO_LARGE ? "the packs would take more than 16 MiB" : "the heap has no such room, or less than 8 MiB would stay free");
        return;
    }
    e->memory = CLEARING;
}

static void poll_memory(Engine* e) {
    if (e->memory != (u32)CLEARING) return;
    void* block = op1f_memory_block(e->handle);
    if (block == 0) return;
    e->state = (u8*)block;
    BARRIER();                                                                      // the memory before the word that hands it over
    e->memory = READY;
    op1f_log(3, LOG_FILE, 9, "engine: '%s' has %u bytes for its six voices at 0x%08x", e->name, (u32)VOICES * e->stride, (u32)block);
}

static void store_read();

// ---------------------------------------------------------------- the unit's user sounds (R6d)

// The unit's user sounds by the firmware's own count, and the most it takes: 1 with both, or 0 while the count is not
// to be had: the firmware counts its presets at the start and again after its own changes of them, and while it
// counts (its byte set) or before it first counted (a total 0, where the firmware's count would stop the unit) the
// base asks at a later frame. The two bytes, the two totals and the count are read inside one mask, so that no count
// begins between them. A file the base writes counts for the firmware from the next start.
static int user_sounds(u32* count, u32* most) {
    u32 mask, got = 0;
    int n = 0;
    MASK_OFF(mask);
    if (RD8(SYNTH_COUNTING) == 0 && RD8(DRUM_COUNTING) == 0 && RD32(SYNTH_TOTAL) != 0 && RD32(DRUM_TOTAL) != 0) {
        n = STOCK_USER_SOUNDS();
        got = 1;
    }
    MASK_ON(mask);
    if (!got) return 0;
    *count = n < 0 ? 0u : (u32)n;
    *most = STOCK_CLASSIC() != 0 ? (u32)USER_CLASSIC : (u32)USER_MOST;              // outside the mask: its first look is at the disk
    return 1;
}

// The count said once a start, at the first frame it is to be had.
static void say_count() {
    u32 count = 0, most = 0;
    if (!user_sounds(&count, &most)) return;
    unit.count_said = 1;
    op1f_log(3, LOG_FILE, 20, "engine: the unit holds %u user sound(s) of the %u it takes, synth and drum together", count, most);
}

// R6d: a heard value too old to be heard, its `live` cleared (the display thread; a mask only when one is to be).
static void heard_expire() {
    u32 now = cycles_now(), old = 0;
    for (u32 s = 0; s < (u32)SLOTS; ++s)
        if (heard[s].live && now - heard[s].stamp > HEARD_CYCLES) old |= 1u << s;
    if (old == 0) return;
    u32 mask;
    MASK_OFF(mask);
    now = cycles_now();
    for (u32 s = 0; s < (u32)SLOTS; ++s)
        if ((old & (1u << s)) && now - heard[s].stamp > HEARD_CYCLES) heard[s].live = 0;
    MASK_ON(mask);
}

static void first_frame() {
    for (u32 i = 0; i < aside_count; ++i) {
        const Aside* s = &asides[i];
        const char* pack = op1f_cards_2.slots[s->slot].card->name;
        if (s->why == WHY_ORDER) op1f_log(2, LOG_FILE, 2, "engine: '%s' of '%s' is left aside: its table is not in order (%u, %u)", s->name, pack, s->a, s->b);
        else op1f_log(2, LOG_FILE, 2, "engine: '%s' of '%s' is left aside: %s", s->name, pack, aside_reason(s));
    }
    if (aside_more) op1f_log(2, LOG_FILE, 2, "engine: %u more ask(s) left aside, not said", aside_more);
    if (list_state) op1f_log(2, LOG_FILE, 2, "engine: the unit's engine list could not be read (%u): no engine is registered", list_state);
    u32 n = op1f_engines_registered;
    u32 named = 0;
    char text[MEMBER_ROOM];
    char name[NAME];
    engine_screen = STOCK_ENGINE_SCREEN((u32)DSYNTH);                               // ready before any ride, at rest too: a load may wake level 9
    make_table();
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        int got = op1f_keeps_engine_get(s, text, (u32)MEMBER_ROOM);
        if (got >= 0 && member_name(text, name)) named |= 1u << s;
    }
    if (n == 0 && named == 0) {
        op1f_log(3, LOG_FILE, 1, "engine: none registered and no sound names one: level 9 is at rest");
        op1f_engines_awake = 0;
        resting = 1;
        return;
    }
    op1f_engines_awake = 1;
    for (u32 k = 0; k < n; ++k) {
        Engine* e = &engines[k];
        op1f_log(3, LOG_FILE, 11, "engine: '%s' of '%s' rides dsynth, entry %u of the engine list, %u cycles a voice declared, %u bytes a voice", e->name,
                 op1f_cards_2.slots[e->slot].card->name, (u32)ID_FIRST + k, e->cycles, e->voice_bytes);
    }
    if (n) map_all();
    if (n) say_count();                                                             // R6d: the room for the engines' sounds
    for (u32 k = 0; k < n; ++k) give_memory(&engines[k]);
    for (u32 s = 0; s < (u32)SLOTS; ++s)
        if (named & (1u << s)) read_member(s);
    swap_if_due();
    if (n) store_read();
    unit.countdown = PERIOD_FRAMES;
}

// `reset` called for a voice's engine, outside every pack's frame function, recorded as a call of its pack.
static void call_reset(const Engine* e, u32 slot, u32 voice, void* state) {
    Op1fCallKept kept;
    op1f_cards_call_in((u32)CALL_SHOWN, e->slot, &kept);
    e->table->reset(slot, voice, state);
    op1f_cards_call_out((u32)CALL_SHOWN, &kept);
}

static void resets() {
    if (!six_are_ours()) return;
    for (u32 v = 0; v < (u32)VOICES; ++v) {
        Voice* o = &voices[v];
        if (!o->due) continue;
        u32 slot = o->slot & 7u;
        const Engine* e = engine_of_ride(rides[slot]);
        if (e == 0 || e->table->reset == 0) {                                       // nothing to reset with: the voice may sound
            o->due = 0;
            continue;
        }
        if (e->memory != (u32)READY) continue;
        u32 swap = o->swap;
        call_reset(e, slot, v, e->voice_bytes ? (void*)(e->state + e->stride * v) : 0);
        u32 mask;
        MASK_OFF(mask);
        if (o->swap == swap) o->due = 0;                                            // a swap since is owed its own reset
        MASK_ON(mask);
    }
}

static void say_off(const Engine* e) {
    const Budget* b = &e->budget;
    if (!b->off || b->off_said) return;
    ((Budget*)b)->off_said = 1;
    if (b->off_overs == 0)
        op1f_log(1, LOG_FILE, 12, "engine: '%s' is switched off until the next start: %u cycles in one voice's render, %u declared", e->name, b->off_cycles, e->cycles);
    else
        op1f_log(1, LOG_FILE, 13, "engine: '%s' is switched off until the next start: %u renders in one second over twice its cost and its allowance, the most %u cycles, %u declared",
                 e->name, b->off_overs, b->off_cycles, e->cycles);
}

static void period() {
    if (--unit.countdown > 0) return;
    unit.countdown = PERIOD_FRAMES;
    u32 n = op1f_engines_registered;
    u32 renders[ENGINES], sum[ENGINES], most[ENGINES], over[ENGINES];
    u32 signature = quiet_mix(quiet_start(), n);
    int activity = 0;
    for (u32 k = 0; k < n; ++k) {                                                   // read and cleared, whatever the gate says
        Engine* e = &engines[k];
        u32 mask;
        MASK_OFF(mask);
        renders[k] = e->renders;
        sum[k] = e->sum;
        most[k] = e->most;
        over[k] = e->over;
        e->renders = 0;
        e->sum = 0;
        e->most = 0;
        e->over = 0;
        MASK_ON(mask);
        signature = quiet_mix(signature, renders[k] != 0 ? 1u : 0u);
        signature = quiet_mix(signature, over[k] != 0 ? 1u : 0u);
        signature = quiet_mix(signature, e->budget.off);
        if (over[k] != 0) activity = 1;
    }
    for (u32 s = 0; s < (u32)SLOTS; ++s) signature = quiet_mix(signature, rides[s]);
    if (!quiet_period(&unit.gate, signature, activity)) return;
    for (u32 k = 0; k < n; ++k) {
        if (renders[k] == 0) continue;
        op1f_log(3, LOG_FILE, 14, "engine: '%s' this period: %u render(s), %u cycles at most, %u on average, %u over", engines[k].name, renders[k], most[k],
                 divided(sum[k], renders[k]), over[k]);
    }
}

// The engine's start values written into the slot's eight words through the firmware's knob setter, as a knob writes
// them: the slot's words the voices play, and the record's document (so the slots file and the next save keep them).
// Values the engine has not leave dsynth's words as they are.
static void set_words(u32 slot, const u32* eight, u32 mask) {
    u32 words = slot_words(slot);
    u32 all[8];
    for (u32 i = 0; i < 8u; ++i) all[i] = RD32(words + 4u * i);
    for (u32 i = 0; i < 8u; ++i) {
        if ((mask & (1u << i)) == 0) continue;
        all[i] = eight[i];
        WR32(words + 4u * i, eight[i]);
    }
    STOCK_KNOB_SET(slot_record(slot), all);
}

static void sounds_frame();

// The frame's work, out of line so that the frame at rest is the test of one word and a return, with no frame of its own.
static ENGINES_APART void frame_work() {
    if (!unit.first_done) {
        unit.first_done = 1;
        first_frame();
        return;
    }
    ++unit.frames;
    u32 n = op1f_engines_registered;
    u32 mask;
    MASK_OFF(mask);
    u32 stopped = ended, by_load = loaded, given = told, seen = job_seen, built = builds, still = settling;
    ended = 0;
    loaded = 0;
    told = 0;
    MASK_ON(mask);
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        u32 bit = 1u << s;
        if (stopped & bit) {
            say_ride(s, 0, "another engine was picked", 0);
            write_member(s);                                                        // the member goes: the next save writes no engine
        }
        if (by_load & bit) {                                                        // the assign set the ride before the build: said, kept
            u32 r = rides[s];
            if (r == (u32)RIDE_AWAY) say_away(s);
            else if (r) say_ride(s, engines[r - 1u].name, 0, 0);
            else say_ride(s, 0, "a sound without it was loaded", 0);
        }
        if ((still & bit) && settle_build[s] != built && ((picked & bit) == 0 || (seen & bit) != 0)) {
            MASK_OFF(mask);
            settling &= ~bit;                                                       // the unit built the slot since: its type word says what it holds
            MASK_ON(mask);
            still &= ~bit;
        }
        if (given & bit) {                                                          // R6d (F-1): a load's member read once its build has come
            if (still & bit) {
                MASK_OFF(mask);
                told |= bit;                                                        // the type word is still the sound's before: again at a frame
                MASK_ON(mask);
            } else {
                read_member(s);
            }
        }
        if ((picked & bit) && (seen & bit) && (still & bit) == 0) {                 // a pick whose job has run and been built: its start values
            const Engine* e = engine_of_ride(rides[s]);
            MASK_OFF(mask);
            picked &= ~bit;
            job_seen &= ~bit;
            MASK_ON(mask);
            if (e && slot_type(s) == (u32)DSYNTH) {
                u32 eight[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                for (u32 i = 0; i < e->count; ++i) eight[i] = word_of(e, i, e->start[i]);
                set_words(s, eight, (1u << e->count) - 1u);
                say_ride(s, e->name, 0, 0);
                write_member(s);
            }
        }
    }
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        u32 bit = 1u << s, r = rides[s];
        if (r == 0 || (settling & bit)) continue;
        if (slot_type(s) != (u32)DSYNTH) {                                          // the unit built another engine there: the ride ends
            MASK_OFF(mask);
            rides[s] = 0;
            picked &= ~bit;
            MASK_ON(mask);
            say_ride(s, 0, 0, slot_type(s));
            write_member(s);
            continue;
        }
        if (member_due & bit) write_member(s);
    }
    if (unknown_builds != unit.unknown_said) {
        unit.unknown_said = unknown_builds;
        op1f_log(2, LOG_FILE, 15, "engine: %u build(s) from a place the base does not know: no swap", unit.unknown_said);
    }
    for (u32 k = 0; k < n; ++k) {
        poll_memory(&engines[k]);
        say_off(&engines[k]);
    }
    if (n && !unit.count_said) say_count();                                         // R6d: not to be had at the first frame: at a later one
    heard_expire();
    swap_if_due();
    resets();
    sounds_frame();
    period();
    u32 any = n;                                                                    // with nothing registered and nothing named, at rest again
    for (u32 s = 0; s < (u32)SLOTS && any == 0; ++s) any = rides[s] | (member_due & (1u << s));
    if (any == 0 && !six_are_ours()) {
        op1f_engines_awake = 0;
        resting = 1;
        for (u32 s = 0; s < (u32)SLOTS; ++s) heard[s].live = 0;                     // R6d: no render writes them at rest
    }
}

extern "C" void op1f_engines_frame() {
    if (resting) return;                                                            // level 9 at rest: one word a frame
    frame_work();
}

// Does an engine's page stand in now, or (`asked` 0) would it, whatever screen the controller asked for
// (module/build_r6_base.h)?
static int stand(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name, int asked) {
    if (op1f_engines_awake == 0 || engine_screen == 0 || RD32(CURRENT_MODE) != 0) return 0;
    u32 g = RD32(GUI_SLOT);
    if (g >= (u32)SLOTS || (asked && op1f_screen_asked != engine_screen)) return 0;
    u32 r = rides[g];
    if (r == 0) return 0;
    out->where = g;
    if (r == (u32)RIDE_AWAY) {
        out->slot = OP1F_STAND_NO_CARD;
        out->ask = 0;
        out->how = (u32)OP1F_SHOWN_UP | (u32)OP1F_STAND_AWAY;
        *name = away_name[g];
    } else {
        const Engine* e = engine_of_ride(r);
        if (e == 0) return 0;
        out->slot = e->slot;
        out->ask = e->ask;
        out->how = (u32)OP1F_SHOWN_UP | (e->budget.off ? (u32)OP1F_SHOWN_OFF | (u32)OP1F_STAND_OVER : 0u) | (e->memory == (u32)REFUSED ? (u32)OP1F_STAND_NO_ROOM : 0u);
        *name = e->name;
    }
    *mode = 0;
    *screen = engine_screen;
    return 1;
}

extern "C" int op1f_engines_standing(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) {
    return stand(out, mode, screen, name, 1);
}

extern "C" int op1f_engines_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen, const char** name) {
    return stand(out, mode, screen, name, 0);
}

// The knob names of the synth's LFO page for the engines' kind, as level 8's W4 asks them (the synth's shown slot, the
// index as it came): the pack's name of value `index` for a slot that rides a registered engine with that many
// values, else 0 (the firmware's own function then answers).
extern "C" const char* op1f_engines_names(unsigned slot, int index) {
    if (index < 0 || op1f_engines_awake == 0 || slot >= (u32)SLOTS) return 0;
    const Engine* e = engine_of_ride(rides[slot]);
    if (e == 0) return 0;
    u32 i = (u32)index & 7u;                                                        // the firmware takes the index by eight knobs for an engine
    return i < e->count ? e->value_names[i] : 0;
}

// ---------------------------------------------------------------- the helpers, for kind 8

static const Engine* engine_of(u32 card_slot, u32 ask) {
    u32 n = op1f_engines_registered;
    for (u32 i = 0; i < n; ++i)
        if (engines[i].slot == card_slot && engines[i].ask == ask) return &engines[i];
    return 0;
}

extern "C" int op1f_engines_param_get(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) {
    const Engine* e = engine_of(card_slot, ask);
    if (e == 0) return OP1F_ENGINE_NOT_MINE;
    if (index >= e->count || out == 0) return OP1F_BAD_NAME;
    if (place >= (u32)SLOTS || engine_of_ride(rides[place]) != e) return OP1F_NOT_HERE;
    *out = value_of(e, index, RD32(slot_words(place) + 4u * index));
    return 0;
}

// R6d, level 10: the value as the slot's voices last rendered it, or as set where none rendered in the last few blocks.
extern "C" int op1f_engines_param_heard(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) {
    const Engine* e = engine_of(card_slot, ask);
    if (e == 0) return OP1F_ENGINE_NOT_MINE;
    if (index >= e->count || out == 0) return OP1F_BAD_NAME;
    if (place >= (u32)SLOTS || engine_of_ride(rides[place]) != e) return OP1F_NOT_HERE;
    const Heard* h = &heard[place];
    for (u32 tries = 0; tries < (u32)HEARD_TRIES; ++tries) {
        u32 seq = h->seq;
        u32 ride = h->ride, live = h->live, stamp = h->stamp;
        int v = h->values[index];
        if ((seq & 1u) != 0 || h->seq != seq) continue;                             // a render came between: read again
        if (live && ride == e->number + 1u && cycles_now() - stamp <= HEARD_CYCLES) {
            *out = v;
            return 0;
        }
        break;
    }
    *out = value_of(e, index, RD32(slot_words(place) + 4u * index));
    return 0;
}

extern "C" int op1f_engines_param_set(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int value) {
    const Engine* e = engine_of(card_slot, ask);
    if (e == 0) return OP1F_ENGINE_NOT_MINE;
    if (index >= e->count) return OP1F_BAD_NAME;
    if (place >= (u32)SLOTS || engine_of_ride(rides[place]) != e) return OP1F_NOT_HERE;
    u32 eight[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    eight[index] = word_of(e, index, value);
    set_words(place, eight, 1u << index);
    return 0;
}

extern "C" int op1f_engines_riding(unsigned card_slot, unsigned ask, unsigned* bits) {
    const Engine* e = engine_of(card_slot, ask);
    if (e == 0) return OP1F_ENGINE_NOT_MINE;
    u32 b = 0;
    for (u32 s = 0; s < (u32)SLOTS; ++s)
        if (engine_of_ride(rides[s]) == e) b |= 1u << s;
    *bits = b;
    return 0;
}

// R17 (base 39): synth slot `slot`'s ride, for the sample unit (module/build_r17_sample.cpp), which names it weakly.
// Answers 0 when the slot rides no registered engine (nothing, an engine whose pack is away, or no such slot); else 1
// with `card` the card's slot in op1f_cards_2 and `ask` the ask's number in its list, or 2 with them written while a sound
// loaded onto the slot is not told yet (its assign seen and not yet said at a frame, or its member still to be read
// once its build has come: the ride may change at a frame). `loads` (not 0): the sounds loaded onto the slot since the
// start. From any thread: single words read in one masked instant, nothing called.
extern "C" unsigned op1f_engines_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* loads) OP1F_NAMED("op1f_engines_ride");
extern "C" unsigned op1f_engines_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* loads) {
    if (slot >= (u32)SLOTS) {
        if (loads) *loads = 0;
        return 0;
    }
    u32 mask, r, untold, count;
    MASK_OFF(mask);
    r = rides[slot];
    untold = (loaded | told) & (1u << slot);
    count = loads_counted[slot];
    MASK_ON(mask);
    if (loads) *loads = count;
    const Engine* e = engine_of_ride(r);
    if (e == 0) return 0;
    if (card) *card = e->slot;
    if (ask) *ask = e->ask;
    return untold ? 2u : 1u;
}

// ---------------------------------------------------------------- the engines' own sounds

// The store user/pf/.engines: for each engine whose sounds the base has written, a line "VERSION MADE NAME": the pack's
// version (in hexadecimal) whose sounds are all in the folder, and 1 when the base made the folder. Read at the first
// frame; written whole through the folders unit (never while the disk mode is open). Lines of engines that are away
// are kept as they were.
struct Kept { u32 version, made; char name[NAME]; };
static Kept kept[STORE_LINES];
static u32 kept_count;
static u32 store_dirty;
// One buffer for the store's text and a sound's: both the display thread's, used one after the other, never at once.
static char file_text[SOUND_ROOM];
static_assert((u32)SOUND_ROOM >= (u32)STORE_MOST + 1u, "the buffer holds the store's text and its ending zero");

static int hex_digit(char c, u32* d) {
    if (c >= '0' && c <= '9') { *d = (u32)(c - '0'); return 1; }
    if (c >= 'a' && c <= 'f') { *d = (u32)(c - 'a') + 10u; return 1; }
    return 0;
}

static void store_read() {
    kept_count = 0;
    int size = STOCK_FILE_SIZE(STORE_PATH);
    if (size <= 0 || size > (int)STORE_MOST) return;
    int handle = STOCK_OPEN(STORE_PATH, READ_MODE);
    if (handle < 0) return;
    int got = STOCK_READ(file_text, 1, (u32)size, handle);
    STOCK_CLOSE(handle);
    if (got != size) return;
    file_text[size] = 0;
    u32 at = 0;
    while (file_text[at] && kept_count < (u32)STORE_LINES) {
        Kept k;
        u32 d = 0, digits = 0;
        k.version = 0;
        while (hex_digit(file_text[at], &d) && digits < 8u) k.version = (k.version << 4) | d, ++at, ++digits;
        int fine = digits != 0 && file_text[at] == ' ' && (file_text[at + 1] == '0' || file_text[at + 1] == '1') && file_text[at + 2] == ' ';
        u32 len = 0;
        if (fine) {
            k.made = file_text[at + 1] == '1' ? 1u : 0u;
            at += 3;
            while (file_text[at + len] && file_text[at + len] != '\n' && len < (u32)NAME) ++len;
            fine = len != 0 && len < (u32)NAME && file_text[at + len] == '\n';
        }
        if (!fine) {                                                                // a line the base did not write: the rest is not read
            op1f_log(2, LOG_FILE, 16, "engine: %s holds a line the base cannot read: its sounds are written again where they are missing", STORE_PATH);
            return;
        }
        for (u32 c = 0; c < (u32)NAME; ++c) k.name[c] = c < len ? file_text[at + c] : 0;
        at += len + 1u;
        if (!plain_name(k.name, NAME)) continue;
        kept[kept_count++] = k;
    }
}

static Kept* kept_of(const char* name) {
    for (u32 i = 0; i < kept_count; ++i)
        if (same_text(kept[i].name, name, NAME)) return &kept[i];
    if (kept_count == (u32)STORE_LINES) return 0;
    Kept* k = &kept[kept_count++];
    k->version = 0;
    k->made = 0;
    copy_name(k->name, name);
    return k;
}

// The store written whole: answers 1 when it landed.
static int store_write() {
    u32 at = 0;
    for (u32 i = 0; i < kept_count; ++i) {
        char digits[9];
        u32 v = kept[i].version, d = 0;
        for (int s = 28; s >= 0; s -= 4) {
            u32 x = (v >> (u32)s) & 15u;
            if (x != 0 || d != 0 || s == 0) digits[d++] = (char)(x < 10u ? '0' + x : 'a' + x - 10u);
        }
        digits[d] = 0;
        at = put(file_text, at, STORE_MOST + 1u, digits);
        at = put(file_text, at, STORE_MOST + 1u, kept[i].made ? " 1 " : " 0 ");
        at = put(file_text, at, STORE_MOST + 1u, kept[i].name);
        at = put(file_text, at, STORE_MOST + 1u, "\n");
    }
    int answer = op1f_folders_write_base(STORE_PATH, file_text, at);
    if (answer < 0) return 0;
    store_dirty = 0;
    return 1;
}

// A sound's text held to what the unit's own synth sounds hold (module/build_r6_card.h, Op1fSound): 0 when it is one the
// base can write, else the reason.
struct Scan { const char* at; const char* end; int depth; };

static void scan_space(Scan* s) {
    while (s->at < s->end && (*s->at == ' ' || *s->at == '\t' || *s->at == '\n' || *s->at == '\r')) ++s->at;
}

// A string: 1 and its characters into `out` (`room` with the zero; longer ones are cut there), with no escape.
static int scan_text(Scan* s, char* out, u32 room) {
    scan_space(s);
    if (s->at >= s->end || *s->at != '"') return 0;
    ++s->at;
    u32 n = 0;
    while (s->at < s->end && *s->at != '"') {
        u8 c = (u8)*s->at;
        if (c < 0x20 || c == '\\' || c > 0x7e) return 0;
        if (n + 1u < room) out[n++] = (char)c;
        ++s->at;
    }
    if (s->at >= s->end) return 0;
    ++s->at;
    out[n] = 0;
    return 1;
}

// A number: 1 with `whole` set when it has no fraction or exponent, and its value (held to 32 bits) in `value`.
static int scan_number(Scan* s, int* whole, int* value) {
    scan_space(s);
    const char* start = s->at;
    int negative = 0;
    u32 v = 0, digits = 0;
    if (s->at < s->end && *s->at == '-') negative = 1, ++s->at;
    while (s->at < s->end && *s->at >= '0' && *s->at <= '9') {
        if (v < 100000000u) v = v * 10u + (u32)(*s->at - '0');
        ++s->at;
        ++digits;
    }
    if (digits == 0) return 0;
    *whole = 1;
    if (s->at < s->end && (*s->at == '.' || *s->at == 'e' || *s->at == 'E')) {
        *whole = 0;
        while (s->at < s->end && ((*s->at >= '0' && *s->at <= '9') || *s->at == '.' || *s->at == 'e' || *s->at == 'E' || *s->at == '+' || *s->at == '-')) ++s->at;
    }
    *value = negative ? -(int)v : (int)v;
    return s->at != start;
}

static int scan_word(Scan* s, const char* word) {
    scan_space(s);
    u32 n = 0;
    while (word[n]) {
        if (s->at + n >= s->end || s->at[n] != word[n]) return 0;
        ++n;
    }
    s->at += n;
    return 1;
}

// Any value, nested at most DEPTH_MOST deep; `ints` counts the whole numbers of an array met at the top of it.
static int scan_value(Scan* s);

static int scan_array(Scan* s, int* ints, int* lows, int* highs, int low, int high, u32 most) {
    scan_space(s);
    if (s->at >= s->end || *s->at != '[') return 0;
    ++s->at;
    *ints = 0;
    *lows = *highs = 0;
    scan_space(s);
    if (s->at < s->end && *s->at == ']') { ++s->at; return 1; }
    for (u32 n = 0;; ++n) {
        if (n >= most) return 0;
        int whole = 0, value = 0;
        scan_space(s);
        if (s->at < s->end && (*s->at == '-' || (*s->at >= '0' && *s->at <= '9'))) {
            if (!scan_number(s, &whole, &value) || !whole) return 0;
            ++*ints;
            if (value < low) ++*lows;
            if (value > high) ++*highs;
        } else {
            return 0;
        }
        scan_space(s);
        if (s->at < s->end && *s->at == ',') { ++s->at; continue; }
        if (s->at < s->end && *s->at == ']') { ++s->at; return 1; }
        return 0;
    }
}

static int scan_value(Scan* s) {
    scan_space(s);
    if (s->at >= s->end) return 0;
    char c = *s->at;
    if (c == '"') {
        char skip[4];
        return scan_text(s, skip, 4);
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        int whole = 0, value = 0;
        return scan_number(s, &whole, &value);
    }
    if (c == 't') return scan_word(s, "true");
    if (c == 'f') return scan_word(s, "false");
    if (c == 'n') return scan_word(s, "null");
    if (c != '[' && c != '{') return 0;
    if (++s->depth > (int)DEPTH_MOST) return 0;
    char close = c == '[' ? ']' : '}';
    ++s->at;
    scan_space(s);
    if (s->at < s->end && *s->at == close) {
        ++s->at;
        --s->depth;
        return 1;
    }
    for (;;) {
        if (close == '}') {
            char key[4];
            if (!scan_text(s, key, 4)) return 0;
            scan_space(s);
            if (s->at >= s->end || *s->at != ':') return 0;
            ++s->at;
        }
        if (!scan_value(s)) return 0;
        scan_space(s);
        if (s->at < s->end && *s->at == ',') { ++s->at; continue; }
        if (s->at < s->end && *s->at == close) { ++s->at; --s->depth; return 1; }
        return 0;
    }
}

enum { SOUND_FINE = 0, SOUND_NOT_OBJECT = 1, SOUND_MEMBER = 2, SOUND_TYPE = 3, SOUND_KNOBS = 4, SOUND_OTHER_ENGINE = 5, SOUND_TWICE = 6, SOUND_VALUE = 7 };

static const char* sound_reason(u32 why) {
    switch (why) {
    case SOUND_NOT_OBJECT: return "it is not one JSON object";
    case SOUND_MEMBER: return "it holds a member the unit's sounds do not hold";
    case SOUND_TYPE: return "its type is not dsynth";
    case SOUND_KNOBS: return "its knobs are not eight whole numbers inside dsynth's rows";
    case SOUND_OTHER_ENGINE: return "it names another engine";
    case SOUND_TWICE: return "it holds a member twice";
    default: return "one of its members does not hold what the unit's sounds hold there";
    }
}

static u32 sound_check(const Engine* e, const Op1fSound* sound, u32* has_member) {
    Scan s;
    s.at = sound->text;
    s.end = sound->text + sound->bytes;
    s.depth = 0;
    *has_member = 0;
    scan_space(&s);
    if (s.at >= s.end || *s.at != '{') return SOUND_NOT_OBJECT;
    ++s.at;
    s.depth = 1;
    u32 seen = 0, typed = 0, knobbed = 0;
    scan_space(&s);
    if (s.at < s.end && *s.at == '}') return SOUND_TYPE;
    for (;;) {
        char key[16];
        if (!scan_text(&s, key, 16)) return SOUND_NOT_OBJECT;
        u32 which = 0;
        while (SOUND_MEMBERS[which][0] && !same_text(key, SOUND_MEMBERS[which], 16)) ++which;
        if (!SOUND_MEMBERS[which][0]) return SOUND_MEMBER;
        if (seen & (1u << which)) return SOUND_TWICE;
        seen |= 1u << which;
        scan_space(&s);
        if (s.at >= s.end || *s.at != ':') return SOUND_NOT_OBJECT;
        ++s.at;
        int ints = 0, lows = 0, highs = 0, whole = 0, value = 0;
        char text[24];
        if (which == (u32)M_KNOBS) {
            int low = dsynth_low[0], high = dsynth_high[0];
            for (u32 i = 1; i < 8u; ++i) {                                          // the narrowest of dsynth's eight rows holds every knob
                if (dsynth_low[i] > low) low = dsynth_low[i];
                if (dsynth_high[i] < high) high = dsynth_high[i];
            }
            if (!scan_array(&s, &ints, &lows, &highs, low, high, 8u) || ints != 8 || lows || highs) return SOUND_KNOBS;
            knobbed = 1;
        } else if (which == (u32)M_ADSR || which == (u32)M_FX_PARAMS || which == (u32)M_LFO_PARAMS) {
            if (!scan_array(&s, &ints, &lows, &highs, -32768, 32767, 8u) || ints != 8 || lows || highs) return SOUND_VALUE;
        } else if (which == (u32)M_TYPE) {
            if (!scan_text(&s, text, 24)) return SOUND_TYPE;
            if (!same_text(text, DSYNTH_NAME, 24)) return SOUND_TYPE;
            typed = 1;
        } else if (which == (u32)M_ENGINE) {
            if (!scan_text(&s, text, 24) || !same_text(text, e->name, NAME)) return SOUND_OTHER_ENGINE;
            *has_member = 1;
        } else if (which == (u32)M_FX_TYPE || which == (u32)M_LFO_TYPE) {
            if (!scan_text(&s, text, 16) || !in_list(text, which == (u32)M_FX_TYPE ? SOUND_EFFECTS : SOUND_LFOS)) return SOUND_VALUE;
        } else if (which == (u32)M_NAME || which == (u32)M_ORIGINAL_FOLDER) {
            if (!scan_text(&s, text, 24)) return SOUND_VALUE;
        } else if (which == (u32)M_FX_ACTIVE || which == (u32)M_LFO_ACTIVE) {   // true or false, or 0 or 1 as some of the unit's own sounds hold
            if (!scan_word(&s, "true") && !scan_word(&s, "false") && (!scan_number(&s, &whole, &value) || !whole || value < 0 || value > 1)) return SOUND_VALUE;
        } else if (which == (u32)M_OCTAVE) {
            if (!scan_number(&s, &whole, &value) || !whole || value < -8 || value > 8) return SOUND_VALUE;
        } else if (which == (u32)M_SYNTH_VERSION) {
            if (!scan_number(&s, &whole, &value) || !whole || value < 0 || value > 1000) return SOUND_VALUE;
        } else {                                                                    // mtime: a number
            if (!scan_number(&s, &whole, &value)) return SOUND_VALUE;
        }
        scan_space(&s);
        if (s.at < s.end && *s.at == ',') { ++s.at; continue; }
        if (s.at < s.end && *s.at == '}') { ++s.at; break; }
        return SOUND_NOT_OBJECT;
    }
    scan_space(&s);
    if (s.at != s.end) return SOUND_NOT_OBJECT;
    if (!typed) return SOUND_TYPE;
    if (!knobbed) return SOUND_KNOBS;
    return SOUND_FINE;
}

// Every sound of every registered engine checked once (dsynth's rows are read by then): an engine with a sound the base
// cannot write writes none, with a line.
static void check_sounds(Engine* e) {
    if (e->sounds_checked) return;
    e->sounds_checked = 1;
    for (u32 i = 0; i < e->table->sounds; ++i) {
        u32 member = 0, why = sound_check(e, &e->table->sound[i], &member);
        if (why == SOUND_FINE) continue;
        e->sounds_checked = 2;
        char words[160];
        u32 at = put(words, 0, 160, "its sound '");
        at = put(words, at, 160, e->table->sound[i].name);
        at = put(words, at, 160, "' is not one the base can write: ");
        put(words, at, 160, sound_reason(why));
        op1f_log(2, LOG_FILE, 17, "engine: '%s' writes no sounds: %s", e->name, words);
        return;
    }
}

static u32 folder_path(char* out, const Engine* e, int slash) {
    u32 at = put(out, 0, PATH, SOUNDS_ROOT);
    at = put(out, at, PATH, e->name);
    if (slash) at = put(out, at, PATH, "/");
    return at;
}

// One sound written into its engine's folder, from its text with the base's member at the top when the text has none:
// to the base's temporary file, then given its name, only when no file of that name is there. Answers 1 when it landed.
static u32 temporary_made;

static int write_sound(const Engine* e, const Op1fSound* sound, const char* path) {
    u32 member = 0;
    sound_check(e, sound, &member);
    u32 at = 0;
    const char* text = sound->text;
    u32 bytes = sound->bytes;
    if (!member) {                                                                  // '{' and what follows it, with the member between
        u32 open = 0;
        while (open < bytes && text[open] != '{') ++open;
        u32 rest = open + 1u;
        while (rest < bytes && (text[rest] == ' ' || text[rest] == '\t' || text[rest] == '\n' || text[rest] == '\r')) ++rest;
        at = put(file_text, 0, SOUND_ROOM, "{\"op1f_engine\": \"");
        at = put(file_text, at, SOUND_ROOM, e->name);
        at = put(file_text, at, SOUND_ROOM, rest < bytes && text[rest] == '}' ? "\"" : "\", ");
        for (u32 k = rest; k < bytes && at + 1u < (u32)SOUND_ROOM; ++k) file_text[at++] = text[k];
        text = file_text;
        bytes = at;
    }
    if (!temporary_made) temporary_made = STOCK_MAKE_FOLDERS(TEMPORARY_FOLDER) != 0 ? 1u : 0u;   // the firmware's own, there on every unit; made if not
    int handle = STOCK_OPEN(TEMPORARY, WRITE_MODE);
    if (handle < 0) return 0;
    int done = STOCK_WRITE(text, 1, bytes, handle);
    STOCK_CLOSE(handle);
    if (done != (int)bytes) return 0;
    if (STOCK_FILE_SIZE(path) >= 0) return 0;                                       // a file of that name came meanwhile: never written over
    if (!STOCK_RENAME(TEMPORARY, path)) return 0;
    return STOCK_FILE_SIZE(path) == (int)bytes;
}

static u32 sounds_next;                                // the engine whose sounds come next
static u32 room[ENGINES];                              // R6d: 0 not weighed this start, ROOM_GIVEN, ROOM_REFUSED
static u32 room_taken;                                 // the sounds given room this start: the firmware counts them from the next
static u32 sounds_written[ENGINES];                    // this start's, an engine
static u32 owners_said;                                // a bit an engine: its "not one the base made" line was said
static u32 sound_failures[ENGINES];

// R6d: the sounds of an engine's list that its folder does not hold (all of them where it has no folder).
static u32 missing_sounds(const Engine* e, int there) {
    if (!there) return e->table->sounds;
    u32 missing = 0;
    for (u32 i = 0; i < e->table->sounds; ++i) {
        char path[PATH];
        u32 at = folder_path(path, e, 1);
        at = put(path, at, PATH, e->table->sound[i].name);
        put(path, at, PATH, ".json");
        if (STOCK_FILE_SIZE(path) < 0) ++missing;
    }
    return missing;
}

// R6d: whether an engine's missing sounds may be written this start, weighed once: all of them when the unit's user
// sounds (the firmware's count, with those given room earlier this start) and these leave USER_FREE free for the
// owner's own saves, else none; a line either way (none where nothing is missing: the version is then recorded). 0
// while the count is not to be had.
static int weigh_room(u32 k, int there) {
    Engine* e = &engines[k];
    u32 count = 0, most = 0;
    if (!user_sounds(&count, &most)) return 0;
    u32 missing = missing_sounds(e, there), held = count + room_taken;
    if (missing == 0 || held + missing + (u32)USER_FREE <= most) {
        room[k] = ROOM_GIVEN;
        room_taken += missing;
        if (missing)
            op1f_log(3, LOG_FILE, 21, "engine: '%s' writes its %u sound(s): the unit holds %u user sound(s) of the %u it takes, %u stay free after them", e->name,
                     missing, held, most, most - held - missing);
    } else {
        room[k] = ROOM_REFUSED;
        op1f_log(2, LOG_FILE, 22, "engine: '%s' writes no sounds: the unit holds %u user sound(s) of the %u it takes, and its %u would leave fewer than %u free",
                 e->name, held, most, missing, (u32)USER_FREE);
    }
    return 1;
}

// At most one file a frame, after the first frame, never while the disk mode is open: for the next engine whose pack's
// version has not all its sounds in its folder, the folder made, or one missing sound written.
static void sounds_frame() {
    u32 n = op1f_engines_registered;
    if (n == 0 || RD32(DISK_OPEN) != 0) return;
    if (store_dirty) {
        store_write();
        return;
    }
    for (u32 tries = 0; tries < n; ++tries) {
        u32 k = sounds_next;
        sounds_next = sounds_next + 1u >= n ? 0u : sounds_next + 1u;
        Engine* e = &engines[k];
        if (e->table->sounds == 0 || sound_failures[k] >= 3u) continue;
        check_sounds(e);
        if (e->sounds_checked != 1u) continue;
        Kept* st = kept_of(e->name);
        if (st == 0 || st->version == e->version) continue;
        char folder[PATH];
        folder_path(folder, e, 0);
        int there = STOCK_IS_FOLDER(folder);
        if (there && !st->made) {                                                   // a folder of the owner's: never touched
            if ((owners_said & (1u << k)) == 0) {
                owners_said |= 1u << k;
                op1f_log(2, LOG_FILE, 17, "engine: '%s' writes no sounds: the folder synth_presets/%s is not one the base made", e->name, e->name);
            }
            continue;
        }
        if (room[k] == 0) {                                                         // R6d: the room weighed first, once a start, a frame of its own
            weigh_room(k, there);
            return;
        }
        if (room[k] != (u32)ROOM_GIVEN) continue;                                   // no room: nothing of this engine's this start
        if (!there) {
            if (!st->made) {                                                        // the store says the folder is the base's before it is made
                st->made = 1;
                store_dirty = 1;
                return;
            }
            char made[PATH];
            folder_path(made, e, 1);
            if (!STOCK_MAKE_FOLDERS(made) || !STOCK_IS_FOLDER(folder)) {
                if (++sound_failures[k] == 3u) op1f_log(2, LOG_FILE, 18, "engine: '%s' writes no sounds: the folder %s could not be made", e->name, folder);
            }
            return;
        }
        for (u32 i = 0; i < e->table->sounds; ++i) {
            const Op1fSound* sound = &e->table->sound[i];
            char path[PATH];
            u32 at = folder_path(path, e, 1);
            at = put(path, at, PATH, sound->name);
            put(path, at, PATH, ".json");
            if (STOCK_FILE_SIZE(path) >= 0) continue;                               // there: never written over
            if (write_sound(e, sound, path)) ++sounds_written[k];
            else if (++sound_failures[k] == 3u) op1f_log(2, LOG_FILE, 18, "engine: '%s' writes no sounds: %s could not be written", e->name, path);
            return;
        }
        st->version = e->version;                                                   // every sound of this version is in the folder
        store_dirty = 1;
        op1f_log(3, LOG_FILE, 19, "engine: '%s' wrote %u sound(s) into synth_presets/%s", e->name, sounds_written[k], e->name);
        return;
    }
}

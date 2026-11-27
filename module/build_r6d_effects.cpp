// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Base 38 (R16), the base pack: base 37's effects unit (base 35's with R13's list starts), edited in place, with hasis's open-field pull request 38 (made
// on base 32's unit) and two mends of its review. The runs inside packs: op1f_rack_ready kept a record for every pack's
// place and effect ever asked, 64 at most, and then answered "no room" (a link of a chain walked through the list used
// them up); now a record no place has run at the last two frames' looks is taken over before a new one is made, one of
// the same effect first, its memory with it. That memory is cleared again over the frames, a quarter of a megabyte a
// frame as a new block is (the review: not in one go inside a pack's frame call); meanwhile op1f_rack_ready answers
// OP1F_NOT_NOW, op1f_rack_run OP1F_NOT_HERE and no other place takes it, then `reset` and `fresh` come as for a new
// record. A switch-off for its cost was until the next start; now a place of the unit's gets another go when an effect
// is picked there again (any row of its list: the review, a layer's row and the row it rides too, which hasis's change
// left out), a run inside a pack when its place picks it again after another, or when its record is taken over (both as
// hasis made them), or when its place asks for it again after a pause (fx train's wagon put on one of the
// unit's own effects, which the base never sees, and back). What base 35's unit said follows and holds, but for "until the next start".
//
// Base 35 (R11), the base pack: base 33's effects unit, edited in place, with layers (module/build_r6e_card.h, "Base 35:
// layers"), for every pack alike. An effect with OP1F_EFFECT_LAYER rides over whichever of the unit's own effects the
// place holds: its ride follows no type word and ends by a pick of another row, a sound loaded without the base's
// member naming it, or op1f_layer_end; the unit's object plays first, then the layer's `process` (`on` 1), and while
// the unit's effect is off, op1f_effects_after calls it with `on` 0; the switch is the owner's in the files (W5 to W8
// hand it through) and the member names the layer with no "on". The lists open with the cursor on the row the place
// rides, for every ride (EH, R13: op1f_fx_start_do for a sound's; C3: op1f_fx_master_start), which also keep what the
// place held as the list opened (its type, eight words and switch): a pick of a layer's row puts that back under it
// (the firmware's pick of that type's row, the words, the switch at the first frame after that pick has returned, the
// master's at its return too), and a pick of the row the place
// rides already, the type unchanged, picks nothing again. The page: the unit's own while the layer's choice is
// OP1F_LAYER_PAGE_UNIT (stand() answers 0), the layer's for the screen of the type the place holds with
// OP1F_LAYER_PAGE_OWN. The helpers op1f_layer_start, op1f_layer_end and op1f_layer_page serve kind 7 here and hand an
// ask of kind OP1F_ASK_LFO to the LFO unit (weak names); the mark op1f_layer_1; at each frame's end, the LFO unit's
// frame (weak). What base 30's unit said follows and holds.
//
// Step R6d, base 30, the release's base: the effects unit's sibling. A sibling of base 29's effects unit (base
// 29), which stays as it was: everything of it is here as it was, its stubs are still module/build_r5_effects.s, and
// these are added or changed:
//   a value as heard (level 10): op1f_param_heard, beside level 8's helpers and through their table of kinds, for an
//       effect and (through the engines unit's op1f_engines_param_heard) an engine; and the mark op1f_level_10. Each
//       place's values as the pack's effect was last called with them, the LFOs applied, kept by the audio thread
//       between two steps of a sequence word; the value as set where the place has not played in the last few blocks
//   a sound loaded onto a slot (base 29's review, F-1): where the loaded sound's effect (its document's fx_type) is
//       not the slot's type word yet at the tell, the place waits, its member unread and its ride left as it stands,
//       until the firmware's apply has put it there (the type word then the document's, or no longer the one before),
//       128 frames at most, so that a ride is never let go on the effect of the sound that was there before
//   and a ride that ends at a load, a start or a drop no longer makes the firmware's object from the display thread:
//       the firmware's apply makes it anew (the reading of the crash at base 29's try on a unit)
// What base 29's unit said follows and holds.
//
// Step R6, the base pack 29: the effects of level 8, with the helpers serving level 9's engines. A sibling of
// base 28's effects unit, which stays as it was: everything of it is here as it was, its stubs are still
// module/build_r5_effects.s, and one thing is added. The table of the kinds that have values (VALUED), through which
// op1f_param_get, op1f_param_set and op1f_riding find an ask by its kind, gains one row: kind 8, OP1F_ASK_ENGINE, its
// places the synth's eight slots, its three functions the engines unit's (op1f_engines_param_get, _param_set,
// _riding, base 29's engines unit), each held to the helpers' answers (an ask that is no registered engine answers
// OP1F_BAD_NAME, as an ask of a kind with no row did). The engines' names on the synth's LFO page were base 28's
// already (op1f_engines_names, the weak name of W4). The engines unit's names are weak here: away, this unit is base
// 28's in all it does. Two mends of base 28's review are here too: R-1, a ride taken up
// from the base's member sets the sound's fx_active to 0 as a pick does (read_member); R-2, a ride that ends other than
// by a pick takes back the stand-in pair its pick wrote, where it still holds it (end_ride, take_back_pair). What base
// 28's unit said follows and holds.
//
// Step R5, the base pack 28: the effects, level 8 (OS 1.7.0+0). A unit of its own, new at R5.
//
// A pack adds an effect through an ask of its card (OP1F_ASK_EFFECT, its table an Op1fEffect, module/build_r5_card.h).
// The effect stands on one of eight of the unit's own effects (`rides`): the unit keeps that effect's name, its four
// knob words and its switch in the sound's file and plays it nowhere; the base makes the pack's effect do the work in
// its place, wherever the unit runs its effect for that sound or for the master. What the units of the base share is in
// module/build_r5_base.h; the eight places of the firmware it stands at are named there, W1 to W8.
//
// What this unit does, by where it runs:
//   the catalog's read (the start's thread, op1f_effects_catalog_read, called by the sequencer unit's sibling): the
//       first call checks every ask of kind OP1F_ASK_EFFECT of the cards in use and registers those in order, ids 15
//       and on; every call adds the registered effects' rows to the document's `fx_types`. No line: the lines come at
//       the first frame
//   an effect object made (W1, op1f_fx_make, any thread): the firmware's factory, then, with an effect registered, the
//       object wrapped in one of two objects of the base's own an index, which hand every call on to the firmware's
//       object but `process`
//   a block (the audio thread, the wrapping object's `process`): the pack's effect for a place that rides one, the
//       firmware's object's own `process` for every other
//   after the effect (the audio thread, op1f_effects_after, from the stages unit): an effect that keeps its tails, for
//       a place that rides it switched off
//   a pick (the key thread: W2 for a sound, W3 for the master): a ride begun or ended, then the firmware's own pick
//   the firmware's switch of the effect (W5, W6, W7, W8): the files never say that a riding place's effect is on
//   the knob names of the LFO pages (W4): the pack's names of its values for a place that rides; for an engine's
//       values, what a unit of a later level offers (op1f_engines_names, weak)
//   a frame (the display thread, op1f_effects_frame, from the cards unit): the lines, the rides held against the type
//       words, the base's own member op1f_effect read and written through the values unit, the working memory, the
//       `reset` calls, the period's lines
//   the helpers op1f_param_get, op1f_param_set, op1f_riding, op1f_rack_at, op1f_rack_list, op1f_rack_ready,
//       op1f_rack_run, as module/build_r5_card.h words them
//
// A place is 0 to 7 for a sound of the synth (its slot), 8 to 15 for a sound of the drum (8 + its slot), 16 for the
// master. Index 0 of the firmware's effect objects is the instrument's (the sound that plays, synth or drum), index 1
// the master's.
//
// What the firmware needs of these, where the code meets it:
//   the factory (type, index) answers an object, or 0; the builder (index, type) clears the index's word, calls the
//       factory, calls the object's init (+0x0c) with the index and publishes it
//   an effect object's first word is a table of four functions: two destructors (+0x00, +0x04), process (+0x08) with
//       (the object, the left buffer, the right buffer, the eight knob words), init (+0x0c) with (the object, index)
//   the instrument whose block runs (0 the synth, 1 the drum) and the slot each instrument built; a place's type
//       word, its on byte and its eight knob words (a value shifted left by sixteen)
//   a knob of an effect type: three halfwords, the lowest (signed), the highest (unsigned), the fine step
//   the catalog's rows are jsoncpp values of twelve bytes, the kind byte at +4 (1 an integer, 2 an unsigned one, 4 a
//       string, 6 an array, 7 an object); each function is handed only the kinds it takes
//
// The threads. The registration on the thread of the start, before any effect object is made; the making of an
// object on any thread; a block on the audio thread; a pick on the key thread; the switch's places on any thread; the
// frame and the helpers' work on the display thread. Nothing here says a line but from the display thread; nothing
// waits; nothing is divided by a number that is not a power of two (the module links no runtime) and nothing is
// multiplied in 64 bits. The compiler counts with an index register in the catalog's read alone (I2 += M0): it is
// reached by a call of the sequencer unit's compiled code, where the length registers are 0 as at every such call;
// the audio thread's way counts with none (the PC's tests hold both). Compiled for the PC as it is, with OP1F_HOST
// defined: the stand-ins come in through a header of the PC's own (for tests this tree does not hold).

#include "build_r6d_base.h"                            // R6d: base 28's shared names, level 9's and level 10's
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

typedef void* (*FactoryFn)(u32 type, u32 index);
typedef void (*BuildFn)(u32 index, u32 type);
typedef void (*SetActiveFn)(u32 record, u32 on);
typedef void (*SetParamsFn)(u32 record, const u32* words);
typedef void (*SetOnFn)(u32 slot, u32 on);
typedef void (*SetMasterOnFn)(u32 on);
typedef u32 (*GetMasterOnFn)();
typedef void (*MasterPickFn)(void* controller, u32 id);
typedef u32 (*ScreenOfFn)(u32 type);
typedef void (*PickFn)(u32 record, u32 id);
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
typedef u32 (*ObjectFn)(void* self, u32 a, u32 b, u32 c);
typedef int (*StartDoFn)(void* record);                 // R13: a sound's list start (its record -> the row's id)
typedef u32 (*LookupFn)(u32 database, const char* key, const void* name, void* out);

// R11 (base 35): the words of base 35's header this unit uses (it reads base 29's header, where they are not).
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_EFFECT_LAYER = 4, OP1F_ASK_LFO = 11, OP1F_LAYER_PAGE_UNIT = 0, OP1F_LAYER_PAGE_OWN = 1 };
#endif
// R22 (base 45): the words of base 45's part "hosted pages" this unit uses.
#ifndef OP1F_BUILD_R6E_CARD_H
enum : unsigned { OP1F_PLACE_NONE = 0xffffffffu };
enum { OP1F_HOSTED_KEPT_MOST = 128, OP1F_HOSTED_PAGE_UP = 256, OP1F_EFFECT_HOSTED_AT = 184, OP1F_EFFECT_HOSTED_BYTES = 188, OP1F_HOSTED_WAGONS = 1,
       OP1F_KEEP_HOSTED = 16 };
#endif

// The room of the pick's entry (module/build_r5_effects.s): the firmware's pick as it always was. The other two rooms
// are the entries' alone.
extern "C" u8 op1f_fx_pick_room[] OP1F_NAMED("op1f_fx_pick_room");
// R13: the room of EH, the head of the firmware's function both sounds' list starts call (its only callers): called
// as a function (the record), it answers the firmware's start as it always did.
extern "C" u8 op1f_layer_eh_room[] OP1F_NAMED("op1f_layer_eh_room");

#ifdef OP1F_HOST
#define OP1F_EFFECTS_R5_UNIT
#include "mock_effects_r6d.h"                           // the PC's stand-ins: the firmware's memory and calls, the masks, the clock (R6d: a record's document)
#define EFFECTS_APART
#else
#define RD32(a) (*(volatile u32*)(a))
#define RD16(a) (*(volatile u16*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
#define EFFECTS_APART __attribute__((noinline))
// The processor's cycle counter, read with a memory clobber so that the compiler moves no load or store of the call it
// times across the stamp.
static inline u32 cycles_now() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
#define STOCK_FACTORY ((FactoryFn)0x8f011068u)
#define STOCK_BUILD ((BuildFn)0x8d505390u)
#define STOCK_SET_ACTIVE ((SetActiveFn)0x8d4f3c24u)
#define STOCK_SET_PARAMS ((SetParamsFn)0x8d4f3b9cu)
#define STOCK_ON_SYNTH ((SetOnFn)0x8d504320u)
#define STOCK_ON_DRUM ((SetOnFn)0x8d5042f0u)
#define STOCK_ON_MASTER ((SetMasterOnFn)0x8d5042c0u)
#define STOCK_MASTER_ON ((GetMasterOnFn)0x8d50424cu)
#define STOCK_MASTER_PICK ((MasterPickFn)0x8c0bb684u)
#define STOCK_SCREEN_OF ((ScreenOfFn)0x8fc273f0u)
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
#define ROOM_PICK ((PickFn)(void*)op1f_fx_pick_room)
#define RECORD_DOCUMENT(record) ((const void*)((record) + 8u))   // R6d: a slot's record holds its sound's document 8 bytes in
#define ROOM_START ((StartDoFn)(void*)op1f_layer_eh_room)  // R13: the firmware's start, through EH's room (never its own address)
#define STOCK_LOOKUP ((LookupFn)0x8d4f221cu)                // R11: the catalog's lookup (1, the group's key, a string, the out value)
// R11: the LFO unit's (module/build_r11_lfos.cpp): an ask of kind OP1F_ASK_LFO at the three helpers, and its frame.
// Weak: away, an LFO ask answers as a bad ask, and the frame calls nothing.
extern "C" int op1f_lfos_layer_start(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_lfos_layer_start") __attribute__((weak));
extern "C" int op1f_lfos_layer_end(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_lfos_layer_end") __attribute__((weak));
extern "C" int op1f_lfos_layer_page(const Op1fCard* card, unsigned ask, unsigned which) OP1F_NAMED("op1f_lfos_layer_page") __attribute__((weak));
extern "C" void op1f_lfos_frame() OP1F_NAMED("op1f_lfos_frame") __attribute__((weak));
extern "C" unsigned op1f_lfos_riding(const Op1fCard* card, unsigned ask) OP1F_NAMED("op1f_lfos_riding") __attribute__((weak));   // bit n for place n
// A unit of a later level may offer the names of an engine's values at a synth slot (a C text, or 0). Weak: the loader
// sets it to 0 while no unit offers it, and the firmware's own function answers then.
extern "C" const char* op1f_engines_names(unsigned slot, int index) OP1F_NAMED("op1f_engines_names") __attribute__((weak));
#define ENGINES_NAMES (&op1f_engines_names)
#endif

// The firmware's words (an enum of their own: they do not fit a signed number).
enum : u32 {
    PLAYING = 0x11902da4u,              // the instrument whose block runs: 0 the synth, 1 the drum (the block writes it before it builds)
    SYNTH_BUILT = 0x84a01524u,          // the slot the synth's object was built for, the drum's behind it
    DRUM_BUILT = 0x84a01528u,
    SYNTH_SHOWN = 0x11904704u,          // the slot the GUI shows, the synth's and the drum's
    DRUM_SHOWN = 0x11904708u,
    SYNTH_TYPES = 0x11904794u,          // a slot's effect type, a word a slot
    DRUM_TYPES = 0x119047b4u,
    MASTER_TYPE = 0x11902e24u,
    SYNTH_ON = 0x84a0154cu,             // a slot's effect switched on, a byte a slot
    DRUM_ON = 0x84a0155cu,
    MASTER_ON = 0x84a0156cu,
    SYNTH_WORDS = 0x119036e0u,          // a slot's eight knob words, 32 bytes a slot
    DRUM_WORDS = 0x119039e0u,
    MASTER_WORDS = 0x11903be0u,         // the master's eight, which its block hands its effect
    STOCK_CTX = 0x11904754u,            // the eight words the instrument's block hands its effect, copied each block
    EFFECT_WORD = 0x84a03c84u,          // the effect objects: index 0, index 1 behind it
    SYNTH_RECORDS = 0x080235dcu,        // a slot's record, 52 bytes a slot
    DRUM_RECORDS = 0x0802377cu,
    KNOB_ROWS = 0x840afe60u,            // 48 bytes a type, 6 a knob
    PERIOD = 0x84d20e80u,               // samples a 96th of a beat at 44,100 a second, times 2^18
    TRANSPORT = 0x11905a04u,            // not 0 while the tape plays
    CURRENT_MODE = 0x8400f978u,         // the mode: 0 synth, 1 drum, 2 tape, 3 mixer
    LFO_HOST = 0x0802fea8u,             // the instrument the LFO pages name: 0 the synth
    EFFECT_NAMES = 0x8490e288u          // the firmware's eleven effects: a name's address and its type, eight bytes each
};

enum {
    EFFECTS = OP1F_EFFECTS_MOST, VALUES = OP1F_EFFECT_VALUES_MOST, PLACES = 17, MASTER = OP1F_PLACE_MASTER, DRUM = OP1F_PLACE_DRUM,
    ID_FIRST = 15, NAME = OP1F_KIND_NAME, FIELD = 24, STOCK_COUNT = 11, TABLE_BYTES = 180, BLOCK_BYTES = 44, ENTRY_BYTES = 20,
    RUN_BYTES = 36, FRAMES = 64, CARDS = OP1F_SLOTS_MOST, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, RACK_PLACES = OP1F_RACK_PLACES,
    RACKS = 64, ASIDES_KEPT = 16, RIDE_LINES = 32, PERIOD_FRAMES = 256, RECORD_BYTES = 52, SLOTS = 8, ROW_BYTES = 48, KNOB_BYTES = 6,
    KIND_INT = 1, KIND_UINT = 2, KIND_STRING = 4, KIND_ARRAY = 6, KIND_OBJECT = 7, KIND_AT = 4, MEMBER_ROOM = 96,
    NAMES_KIND_ENGINE = 0, NAMES_KIND_EFFECT = 2, STATE_ALIGN_LEAST = 4, CALL_SHOWN = OP1F_CALL_SHOWN
};
static const u32 CYCLES_MOST = (u32)OP1F_EFFECT_CYCLES_MOST, CYCLES_OFF = (u32)OP1F_STAGE_CYCLES_OFF, OVER_CALLS = (u32)OP1F_STAGE_OVER_CALLS,
                 SPARE = (u32)OP1F_STAGE_CYCLES_SPARE, STATE_MOST = (u32)OP1F_EFFECT_STATE_MOST, ALIGN_MOST = (u32)OP1F_MEMORY_ALIGN_MOST;
static const u32 SECOND = 400000000u;                  // the processor's cycles in a second
static_assert(sizeof(Op1fEffect) == TABLE_BYTES && sizeof(Op1fEffectBlock) == BLOCK_BYTES && sizeof(Op1fRackEntry) == ENTRY_BYTES &&
              sizeof(Op1fRackRun) == RUN_BYTES, "level 8's tables as the header has them");

static const char LOG_FILE[] = "../src/main/op1f_effects.cpp";
static const char KEY_LIST[] = "fx_types";
static const char KEY_TYPE[] = "type";
static const char KEY_ID[] = "id";
static const char KEY_DEFAULTS[] = "default_params";
static const char KEY_FX_TYPE[] = "fx_type";              // R6d: a sound's effect, in its document

// The unit's own effects, by name, in fixed fields of 24 bytes; and whether a pack's effect may stand on each: never
// on vocoder (never valid), mother (not valid in the classic mode) or fazer (hidden until it is unlocked).
static const char STOCK_NAMES[STOCK_COUNT][FIELD] = {"delay", "grid", "punch", "phone", "spring", "mother", "nitro", "cwo", "vocoder", "terminal", "fazer"};
static const u8 RIDEABLE[STOCK_COUNT] = {1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0};

// The mark of a base of level 8.
extern "C" const u32 op1f_level_8 OP1F_NAMED("op1f_level_8");
const u32 op1f_level_8 = 8;
// R6d: the mark of a base of level 10 (op1f_param_heard, below; module/build_r6d_base.h).
const u32 op1f_level_10 = 10;

// ---- from the cards unit ----

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
// The card's slot + 1 while the base calls that card's frame function on the caller's stack, else 0.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");

// ---- the words other units read ----

volatile unsigned op1f_effects_registered;
// The stand-in entries (module/build_r5_base.h): four pairs {mode + 1, screen}, the effects unit writing the pairs
// OP1F_STAND_EFFECT and OP1F_STAND_MASTER at its picks; the other two are later levels'.
volatile unsigned op1f_stand_screens[2 * OP1F_STAND_ENTRIES];

// ---------------------------------------------------------------- the registry

struct Effect {
    const Op1fEffect* table;
    u32 number;                                        // its place in the registry: its id less 15
    u32 slot, ask;                                     // the card's slot in op1f_cards_2, the ask's number in its list
    u32 type;                                          // the type of the unit's effect it stands on
    u32 ridden_id;                                     // that effect's row's id in the lists
    u32 flags, cycles, count, state_bytes, state_align;
    char name[NAME], rides[NAME];
    char value_names[VALUES][NAME];
    int low[VALUES], high[VALUES], start[VALUES];      // the pack's ranges
    int row_low[VALUES], row_high[VALUES];             // the knob rows of the type it stands on, read at the first frame
    u32 q_in[VALUES], f_in[VALUES];                    // word to value: the whole part and the fraction (16 bits, rounded up) of the ratio
    u32 q_out[VALUES], f_out[VALUES];                  // value to word, the same
    u32 fixed[VALUES];                                 // a row with no span: the value stands at its start
    u32 screen;                                        // the effect screen of the type it stands on
    volatile u32 calls, sum, most, over;               // this period's, all places: the audio thread writes, the display thread reads and clears masked
    volatile u32 page;                                 // R11: a layer's page choice, OP1F_LAYER_PAGE_UNIT or _OWN
    u32 hosted;                                        // R22 (base 45): it declares wagons (OP1F_HOSTED_WAGONS at +184)
};
static Effect effects[EFFECTS];
static Op1fRackEntry entries[EFFECTS];

// What the check found, said at the first frame.
enum { WHY_ORDER = 1, WHY_NAME = 2, WHY_TAKEN = 3, WHY_RIDES = 4, WHY_COST = 5, WHY_SEVENTEENTH = 6 };
struct Aside { u32 slot, why, a, b; char name[NAME], rides[NAME]; };
static Aside asides[ASIDES_KEPT];
static u32 aside_count, aside_more;

struct Candidate { const Op1fEffect* table; u32 slot, ask; };
static Candidate candidates[EFFECTS];
static u32 candidate_count;
static u32 checked;                                    // the asks were looked through (once a start)
enum { LIST_UNREAD = 1, LIST_NOT_OBJECT = 2, LIST_NOT_ARRAY = 3 };
static u32 list_state;                                 // 0 when the effects were registered, else why not
static volatile u32 rows_added;                        // the rows added at the catalog's reads

// ---------------------------------------------------------------- the places, the instances, the wrapping objects

struct Budget {                                        // a place's or a run's: the audio thread writes, the display thread reads
    u32 window_start, window_over, window_most;
    volatile u32 off;                                  // switched off: a place until an effect is picked there again, a run until it is picked there again (R16)
    volatile u32 off_cycles, off_overs;
    u32 off_said;
};

struct Place {
    volatile u32 ride;                                 // the effect's number + 1, 0 for none
    volatile u32 on;                                   // the ride's effect switched on, as the base keeps it
    volatile u32 doc_on;                               // the document's fx_active as the last apply of the slot handed it
    Budget budget;
    u32 member_dirty;                                  // the base's member to be written at a frame (the display thread's)
    u32 absent_said;                                   // the line of a member whose pack is not here was said
};
static Place places[PLACES];

// R6d, level 10: a place's values as the pack's effect was last called with them, the LFOs applied. The audio thread
// writes them at each call between two steps of `seq` (odd while it writes); the display thread reads them between two
// reads of it (op1f_param_heard). `ride` is the ride they are of, `stamp` the cycle counter at the call; `live` is set
// by each call and cleared by the frame once the stamp is older than HEARD_CYCLES (a count of cycles comes round every
// ten seconds).
struct Heard {
    volatile u32 seq, ride, stamp, live;
    volatile int values[VALUES];
};
static Heard heard[PLACES];
static const u32 HEARD_CYCLES = 4643991u;             // eight blocks of 128 frames at 88,200 a second, in the processor's cycles
enum { HEARD_TRIES = 4, WAIT_FRAMES = 128 };

// R6d (base 29's review, F-1): the places whose loaded sound's effect was not the slot's type word at the tell, a bit a
// place (the display thread's): the type the document names, the type word at the tell, the frames waited.
static u32 waiting;
static u32 wait_type[PLACES], wait_from[PLACES], wait_frames[PLACES];

enum { NONE = 0, CLEARING = 1, READY = 2, REFUSED = 3, WIPING = 4 };   // R16: WIPING, a run's kept memory cleared again for its new place
struct Instance { int handle; void* volatile state; volatile u32 status; char whose[64]; };
static Instance instances[EFFECTS][2];                 // an effect's working memory for the instrument, and for the master

struct Wrap;
static u32 wrapped_destroy(Wrap* self, u32 a, u32 b, u32 c);
static u32 wrapped_delete(Wrap* self, u32 a, u32 b, u32 c);
static u32 wrapped_process(Wrap* self, int* left, int* right, const u32* words);
static u32 wrapped_init(Wrap* self, u32 a, u32 b, u32 c);
struct Wrap {
    const void* const* table;                          // the first word, as the firmware reads every effect object
    void* inner;                                       // the firmware's object
    u32 index, type, seq;
    volatile u32 fresh;                                // not yet handed to a pack's `process`
};
static const void* const WRAP_TABLE[4] = {(const void*)&wrapped_destroy, (const void*)&wrapped_delete, (const void*)&wrapped_process,
                                          (const void*)&wrapped_init};
static Wrap wraps[2][2];
static u32 wrap_next[2];
static volatile u32 wrap_seq;
static volatile u32 reset_seq[2];                      // the build whose `reset` was called, an index

// The runs inside a pack: a record a pack's place and an effect. R16 (hasis): a record no place runs any more is taken
// over by the next place that asks for an effect (rack_take, below); `wiped`, the words of its kept memory cleared again.
struct Rack { u32 slot, which, effect; Instance in; Budget budget; volatile u32 fresh; char whose[48]; u32 wiped; };
static Rack racks[RACKS];
static u32 rack_count;
static volatile u8 rack_current[CARDS][RACK_PLACES];   // the record + 1 a pack's place runs, 0 for none
// R16 (base 38): the frame count of each pack's place's last ask (op1f_rack_ready). A pack asks every
// frame while a place holds one of the packs' effects and stops while it holds something else (fx train: one of the
// unit's own effects, which it runs itself and which the base never sees); an ask after a pause of more than
// RACK_PAUSE frames is a new pick of the effect there, as an ask for another effect and then this one is.
static u32 rack_asked[CARDS][RACK_PLACES];
// The review: a pause counts only when the place did not try to run anything either (a pack that asks once and then only
// runs, asking again when op1f_rack_run answers OP1F_NOT_HERE, or one that spreads its asks over frames, must not have a
// switched-off effect armed again every few frames): the frame count at the place's last op1f_rack_run, the audio
// thread's.
static volatile u32 rack_tried[CARDS][RACK_PLACES];
enum { RACK_PAUSE = 2 };

// What other threads tell the display thread: a bit a place, set and taken under a mask.
static volatile u32 picked;                            // a ride begun by a pick
static volatile u32 ended;                             // a ride ended by a pick
static volatile u32 told;                              // the values unit told a load, a start, a drop
static volatile u32 told_load;                         // R6d: the loads among them, whose apply may come after the tell
static volatile u32 back;                              // R11: a pick of a layer's row, or of the row ridden: the switch put back at the frame
static u32 back_on[PLACES];                            // R11: that switch

// R11: what a place held as its list opened (the key thread, EH and C3), put back under a layer at a pick of its row;
// `ride` the place's ride then (the review's S3: only that ride's row picked again picks nothing).
struct Snap { volatile u32 taken; u32 type, on, ride; u32 words[8]; };
static Snap snaps[PLACES];
static u32 stock_ids[STOCK_COUNT];                      // R11: a type's row's id + 1 in the lists, 0 unknown (the catalog's read)
static u32 stock_screens[STOCK_COUNT];                  // R11: a type's effect screen (the first frame)

static struct {
    u32 first_done;
    u32 frames;
    int countdown;
    u32 ride_lines;
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

static u32 place_type(u32 place) {
    if (place < (u32)DRUM) return RD32(SYNTH_TYPES + 4u * place);
    if (place < (u32)MASTER) return RD32(DRUM_TYPES + 4u * (place - (u32)DRUM));
    return RD32(MASTER_TYPE);
}

static u32 place_on(u32 place) {
    if (place < (u32)DRUM) return RD8(SYNTH_ON + place);
    if (place < (u32)MASTER) return RD8(DRUM_ON + place - (u32)DRUM);
    return RD8(MASTER_ON);
}

static void set_place_on(u32 place, u32 on) {
    if (place < (u32)DRUM) STOCK_ON_SYNTH(place, on);
    else if (place < (u32)MASTER) STOCK_ON_DRUM(place - (u32)DRUM, on);
    else STOCK_ON_MASTER(on);
}

static u32 place_words(u32 place) {
    if (place < (u32)DRUM) return SYNTH_WORDS + 32u * place;
    if (place < (u32)MASTER) return DRUM_WORDS + 32u * (place - (u32)DRUM);
    return MASTER_WORDS;
}

static u32 place_record(u32 place) {
    if (place < (u32)DRUM) return SYNTH_RECORDS + (u32)RECORD_BYTES * place;
    return DRUM_RECORDS + (u32)RECORD_BYTES * (place - (u32)DRUM);
}

// A record's place, by its address: 1 and `place` for one of the sixteen slots' records, else 0.
static int record_place(u32 record, u32* place) {
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        if (record == SYNTH_RECORDS + (u32)RECORD_BYTES * s) { *place = s; return 1; }
        if (record == DRUM_RECORDS + (u32)RECORD_BYTES * s) { *place = (u32)DRUM + s; return 1; }
    }
    return 0;
}

// The place whose block runs at index 0 now: the instrument the block runs and the slot it built.
static u32 playing_place() {
    if (RD32(PLAYING) == 0) return RD32(SYNTH_BUILT) & 7u;
    return (u32)DRUM + (RD32(DRUM_BUILT) & 7u);
}

// The place the owner sees in a mode, or PLACES for none.
static u32 shown_place(u32 mode) {
    if (mode == 0) return RD32(SYNTH_SHOWN) & 7u;
    if (mode == 1) return (u32)DRUM + (RD32(DRUM_SHOWN) & 7u);
    if (mode == 3) return MASTER;
    return PLACES;
}

static const Effect* riding(u32 place) {
    u32 r = places[place].ride;
    u32 n = op1f_effects_registered;
    if (r == 0 || r > n) return 0;
    return &effects[r - 1u];
}

// R11: an effect that is a layer.
static inline int is_layer(const Effect* e) {
    return e != 0 && (e->flags & (u32)OP1F_EFFECT_LAYER) != 0;
}

// R11: does the place ride a layer?
static int rides_layer(u32 place) {
    return is_layer(riding(place));
}

// R20 (base 42): on the audio way (op1f_rack_run), so op1f_cards_lists is not called here before the count is read:
// its first call takes a mask. A call that can succeed needs the effects registered (look_through, which has called
// it) and a rack set by a pack's frame, both after the cards unit's check; before them the second list is zeroed data
// under N1, the count 0, and the card is refused as base 41 refused it (OP1F_BAD_NAME). op1f_cards_verdict, called on
// a match, calls op1f_cards_lists too, which by then is ready and takes no mask.
static int slot_of(const Op1fCard* card) {
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    if (card == 0) return -1;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards_2.slots[i].card == card) return op1f_cards_verdict(i) == 0 ? (int)i : -1;
    return -1;
}

// ---------------------------------------------------------------- the mapping of knob words and values

// A word of the knob `i` as the pack's value: the word's upper half, held to the knob's row, carried onto the value's
// range by a subtraction, a multiply and a shift.
static int value_of(const Effect* e, u32 i, u32 word) {
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
static u32 word_of(const Effect* e, u32 i, int value) {
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

// ---------------------------------------------------------------- a block, on the audio thread

static void account(Budget* b, Effect* e, u32 began, u32 took) {
    u32 sum = e->sum + took;
    e->sum = sum < took ? 0xffffffffu : sum;
    ++e->calls;
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

// R6d, level 10: the values a place's effect is called with kept as heard (the audio thread; a function of its own, so
// that the two calls' own code is base 29's as it was).
static EFFECTS_APART void heard_keep(u32 place, const Effect* e, const int* values) {
    u32 ride = e->number + 1u, count = e->count;
    Heard* h = &heard[place];
    u32 seq = h->seq + 1u;
    h->seq = seq;                                                                   // odd: being written
    h->ride = ride;
    for (u32 i = 0; i < count; ++i) h->values[i] = values[i];
    h->stamp = cycles_now();
    h->live = 1;
    h->seq = seq + 1u;                                                              // even: whole
}

// The pack's `process` called once, on the buffers handed in, its values in its own numbers, timed and held to its cost.
static void run(Effect* e, u32 place, int* left, int* right, const int* values, u32 on, u32 fresh, void* state, Budget* b) {
    u32 mono = (e->flags & (u32)OP1F_EFFECT_MONO) != 0;
    if (mono)
        for (u32 k = 0; k < (u32)FRAMES; ++k) left[k] = (left[k] >> 1) + (right[k] >> 1);
    Op1fEffectBlock block;
    block.bytes = BLOCK_BYTES;
    block.place = place;
    block.frames = FRAMES;
    block.left = left;
    block.right = mono ? left : right;
    block.values = values;
    block.on = on;
    block.fresh = fresh ? 1u : 0u;
    block.period = RD32(PERIOD);
    block.playing = RD32(TRANSPORT) != 0 ? 1u : 0u;
    block.state = state;
    u32 began = cycles_now();
    e->table->process(&block);
    account(b, e, began, cycles_now() - began);
    if (mono)
        for (u32 k = 0; k < (u32)FRAMES; ++k) right[k] = left[k];
}

// The effect a place rides, when it may play now on the object `w` built for `index`: its number + 1, else 0.
static u32 may_play(const Wrap* w, u32 index, u32 place) {
    u32 r = places[place].ride;
    if (r == 0 || r > op1f_effects_registered) return 0;
    const Effect* e = &effects[r - 1u];
    if ((e->flags & (u32)OP1F_EFFECT_LAYER) != 0) return r;                         // R11: a layer lies over whatever type
    if (w->type != e->type || place_type(place) != e->type) return 0;
    return r;
}

// Whether the effect `r` may be called at `place` for `index` now: not switched off, its memory there, and its `reset`
// returned for this build.
static void* ready_state(u32 r, u32 index, const Wrap* w, u32 place, int* ok) {
    const Effect* e = &effects[r - 1u];
    *ok = 0;
    if (places[place].budget.off) return 0;
    const Instance* in = &instances[r - 1u][index];
    if (in->status != (u32)READY) return 0;
    void* state = in->state;
    if (e->table->reset != 0 && reset_seq[index] != w->seq) return 0;
    *ok = 1;
    return state;
}

// R11: a layer's `process` after the unit's effect (`on` 1, from the wrap) or where it would have played (`on` 0, after
// the effect): no values, timed and held to its cost as any effect's; a layer with no `process` costs this one test.
static EFFECTS_APART void layer_run(u32 r, u32 index, const Wrap* w, u32 place, int* left, int* right, u32 on) {
    Effect* e = &effects[r - 1u];
    if (e->table->process == 0) return;
    int ok = 0;
    void* state = ready_state(r, index, w, place, &ok);
    if (!ok) return;
    u32 fresh = w->fresh;
    ((Wrap*)w)->fresh = 0;
    run(e, place, left, right, 0, on, fresh, state, &places[place].budget);
}

static u32 wrapped_process(Wrap* self, int* left, int* right, const u32* words) {
    u32 index = self->index;
    u32 place = index ? (u32)MASTER : playing_place();
    u32 r = may_play(self, index, place);
    if (r == 0 || (effects[r - 1u].flags & (u32)OP1F_EFFECT_LAYER) != 0) {         // R11: a layer: the unit's effect, then the layer
        void* inner = self->inner;
        ObjectFn fn = (ObjectFn)(*(const void* const* const*)inner)[2];
        u32 answer = fn(inner, (u32)left, (u32)right, (u32)words);
        if (r != 0) layer_run(r, index, self, place, left, right, 1u);
        return answer;
    }
    int ok = 0;
    void* state = ready_state(r, index, self, place, &ok);
    if (!ok) return 0;                                                              // the place passes the sound dry
    Effect* e = &effects[r - 1u];
    int values[VALUES];
    for (u32 i = 0; i < e->count; ++i) values[i] = value_of(e, i, words[i]);
    heard_keep(place, e, values);                                                   // R6d
    u32 fresh = self->fresh;
    self->fresh = 0;
    run(e, place, left, right, values, 1u, fresh, state, &places[place].budget);
    return 0;
}

static u32 forward(Wrap* self, u32 word, u32 a, u32 b, u32 c) {
    void* inner = self->inner;
    ObjectFn fn = (ObjectFn)(*(const void* const* const*)inner)[word];
    return fn(inner, a, b, c);
}

static u32 wrapped_destroy(Wrap* self, u32 a, u32 b, u32 c) { return forward(self, 0, a, b, c); }
static u32 wrapped_delete(Wrap* self, u32 a, u32 b, u32 c) { return forward(self, 1, a, b, c); }
static u32 wrapped_init(Wrap* self, u32 a, u32 b, u32 c) { return forward(self, 3, a, b, c); }

// After the effect at S1, S2 (OP1F_AT_INSTRUMENT, host 0 the synth, 1 the drum) and S4 (OP1F_AT_MASTER_OUT, host 2):
// an effect that keeps its tails, at the place that plays, while that place's effect is switched off.
extern "C" void op1f_effects_after(unsigned point, unsigned host, int* left, int* right) {
    if (op1f_effects_registered == 0 || host > 2u) return;
    if (host == 2u ? point != (u32)OP1F_AT_MASTER_OUT : point != (u32)OP1F_AT_INSTRUMENT) return;
    u32 index = host == 2u ? 1u : 0u;
    u32 place = host == 2u ? (u32)MASTER : host == 0u ? (RD32(SYNTH_BUILT) & 7u) : (u32)DRUM + (RD32(DRUM_BUILT) & 7u);
    const Wrap* w = (const Wrap*)RD32(EFFECT_WORD + 4u * index);
    if (w != &wraps[index][0] && w != &wraps[index][1]) return;
    u32 r = may_play(w, index, place);
    if (r == 0) return;
    Effect* e = &effects[r - 1u];
    if ((e->flags & (u32)OP1F_EFFECT_LAYER) != 0) {                                  // R11: a layer, while the unit's effect is off
        if (place_on(place) == 0) layer_run(r, index, w, place, left, right, 0u);
        return;
    }
    if ((e->flags & (u32)OP1F_EFFECT_TAILS) == 0 || place_on(place) != 0) return;
    int ok = 0;
    void* state = ready_state(r, index, w, place, &ok);
    if (!ok) return;
    u32 words = index ? (u32)MASTER_WORDS : (u32)STOCK_CTX;
    int values[VALUES];
    for (u32 i = 0; i < e->count; ++i) values[i] = value_of(e, i, RD32(words + 4u * i));
    heard_keep(place, e, values);                                                   // R6d
    u32 fresh = w->fresh;
    ((Wrap*)w)->fresh = 0;
    run(e, place, left, right, values, 0u, fresh, state, &places[place].budget);
}

// ---------------------------------------------------------------- W1: an effect object made

extern "C" void* op1f_fx_make(unsigned type, unsigned index) OP1F_NAMED("op1f_fx_make");
extern "C" void* op1f_fx_make(unsigned type, unsigned index) {
    void* object = STOCK_FACTORY(type, index);
    if (object == 0 || index > 1u || op1f_effects_registered == 0) return object;
    u32 mask;
    MASK_OFF(mask);
    Wrap* w = &wraps[index][wrap_next[index] & 1u];                                 // the other one may still be in a block
    wrap_next[index] = (wrap_next[index] + 1u) & 1u;
    w->table = WRAP_TABLE;
    w->inner = object;
    w->index = index;
    w->type = type;
    w->seq = ++wrap_seq;
    w->fresh = 1;
    MASK_ON(mask);
    return w;
}

// ---------------------------------------------------------------- W2, W3: the picks

// R16 (hasis, open-field pull request 38): another go after a switch-off, the counters first, `off` last. Called only
// while `off` is set: the audio thread then leaves that budget alone (a place's three calls of run() come through
// ready_state, which passes the place dry while `off` is set; op1f_rack_run answers OP1F_NOT_HERE before it), and
// account(), its only writer, sets `off` last, so nothing counts under the reset.
static EFFECTS_APART void budget_rearm(Budget* b) {                             // apart: inlined, the compiler walked its stores with an index register in layer_pick and op1f_rack_ready
    b->window_start = b->window_over = b->window_most = 0;
    b->off_cycles = b->off_overs = 0;
    b->off_said = 0;
    BARRIER();
    b->off = 0;
}

// R16: a pick in the unit's list at `place` (any row: a pack's, a layer's, the row it rides, the unit's own) re-arms
// the place when its effect was switched off there. Called by the key thread inside the pick's own masked section, where
// the ride changes (the review: not before it, where the audio thread could run one more block of the switched-off
// effect), so that no frame says the line of a switch-off half undone either.
static EFFECTS_APART void place_rearm(u32 place) {                             // apart: inlined, it left layer_pick short of registers
    if (places[place].budget.off) budget_rearm(&places[place].budget);
}

static void clear_entry(u32 pair) {
    op1f_stand_screens[2u * pair] = 0;                                              // the mode first
    op1f_stand_screens[2u * pair + 1u] = 0;
}

enum { EFFECT_MODE = 2u * OP1F_STAND_EFFECT, EFFECT_SCREEN = 2u * OP1F_STAND_EFFECT + 1u, MASTER_MODE = 2u * OP1F_STAND_MASTER,
       MASTER_SCREEN = 2u * OP1F_STAND_MASTER + 1u };

// R11: what the place holds now kept (EH and C3 at a list's opening; a pick of a layer's row with none kept).
static void snap_take(u32 place) {
    Snap* s = &snaps[place];
    s->type = place_type(place);
    s->on = place_on(place) ? 1u : 0u;
    s->ride = places[place].ride;
    u32 at = place_words(place);
    for (u32 i = 0; i < 8u; ++i) s->words[i] = RD32(at + 4u * i);
    BARRIER();
    s->taken = 1;
}

// R11: a pick of a pack's row at `place` (the key thread; W2 with the record, W3 with the controller): 1 when it is
// done here, 0 when the pick goes on as before. A layer's row: the place rides it (the mark first), then the firmware's
// pick of the row of the type the place held as its list opened, and that list's eight words put back; its switch at
// the next frame. The row the place rode as its list opened and rides still, the type word still the one of the
// opening (the list's own pick of the row it opened on): nothing picked again, the switch put back at the next frame;
// a row whose ride began after the opening is picked again as any pick (the review's S3). No stand-in pair is written
// for a layer: its page is the cards unit's to place, while stand() says so. The switch's put-back is asked only once
// the firmware's pick has returned: the key thread may wait inside it (the unit, 2026-11-24: the display thread's frame
// ran there, put the switch back, and the pick's own switch-on came after it); the master's pick ends with its switch
// on (0x8c0bad38), so the master's is put back at once as well, a sound's after the controller's +0x4c at the frame.
static int layer_pick(u32 place, u32 id, u32 record, void* controller) {
    const Effect* e = &effects[id - (u32)ID_FIRST];
    u32 r = id - (u32)ID_FIRST + 1u, bit = 1u << place, mask;
    Snap* s = &snaps[place];
    if (is_layer(e) && !s->taken) snap_take(place);
    int same = s->taken && s->ride == r && places[place].ride == r && place_type(place) == s->type;
    if (!same) s->ride = 0;                                                         // the opening's ride stands no more once another pick came
    if (!is_layer(e) && !same) return 0;
    u32 was = places[place].ride;
    u32 pair = place == (u32)MASTER ? (u32)OP1F_STAND_MASTER : (u32)OP1F_STAND_EFFECT;
    u32 mode = place == (u32)MASTER ? 3u + 1u : (place < (u32)DRUM ? 0u : 1u) + 1u;
    MASK_OFF(mask);
    place_rearm(place);                                                             // R16 (hasis): a pick is another go, a layer's row's too
    if (!same && was != 0 && !is_layer(riding(place)) && op1f_stand_screens[2u * pair] == mode) clear_entry(pair);
    places[place].ride = r;
    if (!same) picked |= bit;
    ended &= ~bit;
    if (same) {                                                                     // nothing picked again: the put-back asked now
        back_on[place] = s->on;
        back |= bit;
    }
    MASK_ON(mask);
    if (same) return 1;
    u32 stock = s->type < (u32)STOCK_COUNT ? stock_ids[s->type] : 0u;
    if (stock != 0 && controller != 0) STOCK_MASTER_PICK(controller, stock - 1u);
    else if (stock != 0) ROOM_PICK(record, stock - 1u);
    u32 at = place_words(place);
    for (u32 i = 0; i < 8u; ++i) WR32(at + 4u * i, s->words[i]);
    if (place < (u32)MASTER) STOCK_SET_PARAMS(record, s->words);
    else set_place_on(place, s->on);                                                // the master's pick switched it on before it returned
    MASK_OFF(mask);
    back_on[place] = s->on;                                                         // a frame that ran inside the pick put back nothing
    back |= bit;
    MASK_ON(mask);
    return 1;
}

// R11: the row a sound's list of effects opens on: the firmware's own answer, or the row of the effect the place
// rides; what the place holds kept for a pick (the key thread).
static u32 list_start(u32 place, u32 start) {
    if (op1f_effects_registered == 0) return start;
    snap_take(place);
    const Effect* e = riding(place);
    return e ? (u32)ID_FIRST + e->number : start;
}

// R13: EH, from its entry, in the place of R11's C1 and C2: the head of the firmware's function that answers the row a
// sound's list of effects opens on (the record), which the synth's and the drum's list starts alone call. The
// firmware's answer through the room; for a synth or a drum slot's record, list_start's (what C1 and C2 did); for any
// other record, the firmware's answer and nothing more.
extern "C" int op1f_fx_start_do(void* record) OP1F_NAMED("op1f_fx_start_do");
extern "C" int op1f_fx_start_do(void* record) {
    int start = ROOM_START(record);
    u32 place = 0;
    if (!record_place((u32)record, &place)) return start;
    return (int)list_start(place, (u32)start);
}

// R11: C3, the master's list's lookup of its effect's row (1, the group's key, the firmware's string, the out value):
// the firmware's lookup; then, while the master rides, the found row's id the ridden effect's, which the list opens on.
static u32 kind(const void* value);
extern "C" unsigned op1f_fx_master_start(unsigned database, const char* key, const void* name, void* out) OP1F_NAMED("op1f_fx_master_start");
extern "C" unsigned op1f_fx_master_start(unsigned database, const char* key, const void* name, void* out) {
    u32 found = STOCK_LOOKUP(database, key, name, out);
    if (op1f_effects_registered == 0) return found;
    snap_take((u32)MASTER);
    const Effect* e = riding((u32)MASTER);
    if (found == 0 || e == 0 || out == 0 || kind(out) != (u32)KIND_OBJECT) return found;
    u32 value[3];
    STOCK_JSON_NUMBER(value, (int)((u32)ID_FIRST + e->number));
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(out, KEY_ID), value);
    STOCK_JSON_DESTROY(value);
    return found;
}

// The instruments' pick, from its entry (module/build_r5_effects.s) once an effect is registered: R0 the slot's
// record, R1 the id the owner picked. The key thread.
extern "C" void op1f_fx_pick_do(unsigned record, unsigned id) OP1F_NAMED("op1f_fx_pick_do");
extern "C" void op1f_fx_pick_do(unsigned record, unsigned id) {
    u32 place = 0, n = op1f_effects_registered;
    if (!record_place(record, &place)) {
        ROOM_PICK(record, id);
        return;
    }
    u32 host = place < (u32)DRUM ? 0u : 1u;
    u32 mask;
    if (id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n && layer_pick(place, id, record, 0)) return;   // R11
    if (id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n) {
        const Effect* e = &effects[id - (u32)ID_FIRST];
        MASK_OFF(mask);
        place_rearm(place);                                                         // R16 (hasis): a pick is another go
        places[place].ride = id - (u32)ID_FIRST + 1u;                               // the mark first: the pick's build finds it
        picked |= 1u << place;
        ended &= ~(1u << place);
        op1f_stand_screens[EFFECT_SCREEN] = e->screen;                              // the screen before the mode
        op1f_stand_screens[EFFECT_MODE] = host + 1u;
        MASK_ON(mask);
        ROOM_PICK(record, e->ridden_id);
        STOCK_SET_ACTIVE(record, 0);                                                // the file says the unit's effect is off
        return;
    }
    if (places[place].ride != 0) {
        int layer = rides_layer(place);                                             // R11: a layer's switch is the owner's
        MASK_OFF(mask);
        place_rearm(place);                                                         // R16: the place left as fresh as any pick leaves it
        places[place].ride = 0;
        ended |= 1u << place;
        picked &= ~(1u << place);
        back &= ~(1u << place);
        if (op1f_stand_screens[EFFECT_MODE] == host + 1u) clear_entry(OP1F_STAND_EFFECT);
        MASK_ON(mask);
        if (!layer) STOCK_SET_ACTIVE(record, places[place].on);                     // the file says what played, as the firmware's own toggle would
    }
    ROOM_PICK(record, id);
}

// The master's pick, in the place of the word +0x28 of the mixer's controller: (the controller, the id).
extern "C" void op1f_fx_master_pick(void* controller, unsigned id) OP1F_NAMED("op1f_fx_master_pick");
extern "C" void op1f_fx_master_pick(void* controller, unsigned id) {
    u32 n = op1f_effects_registered, mask;
    if (id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n && layer_pick((u32)MASTER, id, 0, controller)) return;   // R11
    if (id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n) {
        const Effect* e = &effects[id - (u32)ID_FIRST];
        MASK_OFF(mask);
        place_rearm((u32)MASTER);                                                   // R16 (hasis): a pick is another go
        places[MASTER].ride = id - (u32)ID_FIRST + 1u;
        picked |= 1u << (u32)MASTER;
        ended &= ~(1u << (u32)MASTER);
        op1f_stand_screens[MASTER_SCREEN] = e->screen;
        op1f_stand_screens[MASTER_MODE] = 3u + 1u;                                  // the mixer's mode + 1
        MASK_ON(mask);
        STOCK_MASTER_PICK(controller, e->ridden_id);
        return;
    }
    if (places[MASTER].ride != 0) {
        MASK_OFF(mask);
        place_rearm((u32)MASTER);                                                   // R16: the place left as fresh as any pick leaves it
        places[MASTER].ride = 0;
        ended |= 1u << (u32)MASTER;
        picked &= ~(1u << (u32)MASTER);
        back &= ~(1u << (u32)MASTER);                                               // R11
        if (op1f_stand_screens[MASTER_MODE] == 3u + 1u) clear_entry(OP1F_STAND_MASTER);
        MASK_ON(mask);
    }
    STOCK_MASTER_PICK(controller, id);
}

// ---------------------------------------------------------------- W4 to W8: the names and the switch

// The knob names' function, from its entry: the pack's name of value `index` for the effects' kind and a sound that
// rides an effect with that many values; for the engines' kind on the synth's LFO page, what a unit of a later level
// answers for the shown slot (op1f_engines_names, the index as it came: an engine has eight values); else 0, and the
// firmware's own function answers.
extern "C" const char* op1f_fx_names_do(unsigned kind, int index) OP1F_NAMED("op1f_fx_names_do");
extern "C" const char* op1f_fx_names_do(unsigned kind, int index) {
    if (kind == (u32)NAMES_KIND_ENGINE) {
        if (ENGINES_NAMES == 0 || index < 0 || RD32(LFO_HOST) != 0 || RD32(SYNTH_SHOWN) >= (u32)SLOTS) return 0;
        return ENGINES_NAMES(RD32(SYNTH_SHOWN), index);
    }
    if (kind != (u32)NAMES_KIND_EFFECT || index < 0) return 0;
    u32 place = RD32(LFO_HOST) == 0 ? RD32(SYNTH_SHOWN) : (u32)DRUM + RD32(DRUM_SHOWN);
    if (place >= (u32)MASTER || (RD32(LFO_HOST) == 0 && place >= (u32)DRUM)) return 0;
    const Effect* e = riding(place);
    if (e == 0) return 0;
    u32 i = (u32)index & 3u;                                                        // the firmware takes the index by its four knobs
    return i < e->count ? e->value_names[i] : 0;
}

// The setter of a record's fx_active, from its entry: what is handed on in place of `on`.
extern "C" unsigned op1f_fx_active_do(unsigned record, unsigned on) OP1F_NAMED("op1f_fx_active_do");
extern "C" unsigned op1f_fx_active_do(unsigned record, unsigned on) {
    u32 place = 0;
    if (record_place(record, &place) && places[place].ride != 0 && !rides_layer(place)) return 0;   // R11: a layer's switch is the owner's
    return on;
}

// The records' applies, in the place of their calls of the on setters: (slot, on).
static void apply_on(u32 base, u32 slot, u32 on) {
    if (slot < (u32)SLOTS) {
        Place* p = &places[base + slot];
        p->doc_on = on;
        if (p->ride != 0 && !rides_layer(base + slot)) on = p->on;              // R11: a layer's switch is the owner's
    }
    if (base == 0) STOCK_ON_SYNTH(slot, on);
    else STOCK_ON_DRUM(slot, on);
}

extern "C" void op1f_fx_apply_synth(unsigned slot, unsigned on) OP1F_NAMED("op1f_fx_apply_synth");
extern "C" void op1f_fx_apply_synth(unsigned slot, unsigned on) { apply_on(0, slot, on); }

extern "C" void op1f_fx_apply_drum(unsigned slot, unsigned on) OP1F_NAMED("op1f_fx_apply_drum");
extern "C" void op1f_fx_apply_drum(unsigned slot, unsigned on) { apply_on((u32)DRUM, slot, on); }

// The master's save, in the place of its call of the on getter: what it writes as the master's fx_active.
extern "C" unsigned op1f_fx_master_saved() OP1F_NAMED("op1f_fx_master_saved");
extern "C" unsigned op1f_fx_master_saved() {
    if (places[MASTER].ride != 0 && !rides_layer((u32)MASTER)) return 0;          // R11: a layer's switch is the owner's
    return STOCK_MASTER_ON();
}

// ---------------------------------------------------------------- the registration, at the catalog's read

static int plain_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)NAME;
}

// R11: the unit's own LFO kinds, whose names no effect's layer takes (an ordinary effect may bear one: the lists of
// effects and of LFOs are separate, as on base 34).
static const char LFO_KINDS[6][FIELD] = {"tremolo", "value", "random", "element", "midi", "velocity"};
static int lfo_kind_named(const char* name) {
    for (u32 i = 0; i < 6u; ++i)
        if (same_text(name, LFO_KINDS[i], NAME)) return 1;
    return 0;
}

static int stock_type(const char* text, u32* type);

static int stock_number(const char* name) {
    for (u32 i = 0; i < (u32)STOCK_COUNT; ++i)
        if (same_text(name, STOCK_NAMES[i], NAME)) return (int)i;
    return -1;
}

static void set_aside(u32 slot, u32 why, u32 a, u32 b, const char* name, const char* rides) {
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
    copy_name(s->rides, rides ? rides : "");
}

static int inside(u32 at, const Op1fCardSlot* slot, u32 bytes) {
    return at >= slot->first && at < slot->end && slot->end - at >= bytes;
}

// Why an ask of this kind is left aside (WHY_), or 0. For WHY_ORDER, (a, b):
//   (the ask's bytes, 180)                 its table is too small or too large, or there is none
//   (the bytes its head says, the ask's)   the two differ
//   (its place from the pack's first byte, the pack's bytes)   `what` not inside the pack on a word
//   (its flags, 7)                         a flag this base does not know
//   (its count, 0)                         R11: a layer with values
//   (`shown`'s place, the pack's bytes)    `shown` not a function of the pack
//   (`process`'s place, the pack's bytes)  `process` 0 or not a function of the pack
//   (`reset`'s place, the pack's bytes)    `reset` not a function of the pack
//   (its working memory, 8388608)          more than 8 MiB
//   (its alignment, 4096)                  not a power of two from 4 to 4096
//   (its count, 4)                         not 1 to 4 values
//   (0x100 + the value's number, 0)        a value's name, or its range, or its start
static u32 check(const Op1fAsk* ask, const Op1fCardSlot* slot, u32* a, u32* b) {
    const Op1fEffect* t = (const Op1fEffect*)ask->what;
    u32 span = slot->end - slot->first;
    if (t == 0 || ask->bytes < (u32)TABLE_BYTES || ask->bytes > (u32)OP1F_ASK_BYTES_MOST) { *a = ask->bytes; *b = TABLE_BYTES; return WHY_ORDER; }
    if (!inside((u32)t, slot, (u32)TABLE_BYTES) || ((u32)t & 3u) != 0) { *a = (u32)t - slot->first; *b = span; return WHY_ORDER; }
    if (!plain_name(t->head.name)) return WHY_NAME;
    if (t->head.bytes != ask->bytes) { *a = t->head.bytes; *b = ask->bytes; return WHY_ORDER; }
    const u32 known = (u32)(OP1F_EFFECT_MONO | OP1F_EFFECT_TAILS | OP1F_EFFECT_LAYER);   // R11: and a layer
    if ((t->head.flags & ~known) != 0) { *a = t->head.flags; *b = known; return WHY_ORDER; }
    u32 layer = (t->head.flags & (u32)OP1F_EFFECT_LAYER) != 0;
    u32 f = (u32)t->head.shown;
    if (f != 0 && (!inside(f, slot, 2u) || (f & 1u) != 0)) { *a = f - slot->first; *b = span; return WHY_ORDER; }
    f = (u32)t->process;
    if ((f != 0 || !layer) && (!inside(f, slot, 2u) || (f & 1u) != 0)) { *a = f - slot->first; *b = span; return WHY_ORDER; }   // R11: a layer's may be 0
    f = (u32)t->reset;
    if (f != 0 && (!inside(f, slot, 2u) || (f & 1u) != 0)) { *a = f - slot->first; *b = span; return WHY_ORDER; }
    if (t->state_bytes > STATE_MOST) { *a = t->state_bytes; *b = STATE_MOST; return WHY_ORDER; }
    u32 align = t->state_align;
    if (t->state_bytes != 0 && (align < (u32)STATE_ALIGN_LEAST || align > ALIGN_MOST || (align & (align - 1u)) != 0)) { *a = align; *b = ALIGN_MOST; return WHY_ORDER; }
    if (layer) {                                                                    // R11: no values, no ride, its cost only with a process
        if (t->count != 0) { *a = t->count; *b = 0; return WHY_ORDER; }
        for (u32 i = 0; i < (u32)NAME; ++i)
            if (t->rides[i] != 0) return WHY_RIDES;
        if (t->process == 0 ? t->cycles != 0 : (t->cycles < 1u || t->cycles > CYCLES_MOST)) return WHY_COST;
        return 0;
    }
    if (t->count < 1u || t->count > (u32)VALUES) { *a = t->count; *b = VALUES; return WHY_ORDER; }
    for (u32 i = 0; i < t->count; ++i) {
        const Op1fParam* p = &t->params[i];
        if (!plain_name(p->name) || p->low >= p->high || (u32)(p->high - p->low) > 65535u || p->start < p->low || p->start > p->high) {
            *a = 0x100u + i;
            *b = 0;
            return WHY_ORDER;
        }
    }
    int rides = -1;
    for (u32 i = 0; i < (u32)STOCK_COUNT; ++i)
        if (same_text(t->rides, STOCK_NAMES[i], NAME)) rides = (int)i;
    if (rides < 0 || !RIDEABLE[rides]) return WHY_RIDES;
    if (t->cycles < 1u || t->cycles > CYCLES_MOST) return WHY_COST;
    return 0;
}

// The asks of kind OP1F_ASK_EFFECT of the cards the base uses, in the list's order and each card's, held to their form.
static void look_through() {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;                                   // a card the base does not use
        const Op1fCardSlot* slot = &op1f_cards_2.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 8u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_EFFECT) continue;
            u32 a = 0, b = 0;
            u32 why = check(ask, slot, &a, &b);
            const Op1fEffect* t = (const Op1fEffect*)ask->what;
            if (why == WHY_ORDER) {
                int named = t != 0 && inside((u32)t, slot, (u32)TABLE_BYTES) && plain_name(t->head.name);
                set_aside(i, why, a, b, named ? t->head.name : "?", 0);
                continue;
            }
            if (why == WHY_NAME) {
                set_aside(i, why, 0, 0, "?", 0);
                continue;
            }
            int layer = (t->head.flags & (u32)OP1F_EFFECT_LAYER) != 0;               // R11: a layer's name is none of the unit's LFOs;
            int taken = stock_number(t->head.name) >= 0 || (layer && lfo_kind_named(t->head.name));   // an effect's may be (as base 34)
            for (u32 c = 0; c < candidate_count && !taken; ++c)
                if (same_text(candidates[c].table->head.name, t->head.name, NAME)) taken = 1;
            if (taken) {
                set_aside(i, WHY_TAKEN, 0, 0, t->head.name, 0);
                continue;
            }
            if (why) {
                set_aside(i, why, 0, 0, t->head.name, why == WHY_RIDES && plain_name(t->rides) ? t->rides : "?");
                continue;
            }
            if (candidate_count == (u32)EFFECTS) {
                set_aside(i, WHY_SEVENTEENTH, 0, 0, t->head.name, 0);
                continue;
            }
            Candidate* c = &candidates[candidate_count++];
            c->table = t;
            c->slot = i;
            c->ask = k;
        }
    }
}

static u32 kind(const void* value) {
    return ((const volatile u8*)value)[KIND_AT];
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

// The candidates registered in order, each with its id and its ridden row's id; those whose ridden row the catalog
// does not hold are left aside.
static void enrol(const void* list) {
    u32 n = 0;
    for (u32 c = 0; c < candidate_count; ++c) {
        const Op1fEffect* t = candidates[c].table;
        u32 layer = (t->head.flags & (u32)OP1F_EFFECT_LAYER) != 0;               // R11: a layer stands on no row
        const void* row = row_named(list, layer ? STOCK_NAMES[0] : t->rides);
        const void* id = row ? STOCK_JSON_MEMBER(row, KEY_ID) : 0;
        if (id == 0 || (kind(id) != (u32)KIND_INT && kind(id) != (u32)KIND_UINT)) {
            set_aside(candidates[c].slot, WHY_RIDES, 0, 0, t->head.name, t->rides);
            continue;
        }
        Effect* e = &effects[n];
        e->table = t;
        e->number = n;
        e->slot = candidates[c].slot;
        e->ask = candidates[c].ask;
        e->ridden_id = layer ? 0u : (u32)STOCK_JSON_INT(id);
        e->type = layer ? (u32)STOCK_COUNT : (u32)stock_number(t->rides);
        for (u32 k = 0; k < (u32)STOCK_COUNT; ++k) {                                // the type by the firmware's own table of names
            const char* name = (const char*)RD32(EFFECT_NAMES + 8u * k);
            u32 at = 0;
            while (at < (u32)NAME && RD8((u32)name + at) == (u8)t->rides[at] && t->rides[at] != 0) ++at;
            if (!layer && at < (u32)NAME && RD8((u32)name + at) == 0 && t->rides[at] == 0) e->type = RD32(EFFECT_NAMES + 8u * k + 4u);
        }
        e->flags = t->head.flags;
        const Op1fCardSlot* held = &op1f_cards_2.slots[e->slot];                     // R22 (base 45): wagons declared behind the picture's word
        e->hosted = !layer && held->card->asks[e->ask].bytes >= (u32)OP1F_EFFECT_HOSTED_BYTES && inside((u32)t, held, (u32)OP1F_EFFECT_HOSTED_BYTES) &&
                    ((const u32*)t)[(u32)OP1F_EFFECT_HOSTED_AT >> 2] == (u32)OP1F_HOSTED_WAGONS ? 1u : 0u;
        e->cycles = t->cycles;
        e->count = t->count;
        e->state_bytes = t->state_bytes;
        e->state_align = t->state_align < (u32)STATE_ALIGN_LEAST ? (u32)STATE_ALIGN_LEAST : t->state_align;
        e->page = OP1F_LAYER_PAGE_UNIT;                                             // R11
        copy_name(e->name, t->head.name);
        copy_name(e->rides, t->rides);
        for (u32 i = 0; i < t->count; ++i) {
            copy_name(e->value_names[i], t->params[i].name);
            e->low[i] = t->params[i].low;
            e->high[i] = t->params[i].high;
            e->start[i] = t->params[i].start;
        }
        Op1fRackEntry* entry = &entries[n];
        entry->bytes = ENTRY_BYTES;
        entry->id = (u32)ID_FIRST + n;
        entry->pack = op1f_cards_2.slots[e->slot].card->name;
        entry->effect = t;
        entry->rides = e->type;
        ++n;
    }
    BARRIER();                                                                      // every record whole before the word that counts them
    op1f_effects_registered = n;
}

// R11: the rows' ids of the unit's own effects, by type, for a pick of a layer's row (the start's thread).
static void keep_stock_ids(const void* list) {
    for (u32 k = 0; k < (u32)STOCK_COUNT; ++k) {
        u32 type = 0;
        const void* row = row_named(list, STOCK_NAMES[k]);
        const void* id = row ? STOCK_JSON_MEMBER(row, KEY_ID) : 0;
        if (id == 0 || (kind(id) != (u32)KIND_INT && kind(id) != (u32)KIND_UINT) || !stock_type(STOCK_NAMES[k], &type) || type >= (u32)STOCK_COUNT) continue;
        stock_ids[type] = (u32)STOCK_JSON_INT(id) + 1u;
    }
}

static void add_row(void* list, u32 n, const void* ridden) {
    u32 row[3], value[3];
    STOCK_JSON_MAKE(row, KIND_OBJECT);
    STOCK_JSON_TEXT(value, effects[n].name);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_TYPE), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_NUMBER(value, (int)((u32)ID_FIRST + n));
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_ID), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_DEFAULTS), STOCK_JSON_MEMBER(ridden, KEY_DEFAULTS));
    STOCK_JSON_APPEND(list, row);
    STOCK_JSON_DESTROY(row);
}

extern "C" void op1f_effects_catalog_read(void* document) {
    if (!checked) {
        checked = 1;
        look_through();
        if (candidate_count == 0) return;
        if (kind(document) != (u32)KIND_OBJECT) list_state = LIST_NOT_OBJECT;
        else if (kind(STOCK_JSON_MEMBER(document, KEY_LIST)) != (u32)KIND_ARRAY) list_state = LIST_NOT_ARRAY;
        if (list_state) return;                                                     // no row in the lists: no effect of ours either
        enrol(STOCK_JSON_MEMBER(document, KEY_LIST));
    }
    u32 n = op1f_effects_registered;
    if (n == 0 || kind(document) != (u32)KIND_OBJECT) return;
    const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
    if (kind(list) != (u32)KIND_ARRAY) return;
    keep_stock_ids(list);                                                           // R11
    for (u32 i = 0; i < n; ++i) {
        if (row_named(list, effects[i].name)) continue;
        const void* ridden = row_named(list, is_layer(&effects[i]) ? STOCK_NAMES[0] : effects[i].rides);   // R11: a layer's defaults, delay's
        if (ridden == 0) continue;
        add_row(STOCK_JSON_PLACE(document, KEY_LIST), i, ridden);
        ++rows_added;
    }
}

extern "C" int op1f_effects_row_is(const void* row) {
    u32 n = op1f_effects_registered;
    if (n == 0 || kind(row) != (u32)KIND_OBJECT) return 0;
    const char* text = text_of(STOCK_JSON_MEMBER(row, KEY_TYPE));
    if (text == 0) return 0;
    for (u32 i = 0; i < n; ++i)
        if (same_text(text, effects[i].name, NAME)) return 1;
    return 0;
}

// ---------------------------------------------------------------- the base's member, op1f_effect

static u32 keep_where(u32 place) {
    return place < (u32)DRUM ? (u32)OP1F_KEEP_SYNTH : place < (u32)MASTER ? (u32)OP1F_KEEP_DRUM : (u32)OP1F_KEEP_MASTER;
}

static u32 keep_which(u32 place) {
    return place < (u32)DRUM ? place : place < (u32)MASTER ? place - (u32)DRUM : 0u;
}

// The member's text read: 1 with the name (one to fifteen characters) and its switch; 0 when the text is not an object
// with a member "name" holding a text (`name` then "?").
static int member_of(const char* text, char* name, u32* on) {
    copy_name(name, "?");
    *on = 0;
    u32 at = 0;
    while (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r') ++at;
    if (text[at] != '{') return 0;
    int named = 0;
    for (u32 i = at; text[i]; ++i) {
        if (text[i] != '"') continue;
        if (same_text(text + i + 1, "name\"", 5)) {
            u32 k = i + 6;
            while (text[k] == ' ' || text[k] == ':') ++k;
            if (text[k] != '"') return 0;
            ++k;
            u32 len = 0;
            while (text[k + len] && text[k + len] != '"') ++len;
            if (len == 0 || len >= (u32)NAME || text[k + len] != '"') return 0;
            for (u32 c = 0; c < len; ++c) name[c] = text[k + c];
            for (u32 c = len; c < (u32)NAME; ++c) name[c] = 0;
            named = 1;
            i = k + len;
        } else if (same_text(text + i + 1, "on\"", 3)) {
            u32 k = i + 4;
            while (text[k] == ' ' || text[k] == ':') ++k;
            *on = text[k] == '1' || text[k] == 't' ? 1u : 0u;
            i = k;
        }
    }
    return named;
}

static u32 put(char* out, u32 at, const char* text) {
    while (*text && at + 1u < (u32)MEMBER_ROOM) out[at++] = *text++;
    out[at] = 0;
    return at;
}

// The member written, or taken away when the place rides nothing: at once when the values unit takes it, again at the
// next frame only when it answered OP1F_NOT_NOW (a passing state: before its first frame, a load not yet told, another
// thread writing the slot); any other refusal (OP1F_BAD_NAME, OP1F_BAD_VALUE, OP1F_TOO_LARGE) is not tried again.
static void write_member(u32 place) {
    const Effect* e = riding(place);
    char text[MEMBER_ROOM];
    int answer;
    if (e) {
        u32 at = put(text, 0, "{\"name\": \"");
        at = put(text, at, e->name);
        at = put(text, at, is_layer(e) ? "\"}" : places[place].on ? "\", \"on\": 1}" : "\", \"on\": 0}");   // R11: a layer's has no switch
        answer = op1f_keeps_base_set(keep_where(place), keep_which(place), text);
    } else {
        answer = op1f_keeps_base_set(keep_where(place), keep_which(place), 0);
    }
    places[place].member_dirty = answer == OP1F_NOT_NOW ? 1u : 0u;
}

extern "C" void op1f_effects_changed(unsigned where, unsigned which, unsigned how) {
    if (how != (u32)OP1F_KEPT_LOADED && how != (u32)OP1F_KEPT_START && how != (u32)OP1F_KEPT_DROPPED) return;
    u32 place = where == (u32)OP1F_KEEP_SYNTH && which < (u32)SLOTS ? which
              : where == (u32)OP1F_KEEP_DRUM && which < (u32)SLOTS ? (u32)DRUM + which
              : where == (u32)OP1F_KEEP_MASTER ? (u32)MASTER : (u32)PLACES;
    if (place < (u32)PLACES) {
        told |= 1u << place;
        if (how == (u32)OP1F_KEPT_LOADED) told_load |= 1u << place;                  // R6d
    }
}

// ---------------------------------------------------------------- the frame's work

static const char* place_words_for_log(u32 place, char* out) {
    if (place == (u32)MASTER) return "the master";
    const char* host = place < (u32)DRUM ? "synth " : "drum ";
    u32 at = 0;
    while (*host) out[at++] = *host++;
    out[at++] = (char)('1' + (place & 7u));
    out[at] = 0;
    return out;
}

static void say_ride(u32 place, const char* what, const char* why, const char* name) {
    if (unit.ride_lines >= (u32)RIDE_LINES) return;
    ++unit.ride_lines;
    char words[12];
    const char* p = place_words_for_log(place, words);
    if (what) op1f_log(3, LOG_FILE, 3, "effect: %s rides '%s'", p, what);
    else if (name) op1f_log(3, LOG_FILE, 4, "effect: %s rides nothing now: the unit put %s there", p, name);
    else op1f_log(3, LOG_FILE, 4, "effect: %s rides nothing now: %s", p, why);
}

static const char* stock_name(u32 type) {
    return type < (u32)STOCK_COUNT ? STOCK_NAMES[type] : "another effect";
}

// The place's knob words as the frame sets them: the place's own word, and for a sound the record's document through
// the firmware's setter, as a knob would write them.
static void set_words(u32 place, const u32* four, u32 mask) {
    u32 words = place_words(place);
    u32 eight[8];
    for (u32 i = 0; i < 8u; ++i) eight[i] = RD32(words + 4u * i);
    for (u32 i = 0; i < 4u; ++i) {
        if ((mask & (1u << i)) == 0) continue;
        eight[i] = four[i];
        WR32(words + 4u * i, four[i]);
    }
    if (place < (u32)MASTER) STOCK_SET_PARAMS(place_record(place), eight);
}

// Whether a place's sound plays now at its index (the instrument's slot that is built, or the master).
static int plays(u32 place) {
    if (place == (u32)MASTER) return 1;
    return playing_place() == place;
}

// R11: the build that plays at the place, if it plays now, told `fresh` at its next block (a ride begun without a pick),
// and its `reset` called again at the next frame before that block (the review's S2: the build may have had another
// effect's reset already; ready_state holds the layer's `process` back until then).
static EFFECTS_APART void made_fresh(u32 place) {
    if (!plays(place)) return;
    u32 index = place == (u32)MASTER ? 1u : 0u;
    Wrap* w = (Wrap*)RD32(EFFECT_WORD + 4u * index);
    if (w != &wraps[index][0] && w != &wraps[index][1]) return;
    u32 mask;
    MASK_OFF(mask);
    w->fresh = 1;
    reset_seq[index] = 0;                                                           // no build's seq is 0 (wrap_seq counts from 1)
    MASK_ON(mask);
}

// R6, base 28's review R-2: the stand-in pair a pick wrote for the mode of a ride that
// ends, taken back where it still holds that ride's {mode + 1, screen} and no other place of that mode rides an effect
// of that screen. The pick writes its mode's pair before the firmware's pick and the cards unit takes it over within a
// frame; where 77 is not free (a pack's page holds it, as fx train 14's own pick of its first wagon), the pair stayed
// until the next pick of a pack's effect or a restart, and the screen stub answered 77 for that stock effect's screen in
// that mode for a sound that rides nothing. The cards unit's own compare-and-clear then finds 0 and does nothing; a pair
// it holds as its own for a page that still stands is written again at its frame. Inside the caller's mask.
static void take_back_pair(u32 place, u32 ride) {
    u32 screen = effects[ride - 1u].screen;
    u32 first = place == (u32)MASTER ? (u32)MASTER : place < (u32)DRUM ? 0u : (u32)DRUM;
    u32 last = place == (u32)MASTER ? (u32)MASTER + 1u : first + (u32)SLOTS;
    for (u32 p = first; p < last; ++p)
        if (p != place && places[p].ride != 0 && effects[places[p].ride - 1u].screen == screen) return;
    u32 pair = place == (u32)MASTER ? (u32)OP1F_STAND_MASTER : (u32)OP1F_STAND_EFFECT;
    u32 mode = place == (u32)MASTER ? 3u + 1u : (place < (u32)DRUM ? 0u : 1u) + 1u;
    if (op1f_stand_screens[2u * pair] == mode && op1f_stand_screens[2u * pair + 1u] == screen) clear_entry(pair);
}

// A ride that ends other than by a pick: the base's member taken away, the file's fx_active what plays, and the
// firmware's object made anew, so that its effect never goes on from a state it kept unused.
static void end_ride(u32 place, u32 load) {
    u32 mask;
    int layer = rides_layer(place);                                                 // R11: the member alone; the unit's effect is the owner's
    MASK_OFF(mask);
    if (places[place].ride != 0 && !layer) take_back_pair(place, places[place].ride);   // R6: base 28's review R-2
    places[place].ride = 0;
    picked &= ~(1u << place);
    back &= ~(1u << place);
    MASK_ON(mask);
    if (layer) {
        write_member(place);
        return;
    }
    if (load) {
        if (place < (u32)MASTER) set_place_on(place, places[place].doc_on);           // what the loaded sound's file says
    } else if (place < (u32)MASTER) {
        STOCK_SET_ACTIVE(place_record(place), place_on(place));
    }
    write_member(place);
    // R6d: at a load, a start or a drop the firmware's apply makes the object anew (it clears the object's word first and
    // makes it after the type and the words); one made here could be cut by the key thread's apply of another sound.
    if (!load && plays(place)) STOCK_BUILD(place == (u32)MASTER ? 1u : 0u, place_type(place));
}

// R11 (the review's S1): a ride of an ordinary effect that a loaded member naming a layer takes over, ended as end_ride
// ends it: its stand-in pair taken back, the switch what the loaded sound's file says (at a load) or what plays, the
// object made anew when not at a load; the member is left to the layer. Nothing for a place that rides nothing or a
// layer. Out of line, the test too: read_member uses no index register.
static EFFECTS_APART void plain_ride_yields(u32 place, u32 load) {
    if (places[place].ride == 0 || rides_layer(place)) return;
    u32 mask;
    MASK_OFF(mask);
    take_back_pair(place, places[place].ride);
    places[place].ride = 0;
    picked &= ~(1u << place);
    back &= ~(1u << place);
    MASK_ON(mask);
    if (place >= (u32)MASTER) return;                                               // the master's switch is the base's at its save (W8)
    if (load) {
        set_place_on(place, places[place].doc_on);                                  // what the loaded sound's file says
        return;
    }
    STOCK_SET_ACTIVE(place_record(place), place_on(place));
    if (plays(place)) STOCK_BUILD(0u, place_type(place));
}

// The member of a place read after a start or a load: the place rides again when its pack is here and the unit still
// keeps the effect it stands on; else the ride is let go, or the member kept as it is when its pack is away. Read once
// at the first frame and once at each tell of the values unit, never in a loop: OP1F_NO_FILE (none, or the values unit
// not at its first frame yet: its OP1F_KEPT_START tell brings it) and OP1F_BAD_NAME are read as no member.
static void read_member(u32 place, u32 load) {
    char text[MEMBER_ROOM];
    int got = op1f_keeps_base_get(keep_where(place), keep_which(place), text, (u32)MEMBER_ROOM);
    char name[NAME];
    u32 on = 0;
    int named = got >= 0 && member_of(text, name, &on);
    u32 n = op1f_effects_registered, found = 0;
    for (u32 i = 0; named && i < n; ++i)
        if (same_text(name, effects[i].name, NAME)) found = i + 1u;
    u32 was = places[place].ride;
    if (found == 0) {
        if (got >= 0 && !places[place].absent_said) {                               // kept as it is: never less than the firmware alone
            places[place].absent_said = 1;
            char words[12];
            op1f_log(2, LOG_FILE, 5, "effect: %s rides '%s', whose pack is not here: %s plays dry", place_words_for_log(place, words), name,
                     place == (u32)MASTER ? "it" : "the sound");
        }
        if (was) {
            say_ride(place, 0, got >= 0 ? "its pack is not here" : "a sound without it was loaded", 0);
            end_ride(place, load);
        }
        return;
    }
    const Effect* e = &effects[found - 1u];
    if (is_layer(e)) {                                                              // R11: a layer, over whatever the place holds
        if (was != 0) plain_ride_yields(place, load);                               // the review's S1: an ordinary ride's end
        u32 mask;
        MASK_OFF(mask);
        places[place].ride = found;
        MASK_ON(mask);
        if (was != found) {
            made_fresh(place);
            say_ride(place, e->name, 0, 0);
        }
        return;
    }
    if (place_type(place) != e->type) {                                             // the unit keeps another effect there now
        say_ride(place, 0, 0, stock_name(place_type(place)));
        if (was) end_ride(place, load);
        else write_member(place);                                                   // the member taken away: the place rides nothing
        return;
    }
    u32 mask;
    MASK_OFF(mask);
    places[place].on = on;
    places[place].ride = found;
    MASK_ON(mask);
    set_place_on(place, on);
    // R6, base 28's review R-1: a ride taken up from the member (the first frame, a load,
    // a start) sets the sound's fx_active to 0 as a pick does, so that a file made "on" elsewhere (on the way back, or
    // on the original firmware) never says it is on while it rides, nor is saved so. The ride is marked first: W5 hands
    // 0 on, and the firmware's setter queues the slot's write. The master's switch is the base's at its save (W8).
    if (place < (u32)MASTER) STOCK_SET_ACTIVE(place_record(place), 0);
    if (was != found) say_ride(place, e->name, 0, 0);
}

// The knob rows of the types the registered effects stand on, and the factors each way, at the first frame.
static void map_all() {
    u32 n = op1f_effects_registered, said = 0;
    for (u32 k = 0; k < n; ++k) {
        Effect* e = &effects[k];
        int lo[4] = {0, 0, 0, 0}, hi[4] = {0, 0, 0, 0};
        for (u32 i = 0; i < 4u && e->type < (u32)STOCK_COUNT; ++i) {
            u32 row = KNOB_ROWS + (u32)ROW_BYTES * e->type + (u32)KNOB_BYTES * i;
            lo[i] = (int)(short)RD16(row);
            hi[i] = (int)RD16(row + 2u);
        }
        if (e->type < (u32)STOCK_COUNT && (said & (1u << e->type)) == 0) {
            said |= 1u << e->type;
            op1f_log(3, LOG_FILE, 6, "effect: the knob rows of %s: %d..%d, %d..%d, %d..%d, %d..%d", e->rides, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2], lo[3], hi[3]);
        }
        for (u32 i = 0; i < e->count; ++i) {
            e->row_low[i] = lo[i];
            e->row_high[i] = hi[i];
            if (hi[i] <= lo[i]) {
                e->fixed[i] = 1;
                op1f_log(2, LOG_FILE, 7, "effect: '%s' value %u '%s' stands at %d: the knob row of %s has no span", e->name, i + 1u, e->value_names[i], e->start[i], e->rides);
                continue;
            }
            u32 stock = (u32)(hi[i] - lo[i]), own = (u32)(e->high[i] - e->low[i]);
            ratio(own, stock, &e->q_in[i], &e->f_in[i]);
            ratio(stock, own, &e->q_out[i], &e->f_out[i]);
        }
        e->screen = e->type < (u32)STOCK_COUNT ? STOCK_SCREEN_OF(e->type) : 0u;
    }
    for (u32 k = 0; k < (u32)STOCK_COUNT; ++k) stock_screens[k] = STOCK_SCREEN_OF(k);   // R11: a layer's page, by the type under it
}

static const char* aside_reason(const Aside* s) {
    switch (s->why) {
    case WHY_NAME: return "its name is not one the lists can show";
    case WHY_TAKEN: return "its name is a stock effect's or an earlier pack's";
    case WHY_COST: return "its cost is not between 1 and 84000";
    case WHY_SEVENTEENTH: return "it would be the seventeenth";
    default: return 0;
    }
}

static void first_frame() {
    for (u32 i = 0; i < aside_count; ++i) {
        const Aside* s = &asides[i];
        const char* pack = op1f_cards_2.slots[s->slot].card->name;
        if (s->why == WHY_ORDER) op1f_log(2, LOG_FILE, 2, "effect: '%s' of '%s' is left aside: its table is not in order (%u, %u)", s->name, pack, s->a, s->b);
        else if (s->why == WHY_RIDES) op1f_log(2, LOG_FILE, 2, "effect: '%s' of '%s' is left aside: it stands on %s, which is not one an effect can stand on", s->name, pack, s->rides);
        else op1f_log(2, LOG_FILE, 2, "effect: '%s' of '%s' is left aside: %s", s->name, pack, aside_reason(s));
    }
    if (aside_more) op1f_log(2, LOG_FILE, 2, "effect: %u more ask(s) left aside, not said", aside_more);
    if (list_state) op1f_log(2, LOG_FILE, 2, "effect: the unit's lists could not be read (%u): no effect is registered", list_state);
    u32 n = op1f_effects_registered;
    for (u32 k = 0; k < n; ++k)
        op1f_log(3, LOG_FILE, 1, "effect: '%s' of '%s' stands on %s, entry %u of the lists, %u cycles a block declared", effects[k].name,
                 op1f_cards_2.slots[effects[k].slot].card->name, is_layer(&effects[k]) ? "the unit's own effect, a layer" : effects[k].rides, (u32)ID_FIRST + k,
                 effects[k].cycles);
    if (n == 0) return;
    map_all();
    for (u32 p = 0; p < (u32)PLACES; ++p) read_member(p, 0);
    unit.countdown = PERIOD_FRAMES;
}

// An instance's working memory asked, or polled while it is cleared. Answers its status.
static u32 make_ready(Instance* in, const Effect* e, const char* for_what) {
    if (in->status == (u32)READY || in->status == (u32)REFUSED) return in->status;
    if (in->status == (u32)WIPING) return CLEARING;                                 // R16: a run's kept memory, cleared again at the frames (rack_wipe)
    if (e->state_bytes == 0) {
        in->state = 0;
        BARRIER();
        in->status = READY;
        return READY;
    }
    if (in->status == (u32)NONE) {
        u32 at = 0;
        for (const char* c = e->name; *c && at < 15u; ++c) in->whose[at++] = *c;
        for (const char* c = " for "; *c; ++c) in->whose[at++] = *c;
        for (const char* c = for_what; *c && at < 63u; ++c) in->whose[at++] = *c;
        in->whose[at] = 0;
        in->handle = op1f_memory_more(e->state_bytes, e->state_align, in->whose);
        if (in->handle <= 0) {
            in->status = REFUSED;
            op1f_log(1, LOG_FILE, 9, "effect: '%s' has no room for %s: %s", e->name, for_what,
                     in->handle == OP1F_TOO_LARGE ? "the packs would take more than 16 MiB" : "the heap has no such room, or less than 8 MiB would stay free");
            return REFUSED;
        }
        in->status = CLEARING;
    }
    void* block = op1f_memory_block(in->handle);
    if (block == 0) return CLEARING;
    in->state = block;
    BARRIER();                                                                      // the state before the word that hands it over
    in->status = READY;
    op1f_log(3, LOG_FILE, 8, "effect: '%s' has %u bytes for %s at 0x%08x", e->name, e->state_bytes, for_what, (u32)block);
    return READY;
}

// `reset` called for the effect a place rides, outside every pack's frame function, recorded as a call of its pack.
static void call_reset(const Effect* e, u32 place, void* state) {
    Op1fCallKept kept;
    op1f_cards_call_in((u32)CALL_SHOWN, e->slot, &kept);
    e->table->reset(place, state);
    op1f_cards_call_out((u32)CALL_SHOWN, &kept);
}

// R16: until when, by where: a place of the unit's (re-armed by a pick there, place_rearm), a run inside a pack (re-armed
// by the pack's new pick at its place after another, op1f_rack_ready). The widest line, a run's with
// line 11's numbers at their widest, is 218 characters: 258 with the logger's 40 before it.
static const char UNTIL_PLACE[] = "an effect is picked there again";
static const char UNTIL_RUN[] = "it is picked there again";

static void say_off(const Effect* e, const char* where, const char* until, Budget* b) {
    if (!b->off || b->off_said) return;
    b->off_said = 1;
    if (b->off_overs == 0)
        op1f_log(1, LOG_FILE, 10, "effect: '%s' at %s is switched off until %s: %u cycles in one call, %u declared", e->name, where, until, b->off_cycles, e->cycles);
    else
        op1f_log(1, LOG_FILE, 11, "effect: '%s' at %s is switched off until %s: %u calls in one second over twice its cost and its allowance, the most %u cycles, %u declared",
                 e->name, where, until, b->off_overs, b->off_cycles, e->cycles);
}

static void period() {
    if (--unit.countdown > 0) return;
    unit.countdown = PERIOD_FRAMES;
    u32 n = op1f_effects_registered;
    u32 calls[EFFECTS], sum[EFFECTS], most[EFFECTS], over[EFFECTS];
    u32 signature = quiet_mix(quiet_start(), n);
    int activity = 0;
    for (u32 k = 0; k < n; ++k) {                                                   // read and cleared, whatever the gate says
        Effect* e = &effects[k];
        u32 mask;
        MASK_OFF(mask);
        calls[k] = e->calls;
        sum[k] = e->sum;
        most[k] = e->most;
        over[k] = e->over;
        e->calls = 0;
        e->sum = 0;
        e->most = 0;
        e->over = 0;
        MASK_ON(mask);
        signature = quiet_mix(signature, calls[k] != 0 ? 1u : 0u);
        signature = quiet_mix(signature, over[k] != 0 ? 1u : 0u);
        if (over[k] != 0) activity = 1;
    }
    for (u32 p = 0; p < (u32)PLACES; ++p) signature = quiet_mix(signature, places[p].ride | (places[p].budget.off << 8));
    if (!quiet_period(&unit.gate, signature, activity)) return;
    for (u32 k = 0; k < n; ++k) {
        if (calls[k] == 0) continue;
        op1f_log(3, LOG_FILE, 12, "effect: '%s' this period: %u call(s), %u cycles at most, %u on average, %u over", effects[k].name, calls[k], most[k],
                 divided(sum[k], calls[k]), over[k]);
    }
}

// R6d: the type the firmware's table of names gives a name, as enrol finds a ridden effect's: 1 and `type`, or 0.
static int stock_type(const char* text, u32* type) {
    for (u32 k = 0; k < (u32)STOCK_COUNT; ++k) {
        const char* name = (const char*)RD32(EFFECT_NAMES + 8u * k);
        u32 at = 0;
        while (at < (u32)NAME && RD8((u32)name + at) == (u8)text[at] && text[at] != 0) ++at;
        if (at < (u32)NAME && RD8((u32)name + at) == 0 && text[at] == 0) {
            *type = RD32(EFFECT_NAMES + 8u * k + 4u);
            return 1;
        }
    }
    return 0;
}

// R6d: the effect a sound slot's record's document names now (its fx_type): 1 and `type`, or 0 (the master, which has
// no record; no document; no such member; a name the table does not hold). The document read inside a mask, once a
// load's tell, on the display thread; the kind of each value held before the library is handed it. Out of line: its
// loop then counts with no index register.
static EFFECTS_APART int document_type(u32 place, u32* type) {
    if (place >= (u32)MASTER) return 0;
    const void* document = RECORD_DOCUMENT(place_record(place));
    int found = 0;
    u32 mask;
    MASK_OFF(mask);
    if (kind(document) == (u32)KIND_OBJECT) {
        const char* text = text_of(STOCK_JSON_MEMBER(document, KEY_FX_TYPE));
        if (text) found = stock_type(text, type);
    }
    MASK_ON(mask);
    return found;
}

// R6d: a heard value too old to be heard, its `live` cleared (a mask only when one is to be).
static void heard_expire() {
    u32 now = cycles_now(), old = 0;
    for (u32 p = 0; p < (u32)PLACES; ++p)
        if (heard[p].live && now - heard[p].stamp > HEARD_CYCLES) old |= 1u << p;
    if (old == 0) return;
    u32 mask;
    MASK_OFF(mask);
    now = cycles_now();
    for (u32 p = 0; p < (u32)PLACES; ++p)
        if ((old & (1u << p)) && now - heard[p].stamp > HEARD_CYCLES) heard[p].live = 0;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- R16 (hasis, open-field pull request 38): the runs' records kept small

// A record is spare when no place of any pack runs it and none has at the last two frames' looks (the audio thread
// reads rack_current once a call of op1f_rack_run, and a block is shorter than a frame), and its memory is not being
// cleared. A record that has just become ready is held for its place two looks more (frame_work), so that no other
// place takes it before its own asks again. A record switched off may be taken over: its new place gives it another go.
enum { WIPE_A_FRAME = 256 * 1024 };                    // bytes cleared a frame, as the memory unit clears a new block
static u8 rack_was[RACKS];                             // 2 at a look that found the record run, one less at each look after

static void rack_look() {
    u8 now[RACKS];
    for (u32 i = 0; i < (u32)RACKS; ++i) now[i] = 0;
    for (u32 c = 0; c < (u32)CARDS; ++c)
        for (u32 w = 0; w < (u32)RACK_PLACES; ++w) {
            u32 r = rack_current[c][w];
            if (r != 0 && r <= rack_count) now[r - 1u] = 1;
        }
    for (u32 i = 0; i < rack_count; ++i) {
        if (now[i]) rack_was[i] = 2;
        else if (rack_was[i]) --rack_was[i];
    }
}

static int rack_idle(u32 i) {
    for (u32 c = 0; c < (u32)CARDS; ++c)
        for (u32 w = 0; w < (u32)RACK_PLACES; ++w)
            if (rack_current[c][w] == i + 1u) return 0;
    const Rack* k = &racks[i];
    return rack_was[i] == 0 && k->in.status != (u32)CLEARING && k->in.status != (u32)WIPING;
}

// A spare record of the same effect (its memory fits as it is), or 0.
static u32 rack_spare(u32 effect) {
    for (u32 i = 0; i < rack_count; ++i)
        if (racks[i].effect == effect && racks[i].in.status == (u32)READY && rack_idle(i)) return i + 1u;
    return 0;
}

// At 64 records: any spare record without memory, else a spare one whose memory fits `effect` (as big, on its
// alignment: it is cleared again and kept, the review: no block left behind, since the heap gives nothing back), or 0,
// "no room" as before base 38.
static u32 rack_spare_any(u32 effect) {
    for (u32 i = 0; i < rack_count; ++i) {
        u32 status = racks[i].in.status;
        if (rack_idle(i) && (status == (u32)NONE || status == (u32)REFUSED || effects[racks[i].effect].state_bytes == 0)) return i + 1u;
    }
    const Effect* e = &effects[effect];
    if (e->state_bytes == 0) return 0;
    for (u32 i = 0; i < rack_count; ++i) {
        const Rack* k = &racks[i];
        if (rack_idle(i) && k->in.status == (u32)READY && k->in.state != 0 && effects[k->effect].state_bytes >= e->state_bytes &&
            ((u32)k->in.state & (e->state_align - 1u)) == 0)
            return i + 1u;
    }
    return 0;
}

// A spare record handed to (slot, place, effect), a fresh start: its memory (the same effect's, or one that fits,
// rack_spare_any) kept and cleared again over the frames (rack_wipe; op1f_rack_ready answers OP1F_NOT_NOW meanwhile,
// op1f_rack_run OP1F_NOT_HERE, and no other place takes it); a record without memory asks its own as a new record does.
// Its budget re-armed: no place runs it.
static void rack_take(Rack* k, u32 slot, u32 which, u32 effect) {
    if (k->in.status == (u32)READY && k->in.state != 0 && effects[effect].state_bytes != 0) {
        k->wiped = 0;
        k->in.status = WIPING;
    } else {
        k->in.status = NONE;
        k->in.state = 0;
        k->in.handle = 0;
    }
    budget_rearm(&k->budget);
    k->slot = slot;
    k->which = which;
    k->effect = effect;
    k->fresh = 0;
    BARRIER();
}

// Up to `room` words of record i's kept memory cleared, from where the last frame stopped; ready once its last word is
// 0, and held for its place (rack_was). Answers the words left for the other records.
static u32 rack_wipe(u32 i, u32 room) {
    Rack* k = &racks[i];
    u32 words = (effects[k->effect].state_bytes + 3u) >> 2;
    u32 end = k->wiped + room < words ? k->wiped + room : words;
    volatile u32* block = (volatile u32*)k->in.state;
    for (u32 w = k->wiped; w < end; ++w) block[w] = 0;
    room -= end - k->wiped;
    k->wiped = end;
    if (end == words) {
        BARRIER();                                                                  // every word cleared before the word that hands it over
        k->in.status = READY;
        rack_was[i] = 2;
    }
    return room;
}

// ---------------------------------------------------------------- R22 (base 45): hosted pages

// A host's place lent or given a kept text (op1f_rack_place), found by the host's slot + 1 and its `which`: the unit's
// place + 1 (0 for none) and the kept text of the wagon's effect, `effect` that effect's number + 1, 0 while the text
// waits for the place's next ready; `told`, `changed` owed to the effect's pack; `wrote`, the pack wrote it since the
// host's last op1f_rack_kept. The display thread's, but for `place` copied into rack_lent.
enum { LENDS = RACKS, KEPT_MOST = OP1F_HOSTED_KEPT_MOST, MEMBERS_MOST = 32, KEEPS_BYTES = 324, DEPTH_MOST = 4 };
struct Lend { u16 host; u8 which, place, effect, told, wrote, length; char text[KEPT_MOST]; };   // 136 bytes
static_assert(sizeof(Lend) == 136 && sizeof(Op1fKeeps) == KEEPS_BYTES, "a lend's 136 bytes, and level 7's table");
static Lend lends[LENDS];
static volatile u8 rack_lent[RACKS];                   // a record's lent place + 1, 0 for none (the audio thread reads it)
static volatile u8 rack_on[RACKS];                     // the `on` of a record's last run
static volatile int rack_values[RACKS][VALUES];        // the values of a record's last run, in the effect's numbers
static volatile u32 rack_stamp[RACKS];                 // the cycle counter at that run

// The hosted page a host asks for (op1f_rack_page): its slot + 1 (0 for none), its layer's ask, its `which`, the
// record + 1, the place on the screen, the frame of its last ask; `up` as the cards unit says it; the four words held
// and those the page changed; the turns noted on the key thread, not yet moved.
static struct {
    volatile u32 host, record;
    u32 ask, which, place, asked, changed;
    volatile u32 up;
    u32 words[4];
    volatile int turns[4];
} page;

static EFFECTS_APART Lend* lend_of(u32 slot, u32 which) {                       // apart: inlined, it walked the lends with an index register
    for (u32 i = 0; i < (u32)LENDS; ++i)
        if (lends[i].host == slot + 1u && lends[i].which == which) return &lends[i];
    return 0;
}

static EFFECTS_APART Lend* lend_free() {
    for (u32 i = 0; i < (u32)LENDS; ++i)
        if (lends[i].host == 0) return &lends[i];
    return 0;
}

// Does the place + 1 `lent` ride a layer of the host's card `slot`? Any thread: a few reads.
static int lent_ok(u32 lent, u32 slot) {
    if (lent == 0 || lent > (u32)PLACES) return 0;
    const Effect* l = riding(lent - 1u);
    return is_layer(l) && l->slot == slot;
}

// The place record i's effect is told (`process`, `reset`, `shown`, the helpers): its own for an effect that declares
// wagons, the lent place in the fallback while it rides the host's layer, else as before base 45. Any thread.
static u32 run_place(u32 i) {
    const Rack* k = &racks[i];
    if (effects[k->effect].hosted) return (u32)OP1F_PLACE_HOSTED + i;
    u32 lent = rack_lent[i];
    return lent_ok(lent, k->slot) ? lent - 1u : (u32)OP1F_PLACE_HOSTED + k->which;
}

// Is record i the one its host's place runs now?
static int rack_live(u32 i) {
    return i < rack_count && rack_current[racks[i].slot][racks[i].which] == i + 1u;
}

// Does the page stand: asked at this frame or the one before, its record current and ready, and the place on the screen
// its place, riding the host's layer.
static int page_live() {
    u32 host = page.host, record = page.record;
    if (host == 0 || record == 0 || unit.frames - page.asked > 1u || !rack_live(record - 1u) || racks[record - 1u].in.status != (u32)READY) return 0;
    const Effect* l = riding(page.place);
    return is_layer(l) && l->slot == host - 1u && l->ask == page.ask && shown_place(RD32(CURRENT_MODE)) == page.place;
}

// Is record i's page the one on the screen?
static int page_holds(u32 i) {
    return page.up && page.record == i + 1u && page_live();
}

// The turns noted moved onto the page's words, as the unit's knob would move them: a 128th of the knob's row a step,
// held to the row (the display thread).
static EFFECTS_APART void page_move() {
    u32 record = page.record;
    if (record == 0 || record > rack_count) return;
    int steps[4];
    u32 mask;
    MASK_OFF(mask);
    for (u32 i = 0; i < 4u; ++i) {
        int t = page.turns[i];
        steps[i] = t >= 0 ? (t >> 10) : -((-t) >> 10);
        page.turns[i] = t - steps[i] * 1024;
    }
    MASK_ON(mask);
    const Effect* e = &effects[racks[record - 1u].effect];
    for (u32 i = 0; i < e->count; ++i) {
        if (steps[i] == 0 || e->fixed[i]) continue;
        int low = e->row_low[i], high = e->row_high[i];
        int step = (high - low) >> 7;
        if (step < 1) step = 1;
        int v = low < 0 ? ((int)page.words[i] >> 16) : (int)(page.words[i] >> 16);
        v += steps[i] * step;
        if (v < low) v = low;
        if (v > high) v = high;
        page.words[i] = (u32)v << 16;
        page.changed |= 1u << i;
    }
}

// `changed(OP1F_KEEP_HOSTED, n, 0)` of the effect's card's ask of kind OP1F_ASK_KEEPS, when it has one with `changed`,
// recorded as a call of its pack (the display thread, before the cards' frame functions).
static EFFECTS_APART void tell_kept(const Effect* e, u32 n) {
    const Op1fCardSlot* s = &op1f_cards_2.slots[e->slot];
    const Op1fCard* card = s->card;
    u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
    for (u32 k = 0; k < asks; ++k) {
        const Op1fAsk* a = &card->asks[k];
        if (a->kind != (u32)OP1F_ASK_KEEPS) continue;
        if (a->bytes < (u32)KEEPS_BYTES || !inside((u32)a->what, s, (u32)KEEPS_BYTES) || ((u32)a->what & 3u) != 0) return;
        const Op1fKeeps* t = (const Op1fKeeps*)a->what;
        u32 f = (u32)t->changed;
        if (f == 0 || !inside(f, s, 2u) || (f & 1u) != 0) return;
        Op1fCallKept kept;
        op1f_cards_call_in((u32)CALL_SHOWN, e->slot, &kept);
        t->changed((u32)OP1F_KEEP_HOSTED, n, 0);
        op1f_cards_call_out((u32)CALL_SHOWN, &kept);
        return;
    }
}

// The texts handed or dropped since the last frame told to the packs of the effects that declare wagons.
static EFFECTS_APART void lends_tell() {
    for (u32 i = 0; i < (u32)LENDS; ++i) {
        Lend* d = &lends[i];
        if (d->host == 0 || !d->told || d->effect == 0) continue;
        u32 r = rack_current[d->host - 1u][d->which];
        if (r == 0 || r > rack_count || racks[r - 1u].effect + 1u != d->effect) continue;
        d->told = 0;
        const Effect* e = &effects[d->effect - 1u];
        if (e->hosted) tell_kept(e, r - 1u);
    }
}

// A ready at a host's place: the record's lent place, and the kept text taken for its effect (the text handed since the
// last ready), kept (the same effect), or dropped (another). The display thread, before the record's `reset`.
static EFFECTS_APART void lend_bind(u32 slot, u32 which, u32 i, u32 effect) {
    Lend* d = lend_of(slot, which);
    rack_lent[i] = d ? d->place : (u8)0;
    if (d == 0 || d->effect == effect + 1u) return;
    if (d->effect != 0) {                                                           // another effect: its text dropped
        if (d->length) d->told = 1;
        d->length = 0;
        d->wrote = 0;
    }
    d->effect = (u8)(effect + 1u);
}

// A place that begins to run record i: its last run's values at their starts, on.
static EFFECTS_APART void rack_begun(u32 slot, u32 which, u32 i, const Effect* e) {
    for (u32 v = 0; v < e->count; ++v) rack_values[i][v] = e->start[v];
    rack_on[i] = 1;
    rack_stamp[i] = cycles_now() - HEARD_CYCLES - 1u;
    Lend* d = lend_of(slot, which);
    if (d && e->hosted) d->told = 1;                                                // a new instance: its pack reads its text
}

// The values of record i's run kept, with the time (the audio thread; apart, so that op1f_rack_run's own code stays).
static EFFECTS_APART void rack_ran(u32 i, const Effect* e, const int* values) {
    for (u32 v = 0; v < e->count; ++v) rack_values[i][v] = values[v];
    rack_stamp[i] = cycles_now();
}

// The instance of `e` the helpers answer at `place`, into `found`: an instance's own place for an effect that declares
// wagons; in the fallback, at a lent place, the instance whose page is up, else the first lent one. 0 for none.
static EFFECTS_APART int instance_at(const Effect* e, u32 place, u32* found) {
    if (e->hosted) {
        u32 i = place - (u32)OP1F_PLACE_HOSTED;
        if (place < (u32)OP1F_PLACE_HOSTED || i >= rack_count || racks[i].effect != e->number || !rack_live(i)) return 0;
        *found = i;
        return 1;
    }
    if (place >= (u32)PLACES) return 0;
    u32 record = page.record;
    if (record != 0 && record <= rack_count && racks[record - 1u].effect == e->number && page_holds(record - 1u) && run_place(record - 1u) == place) {
        *found = record - 1u;
        return 1;
    }
    for (u32 i = 0; i < rack_count; ++i)
        if (racks[i].effect == e->number && rack_live(i) && rack_lent[i] == place + 1u && lent_ok(place + 1u, racks[i].slot)) {
            *found = i;
            return 1;
        }
    return 0;
}

// The places lent to instances of `e` that ride their hosts' layers now, a bit a place (the fallback).
static EFFECTS_APART u32 lent_riding(const Effect* e) {
    u32 bits = 0;
    if (e->hosted) return 0;
    for (u32 i = 0; i < rack_count; ++i) {
        u32 lent = rack_lent[i];
        if (racks[i].effect == e->number && rack_live(i) && lent_ok(lent, racks[i].slot)) bits |= 1u << (lent - 1u);
    }
    return bits;
}

static void frame_work() {
    if (!unit.first_done) {
        unit.first_done = 1;
        first_frame();
        return;
    }
    u32 n = op1f_effects_registered;
    if (n == 0) return;
    ++unit.frames;
    rack_look();                                                                    // R16 (hasis): which runs the packs' places still have
    page_move();                                                                    // R22 (base 45): the hosted page's turns
    lends_tell();                                                                   // R22: the kept texts handed, told
    u32 mask;
    MASK_OFF(mask);
    u32 began = picked, stopped = ended, loaded = told, by_load = told_load;         // R6d: by_load
    u32 put_back = back;                                                            // R11
    back = 0;
    told_load = 0;
    picked = 0;
    ended = 0;
    told = 0;
    MASK_ON(mask);
    for (u32 p = 0; p < (u32)PLACES; ++p) {
        u32 bit = 1u << p;
        if (stopped & bit) {
            say_ride(p, 0, "another effect was picked", 0);
            write_member(p);
        }
        if ((put_back & bit) && places[p].ride != 0) {                              // R11: the switch of the list's opening put back
            set_place_on(p, back_on[p]);
            if (p < (u32)MASTER) STOCK_SET_ACTIVE(place_record(p), back_on[p]);
            if (!rides_layer(p) && places[p].on != back_on[p]) {                    // the review's S3: the member follows
                places[p].on = back_on[p];
                places[p].member_dirty = 1;
            }
        }
        if ((began & bit) && rides_layer(p)) {                                      // R11: a layer's pick: no values, the member
            say_ride(p, effects[places[p].ride - 1u].name, 0, 0);
            write_member(p);
        } else if ((began & bit) && places[p].ride != 0) {                          // a pick: its start values, the on state the pick set
            const Effect* e = &effects[places[p].ride - 1u];
            u32 four[4] = {0, 0, 0, 0};
            for (u32 i = 0; i < e->count; ++i) four[i] = word_of(e, i, e->start[i]);
            set_words(p, four, (1u << e->count) - 1u);
            places[p].on = place_on(p);
            say_ride(p, e->name, 0, 0);
            write_member(p);
        }
        if (loaded & bit) {                                                         // R6d (F-1): the loaded sound's effect on the slot first
            u32 type = 0;
            if ((by_load & bit) && document_type(p, &type) && place_type(p) != type) {   // a start or a drop: read at once
                waiting |= bit;
                wait_type[p] = type;
                wait_from[p] = place_type(p);
                wait_frames[p] = 0;
            } else {
                waiting &= ~bit;
                read_member(p, 1);
            }
        } else if (waiting & bit) {
            u32 now = place_type(p);
            if (now == wait_type[p] || now != wait_from[p]) {                       // the apply has come
                waiting &= ~bit;
                read_member(p, 1);
            } else if (++wait_frames[p] >= (u32)WAIT_FRAMES) {
                waiting &= ~bit;
                char words[12];
                op1f_log(2, LOG_FILE, 13, "effect: %s waited %u frames for the loaded sound's %s, which did not come: its member is read as the slot stands",
                         place_words_for_log(p, words), wait_frames[p], stock_name(wait_type[p]));
                read_member(p, 1);
            }
        }
    }
    for (u32 p = 0; p < (u32)PLACES; ++p) {
        const Effect* e = riding(p);
        if (e == 0 || (waiting & (1u << p)) != 0) continue;                         // R6d: a place that waits is read once its apply has come
        if (!is_layer(e) && place_type(p) != e->type) {                             // the unit, MIDI or the train put another effect there (R11: a layer stays)
            say_ride(p, 0, 0, stock_name(place_type(p)));
            end_ride(p, 0);
            continue;
        }
        u32 on = place_on(p) ? 1u : 0u;
        if (!is_layer(e) && on != places[p].on) {
            places[p].on = on;
            places[p].member_dirty = 1;
        }
        if (places[p].member_dirty) write_member(p);
        u32 index = p == (u32)MASTER ? 1u : 0u;
        make_ready(&instances[e->number][index], e, index ? "the master" : "the instrument");
        char words[12];
        say_off(e, place_words_for_log(p, words), UNTIL_PLACE, &places[p].budget);
    }
    for (u32 p = 0; p < (u32)PLACES; ++p)                                           // a member refused with OP1F_NOT_NOW, of a place that rides no more
        if (places[p].member_dirty && riding(p) == 0) write_member(p);
    for (u32 index = 0; index < 2u; ++index) {                                      // `reset` for a build that rides
        const Wrap* w = (const Wrap*)RD32(EFFECT_WORD + 4u * index);
        if (w != &wraps[index][0] && w != &wraps[index][1]) continue;
        u32 place = index ? (u32)MASTER : playing_place();
        u32 r = may_play(w, index, place);
        if (r == 0 || reset_seq[index] == w->seq) continue;
        const Effect* e = &effects[r - 1u];
        const Instance* in = &instances[r - 1u][index];
        if (in->status != (u32)READY) continue;
        if (e->table->reset) call_reset(e, place, in->state);
        reset_seq[index] = w->seq;
    }
    u32 room = (u32)WIPE_A_FRAME >> 2;                                              // R16: words, for every run's memory together
    for (u32 i = 0; i < rack_count; ++i) {
        Rack* k = &racks[i];
        if (k->in.status == (u32)WIPING) room = rack_wipe(i, room);                  // R16
        else if (k->in.status == (u32)CLEARING && make_ready(&k->in, &effects[k->effect], k->whose) == (u32)READY) rack_was[i] = 2;   // R16: held for its place
        say_off(&effects[k->effect], k->whose, UNTIL_RUN, &k->budget);
    }
    heard_expire();                                                                 // R6d
    period();
}

// Does an effect's page stand in now, or would it if the controller asked for its screen (module/build_r5_base.h)?
// `asked` 1: only while the screen the controller asked for is the ridden type's effect screen (op1f_effects_standing);
// 0: whatever screen it asked for (op1f_effects_ahead). Nothing else differs between the two.
static int stand(Op1fStanding* out, unsigned* mode, unsigned* screen, int asked) {
    if (op1f_effects_registered == 0) return 0;
    u32 m = RD32(CURRENT_MODE);
    u32 place = shown_place(m);
    if (place >= (u32)PLACES) return 0;
    const Effect* e = riding(place);
    if (e == 0) return 0;
    u32 shown = e->screen;
    if (is_layer(e)) {                                                              // R11: the unit's page, or the layer's for the type the place holds
        u32 type = place_type(place);
        if (e->page != (u32)OP1F_LAYER_PAGE_OWN || type >= (u32)STOCK_COUNT || stock_screens[type] == 0) return 0;
        shown = stock_screens[type];
    } else if (place_type(place) != e->type) {
        return 0;
    }
    if (asked && op1f_screen_asked != shown) return 0;
    u32 index = place == (u32)MASTER ? 1u : 0u;
    out->slot = e->slot;
    out->ask = e->ask;
    out->where = place;
    out->how = (place_on(place) ? 0u : (u32)OP1F_SHOWN_OFF) | (instances[e->number][index].status == (u32)REFUSED ? (u32)OP1F_STAND_NO_ROOM : 0u) |
               (places[place].budget.off ? (u32)OP1F_STAND_OVER : 0u);
    *mode = m;
    *screen = shown;
    return 1;
}

extern "C" int op1f_effects_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) {
    return stand(out, mode, screen, 1);
}

extern "C" int op1f_effects_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) {
    return stand(out, mode, screen, 0);
}

// ---------------------------------------------------------------- the helpers

static const Effect* effect_of(int slot, u32 ask) {
    u32 n = op1f_effects_registered;
    for (u32 i = 0; i < n; ++i)
        if (effects[i].slot == (u32)slot && effects[i].ask == ask) return &effects[i];
    return 0;
}

static int from_frame(int slot) {
    return op1f_cards_calling() == (u32)slot + 1u;
}

// The kinds of ask that have values, as op1f_param_get, op1f_param_set and op1f_riding serve them: a row a kind, found
// by the kind of the card's ask, so that a later level adds its kind by one row (with its own place rules) and no helper
// and no stub (since step R6). The helpers hold the card and the caller's thread first; each
// row's functions answer the rest, as the header words the three helpers:
//   places   the places of the kind: 0 to places - 1
//   get      the value `index` at `place` into `out`: 0, OP1F_BAD_NAME (no such thing registered, no such value),
//            OP1F_NOT_HERE (the place does not hold it now)
//   set      the value set at `place`: the same answers
//   riding   the places that hold it now, a bit a place
struct Valued;
typedef int (*ValuedGetFn)(const Valued* row, int slot, u32 ask, u32 place, u32 index, int* out);
typedef int (*ValuedSetFn)(const Valued* row, int slot, u32 ask, u32 place, u32 index, int value);
typedef u32 (*ValuedRidingFn)(const Valued* row, int slot, u32 ask);
struct Valued { u32 kind, places; ValuedGetFn get; ValuedSetFn set; ValuedRidingFn riding; ValuedGetFn heard; };   // R6d: `heard`, level 10's

// Kind 7, an effect: a place is a sound's slot (0 to 15) or the master (16), and the values are its four knobs.
static int effect_get(const Valued* row, int slot, u32 ask, u32 place, u32 index, int* out) {
    const Effect* e = effect_of(slot, ask);
    if (e == 0 || index >= e->count || out == 0) return OP1F_BAD_NAME;
    if (place >= row->places || riding(place) != e) {
        u32 i = 0;                                                                  // R22 (base 45): an instance's place, or a lent one
        if (!instance_at(e, place, &i)) return OP1F_NOT_HERE;
        *out = page_holds(i) ? value_of(e, index, page.words[index]) : rack_values[i][index];
        return 0;
    }
    *out = value_of(e, index, RD32(place_words(place) + 4u * index));
    return 0;
}

// R6d, level 10: the value as the place's effect was last called with it, or as set where it has not played in the
// last few blocks.
static int effect_heard(const Valued* row, int slot, u32 ask, u32 place, u32 index, int* out) {
    const Effect* e = effect_of(slot, ask);
    if (e == 0 || index >= e->count || out == 0) return OP1F_BAD_NAME;
    if (place >= row->places || riding(place) != e) {
        u32 i = 0;                                                                  // R22 (base 45): the last run's, or as set on the page
        if (!instance_at(e, place, &i)) return OP1F_NOT_HERE;
        int stale = cycles_now() - rack_stamp[i] > HEARD_CYCLES;
        *out = stale && page_holds(i) ? value_of(e, index, page.words[index]) : rack_values[i][index];
        return 0;
    }
    const Heard* h = &heard[place];
    for (u32 tries = 0; tries < (u32)HEARD_TRIES; ++tries) {
        u32 seq = h->seq;
        u32 ride = h->ride, live = h->live, stamp = h->stamp;
        int v = h->values[index];
        if ((seq & 1u) != 0 || h->seq != seq) continue;                             // a call came between: read again
        if (live && ride == e->number + 1u && cycles_now() - stamp <= HEARD_CYCLES) {
            *out = v;
            return 0;
        }
        break;
    }
    *out = value_of(e, index, RD32(place_words(place) + 4u * index));
    return 0;
}

static int effect_set(const Valued* row, int slot, u32 ask, u32 place, u32 index, int value) {
    const Effect* e = effect_of(slot, ask);
    if (e == 0 || index >= e->count) return OP1F_BAD_NAME;
    if (place >= row->places || riding(place) != e) {
        u32 i = 0;                                                                  // R22 (base 45): the page's word, while it is up
        if (!instance_at(e, place, &i) || !page_holds(i)) return OP1F_NOT_HERE;
        page.words[index] = word_of(e, index, value);
        page.changed |= 1u << index;
        return 0;
    }
    u32 four[4] = {0, 0, 0, 0};
    four[index] = word_of(e, index, value);
    set_words(place, four, 1u << index);
    return 0;
}

static u32 effect_riding(const Valued* row, int slot, u32 ask) {
    const Effect* e = effect_of(slot, ask);
    if (e == 0) return 0;
    u32 bits = 0;
    for (u32 p = 0; p < row->places; ++p)
        if (riding(p) == e) bits |= 1u << p;
    return bits | lent_riding(e);                                                   // R22 (base 45): the fallback's lent places
}

// R6: kind 8, an engine: a place is a synth sound's slot (0 to 7); the engines unit answers, the card and the thread
// held by the helper first. An ask that is no registered engine answers OP1F_BAD_NAME, as an ask of a kind with no row.
#if defined(OP1F_HOST)
#ifdef OP1F_R6_HOST_ENGINES
extern "C" int (*mock_engines_param_get)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out);
#define op1f_engines_param_get (*mock_engines_param_get)
extern "C" int (*mock_engines_param_set)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int value);
#define op1f_engines_param_set (*mock_engines_param_set)
extern "C" int (*mock_engines_riding)(unsigned card_slot, unsigned ask, unsigned* bits);
#define op1f_engines_riding (*mock_engines_riding)
extern "C" int (*mock_engines_param_heard)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out);   // R6d
#define op1f_engines_param_heard (*mock_engines_param_heard)
#else
static int (*const r6_none_engines_param_get)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) = 0;
#define op1f_engines_param_get (*r6_none_engines_param_get)
static int (*const r6_none_engines_param_set)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int value) = 0;
#define op1f_engines_param_set (*r6_none_engines_param_set)
static int (*const r6_none_engines_riding)(unsigned card_slot, unsigned ask, unsigned* bits) = 0;
#define op1f_engines_riding (*r6_none_engines_riding)
static int (*const r6_none_engines_param_heard)(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) = 0;   // R6d
#define op1f_engines_param_heard (*r6_none_engines_param_heard)
#endif
#else
extern "C" int op1f_engines_param_get(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_engines_param_get") __attribute__((weak));
extern "C" int op1f_engines_param_set(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int value) OP1F_NAMED("op1f_engines_param_set") __attribute__((weak));
extern "C" int op1f_engines_riding(unsigned card_slot, unsigned ask, unsigned* bits) OP1F_NAMED("op1f_engines_riding") __attribute__((weak));
extern "C" int op1f_engines_param_heard(unsigned card_slot, unsigned ask, unsigned place, unsigned index, int* out) OP1F_NAMED("op1f_engines_param_heard") __attribute__((weak));   // R6d
#endif

static int engine_get(const Valued* row, int slot, u32 ask, u32 place, u32 index, int* out) {
    (void)row;
    if (&op1f_engines_param_get == 0) return OP1F_BAD_NAME;
    int answer = op1f_engines_param_get((u32)slot, ask, place, index, out);
    return answer == (int)OP1F_ENGINE_NOT_MINE ? (int)OP1F_BAD_NAME : answer;
}

static int engine_set(const Valued* row, int slot, u32 ask, u32 place, u32 index, int value) {
    (void)row;
    if (&op1f_engines_param_set == 0) return OP1F_BAD_NAME;
    int answer = op1f_engines_param_set((u32)slot, ask, place, index, value);
    return answer == (int)OP1F_ENGINE_NOT_MINE ? (int)OP1F_BAD_NAME : answer;
}

static int engine_heard(const Valued* row, int slot, u32 ask, u32 place, u32 index, int* out) {   // R6d
    (void)row;
    if (&op1f_engines_param_heard == 0) return OP1F_BAD_NAME;
    int answer = op1f_engines_param_heard((u32)slot, ask, place, index, out);
    return answer == (int)OP1F_ENGINE_NOT_MINE ? (int)OP1F_BAD_NAME : answer;
}

static u32 engine_riding(const Valued* row, int slot, u32 ask) {
    (void)row;
    unsigned bits = 0;
    if (&op1f_engines_riding == 0 || op1f_engines_riding((u32)slot, ask, &bits) != 0) return 0;
    return bits;
}

static const Valued VALUED[] = {
    {OP1F_ASK_EFFECT, PLACES, effect_get, effect_set, effect_riding, effect_heard},
    {OP1F_ASK_ENGINE, SLOTS, engine_get, engine_set, engine_riding, engine_heard},             // R6; R6d: `heard`
};
enum { VALUED_KINDS = sizeof VALUED / sizeof VALUED[0] };

// The row of the kind of the card's ask `ask`, or 0: the card has no such ask, or its kind has no values.
static const Valued* valued(int slot, u32 ask) {
    const Op1fCard* card = op1f_cards_2.slots[slot].card;
    if (card->bytes < (u32)CARD_4_BYTES || card->asks == 0 || ask >= card->ask_count || ask >= (u32)OP1F_ASKS_MOST) return 0;
    u32 kind = card->asks[ask].kind;
    for (u32 i = 0; i < (u32)VALUED_KINDS; ++i)
        if (VALUED[i].kind == kind) return &VALUED[i];
    return 0;
}

extern "C" int op1f_param_get(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot) && !op1f_cards_inside(card)) return OP1F_NOT_NOW;
    const Valued* row = valued(slot, ask);
    if (row == 0) return OP1F_BAD_NAME;
    return row->get(row, slot, ask, place, index, out);
}

// R6d, level 10 (module/build_r6d_base.h words it): op1f_param_get's checks, then the row's value as heard.
extern "C" int op1f_param_heard(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int* out) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot) && !op1f_cards_inside(card)) return OP1F_NOT_NOW;
    const Valued* row = valued(slot, ask);
    if (row == 0) return OP1F_BAD_NAME;
    return row->heard(row, slot, ask, place, index, out);
}

extern "C" int op1f_param_set(const Op1fCard* card, unsigned ask, unsigned place, unsigned index, int value) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    const Valued* row = valued(slot, ask);
    if (row == 0) return OP1F_BAD_NAME;
    return row->set(row, slot, ask, place, index, value);
}

static u32 ask_kind(int slot, u32 ask);

// R11: an ask of kind OP1F_ASK_LFO handed to the LFO unit (no values: the value helpers answer OP1F_BAD_NAME for it, as
// for a kind with no row).
extern "C" unsigned op1f_riding(const Op1fCard* card, unsigned ask) {
    int slot = slot_of(card);
    if (slot < 0) return 0;
    const Valued* row = valued(slot, ask);
    if (row == 0) return ask_kind(slot, ask) == (u32)OP1F_ASK_LFO && &op1f_lfos_riding != 0 ? op1f_lfos_riding(card, ask) : 0u;   // R11
    return row->riding(row, slot, ask);
}

extern "C" unsigned op1f_rack_at(const Op1fCard* card, unsigned place) {
    if (slot_of(card) < 0 || place >= (u32)PLACES) return 0;
    const Effect* e = riding(place);
    return e ? (u32)ID_FIRST + e->number : 0u;
}

extern "C" const Op1fRackEntry* op1f_rack_list(const Op1fCard* card, unsigned* count) {
    u32 n = slot_of(card) < 0 ? 0u : op1f_effects_registered;
    if (count) *count = n;
    return n ? entries : 0;
}

// A run's name for the log, "place N of 'pack'", written into `whose` (48 bytes).
static EFFECTS_APART void rack_name(char* whose, const Op1fCard* card, u32 which) {
    u32 at = 0;
    for (const char* c = "place "; *c; ++c) whose[at++] = *c;
    whose[at++] = (char)('0' + which);
    for (const char* c = " of '"; *c; ++c) whose[at++] = *c;
    for (u32 c = 0; c < 15u && card->name[c]; ++c) whose[at++] = card->name[c];
    whose[at++] = '\'';
    whose[at] = 0;
}

// The record + 1 of (slot, place, effect): its own, a spare one taken over, or a new one; 0 when there is no room (a line).
// R16: apart from op1f_rack_ready, whose code it was, so that neither is left short of registers (the listing's rule: no
// index register stepped outside op1f_effects_catalog_read).
static EFFECTS_APART u32 rack_record(const Op1fCard* card, u32 slot, u32 which, u32 effect) {
    for (u32 i = 0; i < rack_count; ++i)
        if (racks[i].slot == slot && racks[i].which == which && racks[i].effect == effect) return i + 1u;
    // R16 (hasis): a record no place runs any more is taken over before a new one is made, one of the same effect first
    // (its memory with it); at 64, a spare one without memory or whose memory fits. Walking a link of a chain through the
    // list no longer uses the 64 up. Its name is written into it in place (the review's listing: no copy of a local).
    Rack* k = 0;
    u32 record = rack_spare(effect);
    if (record == 0 && rack_count == (u32)RACKS) record = rack_spare_any(effect);
    if (record != 0) {
        k = &racks[record - 1u];
        rack_take(k, slot, which, effect);
    } else {
        if (rack_count == (u32)RACKS) {
            char whose[48];
            rack_name(whose, card, which);
            op1f_log(1, LOG_FILE, 9, "effect: '%s' has no room for %s: %s", effects[effect].name, whose, "the base keeps 64 runs inside packs");
            return 0;
        }
        k = &racks[rack_count];
        k->slot = slot;
        k->which = which;
        k->effect = effect;
        BARRIER();
        record = ++rack_count;
    }
    rack_name(k->whose, card, which);
    return record;
}

extern "C" int op1f_rack_ready(const Op1fCard* card, unsigned which, unsigned id) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    u32 n = op1f_effects_registered;
    if (which >= (u32)RACK_PLACES || id < (u32)ID_FIRST || id >= (u32)ID_FIRST + n) return OP1F_BAD_NAME;
    u32 effect = id - (u32)ID_FIRST;
    if (is_layer(&effects[effect])) return OP1F_BAD_NAME;                           // R11: a layer runs only over a place of the unit's
    u32 record = rack_record(card, (u32)slot, which, effect);
    if (record == 0) return OP1F_FAILED;
    Rack* k = &racks[record - 1u];
    rack_was[record - 1u] = 2;                                                      // R16 (the review): its place asks for it
    lend_bind((u32)slot, which, record - 1u, effect);                               // R22 (base 45): the lend before any `reset`
    const Effect* e = &effects[effect];
    u32 status = make_ready(&k->in, e, k->whose);
    if (status == (u32)CLEARING) return OP1F_NOT_NOW;
    if (status == (u32)REFUSED) return k->in.handle == OP1F_TOO_LARGE ? OP1F_TOO_LARGE : OP1F_FAILED;
    u32 asked = rack_asked[slot][which], tried = rack_tried[slot][which];
    rack_asked[slot][which] = unit.frames;
    if (k->budget.off) {                                                            // R16 (hasis): another go on a new pick
        if (rack_current[slot][which] == record) {
            if (unit.frames - asked <= (u32)RACK_PAUSE || unit.frames - tried <= (u32)RACK_PAUSE) return OP1F_NOT_HERE;
            if (e->table->reset) call_reset(e, run_place(record - 1u), k->in.state);   // asked again after a pause: `reset` (R22: its place)
            k->fresh = 1;                                                           // returned while `off` keeps the audio thread out
            BARRIER();                                                              // (the review), then the go
            budget_rearm(&k->budget);
            return 0;
        }
        budget_rearm(&k->budget);                                                   // picked again after another: the place names it
    }                                                                               // only once its `reset` has returned, below
    if (rack_current[slot][which] != record) {                                      // the place held another effect, or none
        rack_begun((u32)slot, which, record - 1u, e);                               // R22 (base 45)
        if (e->table->reset) call_reset(e, run_place(record - 1u), k->in.state);    // R22: the instance's place, or the lent one
        k->fresh = 1;
        BARRIER();
        rack_current[slot][which] = (u8)record;
    }
    return 0;
}

extern "C" int op1f_rack_run(const Op1fCard* card, unsigned which, const Op1fRackRun* r) {
    int slot = slot_of(card);
    u32 n = op1f_effects_registered;
    if (slot < 0 || r == 0 || r->bytes < (u32)RUN_BYTES || which >= (u32)RACK_PLACES || r->frames != (u32)FRAMES || r->left == 0 || r->right == 0 ||
        r->values == 0 || r->form > (u32)OP1F_RACK_VALUES || r->id < (u32)ID_FIRST || r->id >= (u32)ID_FIRST + n)
        return OP1F_BAD_NAME;
    rack_tried[slot][which] = unit.frames;                                         // R16: the place tried to run (rack_tried)
    u32 record = rack_current[slot][which];
    if (record == 0 || record > rack_count) return OP1F_NOT_HERE;
    Rack* k = &racks[record - 1u];
    if (k->effect != r->id - (u32)ID_FIRST || k->in.status != (u32)READY || k->budget.off) return OP1F_NOT_HERE;
    Effect* e = &effects[k->effect];
    rack_on[record - 1u] = r->on ? 1u : 0u;                                         // R22 (base 45): for the hosted page
    if (r->on == 0 && (e->flags & (u32)OP1F_EFFECT_TAILS) == 0) return 0;           // switched off, and nothing to ring out
    int values[VALUES];
    for (u32 i = 0; i < e->count; ++i) {
        int v = r->values[i];
        if (r->form == (u32)OP1F_RACK_WORDS) v = value_of(e, i, (u32)v);
        else v = v < e->low[i] ? e->low[i] : v > e->high[i] ? e->high[i] : v;
        values[i] = v;
    }
    u32 fresh = r->fresh || k->fresh;
    k->fresh = 0;
    rack_ran(record - 1u, e, values);                                               // R22: the last run's values, as heard
    run(e, run_place(record - 1u), r->left, r->right, values, r->on ? 1u : 0u, fresh, k->in.state, &k->budget);   // R22: its place
    return 0;
}

// ---------------------------------------------------------------- R11: layers' helpers and the frame's end

extern "C" void op1f_effects_frame() {
    frame_work();
    if (&op1f_lfos_frame != 0) op1f_lfos_frame();                                   // R11: the LFO unit's, when it is linked
}

// The kind of the card's ask `ask`, or 0 (the card's slot held already).
static u32 ask_kind(int slot, u32 ask) {
    const Op1fCard* card = op1f_cards_2.slots[slot].card;
    if (card->bytes < (u32)CARD_4_BYTES || card->asks == 0 || ask >= card->ask_count || ask >= (u32)OP1F_ASKS_MOST) return 0;
    return card->asks[ask].kind;
}

// The layer of the card's ask `ask`, or 0.
static const Effect* layer_of(int slot, u32 ask) {
    const Effect* e = effect_of(slot, ask);
    return is_layer(e) ? e : 0;
}

extern "C" int op1f_layer_start(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_layer_start");
extern "C" int op1f_layer_start(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (ask_kind(slot, ask) == (u32)OP1F_ASK_LFO) return &op1f_lfos_layer_start != 0 ? op1f_lfos_layer_start(card, ask, place) : (int)OP1F_BAD_NAME;
    const Effect* e = layer_of(slot, ask);
    if (e == 0 || place >= (u32)PLACES) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    u32 bit = 1u << place, r = e->number + 1u, was = places[place].ride, mask;
    if ((waiting & bit) != 0 || (told & bit) != 0) return OP1F_NOT_NOW;             // a sound loaded there, not read yet
    if (was == r) return 0;
    int layer = rides_layer(place);
    MASK_OFF(mask);
    if (was != 0 && !layer) take_back_pair(place, was);                             // a ride there ends first, as a pick of another row
    places[place].ride = r;
    picked &= ~bit;
    ended &= ~bit;
    back &= ~bit;
    MASK_ON(mask);
    if (was != 0 && !layer && place < (u32)MASTER) STOCK_SET_ACTIVE(place_record(place), places[place].on);   // the file says what played
    made_fresh(place);
    say_ride(place, e->name, 0, 0);
    write_member(place);
    return 0;
}

extern "C" int op1f_layer_end(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_layer_end");
extern "C" int op1f_layer_end(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (ask_kind(slot, ask) == (u32)OP1F_ASK_LFO) return &op1f_lfos_layer_end != 0 ? op1f_lfos_layer_end(card, ask, place) : (int)OP1F_BAD_NAME;
    const Effect* e = layer_of(slot, ask);
    if (e == 0 || place >= (u32)PLACES) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    if (places[place].ride != e->number + 1u) return OP1F_NOT_HERE;
    say_ride(place, 0, "its pack ended it", 0);
    end_ride(place, 0);
    return 0;
}

extern "C" int op1f_layer_page(const Op1fCard* card, unsigned ask, unsigned which) OP1F_NAMED("op1f_layer_page");
extern "C" int op1f_layer_page(const Op1fCard* card, unsigned ask, unsigned which) {
    int slot = slot_of(card);
    if (slot < 0) return 0;
    if (ask_kind(slot, ask) == (u32)OP1F_ASK_LFO) return &op1f_lfos_layer_page != 0 ? op1f_lfos_layer_page(card, ask, which) : 0;
    const Effect* e = layer_of(slot, ask);
    if (e == 0 || (which != (u32)OP1F_LAYER_PAGE_UNIT && which != (u32)OP1F_LAYER_PAGE_OWN) || !from_frame(slot)) return 0;
    effects[e->number].page = which;
    return 1;
}

// The mark of a base that carries out layers (base 35).
extern "C" const u32 op1f_layer_1 OP1F_NAMED("op1f_layer_1");
const u32 op1f_layer_1 = 1;

// ---------------------------------------------------------------- R22 (base 45): hosted pages, the helpers

// The mark of a base that carries out hosted pages (base 45).
const u32 op1f_hosted_1 = 1;

// JSON texts of the kinds level 7 keeps, for the kept texts: whole numbers of 32 bits, true, false, null, texts, and
// lists and objects of these, at most DEPTH_MOST deep. The end of the value at `at` (spaces before it passed), or 0.
static u32 past_space(const char* s, u32 at, u32 n) {
    while (at < n && (s[at] == ' ' || s[at] == '\t' || s[at] == '\n' || s[at] == '\r')) ++at;
    return at;
}

static int hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static u32 word_end(const char* s, u32 at, u32 n, const char* word) {
    for (; *word; ++word, ++at)
        if (at >= n || s[at] != *word) return 0;
    return at;
}

static u32 text_end(const char* s, u32 at, u32 n) {
    for (++at; at < n; ++at) {
        char c = s[at];
        if (c == '"') return at + 1u;
        if ((u8)c < 0x20u) return 0;
        if (c != '\\') continue;
        if (++at >= n) return 0;
        c = s[at];
        if (c == 'u') {
            for (u32 h = 0; h < 4u; ++h)
                if (++at >= n || !hex_digit(s[at])) return 0;
        } else if (c != '"' && c != '\\' && c != '/' && c != 'b' && c != 'f' && c != 'n' && c != 'r' && c != 't') {
            return 0;
        }
    }
    return 0;
}

static u32 number_end(const char* s, u32 at, u32 n) {
    int minus = s[at] == '-';
    u32 first = minus ? at + 1u : at, end = first;
    while (end < n && s[end] >= '0' && s[end] <= '9') ++end;
    u32 digits = end - first;
    if (digits == 0 || digits > 10u || (digits > 1u && s[first] == '0')) return 0;
    if (end < n && (s[end] == '.' || s[end] == 'e' || s[end] == 'E')) return 0;    // no fraction, no exponent
    if (digits == 10u) {
        const char* most = minus ? "2147483648" : "2147483647";
        for (u32 k = 0; k < 10u; ++k) {
            if (s[first + k] == most[k]) continue;
            if (s[first + k] > most[k]) return 0;
            break;
        }
    }
    return end;
}

static u32 json_end(const char* s, u32 at, u32 n, u32 depth) {
    at = past_space(s, at, n);
    if (at >= n) return 0;
    char c = s[at];
    if (c == '"') return text_end(s, at, n);
    if (c == '-' || (c >= '0' && c <= '9')) return number_end(s, at, n);
    if (c == 't') return word_end(s, at, n, "true");
    if (c == 'f') return word_end(s, at, n, "false");
    if (c == 'n') return word_end(s, at, n, "null");
    if ((c != '{' && c != '[') || depth >= (u32)DEPTH_MOST) return 0;
    char close = c == '{' ? '}' : ']';
    at = past_space(s, at + 1u, n);
    if (at < n && s[at] == close) return at + 1u;
    for (;;) {
        if (c == '{') {
            if (at >= n || s[at] != '"') return 0;
            at = past_space(s, text_end(s, at, n), n);
            if (at == 0 || at >= n || s[at] != ':') return 0;
            ++at;
        }
        at = json_end(s, at, n, depth + 1u);
        if (at == 0) return 0;
        at = past_space(s, at, n);
        if (at >= n) return 0;
        if (s[at] == close) return at + 1u;
        if (s[at] != ',') return 0;
        at = past_space(s, at + 1u, n);
    }
}

// A member's name: 1 to 31 characters of a-z, 0-9 and '_' (a key's).
static int plain_member(const char* s, u32 n) {
    if (n == 0 || n >= (u32)OP1F_KEY_BYTES) return 0;
    for (u32 i = 0; i < n; ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return 0;
    }
    return 1;
}

static u32 length_most(const char* s, u32 most) {
    u32 n = 0;
    while (n < most && s[n]) ++n;
    return n;
}

// The members of a kept text (an object, or "" for none): their names and values as places in it. Answers how many,
// or -1 when it is not an object of plainly named members.
struct Member { u8 name, name_bytes, value, value_bytes; };
static int members_of(const char* s, u32 n, Member* m) {
    u32 at = past_space(s, 0, n), count = 0;
    if (at == n) return 0;
    if (s[at] != '{') return -1;
    at = past_space(s, at + 1u, n);
    if (at < n && s[at] == '}') return past_space(s, at + 1u, n) == n ? 0 : -1;
    for (;;) {
        if (count == (u32)MEMBERS_MOST || at >= n || s[at] != '"') return -1;
        u32 end = text_end(s, at, n);
        if (end == 0 || !plain_member(s + at + 1u, end - at - 2u)) return -1;
        m[count].name = (u8)(at + 1u);
        m[count].name_bytes = (u8)(end - at - 2u);
        at = past_space(s, end, n);
        if (at >= n || s[at] != ':') return -1;
        u32 first = past_space(s, at + 1u, n);
        end = json_end(s, first, n, 1u);
        if (end == 0) return -1;
        m[count].value = (u8)first;
        m[count].value_bytes = (u8)(end - first);
        ++count;
        at = past_space(s, end, n);
        if (at >= n) return -1;
        if (s[at] == '}') return past_space(s, at + 1u, n) == n ? (int)count : -1;
        if (s[at] != ',') return -1;
        at = past_space(s, at + 1u, n);
    }
}

// A name against a member's: less than 0, 0 or more than 0, as their bytes sort.
static int name_order(const char* a, u32 an, const char* b, u32 bn) {
    for (u32 i = 0; i < an && i < bn; ++i)
        if (a[i] != b[i]) return (int)(u8)a[i] - (int)(u8)b[i];
    return (int)an - (int)bn;
}

static int put_bytes(char* out, u32* at, const char* s, u32 n) {
    if (*at + n > (u32)KEPT_MOST) return 0;
    for (u32 i = 0; i < n; ++i) out[*at + i] = s[i];
    *at += n;
    return 1;
}

static int put_member(char* out, u32* at, const char* name, u32 name_bytes, const char* value, u32 value_bytes) {
    if (*at > 1u && !put_bytes(out, at, ",", 1u)) return 0;
    return put_bytes(out, at, "\"", 1u) && put_bytes(out, at, name, name_bytes) && put_bytes(out, at, "\":", 2u) && put_bytes(out, at, value, value_bytes);
}

// The kept text of instance `which` of a declared effect of the card at `slot`, or 0 (no such instance); `make`, a lend
// made for it when its place has none. `*why` the answer for 0.
static Lend* kept_of(int slot, u32 which, int make, int* why) {
    *why = OP1F_BAD_NAME;
    if (!rack_live(which)) return 0;
    const Rack* k = &racks[which];
    const Effect* e = &effects[k->effect];
    if (!e->hosted || e->slot != (u32)slot) return 0;
    Lend* d = lend_of(k->slot, k->which);
    if (d == 0 && make && (d = lend_free()) != 0) {
        d->host = (u16)(k->slot + 1u);
        d->which = (u8)k->which;
        d->place = 0;
        d->effect = (u8)(k->effect + 1u);
        d->told = 0;
        d->wrote = 0;
        d->length = 0;
    }
    *why = make ? (d == 0 ? OP1F_TOO_LARGE : OP1F_NOT_NOW) : OP1F_NO_FILE;
    return d != 0 && d->effect == k->effect + 1u ? d : 0;
}

extern "C" int op1f_effects_hosted_get(const Op1fCard* card, unsigned which, const char* key, char* out, unsigned room) {
    int slot = slot_of(card);
    if (slot < 0 || !from_frame(slot)) return OP1F_NOT_NOW;
    u32 key_bytes = key ? length_most(key, (u32)OP1F_KEY_BYTES) : 0u;
    if (!plain_member(key, key_bytes)) return OP1F_BAD_NAME;
    int why = 0;
    const Lend* d = kept_of(slot, which, 0, &why);
    if (d == 0) return why;
    Member m[MEMBERS_MOST];
    int count = members_of(d->text, d->length, m);
    for (int i = 0; i < count; ++i) {
        if (name_order(key, key_bytes, d->text + m[i].name, m[i].name_bytes) != 0) continue;
        u32 n = m[i].value_bytes;
        if (out == 0 || room <= n) return OP1F_TOO_LARGE;
        for (u32 b = 0; b < n; ++b) out[b] = d->text[m[i].value + b];
        out[n] = 0;
        return (int)n;
    }
    return OP1F_NO_FILE;
}

extern "C" int op1f_effects_hosted_set(const Op1fCard* card, unsigned which, const char* key, const char* text) {
    int slot = slot_of(card);
    if (slot < 0 || !from_frame(slot)) return OP1F_NOT_NOW;
    u32 key_bytes = key ? length_most(key, (u32)OP1F_KEY_BYTES) : 0u;
    if (!plain_member(key, key_bytes)) return OP1F_BAD_NAME;
    u32 first = 0, end = 0;
    if (text) {
        u32 n = length_most(text, (u32)KEPT_MOST + 1u);
        if (n > (u32)KEPT_MOST) return OP1F_TOO_LARGE;
        first = past_space(text, 0, n);
        end = json_end(text, first, n, 1u);
        if (end == 0 || past_space(text, end, n) != n) return OP1F_BAD_VALUE;
    }
    int why = 0;
    Lend* d = kept_of(slot, which, text != 0, &why);
    if (d == 0) return text || why == OP1F_BAD_NAME ? why : 0;                      // else nothing to take away
    Member m[MEMBERS_MOST];
    int count = members_of(d->text, d->length, m);
    if (count < 0) count = 0;                                                       // never: the texts are held to the form
    char out[KEPT_MOST];
    u32 at = 0, placed = text == 0;
    put_bytes(out, &at, "{", 1u);
    for (int i = 0; i < count; ++i) {
        const char* name = d->text + m[i].name;
        int order = name_order(key, key_bytes, name, m[i].name_bytes);
        if (!placed && order < 0) {
            if (!put_member(out, &at, key, key_bytes, text + first, end - first)) return OP1F_TOO_LARGE;
            placed = 1;
        }
        if (order == 0) continue;
        if (!put_member(out, &at, name, m[i].name_bytes, d->text + m[i].value, m[i].value_bytes)) return OP1F_TOO_LARGE;
    }
    if (!placed && !put_member(out, &at, key, key_bytes, text + first, end - first)) return OP1F_TOO_LARGE;
    if (!put_bytes(out, &at, "}", 1u)) return OP1F_TOO_LARGE;
    int same = at == d->length;
    for (u32 i = 0; same && i < at; ++i) same = out[i] == d->text[i];
    if (same) return 0;
    for (u32 i = 0; i < at; ++i) d->text[i] = out[i];
    d->length = (u8)at;
    d->wrote = 1;
    return 0;
}

extern "C" int op1f_rack_place(const Op1fCard* card, unsigned which, unsigned place, const char* kept) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    if (which >= (u32)RACK_PLACES || (place != (u32)OP1F_PLACE_NONE && place >= (u32)PLACES)) return OP1F_BAD_NAME;
    u32 n = kept ? length_most(kept, (u32)KEPT_MOST + 1u) : 0u;
    if (n > (u32)KEPT_MOST) return OP1F_TOO_LARGE;
    Member m[MEMBERS_MOST];
    if (members_of(kept, n, m) < 0) return OP1F_BAD_NAME;
    u32 lent = place == (u32)OP1F_PLACE_NONE ? 0u : place + 1u;
    Lend* d = lend_of((u32)slot, which);
    u32 r = rack_current[slot][which];
    if (d == 0 && lent == 0 && n == 0) {                                            // nothing lent, nothing kept
        if (r != 0 && r <= rack_count) rack_lent[r - 1u] = 0;
        return 0;
    }
    if (d == 0 && (d = lend_free()) == 0) return OP1F_TOO_LARGE;
    int same = d->host == (u32)slot + 1u && d->length == n;
    for (u32 i = 0; same && i < n; ++i) same = d->text[i] == kept[i];
    d->host = (u16)(slot + 1);
    d->which = (u8)which;
    d->place = (u8)lent;
    d->effect = 0;                                                                  // for the next ready at `which`
    if (!same) {
        for (u32 i = 0; i < n; ++i) d->text[i] = kept[i];
        d->length = (u8)n;
        d->told = 1;
        d->wrote = 0;
    }
    if (r != 0 && r <= rack_count) rack_lent[r - 1u] = (u8)lent;
    return 0;
}

extern "C" int op1f_rack_kept(const Op1fCard* card, unsigned which, char* out, unsigned room) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    if (which >= (u32)RACK_PLACES) return OP1F_BAD_NAME;
    u32 r = rack_current[slot][which];
    if (r == 0 || r > rack_count || !effects[racks[r - 1u].effect].hosted) return OP1F_NOT_HERE;
    Lend* d = lend_of((u32)slot, which);
    if (d == 0 || !d->wrote || d->effect != racks[r - 1u].effect + 1u) return 0;
    u32 n = d->length;
    if (out == 0 || room <= n) return OP1F_TOO_LARGE;
    for (u32 i = 0; i < n; ++i) out[i] = d->text[i];
    out[n] = 0;
    d->wrote = 0;
    return (int)n;
}

extern "C" int op1f_rack_page(const Op1fCard* card, unsigned ask, unsigned which, int* words) {
    int slot = slot_of(card);
    if (slot < 0) return OP1F_BAD_NAME;
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    const Effect* l = layer_of(slot, ask);
    if (l == 0 || which >= (u32)RACK_PLACES || words == 0) return OP1F_BAD_NAME;
    u32 r = rack_current[slot][which], place = shown_place(RD32(CURRENT_MODE));
    if (r == 0 || r > rack_count || racks[r - 1u].in.status != (u32)READY || place >= (u32)PLACES || riding(place) != l) {
        if (page.host == (u32)slot + 1u && page.which == which) page.host = 0;     // its page ends
        return OP1F_NOT_HERE;
    }
    if (page.host != (u32)slot + 1u || page.ask != ask || page.which != which || page.record != r || page.place != place) {
        u32 mask;
        MASK_OFF(mask);
        page.host = (u32)slot + 1u;
        page.record = r;
        for (u32 i = 0; i < 4u; ++i) page.turns[i] = 0;
        MASK_ON(mask);
        page.ask = ask;
        page.which = which;
        page.place = place;
        page.changed = 0;
        for (u32 i = 0; i < 4u; ++i) page.words[i] = (u32)words[i];
        page.asked = unit.frames;
        return page.up ? (int)OP1F_HOSTED_PAGE_UP : 0;
    }
    page_move();
    u32 changed = page.changed;
    for (u32 i = 0; i < 4u; ++i) {
        if (changed & (1u << i)) words[i] = (int)page.words[i];
        else page.words[i] = (u32)words[i];
    }
    page.changed = 0;
    page.asked = unit.frames;
    return (int)(changed | (page.up ? (u32)OP1F_HOSTED_PAGE_UP : 0u));
}

extern "C" int op1f_effects_hosted(unsigned place, Op1fStanding* out) {
    if (out == 0 || place >= (u32)PLACES || page.place != place || !page_live()) return 0;
    u32 i = page.record - 1u;
    const Rack* k = &racks[i];
    const Effect* e = &effects[k->effect];
    out->slot = e->slot;
    out->ask = e->ask;
    out->where = run_place(i);
    out->how = (rack_on[i] ? 0u : (u32)OP1F_SHOWN_OFF) | (k->budget.off ? (u32)OP1F_STAND_OVER : 0u);
    return 1;
}

extern "C" void op1f_effects_hosted_up(unsigned up) {
    u32 mask;
    MASK_OFF(mask);
    page.up = up ? 1u : 0u;
    for (u32 i = 0; i < 4u; ++i) page.turns[i] = 0;
    MASK_ON(mask);
}

extern "C" int op1f_effects_hosted_turn(unsigned knob, int turn) {
    u32 record = page.record;
    if (!page.up || knob >= 4u || record == 0 || record > rack_count) return 0;
    const Effect* e = &effects[racks[record - 1u].effect];
    if (knob >= e->count || e->fixed[knob]) return 0;
    u32 mask;
    MASK_OFF(mask);
    page.turns[knob] += turn;
    MASK_ON(mask);
    return 1;
}

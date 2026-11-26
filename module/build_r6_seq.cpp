// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R6, the base pack 29: the sequencers of the packs (level 5), and the catalog's read and write for the effects
// of level 8 and the engines of level 9. A sibling of base 28's sequencer unit, which stays as it was: its
// stubs are still module/build_r3_seq.s, and everything of it is here as it was, and the two places of the catalog
// serve the engines unit (base 29's engines unit) too: at the read, after the effects' rows,
// op1f_engines_catalog_read(document) (its first call registers the packs' engines, every call adds their rows to
// `synth_types`); at the write, the rows the engines unit says are its own (op1f_engines_row_is) left out of
// `synth_types` in the same copy as the other lists' rows. With no engine registered, the write is base 28's. The
// engines unit's names are weak: away, this unit is base 28's in all it does. What base 28's unit said follows and holds.
//
// Step R5, the base pack 28: the sequencers of the packs (level 5), and the catalog's read and write for the effects
// of level 8. A sibling of base 20's sequencer unit, which stays as it was for the bases before; its stubs stay
// module/build_r3_seq.s. Everything of base 20's is here as it was, the sequencers' work word for word, and the two
// places of the catalog serve the effects unit (base 28's effects unit) too:
//
//   the catalog's read   after the firmware's reader and the sequencers' rows, op1f_effects_catalog_read(document), at
//                        every read and also when no sequencer is registered: its first call registers the packs'
//                        effects, every call adds their rows to `fx_types`. The sequencers' part is what it was: the
//                        early returns of base 20 became a part that is passed by when no sequencer is registered
//   the catalog's write  the rows the effects unit says are its own (op1f_effects_row_is) left out of `fx_types` in the
//                        same copy in which the sequencers' rows are left out of `seq_types`, one copy for both, written
//                        in the firmware's document's place; when neither a sequencer nor an effect is registered, or
//                        the document holds no row of either, the firmware's write as it came, with the firmware's
//                        own document. With sequencers alone, the calls made are base 20's, in its order
//
// The effects unit's three names are weak: away, the unit is base 20's in all it does. The line of a write made
// without the packs' rows counts the rows of both lists. Compiled for the PC with OP1F_HOST defined over
// the PC's stand-ins (base 20's, and the effects unit's names as pointers the harness sets), with base 20's tests and
// with the effects unit itself (for tests this tree does not hold).
//
// What follows is base 20's description, which holds.
//
// Step R3, the base pack 20: the sequencers of the packs (level 5). A pack adds a sequencer to the unit's list through
// an ask of its card (module/build_r3_card.h, Op1fSequencer), and changes no place of the firmware itself: the base
// stands at the places a sequencer needs, makes a player and a controller of its own for every one registered, and
// calls the pack.
//
// The places, each a change of the base's table (the entries live in module/build_r3_seq.s):
//   the switch to a type        a stub: a type of the firmware's goes on to the firmware; one of ours comes to
//                               op1f_seq_switch. The stub counts every switch, so that a pick of any sequencer is seen
//   the name to a type          a stub: op1f_seq_type_of is asked once a sequencer is registered; -1 lets the firmware
//                               answer
//   the type to a name          a stub: one of ours is answered by op1f_seq_name_of
//   the two controller getters  two stubs: while the instrument's type is one of ours, op1f_seq_controller
//   the catalog's read          the call is op1f_seq_catalog_read: the firmware's reader, then the registration
//   the catalog's write         the call is op1f_seq_catalog_write: the packs' rows never reach the disk
//   the "sequencer off" notice  the word of its matcher is op1f_seq_overlay_is
//
// What the firmware needs of these, where the code meets it:
//   the instrument word: 0 the synth, 1 the drum; a sequencer's words of the drum are read for any other
//   the byte the firmware sets while it changes the instrument (its own ticks wait while it is set)
//   the type words (synth, then drum) and the player words (drum, then synth), behind the firmware's getters
//   a player is an object whose first word is a table of eleven functions; the firmware calls nothing else of it
//   a controller of a sequencer is the same, reached only through the two getters
//   the catalog is a document of JSON values of twelve bytes; each function is handed only the kinds it takes (the
//   kind byte at +4: 4 a string, 6 an array, 7 an object), and a value made here is destroyed here
//   a name handed to the name lookup is the firmware's string object: its length at +0x14, its room at +0x18, its
//   text at +4 when the room is under 16, else behind the pointer at +4
//   the note functions take (note, velocity, 0, 0, 0, 0, no MIDI) and (note, 64, 0, 0, 0, no MIDI); a note above 127
//   is never handed on
//   the bend takes a note and a pitch word: the note's own pitch from the firmware's table of pitches, moved here by
//   cents with the firmware's table of ratios (index 1,200 is the ratio 1)
//
// The threads. The registration runs on the thread of the start, before the first frame; the switch and the name
// lookup on the key thread or the start's; a tick and an octave on the sequencer thread; a key on the key thread, the
// audio thread or a MIDI handler's; the catalog's write under the database's lock (nothing but the JSON functions and
// the firmware's writer is called there). None of these says a line: they record, and op1f_seq_frame says it on the
// display thread. Nothing is divided and nothing multiplied in 64 bits (the module links no runtime). Compiled for the
// PC as it is, with OP1F_HOST defined (for tests this tree does not hold).

#include "build_r6_base.h"                             // R5: level 5's names (build_r3_base.h) and the effects unit's; R6: and the engines unit's
#include "build_q_quiet.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*NoteOnFn)(int note, int velocity, int third, int fourth, int octave, int target, int no_midi);
typedef void (*NoteOffFn)(int note, int velocity, int third, int fourth, int target, int no_midi);
typedef void (*BendFn)(int note, int pitch);
struct NoteRecord { u32 note, velocity, third, fourth; };
typedef NoteRecord (*NoteRecordFn)(int key, int velocity);      // the record comes back through P0
typedef int (*AnswerFn)();
typedef void (*DoFn)();
typedef u32* (*WordFn)();
typedef void* (*PlayerFn)();
typedef int (*MatchFn)(void* overlay, u32 screen);
typedef int (*JsonReadFn)(const char* path, void* document);
typedef int (*JsonWriteFn)(void* path, void* document);
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
typedef int (*FileSizeFn)(const char* path);
typedef int (*FileOpenFn)(const char* path, const char* mode);
typedef int (*FileReadFn)(void* buffer, int size, int count, int descriptor);
typedef int (*FileCloseFn)(int descriptor);

#ifdef OP1F_HOST
#include "mock_seq_r5.h"                                // the PC's stand-ins: the firmware's memory and calls, the mask
#define SEQ_APART
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_NOTE_ON ((NoteOnFn)0x8f008388u)
#define STOCK_NOTE_OFF ((NoteOffFn)0x8f008518u)
#define STOCK_NOTE_RECORD ((NoteRecordFn)0x8f007d68u)
#define STOCK_BEND ((BendFn)0x8f008ba8u)
#define STOCK_RESTART_ALLOWED ((AnswerFn)0x8fc9e818u)
#define STOCK_RESTART ((DoFn)0x8fc9e868u)
#define STOCK_PLAYER_WORD ((WordFn)0x8fc97844u)
#define STOCK_CURRENT_PLAYER ((PlayerFn)0x8fc978fcu)
#define STOCK_OVERLAY_MATCH ((MatchFn)0x8fc681a0u)
#define STOCK_JSON_READ ((JsonReadFn)0x8c0534d0u)
#define STOCK_JSON_WRITE ((JsonWriteFn)0x8c053168u)
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
#define STOCK_JSON_TRUTH ((JsonNumberFn)0x8c083aecu)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_OPEN ((FileOpenFn)0x8fc8a9d8u)
#define STOCK_FILE_READ ((FileReadFn)0x8fc8a9a8u)
#define STOCK_FILE_CLOSE ((FileCloseFn)0x8fc8a994u)
#define STOCK_READ_MODE ((const char*)0x848daec0u)         // the firmware's own text "rb", which its open compares
#define W32(a) (*(volatile u32*)(a))
#define W8(a) (*(volatile u8*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define SEQ_APART __attribute__((noinline))
#endif

// The firmware's words (an enum of their own: they do not fit a signed number).
enum : u32 {
    INSTRUMENT = 0x84d21088u,           // 0 the synth, 1 the drum
    TYPE_WORDS = 0x84d1ccccu,           // the synth's sequencer type, and the drum's behind it
    CHANGING = 0x84d1ccd4u,             // a byte: not 0 while the firmware changes the instrument
    ON_SYNTH = 0x84d1ccc9u,             // not 0 while the synth's sequencer is on (the byte the tick reads)
    ON_DRUM = 0x84d1ccc8u,              // the drum's
    PENDING_OCTAVE = 0x8490ba40u,       // the octave steps a key made while a sequencer is on, until the next tick
    TEMPO = 0x8490c8fcu,                // the tempo, a float of beats a minute
    PERIOD = 0x84d20e80u,               // samples a pulse at 44,100 a second, times 2^18
    SYNC = 0x8490c8f4u,                 // which clock leads: 2 and 4 one from outside over MIDI
    TRANSPORT = 0x11905a04u,            // 1 or 2 while the tape plays
    CATALOG = 0x8490cd14u,              // the firmware's seven sequencers: a name and a type each, eight bytes
    PITCHES = 0x848c5270u,              // a note's own pitch word, 128 words
    RATIOS = 0x0802be58u,               // 2,400 words, filled at the start: index 1,200 is the ratio 1
    MODE = 0x8400f978u,                 // the current controller: 7 while the sequencer's screen is up
    SCREEN = 0x848d2540u,               // the screen shown
    POPUP = 0x84c9b3e4u                 // the power popup's byte: 1 while the countdown is on the screen
};

enum {
    TYPE_FIRST = 7, ID_FIRST = 8, STOCK_TYPES = 7, SEQ_MOST = OP1F_SEQUENCERS_MOST, NAME_ROOM = OP1F_KIND_NAME,
    SEQ_LEAST_BYTES = 48, SEQ_FLAGS = OP1F_SEQ_SYNTH_ONLY | OP1F_SEQ_ALL_MIDI, SEQ_MODE = 7, SEQ_SCREEN = 77,
    OVERLAY_OFF = 0x12, NOTES = 128, LEDGER_WORDS = NOTES / 32, KEYS_AT = 53, OFF_VELOCITY = 64,
    KIND_STRING = 4, KIND_ARRAY = 6, KIND_OBJECT = 7, KIND_AT = 4,
    STRING_TEXT = 4, STRING_LENGTH = 0x14, STRING_ROOM = 0x18, STRING_INLINE = 16,
    KEPT_MOST = 64, ASIDES_KEPT = 16, PERIOD_MS = 16384, PULSE_WRAP = 48,
    CENTS_MOST = 4800, OCTAVE_CENTS = 1200, RATIO_ONE = 1200,
    NO_PULSE = -1, KEPT_STILL = 64, KEPT_LINES_SAID = 64
};

static const char LOG_FILE[] = "../src/main/op1f_seq.cpp";
static const char KEPT_PATH[] = "user/pf/.seq";
static const char KEY_TYPE[] = "type";
static const char KEY_ID[] = "id";
static const char KEY_SYNTH_ONLY[] = "synth_only";
static const char KEY_LIST[] = "seq_types";
static const char KEY_FX_LIST[] = "fx_types";             // R5: the effects' list, whose rows the effects unit adds
static const char KEY_SYNTH_LIST[] = "synth_types";       // R6: the engines' list, whose rows the engines unit adds
static const char FALLBACK[] = "endless";
static const char* const HOSTS[2] = { "synth", "drum" };

extern "C" Op1fCards op1f_cards OP1F_NAMED("op1f_cards");   // the cards unit's list, filled by the loader

// R5: the effects unit's three names at the catalog (module/build_r5_base.h), weak: away, nothing of effects is done.
extern "C" void __attribute__((weak)) op1f_effects_catalog_read(void* document) OP1F_NAMED("op1f_effects_catalog_read");
extern "C" int __attribute__((weak)) op1f_effects_row_is(const void* row) OP1F_NAMED("op1f_effects_row_is");
extern "C" volatile unsigned op1f_effects_registered OP1F_NAMED("op1f_effects_registered") __attribute__((weak));

// R11: the LFOs unit's two names at the catalog (module/build_r11_lfos.cpp), weak: away, nothing of LFOs is done.
// op1f_lfos_catalog_read(document): given the same document the effects unit's is (the firmware's catalog object, as
// read, `void*`); adds the packs' rows to `lfo_types`. op1f_lfos_row_is(row): given one element of a list (`const void*`);
// answers non-zero for a row the LFOs unit added.
#if defined(OP1F_HOST)
#ifdef OP1F_R11_HOST_LFOS
extern "C" void (*mock_lfos_catalog_read)(void* document);
#define op1f_lfos_catalog_read (*mock_lfos_catalog_read)
extern "C" int (*mock_lfos_row_is)(const void* row);
#define op1f_lfos_row_is (*mock_lfos_row_is)
#else
static void (*const r11_none_lfos_catalog_read)(void* document) = 0;
#define op1f_lfos_catalog_read (*r11_none_lfos_catalog_read)
static int (*const r11_none_lfos_row_is)(const void* row) = 0;
#define op1f_lfos_row_is (*r11_none_lfos_row_is)
#endif
#else
extern "C" void op1f_lfos_catalog_read(void* document) OP1F_NAMED("op1f_lfos_catalog_read") __attribute__((weak));
extern "C" int op1f_lfos_row_is(const void* row) OP1F_NAMED("op1f_lfos_row_is") __attribute__((weak));
#endif
static const char KEY_LFO_LIST[] = "lfo_types";            // R11: the LFOs' list, whose rows the LFOs unit adds

// R18 (base 40): the drum unit's two names at the catalog (module/build_r18_drum.cpp), weak: away, nothing of a drum
// engine is done. op1f_drum_catalog_read(document) adds the pack's row to `drum_types`; op1f_drum_row_is(row) answers
// non-zero for that row.
#if defined(OP1F_HOST)
static void (*const r18_none_drum_catalog_read)(void* document) = 0;
#define op1f_drum_catalog_read (*r18_none_drum_catalog_read)
static int (*const r18_none_drum_row_is)(const void* row) = 0;
#define op1f_drum_row_is (*r18_none_drum_row_is)
#else
extern "C" void op1f_drum_catalog_read(void* document) OP1F_NAMED("op1f_drum_catalog_read") __attribute__((weak));
extern "C" int op1f_drum_row_is(const void* row) OP1F_NAMED("op1f_drum_row_is") __attribute__((weak));
#endif
static const char KEY_DRUM_LIST[] = "drum_types";          // R18 (base 40): the drum engines' list, whose row the drum unit adds

// R6: the engines unit's three names at the catalog (module/build_r6_base.h), weak: away, nothing of engines is done.
#if defined(OP1F_HOST)
#ifdef OP1F_R6_HOST_ENGINES
extern "C" void (*mock_engines_catalog_read)(void* document);
#define op1f_engines_catalog_read (*mock_engines_catalog_read)
extern "C" int (*mock_engines_row_is)(const void* row);
#define op1f_engines_row_is (*mock_engines_row_is)
#else
static void (*const r6_none_engines_catalog_read)(void* document) = 0;
#define op1f_engines_catalog_read (*r6_none_engines_catalog_read)
static int (*const r6_none_engines_row_is)(const void* row) = 0;
#define op1f_engines_row_is (*r6_none_engines_row_is)
#endif
#else
extern "C" void op1f_engines_catalog_read(void* document) OP1F_NAMED("op1f_engines_catalog_read") __attribute__((weak));
extern "C" int op1f_engines_row_is(const void* row) OP1F_NAMED("op1f_engines_row_is") __attribute__((weak));
#endif
#if defined(OP1F_HOST) && defined(OP1F_R6_HOST_ENGINES)
extern "C" volatile unsigned* mock_engines_registered;
#define ENGINES_REGISTERED() (mock_engines_registered != 0 ? *mock_engines_registered : 0u)
#elif defined(OP1F_HOST)
#define ENGINES_REGISTERED() 0u
#else
extern "C" volatile unsigned op1f_engines_registered OP1F_NAMED("op1f_engines_registered") __attribute__((weak));
#define ENGINES_REGISTERED() (&op1f_engines_registered != 0 && &op1f_engines_row_is != 0 ? (u32)op1f_engines_registered : 0u)
#endif

// The count of sequencers registered at this start (read by the stubs), and the count of switches the firmware was
// asked for (written by the stub at the switch, read by the frame).
extern "C" volatile u32 op1f_seq_registered OP1F_NAMED("op1f_seq_registered");
volatile u32 op1f_seq_registered;
extern "C" volatile u32 op1f_seq_switches OP1F_NAMED("op1f_seq_switches");
volatile u32 op1f_seq_switches;

// ---------------------------------------------------------------- the registration

struct Registered {
    u32 slot, ask;                                      // the card's slot in op1f_cards, the ask's number in its list
    const Op1fSequencer* table;
    char name[NAME_ROOM];                               // the name, its end within the sixteen bytes
};
static Registered reg[SEQ_MOST];
static u32 checked;                                     // the asks were looked through (once a start)

enum { WHY_STOCK = 1, WHY_FIRST = 2, WHY_SEVENTEENTH = 3, WHY_NAME = 4, WHY_ORDER = 5 };
struct Aside { u32 slot, why, a, b, other; char name[NAME_ROOM]; };
static Aside asides[ASIDES_KEPT];
static u32 aside_count, aside_more;

enum { LIST_UNREAD = 1, LIST_NOT_OBJECT = 2, LIST_NOT_ARRAY = 3 };
static u32 list_state;                                  // 0 when the rows were added, else why not
static u32 asks_seen;                                   // asks of a sequencer, kept or left aside

// The owner's choice kept: read at the catalog's read, the text of the file as last read or written.
enum { KEPT_NONE = 0, KEPT_READ = 1, KEPT_LONG = 2, KEPT_UNREAD = 3, KEPT_MALFORMED = 4, KEPT_NOT_LOOKED = 5 };
static u32 kept_state = KEPT_NOT_LOOKED;
static char kept_name[2][NAME_ROOM];
static char kept_text[KEPT_MOST + 1];
static u32 kept_known;                                  // kept_text holds what the file holds
static volatile u32 put_back[2];                        // the registration + 1 put back for a host at the start

// The catalog's write (the database's job): what it did, for the frame's lines.
static volatile u32 writes_without, rows_left, writes_left_out;

// The "sequencer off" notice: its byte, as its matcher read it at its last call (it is asked every frame).
static volatile u32 overlay_covered;

// ---------------------------------------------------------------- the players and the controller

struct Player { const void* const* table; u32 host, type, state; };
struct Controller { const void* const* table; u32 id; };
static Player players[2];
static Controller controller;

// Per host: the notes begun (a bit a note), those begun without MIDI, the stops, the ticks' count and last pulse.
static volatile u32 ledger[2][LEDGER_WORDS], quiet_notes[2][LEDGER_WORDS], stops[2];
static volatile u32 tick_count[2];
static volatile int last_pulse[2] = { NO_PULSE, NO_PULSE };
static volatile u32 told_on[2];                         // the pack was told ON for the host, and no STOP since

// A period's counts, per registration and per card's slot (refused).
static volatile u32 counts[SEQ_MOST][6];
static volatile u32 refused[OP1F_CARDS_MOST];
enum { C_TICKS = 0, C_KEYS = 1, C_TAKEN = 2, C_ONS = 3, C_OFFS = 4, C_ENDED = 5 };

// ---------------------------------------------------------------- small helpers

static int is_name_char(u32 c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
}

static int same_text(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// The name of a sequencer as an ask holds it: 1 to 15 characters the list can show, then zeros to the sixteenth.
static int name_in_order(const char* name) {
    u32 n = 0;
    while (n < (u32)NAME_ROOM && name[n]) {
        if (!is_name_char((u8)name[n])) return 0;
        ++n;
    }
    if (n == 0 || n >= (u32)NAME_ROOM) return 0;
    for (u32 i = n; i < (u32)NAME_ROOM; ++i)
        if (name[i]) return 0;
    return 1;
}

// A name copied for a line: fifteen characters at most, each one a line can show.
static void copy_said(char* to, const char* from) {
    u32 n = 0;
    for (; n + 1 < (u32)NAME_ROOM && from[n]; ++n) to[n] = (from[n] >= 0x20 && from[n] < 0x7f) ? from[n] : '?';
    to[n] = 0;
}

static const char* stock_name(u32 i) {
    return (const char*)W32(CATALOG + 8u * i);
}

static u32 stock_type(u32 i) {
    return W32(CATALOG + 8u * i + 4u);
}

static int registered_of(u32 type) {
    u32 n = type - (u32)TYPE_FIRST;
    return type >= (u32)TYPE_FIRST && n < op1f_seq_registered ? (int)n : -1;
}

// A type's name, the firmware's or ours: 0 for a type nobody has.
static const char* name_of(u32 type) {
    int n = registered_of(type);
    if (n >= 0) return reg[n].name;
    for (u32 i = 0; i < (u32)STOCK_TYPES; ++i)
        if (stock_type(i) == type) return stock_name(i);
    return 0;
}

static u32 host_now() {
    return W32(INSTRUMENT) == 0 ? 0u : 1u;              // as the firmware's own getters take it
}

static u32 type_of_host(u32 host) {
    return W32(TYPE_WORDS + 4u * host);
}

static int slot_of(const Op1fCard* card) {
    u32 count = op1f_cards.count < (u32)OP1F_CARDS_MOST ? op1f_cards.count : (u32)OP1F_CARDS_MOST;
    for (u32 i = 0; i < count; ++i)
        if (card && op1f_cards.slots[i].card == card) return (int)i;
    return -1;
}

// ---------------------------------------------------------------- the asks looked through (the start's thread)

static void aside(u32 slot, const char* name, u32 why, u32 a, u32 b, u32 other) {
    if (aside_count >= (u32)ASIDES_KEPT) { ++aside_more; return; }
    Aside* s = &asides[aside_count];
    s->slot = slot; s->why = why; s->a = a; s->b = b; s->other = other;
    copy_said(s->name, name);
    ++aside_count;
}

static int in_pack(const Op1fCardSlot* s, u32 at, u32 bytes) {
    return at >= s->first && at < s->end && bytes <= s->end - at;
}

static int function_in_order(const Op1fCardSlot* s, const void* f) {
    u32 at = (u32)f;
    return f == 0 || (in_pack(s, at, 2) && (at & 1u) == 0);
}

// One ask of a sequencer checked; its table's name copied for the lines whatever comes of it.
static void look_at(u32 slot, u32 number, const Op1fAsk* a) {
    const Op1fCardSlot* s = &op1f_cards.slots[slot];
    ++asks_seen;
    u32 at = (u32)a->what;
    if (a->bytes < (u32)SEQ_LEAST_BYTES || a->bytes > (u32)OP1F_ASK_BYTES_MOST || (at & 3u) != 0 || !in_pack(s, at, a->bytes)) {
        aside(slot, "", WHY_ORDER, a->bytes, at, 0);
        return;
    }
    const Op1fSequencer* q = (const Op1fSequencer*)a->what;
    if (q->head.bytes != a->bytes) { aside(slot, q->head.name, WHY_ORDER, a->bytes, q->head.bytes, 0); return; }
    if (!name_in_order(q->head.name)) { aside(slot, q->head.name, WHY_NAME, 0, 0, 0); return; }
    if ((q->head.flags & ~(u32)SEQ_FLAGS) != 0) { aside(slot, q->head.name, WHY_ORDER, q->head.flags, (u32)SEQ_FLAGS, 0); return; }
    if (q->tick == 0 && q->key == 0) { aside(slot, q->head.name, WHY_ORDER, 0, 0, 0); return; }
    const void* functions[6] = { (const void*)q->head.shown, (const void*)q->tick, (const void*)q->key, (const void*)q->run, (const void*)q->octave,
                                 (const void*)q->playing };
    for (u32 i = 0; i < 6; ++i)
        if (!function_in_order(s, functions[i])) { aside(slot, q->head.name, WHY_ORDER, 24u + 4u * i, (u32)functions[i], 0); return; }
    for (u32 i = 0; i < (u32)STOCK_TYPES; ++i)
        if (same_text(stock_name(i), q->head.name)) { aside(slot, q->head.name, WHY_STOCK, 0, 0, 0); return; }
    u32 count = op1f_seq_registered;
    for (u32 i = 0; i < count; ++i)
        if (same_text(reg[i].name, q->head.name)) { aside(slot, q->head.name, WHY_FIRST, 0, 0, reg[i].slot); return; }
    if (count >= (u32)SEQ_MOST) { aside(slot, q->head.name, WHY_SEVENTEENTH, 0, 0, 0); return; }
    Registered* r = &reg[count];
    r->slot = slot;
    r->ask = number;
    r->table = q;
    for (u32 i = 0; i < (u32)NAME_ROOM; ++i) r->name[i] = q->head.name[i];
    op1f_seq_registered = count + 1;
}

static void look_through() {
    u32 count = op1f_cards.count < (u32)OP1F_CARDS_MOST ? op1f_cards.count : (u32)OP1F_CARDS_MOST;
    for (u32 slot = 0; slot < count; ++slot) {
        if (op1f_cards_verdict(slot) != 0) continue;
        const Op1fCard* card = op1f_cards.slots[slot].card;
        if (card->level < 4 || card->bytes < (u32)OP1F_CARD_LEVEL_4_BYTES || card->asks == 0) continue;
        u32 asks = card->ask_count < (u32)OP1F_ASKS_MOST ? card->ask_count : (u32)OP1F_ASKS_MOST;
        for (u32 i = 0; i < asks; ++i)
            if (card->asks[i].kind == (u32)OP1F_ASK_SEQUENCER) look_at(slot, i, &card->asks[i]);
    }
}

// ---------------------------------------------------------------- the owner's choice kept (read on the start's thread)

static int take_line(const char* text, u32 bytes, u32* at, const char* word, char* name) {
    u32 i = *at, n = 0;
    while (*word) {
        if (i >= bytes || text[i] != *word) return 0;
        ++i; ++word;
    }
    while (i < bytes && text[i] != '\n') {
        if (n + 1 >= (u32)NAME_ROOM || !is_name_char((u8)text[i])) return 0;
        name[n++] = text[i++];
    }
    if (n == 0 || i >= bytes) return 0;
    name[n] = 0;
    *at = i + 1;
    return 1;
}

static void read_kept() {
    int size = STOCK_FILE_SIZE(KEPT_PATH);
    if (size < 0) { kept_state = KEPT_NONE; return; }
    if (size > (int)KEPT_MOST) { kept_state = KEPT_LONG; return; }
    int descriptor = STOCK_FILE_OPEN(KEPT_PATH, STOCK_READ_MODE);
    if (descriptor < 0) { kept_state = KEPT_UNREAD; return; }
    int got = size > 0 ? STOCK_FILE_READ(kept_text, 1, size, descriptor) : 0;
    STOCK_FILE_CLOSE(descriptor);
    if (got != size) { kept_state = KEPT_UNREAD; return; }
    kept_text[size] = 0;
    kept_known = 1;
    u32 at = 0;
    if (!take_line(kept_text, (u32)size, &at, "synth ", kept_name[0]) || !take_line(kept_text, (u32)size, &at, "drum ", kept_name[1]) || at != (u32)size) {
        kept_name[0][0] = kept_name[1][0] = 0;
        kept_state = KEPT_MALFORMED;
        return;
    }
    kept_state = KEPT_READ;
}

// ---------------------------------------------------------------- the catalog (the firmware's jsoncpp document)

static u32 kind(const void* value) {
    return ((const volatile u8*)value)[KIND_AT];
}

// 1 when an element of `seq_types` is a row whose `type` is the text `name` (or, with no name, one of ours).
static int row_named(const void* row, const char* name) {
    if (kind(row) != (u32)KIND_OBJECT) return 0;
    const void* type = STOCK_JSON_MEMBER(row, KEY_TYPE);
    if (kind(type) != (u32)KIND_STRING) return 0;
    const char* text = *(const char* const*)type;
    if (text == 0) return 0;
    if (name) return same_text(text, name);
    u32 count = op1f_seq_registered;
    for (u32 i = 0; i < count; ++i)
        if (same_text(text, reg[i].name)) return 1;
    return 0;
}

static int has_row(const void* list, const char* name) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i)
        if (row_named(STOCK_JSON_ELEMENT(list, i), name)) return 1;
    return 0;
}

// A value made by `make` with `n` assigned to the member `key` of `row`, and destroyed.
static void set_member(void* row, const char* key, JsonNumberFn make, const char* text, int n) {
    u32 value[3];
    void* place = STOCK_JSON_PLACE(row, key);
    if (text) STOCK_JSON_TEXT(value, text);
    else make(value, n);
    STOCK_JSON_ASSIGN(place, value);
    STOCK_JSON_DESTROY(value);
}

static void add_row(void* document, u32 n) {
    u32 row[3];
    STOCK_JSON_MAKE(row, KIND_OBJECT);
    set_member(row, KEY_TYPE, 0, reg[n].name, 0);
    set_member(row, KEY_ID, STOCK_JSON_NUMBER, 0, (int)(ID_FIRST + n));
    set_member(row, KEY_SYNTH_ONLY, STOCK_JSON_TRUTH, 0, (reg[n].table->head.flags & (u32)OP1F_SEQ_SYNTH_ONLY) ? 1 : 0);
    STOCK_JSON_APPEND(STOCK_JSON_PLACE(document, KEY_LIST), row);
    STOCK_JSON_DESTROY(row);
}

// R5: the sequencers' part of the catalog's read, base 20's as it was: the list held to its form, then the rows.
static SEQ_APART void sequencers_read(int answer, void* document) {
    if (!answer) list_state = LIST_UNREAD;
    else if (kind(document) != (u32)KIND_OBJECT) list_state = LIST_NOT_OBJECT;
    else if (kind(STOCK_JSON_MEMBER(document, KEY_LIST)) != (u32)KIND_ARRAY) list_state = LIST_NOT_ARRAY;
    else list_state = 0;
    if (list_state) {
        op1f_seq_registered = 0;                        // no row in the list: no type of ours either
        return;
    }
    u32 count = op1f_seq_registered;
    for (u32 n = 0; n < count; ++n)
        if (!has_row(STOCK_JSON_MEMBER(document, KEY_LIST), reg[n].name)) add_row(document, n);
}

// The firmware's read of its catalog, then the packs' sequencers added to it. On the thread of the start.
// R5: then the effects unit's rows, at every read, a sequencer registered or not.
extern "C" int op1f_seq_catalog_read(const char* path, void* document) OP1F_NAMED("op1f_seq_catalog_read");
extern "C" int op1f_seq_catalog_read(const char* path, void* document) {
    int answer = STOCK_JSON_READ(path, document);
    if (!checked) {
        checked = 1;
        look_through();
        if (op1f_seq_registered) read_kept();
    }
    if (op1f_seq_registered != 0) sequencers_read(answer, document);
    if (&op1f_effects_catalog_read != 0) op1f_effects_catalog_read(document);   // R5: after the sequencers' rows
    if (&op1f_engines_catalog_read != 0) op1f_engines_catalog_read(document);   // R6: after the effects' rows
    if (&op1f_lfos_catalog_read != 0) op1f_lfos_catalog_read(document);         // R11: after the effects' rows
    if (&op1f_drum_catalog_read != 0) op1f_drum_catalog_read(document);         // R18 (base 40): after the LFOs' rows
    return answer;
}

// R5: how many effects the effects unit registered: 0 where it is away.
static u32 effects_registered() {
    return &op1f_effects_registered != 0 && &op1f_effects_row_is != 0 ? (u32)op1f_effects_registered : 0u;
}

// R5: 1 when `fx_types` holds a row the effects unit added.
static SEQ_APART int has_effect_row(const void* list) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i)
        if (op1f_effects_row_is(STOCK_JSON_ELEMENT(list, i))) return 1;
    return 0;
}

// R6: 1 when `synth_types` holds a row the engines unit added.
static SEQ_APART int has_engine_row(const void* list) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i)
        if (op1f_engines_row_is(STOCK_JSON_ELEMENT(list, i))) return 1;
    return 0;
}

// R18 (base 40): 1 when `drum_types` holds the row the drum unit added.
static SEQ_APART int has_drum_row(const void* list) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i)
        if (op1f_drum_row_is(STOCK_JSON_ELEMENT(list, i))) return 1;
    return 0;
}

// R11: 1 when `lfo_types` holds a row the LFOs unit added.
static SEQ_APART int has_lfo_row(const void* list) {
    u32 count = STOCK_JSON_SIZE(list);
    for (u32 i = 0; i < count; ++i)
        if (op1f_lfos_row_is(STOCK_JSON_ELEMENT(list, i))) return 1;
    return 0;
}

// R5: one list of the copy written without the packs' rows: the sequencers' (`effects` 0, the rows row_named knows)
// or the effects' (`effects` 1, the rows the effects unit knows). Answers how many rows were left out. Base 20's loop,
// its calls in its order.
// R6: or the engines' (`effects` 2, the rows the engines unit knows).
// R18 (base 40): or the drum engines' (`effects` 4, the row the drum unit knows).
static SEQ_APART u32 without_rows(void* copy, const char* key, u32 effects) {
    u32 keep[3];
    STOCK_JSON_MAKE(keep, KIND_ARRAY);
    const void* list = STOCK_JSON_MEMBER(copy, key);
    u32 count = kind(list) == (u32)KIND_ARRAY ? STOCK_JSON_SIZE(list) : 0u, left = 0;
    for (u32 i = 0; i < count; ++i) {
        const void* row = STOCK_JSON_ELEMENT(list, i);
        if (effects == 4u ? op1f_drum_row_is(row) != 0 : effects == 3u ? op1f_lfos_row_is(row) != 0 : effects == 2u ? op1f_engines_row_is(row) != 0 : effects ? op1f_effects_row_is(row) != 0 : row_named(row, 0) != 0) ++left;   // R6: the engines' too
        else STOCK_JSON_APPEND(keep, row);
    }
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(copy, key), keep);
    STOCK_JSON_DESTROY(keep);
    return left;
}

// The firmware's write of its catalog: the packs' rows never reach the disk. Under the database's lock: nothing is
// called here but the JSON functions and the firmware's writer, and `path` (the firmware's string object) is handed on.
// R5: the effects' rows too (op1f_effects_row_is calls only the JSON functions' readers), in the same copy.
extern "C" int op1f_seq_catalog_write(void* path, void* document) OP1F_NAMED("op1f_seq_catalog_write");
extern "C" int op1f_seq_catalog_write(void* path, void* document) {
    u32 effects = effects_registered();
    u32 engines = ENGINES_REGISTERED();                                             // R6
    u32 lfos = &op1f_lfos_row_is != 0 ? 1u : 0u;                                    // R11: the LFOs unit linked
    u32 drums = &op1f_drum_row_is != 0 ? 1u : 0u;                                   // R18 (base 40): the drum unit linked
    if ((op1f_seq_registered == 0 && effects == 0 && engines == 0 && lfos == 0 && drums == 0) || kind(document) != (u32)KIND_OBJECT) return STOCK_JSON_WRITE(path, document);
    u32 sequencers = 0, rows = 0, synth_rows = 0, lfo_rows = 0, drum_rows = 0;
    if (op1f_seq_registered != 0) {
        const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
        sequencers = kind(list) == (u32)KIND_ARRAY && has_row(list, 0) ? 1u : 0u;
    }
    if (effects != 0) {
        const void* list = STOCK_JSON_MEMBER(document, KEY_FX_LIST);
        rows = kind(list) == (u32)KIND_ARRAY && has_effect_row(list) ? 1u : 0u;
    }
    if (engines != 0) {                                                             // R6
        const void* list = STOCK_JSON_MEMBER(document, KEY_SYNTH_LIST);
        synth_rows = kind(list) == (u32)KIND_ARRAY && has_engine_row(list) ? 1u : 0u;
    }
    if (lfos != 0) {                                                                // R11
        const void* list = STOCK_JSON_MEMBER(document, KEY_LFO_LIST);
        lfo_rows = kind(list) == (u32)KIND_ARRAY && has_lfo_row(list) ? 1u : 0u;
    }
    if (drums != 0) {                                                               // R18 (base 40)
        const void* list = STOCK_JSON_MEMBER(document, KEY_DRUM_LIST);
        drum_rows = kind(list) == (u32)KIND_ARRAY && has_drum_row(list) ? 1u : 0u;
    }
    if (sequencers == 0 && rows == 0 && synth_rows == 0 && lfo_rows == 0 && drum_rows == 0) return STOCK_JSON_WRITE(path, document);
    u32 copy[3];
    STOCK_JSON_MAKE(copy, 0);
    STOCK_JSON_ASSIGN(copy, document);
    if (kind(copy) != (u32)KIND_OBJECT) {
        STOCK_JSON_DESTROY(copy);
        ++writes_left_out;
        return 0;
    }
    u32 left = 0;
    if (sequencers) left += without_rows(copy, KEY_LIST, 0);
    if (rows) left += without_rows(copy, KEY_FX_LIST, 1);
    if (synth_rows) left += without_rows(copy, KEY_SYNTH_LIST, 2);                 // R6
    if (lfo_rows) left += without_rows(copy, KEY_LFO_LIST, 3);                      // R11
    if (drum_rows) left += without_rows(copy, KEY_DRUM_LIST, 4);                    // R18 (base 40)
    int answer = STOCK_JSON_WRITE(path, copy);
    STOCK_JSON_DESTROY(copy);
    rows_left = left;
    ++writes_without;
    return answer;
}

// ---------------------------------------------------------------- the places of the stubs

static volatile u32 frames_seen;                        // the display thread's frames: 0 before the first

// Whether a registered sequencer serves a host: one for the synth only never serves the drum.
static int serves(u32 n, u32 host) {
    return host == 0 || (reg[n].table->head.flags & (u32)OP1F_SEQ_SYNTH_ONLY) == 0;
}

// The name lookup: the type of one of ours, or -1 for the firmware's own answer. And at the start (before the first
// frame) `endless` for a host whose kept choice is registered now: that one, put back.
extern "C" int op1f_seq_type_of(const void* name) OP1F_NAMED("op1f_seq_type_of");
extern "C" int op1f_seq_type_of(const void* name) {
    const u8* object = (const u8*)name;
    u32 length = *(const u32*)(object + STRING_LENGTH);
    u32 room = *(const u32*)(object + STRING_ROOM);
    const char* text = room >= (u32)STRING_INLINE ? *(const char* const*)(object + STRING_TEXT) : (const char*)(object + STRING_TEXT);
    if (length == 0 || length >= (u32)NAME_ROOM || text == 0) return -1;
    char asked[NAME_ROOM];
    for (u32 i = 0; i < length; ++i) asked[i] = text[i];
    asked[length] = 0;
    u32 count = op1f_seq_registered, host = host_now();
    for (u32 i = 0; i < count; ++i)
        if (same_text(asked, reg[i].name) && serves(i, host)) return (int)(TYPE_FIRST + i);
    if (frames_seen == 0 && kept_state == KEPT_READ && same_text(asked, FALLBACK)) {
        for (u32 i = 0; i < count; ++i)
            if (same_text(kept_name[host], reg[i].name) && serves(i, host)) {
                put_back[host] = i + 1;
                return (int)(TYPE_FIRST + i);
            }
    }
    return -1;
}

extern "C" const char* op1f_seq_name_of(u32 type) OP1F_NAMED("op1f_seq_name_of");
extern "C" const char* op1f_seq_name_of(u32 type) {
    int n = registered_of(type);
    return n >= 0 ? reg[n].name : 0;
}

// The matcher of the "sequencer off" notice: over a sequencer's page of a pack, the notice's own byte, as the
// firmware's matcher answers for its own seven screens.
extern "C" int op1f_seq_overlay_is(void* object, u32 screen) OP1F_NAMED("op1f_seq_overlay_is");
extern "C" int op1f_seq_overlay_is(void* object, u32 screen) {
    overlay_covered = ((const volatile u8*)object)[OVERLAY_OFF];
    if (screen != (u32)SEQ_SCREEN || op1f_cards_stand_in_shown != (unsigned)OP1F_ASK_SEQUENCER) return STOCK_OVERLAY_MATCH(object, screen);
    return ((const volatile u8*)object)[OVERLAY_OFF];
}

// ---------------------------------------------------------------- calls into a pack

static const Op1fSequencer* table_of(const Player* p, int* n) {
    *n = registered_of(p->type);
    return *n >= 0 ? reg[*n].table : 0;
}

static void call_run(int n, u32 host, u32 how) {
    const Op1fSequencer* q = reg[n].table;
    if (!q->run) return;
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_RUN, reg[n].slot, &kept);
    q->run(host, how);
    op1f_cards_call_out(OP1F_CALL_RUN, &kept);
}

static void seq_nothing(void*) {}

static int seq_key(const Player* p, int key, u32 what, u32 shift, u32 from_keys, u32 velocity) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    if (!q || !q->key) return 0;
    Op1fKey e;
    e.bytes = sizeof(Op1fKey);
    e.what = what;
    e.host = p->host;
    e.key = key;
    if (from_keys) {
        NoteRecord r = STOCK_NOTE_RECORD(key, (int)velocity);
        e.note = r.note;
    } else {
        e.note = (u32)(key + KEYS_AT);
    }
    if (e.note > (u32)NOTES - 1u) e.note = (u32)NOTES - 1u;
    if (what == (u32)OP1F_DOWN) e.velocity = velocity == 0 ? 1u : velocity > 127u ? 127u : velocity;
    else e.velocity = 0;
    e.shift = what == (u32)OP1F_DOWN && from_keys && shift ? 1u : 0u;
    e.from = from_keys ? (u32)OP1F_FROM_KEYS : (u32)OP1F_FROM_MIDI;
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_KEY, reg[n].slot, &kept);
    int taken = q->key(&e);
    op1f_cards_call_out(OP1F_CALL_KEY, &kept);
    ++counts[n][C_KEYS];
    if (taken) ++counts[n][C_TAKEN];
    return taken ? 1 : 0;
}

static int seq_key_down(Player* p, int key, u32 shift, u32 from_keys, u32 velocity) {
    return seq_key(p, key, OP1F_DOWN, shift, from_keys, velocity);
}

static int seq_key_up(Player* p, int key, u32 from_keys) {
    return seq_key(p, key, OP1F_UP, 0, from_keys, 0);
}

// The tempo in hundredths of a beat a minute, from the float's bits: (mantissa | 2^23) x 100, shifted by 150 - exponent.
static u32 tempo_hundredths(u32 bits) {
    u32 exponent = (bits >> 23) & 0xffu;
    if ((bits >> 31) != 0 || exponent < 127u || exponent > 150u) return 0;
    u32 mantissa = (bits & 0x7fffffu) | 0x800000u;
    return (mantissa * 100u) >> (150u - exponent);
}

// Microseconds a pulse, from the period (samples at 44,100 a second, times 2^18): period x 10^6 / 44,100 / 2^18, as
// period x 46,440 / 2^29 in two halves, rounded.
static u32 pulse_us(u32 period) {
    u32 high = period >> 13, low = period & 0x1fffu;
    return (high * 46440u + ((low * 46440u) >> 13) + 32768u) >> 16;
}

static u32 clock_now() {
    u32 sync = W32(SYNC);
    if (sync == 2u || sync == 4u) return OP1F_CLOCK_MIDI;
    u32 transport = W32(TRANSPORT);
    return transport == 1u || transport == 2u ? (u32)OP1F_CLOCK_TAPE : (u32)OP1F_CLOCK_OWN;
}

static void seq_tick(Player* p, u32 pulse) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    if (!q) return;
    u32 host = p->host;
    int last = last_pulse[host];
    u32 next = last < 0 ? pulse : ((u32)last + 1u == (u32)PULSE_WRAP ? 0u : (u32)last + 1u);
    last_pulse[host] = (int)pulse;
    ++counts[n][C_TICKS];
    if (q->tick) {
        Op1fTick t;
        t.bytes = sizeof(Op1fTick);
        t.host = host;
        t.pulse = pulse;
        t.count = tick_count[host];
        t.tempo = tempo_hundredths(W32(TEMPO));
        t.us = pulse_us(W32(PERIOD));
        t.clock = clock_now();
        t.jumped = pulse != next ? 1u : 0u;
        Op1fCallKept kept;
        op1f_cards_call_in(OP1F_CALL_TICK, reg[n].slot, &kept);
        q->tick(&t);
        op1f_cards_call_out(OP1F_CALL_TICK, &kept);
    }
    tick_count[host] = tick_count[host] + 1u;
}

// The stop: the pack's run(STOP), then every note it left sounding on the host ended by the base.
static void seq_stop(Player* p) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    if (!q) return;
    u32 host = p->host;
    told_on[host] = 0;
    call_run(n, host, OP1F_RUN_STOP);
    u32 left[LEDGER_WORDS], quiet[LEDGER_WORDS], mask;
    // An instrument the unit is not on: the firmware ended its notes at the change, and an off now would go to the
    // instrument the unit is on. The list is emptied and nothing is sent.
    int elsewhere = W32(INSTRUMENT) != host;
    MASK_OFF(mask);
    for (u32 w = 0; w < (u32)LEDGER_WORDS; ++w) {
        left[w] = ledger[host][w];
        quiet[w] = quiet_notes[host][w];
        ledger[host][w] = 0;
        quiet_notes[host][w] = 0;
    }
    stops[host] = stops[host] + 1u;
    MASK_ON(mask);
    for (u32 w = 0; w < (u32)LEDGER_WORDS; ++w)
        for (u32 b = 0; b < 32u; ++b)
            if (!elsewhere && ((left[w] >> b) & 1u)) {
                STOCK_NOTE_OFF((int)(32u * w + b), OFF_VELOCITY, 0, 0, 0, (int)((quiet[w] >> b) & 1u));
                ++counts[n][C_ENDED];
            }
    tick_count[host] = 0;
    last_pulse[host] = NO_PULSE;
}

static int seq_playing(Player* p) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    if (!q || !q->playing) return 1;
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_SHOWN, reg[n].slot, &kept);
    int playing = q->playing(p->host);
    op1f_cards_call_out(OP1F_CALL_SHOWN, &kept);
    return playing;
}

static int seq_octave(Player* p) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    if (!q || !q->octave) return 1;
    int steps = (int)W32(PENDING_OCTAVE);
    Op1fCallKept kept;
    op1f_cards_call_in(OP1F_CALL_OCTAVE, reg[n].slot, &kept);
    int follow = q->octave(p->host, steps);
    op1f_cards_call_out(OP1F_CALL_OCTAVE, &kept);
    return follow;
}

static int seq_midi_beyond(Player* p) {
    int n;
    const Op1fSequencer* q = table_of(p, &n);
    return q && (q->head.flags & (u32)OP1F_SEQ_ALL_MIDI) ? 1 : 0;
}

static int ctl_nothing(void*) { return 0; }

static u32 ctl_screen(void*) { return W32(SCREEN); }

// The controller's notice of on and off: on, the pack's run(ON); off, nothing (the player's stop follows at once).
static int ctl_on(void*, u32 on) {
    if (!on) return 0;
    u32 host = host_now();
    int n = registered_of(type_of_host(host));
    if (n < 0 || told_on[host]) return 0;               // the firmware says on again at each entry of the screen
    told_on[host] = 1;
    tick_count[host] = 0;
    last_pulse[host] = NO_PULSE;
    call_run(n, host, OP1F_RUN_ON);
    return 0;
}

static const void* const PLAYER_TABLE[11] = {
    (const void*)&seq_nothing, (const void*)&seq_nothing, (const void*)&seq_key_down, (const void*)&seq_key_up, (const void*)&seq_nothing,
    (const void*)&seq_tick, (const void*)&seq_nothing, (const void*)&seq_stop, (const void*)&seq_playing, (const void*)&seq_octave,
    (const void*)&seq_midi_beyond
};

static const void* const CONTROLLER_TABLE[11] = {
    (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_nothing,
    (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_nothing, (const void*)&ctl_screen,
    (const void*)&ctl_on
};

// The switch to one of ours: the player that plays now is stopped, the base's player of the host takes the type.
extern "C" void op1f_seq_switch(u32 type) OP1F_NAMED("op1f_seq_switch");
extern "C" void op1f_seq_switch(u32 type) {
    void* now = STOCK_CURRENT_PLAYER();
    if (now) {
        typedef void (*StopFn)(void* player);
        ((StopFn)(*(const void* const* const*)now)[7])(now);
    }
    u32 host = host_now();
    Player* p = &players[host];
    p->table = PLAYER_TABLE;
    p->host = host;
    p->type = type;
    *STOCK_PLAYER_WORD() = (u32)p;
    W32(TYPE_WORDS + 4u * host) = type;
}

// The controller of every sequencer of ours: the manager reaches it through the two getters.
extern "C" void* op1f_seq_controller() OP1F_NAMED("op1f_seq_controller");
extern "C" void* op1f_seq_controller() {
    controller.table = CONTROLLER_TABLE;
    controller.id = SEQ_MODE;
    return &controller;
}

// ---------------------------------------------------------------- notes out, for a pack

// The registration of the card's whose sequencer is chosen for the instrument the unit is on, when a note may be
// played from where this is called; else -1, and the call counted against the card as refused. A note out of range, a
// note already sounding or not sounding answers 0 and is not counted: a pack may ask so as a matter of course.
static int playing_for(const Op1fCard* card, u32* host) {
    int slot = slot_of(card);
    if (slot < 0) return -1;
    u32 instrument = W32(INSTRUMENT);
    *host = instrument;
    int n = instrument <= 1u && op1f_cards_inside(card) ? registered_of(type_of_host(instrument)) : -1;
    if (n < 0 || reg[n].slot != (u32)slot) {
        ++refused[slot];
        return -1;
    }
    return n;
}

extern "C" int op1f_note_on(const Op1fCard* card, unsigned note, unsigned velocity, unsigned flags) {
    if (note > 127u || velocity == 0 || velocity > 127u) return 0;
    u32 host;
    int n = playing_for(card, &host);
    if (n < 0) return 0;
    u32 word = note >> 5, bit = 1u << (note & 31u), mask, stopped, quiet = flags & (u32)OP1F_NOTE_NO_MIDI;
    MASK_OFF(mask);
    // Still the instrument and the sequencer checked, and the firmware not changing the instrument; nor sounding.
    if (W32(INSTRUMENT) != host || type_of_host(host) != (u32)(TYPE_FIRST + n) || W8(CHANGING) != 0 || (ledger[host][word] & bit)) {
        MASK_ON(mask);
        return 0;
    }
    ledger[host][word] = ledger[host][word] | bit;
    if (quiet) quiet_notes[host][word] = quiet_notes[host][word] | bit;
    else quiet_notes[host][word] = quiet_notes[host][word] & ~bit;
    stopped = stops[host];
    MASK_ON(mask);
    STOCK_NOTE_ON((int)note, (int)velocity, 0, 0, 0, 0, (int)quiet);
    ++counts[n][C_ONS];
    MASK_OFF(mask);
    int ended = stops[host] != stopped;                 // a stop came meanwhile: what it ended may have begun after it
    if (ended) {
        ledger[host][word] = ledger[host][word] & ~bit;
        quiet_notes[host][word] = quiet_notes[host][word] & ~bit;
    }
    MASK_ON(mask);
    if (ended) {
        STOCK_NOTE_OFF((int)note, OFF_VELOCITY, 0, 0, 0, (int)quiet);
        ++counts[n][C_ENDED];
    }
    return 1;
}

extern "C" int op1f_note_off(const Op1fCard* card, unsigned note, unsigned flags) {
    if (note > 127u) return 0;
    u32 host;
    int n = playing_for(card, &host);
    if (n < 0) return 0;
    u32 word = note >> 5, bit = 1u << (note & 31u), mask;
    MASK_OFF(mask);
    int sounding = W32(INSTRUMENT) == host && (ledger[host][word] & bit) != 0;
    MASK_ON(mask);
    if (!sounding) return 0;
    STOCK_NOTE_OFF((int)note, OFF_VELOCITY, 0, 0, 0, (int)(flags & (u32)OP1F_NOTE_NO_MIDI));
    MASK_OFF(mask);
    ledger[host][word] = ledger[host][word] & ~bit;
    quiet_notes[host][word] = quiet_notes[host][word] & ~bit;
    MASK_ON(mask);
    ++counts[n][C_OFFS];
    return 1;
}

// (a x b) >> 31 for two words below 2^31, made from four products of sixteen bits.
static u32 fraction(u32 a, u32 b) {
    u32 al = a & 0xffffu, ah = a >> 16, bl = b & 0xffffu, bh = b >> 16;
    u32 low = al * bl, middle1 = ah * bl, middle2 = al * bh, high = ah * bh;
    u32 carry = (low >> 16) + (middle1 & 0xffffu) + (middle2 & 0xffffu);
    u32 low_word = (low & 0xffffu) | (carry << 16);
    u32 high_word = high + (middle1 >> 16) + (middle2 >> 16) + (carry >> 16);
    return (high_word << 1) | (low_word >> 31);
}

// A note's pitch word moved by `cents`, -4800 to 4800: whole octaves by shifts (held below 2^31), the rest through the
// upper half of the table of ratios.
extern "C" u32 op1f_seq_pitch(u32 note, int cents) OP1F_NAMED("op1f_seq_pitch");
extern "C" u32 op1f_seq_pitch(u32 note, int cents) {
    if (cents > (int)CENTS_MOST) cents = CENTS_MOST;
    if (cents < -(int)CENTS_MOST) cents = -(int)CENTS_MOST;
    int octaves = 0;
    while (cents >= (int)OCTAVE_CENTS) { cents -= OCTAVE_CENTS; ++octaves; }
    while (cents < 0) { cents += OCTAVE_CENTS; --octaves; }
    u32 pitch = W32(PITCHES + 4u * note);
    if (cents) pitch = fraction(pitch, W32(RATIOS + 4u * (u32)(RATIO_ONE + cents))) << 1;
    for (; octaves > 0; --octaves) pitch = pitch >= 0x40000000u ? 0x7fffffffu : pitch << 1;
    if (octaves < 0) pitch >>= (u32)(-octaves);
    return pitch;
}

extern "C" int op1f_note_bend(const Op1fCard* card, unsigned note, int cents) {
    if (note > 127u) return 0;
    u32 host;
    if (playing_for(card, &host) < 0) return 0;
    // The bend is asked on the synth only: the instrument word first, then the synth's list of this pack's notes.
    if (W32(INSTRUMENT) != 0) return 0;
    u32 word = note >> 5, bit = 1u << (note & 31u);
    if ((ledger[0][word] & bit) == 0) return 0;
    STOCK_BEND((int)note, (int)op1f_seq_pitch(note, cents));
    return 1;
}

extern "C" int op1f_seq_restart(const Op1fCard* card) {
    u32 host;
    if (playing_for(card, &host) < 0) return 0;
    if (!STOCK_RESTART_ALLOWED()) return 0;
    STOCK_RESTART();
    return 1;
}

// ---------------------------------------------------------------- for the cards unit

extern "C" int op1f_seq_standing(Op1fStanding* out) {
    if (W32(MODE) != (u32)SEQ_MODE) return 0;
    u32 host = host_now();
    int n = registered_of(type_of_host(host));
    if (n < 0) return 0;
    u32 how = 0;
    if (W8(host == 0 ? ON_SYNTH : ON_DRUM) == 0) how |= OP1F_SHOWN_OFF;
    if (overlay_covered != 0) how |= OP1F_SHOWN_COVERED;
    out->slot = reg[n].slot;
    out->ask = reg[n].ask;
    out->where = host;
    out->how = how;
    return 1;
}

// ---------------------------------------------------------------- the lines (the display thread)

static u32 switches_seen, kept_lines, write_due, writes_said, left_said, period_start;
static u32 still_frames;                                // frames since the last switch, up to KEPT_STILL
static QuietGate quiet;

static const char* pack_name(u32 slot) {
    return op1f_cards.slots[slot].card->name;
}

static void say_start() {
    for (u32 i = 0; i < aside_count; ++i) {
        const Aside* s = &asides[i];
        if (s->why == WHY_FIRST)
            STOCK_LOG(2, LOG_FILE, 3, "seq: '%s' asks for a sequencer '%s', which '%s' asked for first: left aside", pack_name(s->slot), s->name, pack_name(s->other));
        else if (s->why == WHY_ORDER)
            STOCK_LOG(2, LOG_FILE, 4, "seq: '%s' asks for a sequencer '%s', which its table is not in order (%u, %u): left aside", pack_name(s->slot), s->name, s->a, s->b);
        else
            STOCK_LOG(2, LOG_FILE, 2, "seq: '%s' asks for a sequencer '%s', which %s: left aside", pack_name(s->slot), s->name,
                      s->why == WHY_STOCK ? "a sequencer of the firmware has that name" : s->why == WHY_SEVENTEENTH ? "would be the seventeenth"
                                                                                      : "is not a name the list can show");
    }
    if (aside_more) STOCK_LOG(2, LOG_FILE, 15, "seq: %u more ask(s) for a sequencer left aside", aside_more);
    if (list_state) {
        STOCK_LOG(1, LOG_FILE, 6, "seq: the list of sequencers could not be read as the base expects (%u): no sequencer of a pack in it", list_state);
    } else if (asks_seen) {
        u32 count = op1f_seq_registered;
        for (u32 n = 0; n < count; ++n)
            STOCK_LOG(3, LOG_FILE, 1, "seq: '%s' of '%s' is in the list of sequencers as number %u, %s", reg[n].name, pack_name(reg[n].slot), (u32)(ID_FIRST + n),
                      (reg[n].table->head.flags & (u32)OP1F_SEQ_SYNTH_ONLY) ? "for the synth only" : "for the synth and the drum");
        STOCK_LOG(3, LOG_FILE, 5, "seq: %u sequencer(s) of the packs added to the list", count);
    }
    if (kept_state == KEPT_LONG || kept_state == KEPT_UNREAD || kept_state == KEPT_MALFORMED)
        STOCK_LOG(2, LOG_FILE, 14, "seq: the choice of sequencers kept before is not used: %s",
                  kept_state == KEPT_LONG ? "the file is longer than 64 bytes" : kept_state == KEPT_UNREAD ? "the file could not be read" : "the file is not two lines of a name each");
    for (u32 host = 0; host < 2u; ++host)
        if (put_back[host]) STOCK_LOG(3, LOG_FILE, 8, "seq: '%s' put back for the %s: it was chosen before a start without it", reg[put_back[host] - 1].name, HOSTS[host]);
}

// The sequencers the start chose, at the first frame only. A pick is said by the line of the kept choice once it
// stands still: the unit's list makes a switch at every row it passes, so a line a switch would fill the log.
static void say_chosen() {
    for (u32 host = 0; host < 2u; ++host) {
        const char* name = name_of(type_of_host(host));
        STOCK_LOG(3, LOG_FILE, 7, "seq: the %s's sequencer is '%s'", HOSTS[host], name ? name : "?");
    }
}

static void keep_choice() {
    const char* names[2] = { name_of(type_of_host(0)), name_of(type_of_host(1)) };
    if (!names[0] || !names[1]) { write_due = 0; return; }
    char text[KEPT_MOST + 1];
    u32 n = 0;
    const char* parts[4] = { "synth ", names[0], "\ndrum ", names[1] };
    for (u32 p = 0; p < 4u; ++p)
        for (const char* c = parts[p]; *c && n < (u32)KEPT_MOST - 1u; ++c) text[n++] = *c;
    text[n++] = '\n';
    text[n] = 0;
    if (kept_known && same_text(text, kept_text)) { write_due = 0; return; }
    int answer = op1f_folders_write_base(KEPT_PATH, text, n);
    if (answer == OP1F_NOT_NOW) return;                 // the disk mode is open: at a later frame
    write_due = 0;
    if (answer == (int)n) {
        for (u32 i = 0; i <= n; ++i) kept_text[i] = text[i];
        kept_known = 1;
        if (kept_lines < (u32)KEPT_LINES_SAID) {
            ++kept_lines;
            STOCK_LOG(3, LOG_FILE, 9, "seq: the choice of sequencers kept: synth '%s', drum '%s'", names[0], names[1]);
        }
    } else {
        kept_known = 0;
        STOCK_LOG(1, LOG_FILE, 10, "seq: the choice of sequencers could not be kept: %s", "the disk did not take the file");
    }
}

// The period's lines, for each sequencer that did anything: at the period's end through the gate, or at once (`now`)
// when the power popup comes up.
static void say_period(int now) {
    u32 count = op1f_seq_registered, mixed = quiet_start(), active = 0;
    u32 taken[SEQ_MOST][6], refusals[OP1F_CARDS_MOST];
    for (u32 n = 0; n < count; ++n)
        for (u32 c = 0; c < 6u; ++c) {
            taken[n][c] = counts[n][c];
            counts[n][c] = 0;
            mixed = quiet_mix(mixed, taken[n][c]);
            active |= taken[n][c];
        }
    for (u32 s = 0; s < (u32)OP1F_CARDS_MOST; ++s) {
        refusals[s] = refused[s];
        refused[s] = 0;
        active |= refusals[s];
    }
    if (!now && !quiet_period(&quiet, mixed, active != 0)) return;
    u32 slots_said = 0;
    for (u32 n = 0; n < count; ++n) {
        u32 slot = reg[n].slot, first = !((slots_said >> slot) & 1u);
        u32 refusal = first ? refusals[slot] : 0u;
        slots_said |= 1u << slot;
        if ((taken[n][C_TICKS] | taken[n][C_KEYS] | taken[n][C_ONS] | taken[n][C_OFFS] | taken[n][C_ENDED] | refusal) == 0) continue;
        STOCK_LOG(3, LOG_FILE, 13, "seq: '%s' this period: %u tick(s), %u key(s) (%u taken), %u note(s) on, %u off, %u ended by the base, %u refused", reg[n].name,
                  taken[n][C_TICKS], taken[n][C_KEYS], taken[n][C_TAKEN], taken[n][C_ONS], taken[n][C_OFFS], taken[n][C_ENDED], refusal);
    }
    u32 cards = op1f_cards.count < (u32)OP1F_CARDS_MOST ? op1f_cards.count : (u32)OP1F_CARDS_MOST;
    for (u32 s = 0; s < cards; ++s)
        if (refusals[s] && !((slots_said >> s) & 1u))
            STOCK_LOG(2, LOG_FILE, 16, "seq: '%s' asked for %u note(s) this period with no sequencer of its own playing: refused", pack_name(s), refusals[s]);
}

extern "C" void op1f_seq_frame() {
    u32 now = op1f_ms();
    if (frames_seen++ == 0) {
        say_start();
        switches_seen = op1f_seq_switches;
        say_chosen();
        period_start = now;
        return;
    }
    u32 switches = op1f_seq_switches;
    if (switches != switches_seen) {
        switches_seen = switches;
        write_due = 1;
        still_frames = 0;
    } else if (still_frames < (u32)KEPT_STILL) {
        ++still_frames;
    }
    if (writes_without != writes_said) {
        writes_said = writes_without;
        STOCK_LOG(2, LOG_FILE, 11, "seq: a write of the factory settings was asked; written without the packs' %u row(s)", rows_left);
    }
    if (writes_left_out != left_said) {
        left_said = writes_left_out;
        STOCK_LOG(1, LOG_FILE, 12, "seq: a write of the factory settings was asked and left out: its copy could not be made");
    }
    // The choice is kept once it has stood still for KEPT_STILL frames, so that a list whose every step is a pick writes
    // the file once and not at every step; at once when the power popup comes up with a write waiting.
    if (write_due && (still_frames >= (u32)KEPT_STILL || W8(POPUP) != 0)) keep_choice();
    if (quiet_popup(&quiet, W8(POPUP) != 0)) {
        say_period(1);                                  // the last moment to say them before the unit is off
    } else if (now - period_start >= (u32)PERIOD_MS) {
        period_start = now;
        say_period(0);
    }
}

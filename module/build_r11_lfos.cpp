// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R11, base 35: the LFO unit, new. Layers of a sound's list of LFOs (module/build_r6e_card.h, "Base 35: layers").
// A pack asks for one with an ask of kind OP1F_ASK_LFO (11), its table the head of a kind
// (Op1fLfo, 28 bytes at base 35). The layer is a row of the list of LFO kinds (shift and T4) that rides over the unit's
// own LFO of the place: the unit's LFO stays under it and plays as always, and the row never reaches the firmware.
// Step R13: drum sounds ride again, the lists' start held at one head hook, LH, in place of the
// synth list's L1.
//
// What this unit does, by where it runs:
//   the catalog's read (the start's thread, op1f_lfos_catalog_read, called by the sequencer unit after the engines'
//       rows): the first call looks through the asks of kind 11 of the cards in use and registers those in order, ids
//       16 and on, with the row ids of the unit's six LFO kinds; every call adds the layers' rows to `lfo_types` (their
//       `default_params` copied from tremolo's row). op1f_lfos_row_is names those rows at the catalog's write
//   the start of a list of LFO kinds, the synth's or the drum's (the key thread, LH: op1f_lfo_start_do, at the head of
//       the firmware's LFO start (record), which both lists' starts call with their slot's record): the firmware's own
//       start through LH's room, then, for a synth or drum slot's record, the place's LFO as it stands taken (its kind
//       word, its eight words, its switch), the place marked as having its list open, and the start answered: the
//       layer's id where the place rides one, else the firmware's. Any other record: the firmware's answer alone
//   the LFO pick (the key thread, LG: op1f_lfo_pick_do, at the head of the firmware's pick): a layer's id is never
//       handed on (the guard). Picked in the open list, the place rides it, and the unit's LFO of the list's opening is
//       put back under it through the firmware's own pick of that kind, the eight words into the document and the
//       record's LFO apply; the switch the controller sets after the pick is put back at the next two frames after the
//       pick has returned. A layer without OP1F_LFO_DRUM picked on a drum's record: no ride, nothing handed on. Any
//       other id goes on unchanged; while the place's list is open it ends the place's ride first. With no layer
//       registered every call goes on at once
//   a frame (the display thread, op1f_lfos_frame, called by the effects unit at the end of its frame): the lines, the
//       member op1f_lfo read and written through the values unit (op1f_keeps_lfo_get, _set, told by op1f_lfos_changed),
//       the switch put back, the list's mark cleared once the screen asked is no longer the list's (45)
//   the page (op1f_lfos_standing, op1f_lfos_ahead, for the cards unit's stand-in entry 3): a place that rides a layer
//       whose pack chose its own page (OP1F_LAYER_PAGE_OWN), on the LFO screen of the kind the place holds now
//   the helpers' part for kind 11 (op1f_lfos_layer_start, _end, _page, called by the effects unit's op1f_layer_start,
//       op1f_layer_end and op1f_layer_page), from the pack's frame function
// Nothing of it runs on the audio thread: an LFO layer has no function there.
//
// A place is 0 to 7 for a synth sound (its slot), 8 to 15 for a drum sound (8 + its slot), the drum's only for a layer
// with OP1F_LFO_DRUM (step R13). The drum's list shows every layer's row (the unit builds it from the same rows): a pick
// there of a layer without the flag starts no ride and hands the firmware nothing. Never the master. The checks of an
// ask are the cards unit's (its lfo_verdict, which says the line of a refusal): this unit says none of its own for a
// refusal.
//
// What the firmware needs of these, where the code meets it: the LFO start (record) answers a row's id; the LFO pick
// (record, id) puts the catalog row of that id onto the record (its kind and its default words) and applies it; the
// controller switches the LFO on after the pick; the kind word is 0 to 5 in the order tremolo, value, random, element,
// midi, velocity (6 for a name the firmware does not know), the catalog's ids 1 to 6. The catalog's rows are jsoncpp
// values of twelve bytes, the kind byte at +4. Nothing is divided, nothing waits. Compiled for the PC with OP1F_HOST
// defined: the stand-ins come in through a header of the PC's own (for tests this tree does not hold).

#include "build_r6d_base.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef int (*StartRoomFn)(void* record);
typedef void (*WordsDocFn)(void* record, const u32* words);
typedef void (*ActiveDocFn)(void* record, int on);
typedef void (*OnSetFn)(u32 slot, int on);
typedef u32 (*OnReadFn)(u32 slot);
typedef void (*ApplyFn)(void* record);
typedef u32 (*ScreenOfFn)(u32 kind);
typedef void (*PickFn)(void* record, int id);
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

// LG's room (module/build_r11_layers.s): the firmware's LFO pick as it always was.
extern "C" u8 op1f_layer_lg_room[] OP1F_NAMED("op1f_layer_lg_room");
// LH's room (module/build_r11_layers.s): the firmware's LFO start (record) as it always was.
extern "C" u8 op1f_layer_lh_room[] OP1F_NAMED("op1f_layer_lh_room");

#ifdef OP1F_HOST
#include "mock_lfos_r11.h"                              // the PC's stand-ins: the firmware's memory and calls, the masks
#else
#define RD32(a) (*(volatile u32*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
#define STOCK_LFO_WORDS_DOC ((WordsDocFn)0x8d4f3c84u)
#define STOCK_LFO_ACTIVE_DOC ((ActiveDocFn)0x8d4f3d0cu)
#define STOCK_LFO_ON_SYNTH ((OnSetFn)0x8d504308u)
#define STOCK_LFO_ON_SYNTH_READ ((OnReadFn)0x8d504290u)
#define STOCK_LFO_ON_DRUM ((OnSetFn)0x8d5042d8u)
#define STOCK_LFO_ON_DRUM_READ ((OnReadFn)0x8d504260u)
#define STOCK_LFO_APPLY_SYNTH ((ApplyFn)0x8d4f64c0u)
#define STOCK_LFO_APPLY_DRUM ((ApplyFn)0x8d4f4f90u)
#define STOCK_LFO_SCREEN_OF ((ScreenOfFn)0x8fc275e4u)
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
#define ROOM_PICK(record, id) ((PickFn)(void*)op1f_layer_lg_room)((void*)(record), (int)(id))
#define ROOM_START(record) ((StartRoomFn)(void*)op1f_layer_lh_room)((void*)(record))
#endif

// The firmware's words (an enum of their own: they do not fit a signed number).
enum : u32 {
    SYNTH_KINDS = 0x119035a0u,          // a slot's LFO kind, a word a slot; the drum's behind them
    DRUM_KINDS = 0x119035c0u,
    SYNTH_WORDS = 0x119037e0u,          // a slot's eight LFO words, 32 bytes a slot
    DRUM_WORDS = 0x11903ae0u,
    SYNTH_SHOWN = 0x11904704u,          // the slot the GUI shows, the synth's and the drum's
    DRUM_SHOWN = 0x11904708u,
    CURRENT_MODE = 0x8400f978u,         // the mode: 0 synth, 1 drum, 2 tape, 3 mixer
    SYNTH_RECORDS = 0x080235dcu,        // a slot's record, 52 bytes a slot
    DRUM_RECORDS = 0x0802377cu
};

// Base 35's words (module/build_r6e_card.h), named here where that header is not read, as the cards unit does.
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_ASK_LFO = 11, OP1F_LFO_DRUM = 1, OP1F_LFOS_MOST = 8 };
enum { OP1F_LAYER_PAGE_UNIT = 0, OP1F_LAYER_PAGE_OWN = 1 };
#endif

enum {
    LAYERS = OP1F_LFOS_MOST, SLOTS = 8, DRUM = OP1F_PLACE_DRUM, PLACES = DRUM + SLOTS, WORDS = 8, KINDS = 6, ID_FIRST = 16, NAME = OP1F_KIND_NAME,
    TABLE_BYTES = 28, CARDS = OP1F_CARDS_MOST, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, RECORD_BYTES = 52, LIST_SCREEN = 45,
    OPEN_FRAMES = 64, BACK_FRAMES = 2, RIDE_LINES = 32, MEMBER_ROOM = 64,
    KIND_INT = 1, KIND_UINT = 2, KIND_STRING = 4, KIND_ARRAY = 6, KIND_OBJECT = 7, KIND_AT = 4
};
static_assert(sizeof(Op1fKindHead) == TABLE_BYTES, "an LFO layer's table is the head, 28 bytes at base 35");

static const char LOG_FILE[] = "../src/main/op1f_lfos.cpp";
static const char KEY_LIST[] = "lfo_types";
static const char KEY_TYPE[] = "type";
static const char KEY_ID[] = "id";
static const char KEY_DEFAULTS[] = "default_params";

// The unit's six LFO kinds, by the kind word's number.
static const char KIND_NAMES[KINDS][NAME] = {"tremolo", "value", "random", "element", "midi", "velocity"};

// ---- from the cards unit ----

extern "C" Op1fCards op1f_cards OP1F_NAMED("op1f_cards");
// The card's slot + 1 while the base calls that card's frame function on the caller's stack, else 0.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");

// ---- from the values unit (module/build_r6b_keeps.cpp): the base's third member, op1f_lfo ----

extern "C" int op1f_keeps_lfo_get(unsigned where, unsigned which, char* out, unsigned room) OP1F_NAMED("op1f_keeps_lfo_get");
extern "C" int op1f_keeps_lfo_set(unsigned where, unsigned which, const char* text) OP1F_NAMED("op1f_keeps_lfo_set");

// ---- what this unit offers ----

extern "C" void op1f_lfos_catalog_read(void* document) OP1F_NAMED("op1f_lfos_catalog_read");
extern "C" int op1f_lfos_row_is(const void* row) OP1F_NAMED("op1f_lfos_row_is");
extern "C" void op1f_lfos_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_lfos_changed");
extern "C" int op1f_lfo_start_do(void* record) OP1F_NAMED("op1f_lfo_start_do");
extern "C" void op1f_lfo_pick_do(unsigned record, unsigned id) OP1F_NAMED("op1f_lfo_pick_do");
extern "C" void op1f_lfos_frame() OP1F_NAMED("op1f_lfos_frame");
extern "C" int op1f_lfos_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_lfos_standing");
extern "C" int op1f_lfos_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) OP1F_NAMED("op1f_lfos_ahead");
extern "C" int op1f_lfos_layer_start(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_lfos_layer_start");
extern "C" int op1f_lfos_layer_end(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_lfos_layer_end");
extern "C" int op1f_lfos_layer_page(const Op1fCard* card, unsigned ask, unsigned which) OP1F_NAMED("op1f_lfos_layer_page");
extern "C" unsigned op1f_lfos_riding(const Op1fCard* card, unsigned ask) OP1F_NAMED("op1f_lfos_riding");

// ---------------------------------------------------------------- the registry

struct Layer {
    const Op1fKindHead* head;
    u32 slot, ask;                                     // the card's slot in op1f_cards, the ask's number in its list
    u32 flags;                                         // the head's, OP1F_LFO_DRUM
    volatile u32 page;                                // OP1F_LAYER_PAGE_UNIT or _OWN, the pack's choice
    char name[NAME];
};
static Layer layers[LAYERS];
static volatile u32 layer_count;                       // the layers registered, written once at the catalog's first read

struct Candidate { const Op1fKindHead* head; u32 slot, ask; };
static Candidate candidates[LAYERS];
static u32 candidate_count;
static u32 checked;
enum { LIST_NOT_OBJECT = 1, LIST_NOT_ARRAY = 2, LIST_NO_KIND = 3 };
static u32 list_state;                                 // 0 when the layers were registered, else why not
static u32 stock_ids[KINDS];                           // the row id of each of the unit's kinds, 0 where the catalog has none
static u32 kind_screens[KINDS];                        // the LFO screen of each kind, read at the first frame
static volatile u32 rows_added;

// ---------------------------------------------------------------- the places: the synth's eight slots, the drum's eight

struct Place {
    volatile u32 ride;                                 // the layer's number + 1, 0 for none
    u32 member_dirty;                                  // the member to be written again at a frame (OP1F_NOT_NOW)
    u32 absent_said;
    // The place's LFO as it stood when its list opened (the key thread writes it, at LH).
    u32 snap_kind, snap_on;
    u32 snap_words[WORDS];
    // The switch to put back after a layer's pick: the ride it is for, the switch, the frames left (the display thread's).
    volatile u32 back_ride, back_on;
    u32 back_frames;
    u32 open_frames;
};
static Place places[PLACES];

// What the key thread and the values unit tell the display thread: a bit a place, set and taken under a mask.
static volatile u32 picked;                            // a ride begun by a pick
static volatile u32 ended;                             // a ride ended by a pick
static volatile u32 told;                              // the values unit told a load, a start, a drop
static volatile u32 back;                              // a switch to put back
static volatile u32 list_open;                         // the place's list of LFO kinds is open (LH)
static volatile u32 list_seen;                         // and the frame saw its screen asked

static struct {
    u32 first_done;
    u32 ride_lines;
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

static u32 place_kind(u32 place) {
    return place < (u32)DRUM ? RD32(SYNTH_KINDS + 4u * place) : RD32(DRUM_KINDS + 4u * (place - (u32)DRUM));
}

static u32 place_words(u32 place) {
    return place < (u32)DRUM ? SYNTH_WORDS + 32u * place : DRUM_WORDS + 32u * (place - (u32)DRUM);
}

static u32 place_on(u32 place) {
    return (place < (u32)DRUM ? STOCK_LFO_ON_SYNTH_READ(place) : STOCK_LFO_ON_DRUM_READ(place - (u32)DRUM)) != 0 ? 1u : 0u;
}

static u32 place_record(u32 place) {
    return place < (u32)DRUM ? SYNTH_RECORDS + (u32)RECORD_BYTES * place : DRUM_RECORDS + (u32)RECORD_BYTES * (place - (u32)DRUM);
}

// A record's place, by its address: 1 and `place` for one of the sixteen slots' records, else 0.
static int record_place(u32 record, u32* place) {
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        if (record == SYNTH_RECORDS + (u32)RECORD_BYTES * s) { *place = s; return 1; }
        if (record == DRUM_RECORDS + (u32)RECORD_BYTES * s) { *place = (u32)DRUM + s; return 1; }
    }
    return 0;
}

// The switch of a place's LFO set as its own key sets it: the on byte, and the record's document.
static void set_switch(u32 place, u32 on) {
    if (place < (u32)DRUM) STOCK_LFO_ON_SYNTH(place, (int)on);
    else STOCK_LFO_ON_DRUM(place - (u32)DRUM, (int)on);
    STOCK_LFO_ACTIVE_DOC((void*)place_record(place), (int)on);
}

// The record's LFO apply, the synth's or the drum's.
static void apply(u32 place, u32 record) {
    if (place < (u32)DRUM) STOCK_LFO_APPLY_SYNTH((void*)record);
    else STOCK_LFO_APPLY_DRUM((void*)record);
}

// May the layer ride the place: every synth slot; a drum slot with OP1F_LFO_DRUM.
static int may_ride(const Layer* l, u32 place) {
    return place < (u32)DRUM || (place < (u32)PLACES && (l->flags & (u32)OP1F_LFO_DRUM) != 0);
}

static const Layer* riding(u32 place) {
    u32 r = places[place].ride;
    if (r == 0 || r > layer_count) return 0;
    return &layers[r - 1u];
}

static int slot_of(const Op1fCard* card) {
    u32 seen = op1f_cards.count > (u32)CARDS ? (u32)CARDS : op1f_cards.count;
    if (card == 0) return -1;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards.slots[i].card == card) return op1f_cards_verdict(i) == 0 ? (int)i : -1;
    return -1;
}

static u32 kind(const void* value) {
    return ((const volatile u8*)value)[KIND_AT];
}

static const char* text_of(const void* value) {
    if (kind(value) != (u32)KIND_STRING) return 0;
    const char* text = *(const char* const*)value;
    return text ? text : "";
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

static int inside(u32 at, const Op1fCardSlot* slot, u32 bytes) {
    return at >= slot->first && at < slot->end && slot->end - at >= bytes;
}

// Whether an ask of kind 11 is in order, by the cards unit's rules (its lfo_verdict): its table at least the head, in
// its pack on a word, the head saying the ask's bytes; its name plain, 1 to 15 characters, none of the unit's kinds nor
// a layer's taken before it; no flag but OP1F_LFO_DRUM; its `shown` 0 or of its pack on an even address; eight at most.
static int in_order(const Op1fAsk* ask, const Op1fCardSlot* slot) {
    const Op1fKindHead* h = (const Op1fKindHead*)ask->what;
    if (h == 0 || ask->bytes < (u32)TABLE_BYTES || ask->bytes > (u32)OP1F_ASK_BYTES_MOST) return 0;
    if (!inside((u32)h, slot, (u32)TABLE_BYTES) || ((u32)h & 3u) != 0 || h->bytes != ask->bytes) return 0;
    if (!plain_name(h->name)) return 0;
    for (u32 k = 0; k < (u32)KINDS; ++k)
        if (same_text(h->name, KIND_NAMES[k], NAME)) return 0;
    for (u32 c = 0; c < candidate_count; ++c)
        if (same_text(candidates[c].head->name, h->name, NAME)) return 0;
    if ((h->flags & ~(u32)OP1F_LFO_DRUM) != 0) return 0;
    u32 f = (u32)h->shown;
    if (f != 0 && (f < slot->first || f >= slot->end || (f & 1u) != 0)) return 0;
    return candidate_count < (u32)LAYERS;
}

// The asks of kind OP1F_ASK_LFO of the cards the base uses, in the list's order and each card's.
static void look_through() {
    u32 seen = op1f_cards.count > (u32)CARDS ? (u32)CARDS : op1f_cards.count;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;
        const Op1fCardSlot* slot = &op1f_cards.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 8u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_LFO || !in_order(ask, slot)) continue;
            Candidate* c = &candidates[candidate_count++];
            c->head = (const Op1fKindHead*)ask->what;
            c->slot = i;
            c->ask = k;
        }
    }
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

// The row a layer's `default_params` are copied from: tremolo's, else the first of the unit's kinds the list holds.
static const void* source_row(const void* list) {
    for (u32 k = 0; k < (u32)KINDS; ++k) {
        const void* row = row_named(list, KIND_NAMES[k]);
        if (row) return row;
    }
    return 0;
}

// The candidates registered in order, ids 16 and on, with the row ids of the unit's kinds; none when the list holds
// none of the unit's kinds.
static void enrol(const void* list) {
    u32 found = 0;
    for (u32 k = 0; k < (u32)KINDS; ++k) {
        const void* row = row_named(list, KIND_NAMES[k]);
        const void* id = row ? STOCK_JSON_MEMBER(row, KEY_ID) : 0;
        stock_ids[k] = id != 0 && (kind(id) == (u32)KIND_INT || kind(id) == (u32)KIND_UINT) ? (u32)STOCK_JSON_INT(id) : 0u;
        if (stock_ids[k] != 0) ++found;
    }
    if (found == 0) {
        list_state = LIST_NO_KIND;
        return;
    }
    for (u32 c = 0; c < candidate_count; ++c) {
        Layer* l = &layers[c];
        l->head = candidates[c].head;
        l->slot = candidates[c].slot;
        l->ask = candidates[c].ask;
        l->flags = l->head->flags;
        l->page = OP1F_LAYER_PAGE_UNIT;
        copy_name(l->name, l->head->name);
    }
    BARRIER();                                                                      // every record whole before the word that counts them
    layer_count = candidate_count;
}

static void add_row(void* list, u32 n, const void* source) {
    u32 row[3], value[3];
    STOCK_JSON_MAKE(row, KIND_OBJECT);
    STOCK_JSON_TEXT(value, layers[n].name);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_TYPE), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_NUMBER(value, (int)((u32)ID_FIRST + n));
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_ID), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_DEFAULTS), STOCK_JSON_MEMBER(source, KEY_DEFAULTS));
    STOCK_JSON_APPEND(list, row);
    STOCK_JSON_DESTROY(row);
}

extern "C" void op1f_lfos_catalog_read(void* document) {
    if (!checked) {
        checked = 1;
        look_through();
        if (candidate_count == 0) return;
        if (kind(document) != (u32)KIND_OBJECT) list_state = LIST_NOT_OBJECT;
        else if (kind(STOCK_JSON_MEMBER(document, KEY_LIST)) != (u32)KIND_ARRAY) list_state = LIST_NOT_ARRAY;
        if (list_state) return;
        enrol(STOCK_JSON_MEMBER(document, KEY_LIST));
    }
    u32 n = layer_count;
    if (n == 0 || kind(document) != (u32)KIND_OBJECT) return;
    const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
    if (kind(list) != (u32)KIND_ARRAY) return;
    const void* source = source_row(list);
    if (source == 0) return;
    for (u32 i = 0; i < n; ++i) {
        if (row_named(list, layers[i].name)) continue;
        add_row(STOCK_JSON_PLACE(document, KEY_LIST), i, source);
        ++rows_added;
    }
}

// At the catalog's write, under the database's lock: only the JSON functions' readers are called.
extern "C" int op1f_lfos_row_is(const void* row) {
    u32 n = layer_count;
    if (n == 0 || kind(row) != (u32)KIND_OBJECT) return 0;
    const char* text = text_of(STOCK_JSON_MEMBER(row, KEY_TYPE));
    if (text == 0) return 0;
    for (u32 i = 0; i < n; ++i)
        if (same_text(text, layers[i].name, NAME)) return 1;
    return 0;
}

// ---------------------------------------------------------------- LH: a list of LFO kinds opens

// The place's LFO as it stands taken, the list marked open; the start the list opens on answered.
static u32 list_starts(u32 place, u32 start) {
    Place* p = &places[place];
    u32 bit = 1u << place, mask;
    u32 words = place_words(place);
    MASK_OFF(mask);
    p->snap_kind = place_kind(place);
    for (u32 i = 0; i < (u32)WORDS; ++i) p->snap_words[i] = RD32(words + 4u * i);
    MASK_ON(mask);
    u32 on = place_on(place);
    MASK_OFF(mask);
    p->snap_on = on;
    list_open |= bit;
    list_seen &= ~bit;
    p->open_frames = 0;
    u32 r = p->ride;
    MASK_ON(mask);
    if (r != 0 && r <= layer_count) return (u32)ID_FIRST + r - 1u;
    return start;
}

// The head of the firmware's LFO start (record), from LH's entry: the key thread, the synth's list and the drum's (their
// starts of a slot call it with the slot's record, and nothing else calls it). The firmware's answer first, through
// the room; then, for a slot's record, the snapshot and the mark. Any other record: the firmware's answer alone.
extern "C" int op1f_lfo_start_do(void* record) {
    int start = ROOM_START(record);
    u32 place = 0;
    if (layer_count == 0 || !record_place((u32)record, &place)) return start;
    return (int)list_starts(place, (u32)start);
}

// ---------------------------------------------------------------- LG: the LFO pick

// A pick of another row ends the place's ride, while its list is open. Inside no mask.
static void end_by_pick(u32 place) {
    u32 bit = 1u << place, mask;
    MASK_OFF(mask);
    if (places[place].ride != 0 && (list_open & bit) != 0) {
        places[place].ride = 0;
        ended |= bit;
        picked &= ~bit;
        back &= ~bit;
    }
    MASK_ON(mask);
}

// The head of the firmware's pick (record, id), from LG's entry: the key thread, and `lfo` 11's own calls.
extern "C" void op1f_lfo_pick_do(unsigned record, unsigned id) {
    u32 n = layer_count;
    if (n == 0) {                                                                   // no layer: the firmware's pick at once
        ROOM_PICK(record, id);
        return;
    }
    int ours = id >= (u32)ID_FIRST && id < (u32)ID_FIRST + n;
    u32 place = 0;
    if (!record_place(record, &place)) {
        if (!ours) ROOM_PICK(record, id);                                           // the guard: a layer's id never goes on
        return;
    }
    if (!ours) {
        end_by_pick(place);
        ROOM_PICK(record, id);
        return;
    }
    u32 k = id - (u32)ID_FIRST, bit = 1u << place, mask;
    if (!may_ride(&layers[k], place)) {                                             // a layer the drum does not take: no ride, nothing handed on
        end_by_pick(place);                                                         // the cursor left the row the place rode
        return;
    }
    Place* p = &places[place];
    MASK_OFF(mask);
    u32 open = list_open & bit;
    u32 was = p->ride;
    u32 again = was == k + 1u && open && place_kind(place) == p->snap_kind;          // the list's own pick of the row it opened on
    p->ride = k + 1u;
    if (was != k + 1u) {
        picked |= bit;
        ended &= ~bit;
    }
    u32 kind_then = p->snap_kind, on_then = p->snap_on;
    u32 words[WORDS];
    for (u32 i = 0; i < (u32)WORDS; ++i) words[i] = p->snap_words[i];
    MASK_ON(mask);
    if (!open) return;                                                              // outside the list: the ride over the LFO as it stands
    u32 stock = !again && kind_then < (u32)KINDS ? stock_ids[kind_then] : 0u;
    if (stock != 0) {
        ROOM_PICK(record, stock);                                                   // the unit's LFO of the list's opening: its kind
        STOCK_LFO_WORDS_DOC((void*)record, words);                                 // its eight words, into the document and applied
        apply(place, record);
        u32 at = place_words(place);
        for (u32 i = 0; i < (u32)WORDS; ++i) WR32(at + 4u * i, words[i]);          // as they were, to the last bit
    }
    // The controller switches the LFO on behind the pick (+0x54): the switch of the opening is asked back only now that
    // the firmware's pick has returned, so that no frame run while the key thread waited inside it counts (the effects'
    // walk on the unit, 2026-11-24); the next two frames then come after the controller's switch.
    MASK_OFF(mask);
    p->back_ride = k + 1u;
    p->back_on = on_then;
    p->back_frames = 0;
    back |= bit;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the base's member, op1f_lfo

static u32 keep_where(u32 place) {
    return place < (u32)DRUM ? (u32)OP1F_KEEP_SYNTH : (u32)OP1F_KEEP_DRUM;
}

static u32 keep_which(u32 place) {
    return place < (u32)DRUM ? place : place - (u32)DRUM;
}

// The member's text read: 1 with the name (one to fifteen characters); 0 when the text is not an object with a member
// "name" holding a text (`name` then "?").
static int member_of(const char* text, char* name) {
    copy_name(name, "?");
    u32 at = 0;
    while (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r') ++at;
    if (text[at] != '{') return 0;
    for (u32 i = at; text[i]; ++i) {
        if (text[i] != '"' || !same_text(text + i + 1, "name\"", 5)) continue;
        u32 k = i + 6;
        while (text[k] == ' ' || text[k] == ':') ++k;
        if (text[k] != '"') return 0;
        ++k;
        u32 len = 0;
        while (text[k + len] && text[k + len] != '"') ++len;
        if (len == 0 || len >= (u32)NAME || text[k + len] != '"') return 0;
        for (u32 c = 0; c < len; ++c) name[c] = text[k + c];
        for (u32 c = len; c < (u32)NAME; ++c) name[c] = 0;
        return 1;
    }
    return 0;
}

static u32 put(char* out, u32 at, const char* text) {
    while (*text && at + 1u < (u32)MEMBER_ROOM) out[at++] = *text++;
    out[at] = 0;
    return at;
}

// The member written as the place rides, or taken away when it rides nothing: again at the next frame only when the
// values unit answered OP1F_NOT_NOW.
static void write_member(u32 place) {
    const Layer* l = riding(place);
    int answer;
    if (l) {
        char text[MEMBER_ROOM];
        u32 at = put(text, 0, "{\"name\": \"");
        at = put(text, at, l->name);
        put(text, at, "\"}");
        answer = op1f_keeps_lfo_set(keep_where(place), keep_which(place), text);
    } else {
        answer = op1f_keeps_lfo_set(keep_where(place), keep_which(place), 0);
    }
    places[place].member_dirty = answer == OP1F_NOT_NOW ? 1u : 0u;
}

extern "C" void op1f_lfos_changed(unsigned where, unsigned which, unsigned how) {
    if (how != (u32)OP1F_KEPT_LOADED && how != (u32)OP1F_KEPT_START && how != (u32)OP1F_KEPT_DROPPED) return;
    if (which >= (u32)SLOTS) return;
    u32 place = where == (u32)OP1F_KEEP_SYNTH ? which : where == (u32)OP1F_KEEP_DRUM ? (u32)DRUM + which : (u32)PLACES;
    if (place >= (u32)PLACES) return;
    u32 mask;
    MASK_OFF(mask);
    told |= 1u << place;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the frame's work

static const char* place_words_for_log(u32 place, char* out) {
    const char* host = place < (u32)DRUM ? "synth " : "drum ";
    u32 at = 0;
    while (*host) out[at++] = *host++;
    out[at++] = (char)('1' + (place & 7u));
    out[at] = 0;
    return out;
}

static void say_ride(u32 place, const char* what, const char* why) {
    if (unit.ride_lines >= (u32)RIDE_LINES) return;
    ++unit.ride_lines;
    char words[12];
    const char* p = place_words_for_log(place, words);
    if (what) op1f_log(3, LOG_FILE, 2, "lfo: %s rides '%s'", p, what);
    else op1f_log(3, LOG_FILE, 3, "lfo: %s rides nothing now: %s", p, why);
}

// The member of a place read after a start or a load: the place rides the layer it names when its pack is here (and
// the drum's place takes it); else the ride is let go, and the member left as the values unit holds it. Nothing of the
// unit's LFO changes: it is the owner's.
static void read_member(u32 place) {
    char text[MEMBER_ROOM];
    int got = op1f_keeps_lfo_get(keep_where(place), keep_which(place), text, (u32)MEMBER_ROOM);
    char name[NAME];
    int named = got >= 0 && member_of(text, name);
    u32 n = layer_count, found = 0, known = 0;
    for (u32 i = 0; named && i < n; ++i) {
        if (!same_text(name, layers[i].name, NAME)) continue;
        known = 1;                                                                  // a layer of a pack that is here
        if (may_ride(&layers[i], place)) found = i + 1u;
    }
    u32 was = places[place].ride, mask;
    MASK_OFF(mask);
    places[place].ride = found;
    if (found != was) back &= ~(1u << place);
    MASK_ON(mask);
    if (found == 0) {
        if (named && !known && !places[place].absent_said) {
            places[place].absent_said = 1;
            char words[12];
            op1f_log(2, LOG_FILE, 4, "lfo: %s rides '%s', whose pack is not here: the sound plays its own LFO", place_words_for_log(place, words), name);
        }
        if (was) say_ride(place, 0, known ? "the drum does not take the layer it names" : named ? "the layer it names is not here" : "a sound without it was loaded");
        return;
    }
    if (was != found) say_ride(place, layers[found - 1u].name, 0);
}

static void first_frame() {
    u32 n = layer_count;
    if (list_state) op1f_log(2, LOG_FILE, 5, "lfo: the unit's list of LFOs could not be read (%u): no LFO layer is registered", list_state);
    for (u32 k = 0; k < n; ++k)
        op1f_log(3, LOG_FILE, 1, "lfo: '%s' of '%s' is a layer of the lists of LFOs, entry %u%s", layers[k].name, op1f_cards.slots[layers[k].slot].card->name,
                 (u32)ID_FIRST + k, (layers[k].flags & (u32)OP1F_LFO_DRUM) ? ", the drum's too" : "");
    if (n == 0) return;
    for (u32 k = 0; k < (u32)KINDS; ++k) kind_screens[k] = STOCK_LFO_SCREEN_OF(k);
    for (u32 p = 0; p < (u32)PLACES; ++p) read_member(p);
}

extern "C" void op1f_lfos_frame() {
    if (!unit.first_done) {
        unit.first_done = 1;
        first_frame();
        return;
    }
    if (layer_count == 0) return;
    for (u32 p = 0; p < (u32)PLACES; ++p)                                           // a member refused with OP1F_NOT_NOW at an earlier frame
        if (places[p].member_dirty) write_member(p);
    u32 mask;
    MASK_OFF(mask);
    u32 began = picked, stopped = ended, loaded = told, again = back;
    picked = 0;
    ended = 0;
    told = 0;
    MASK_ON(mask);
    for (u32 p = 0; p < (u32)PLACES; ++p) {
        u32 bit = 1u << p;
        if (stopped & bit) {
            if (places[p].ride == 0) say_ride(p, 0, "another LFO was picked");
            write_member(p);
        }
        if ((began & bit) && places[p].ride != 0) {
            say_ride(p, riding(p)->name, 0);
            write_member(p);
        }
        if (again & bit) {                                                          // after a layer's pick: the switch of the list's opening
            if ((loaded & bit) == 0 && places[p].ride == places[p].back_ride && place_on(p) != places[p].back_on) set_switch(p, places[p].back_on);
            MASK_OFF(mask);
            if (++places[p].back_frames >= (u32)BACK_FRAMES) {
                back &= ~bit;
                places[p].back_frames = 0;
            }
            MASK_ON(mask);
        }
        if (loaded & bit) read_member(p);
    }
    u32 asked = op1f_screen_asked, open = list_open;
    for (u32 p = 0; open != 0 && p < (u32)PLACES; ++p) {                           // the list's mark, cleared once its screen is no longer asked
        u32 bit = 1u << p;
        if ((open & bit) == 0) continue;
        MASK_OFF(mask);
        if (asked == (u32)LIST_SCREEN) list_seen |= bit;
        else if ((list_seen & bit) != 0 || ++places[p].open_frames >= (u32)OPEN_FRAMES) {
            list_open &= ~bit;
            list_seen &= ~bit;
        }
        MASK_ON(mask);
    }
}

// ---------------------------------------------------------------- the page: stand-in entry 3

// Does an LFO layer's page stand in now, or would it if the controller asked for its screen? `asked` 1: only while the
// screen asked is the LFO screen of the kind the place holds (op1f_lfos_standing); 0: whatever screen (op1f_lfos_ahead).
static int stand(Op1fStanding* out, unsigned* mode, unsigned* screen, int asked) {
    if (layer_count == 0 || !unit.first_done) return 0;
    u32 m = RD32(CURRENT_MODE);
    if (m > 1u) return 0;                                                           // the synth's and the drum's modes alone
    u32 place = m == 0 ? (RD32(SYNTH_SHOWN) & 7u) : (u32)DRUM + (RD32(DRUM_SHOWN) & 7u);
    const Layer* l = riding(place);
    if (l == 0 || l->page != (u32)OP1F_LAYER_PAGE_OWN) return 0;
    u32 k = place_kind(place);
    if (k >= (u32)KINDS || kind_screens[k] == 0) return 0;
    if (asked && op1f_screen_asked != kind_screens[k]) return 0;
    out->slot = l->slot;
    out->ask = l->ask;
    out->where = place;
    out->how = place_on(place) ? 0u : (u32)OP1F_SHOWN_OFF;
    *mode = m;
    *screen = kind_screens[k];
    return 1;
}

extern "C" int op1f_lfos_standing(Op1fStanding* out, unsigned* mode, unsigned* screen) {
    return stand(out, mode, screen, 1);
}

extern "C" int op1f_lfos_ahead(Op1fStanding* out, unsigned* mode, unsigned* screen) {
    return stand(out, mode, screen, 0);
}

// ---------------------------------------------------------------- the helpers' part for kind 11

static int layer_of(int slot, u32 ask) {
    u32 n = layer_count;
    for (u32 i = 0; i < n; ++i)
        if (layers[i].slot == (u32)slot && layers[i].ask == ask) return (int)i;
    return -1;
}

static int from_frame(int slot) {
    return op1f_cards_calling() == (u32)slot + 1u;
}

// The ride begun over the place's LFO as it stands: nothing of the unit's put back or changed.
extern "C" int op1f_lfos_layer_start(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    int k = slot < 0 ? -1 : layer_of(slot, ask);
    if (k < 0 || !may_ride(&layers[k], place)) return OP1F_BAD_NAME;               // a drum's place without the flag among them
    if (!from_frame(slot) || (told & (1u << place)) != 0) return OP1F_NOT_NOW;
    if (places[place].ride == (u32)k + 1u) return 0;
    u32 mask;
    MASK_OFF(mask);
    places[place].ride = (u32)k + 1u;
    back &= ~(1u << place);
    MASK_ON(mask);
    say_ride(place, layers[k].name, 0);
    write_member(place);
    return 0;
}

// The ride ended: the member taken away, the unit's LFO left as it is.
extern "C" int op1f_lfos_layer_end(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    int k = slot < 0 ? -1 : layer_of(slot, ask);
    if (k < 0 || place >= (u32)PLACES) return OP1F_BAD_NAME;                       // 0 to 15: a synth's place or a drum's
    if (!from_frame(slot)) return OP1F_NOT_NOW;
    if (places[place].ride != (u32)k + 1u) return OP1F_NOT_HERE;
    u32 mask;
    MASK_OFF(mask);
    places[place].ride = 0;
    back &= ~(1u << place);
    MASK_ON(mask);
    say_ride(place, 0, "its pack ended the ride");
    write_member(place);
    return 0;
}

// Which page T4 shows for a place that rides the layer.
extern "C" int op1f_lfos_layer_page(const Op1fCard* card, unsigned ask, unsigned which) {
    int slot = slot_of(card);
    int k = slot < 0 ? -1 : layer_of(slot, ask);
    if (k < 0 || (which != (u32)OP1F_LAYER_PAGE_UNIT && which != (u32)OP1F_LAYER_PAGE_OWN) || !from_frame(slot)) return 0;
    layers[k].page = which;
    return 1;
}

// The places that ride the card's LFO layer `ask` now, a bit a place (synth 0 to 7, drum 8 to 15); 0 for no such layer.
// For the effects unit's op1f_riding, which hands kind 11 here; from any thread, as op1f_riding.
extern "C" unsigned op1f_lfos_riding(const Op1fCard* card, unsigned ask) {
    int slot = slot_of(card);
    int k = slot < 0 ? -1 : layer_of(slot, ask);
    if (k < 0) return 0;
    u32 bits = 0;
    for (u32 p = 0; p < (u32)PLACES; ++p)
        if (places[p].ride == (u32)k + 1u) bits |= 1u << p;
    return bits;
}

// Step R21, base 43: a pack's drum engine. A unit of its own, new at base 43.
//
// A pack adds an engine to the drum's own engine list (shift and T1 in the drum's mode) through one ask of its card
// (OP1F_ASK_DRUM_ENGINE, 12, its table a level-5 head, module/build_r6e_card.h). The engine rides the unit's drum
// sampler: the unit keeps the drum sampler in the sound and in its files (its kit, its sample, its keys' points), and
// the pack's engine is a page on T1 (the cards unit's, by the engine's entry of the table of screens) and a name in the
// list and in the sound's file. It makes no sound of its own. One pack holds it: the first card in the list's order
// (the packs' file names) with such an ask, the same rule the cards unit holds its page by.
//
// What this unit does, by where it runs:
//   the catalog's read (the start's thread, op1f_drum_catalog_read, called by the sequencer unit): the first call finds
//       the holder and, when its name is free, registers the engine as id 32 of `drum_types`; every call adds its row
//       to the document's `drum_types` (its `default_params` dbox's). No line: the lines come at the first frame
//   the catalog's write (op1f_drum_row_is, from the sequencer unit): the row is left out of what is written
//   a pick in the drum's engine list or by MIDI (D1, op1f_drum_pick_do, the key thread or MIDI's): the engine picked on
//       a drum sound that plays the drum sampler begins the ride, and the firmware's pick is not made (the sound stays
//       as it is); picked on one that plays another engine, nothing changes and a line says why; any other pick ends
//       the ride and goes on to the firmware's pick. Base 40 on the unit: the list loads a kit of the drum sampler as
//       it closes, some 0.1 to 1 s after the pick (a preset_drum kit, read as a loaded sound). Such a load, or a pick
//       of a kit, within GRACE_FRAMES of the pick keeps the ride, and the member is written into the kit loaded
//   a frame (the display thread, op1f_drum_frame, from the cards unit): the lines; the base's member op1f_engine of a
//       drum sound written or taken away (through the values unit, as for a synth sound) after a pick; a drum sound
//       loaded, or the store read at the start, read back into its ride
//   the cards unit asks whether a drum sound rides the engine (op1f_drum_rides), to stand the pack's page in on T1
//
// What the firmware needs of these, where the code meets it (OS 1.7.3):
//   the drum's pick (0x8d4eb160) takes the catalog id picked and the drum slot: an id of `drum_types` (dbox is 1) or,
//       0 and below, a kit of the drum sampler; it hands the work to the background worker and returns
//   the drum's engine list is made from `drum_types` of the catalog (and the drum sampler's kits), as the synth's is
//       from `synth_types`: a row the base adds there shows as the base's rows for synth engines show
//   a drum slot's engine type, a word a slot at 0x11904734 (9 the drum sampler, 10 dbox)
//   the catalog's rows are jsoncpp values of twelve bytes, the kind byte at +4 (as the engines unit reads them)
//
// The threads. The registration on the thread of the start, before the first frame; a pick on the key thread or MIDI's;
// the frame and the member's work on the display thread. What the pick writes is single words inside a short mask.
// Nothing here says a line but from the display thread, nothing waits, and nothing runs on the audio thread.

#include "build_r6d_base.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*PickFn)(u32 id, u32 slot);
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

// The room of the pick's entry (module/build_r18_drum.s): the firmware's pick as it always was.
extern "C" u8 op1f_drum_pick_room[] OP1F_NAMED("op1f_drum_pick_room");

#define RD32(a) (*(volatile u32*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
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
#define ROOM_PICK ((PickFn)(void*)op1f_drum_pick_room)

// The kind of ask, as the header of base 43 (module/build_r6e_card.h) names it; this unit reads base 29's header.
#ifndef OP1F_BUILD_R18_DRUM_H
enum { OP1F_ASK_DRUM_ENGINE = 12 };
#endif

enum : u32 {
    DRUM_TYPES = 0x11904734u            // a drum slot's engine type, a word a slot (9 the drum sampler)
};

enum {
    SLOTS = 8, DRUM_SAMPLER = 9, ID = 32, NAME = OP1F_KIND_NAME, HEAD_BYTES = 28, CARDS = OP1F_SLOTS_MOST,
    KIND_STRING = 4, KIND_ARRAY = 6, KIND_OBJECT = 7, KIND_AT = 4, MEMBER_ROOM = 64, RIDE_LINES = 32,
    GRACE_FRAMES = 150                 // frames after a pick of the engine in which the list's own load of a kit keeps the ride
};
static_assert(sizeof(Op1fKindHead) == HEAD_BYTES, "level 5's head, 28 bytes");

static const char LOG_FILE[] = "../src/main/op1f_drum.cpp";
static const char KEY_LIST[] = "drum_types";
static const char KEY_TYPE[] = "type";
static const char KEY_ID[] = "id";
static const char KEY_DEFAULTS[] = "default_params";
static const char DBOX_NAME[] = "dbox";

// Names no pack's drum engine may take: the unit's own drum engines as the catalog and the list may name them. Fixed
// fields of 16 bytes; the empty name ends the list. The name is also held against every row of `drum_types`.
static const char REFUSED_NAMES[][16] = {"dbox", "drum", "drums", "sampler", "drum sampler", "drum_sampler", "synth", ""};

// The mark of a base of base 43's drum engine (a constant 1): a pack with an ask of kind OP1F_ASK_DRUM_ENGINE names it.
extern "C" const unsigned op1f_drum_engine_1 OP1F_NAMED("op1f_drum_engine_1");
const unsigned op1f_drum_engine_1 = 1;

#include "build_r3_base.h"                              // R20 (base 42): op1f_cards_2, op1f_cards_lists

// The values unit's pair for the base's member op1f_engine at a drum slot (module/build_r6b_keeps.cpp): as
// op1f_keeps_engine_get and op1f_keeps_engine_set for a synth slot. From the display thread only.
extern "C" int op1f_keeps_drum_engine_get(unsigned slot, char* out, unsigned room) OP1F_NAMED("op1f_keeps_drum_engine_get");
extern "C" int op1f_keeps_drum_engine_set(unsigned slot, const char* text) OP1F_NAMED("op1f_keeps_drum_engine_set");

// ---- the words other units read ----

extern "C" volatile unsigned op1f_drum_awake OP1F_NAMED("op1f_drum_awake");
volatile unsigned op1f_drum_awake;                     // not 0 once the engine is registered: the pick's entry reads it

// ---- this unit's state ----

enum { WHY_NONE = 0, WHY_TAKEN = 1, WHY_NAME = 2, WHY_LIST = 3, WHY_DBOX = 4 };

static struct {
    u32 checked;                                       // the first catalog read made
    u32 slot1, ask;                                    // the holder's card slot + 1 (0 for none), its ask
    u32 why;                                           // WHY_ when the holder's engine is left aside
    u32 registered;                                    // 1 when the row is the list's
    char name[NAME];
    u32 said;                                          // the registration's line said
    u32 ride_lines;
} unit;

static volatile u32 rides;                             // a bit a drum slot: it rides the pack's engine
static volatile u32 member_due;                        // a bit a drum slot: its member to be written at a frame
static volatile u32 refused;                           // a bit a drum slot: the engine was picked on another engine
static volatile u32 told;                              // a bit a drum slot: a sound was loaded there, its member to read
static u32 said_rides;                                 // the rides the lines last said
static volatile u32 frames;                            // the frames this unit has seen, written by the display thread
static volatile u32 grace_until[SLOTS];                // the frame up to which a kit loaded on that slot keeps the ride
static volatile u32 kept;                              // a bit a drum slot: a load after the pick was taken into the ride
static volatile u32 loads[SLOTS];                      // the sounds loaded onto each drum slot since the start (the sample unit's `dropped`)

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

static int plain_name(const char* name, u32 field) {
    u32 n = 0;
    for (; n < field && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < field;
}

static int refused_name(const char* name) {
    for (u32 i = 0; REFUSED_NAMES[i][0]; ++i)
        if (same_text(name, REFUSED_NAMES[i], 16)) return 1;
    return 0;
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

// ---------------------------------------------------------------- the catalog

// The holder: the first card the base uses, of level 8 or later, with an ask of this kind whose table is a head. The
// cards unit holds the page by the same rule (module/build_r6d_cards.cpp, drum_look).
static void find_holder() {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;
        const Op1fCard* card = op1f_cards_2.slots[i].card;
        if (card->bytes < (u32)OP1F_CARD_LEVEL_8_BYTES || card->level < 8u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* a = &card->asks[k];
            if (a->kind != (u32)OP1F_ASK_DRUM_ENGINE || a->what == 0 || a->bytes < (u32)HEAD_BYTES) continue;
            unit.slot1 = i + 1u;
            unit.ask = k;
            const Op1fKindHead* head = (const Op1fKindHead*)a->what;
            if (!plain_name(head->name, NAME)) {
                unit.why = WHY_NAME;
                return;
            }
            copy_name(unit.name, head->name);
            if (refused_name(unit.name)) unit.why = WHY_TAKEN;
            return;
        }
    }
}

static void add_row(void* list, const void* dbox) {
    u32 row[3], value[3];
    STOCK_JSON_MAKE(row, KIND_OBJECT);
    STOCK_JSON_TEXT(value, unit.name);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_TYPE), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_NUMBER(value, (int)ID);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_ID), value);
    STOCK_JSON_DESTROY(value);
    STOCK_JSON_ASSIGN(STOCK_JSON_PLACE(row, KEY_DEFAULTS), STOCK_JSON_MEMBER(dbox, KEY_DEFAULTS));
    STOCK_JSON_APPEND(list, row);
    STOCK_JSON_DESTROY(row);
}

extern "C" void op1f_drum_catalog_read(void* document) OP1F_NAMED("op1f_drum_catalog_read");
extern "C" void op1f_drum_catalog_read(void* document) {
    if (!unit.checked) {
        unit.checked = 1;
        find_holder();
        if (unit.slot1 == 0 || unit.why) return;
        if (kind(document) != (u32)KIND_OBJECT || kind(STOCK_JSON_MEMBER(document, KEY_LIST)) != (u32)KIND_ARRAY) {
            unit.why = WHY_LIST;
            return;
        }
        const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
        if (row_named(list, unit.name)) {
            unit.why = WHY_TAKEN;
            return;
        }
        if (row_named(list, DBOX_NAME) == 0) {
            unit.why = WHY_DBOX;
            return;
        }
        BARRIER();
        unit.registered = 1;
        op1f_drum_awake = 1;
    }
    if (!unit.registered || kind(document) != (u32)KIND_OBJECT) return;
    const void* list = STOCK_JSON_MEMBER(document, KEY_LIST);
    if (kind(list) != (u32)KIND_ARRAY || row_named(list, unit.name)) return;
    const void* dbox = row_named(list, DBOX_NAME);
    if (dbox == 0) return;
    add_row(STOCK_JSON_PLACE(document, KEY_LIST), dbox);
}

extern "C" int op1f_drum_row_is(const void* row) OP1F_NAMED("op1f_drum_row_is");
extern "C" int op1f_drum_row_is(const void* row) {
    if (!unit.registered || kind(row) != (u32)KIND_OBJECT) return 0;
    const char* text = text_of(STOCK_JSON_MEMBER(row, KEY_TYPE));
    return text != 0 && same_text(text, unit.name, NAME) ? 1 : 0;
}

// ---------------------------------------------------------------- D1: a pick in the drum's engine list, or by MIDI

extern "C" void op1f_drum_pick_do(unsigned id, unsigned slot) OP1F_NAMED("op1f_drum_pick_do");
extern "C" void op1f_drum_pick_do(unsigned id, unsigned slot) {
    u32 mask;
    if (slot >= (u32)SLOTS || !unit.registered) {
        ROOM_PICK(id, slot);
        return;
    }
    u32 bit = 1u << slot;
    if (id == (u32)ID) {
        MASK_OFF(mask);
        if (RD32(DRUM_TYPES + 4u * slot) == (u32)DRUM_SAMPLER) {
            if ((rides & bit) == 0) member_due |= bit;
            rides |= bit;
            told &= ~bit;
            grace_until[slot] = frames + (u32)GRACE_FRAMES;                         // the list loads a kit as it closes
        } else {
            refused |= bit;                                                         // the sound stays as it is
        }
        MASK_ON(mask);
        return;                                                                     // the drum sampler plays on: no pick of the firmware's
    }
    if ((rides & bit) != 0) {
        MASK_OFF(mask);
        if ((int)id <= 0 && (int)(grace_until[slot] - frames) > 0) {
            member_due |= bit;                                                      // a kit of the drum sampler, just after the pick: the ride stays
            kept |= bit;
        } else {
            rides &= ~bit;
            member_due |= bit;
            told &= ~bit;
        }
        MASK_ON(mask);
    }
    ROOM_PICK(id, slot);
}

// ---------------------------------------------------------------- the member, op1f_engine, of a drum sound

// What the values unit tells of the member at a drum slot (module/build_r6b_keeps.cpp, from op1f_keeps_frame on the
// display thread): a sound loaded there, the store read at the start, a value dropped. Read back at the next frame.
extern "C" void op1f_drum_changed(unsigned where, unsigned which, unsigned how) OP1F_NAMED("op1f_drum_changed");
extern "C" void op1f_drum_changed(unsigned where, unsigned which, unsigned how) {
    if (where != (u32)OP1F_KEEP_DRUM || which >= (u32)SLOTS || !unit.registered) return;
    if (how != (u32)OP1F_KEPT_LOADED && how != (u32)OP1F_KEPT_START && how != (u32)OP1F_KEPT_DROPPED) return;
    u32 mask, bit = 1u << which;
    MASK_OFF(mask);
    if (how == (u32)OP1F_KEPT_LOADED) loads[which] = loads[which] + 1u;
    if (how == (u32)OP1F_KEPT_LOADED && (rides & bit) != 0 && (int)(grace_until[which] - frames) > 0) {
        member_due |= bit;                                                          // the kit the list loads as it closes: the ride stays, its member written
        kept |= bit;
        told &= ~bit;
    } else {
        told |= bit;
        member_due &= ~bit;                                                         // the loaded sound's member decides
    }
    MASK_ON(mask);
}

// The member's text: 1 when it is the JSON string of the engine's name.
static int names_ours(const char* text) {
    u32 at = 0;
    while (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r') ++at;
    if (text[at] != '"') return 0;
    ++at;
    u32 len = 0;
    while (text[at + len] && text[at + len] != '"' && len < (u32)NAME) ++len;
    if (len == 0 || len >= (u32)NAME || text[at + len] != '"') return 0;
    for (u32 c = 0; c < len; ++c)
        if (text[at + c] != unit.name[c]) return 0;
    return unit.name[len] == 0;
}

static void read_member(u32 slot) {
    char text[MEMBER_ROOM];
    int got = op1f_keeps_drum_engine_get(slot, text, (unsigned)MEMBER_ROOM);
    u32 bit = 1u << slot, mask;
    MASK_OFF(mask);
    if (got > 0 && names_ours(text) && RD32(DRUM_TYPES + 4u * slot) == (u32)DRUM_SAMPLER) rides |= bit;
    else rides &= ~bit;                                                             // none, another's, or a sound on another engine
    told &= ~bit;
    MASK_ON(mask);
}

static void write_member(u32 slot) {
    u32 bit = 1u << slot;
    int answer;
    if ((rides & bit) != 0) {
        char text[MEMBER_ROOM];
        u32 at = put(text, 0, MEMBER_ROOM, "\"");
        at = put(text, at, MEMBER_ROOM, unit.name);
        put(text, at, MEMBER_ROOM, "\"");
        answer = op1f_keeps_drum_engine_set(slot, text);
    } else {
        answer = op1f_keeps_drum_engine_set(slot, 0);
    }
    if (answer == OP1F_NOT_NOW) return;                                             // again at the next frame
    u32 mask;
    MASK_OFF(mask);
    member_due &= ~bit;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the frame's work

static const char* why_text(u32 why) {
    switch (why) {
    case WHY_TAKEN: return "its name is one the drum's engine list already shows";
    case WHY_NAME: return "its name is not one the drum's engine list can show";
    case WHY_LIST: return "the unit's catalog holds no list of drum engines";
    case WHY_DBOX: return "the unit's list of drum engines holds no row for dbox";
    }
    return "?";
}

extern "C" void op1f_drum_frame() OP1F_NAMED("op1f_drum_frame");
extern "C" void op1f_drum_frame() {
    frames = frames + 1u;
    if (unit.slot1 == 0) return;
    const char* pack = op1f_cards_2.slots[unit.slot1 - 1u].card->name;
    if (!unit.said) {
        unit.said = 1;
        if (unit.registered)
            op1f_log(3, LOG_FILE, 1, "drum: '%s' of '%s' rides the drum sampler, entry %u of the drum's engine list", unit.name, pack, (u32)ID);
        else
            op1f_log(2, LOG_FILE, 2, "drum: the drum engine of '%s' (its ask %u) is left aside: %s", pack, unit.ask, why_text(unit.why));
    }
    if (!unit.registered) return;
    u32 r = refused;
    if (r) {
        u32 mask;
        MASK_OFF(mask);
        refused = 0;
        MASK_ON(mask);
        for (u32 s = 0; s < (u32)SLOTS; ++s)
            if (((r >> s) & 1u) && unit.ride_lines < (u32)RIDE_LINES) {
                ++unit.ride_lines;
                op1f_log(2, LOG_FILE, 3, "drum: '%s' picked on drum %u, which plays another engine: pick the drum sampler there first", unit.name, s + 1u);
            }
    }
    u32 k = kept;
    if (k) {
        u32 mask;
        MASK_OFF(mask);
        kept = 0;
        MASK_ON(mask);
        for (u32 s = 0; s < (u32)SLOTS; ++s)
            if (((k >> s) & 1u) && unit.ride_lines < (u32)RIDE_LINES) {
                ++unit.ride_lines;
                op1f_log(3, LOG_FILE, 5, "drum: drum %u keeps '%s' over the kit loaded just after the pick", s + 1u, unit.name);
            }
    }
    for (u32 s = 0; s < (u32)SLOTS; ++s) {
        u32 bit = 1u << s;
        if ((told & bit) != 0) read_member(s);
        else if ((member_due & bit) != 0) write_member(s);
    }
    u32 now = rides;
    if (now != said_rides) {
        for (u32 s = 0; s < (u32)SLOTS; ++s) {
            u32 bit = 1u << s;
            if (((now ^ said_rides) & bit) == 0 || unit.ride_lines >= (u32)RIDE_LINES) continue;
            ++unit.ride_lines;
            if (now & bit) op1f_log(3, LOG_FILE, 4, "drum: drum %u rides '%s'", s + 1u, unit.name);
            else op1f_log(3, LOG_FILE, 4, "drum: drum %u plays the drum sampler's own page again", s + 1u);
        }
        said_rides = now;
    }
}

// For the cards unit: 1 while drum slot `drum` rides the engine of the ask `ask` of the card in slot `slot`, and still
// plays the drum sampler.
extern "C" int op1f_drum_rides(unsigned drum, unsigned slot, unsigned ask) OP1F_NAMED("op1f_drum_rides");
extern "C" int op1f_drum_rides(unsigned drum, unsigned slot, unsigned ask) {
    if (!unit.registered || drum >= (u32)SLOTS || unit.slot1 != slot + 1u || unit.ask != ask) return 0;
    if (((rides >> drum) & 1u) == 0) return 0;
    return RD32(DRUM_TYPES + 4u * drum) == (u32)DRUM_SAMPLER ? 1 : 0;
}

// For the sample unit (module/build_r17_sample.cpp), in the engines unit's words (op1f_engines_ride): drum slot `slot`'s
// ride, 0 none, 1 with `card` (the card's slot) and `ask`, 2 the same while a sound loaded there is not read back yet;
// `loads` the sounds loaded onto the slot since the start. Any thread: single words.
extern "C" unsigned op1f_drum_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* count) OP1F_NAMED("op1f_drum_ride");
extern "C" unsigned op1f_drum_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* count) {
    if (slot >= (u32)SLOTS) return 0;
    if (count) *count = loads[slot];
    if (!unit.registered || unit.slot1 == 0) return 0;
    if (card) *card = unit.slot1 - 1u;
    if (ask) *ask = unit.ask;
    if ((told >> slot) & 1u) return 2;
    if (((rides >> slot) & 1u) == 0 || RD32(DRUM_TYPES + 4u * slot) != (u32)DRUM_SAMPLER) return 0;
    return 1;
}

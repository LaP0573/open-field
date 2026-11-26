// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R17, base 39: the sample unit, new. A lifted cut of the tape dropped into the sample of a synth sound that rides
// a pack's engine, by the unit's own clipboard worker; any sound's sample read (the words for packs are
// module/build_r6e_card.h's part "Base 39").
//
// What it does, by where it runs:
//   the call (op1f_sample_drop, the key thread, inside the card's own input function: the cards unit's dispatch marks
//       the card it offers an event to and that event's thread, op1f_cards_input_keyed): the checks of
//       module/build_r17_sample.h over single words (and the clipboard's item test, the firmware's own),
//       then, in one masked instant, the ticket armed (the slot, its number, `queued`) and the slot written where SD's
//       stub looks; out of the mask, the firmware's synth-drop poster for the slot. When the queue's write place did
//       not move, the unit took nothing: the ticket is disarmed and the call answers OP1F_FAILED
//   the unit's turn (SD, the head of the worker's synth-drop job, 0x8f00b7b0; the stub and its room are
//       module/build_r17_sample.s): a job for a slot no ticket is armed for goes on to the room, the firmware's own
//       job as it was, and is counted (the stub counts it). A job for the armed slot comes here (op1f_sample_job): the
//       ticket taken in a mask (`running`), the checks again (in the unit's turn: neither the ticket nor the queue,
//       which holds this job), and then either a refusal, which calls nothing of the firmware's and writes no file, or
//       the drop, by four calls of the firmware's: the holder (slot), the import at its entry (the holder's word),
//       the mark (slot), the record's method 6 (the record); then the place's `dropped`, `answer` and `done`, and
//       the ticket `ended`. No line, nothing of a pack's
//   a frame (op1f_sample_frame, the display thread, from the cards unit before the frame functions): the lines of the
//       call's refusals, of the ticket (queued, then rendered or refused in the unit's turn) and of the unit's own
//       drops counted at SD; an end said frees the ticket, and a drop that rendered is handed to op1f_sample_kept (weak:
//       the restore's, which keeps its frames and channels for the next start)
//   the read (op1f_sample_get, the card's frame function): any synth or drum place's sample, three fields of its
//       object and the base's words; no firmware call
//   the restore's two calls (op1f_sample_restore_begin, op1f_sample_restore_end: the restore unit, module/
//       build_r17_restore.cpp, at the start): between them a drop into that slot answers OP1F_NOT_NOW and `dropped`
//       reads 0
//
// `dropped` goes to 0 when a sound is loaded onto the slot: the engines unit counts the loads of each synth slot where
// the values unit hands it a sound loaded (its op1f_engines_assign with OP1F_KEPT_LOADED, a preset, a sampler's sound
// or the list's), and answers the count with the slot's ride (op1f_engines_ride). The count is kept with `dropped` at
// the drop's end (and at the restore's); a count that moved since makes `dropped` read 0.
//
// One drop at a time for the whole unit: one ticket, `free`, `queued` (armed, the post made), `running` (the job took
// it), `ended` (the job wrote the end); the frame says the end and frees it. Nothing waits; the shared words are single
// words or written in masked instants. The module links no runtime: nothing is divided. Compiled for the unit only;
// the checks it makes are module/build_r17_sample.h's, which the PC compiles for the tests.

#include "build_r6d_base.h"                            // base 29's words, the base's shared names, level 10's
#include "build_r17_sample.h"                          // R17: the checks as plain code

// R21 (base 43): a drum place too. op1f_sample_drop takes a drum place (OP1F_PLACE_DRUM + slot) when the ask is the
// card's drum engine (kind 12, module/build_r18_drum.cpp) and that drum sound rides it: the drum sampler page's own post
// (0x8f00c4e0, the drum's twin of the synth's) and, in the unit's turn, DD, the head of the worker's drum drop job
// (0x8f00ba38, request 10; the stub and its room are module/build_r17_sample.s), where the base's job makes the same
// four calls as the drum job of the firmware (0x8f00bac0 on): the drum slot's holder (0x8f014848), the import, the mark
// (0x8d4eb68a), the drum record's method 6. The unit keeps a drum sampler's sample over a restart itself (the mark),
// so the restore is the synth's alone. op1f_sample_get answers a drum place's `dropped`, `ticket`, `done` and `answer`
// as a synth place's, its loads counted by the drum unit.
#ifndef OP1F_BUILD_R18_DRUM_H
enum { OP1F_ASK_DRUM_ENGINE = 12 };
#endif

typedef unsigned int u32;
typedef unsigned char u8;

// Base 39's words (module/build_r6e_card.h, "Base 39"), named here where that header is not read, as the cards unit does.
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_BUSY = -9, OP1F_EMPTY = -10 };
struct Op1fSample {
    unsigned bytes;
    const short* data;
    unsigned stereo, room, capacity, dropped, ticket, done;
    int answer;
};
#endif
static_assert(sizeof(Op1fSample) == 36, "a slot's sample as the header has it, 36 bytes at base 39");
static_assert((int)Op1fSampleAnswer::NOT_NOW == (int)OP1F_NOT_NOW && (int)Op1fSampleAnswer::BAD_NAME == (int)OP1F_BAD_NAME &&
              (int)Op1fSampleAnswer::FAILED == (int)OP1F_FAILED && (int)Op1fSampleAnswer::NOT_HERE == (int)OP1F_NOT_HERE &&
              (int)Op1fSampleAnswer::BUSY == (int)OP1F_BUSY && (int)Op1fSampleAnswer::EMPTY == (int)OP1F_EMPTY &&
              (int)Op1fSampleAnswer::TOO_LARGE == (int)OP1F_TOO_LARGE, "the checks' answers are the header's");

extern "C" int op1f_sample_drop(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_sample_drop");
extern "C" int op1f_sample_get(const Op1fCard* card, unsigned place, Op1fSample* out) OP1F_NAMED("op1f_sample_get");
extern "C" int op1f_sample_changed(const Op1fCard* card, unsigned ask, unsigned place) OP1F_NAMED("op1f_sample_changed");
extern "C" const unsigned op1f_sample_1 OP1F_NAMED("op1f_sample_1");
extern "C" void op1f_sample_job(unsigned slot, unsigned clip) OP1F_NAMED("op1f_sample_job");
extern "C" void op1f_sample_drum_job(unsigned slot, unsigned clip) OP1F_NAMED("op1f_sample_drum_job");
extern "C" void op1f_sample_frame() OP1F_NAMED("op1f_sample_frame");
extern "C" int op1f_sample_restore_begin(unsigned slot) OP1F_NAMED("op1f_sample_restore_begin");
extern "C" void op1f_sample_restore_end(unsigned slot, unsigned frames) OP1F_NAMED("op1f_sample_restore_end");

// The stub's two words (module/build_r17_sample.s): the slot + 1 a ticket is queued for (0: none), which the stub
// compares with the job's slot; the jobs of the unit's own that passed SD, counted by the stub (and here, for a job
// that came while a ticket was armed for another moment), the worker's alone.
extern "C" volatile unsigned op1f_sample_sd_armed OP1F_NAMED("op1f_sample_sd_armed");
extern "C" volatile unsigned op1f_sample_sd_passes OP1F_NAMED("op1f_sample_sd_passes");
// SD's room: the job's first eight bytes, which the loader copies from the running firmware, and a jump back behind
// them. A call of it is stock's job as it always was.
extern "C" u8 op1f_sample_sd_room[] OP1F_NAMED("op1f_sample_sd_room");
// R21: DD's twins of the three, for the drum's job.
extern "C" volatile unsigned op1f_sample_dd_armed OP1F_NAMED("op1f_sample_dd_armed");
extern "C" volatile unsigned op1f_sample_dd_passes OP1F_NAMED("op1f_sample_dd_passes");
extern "C" u8 op1f_sample_dd_room[] OP1F_NAMED("op1f_sample_dd_room");

// ---- from the cards unit ----
// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
// The card's slot + 1 while the base calls that card's frame function on the caller's stack, else 0.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
// 1 when the nearest of the base's calls into packs around the caller is an input call of this card, for an event on
// the key thread; 0 otherwise.
extern "C" int op1f_cards_input_keyed(const Op1fCard* card) OP1F_NAMED("op1f_cards_input_keyed");

// ---- from the engines unit, weak: away, no slot rides an engine ----
// Synth slot `slot`'s ride: 0 none; 1 with `card` (the card's slot) and `ask`; 2 the same while a sound loaded there is
// not told yet. `loads`: the sounds loaded onto the slot since the start.
extern "C" unsigned op1f_engines_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* loads) OP1F_NAMED("op1f_engines_ride") __attribute__((weak));
// R21: drum slot `slot`'s ride, the drum unit's, in the same words. Weak: away, no drum sound rides an engine.
extern "C" unsigned op1f_drum_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* loads) OP1F_NAMED("op1f_drum_ride") __attribute__((weak));

// ---- for the restore, weak: away, nothing is kept ----
// A drop that rendered, from the frame that says its end: the slot, the frames rendered and the channels (1 stereo).
// It only sets words.
extern "C" void op1f_sample_kept(unsigned slot, unsigned frames, unsigned stereo) OP1F_NAMED("op1f_sample_kept") __attribute__((weak));
// Its frame work (module/build_r17_restore.cpp): the put-back at the start, a kept cut ended by a sound's load.
extern "C" void op1f_sample_restore_frame() OP1F_NAMED("op1f_sample_restore_frame") __attribute__((weak));

#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define SAMPLE_APART __attribute__((noinline))

// The firmware's words.
enum : u32 {
    DISK_OPEN = 0x84d25bccu,            // not 0 while the disk mode is open
    QUEUE_READ = 0x84bdd930u,           // the worker's queue (0x84bdd8a8): its read place, +0x88
    QUEUE_WRITE = 0x84bdd934u,          // its write place, +0x8c: a post moves it; the two are the same when the queue is empty
    PATCH_SIZE = 0x84bdda00u,           // the clipboard's word of a copied sound's size: not 0 while one is held
    CLIP_LENGTH = 0x84bdd9e4u,          // the clipboard's length in frames: a drop puts in at most the slot's capacity
    TRACKS_BEGIN = 0x84bdd948u,         // the vector of lifted tracks, a word a track: its begin, then its end
    TRACKS_END = 0x84bdd94cu,
    SYNTH_HOLDERS = 0x0802f73cu,        // a synth slot's holder, a word a slot: its first word is the slot's object
    DRUM_HOLDERS = 0x0802f0acu,         // a drum slot's
    SYNTH_POOLS = 0x840b0becu,          // synth slot 0's pool; slot i's SYNTH_STRIDE x i further
    DRUM_POOLS = 0x8d518598u,           // drum slot 0's pool
    SYNTH_RECORDS = 0x080235dcu,        // a synth slot's record, 52 bytes a slot: its table at +0, method 6 the table's +0x18
    DRUM_RECORDS = 0x0802377cu,         // R21: a drum slot's, the same form
    IMPORT_ENTRY = 0x8f004204u          // the clipboard's import into an object: its entry, `8 tracks`' place too
};
enum : u32 {
    SYNTH_STRIDE = 0x102660u,           // 529,200 halfwords
    DRUM_STRIDE = 0x35d540u,            // 1,764,000 halfwords
    CAPACITY_AT = 0xd768u,              // the object's frames
    CHANNELS_AT = 0xd770u,              // its channels byte: 0 mono
    RECORD_BYTES = 52u,
    METHOD_6 = 0x18u
};

typedef void (*PostFn)(u32 slot);                      // 0x8f00b564: the synth drop's poster, for the slot: it may post nothing (the write place does not move)
typedef u32 (*HolderFn)(u32 slot);                     // 0x8f01aea4: the slot's holder (the table's word)
typedef u32 (*ItemsFn)();                              // 0x8f00b78c: not 0 when the clipboard holds more than one item
typedef void (*ImportFn)(u32 object);                  // 0x8f004204: the clipboard rendered into the object's sample
typedef void (*MarkFn)(u32 slot);                      // 0x8d4eb6b2: the mark: the slot's sample kept over a restart, as the unit keeps its sampler's
typedef void (*MethodFn)(u32 record);                  // the record's method 6, called with the record
typedef void (*JobFn)(u32 slot, u32 clip);             // SD's room: stock's job
#define STOCK_POST ((PostFn)0x8f00b564u)
#define STOCK_HOLDER ((HolderFn)0x8f01aea4u)
#define STOCK_ITEMS ((ItemsFn)0x8f00b78cu)
#define STOCK_IMPORT ((ImportFn)IMPORT_ENTRY)
#define STOCK_MARK ((MarkFn)0x8d4eb6b2u)
#define ROOM_JOB ((JobFn)(void*)op1f_sample_sd_room)
#define STOCK_DRUM_POST ((PostFn)0x8f00c4e0u)              // R21: the drum page's post of the drum's drop, for the slot
#define STOCK_DRUM_HOLDER ((HolderFn)0x8f014848u)          // R21: a drum slot's holder
#define STOCK_DRUM_MARK ((MarkFn)0x8d4eb68au)              // R21: the drum slot's mark
#define ROOM_DRUM_JOB ((JobFn)(void*)op1f_sample_dd_room)

static const char LOG_FILE[] = "../src/main/op1f_sample.cpp";

enum { SLOTS = 8, NAME = OP1F_KIND_NAME, CARD_NAME = 16, CARDS = OP1F_SLOTS_MOST, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES };
enum { FREE = 0, QUEUED = 1, RUNNING = 2, ENDED = 3 };
enum { REFUSALS = 8, REFUSAL_LINES = 32, AGAIN_FRAMES = 256 };
static const u32 TICKET_MOST = 0x7fffffffu;            // a ticket is answered as a positive int

// The stub's words, zeroed at the load.
volatile unsigned op1f_sample_sd_armed;
volatile unsigned op1f_sample_sd_passes;
volatile unsigned op1f_sample_dd_armed;                // R21
volatile unsigned op1f_sample_dd_passes;

// The mark of a base whose packs may drop a lifted cut into their engine's sound and read any sound's sample.
const unsigned op1f_sample_1 = 1;

// The one drop under way. Written by the call (arming, disarming), the job (running, ended) and the frame (freed), each
// in a masked instant.
static struct {
    volatile u32 state;                                // FREE, QUEUED, RUNNING, ENDED
    volatile u32 number;                               // the ticket
    volatile u32 slot, card, ask;                      // the slot (0 to 7), the card's slot, the ask's number
    volatile u32 drum;                                 // R21: 1 a drum slot
    volatile u32 mark;                                 // R21: 1 the mark alone (op1f_sample_changed), no import
    volatile u32 at_frame;                             // the unit's frames counted here when it was queued
    volatile u32 said;                                 // the frame said its "queued" line
    volatile u32 why;                                  // the end: 0 rendered, else why the unit's turn refused it
    volatile u32 frames, stereo;                       // the end: the frames rendered, the channels
} ticket;
static volatile u32 last_number;                       // the last ticket handed, from 1
// The ticket whose request the call saw the queue take (its write place moved), 0 from the arming
// until then. The frame holds it with the queue's places and the ticket's state (op1f_sample_lost).
static volatile u32 posted;
static_assert((int)FREE == (int)OP1F_SAMPLE_FREE && (int)QUEUED == (int)OP1F_SAMPLE_QUEUED && (int)RUNNING == (int)OP1F_SAMPLE_RUNNING &&
              (int)ENDED == (int)OP1F_SAMPLE_ENDED, "the ticket's states are the header's");

// A synth slot's words, for op1f_sample_get.
struct Place {
    volatile u32 dropped;                              // frames of the last drop, or put back at the start
    volatile u32 loads;                                // the engines unit's count of loads when `dropped` was set
    volatile u32 done;                                 // the ticket of the last drop that ended here
    volatile int answer;                               // how it ended
    volatile u32 restoring;                            // the restore puts its sample back now
};
static Place places[2 * SLOTS];                         // R21: the synth's eight, then the drum's

// The call's refusals, for the frame to say: a ring, its records counted from 1. A refusal like the newest record (the
// same card, ask, place and why) is counted on it, not recorded again.
struct Refusal { u32 seq, card, ask, place, why, a, b, again; };
static Refusal refusals[REFUSALS];
static volatile u32 refusals_written;

static volatile u32 frames_seen;                       // op1f_sample_frame's calls: the unit's frames since the start

static struct {
    u32 next;                                          // the next record to say, from 1
    u32 last_seq, last_again;                          // the record said last and its repeats said
    u32 lines;                                         // refusal lines said this start
    u32 passes_said;                                   // the unit's own drops last said
} unit;

// ---------------------------------------------------------------- small helpers

static int slot_of(const Op1fCard* card) {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    if (card == 0) return -1;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards_2.slots[i].card == card) return op1f_cards_verdict(i) == 0 ? (int)i : -1;
    return -1;
}

// A word that may stand for one of the firmware's objects: on a word, in L2 (0x08......) or in DDR (0x8.......).
static int plausible(u32 at) {
    return (at & 3u) == 0 && ((at >> 24) == 8u || (at >> 28) == 8u);
}

// The object of a synth slot (`drum` 0) or a drum slot: the holder table's word, then the holder's first word; 0 when
// either is not one.
static u32 object_of(u32 drum, u32 slot) {
    u32 holder = RD32((drum ? (u32)DRUM_HOLDERS : (u32)SYNTH_HOLDERS) + 4u * slot);
    if (!plausible(holder)) return 0;
    u32 object = RD32(holder);
    return plausible(object) ? object : 0u;
}

static u32 pool_of(u32 drum, u32 slot) {
    return drum ? (u32)DRUM_POOLS + (u32)DRUM_STRIDE * slot : (u32)SYNTH_POOLS + (u32)SYNTH_STRIDE * slot;
}

// The slot's ride against the card's ask, as the checks take it; and the engines unit's count of its loads.
static u32 ride_of(u32 slot, u32 card, u32 ask, u32* loads, u32 drum = 0) {
    u32 c = 0, a = 0, l = 0, r = 0;
    if (drum) { if (&op1f_drum_ride != 0) r = op1f_drum_ride(slot, &c, &a, &l); }   // R21
    else if (&op1f_engines_ride != 0) r = op1f_engines_ride(slot, &c, &a, &l);
    if (loads) *loads = l;
    if (r == 0 || c != card || a != ask) return OP1F_SAMPLE_RIDES_NOT;
    return r == 2u ? (u32)OP1F_SAMPLE_RIDES_UNTOLD : (u32)OP1F_SAMPLE_RIDES;
}

// The card's ask of that number, when it is of kind engine: its table; else 0.
static const Op1fEngine* engine_ask(const Op1fCard* card, u32 ask) {
    if (card->bytes < (u32)CARD_4_BYTES || card->asks == 0 || ask >= card->ask_count) return 0;
    const Op1fAsk* a = &card->asks[ask];
    return a->kind == (u32)OP1F_ASK_ENGINE ? (const Op1fEngine*)a->what : 0;
}

// R21: the card's ask of that number, when it is of kind drum engine: its head; else 0.
static const Op1fKindHead* drum_ask(const Op1fCard* card, u32 ask) {
    if (card->bytes < (u32)CARD_4_BYTES || card->asks == 0 || ask >= card->ask_count) return 0;
    const Op1fAsk* a = &card->asks[ask];
    return a->kind == (u32)OP1F_ASK_DRUM_ENGINE ? (const Op1fKindHead*)a->what : 0;
}

static void copy_name(char* to, const char* from, u32 room) {
    u32 i = 0;
    for (; from && i + 1u < room && from[i] != 0; ++i) to[i] = from[i];
    to[i] = 0;
}

// What the checks read: the firmware's words and the base's. Only as far as the checks will look: a call from
// elsewhere than an input function, or for a place of no synth sound, reads nothing of the clipboard or the slot.
static void scene_of(Op1fSampleScene* s, u32 turn, u32 keyed, u32 place, u32 card, u32 ask, u32 engine) {
    u8* bytes = (u8*)s;
    for (u32 i = 0; i < (u32)sizeof(Op1fSampleScene); ++i) bytes[i] = 0;
    s->tracks = 0;
    s->tracks_end = 0;
    s->turn = turn;
    s->keyed = keyed;
    s->place = place;
    s->engine = engine;
    s->drums = 1;                                                                   // R21: base 43 drops into drum sounds too
    s->disk = RD32(DISK_OPEN) != 0 ? 1u : 0u;
    if ((!turn && !keyed) || s->disk || place >= 2u * (u32)SLOTS) return;
    const u32 drum = place >= (u32)SLOTS ? 1u : 0u, slot = place & 7u;
    s->restoring = places[place].restoring;
    s->ride = ride_of(slot, card, ask, 0, drum);
    s->ticket = ticket.state != (u32)FREE ? 1u : 0u;
    s->queue_read = RD32(QUEUE_READ);
    s->queue_write = RD32(QUEUE_WRITE);
    s->many = STOCK_ITEMS();
    s->patch = RD32(PATCH_SIZE);
    s->tracks = (const unsigned*)RD32(TRACKS_BEGIN);
    s->tracks_end = (const unsigned*)RD32(TRACKS_END);
    for (u32 i = 0; i < 8u; ++i) s->head[i] = RD8(IMPORT_ENTRY + i);
    u32 object = object_of(drum, slot);
    s->pool = pool_of(drum, slot);
    if (object) {
        s->data = RD32(object);
        s->capacity = RD32(object + (u32)CAPACITY_AT);
        s->stereo = RD8(object + (u32)CHANNELS_AT) != 0 ? 1u : 0u;
    }
    s->length = RD32(CLIP_LENGTH);
}

// A refusal of the call noted for the frame's line, in one masked instant.
static int refused(int slot, u32 ask, u32 place, u32 why, u32 a, u32 b) {
    u32 mask, card = slot < 0 ? 0xffu : (u32)slot;
    MASK_OFF(mask);
    u32 n = refusals_written;
    Refusal* last = n != 0 ? &refusals[(n - 1u) & ((u32)REFUSALS - 1u)] : 0;
    if (last && last->card == card && last->ask == ask && last->place == place && last->why == why) {
        last->again = last->again + 1u;
    } else {
        Refusal* r = &refusals[n & ((u32)REFUSALS - 1u)];
        r->seq = n + 1u;
        r->card = card;
        r->ask = ask;
        r->place = place;
        r->why = why;
        r->a = a;
        r->b = b;
        r->again = 0;
        refusals_written = n + 1u;
    }
    MASK_ON(mask);
    return op1f_sample_answer(why);
}

// ---------------------------------------------------------------- the call

extern "C" SAMPLE_APART int op1f_sample_drop(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    u32 keyed = slot >= 0 && op1f_cards_input_keyed(card) ? 1u : 0u;
    u32 engine = keyed && (place >= (u32)SLOTS && place < 2u * (u32)SLOTS ? drum_ask(card, ask) != 0 : engine_ask(card, ask) != 0) ? 1u : 0u;   // R21
    Op1fSampleScene s;
    scene_of(&s, 0, keyed, place, (u32)slot, ask, engine);
    u32 why = op1f_sample_check(&s);
    if (why == (u32)OP1F_SAMPLE_LARGE) return refused(slot, ask, place, why, op1f_sample_frames(s.length, s.capacity),
                                                        place >= (u32)SLOTS ? (u32)OP1F_SAMPLE_DRUM_POOL >> (s.stereo ? 1 : 0) : op1f_sample_room(s.stereo));   // R21
    if (why != (u32)OP1F_SAMPLE_PASSED) return refused(slot, ask, place, why, 0, 0);
    u32 mask, number = 0, before = 0, busy = 0;
    MASK_OFF(mask);                                                                 // the ticket armed, the queue and the restore looked at again in the same instant
    if (ticket.state != (u32)FREE) busy = OP1F_SAMPLE_TICKET;
    else if (RD32(QUEUE_READ) != RD32(QUEUE_WRITE)) busy = OP1F_SAMPLE_QUEUE;
    else if (places[place].restoring) busy = OP1F_SAMPLE_RESTORING;
    else {
        number = last_number + 1u;
        if (number > TICKET_MOST) number = 1u;
        last_number = number;
        ticket.number = number;
        ticket.slot = place & 7u;
        ticket.drum = place >= (u32)SLOTS ? 1u : 0u;                                // R21
        ticket.mark = 0;
        ticket.card = (u32)slot;
        ticket.ask = ask;
        ticket.at_frame = frames_seen;
        ticket.said = 0;
        ticket.why = 0;
        ticket.frames = 0;
        ticket.stereo = s.stereo;
        ticket.state = QUEUED;
        posted = 0;                                                                 // not posted yet
        if (place >= (u32)SLOTS) op1f_sample_dd_armed = (place & 7u) + 1u;           // R21: what DD's stub compares
        else op1f_sample_sd_armed = place + 1u;                                     // what the stub compares with the job's slot
        before = RD32(QUEUE_WRITE);
    }
    MASK_ON(mask);
    if (busy) return refused(slot, ask, place, busy, 0, 0);
    if (place >= (u32)SLOTS) STOCK_DRUM_POST(place & 7u);                          // R21: the drum page's own post
    else STOCK_POST(place);                                                         // the unit's own post, out of the mask, on the key thread
    if (RD32(QUEUE_WRITE) == before) {                                              // nothing was posted: the ticket goes
        MASK_OFF(mask);
        if (ticket.state == (u32)QUEUED && ticket.number == number) {
            ticket.state = FREE;
            op1f_sample_sd_armed = 0;
            op1f_sample_dd_armed = 0;                                               // R21
        }
        MASK_ON(mask);
        return refused(slot, ask, place, OP1F_SAMPLE_POST, 0, 0);
    }
    posted = number;                                                                // the queue took it; from here the frame watches for its loss
    return (int)number;
}

// R21 (base 43): the drum mark alone, for a pack that changed its drum sound's sample itself (op1f_sample_changed):
// from the card's frame function, or its input function for an event on the key thread; a drum place that rides the
// card's drum engine of that ask. The same ticket, post and DD as a drop; in the unit's turn the drum job's mark
// (0x8d4eb68a) and the record's method 6, and no import: the unit then keeps the drum sampler's sample as after a drop.
extern "C" SAMPLE_APART int op1f_sample_changed(const Op1fCard* card, unsigned ask, unsigned place) {
    int slot = slot_of(card);
    u32 why = 0;
    if (slot < 0) return OP1F_BAD_NAME;
    u32 calling = op1f_cards_calling() == (u32)slot + 1u || op1f_cards_input_keyed(card) ? 1u : 0u;
    if (!calling) why = OP1F_SAMPLE_NOT_INPUT;
    else if (RD32(DISK_OPEN) != 0) why = OP1F_SAMPLE_DISK;
    else if (place < (u32)SLOTS || place >= 2u * (u32)SLOTS) why = OP1F_SAMPLE_NO_PLACE;
    else if (drum_ask(card, ask) == 0) why = OP1F_SAMPLE_NO_ASK;
    else {
        u32 r = ride_of(place & 7u, (u32)slot, ask, 0, 1);
        if (r == (u32)OP1F_SAMPLE_RIDES_UNTOLD) why = OP1F_SAMPLE_UNTOLD;
        else if (r != (u32)OP1F_SAMPLE_RIDES) why = OP1F_SAMPLE_NOT_RIDING;
    }
    if (why) return refused(slot, ask, place, why, 0, 0);
    u32 mask, number = 0, before = 0, busy = 0;
    MASK_OFF(mask);
    if (ticket.state != (u32)FREE) busy = OP1F_SAMPLE_TICKET;
    else if (RD32(QUEUE_READ) != RD32(QUEUE_WRITE)) busy = OP1F_SAMPLE_QUEUE;
    else {
        number = last_number + 1u;
        if (number > TICKET_MOST) number = 1u;
        last_number = number;
        ticket.number = number;
        ticket.slot = place & 7u;
        ticket.drum = 1;
        ticket.mark = 1;
        ticket.card = (u32)slot;
        ticket.ask = ask;
        ticket.at_frame = frames_seen;
        ticket.said = 0;
        ticket.why = 0;
        ticket.frames = 0;
        ticket.stereo = 0;
        ticket.state = QUEUED;
        posted = 0;
        op1f_sample_dd_armed = (place & 7u) + 1u;
        before = RD32(QUEUE_WRITE);
    }
    MASK_ON(mask);
    if (busy) return refused(slot, ask, place, busy, 0, 0);
    STOCK_DRUM_POST(place & 7u);                                                    // the drum page's own post: DD takes it
    if (RD32(QUEUE_WRITE) == before) {
        MASK_OFF(mask);
        if (ticket.state == (u32)QUEUED && ticket.number == number) {
            ticket.state = FREE;
            op1f_sample_dd_armed = 0;
        }
        MASK_ON(mask);
        return refused(slot, ask, place, OP1F_SAMPLE_POST, 0, 0);
    }
    posted = number;
    return (int)number;
}

// ---------------------------------------------------------------- the unit's turn

// From SD's stub, on the worker, for a job whose slot a ticket was armed for: R0 the slot, R1 the clipboard, as the
// worker's dispatch handed them to the job.
extern "C" SAMPLE_APART void op1f_sample_job(unsigned slot, unsigned clip) {
    u32 mask, mine = 0, card = 0, ask = 0;
    MASK_OFF(mask);
    if (ticket.state == (u32)QUEUED && ticket.slot == slot && !ticket.drum) {
        ticket.state = RUNNING;
        op1f_sample_sd_armed = 0;
        card = ticket.card;
        ask = ticket.ask;
        mine = 1;
    }
    MASK_ON(mask);
    if (!mine) {                                                                    // not the ticket's: stock's job, as the stub's own way
        op1f_sample_sd_passes = op1f_sample_sd_passes + 1u;
        ROOM_JOB(slot, clip);
        return;
    }
    Op1fSampleScene s;
    scene_of(&s, 1, 1, slot, card, ask, 1);
    u32 why = op1f_sample_check(&s);
    u32 frames = 0;
    if (why == (u32)OP1F_SAMPLE_PASSED) {                                           // the drop: the firmware's four calls
        frames = op1f_sample_frames(s.length, s.capacity);
        u32 holder = STOCK_HOLDER(slot);
        STOCK_IMPORT(RD32(holder));
        STOCK_MARK(slot);
        u32 record = (u32)SYNTH_RECORDS + (u32)RECORD_BYTES * slot;
        ((MethodFn)RD32(RD32(record) + (u32)METHOD_6))(record);
    }
    u32 loads = 0;
    ride_of(slot, card, ask, &loads);
    MASK_OFF(mask);
    Place* p = &places[slot];
    p->answer = why == (u32)OP1F_SAMPLE_PASSED ? (int)frames : op1f_sample_answer(why);
    if (why == (u32)OP1F_SAMPLE_PASSED) {
        p->dropped = frames;
        p->loads = loads;
    }
    p->done = ticket.number;
    ticket.why = why;
    ticket.frames = frames;
    ticket.stereo = s.stereo;
    ticket.state = ENDED;
    MASK_ON(mask);
}

// R21: from DD's stub, on the worker, for a drum job whose slot a ticket was armed for: R0 the drum slot, R1 the
// clipboard. The checks again, then the firmware drum job's own four calls (0x8f00bac0 on), or a refusal.
extern "C" SAMPLE_APART void op1f_sample_drum_job(unsigned slot, unsigned clip) {
    u32 mask, mine = 0, card = 0, ask = 0;
    MASK_OFF(mask);
    if (ticket.state == (u32)QUEUED && ticket.slot == slot && ticket.drum) {
        ticket.state = RUNNING;
        op1f_sample_dd_armed = 0;
        card = ticket.card;
        ask = ticket.ask;
        mine = 1;
    }
    MASK_ON(mask);
    if (!mine) {                                                                    // not the ticket's: stock's drum job
        op1f_sample_dd_passes = op1f_sample_dd_passes + 1u;
        ROOM_DRUM_JOB(slot, clip);
        return;
    }
    if (ticket.mark) {                                                              // the mark alone: the sample changed by the pack
        u32 why = ride_of(slot, card, ask, 0, 1) == (u32)OP1F_SAMPLE_RIDES ? (u32)OP1F_SAMPLE_PASSED : (u32)OP1F_SAMPLE_NOT_RIDING;
        if (why == (u32)OP1F_SAMPLE_PASSED) {
            STOCK_DRUM_MARK(slot);
            u32 record = (u32)DRUM_RECORDS + (u32)RECORD_BYTES * slot;
            ((MethodFn)RD32(RD32(record) + (u32)METHOD_6))(record);
        }
        MASK_OFF(mask);
        Place* p = &places[slot + (u32)SLOTS];
        p->answer = why == (u32)OP1F_SAMPLE_PASSED ? 0 : op1f_sample_answer(why);
        p->done = ticket.number;
        ticket.why = why;
        ticket.frames = 0;
        ticket.state = ENDED;
        MASK_ON(mask);
        return;
    }
    Op1fSampleScene s;
    scene_of(&s, 1, 1, slot + (u32)SLOTS, card, ask, 1);
    u32 why = op1f_sample_check(&s);
    u32 frames = 0;
    if (why == (u32)OP1F_SAMPLE_PASSED) {                                           // the drop: the firmware's four calls
        frames = op1f_sample_frames(s.length, s.capacity);
        u32 holder = STOCK_DRUM_HOLDER(slot);
        STOCK_IMPORT(RD32(holder));
        STOCK_DRUM_MARK(slot);
        u32 record = (u32)DRUM_RECORDS + (u32)RECORD_BYTES * slot;
        ((MethodFn)RD32(RD32(record) + (u32)METHOD_6))(record);
    }
    u32 loads = 0;
    ride_of(slot, card, ask, &loads, 1);
    MASK_OFF(mask);
    Place* p = &places[slot + (u32)SLOTS];
    p->answer = why == (u32)OP1F_SAMPLE_PASSED ? (int)frames : op1f_sample_answer(why);
    if (why == (u32)OP1F_SAMPLE_PASSED) {
        p->dropped = frames;
        p->loads = loads;
    }
    p->done = ticket.number;
    ticket.why = why;
    ticket.frames = frames;
    ticket.stereo = s.stereo;
    ticket.state = ENDED;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the read

extern "C" SAMPLE_APART int op1f_sample_get(const Op1fCard* card, unsigned place, Op1fSample* out) {
    int slot = slot_of(card);
    if (slot < 0 || out == 0 || place >= 16u) return OP1F_BAD_NAME;
    if (op1f_cards_calling() != (u32)slot + 1u) return OP1F_NOT_NOW;
    out->bytes = (u32)sizeof(Op1fSample);
    u32 drum = place >= (u32)SLOTS ? 1u : 0u, s = place & 7u;
    u32 object = object_of(drum, s);
    u32 data = object ? RD32(object) : 0u;
    if (data == 0 || data != pool_of(drum, s)) return OP1F_FAILED;
    u32 stereo = RD8(object + (u32)CHANNELS_AT) != 0 ? 1u : 0u;
    out->data = (const short*)data;
    out->stereo = stereo;
    out->room = (drum ? (u32)OP1F_SAMPLE_DRUM_POOL : (u32)OP1F_SAMPLE_POOL) >> stereo;
    out->capacity = RD32(object + (u32)CAPACITY_AT);
    u32 dropped = 0, number = 0, done = 0, loads = 0;
    int answer = 0;
    {                                                                               // R21: a drum place's as a synth place's
        ride_of(s, 0, 0, &loads, drum);
        u32 mask;
        MASK_OFF(mask);
        const Place* p = &places[place];
        dropped = p->restoring || p->loads != loads ? 0u : p->dropped;
        done = p->done;
        answer = p->answer;
        if ((ticket.state == (u32)QUEUED || ticket.state == (u32)RUNNING) && ticket.slot == s && ticket.drum == drum) number = ticket.number;
        MASK_ON(mask);
    }
    out->dropped = dropped;
    out->ticket = number;
    out->done = done;
    out->answer = answer;
    return 0;
}

// ---------------------------------------------------------------- the restore's calls

// Its sample is being put back: 1, or 0 when a drop into that slot is under way or not yet said (the restore asks again
// at a later frame).
extern "C" int op1f_sample_restore_begin(unsigned slot) {
    if (slot >= (u32)SLOTS) return 0;
    u32 mask, begun = 0;
    MASK_OFF(mask);
    if (ticket.state == (u32)FREE || ticket.slot != slot || ticket.drum) {
        places[slot].restoring = 1;
        places[slot].dropped = 0;
        begun = 1;
    }
    MASK_ON(mask);
    return (int)begun;
}

// Put back: `frames` (0 when nothing was) is the slot's `dropped` from now, until a sound is loaded there.
extern "C" void op1f_sample_restore_end(unsigned slot, unsigned frames) {
    if (slot >= (u32)SLOTS) return;
    u32 loads = 0, mask;
    ride_of(slot, 0, 0, &loads);
    MASK_OFF(mask);
    places[slot].dropped = frames;
    places[slot].loads = loads;
    places[slot].restoring = 0;
    MASK_ON(mask);
}

// ---------------------------------------------------------------- the frame's lines

static void place_words(u32 place, const char** what, u32* number) {
    if (place < (u32)SLOTS) { *what = "synth"; *number = place + 1u; }
    else if (place < 2u * (u32)SLOTS) { *what = "drum"; *number = place - (u32)SLOTS + 1u; }
    else { *what = "place"; *number = place; }
}

static void card_name(u32 card, char* name) {
    op1f_cards_lists();                                                             // R20 (base 42)
    const Op1fCard* c = card < (u32)CARDS && card < op1f_cards_2.count ? op1f_cards_2.slots[card].card : 0;
    copy_name(name, c ? c->name : 0, (u32)CARD_NAME + 1u);
}

static void engine_name(u32 card, u32 ask, char* name, u32 drum) {
    op1f_cards_lists();                                                             // R20 (base 42)
    const Op1fCard* c = card < (u32)CARDS && card < op1f_cards_2.count ? op1f_cards_2.slots[card].card : 0;
    if (drum) {                                                                     // R21
        const Op1fKindHead* h = c ? drum_ask(c, ask) : 0;
        copy_name(name, h ? h->name : 0, (u32)NAME + 1u);
        return;
    }
    const Op1fEngine* e = c ? engine_ask(c, ask) : 0;
    copy_name(name, e ? e->head.name : 0, (u32)NAME + 1u);
}

static int line_free() {
    if (unit.lines < (u32)REFUSAL_LINES) {
        ++unit.lines;
        return 1;
    }
    if (unit.lines == (u32)REFUSAL_LINES) {
        ++unit.lines;
        op1f_log(2, LOG_FILE, 1, "sample: more refusals are not said this start");
    }
    return 0;
}

static void say_refusals(u32 tick) {
    Refusal ring[REFUSALS];
    u32 mask, written;
    const u32* from = (const u32*)refusals;
    u32* to = (u32*)ring;
    MASK_OFF(mask);                                                                 // the ring copied whole in one instant, a word at a time (no runtime's copy)
    written = refusals_written;
    for (u32 i = 0; i < (u32)(sizeof refusals / 4u); ++i) to[i] = from[i];
    MASK_ON(mask);
    if (unit.next == 0) unit.next = 1;
    if (unit.last_seq != 0) {                                                       // the repeats of the record said last, once it is not the newest, or once a while
        const Refusal* r = &ring[(unit.last_seq - 1u) & ((u32)REFUSALS - 1u)];
        if (r->seq == unit.last_seq && r->again > unit.last_again && (written > unit.last_seq || tick)) {
            if (line_free()) op1f_log(2, LOG_FILE, 2, "sample: the refusal above came %u time(s) more", r->again - unit.last_again);
            unit.last_again = r->again;
        }
    }
    if (written >= unit.next + (u32)REFUSALS) {                                     // overwritten before a frame came
        u32 lost = written + 1u - (u32)REFUSALS - unit.next;
        if (line_free()) op1f_log(2, LOG_FILE, 3, "sample: %u refusal(s) came too fast to be said", lost);
        unit.next = written + 1u - (u32)REFUSALS;
    }
    for (; unit.next <= written; ++unit.next) {
        const Refusal* r = &ring[(unit.next - 1u) & ((u32)REFUSALS - 1u)];
        unit.last_seq = unit.next;
        unit.last_again = r->again;
        if (!line_free()) continue;
        char name[CARD_NAME + 1];
        const char* what;
        u32 number;
        card_name(r->card, name);
        place_words(r->place, &what, &number);
        int answer = op1f_sample_answer(r->why);
        if (r->why == (u32)OP1F_SAMPLE_LARGE)
            op1f_log(2, LOG_FILE, 4, "sample: '%s' asked a drop into %s %u (ask %u): %d, %s (the cut %u frames, the room %u)", name, what, number, r->ask,
                     answer, op1f_sample_why_words(r->why), r->a, r->b);
        else
            op1f_log(2, LOG_FILE, 5, "sample: '%s' asked a drop into %s %u (ask %u): %d, %s", name, what, number, r->ask, answer, op1f_sample_why_words(r->why));
        if (r->again) op1f_log(2, LOG_FILE, 6, "sample: the refusal above came %u time(s) more", r->again);
    }
}

// A request the queue lost (op1f_sample_lost) is ended here, in the masked instant it is read in:
// SD disarmed, the slot's `answer` OP1F_FAILED and `done` the ticket, the ticket freed; its line said once.
static void say_ticket() {
    u32 mask, state, number, slot, card, ask, at, said, why, frames, stereo, was_posted, read, write, lost, drum, mark;
    MASK_OFF(mask);
    was_posted = posted;                                                            // in this order: posted, the queue's places, the state
    read = RD32(QUEUE_READ);
    write = RD32(QUEUE_WRITE);
    state = ticket.state;
    number = ticket.number;
    slot = ticket.slot;
    drum = ticket.drum;
    mark = ticket.mark;
    card = ticket.card;
    ask = ticket.ask;
    at = ticket.at_frame;
    said = ticket.said;
    why = ticket.why;
    frames = ticket.frames;
    stereo = ticket.stereo;
    lost = state != (u32)FREE && op1f_sample_lost(was_posted, number, read, write, state) ? 1u : 0u;
    if (lost) {
        op1f_sample_sd_armed = 0;
        op1f_sample_dd_armed = 0;
        places[slot + (drum ? (u32)SLOTS : 0u)].answer = OP1F_FAILED;
        places[slot + (drum ? (u32)SLOTS : 0u)].done = number;
        ticket.state = FREE;
    }
    MASK_ON(mask);
    if (state == (u32)FREE) return;
    if (!said) {
        char name[NAME + 1];
        engine_name(card, ask, name, drum);
        op1f_log(3, LOG_FILE, 7, "sample: %s %u into %s %u for '%s': queued", mark ? "mark" : "drop", number, drum ? "drum" : "synth", slot + 1u, name);
        MASK_OFF(mask);
        if (ticket.number == number) ticket.said = 1;
        MASK_ON(mask);
    }
    if (lost) {
        op1f_log(2, LOG_FILE, 11, "sample: drop %u: lost from the unit's queue, not done", number);
        return;
    }
    if (state != (u32)ENDED) return;
    u32 after = frames_seen - at;
    if (why == (u32)OP1F_SAMPLE_PASSED && mark)
        op1f_log(3, LOG_FILE, 12, "sample: mark %u: drum %u marked changed after %u frame(s): the unit keeps its sample", number, slot + 1u, after);
    else if (why == (u32)OP1F_SAMPLE_PASSED)
        op1f_log(3, LOG_FILE, 8, "sample: drop %u: %u frames rendered (stereo %u, room %u) after %u frame(s)", number, frames, stereo,
                 drum ? (u32)OP1F_SAMPLE_DRUM_POOL >> (stereo ? 1 : 0) : op1f_sample_room(stereo), after);
    else
        op1f_log(2, LOG_FILE, 9, "sample: drop %u into %s %u refused in the unit's turn: %d, %s, after %u frame(s)", number, drum ? "drum" : "synth", slot + 1u,
                 op1f_sample_answer(why), op1f_sample_why_words(why), after);
    if (why == (u32)OP1F_SAMPLE_PASSED && !drum && !mark && &op1f_sample_kept != 0) op1f_sample_kept(slot, frames, stereo);   // R21: the unit keeps a drum's itself
    MASK_OFF(mask);
    if (ticket.state == (u32)ENDED && ticket.number == number) ticket.state = FREE;
    MASK_ON(mask);
}

// From the cards unit, once a frame on the display thread, before the frame functions.
extern "C" SAMPLE_APART void op1f_sample_frame() {
    u32 frame = frames_seen + 1u;
    frames_seen = frame;
    say_refusals((frame & ((u32)AGAIN_FRAMES - 1u)) == 0 ? 1u : 0u);
    say_ticket();
    u32 passes = op1f_sample_sd_passes + op1f_sample_dd_passes;                    // R21: the drum's too
    if (passes != unit.passes_said) {
        unit.passes_said = passes;
        op1f_log(3, LOG_FILE, 10, "sample: the unit's own drops into its samplers so far: %u", passes);
    }
    if (&op1f_sample_restore_frame != 0) op1f_sample_restore_frame();             // the restore's, after the drops' lines
}

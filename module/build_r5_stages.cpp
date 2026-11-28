// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R5, the base pack 28: the sound stages (OS 1.7.0+0). A sibling of base 24's stages unit (which
// stays as it was; its stubs, module/build_r4_stages.s, serve this unit as they are): everything that unit does stands
// here as it was, and one thing is added. Once an effect of a pack is registered (op1f_effects_registered, level 8),
// the places after the synth's, the drum's and the master's effect (0, 1 and 3) call the effects unit first, before
// level 6's stages there: op1f_effects_after(point, host, the two buffers), which calls an effect that keeps its tails
// while its place's effect is switched off (module/build_r5_base.h). So the word a stub reads at those places is not 0
// while an effect is registered, even with no stage there: it is the count of stages there and one more. The count of
// stages each place runs is the unit's own (`counts`), as base 24's word was. With no effect registered every word, every
// call and every line is base 24's.
//
// Base 45 (R24): a wave of an LFO layer (module/build_r6e_card.h, "Base 45") is called by the effects unit, at the
// synth's and the drum's places too when no effect of a pack is registered: once a wave is registered
// (op1f_lfos_waves, weak), places 0 and 1 call op1f_effects_after as an effect would have them, and their words say one
// more. With no wave registered every word, every call and every line is base 44's.
//
// What base 24's unit said of itself follows and holds:
//
// Step R4, the base pack (base.pack 24): the sound stages, level 6's first kind of ask (OS 1.7.0+0). A unit of its own,
// new at R4.
//
// Any pack may ask, through its card, for a function of its own in the sound's path (OP1F_ASK_STAGE, its table an
// Op1fStage, base 24's header for pack authors). Four places of the firmware's sound, each one instruction under a CALL.a of the
// base's table, go to the four stubs of module/build_r4_stages.s; a stub with stages at its place calls
// op1f_stages_run below with its place's number:
//   0  the synth's sound after its effect   host 0, point OP1F_AT_INSTRUMENT, the instrument's two buffers
//   1  the drum's sound after its effect    host 1, point OP1F_AT_INSTRUMENT, the same two buffers
//   2  the unit's sum before the master effect   host 2, point OP1F_AT_MASTER_IN, the master's two buffers
//   3  the unit's sum after the master effect    host 2, point OP1F_AT_MASTER_OUT, the same
// The buffers are the ones the firmware's effect at that point is handed: 64 words of 32 bits each, in place.
//
// The registration, once, at the base's first frame (op1f_stages_frame, from the cards unit, on the display thread):
// every card the base uses (op1f_cards_verdict) that says level 6 or later and is of level 4's size, in the list's
// order, is looked through for asks of this kind, each in its card's order. An ask is held to its form: its table's
// bytes at least an Op1fStage's and the same as its head says, its name one to fifteen characters of a-z, 0-9, '-'
// and '_' with its end, no flag but OP1F_STAGE_LISTENS, no `shown`, a point this base knows, `process` a function of
// its own pack (inside the pack, on two bytes), and `memory` 0 or naming an ask of kind OP1F_ASK_MEMORY of the same
// card. Then its cost: 1 to OP1F_STAGE_CYCLES_MOST a call, and all stages' together at most OP1F_STAGE_CYCLES_ALL a
// block, a stage at the instrument counted twice (it is called for the synth and for the drum). An ask not in order,
// or beyond a limit, or the seventeenth, is left aside with a line, and its card is still used. A stage that names a
// memory waits for it: the memory unit says when it is handed over (op1f_memory_state), and a stage whose memory was
// refused is left aside with a line, at the frame the refusal is known.
//
// The order at a place: first the stages that change the sound, in the order of the registration, then those that
// only listen, in the same order. The table of each place is written whole before the word that says how many stand
// there (op1f_stages_at, a word a place), which the stubs read: until it is written, a stub costs a load and a test.
//
// A block, on the audio thread: each stage that runs is handed an Op1fBlock of its own (the place's host and point, 64
// frames, the two buffers; for a stage that listens, a copy of the two made just before its call, so that nothing it
// writes is heard; the clock's period word and the transport word as the firmware has them now; the stage's count of
// calls at that host), and is timed by the processor's cycle counter. Its counts of the period (calls, cycles in all,
// the most, the calls over what it declared) are kept in words only the audio thread writes and the display thread
// reads and clears with interrupts masked. A stage is switched off, until the next start, when one call takes more
// than OP1F_STAGE_CYCLES_OFF, or when OP1F_STAGE_OVER_CALLS of its calls within one second (by the cycle counter, at
// 400,000,000 a second) go over: take more than twice what it declared and more than what it declared and
// OP1F_STAGE_CYCLES_SPARE (the time measured includes the interrupts served during the call, and the allowance keeps
// a light stage from being blamed for them); from that call on it is passed by, and the display thread says one line
// at its next frame, which says which of the two rules switched it off. Nothing on this way says a line, waits, allocates, masks, divides or calls
// anything of the firmware's.
//
// The display thread, once a frame: a stage waiting for its memory begins when the memory is handed over; a stage
// switched off has its line; and each period of 256 frames one line for each stage that was called, through a quiet
// gate of this unit's own: the gate's signature is what the lines state (each stage's state and whether any call went
// over), so that a unit whose stages keep to their costs says the lines at the gate's slow pace.
//
// op1f_stage(card, ask) answers from any thread whether the base calls that stage.
//
// Nothing is divided by a number that is not a power of two (the module links no runtime): the period line's average
// is made by shifts and subtractions. Compiled for the PC as it is, with OP1F_HOST defined: the stand-ins come in
// through a header of the PC's own (for tests this tree does not hold).

#include "build_r5_base.h"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"     // the gate's half for the power popup: this unit is handed no frame to see it by
#endif
#include "build_q_quiet.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

typedef unsigned int u32;
typedef unsigned char u8;

#ifdef OP1F_HOST
#include "mock_stages_r4.h"                            // the PC's stand-ins: the words read, the buffers, the cycle counter, the masks
#define BUFFER(a) mock_buffer(a)
#define MASK_OFF(mask) ((void)((mask) = mock_mask()))
#define MASK_ON(mask) mock_unmask(mask)
#define BARRIER() ((void)0)                            // the PC's run is one thread at a time
static inline u32 cycles_now() { return mock_cycles(); }
#else
#define RD32(a) (*(volatile u32*)(a))
#define BUFFER(a) ((int*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
// The compiler moves no load or store across this: a table is stored before the word that says how much of it there
// is, and read after it.
#define BARRIER() asm volatile("" : : : "memory")
static inline u32 cycles_now() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}
#endif

// ---- from the cards unit, and from the memory unit (base 24's memory unit) ----

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
// 0 when the base uses the card of a slot of op1f_cards_2, not 0 when it does not. From any thread; no line.
extern "C" int op1f_cards_verdict(unsigned slot) OP1F_NAMED("op1f_cards_verdict");
// What became of the memory a card's ask asked for: 1 handed over, 0 not yet (or the memory unit has not looked at
// the asks yet), -1 never (refused, left aside, or no such ask). From any thread; no line.
extern "C" int op1f_memory_state(unsigned slot, unsigned ask) OP1F_NAMED("op1f_memory_state");

// ---- the firmware's words ----

static const u32 PERIOD = 0x84d20e80u;                 // the clock's period: samples a 96th of a beat at 44,100 a second, times 2^18
static const u32 TRANSPORT = 0x11905a04u;              // not 0 while the tape plays
static const u32 INSTRUMENT_LEFT = 0x11902ba0u;        // the instrument's two buffers after its effect, 64 words each
static const u32 INSTRUMENT_RIGHT = 0x11902ca0u;
static const u32 MASTER_LEFT = 0x119027a0u;            // the master's two, before and after its effect
static const u32 MASTER_RIGHT = 0x119028a0u;

enum { MOST = OP1F_STAGES_MOST, PLACES = 4, CARDS = OP1F_SLOTS_MOST, NAME = OP1F_KIND_NAME, TABLE_BYTES = 44, BLOCK_BYTES = 36,
       CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, FRAMES = 64, CYCLES_MOST = OP1F_STAGE_CYCLES_MOST, CYCLES_ALL = OP1F_STAGE_CYCLES_ALL,
       PERIOD_FRAMES = 256 };
static const u32 CYCLES_OFF = (u32)OP1F_STAGE_CYCLES_OFF, OVER_CALLS = (u32)OP1F_STAGE_OVER_CALLS, SPARE = (u32)OP1F_STAGE_CYCLES_SPARE;
static const u32 SECOND = 400000000u;                  // the processor's cycles in a second
static_assert(sizeof(Op1fStage) == TABLE_BYTES && sizeof(Op1fBlock) == BLOCK_BYTES, "a stage's table is 44 bytes and its block 36 at level 6");

static const char LOG_FILE[] = "../src/main/op1f_stages.cpp";

// A stage's state, in one word: the stubs' way reads it at every call.
enum { RUNNING = 1, WAITING = 2, OFF = 3, ASIDE = 4 };

struct Stage {
    void (*process)(const Op1fBlock* now);
    u32 slot, ask;                                     // the card's slot in op1f_cards_2, the ask's number in its list
    u32 point, cycles, listens;
    u32 memory;                                        // the memory ask's number + 1, 0 for none
    volatile u32 state;
    u32 calls_at[2];                                   // its calls at each host (the synth and the drum at the instrument; 0 at the master)
    u32 window_start, window_over, window_most;        // the second in which calls over twice its cost are counted
    volatile u32 off_cycles;                           // the call (or the second's most) that switched it off
    volatile u32 off_overs;                            // 0 when one call switched it off, else the calls over in its second
    u32 off_said;
    volatile u32 calls, sum, most, over;               // this period's, written by the audio thread, read and cleared masked
    char name[NAME];
};

static Stage stages[MOST];
static u8 order[PLACES][MOST];                         // each place's stages, by their number in `stages`
static int listening_left[FRAMES], listening_right[FRAMES];   // the copy a stage that listens is handed

// How many stages stand at each place: 0 until the registration, then written once. The stubs read it first. R5: and one
// more at the places after the three effects (0, 1, 3) while an effect of a pack is registered.
extern "C" volatile u32 op1f_stages_at[PLACES] OP1F_NAMED("op1f_stages_at");
volatile u32 op1f_stages_at[PLACES] = {0, 0, 0, 0};
static u32 counts[PLACES];                             // R5: how many stages each place runs

static struct {
    u32 enrolled;                                      // the registration was made
    u32 count;                                         // stages in the table
    int countdown;                                     // frames to the period's end
    QuietGate gate;
} unit;

// ---------------------------------------------------------------- a block, on the audio thread

// One call's cycles counted, and the stage switched off when it went too far.
static inline void account(Stage* s, u32 began, u32 took) {
    u32 sum = s->sum + took;
    s->sum = sum < took ? 0xffffffffu : sum;                                        // held at the top, never wrapped
    ++s->calls;
    if (took > s->most) s->most = took;
    if (took > s->cycles) ++s->over;
    if (began - s->window_start >= SECOND) {                                        // a new second
        s->window_start = began;
        s->window_over = 0;
        s->window_most = 0;
    }
    if (took > CYCLES_OFF) {
        s->off_overs = 0;
        s->off_cycles = took;
        s->state = OFF;
        return;
    }
    if (took > (s->cycles << 1) && took - s->cycles > SPARE) {                      // over: twice its cost, and its allowance
        if (took > s->window_most) s->window_most = took;
        if (++s->window_over >= OVER_CALLS) {
            s->off_overs = s->window_over;
            s->off_cycles = s->window_most;
            s->state = OFF;
        }
    }
}

// From the stubs: the stages of one place, once a block.
extern "C" void op1f_stages_run(u32 place) OP1F_NAMED("op1f_stages_run");
extern "C" void op1f_stages_run(u32 place) {
    if (place >= (u32)PLACES) return;
    if (op1f_stages_at[place] == 0) return;                                         // R5: the stub's word, nothing written yet
    BARRIER();                                                                      // the table is read after the word
    u32 count = counts[place];                                                      // R5: the word may say one more, for the effects
    if (count > (u32)MOST) count = MOST;
    u32 host = place < 2u ? place : 2u;
    u32 point = place < 2u ? (u32)OP1F_AT_INSTRUMENT : place == 2u ? (u32)OP1F_AT_MASTER_IN : (u32)OP1F_AT_MASTER_OUT;
    int* left = place < 2u ? BUFFER(INSTRUMENT_LEFT) : BUFFER(MASTER_LEFT);
    int* right = place < 2u ? BUFFER(INSTRUMENT_RIGHT) : BUFFER(MASTER_RIGHT);
    if (place != 2u && (op1f_effects_registered != 0 || (place < 2u && &op1f_lfos_waves != 0 && op1f_lfos_waves != 0)))   // R24: or a wave's
        op1f_effects_after(point, host, left, right);                               // R5: an effect's tails, before the stages
    u32 period = RD32(PERIOD);
    u32 playing = RD32(TRANSPORT) != 0 ? 1u : 0u;
    for (u32 i = 0; i < count; ++i) {
        Stage* s = &stages[order[place][i]];
        if (s->state != (u32)RUNNING) continue;
        Op1fBlock block;                                                            // each stage its own: none sees what another did to it
        block.bytes = BLOCK_BYTES;
        block.point = point;
        block.host = host;
        block.frames = FRAMES;
        if (s->listens) {
            for (u32 k = 0; k < (u32)FRAMES; ++k) {
                listening_left[k] = left[k];
                listening_right[k] = right[k];
            }
            block.left = listening_left;
            block.right = listening_right;
        } else {
            block.left = left;
            block.right = right;
        }
        block.period = period;
        block.playing = playing;
        u32* calls = &s->calls_at[host == 1u ? 1 : 0];
        block.block = *calls;
        *calls = block.block + 1u;
        u32 began = cycles_now();
        s->process(&block);
        account(s, began, cycles_now() - began);
    }
}

// ---------------------------------------------------------------- the helper, from any thread

static int slot_of(const Op1fCard* card) {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    if (card == 0) return -1;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards_2.slots[i].card == card) return op1f_cards_verdict(i) == 0 ? (int)i : -1;
    return -1;
}

extern "C" int op1f_stage(const Op1fCard* card, unsigned ask) {
    int slot = slot_of(card);
    if (slot < 0) return 0;
    u32 count = unit.count;
    BARRIER();
    for (u32 i = 0; i < count && i < (u32)MOST; ++i)
        if (stages[i].slot == (u32)slot && stages[i].ask == ask) return stages[i].state == (u32)RUNNING ? 1 : 0;
    return 0;
}

// ---------------------------------------------------------------- the registration, on the display thread

// An ask's name: one to fifteen characters of a-z, 0-9, '-' and '_', ended within its sixteen bytes.
static int plain_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)NAME;
}

static const char* point_words(u32 point) {
    return point == (u32)OP1F_AT_INSTRUMENT ? "the synth and the drum after their effects"
         : point == (u32)OP1F_AT_MASTER_IN ? "the master before its effect" : "the master after its effect";
}

// Why an ask of this kind is left aside, or 0; `a` and `b` for the line when its table is not in order:
//   (the ask's bytes, the bytes of an Op1fStage)    its table is too small, or there is none
//   (the bytes its head says, the ask's bytes)      the two differ
//   (its flags, OP1F_STAGE_LISTENS)                 a flag this base does not know
//   (its `shown`'s place from the pack's first byte, 0)   a `shown` (the kind has no page)
//   (its point, 3)                                  a point this base does not know
//   (its function's place from the pack's first byte, the pack's bytes)   `process` is not a function of its pack
//   (its memory, the card's asks)                   `memory` names no ask of kind OP1F_ASK_MEMORY of the card
// `plain` says whether its name may be shown.
enum { IN_ORDER = 0, NOT_IN_ORDER = 1, NOT_A_NAME = 2 };
static int refused(const Op1fAsk* ask, const Op1fCard* card, const Op1fCardSlot* slot, u32* a, u32* b, int* plain) {
    *plain = 0;
    const Op1fStage* table = (const Op1fStage*)ask->what;
    if (table == 0 || ask->bytes < (u32)TABLE_BYTES) {                              // the card's check holds the table in its pack, with its bytes
        *a = ask->bytes;
        *b = TABLE_BYTES;
        return NOT_IN_ORDER;
    }
    if (!plain_name(table->head.name)) return NOT_A_NAME;
    *plain = 1;
    if (table->head.bytes != ask->bytes) {
        *a = table->head.bytes;
        *b = ask->bytes;
        return NOT_IN_ORDER;
    }
    if ((table->head.flags & ~(u32)OP1F_STAGE_LISTENS) != 0) {
        *a = table->head.flags;
        *b = OP1F_STAGE_LISTENS;
        return NOT_IN_ORDER;
    }
    if (table->head.shown != 0) {
        *a = (u32)table->head.shown - slot->first;
        *b = 0;
        return NOT_IN_ORDER;
    }
    if (table->point < (u32)OP1F_AT_INSTRUMENT || table->point > (u32)OP1F_AT_MASTER_OUT) {
        *a = table->point;
        *b = OP1F_AT_MASTER_OUT;
        return NOT_IN_ORDER;
    }
    u32 process = (u32)table->process;
    if (process < slot->first || process >= slot->end || (process & 1u) != 0) {
        *a = process - slot->first;
        *b = slot->end - slot->first;
        return NOT_IN_ORDER;
    }
    if (table->memory != 0 && (table->memory > card->ask_count || card->asks[table->memory - 1u].kind != (u32)OP1F_ASK_MEMORY)) {
        *a = table->memory;
        *b = card->ask_count;
        return NOT_IN_ORDER;
    }
    return IN_ORDER;
}

static void enrol() {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    u32 n = 0, declared = 0;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;                                   // a card the base does not use
        const Op1fCardSlot* slot = &op1f_cards_2.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 6u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_STAGE) continue;
            u32 a = 0, b = 0;
            int plain = 0, why = refused(ask, card, slot, &a, &b, &plain);
            const Op1fStage* table = (const Op1fStage*)ask->what;
            const char* name = plain ? table->head.name : "?";
            if (why == NOT_A_NAME) {
                op1f_log(2, LOG_FILE, 2, "stage: '%s' of '%s' is left aside: %s", name, card->name, "its name is not one the log can show");
                continue;
            }
            if (why == NOT_IN_ORDER) {
                op1f_log(2, LOG_FILE, 3, "stage: '%s' of '%s' is left aside: its table is not in order (%u, %u)", name, card->name, a, b);
                continue;
            }
            if (table->cycles < 1u || table->cycles > (u32)CYCLES_MOST) {
                op1f_log(2, LOG_FILE, 2, "stage: '%s' of '%s' is left aside: %s", name, card->name, "its cost is not between 1 and 116000");
                continue;
            }
            u32 cost = table->point == (u32)OP1F_AT_INSTRUMENT ? table->cycles << 1 : table->cycles;   // the instrument's: the synth's call and the drum's
            if (declared + cost > (u32)CYCLES_ALL) {
                op1f_log(2, LOG_FILE, 2, "stage: '%s' of '%s' is left aside: %s", name, card->name, "the stages would take more than 174000 cycles a block");
                continue;
            }
            if (n == (u32)MOST) {
                op1f_log(2, LOG_FILE, 2, "stage: '%s' of '%s' is left aside: %s", name, card->name, "it would be the seventeenth");
                continue;
            }
            declared += cost;
            Stage* s = &stages[n++];
            s->process = table->process;
            s->slot = i;
            s->ask = k;
            s->point = table->point;
            s->cycles = table->cycles;
            s->listens = (table->head.flags & (u32)OP1F_STAGE_LISTENS) != 0 ? 1u : 0u;
            s->memory = table->memory;
            for (u32 c = 0; c < (u32)NAME; ++c) s->name[c] = table->head.name[c];
            s->state = s->memory != 0 ? (u32)WAITING : (u32)RUNNING;
            op1f_log(3, LOG_FILE, 1, "stage: '%s' of '%s' at %s, %u cycles a block declared", s->name, card->name, point_words(s->point), s->cycles);
        }
    }
    u32 at[PLACES] = {0, 0, 0, 0};
    for (u32 pass = 0; pass < 2u; ++pass) {                                         // those that change the sound, then those that listen
        for (u32 j = 0; j < n; ++j) {
            const Stage* s = &stages[j];
            if (s->listens != pass) continue;
            if (s->point == (u32)OP1F_AT_INSTRUMENT) {
                order[0][at[0]++] = (u8)j;
                order[1][at[1]++] = (u8)j;
            } else {
                u32 place = s->point == (u32)OP1F_AT_MASTER_IN ? 2u : 3u;
                order[place][at[place]++] = (u8)j;
            }
        }
    }
    unit.count = n;
    u32 effects = op1f_effects_registered != 0 ? 1u : 0u;                           // R5: the effects' call after their effect
    u32 waves = &op1f_lfos_waves != 0 && op1f_lfos_waves != 0 ? 1u : 0u;            // R24: a wave's call, at the synth and the drum
    for (u32 p = 0; p < (u32)PLACES; ++p) counts[p] = at[p];
    BARRIER();                                                                      // the tables are whole before the words that say so
    // A place's word: its stages, and one more where the effects unit is called after the effect: at 0, 1 and 3 once an
    // effect of a pack is registered, at 0 and 1 once a wave is (R24). Else base 44's.
    for (u32 p = 0; p < (u32)PLACES; ++p) op1f_stages_at[p] = at[p] + ((p != 2u && effects) || (p < 2u && waves) ? 1u : 0u);
}

// ---------------------------------------------------------------- once a frame, on the display thread

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

static const char* pack_name(u32 slot) {
    return op1f_cards_2.slots[slot].card->name;
}

extern "C" void op1f_stages_frame() OP1F_NAMED("op1f_stages_frame");
extern "C" void op1f_stages_frame() {
    if (!unit.enrolled) {
        unit.enrolled = 1;
        unit.countdown = PERIOD_FRAMES;
        enrol();
        return;
    }
    u32 count = unit.count;
    for (u32 i = 0; i < count; ++i) {
        Stage* s = &stages[i];
        if (s->state == (u32)WAITING) {
            int memory = op1f_memory_state(s->slot, s->memory - 1u);
            if (memory > 0) {
                s->state = RUNNING;
            } else if (memory < 0) {
                s->state = ASIDE;
                op1f_log(2, LOG_FILE, 2, "stage: '%s' of '%s' is left aside: %s", s->name, pack_name(s->slot), "its memory was refused");
            }
        } else if (s->state == (u32)OFF && !s->off_said) {
            s->off_said = 1;
            if (s->off_overs == 0)
                op1f_log(1, LOG_FILE, 5, "stage: '%s' of '%s' is switched off until the next start: %u cycles in one call, %u declared", s->name, pack_name(s->slot),
                         s->off_cycles, s->cycles);
            else
                op1f_log(1, LOG_FILE, 6, "stage: '%s' of '%s' is switched off until the next start: %u calls in one second over twice its cost and its allowance, the most %u cycles, %u declared",
                         s->name, pack_name(s->slot), s->off_overs, s->off_cycles, s->cycles);
        }
    }
    if (count == 0 || --unit.countdown > 0) return;
    unit.countdown = PERIOD_FRAMES;
    u32 calls[MOST], sum[MOST], most[MOST], over[MOST];
    u32 signature = quiet_mix(quiet_start(), count);
    int activity = 0;
    for (u32 i = 0; i < count; ++i) {                                               // read and cleared, whatever the gate says
        Stage* s = &stages[i];
        u32 mask;
        MASK_OFF(mask);
        calls[i] = s->calls;
        sum[i] = s->sum;
        most[i] = s->most;
        over[i] = s->over;
        s->calls = 0;
        s->sum = 0;
        s->most = 0;
        s->over = 0;
        MASK_ON(mask);
        signature = quiet_mix(signature, s->state);
        signature = quiet_mix(signature, over[i] != 0 ? 1u : 0u);
        if (over[i] != 0) activity = 1;
    }
    if (!quiet_period(&unit.gate, signature, activity)) return;
    for (u32 i = 0; i < count; ++i) {
        if (calls[i] == 0) continue;
        op1f_log(3, LOG_FILE, 4, "stage: '%s' this period: %u call(s), %u cycles at most, %u on average, %u over", stages[i].name, calls[i], most[i],
                 divided(sum[i], calls[i]), over[i]);
    }
}

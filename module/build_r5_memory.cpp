// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R5, the base pack 28: working memory (OS 1.7.0+0). A sibling of base 24's memory unit (which
// stays as it was): everything that unit does stands here as it was, and one thing is added, memory asked after the
// start for the base's own use, by the same machinery and inside the same limits (op1f_memory_more, op1f_memory_block,
// module/build_r5_base.h): the effects unit asks it for an effect's working memory at a place.
//
// Any pack may ask, through its card, for a block of working memory (OP1F_ASK_MEMORY, its table an Op1fMemory,
// module/build_r5_card.h). The memory comes from the C runtime's first heap of the firmware, which hands out blocks
// and answers 0 when it has no room; it takes no lock, so the base asks it only from the display thread, with
// interrupts masked around each call. A block is never given back while the unit runs.
//
// The registration, once, at the base's first frame (op1f_memory_frame, from the cards unit, on the display thread,
// before the stages unit's frame work): every card the base uses (op1f_cards_verdict) that says level 6 or later and
// is of level 4's size, in the list's order, is looked through for asks of this kind, each in its card's order. An ask
// is held to its form: its table's bytes at least an Op1fMemory's and the same as its head says, its name one to
// fifteen characters of a-z, 0-9, '-' and '_' with its end, no flag, no `shown`, `bytes` not 0 and `align` a power of
// two from 4 to OP1F_MEMORY_ALIGN_MOST; an ask not in order is left aside with a line, its card still used. Then, in
// that order, each ask is refused with a line when it asks more than OP1F_MEMORY_MOST, when the asks given before it
// and it would take more than OP1F_MEMORY_MOST together, when it would be the thirty-third (base 39; the seventeenth before), when the heap's free space
// after it would be under OP1F_MEMORY_KEPT, or when the heap answers 0. A block given has its line, with its address
// and what stays free.
//
// The base's own asks (op1f_memory_more, from the display thread, at any frame): each is held to the same limits, its
// bytes counted with the packs' (16 MiB in all, 8 MiB kept free), and has the same lines with the name it is handed
// (`whose`) in the place of the ask's and the pack's. They are records of the same table behind the packs' asks, at
// most 16 more (as before base 39, when OP1F_MEMORIES_MOST was 16 too); a record of the base's own matches no card's slot, so that no stage waits for it. The
// handle is the record's number + 1.
//
// The block is asked for its bytes and its alignment more, and begins at the first multiple of the alignment inside
// it. It is then cleared to 0 by the display thread, a quarter of a megabyte a frame, the asks in their order; only when
// the whole of it is cleared is it handed over: op1f_memory and op1f_memory_block answer it from then on, and
// op1f_memory_state, which the stages unit asks for a stage that waits for its memory, answers 1. The record of each ask
// is written whole before the word that says how many records there are, and the word that says a block is handed over
// is written after the last word of it was cleared, so that a reader on any thread finds a record whole or none.
//
// Nothing is divided (the module links no runtime). Compiled for the PC as it is, with OP1F_HOST defined: the
// stand-ins come in through headers of the PC's own (for tests this tree does not hold).

#include "build_r5_base.h"

typedef unsigned int u32;
typedef unsigned char u8;

#ifdef OP1F_HOST
#ifdef OP1F_EFFECTS_R5_HOST
#include "mock_effects_r5.h"                           // the effects unit's stand-ins: the heap, the masks
#else
#include "mock_stages_r4.h"                            // level 6's stand-ins: the heap, the masks
#endif
#define HEAP_TAKE(index, bytes) mock_heap_take(index, bytes)
#define HEAP_FREE(index) mock_heap_free(index)
#define MASK_OFF(mask) ((void)((mask) = mock_mask()))
#define MASK_ON(mask) mock_unmask(mask)
#define BARRIER() ((void)0)
#else
typedef void* (*HeapTakeFn)(int index, u32 bytes);
typedef int (*HeapFreeFn)(int index);
// The C runtime's heaps: a block of a heap (0 when it has no room), and the heap's free space in bytes (-1 for no such
// heap). Heap 0 is the one the base takes from.
#define HEAP_TAKE(index, bytes) (((HeapTakeFn)0x8fcc5a68u)(index, bytes))
#define HEAP_FREE(index) (((HeapFreeFn)0x8fcc51fcu)(index))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
#define BARRIER() asm volatile("" : : : "memory")
#endif

extern "C" Op1fCards op1f_cards OP1F_NAMED("op1f_cards");

enum { MOST = OP1F_MEMORIES_MOST, MORE = 16, RECORDS = MOST + MORE, CARDS = OP1F_CARDS_MOST, NAME = OP1F_KIND_NAME, TABLE_BYTES = 36,
       CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES, HEAP = 0, ALIGN_LEAST = 4, CLEAR_A_FRAME = 256 * 1024, WHOSE = 48 };
static const u32 BYTES_MOST = (u32)OP1F_MEMORY_MOST, KEPT = (u32)OP1F_MEMORY_KEPT, ALIGN_MOST = (u32)OP1F_MEMORY_ALIGN_MOST;
static const u32 BASES = 0xffffffffu;                  // the slot of a record of the base's own: no card's
static_assert(sizeof(Op1fMemory) == TABLE_BYTES, "a memory's table is 36 bytes at level 6");

static const char LOG_FILE[] = "../src/main/op1f_memory.cpp";

enum { CLEARING = 0, GIVEN = 1, REFUSED = 2 };

struct Memory {
    u32 slot, ask;                                     // the card's slot in op1f_cards, the ask's number in its list (BASES for the base's own)
    u32 bytes;
    u32* block;                                        // on its alignment; 0 when refused
    u32 cleared;                                       // the words cleared so far
    volatile u32 state;
    char name[NAME];
};

static Memory memories[RECORDS];

static struct {
    u32 enrolled;                                      // the registration was made
    volatile u32 count;                                // records in the table, each whole
    u32 taken;                                         // the bytes given so far
    u32 packs;                                         // records of the packs' asks
    u32 bases;                                         // records of the base's own
} unit;

// ---------------------------------------------------------------- the helpers, from any thread

static const Memory* find(u32 slot, u32 ask) {
    u32 count = unit.count;
    BARRIER();
    for (u32 i = 0; i < count && i < (u32)RECORDS; ++i)
        if (memories[i].slot == slot && memories[i].ask == ask) return &memories[i];
    return 0;
}

extern "C" int op1f_memory_state(unsigned slot, unsigned ask) OP1F_NAMED("op1f_memory_state");
extern "C" int op1f_memory_state(unsigned slot, unsigned ask) {
    if (!unit.enrolled || slot == BASES) return 0;
    const Memory* m = find(slot, ask);
    if (m == 0) return -1;
    u32 state = m->state;
    return state == (u32)GIVEN ? 1 : state == (u32)REFUSED ? -1 : 0;
}

extern "C" void* op1f_memory(const Op1fCard* card, unsigned ask) {
    u32 seen = op1f_cards.count > (u32)CARDS ? (u32)CARDS : op1f_cards.count;
    if (card == 0) return 0;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards.slots[i].card != card) continue;
        if (op1f_cards_verdict(i) != 0) return 0;
        const Memory* m = find(i, ask);
        if (m == 0 || m->state != (u32)GIVEN) return 0;
        BARRIER();
        return m->block;
    }
    return 0;
}

extern "C" void* op1f_memory_block(int handle) {
    u32 count = unit.count;
    BARRIER();
    if (handle <= 0 || (u32)handle > count) return 0;
    const Memory* m = &memories[handle - 1];
    if (m->slot != BASES || m->state != (u32)GIVEN) return 0;
    BARRIER();
    return m->block;
}

// ---------------------------------------------------------------- the registration, on the display thread

static int plain_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)NAME;
}

// Why an ask of this kind is left aside, or 0; `a` and `b` for the line when its table is not in order:
//   (the ask's bytes, the bytes of an Op1fMemory)   its table is too small, or there is none
//   (the bytes its head says, the ask's bytes)      the two differ
//   (its flags, 0)                                  a flag is set
//   (its `shown`'s place from the pack's first byte, 0)   a `shown` (the kind has no page)
//   (its bytes, 0)                                  no bytes
//   (its alignment, 4096)                           not a power of two from 4 to 4096
enum { IN_ORDER = 0, NOT_IN_ORDER = 1, NOT_A_NAME = 2 };
static int refused(const Op1fAsk* ask, const Op1fCardSlot* slot, u32* a, u32* b, int* plain) {
    *plain = 0;
    const Op1fMemory* table = (const Op1fMemory*)ask->what;
    if (table == 0 || ask->bytes < (u32)TABLE_BYTES) {
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
    if (table->head.flags != 0) {
        *a = table->head.flags;
        *b = 0;
        return NOT_IN_ORDER;
    }
    if (table->head.shown != 0) {
        *a = (u32)table->head.shown - slot->first;
        *b = 0;
        return NOT_IN_ORDER;
    }
    if (table->bytes == 0) {
        *a = 0;
        *b = 0;
        return NOT_IN_ORDER;
    }
    u32 align = table->align;
    if (align < (u32)ALIGN_LEAST || align > ALIGN_MOST || (align & (align - 1u)) != 0) {
        *a = align;
        *b = ALIGN_MOST;
        return NOT_IN_ORDER;
    }
    return IN_ORDER;
}

// The heap asked for `bytes` on `align`, inside the limits: the block on its alignment, or 0 with `why` set.
static u32* take_from_heap(u32 bytes, u32 align, const char** why, u32* free_after) {
    void* got = 0;
    *why = 0;
    *free_after = 0;
    if (bytes > BYTES_MOST) *why = "more than 16 MiB";
    else if (unit.taken + bytes > BYTES_MOST) *why = "the packs would take more than 16 MiB";
    else {
        u32 want = bytes + align;                                                   // room to move the block onto its alignment
        u32 mask;
        MASK_OFF(mask);
        int space = HEAP_FREE(HEAP);
        if (space >= 0 && (u32)space >= want && (u32)space - want >= KEPT) got = HEAP_TAKE(HEAP, want);
        int after = HEAP_FREE(HEAP);
        MASK_ON(mask);
        *free_after = after < 0 ? 0u : (u32)after;
        if (space < 0 || (u32)space < want || (u32)space - want < KEPT) *why = "less than 8 MiB would stay free";
        else if (got == 0) *why = "the heap has no such room";
    }
    if (*why) return 0;
    unit.taken += bytes;
    return (u32*)(((u32)got + align - 1u) & ~(align - 1u));
}

// One ask, in order: given, or refused with its line. Answers the block on its alignment, or 0.
static u32* take(const char* name, const char* pack, u32 bytes, u32 align) {
    const char* why = 0;
    u32 free_after = 0;
    u32* at = take_from_heap(bytes, align, &why, &free_after);
    if (why) {
        op1f_log(1, LOG_FILE, 2, "memory: '%s' of '%s' is refused %u bytes: %s", name, pack, bytes, why);
        return 0;
    }
    op1f_log(3, LOG_FILE, 1, "memory: '%s' of '%s' is given %u bytes at 0x%08x; %u KiB stay free", name, pack, bytes, (u32)at, free_after >> 10);
    return at;
}

static void enrol() {
    u32 seen = op1f_cards.count > (u32)CARDS ? (u32)CARDS : op1f_cards.count;
    u32 n = unit.count;                                                             // behind any record of the base's own
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;                                   // a card the base does not use
        const Op1fCardSlot* slot = &op1f_cards.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 6u || card->asks == 0) continue;
        u32 asks = card->ask_count > (u32)OP1F_ASKS_MOST ? (u32)OP1F_ASKS_MOST : card->ask_count;
        for (u32 k = 0; k < asks; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_MEMORY) continue;
            u32 a = 0, b = 0;
            int plain = 0, why = refused(ask, slot, &a, &b, &plain);
            const Op1fMemory* table = (const Op1fMemory*)ask->what;
            const char* name = plain ? table->head.name : "?";
            if (why == NOT_A_NAME) {
                op1f_log(2, LOG_FILE, 3, "memory: '%s' of '%s' is left aside: %s", name, card->name, "its name is not one the log can show");
                continue;
            }
            if (why == NOT_IN_ORDER) {
                op1f_log(2, LOG_FILE, 4, "memory: '%s' of '%s' is left aside: its table is not in order (%u, %u)", name, card->name, a, b);
                continue;
            }
            if (unit.packs == (u32)MOST) {
                op1f_log(1, LOG_FILE, 2, "memory: '%s' of '%s' is refused %u bytes: %s", name, card->name, table->bytes, "it would be the thirty-third");
                continue;
            }
            Memory* m = &memories[n];
            m->slot = i;
            m->ask = k;
            m->bytes = table->bytes;
            m->cleared = 0;
            for (u32 c = 0; c < (u32)NAME; ++c) m->name[c] = table->head.name[c];
            m->block = take(m->name, card->name, table->bytes, table->align);
            m->state = m->block != 0 ? (u32)CLEARING : (u32)REFUSED;
            BARRIER();                                                              // the record is whole before the word that counts it
            ++unit.packs;
            unit.count = ++n;
        }
    }
}

// R5: a block for the base's own use. The display thread.
extern "C" int op1f_memory_more(unsigned bytes, unsigned align, const char* whose) {
    if (whose == 0) whose = "the base";
    if (bytes == 0 || align < (u32)ALIGN_LEAST || align > ALIGN_MOST || (align & (align - 1u)) != 0) {
        op1f_log(1, LOG_FILE, 6, "memory: %s is refused %u bytes: %s", whose, bytes, "its alignment or its size is not in order");
        return OP1F_FAILED;
    }
    if (unit.bases == (u32)MORE) {
        op1f_log(1, LOG_FILE, 6, "memory: %s is refused %u bytes: %s", whose, bytes, "the base has asked sixteen blocks already");
        return OP1F_FAILED;
    }
    const char* why = 0;
    u32 free_after = 0;
    int over = bytes > BYTES_MOST || unit.taken + bytes > BYTES_MOST;
    u32* at = take_from_heap(bytes, align, &why, &free_after);
    if (why) {
        op1f_log(1, LOG_FILE, 6, "memory: %s is refused %u bytes: %s", whose, bytes, why);
        return over ? OP1F_TOO_LARGE : OP1F_FAILED;
    }
    op1f_log(3, LOG_FILE, 5, "memory: %s is given %u bytes at 0x%08x; %u KiB stay free", whose, bytes, (u32)at, free_after >> 10);
    u32 n = unit.count;
    Memory* m = &memories[n];
    m->slot = BASES;
    m->ask = n;
    m->bytes = bytes;
    m->cleared = 0;
    m->name[0] = 0;
    m->block = at;
    m->state = CLEARING;
    BARRIER();                                                                      // the record is whole before the word that counts it
    ++unit.bases;
    unit.count = n + 1u;
    return (int)(n + 1u);
}

// ---------------------------------------------------------------- once a frame, on the display thread

// Up to a quarter of a megabyte of the blocks still to clear, in their order; a block cleared whole is handed over.
static void clear_some() {
    u32 budget = (u32)CLEAR_A_FRAME >> 2;                                           // words
    u32 count = unit.count;
    for (u32 i = 0; i < count && budget != 0; ++i) {
        Memory* m = &memories[i];
        if (m->state != (u32)CLEARING) continue;
        u32 words = (m->bytes + 3u) >> 2;                                           // the block has its alignment's room behind it
        u32 end = m->cleared + budget < words ? m->cleared + budget : words;
        volatile u32* block = m->block;
        for (u32 w = m->cleared; w < end; ++w) block[w] = 0;
        budget -= end - m->cleared;
        m->cleared = end;
        if (end == words) {
            BARRIER();                                                              // every word cleared before the word that hands it over
            m->state = GIVEN;
        }
    }
}

extern "C" void op1f_memory_frame() OP1F_NAMED("op1f_memory_frame");
extern "C" void op1f_memory_frame() {
    if (!unit.enrolled) {
        enrol();
        BARRIER();
        unit.enrolled = 1;                                                          // the records counted before the word that says they were looked at
        return;
    }
    clear_some();
}

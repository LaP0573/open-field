// Made for OS 1.7.3.
// Step R7, the base pack (base.pack 44, on top of base 43): files per tape. A unit of its own; no unit of base 43
// changes but the cards unit, which calls this one's frame work and counts its kind of ask as known.
//
// A card of level 7 or later may carry one ask of kind OP1F_ASK_TAPES (module/build_r6e_card.h, "Base 44"): the number of
// one of its folders, whose files named tape<N>.<anything> (N the tape's number, 1 to 8) the base keeps with the unit's
// tapes, and a function told when it did. This unit
//
//   notes the unit's copy of a tape: one entry of the base's table (profile group "base") puts a jump at the entry of
//     the firmware's tape clone 0x8d4fb908 (the source's and the copy's index, a progress function, and a fourth word
//     whose slot in the caller's frame it writes). Ours runs the clone through its trampoline, then only notes the
//     event in a ring, on the tape list's key thread. No line, no file there. 8 tracks' copy of tracks 5 to 8 retargets
//     the tape list's call of the clone (0x8c0d84ae), not its entry: both stand;
//   notes a tape made anew (T3 in the tape list, on a slot a delete emptied, or a unit's first tape) without a place
//     of its own: the values unit's tape load, which the firmware hands the flag 1 for a tape made anew and for a copy,
//     calls op1f_tapes_made(); at the frame the mounted tape (0x8fc219a8) is that tape, and unless it is the copy the
//     clone just noted, a DELETE of it is noted: its files go, as a delete's did. This unit's first form had a second
//     place at the firmware's tape delete; 65 places were one more than the door of bases 33 to 36 reads of a base
//     (64), and it refused every pack sorting before base.pack, Faust's 8 tracks, beside such a base. So one place
//     it stays (base 43's 66 and this one: 67; a unit on base 36 or older gets this base with the install page's
//     whole install, not through its door);
//   carries what was noted at its frame work (op1f_tapes_frame, from the cards unit, on the display thread, behind the
//     values' unit and before any frame function), never while the disk mode is open (the events wait for it to
//     close; a delete from the computer is carried when a tape is next made on that slot): for each card in use with such an ask, in the list's order, a copy copies every tape<src>.<rest> of the
//     folder to tape<dst>.<rest> (through tmp/op1f_tape.part and a rename, as a write lands, so a file is whole or not
//     there) and removes a tape<dst>.<rest> the source has not; a delete removes every tape<N>.<rest>. Names that are
//     not tape<1..8>.<rest>, and folders inside, are left alone. Then the ask's `changed` is called, outside any frame
//     function. One line a folder an event.
//
// The firmware's calls are the folders unit's, and the mounted tape 0x8fc219a8 (-1 for none, else 0 to 7; tape train
// and markers read it the same way) (module/build_r2_folders.cpp says how each was read): the size
// 0x8c051f9c, the removal 0x8fcbefb4, createFolderRecursive 0x8c05206c, 0x8c052054 (a folder), the listing 0x8c051e28,
// 0x8c0a4bd0 (an entry's name at +11), 0x8c0a57d4, open 0x8fcba988, read 0x8fcba954, write 0x8fcba8c0, close
// 0x8fcba8ac, rename 0x8c051f88, the log 0x8c053a20, and the word [0x84d25bcc], not 0 while the disk mode is open. A
// folder's place comes from the folders unit (op1f_path), which answers it from any thread once the folders are settled.
// Freestanding: no library, no division. Compiled for the PC as it is, with OP1F_HOST defined, for its test
// (tools/tests/host/tapes_test.cpp, its stand-ins in tools/tests/host/mock_tapes_r7.h).

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*FileSizeFn)(const char* path);
typedef int (*FileRemoveFn)(const char* path);
typedef int (*MakeFoldersFn)(const char* path);
typedef int (*IsFolderFn)(const char* path);
typedef void* (*DirOpenFn)(const char* path);
typedef void* (*DirNextFn)(void* handle);
typedef int (*DirCloseFn)(void* handle);
typedef int (*OpenFn)(const char* path, const char* mode);
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);
typedef int (*WriteFn)(const void* data, u32 size, u32 count, int handle);
typedef void (*CloseFn)(int handle);
typedef int (*RenameFn)(const char* from, const char* to);
typedef int (*MountedTapeFn)();

#ifdef OP1F_HOST
#include "mock_tapes_r7.h"
#define BARRIER() ((void)0)
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_REMOVE ((FileRemoveFn)0x8fcbefb4u)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_IS_FOLDER ((IsFolderFn)0x8c052054u)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)
#define STOCK_READ ((ReadFn)0x8fcba954u)
#define STOCK_WRITE ((WriteFn)0x8fcba8c0u)
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)
#define STOCK_RENAME ((RenameFn)0x8c051f88u)
#define STOCK_MOUNTED_TAPE ((MountedTapeFn)0x8fc219a8u)
#define RD32(a) (*(volatile u32*)(a))
#define BARRIER() asm volatile("" : : : "memory")
#endif

static const u32 DISK_OPEN = 0x84d25bccu;
enum { RING = 16, TAPES = 8, CHUNK = 4096, CLONE = 1, DELETE = 2, PREFIX = 6, NAMES = 64, NAME = OP1F_FILE_NAME, PATH = 96,
       DIR_NAME_AT = 11, TABLE_BYTES = 36 };
static const char LOG_FILE[] = "../src/main/op1f_tapes.cpp";
static const char TEMPORARY_FOLDER[] = "tmp/";
static const char TEMPORARY[] = "tmp/op1f_tape.part";
static const char WRITE_MODE[] = "w+b";
static const char READ_MODE[] = "rb";

// base 42 (R20): the cards unit's second list, 128 slots (module/build_r3_base.h, as build_r8_options.cpp declares it)
#ifndef OP1F_BUILD_R3_BASE_H
enum { OP1F_SLOTS_MOST = 128 };
struct Op1fCards2 { unsigned count, more; Op1fCardSlot slots[OP1F_SLOTS_MOST]; };
extern "C" Op1fCards2 op1f_cards_2 OP1F_NAMED("op1f_cards_2");
extern "C" void op1f_cards_lists() OP1F_NAMED("op1f_cards_lists");
extern "C" int op1f_cards_in_use(unsigned slot) OP1F_NAMED("op1f_cards_in_use");
#endif
enum { MOST = OP1F_SLOTS_MOST };

// The mark of a base that carries out this kind of ask.
extern "C" const u32 op1f_tapes_1 OP1F_NAMED("op1f_tapes_1");
const u32 op1f_tapes_1 = 1;

struct Event { u32 kind, src, dst; };
static Event events[RING];
static volatile u32 noted;                             // events noted since the start; the ring holds the last RING
static u32 carried;                                    // the display thread's: events carried (or passed over)
static char names[NAMES][NAME];                        // the display thread's: one folder's names
static u8 chunk[CHUNK];                                // the display thread's: a copy's bytes on their way
static u32 temporary_made;
static volatile u32 made;                              // tapes made anew (or copies) loaded, from the values unit
static u32 made_seen;                                  // the display thread's
static volatile u32 clone_to;                          // the copy's index + 1 the clone noted, until a tape made is it

// From the clone (the key thread) and the frame (a tape made). ponytail: the two writers are not locked against each
// other; the frame notes a tape made in the frame after its load, and a copy needs another key press after that.
static void note(u32 kind, int src, int dst) {
    if ((u32)src >= (u32)TAPES || (u32)dst >= (u32)TAPES) return;
    Event* e = &events[noted & (RING - 1)];
    e->kind = kind;
    e->src = (u32)src;
    e->dst = (u32)dst;
    BARRIER();                                         // the event before the count
    noted = noted + 1;
}

#ifndef OP1F_HOST
// The room (zeroed data): the eight bytes the jump of the table displaced, copied there by the loader with a jump
// back behind them. The clone writes its fourth argument's slot in the caller's frame, so ours calls it with four.
typedef int (*CloneFn)(int src, int dst, void* progress, u32 fourth);
extern "C" u32 op1f_tapes_clone_room[4] OP1F_NAMED("op1f_tapes_clone_room");
u32 op1f_tapes_clone_room[4];

extern "C" int op1f_tapes_clone(int src, int dst, void* progress, u32 fourth) OP1F_NAMED("op1f_tapes_clone");
extern "C" int op1f_tapes_clone(int src, int dst, void* progress, u32 fourth) {
    int answer = ((CloneFn)(void*)op1f_tapes_clone_room)(src, dst, progress, fourth);
    clone_to = (u32)dst + 1u;
    note(CLONE, src, dst);
    return answer;
}
#else
static void op1f_tapes_note(u32 kind, int src, int dst) {   // the test's way in
    if (kind == CLONE) clone_to = (u32)dst + 1u;
    note(kind, src, dst);
}
#endif

// From the values unit's tape load with the firmware's flag set (module/build_r6b_keeps.cpp tape_take), on the
// thread that loaded the tape: a tape made anew or a copy is mounted.
extern "C" void op1f_tapes_made() OP1F_NAMED("op1f_tapes_made");
extern "C" void op1f_tapes_made() { made = made + 1; }

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// "tape<n>." at the start of a name, n 1 to 8: PREFIX, or 0.
static u32 tape_prefix(const char* s, u32 n) {
    return s[0] == 't' && s[1] == 'a' && s[2] == 'p' && s[3] == 'e' && s[4] == (char)('0' + n) && s[5] == '.' ? (u32)PREFIX : 0u;
}

static u32 put(char* out, u32 at, const char* text) {
    while (*text && at + 1 < (u32)PATH) out[at++] = *text++;
    out[at] = 0;
    return at;
}

// `from` copied over `to` through the temporary file and a rename: 1 when `to` holds it whole, else 0 and the
// temporary file taken off the disk.
static int copy_file(const char* from, const char* to) {
    int size = STOCK_FILE_SIZE(from);
    if (size < 0) return 0;
    if (!temporary_made) temporary_made = STOCK_MAKE_FOLDERS(TEMPORARY_FOLDER) != 0 ? 1u : 0u;
    int in = STOCK_OPEN(from, READ_MODE);
    if (in < 0) return 0;
    int out = STOCK_OPEN(TEMPORARY, WRITE_MODE);
    u32 done = 0;
    while (out >= 0) {
        int got = STOCK_READ(chunk, 1, CHUNK, in);
        if (got <= 0 || STOCK_WRITE(chunk, 1, (u32)got, out) != got) break;
        done += (u32)got;
    }
    STOCK_CLOSE(in);
    if (out >= 0) STOCK_CLOSE(out);
    if (out >= 0 && done == (u32)size && STOCK_RENAME(TEMPORARY, to) && STOCK_FILE_SIZE(to) == size) return 1;
    STOCK_FILE_REMOVE(TEMPORARY);
    return 0;
}

// One event on one card's folder, `folder` its place with the slash behind.
static void carry(const Event* e, const char* card, const char* folder) {
    char path[PATH], other[PATH];
    u32 base = put(path, 0, folder);
    void* dir = STOCK_DIR_OPEN(folder);
    if (dir == 0) {
        STOCK_LOG(1, LOG_FILE, 1, "tapes: '%s' could NOT list %s", card, folder);
        return;
    }
    u32 count = 0;
    for (;;) {
        const char* entry = (const char*)STOCK_DIR_NEXT(dir);
        if (!entry) break;
        const char* name = entry + DIR_NAME_AT;
        u32 n = 0;
        while (name[n] && n + 1 < (u32)NAME) ++n;
        if (count == (u32)NAMES || name[n] != 0 || n < (u32)PREFIX + 1u || name[0] != 't') continue;   // a tape's file or nothing of ours
        for (u32 i = 0; i <= n; ++i) names[count][i] = name[i];
        ++count;
    }
    STOCK_DIR_CLOSE(dir);
    const u32 src = e->src + 1, dst = e->dst + 1;
    u32 copied = 0, removed = 0, failures = 0;
    for (u32 i = 0; i < count; ++i) {
        const char* name = names[i];
        put(path, base, name);
        if (STOCK_IS_FOLDER(path)) continue;
        u32 at = tape_prefix(name, dst);
        if (at && e->kind == CLONE)                                                 // a tape<dst>.<rest> the source has not
            for (u32 j = 0; j < count && at; ++j)
                if (tape_prefix(names[j], src) && same(names[j] + PREFIX, name + PREFIX)) at = 0;
        if (at) {
            STOCK_FILE_REMOVE(path);
            if (STOCK_FILE_SIZE(path) < 0) ++removed; else ++failures;
        }
        if (e->kind == CLONE && tape_prefix(name, src)) {
            u32 o = put(other, 0, folder);
            put(other, o, name);
            other[o + 4] = (char)('0' + dst);                                       // the same length: one digit for another
            if (copy_file(path, other)) ++copied; else ++failures;
        }
    }
    if (e->kind == CLONE)
        STOCK_LOG(failures ? 1 : 3, LOG_FILE, 2, "tapes: '%s' tape %u copied to tape %u: %s %u file(s) copied, %u removed, %u NOT done", card, src, dst, folder, copied, removed, failures);
    else
        STOCK_LOG(failures ? 1 : 3, LOG_FILE, 3, "tapes: '%s' tape %u deleted: %s %u file(s) removed, %u NOT done", card, src, folder, removed, failures);
}

// A card's ask of this kind, or 0: a card of level 7 or later, of level 4's size, its table of this kind's bytes. The
// cards unit's check has held the list and each table to lie in the pack.
static const Op1fTapes* ask_of(const Op1fCard* card) {
    if (card->bytes < (u32)OP1F_CARD_LEVEL_7_BYTES || card->level < 7u || card->asks == 0) return 0;
    for (u32 i = 0; i < card->ask_count && i < (u32)OP1F_ASKS_MOST; ++i) {
        const Op1fAsk* a = &card->asks[i];
        if (a->kind == (u32)OP1F_ASK_TAPES && a->bytes >= (u32)TABLE_BYTES && a->what != 0) return (const Op1fTapes*)a->what;
    }
    return 0;
}

// Once a frame, on the display thread, before any frame function: the events noted since the last frame carried into
// every card's folder, and its `changed` told.
extern "C" void op1f_tapes_frame() OP1F_NAMED("op1f_tapes_frame");
extern "C" void op1f_tapes_frame() {
    if (RD32(DISK_OPEN) != 0) return;
    if (made != made_seen) {                           // ponytail: one look a frame; two tapes made between frames, the last
        made_seen = made;                              // one's files go. A copy whose load is not flagged keeps clone_to
        int mounted = STOCK_MOUNTED_TAPE();            // until the next tape made: that one's files then stay once.
        if (clone_to != 0 && clone_to == (u32)mounted + 1u) clone_to = 0;
        else note(DELETE, mounted, mounted);
    }
    u32 now = noted;
    if (now == carried) return;
    if (now - carried > (u32)RING) {
        STOCK_LOG(1, LOG_FILE, 4, "tapes: %u tape event(s) passed over: more than the base holds between two frames", now - carried - (u32)RING);
        carried = now - (u32)RING;
    }
    BARRIER();                                         // the count before the events it covers
    op1f_cards_lists();
    u32 seen = op1f_cards_2.count > (u32)MOST ? (u32)MOST : op1f_cards_2.count;
    for (; carried != now; ++carried) {
        const Event* e = &events[carried & (RING - 1)];
        for (u32 slot = 0; slot < seen; ++slot) {
            if (!op1f_cards_in_use(slot)) continue;
            const Op1fCard* card = op1f_cards_2.slots[slot].card;
            const Op1fTapes* t = ask_of(card);
            if (t == 0) continue;
            char folder[PATH];
            if (op1f_path(card, t->folder, 0, folder, sizeof folder) <= 0) {
                STOCK_LOG(2, LOG_FILE, 5, "tapes: '%s' asks for its folder %u, which it does not have: left aside", card->name, t->folder);
                continue;
            }
            carry(e, card->name, folder);
            if (t->changed) t->changed(e->kind == CLONE ? (u32)OP1F_TAPES_COPIED : (u32)OP1F_TAPES_DELETED, e->src + 1, e->dst + 1);
        }
    }
}

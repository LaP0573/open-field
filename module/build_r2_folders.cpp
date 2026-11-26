// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R10 (base.pack 34): no pack's folder is shown under `logs` either, the base's own (module/build_r10_logs.cpp).
// Step R2, the base pack (base.pack 21): the packs' folders and files of a base that holds nothing made for one pack.
// A sibling of base 20's folders unit, which stays as it was for base.pack 20. The names fx and lfo are no
// longer the base's: a pack's folder may be shown under them, as under any name no earlier folder has. The names a
// pack's folder is never shown under are the firmware's four (synth, drum, tapes, mixdown) and the loader's (packs).
//
// The rest is the elder's, unchanged; its description follows and holds, but where it counts the base among those
// whose names a folder is not shown under.
//
// Step R3, the base pack (base.pack 20): the packs' folders and files, and a file of the base's own. A sibling of
// base 19's folders unit, which stays as it was (base.pack 19 uses it). One function added:
//
//   op1f_folders_write_base(path, data, bytes): a file of the base's own written whole (module/build_r3_base.h), for
//     the base's units and never for a pack: the choice of sequencers the sequencer unit keeps, user/pf/.seq. Its
//     path lies in user/pf/ and its name begins with a dot, so it is no pack's file and no folder's, and the disk
//     mode never shows it. It is written by the steps of op1f_write, each the same: a temporary file, here the
//     base's own tmp/op1f_base.part, which no card's place in the list can name (a card's is tmp/op1f_NN.part),
//     opened "w+b", written, closed, renamed over the file, and held by the file's size afterwards; a step that fails
//     takes the temporary off the disk. Before them: user/pf/ is made when it is not there (it is made otherwise
//     only for a pack's folder), and nothing is written while the disk mode is open (OP1F_NOT_NOW, and no line: the
//     caller waits for the disk mode to close, frame by frame). A path that is not one of the base's own, or bytes
//     that are not a file's, answer OP1F_FAILED before anything is touched. Every failure but the first sixteen and
//     every write that landed but the first sixteen go unsaid; the lines are the packs' lines, by the name 'base'.
//     From the display thread: it is not held to a pack's frame function, since it is the base's own call.
//
// op1f_write is not changed: its steps stay where they were, and a test holds the new function's steps to them line
// for line.
//
// The rest is R1's, unchanged; its description follows and holds.
//
// Step R1, the base pack (base.pack 19): the packs' folders and files at level 4. A sibling of
// M3's folders unit, which stays as it was (base.pack 17 and 18 use it). One function added:
//
//   op1f_remove(card, folder, name): a file of one of the pack's folders taken off the disk, with the firmware's
//     removal the unit already uses for the disk mode's delete (0x8fcbefb4). That removal answers 0 when the file
//     went and -1 for anything else, a file that is not there among them, so it is not taken at its word: the file's
//     size is asked before (-1, no file: OP1F_NO_FILE, and the removal is not called) and after (a file still there:
//     OP1F_FAILED, said on the log), as the disk mode's delete holds it. A folder inside the pack's folder is no file
//     and is never handed to the removal. From the pack's own frame function only, and refused while the disk mode
//     is open, as op1f_write is; the same refusals answer the same numbers. One line for each of the first sixteen
//     removals and for every one that failed, by the pack's name.
//
// The rest is M3's, unchanged; its description follows and holds.
//
// Step M3, the base pack: the packs' folders and files (OS 1.7.0+0).
// A unit of its own, new at M3.
//
// A card of level 3 may list folders, none to eight (module/build_r1_card.h). This unit
//
//   settles them, once, at the base's first frame, before any card's frame function (the cards unit asks it,
//     op1f_folders_settle): in the order of the cards and then of each card's list, each folder's name and place are
//     held to the rule of a folder's text; a name the firmware, the loader, the base or an earlier folder has, capitals
//     counted as small letters, keeps the folder from being shown; a place an earlier folder has keeps the folder from
//     being the pack's at all. Each folder the pack keeps is made on the disk, user/pf/ and the place with the slash
//     behind (the folder maker makes the part before each slash), and one line a folder says what came of it;
//   answers the disk mode's tree for the folders that are shown: the base's root child maker (M3's disk unit)
//     asks op1f_folders_child for a name, and its population calls op1f_folders_populate once its own folders are
//     made. Each shown folder is made in the tree with the files its place holds on the disk, those of the loader's
//     rule for `packs` (module/build_m1_loader_disk.cpp): every name but those that begin with a dot and the folders
//     inside, which are not shown; a file the computer brings gets a place first; a delete takes the file off the
//     disk. The disk mode is built on the thread that handles keys, and the computer's requests come on the USB
//     device's; both may write lines, as the firmware's own folder maker does there;
//   reads, writes and lists a pack's files for it (op1f_list, op1f_read, op1f_write) and says where they lie
//     (op1f_path). The first three are the pack's frame function's alone: the cards unit says whose frame function
//     it is calling (op1f_cards_calling), and any other call is refused, so that nothing of the disk is touched from
//     the thread that handles keys or from the audio thread. A write goes to a temporary file of the base's in tmp/
//     (tmp/op1f_NN.part, NN the card's place in the list) and is renamed over the pack's file, as the firmware writes
//     its own; it is refused while the disk mode is open; and it is held by the file's size afterwards, since the
//     firmware's own close of such a file answers 1 whatever its rename answered.
//
// The firmware's calls, each as our code meets it: the disk mode's folder maker 0x8fcaeb10 and its file
// adder 0x8fcaed68; the heap 0x8d4e63bc; the file class's constructor 0x8fcbabb8, whose twelve virtuals a file of
// ours takes with the delete ours; stock's removal 0x8fcbefb4; the size 0x8c051f9c (-1 for no file, no line);
// createFolderRecursive 0x8c05206c (1 when every part is there); 0x8c052054 (1 for a path that opens as a folder);
// the folder listing 0x8c051e28, 0x8c0a4bd0 (an entry's name at +11), 0x8c0a57d4; the small file calls the file class
// uses, open 0x8fcba988(path, mode) (a handle, -1 with a line), read 0x8fcba954 and write 0x8fcba8c0 (buffer, size,
// count, the handle as the fourth argument; the count done; the write's failure line is the firmware's own `write
// error: %s`), close 0x8fcba8ac; rename 0x8c051f88(from, to) (1 when it
// was made); and the word [0x84d25bcc], not 0 while the disk mode is open. Each takes a C string: no stock string is
// made here, so none has to be given back. Nothing is divided (the module links no runtime). Compiled for the PC as
// it is, with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for tests this tree does not
// hold).

#include "build_r3_base.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*MakeFolderFn)(void* parent, const char* name);              // 0x8fcaeb10: a folder of the disk mode's tree
typedef void* (*AddFileFn)(void* folder, const char* name);                  // 0x8fcaed68: a file of the disk mode's tree
typedef void* (*HeapAllocFn)(u32 bytes);                                     // 0x8d4e63bc
typedef void (*FsFileCtorFn)(void* obj, const char* path);                   // 0x8fcbabb8: the filesystem-backed file object (0x40 bytes)
typedef int (*FileSizeFn)(const char* path);                                 // 0x8c051f9c: -1 when there is no file
typedef int (*FileRemoveFn)(const char* path);                               // 0x8fcbefb4
typedef int (*MakeFoldersFn)(const char* path);                              // 0x8c05206c: createFolderRecursive
typedef int (*IsFolderFn)(const char* path);                                 // 0x8c052054: 1 when the path opens as a folder
typedef void* (*DirOpenFn)(const char* path);                                // 0x8c051e28
typedef void* (*DirNextFn)(void* handle);                                    // 0x8c0a4bd0: an entry, its name at +11; 0 at the end
typedef int (*DirCloseFn)(void* handle);                                     // 0x8c0a57d4
typedef int (*OpenFn)(const char* path, const char* mode);                   // 0x8fcba988: a handle, -1 when it cannot be opened
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);        // 0x8fcba954: the count read (on to the file system's read, 0x8c0a4c10, 0x8c0a49e8)
typedef int (*WriteFn)(const void* data, u32 size, u32 count, int handle);   // 0x8fcba8c0: the count written (on to its write, 0x8c0a4e54, 0x8c0a4c28)
typedef void (*CloseFn)(int handle);                                         // 0x8fcba8ac
typedef int (*RenameFn)(const char* from, const char* to);                   // 0x8c051f88: 1 when it was made

#ifdef OP1F_HOST
#include "mock_files_r3.h"                             // the PC's stand-ins: the logger, a model of the disk and of the disk mode's tree, the word read
#define BARRIER() ((void)0)                            // the PC's run is one thread
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MAKE_FOLDER ((MakeFolderFn)0x8fcaeb10u)
#define STOCK_ADD_FILE ((AddFileFn)0x8fcaed68u)
#define STOCK_HEAP_ALLOC ((HeapAllocFn)0x8d4e63bcu)
#define STOCK_FS_FILE_CTOR ((FsFileCtorFn)0x8fcbabb8u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_REMOVE ((FileRemoveFn)0x8fcbefb4u)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_IS_FOLDER ((IsFolderFn)0x8c052054u)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)
#define STOCK_READ ((ReadFn)0x8fcba954u)                // M3's try on a unit found the two swapped: the write read
#define STOCK_WRITE ((WriteFn)0x8fcba8c0u)              // the empty temporary file and answered 0
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)
#define STOCK_RENAME ((RenameFn)0x8c051f88u)
#define STOCK_FS_EMPTY_DELETE ((u32)0x8fcba894u)
#define RD32(a) (*(volatile u32*)(a))
// The compiler moves no load or store across this: the table is stored before the word that says it was settled.
#define BARRIER() asm volatile("" : : : "memory")
#endif

static const u32 DISK_OPEN = 0x84d25bccu;              // not 0 while the disk mode is open (mtp::init writes it, mtp::shutdown clears it)
enum { ALL = 64, TEXT = OP1F_FOLDER_TEXT, NAME = OP1F_FILE_NAME, PATH = 64, FILES_MOST = 64, DIR_NAME_AT = 11, FS_FILE_BYTES = 0x40 };
enum { FS_FILE_SLOTS = 12, FS_DELETE_SLOT = 7, FS_PATH_OWN = 8, FS_PATH_ROOM = 0x1c };
enum { CARD_3_BYTES = OP1F_CARD_LEVEL_3_BYTES, MOST = OP1F_SLOTS_MOST, LARGEST = 1 << 20, LINES_SAID = 16 };

static const char LOG_FILE[] = "../src/main/op1f_folders.cpp";
static const char ROOT[] = "user/pf/";                 // every pack's folders lie in it
static const char TEMPORARY[] = "tmp/";                // the firmware's own folder for its temporary files
static const char WRITE_MODE[] = "w+b";                // texts the open compares with its own table: a new file, written
static const char READ_MODE[] = "rb";
static const char ROOT_FOLDER[] = "user/pf";            // R3: ROOT as a folder is opened, without the slash
static const char BASE_TEMPORARY[] = "tmp/op1f_base.part";   // R3: the base's own temporary file
static const char BASE_NAME[] = "base";                // R3: the name the base's own writes are said by

// The names a pack's folder is never shown under: the firmware's at the top of the disk mode (its root child maker
// 0x8fcacea0 takes these four), the loader's, and the base's own (R10: the firmware's log files,
// module/build_r10_logs.cpp).
static const char* const FIRMWARE_NAMES[] = { "synth", "drum", "tapes", "mixdown" };
static const char LOADER_NAME[] = "packs";
static const char LOGS_NAME[] = "logs";

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).
extern "C" int op1f_cards_slot(const Op1fCard* card) OP1F_NAMED("op1f_cards_slot");
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");

// One folder of a card's list, as settled.
struct Folder {
    u32 slot, index;                                   // the card's place in the list, the folder's in the card's
    char name[TEXT], place[TEXT];
    u32 shown;                                         // the computer sees it
    u32 kept;                                          // its place is this folder's: the pack has it
    u32 made;                                          // it is on the disk
};

static Folder folders[ALL];
static u32 folder_count;
static volatile u32 settled;
static const void* behaviours[ALL][2];                 // a shown folder's behaviour in the disk mode's tree: the table, the folder's number
static char listed[FILES_MOST][NAME];                  // the disk mode's thread's own: the names of one folder as it adds them

static struct {
    u32 written, write_failures, refused_now, refused_disk, read_failures, lines;   // since the start, for the lines' budget
    u32 temporary_made;
    u32 deleted, brought;                              // the computer's deletes and new files since the start: a line for each of the first DISK_LINES
    u32 removed, remove_failures;                      // R1: the packs' removals since the start: a line for each of the first LINES_SAID, and for every failure
    u32 base_written, base_failures, base_refused_disk;   // R3: the base's own writes since the start: a line for each of the first LINES_SAID of each kind
} files;

enum { DISK_LINES = 32 };                               // the review of M3: hundreds of files copied are not hundreds of lines; a failure is always said

// ---------------------------------------------------------------- texts

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static char folded(char c) {
    return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c;
}

// The same, capitals counted as small letters.
static int same_folded(const char* a, const char* b) {
    while (*a && folded(*a) == folded(*b)) { ++a; ++b; }
    return folded(*a) == folded(*b);
}

// A folder's name or place: one to fifteen characters of A-Z, a-z, 0-9, space, '_', '-' and '.', ended within its
// sixteen bytes, not beginning with a space, a dot or a dash and not ending with a space or a dot.
static int folder_text(const char* text) {
    u32 n = 0;
    for (; n < (u32)TEXT && text[n]; ++n) {
        char c = text[n];
        int letter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (!letter && c != ' ' && c != '_' && c != '-' && c != '.') return 0;
    }
    if (n == 0 || n == (u32)TEXT) return 0;
    if (text[0] == ' ' || text[0] == '.' || text[0] == '-') return 0;
    if (text[n - 1] == ' ' || text[n - 1] == '.') return 0;
    return 1;
}

// A file's name in a pack's folder: one to thirty-nine plain characters, no slash, not beginning with a dot.
static int file_name(const char* name) {
    if (name == 0 || name[0] == 0 || name[0] == '.') return 0;
    u32 n = 0;
    for (; name[n]; ++n) {
        if (n == (u32)NAME - 1) return 0;
        if ((u8)name[n] < 0x20 || (u8)name[n] > 0x7e || name[n] == '/') return 0;
    }
    return 1;
}

static u32 put(char* out, u32 at, const char* text) {
    while (*text && at < (u32)PATH - 1) out[at++] = *text++;
    out[at] = 0;
    return at;
}

// user/pf/ and the place, with the slash behind (`slash`) or without it.
static void folder_path(char* out, const Folder* f, int slash) {
    u32 at = put(out, 0, ROOT);
    at = put(out, at, f->place);
    if (slash) put(out, at, "/");
}

// user/pf/, the place, a slash and the name: at most 8 + 15 + 1 + 39 characters and the end, the sixty-four bytes.
static void file_path(char* out, const Folder* f, const char* name) {
    folder_path(out, f, 1);
    u32 at = 0;
    while (out[at]) ++at;
    put(out, at, name);
}

// ---------------------------------------------------------------- the settling

static const Op1fFolder* list_of(const Op1fCard* card, u32* count) {
    int level_3 = card->bytes >= (u32)CARD_3_BYTES && card->level >= 3u;
    *count = level_3 ? card->folder_count : 0u;
    return level_3 ? card->folders : 0;
}

// Why a folder is not shown or not the pack's, by its name and place against the ones before it: 0 when it is
// shown and kept. `other` names the earlier folder's pack where an earlier folder is the cause. The name of a folder
// the card does not ask to be shown is never shown, and is held to nothing but the rule of a folder's text, as the
// tool holds it (since the review of M3).
static const char* refused(const Folder* f, u32 upto, const char** other, u32* kept, u32 asked) {
    *kept = 1;
    *other = "";
    for (u32 j = 0; j < upto; ++j) {
        if (folders[j].kept && same_folded(f->place, folders[j].place)) {
            *kept = 0;
            *other = op1f_cards_2.slots[folders[j].slot].card->name;
            return "NOT the pack's: its place is the place of an earlier folder, of";
        }
    }
    if (!asked) return 0;
    for (u32 i = 0; i < sizeof FIRMWARE_NAMES / sizeof FIRMWARE_NAMES[0]; ++i)
        if (same_folded(f->name, FIRMWARE_NAMES[i])) return "NOT shown: the name is the firmware's";
    if (same_folded(f->name, LOADER_NAME)) return "NOT shown: the name is the loader's";
    if (same_folded(f->name, LOGS_NAME)) return "NOT shown: the name is the base's";
    for (u32 j = 0; j < upto; ++j) {
        if (folders[j].shown && same_folded(f->name, folders[j].name)) {
            *other = op1f_cards_2.slots[folders[j].slot].card->name;
            return "NOT shown: the name is an earlier folder's, of";
        }
    }
    return 0;
}

// Once, at the base's first frame, on the display thread: every card's folders held to the rules and made.
extern "C" void op1f_folders_settle() OP1F_NAMED("op1f_folders_settle");
extern "C" void op1f_folders_settle() {
    if (settled) return;
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)MOST ? (u32)MOST : op1f_cards_2.count;
    u32 cards = 0, shown = 0, hidden = 0, lost = 0, more = 0;
    for (u32 i = 0; i < seen; ++i) {
        if (!op1f_cards_in_use(i)) continue;   // R20: any of the 128
        const Op1fCard* card = op1f_cards_2.slots[i].card;
        u32 count = 0;
        const Op1fFolder* list = list_of(card, &count);
        if (list == 0 || count == 0) continue;                                       // most packs: no folder, nothing of the disk touched
        ++cards;
        for (u32 k = 0; k < count; ++k) {
            if (folder_count == (u32)ALL) { ++more; continue; }
            Folder* f = &folders[folder_count];
            f->slot = i;
            f->index = k;
            f->shown = f->kept = f->made = 0;
            int plain = folder_text(list[k].name) && folder_text(list[k].place);
            for (u32 c = 0; c < (u32)TEXT; ++c) {
                f->name[c] = plain ? list[k].name[c] : 0;
                f->place[c] = plain ? list[k].place[c] : 0;
            }
            const char* why = "NOT the pack's: its name or its place is not a folder's text";
            const char* other = "";
            u32 kept = 0;
            if (plain) why = refused(f, folder_count, &other, &kept, list[k].shown);
            f->kept = kept;
            f->shown = kept && why == 0 && list[k].shown != 0;
            char path[PATH];
            const char* disk = "nothing made";
            if (kept) {
                char open[PATH];
                folder_path(open, f, 0);                                             // a folder is opened without the slash, as stock opens one
                folder_path(path, f, 1);
                int there = STOCK_IS_FOLDER(open);
                int made = STOCK_MAKE_FOLDERS(path);                                 // user/pf/ and the place, the slash behind
                f->made = made != 0 && STOCK_IS_FOLDER(open) != 0;
                disk = !f->made ? "could NOT be made on the disk" : there ? "already on the disk" : "made on the disk";
            }
            const char* says = why ? why : list[k].shown ? "shown in the disk mode" : "not shown, as the card says";
            if (f->shown) ++shown; else if (kept) ++hidden; else ++lost;
            STOCK_LOG(why && !kept ? 1 : (why || !f->made) ? 2 : 3, LOG_FILE, 1, "folders: '%s' folder %u of %u, '%s' at %s%s/: %s%s%s%s, %s",
                      card->name, k + 1, count, plain ? f->name : "?", ROOT, plain ? f->place : "?", says, other[0] ? " '" : "", other, other[0] ? "'" : "", disk);
            ++folder_count;
        }
    }
    for (u32 n = 0; n < folder_count; ++n) {
        behaviours[n][0] = 0;
        behaviours[n][1] = (const void*)n;
    }
    BARRIER();                                                                      // the table is whole before the word that says so
    settled = 1;
    if (cards != 0 || more != 0)
        STOCK_LOG(3, LOG_FILE, 2, "folders: %u folder(s) of %u card(s): %u shown in the disk mode, %u the pack's alone, %u not the pack's, %u more than the base holds",
                  folder_count, cards, shown, hidden, lost, more);
}

// The folder a card has under a number, kept and made, or 0.
static Folder* folder_of(const Op1fCard* card, u32 index, int* slot) {
    *slot = op1f_cards_slot(card);
    if (*slot < 0 || !settled) return 0;
    for (u32 n = 0; n < folder_count; ++n) {
        Folder* f = &folders[n];
        if (f->slot == (u32)*slot && f->index == index) return f->kept && f->made ? f : 0;
    }
    return 0;
}

// ---------------------------------------------------------------- the files, for the packs

static const char* pack_name(int slot) {
    return op1f_cards_2.slots[slot].card->name;
}

// May this pack's frame function work on this folder now? 0, or why not.
static int may(const Op1fCard* card, u32 index, Folder** f, int* slot) {
    *f = folder_of(card, index, slot);
    if (*f == 0) return OP1F_NO_FOLDER;
    if (op1f_cards_calling() != (u32)*slot + 1u) {
        ++files.refused_now;
        if (files.lines < (u32)LINES_SAID) {
            ++files.lines;
            STOCK_LOG(2, LOG_FILE, 3, "files: '%s' asked for its folder %u while the base was not calling its frame function: refused", pack_name(*slot), index);
        }
        return OP1F_NOT_NOW;
    }
    return 0;
}

extern "C" int op1f_path(const Op1fCard* card, unsigned folder, const char* name, char* out, unsigned room) {
    int slot = 0;
    Folder* f = folder_of(card, folder, &slot);
    if (f == 0) return OP1F_NO_FOLDER;
    if (name != 0 && !file_name(name)) return OP1F_BAD_NAME;
    char path[PATH];
    if (name == 0) folder_path(path, f, 1); else file_path(path, f, name);
    u32 n = 0;
    while (path[n]) ++n;
    if (out == 0 || n + 1 > room) return OP1F_TOO_LARGE;
    for (u32 i = 0; i <= n; ++i) out[i] = path[i];
    return (int)n;
}

extern "C" int op1f_list(const Op1fCard* card, unsigned folder, char* names, unsigned room) {
    Folder* f = 0;
    int slot = 0, why = may(card, folder, &f, &slot);
    if (why) return why;
    char path[PATH];
    folder_path(path, f, 0);                                                        // a folder is opened without the slash
    void* dir = STOCK_DIR_OPEN(path);
    if (dir == 0) {
        STOCK_LOG(1, LOG_FILE, 4, "files: '%s' could NOT list %s", pack_name(slot), path);
        return OP1F_FAILED;
    }
    u32 at = 0, count = 0;                                                          // the names first, the folder closed, then the folders among them taken out
    for (;;) {
        const char* entry = (const char*)STOCK_DIR_NEXT(dir);
        if (!entry) break;
        const char* name = entry + DIR_NAME_AT;
        if (!file_name(name)) continue;
        u32 n = 0;
        while (name[n]) ++n;
        if (names == 0 || at + n + 1 > room) continue;                              // as many as fit whole
        for (u32 i = 0; i <= n; ++i) names[at + i] = name[i];
        at += n + 1;
        ++count;
    }
    STOCK_DIR_CLOSE(dir);
    u32 from = 0, to = 0, kept = 0;
    for (u32 k = 0; k < count; ++k) {
        u32 n = 0;
        while (names[from + n]) ++n;
        char file[PATH];
        file_path(file, f, names + from);
        if (!STOCK_IS_FOLDER(file)) {
            for (u32 i = 0; i <= n; ++i) names[to + i] = names[from + i];
            to += n + 1;
            ++kept;
        }
        from += n + 1;
    }
    return (int)kept;
}

extern "C" int op1f_read(const Op1fCard* card, unsigned folder, const char* name, void* buffer, unsigned room) {
    Folder* f = 0;
    int slot = 0, why = may(card, folder, &f, &slot);
    if (why) return why;
    if (!file_name(name)) return OP1F_BAD_NAME;
    char path[PATH];
    file_path(path, f, name);
    int size = STOCK_FILE_SIZE(path);                                               // no file: -1, and no line from the firmware
    if (size < 0) return OP1F_NO_FILE;
    if ((u32)size > room) return OP1F_TOO_LARGE;
    if (size == 0) return 0;
    if (buffer == 0) return OP1F_TOO_LARGE;
    int handle = STOCK_OPEN(path, READ_MODE);
    int got = handle < 0 ? 0 : STOCK_READ(buffer, 1, (u32)size, handle);
    if (handle >= 0) STOCK_CLOSE(handle);
    if (got != size) {
        ++files.read_failures;
        if (files.read_failures <= (u32)LINES_SAID)
            STOCK_LOG(1, LOG_FILE, 5, "files: '%s' could NOT read %s: %s (%d of its %d bytes)", pack_name(slot), path, handle < 0 ? "it could not be opened" : "it was not read whole",
                      got, size);
        return OP1F_FAILED;
    }
    return got;
}

// A write that did not land: said, counted, and its temporary file taken off the disk.
static int failed(int slot, const char* path, const char* temporary, const char* step, int a, int b) {
    STOCK_FILE_REMOVE(temporary);
    ++files.write_failures;
    if (files.write_failures <= (u32)LINES_SAID)
        STOCK_LOG(1, LOG_FILE, 6, "files: '%s' could NOT write %s: %s (%d, %d)", pack_name(slot), path, step, a, b);
    return OP1F_FAILED;
}

extern "C" int op1f_write(const Op1fCard* card, unsigned folder, const char* name, const void* data, unsigned bytes) {
    Folder* f = 0;
    int slot = 0, why = may(card, folder, &f, &slot);
    if (why) return why;
    if (!file_name(name)) return OP1F_BAD_NAME;
    if (bytes > (u32)LARGEST || (data == 0 && bytes != 0)) return OP1F_TOO_LARGE;
    char path[PATH];
    file_path(path, f, name);
    if (RD32(DISK_OPEN) != 0) {                                                     // the computer owns the folders now
        ++files.refused_disk;
        if (files.refused_disk <= (u32)LINES_SAID)
            STOCK_LOG(2, LOG_FILE, 7, "files: '%s' asked to write %s while the disk mode is open: refused", pack_name(slot), path);
        return OP1F_NOT_NOW;
    }
    if (!files.temporary_made) files.temporary_made = STOCK_MAKE_FOLDERS(TEMPORARY) != 0 ? 1u : 0u;   // the firmware's own, there on every unit; made if not
    char temporary[PATH];
    u32 at = put(temporary, 0, TEMPORARY);
    at = put(temporary, at, "op1f_");
    u32 ones = (u32)slot, tens = 0;                                                 // the card's place in two digits, by subtraction: nothing is divided
    while (ones >= 10u) { ones -= 10u; ++tens; }
    temporary[at++] = (char)('0' + tens);
    temporary[at++] = (char)('0' + ones);
    put(temporary, at, ".part");
    int handle = STOCK_OPEN(temporary, WRITE_MODE);
    if (handle < 0) return failed(slot, path, temporary, "the temporary file could not be opened", handle, 0);
    int done = bytes ? STOCK_WRITE(data, 1, bytes, handle) : 0;
    STOCK_CLOSE(handle);
    if (done != (int)bytes) return failed(slot, path, temporary, "the temporary file was not written whole", done, (int)bytes);
    int renamed = STOCK_RENAME(temporary, path);
    if (!renamed) return failed(slot, path, temporary, "it could not be renamed over the pack's file", renamed, 0);
    int size = STOCK_FILE_SIZE(path);
    if (size != (int)bytes) return failed(slot, path, temporary, "the file's size afterwards is not what was written", size, (int)bytes);
    ++files.written;
    if (files.written <= (u32)LINES_SAID)
        STOCK_LOG(3, LOG_FILE, 8, "files: '%s' wrote %s, %u bytes: it landed (%u written since the start)", pack_name(slot), path, bytes, files.written);
    return (int)bytes;
}

// R1: a file of the pack's folder taken off the disk. The removal's answer is said, never trusted: the size before
// and after says what the disk holds.
extern "C" int op1f_remove(const Op1fCard* card, unsigned folder, const char* name) {
    Folder* f = 0;
    int slot = 0, why = may(card, folder, &f, &slot);
    if (why) return why;
    if (!file_name(name)) return OP1F_BAD_NAME;
    char path[PATH];
    file_path(path, f, name);
    if (RD32(DISK_OPEN) != 0u) {                                                    // the computer owns the folders now
        ++files.refused_disk;
        if (files.refused_disk <= (u32)LINES_SAID)
            STOCK_LOG(2, LOG_FILE, 14, "files: '%s' asked to remove %s while the disk mode is open: refused", pack_name(slot), path);
        return OP1F_NOT_NOW;
    }
    if (STOCK_IS_FOLDER(path) || STOCK_FILE_SIZE(path) < 0) return OP1F_NO_FILE;    // a folder inside is no file; no file: -1, and no line from the firmware
    int answered = STOCK_FILE_REMOVE(path);
    int still = STOCK_FILE_SIZE(path);                                              // what the disk holds now, whatever the removal answered
    if (still >= 0) {
        ++files.remove_failures;
        STOCK_LOG(1, LOG_FILE, 15, "files: '%s' could NOT remove %s: the file is STILL there (the removal answered %d, the size is %d)", pack_name(slot), path, answered, still);
        return OP1F_FAILED;
    }
    ++files.removed;
    if (files.removed <= (u32)LINES_SAID)
        STOCK_LOG(3, LOG_FILE, 16, "files: '%s' removed %s: it is gone (%u removed since the start)", pack_name(slot), path, files.removed);
    return 0;
}

// ---------------------------------------------------------------- R3: a file of the base's own

// A path of the base's own file: user/pf/ and a name of two to thirty-nine plain characters that begins with a dot,
// with no slash, and not "..".
static int base_path(const char* path) {
    if (path == 0) return 0;
    u32 at = 0;
    for (; ROOT[at]; ++at)
        if (path[at] != ROOT[at]) return 0;
    const char* name = path + at;
    if (name[0] != '.' || name[1] == 0 || (name[1] == '.' && name[2] == 0)) return 0;
    for (u32 n = 0; name[n]; ++n) {
        if (n == (u32)NAME - 1) return 0;
        if ((u8)name[n] < 0x20 || (u8)name[n] > 0x7e || name[n] == '/') return 0;
    }
    return 1;
}

// A write of the base's own that did not begin: said, counted, nothing on the disk touched.
static int base_refused(const char* path, const char* step, int a, int b) {
    ++files.base_failures;
    if (files.base_failures <= (u32)LINES_SAID)
        STOCK_LOG(1, LOG_FILE, 17, "files: '%s' could NOT write %s: %s (%d, %d)", BASE_NAME, path, step, a, b);
    return OP1F_FAILED;
}

// A write of the base's own that did not land: said, counted, and its temporary file taken off the disk.
static int base_failed(const char* path, const char* step, int a, int b) {
    STOCK_FILE_REMOVE(BASE_TEMPORARY);
    return base_refused(path, step, a, b);
}

extern "C" int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) {
    if (!base_path(path)) return base_refused("?", "it is not a file of the base's own", (int)bytes, 0);
    if (bytes > (u32)LARGEST || (data == 0 && bytes != 0)) return base_refused(path, "what was handed is not a file's bytes", (int)bytes, LARGEST);
    if (RD32(DISK_OPEN)) {                                                          // the computer owns the folders now
        ++files.base_refused_disk;
        return OP1F_NOT_NOW;
    }
    if (!STOCK_IS_FOLDER(ROOT_FOLDER) && (!STOCK_MAKE_FOLDERS(ROOT) || !STOCK_IS_FOLDER(ROOT_FOLDER)))
        return base_refused(path, "user/pf/ could not be made", 0, 0);
    if (!files.temporary_made) files.temporary_made = STOCK_MAKE_FOLDERS(TEMPORARY) != 0 ? 1u : 0u;   // the firmware's own, there on every unit; made if not
    const char* temporary = BASE_TEMPORARY;
    int handle = STOCK_OPEN(temporary, WRITE_MODE);
    if (handle < 0) return base_failed(path, "the temporary file could not be opened", handle, 0);
    int done = bytes ? STOCK_WRITE(data, 1, bytes, handle) : 0;
    STOCK_CLOSE(handle);
    if (done != (int)bytes) return base_failed(path, "the temporary file was not written whole", done, (int)bytes);
    int renamed = STOCK_RENAME(temporary, path);
    if (!renamed) return base_failed(path, "it could not be renamed over the base's file", renamed, 0);
    int size = STOCK_FILE_SIZE(path);
    if (size != (int)bytes) return base_failed(path, "the file's size afterwards is not what was written", size, (int)bytes);
    ++files.base_written;
    if (files.base_written <= (u32)LINES_SAID)
        STOCK_LOG(3, LOG_FILE, 18, "files: '%s' wrote %s, %u bytes: it landed (%u written since the start)", BASE_NAME, path, bytes, files.base_written);
    return (int)bytes;
}

// ---------------------------------------------------------------- the disk mode

static u32 file_vtable[FS_FILE_SLOTS];                   // stock's twelve slots with the delete ours; made at the first file object

// The delete slot: (object). The object's path is the stock string at +4: its own buffer at +8 while the room word at
// +0x1c is under sixteen, else a pointer there (as stock's own open, close, size and time virtuals read it).
extern "C" void op1f_folders_file_delete(void* obj) OP1F_NAMED("op1f_folders_file_delete");
extern "C" void op1f_folders_file_delete(void* obj) {
    const char* own = (const char*)obj + FS_PATH_OWN;
    const char* path = *(const u32*)((const char*)obj + FS_PATH_ROOM) >= 16 ? *(const char* const*)own : own;
    int answer = STOCK_FILE_REMOVE(path);
    int left = STOCK_FILE_SIZE(path);                                               // what the disk holds now, whatever the removal answered
    if (left < 0 && ++files.deleted > (u32)DISK_LINES) return;
    STOCK_LOG(left < 0 ? 3 : 1, LOG_FILE, 9, "folders: %s deleted from the computer: %s (the removal answered %d, the size is %d)",
              path, left < 0 ? "the file is gone" : "the file is STILL there", answer, left);
}

// A file object fresh from stock's constructor gets the delete above.
static void give_delete(void* obj) {
    u32* stock = *(u32**)obj;
    if (!file_vtable[0]) {
        if (stock[FS_DELETE_SLOT] != STOCK_FS_EMPTY_DELETE) return;                 // not the class as it was read: left as stock made it
        for (int i = 0; i < FS_FILE_SLOTS; ++i) file_vtable[i] = stock[i];
        file_vtable[FS_DELETE_SLOT] = (u32)&op1f_folders_file_delete;
    }
    *(u32**)obj = file_vtable;
}

extern "C" void op1f_folders_noop(void* self) OP1F_NAMED("op1f_folders_noop");
extern "C" void op1f_folders_noop(void* self) { (void)self; }                    // the destructors (the object is static) and the +8 slot

extern "C" void* op1f_folders_zero(void* self, const char* name) OP1F_NAMED("op1f_folders_zero");
extern "C" void* op1f_folders_zero(void* self, const char* name) { (void)self; (void)name; return 0; }   // +0xc, and +0x10: no folder inside

// +0x14: a file of the folder as stock's filesystem-backed file object. A name the computer brings gets its file
// first, empty: the object stands on a file that exists, as since J3, and the computer's bytes replace it through
// stock's class (a temporary and a rename).
extern "C" void* op1f_folders_lookup(void* self, const char* name) OP1F_NAMED("op1f_folders_lookup");
extern "C" void* op1f_folders_lookup(void* self, const char* name) {
    u32 n = (u32)((const void* const*)self)[1];
    if (n >= folder_count || !file_name(name)) return 0;                            // refused, and stock says so
    char path[PATH];
    file_path(path, &folders[n], name);
    if (STOCK_FILE_SIZE(path) < 0) {
        int handle = STOCK_OPEN(path, WRITE_MODE);
        if (handle >= 0) STOCK_CLOSE(handle);
        if (handle < 0 || ++files.brought <= (u32)DISK_LINES)
            STOCK_LOG(handle >= 0 ? 3 : 1, LOG_FILE, 10, "folders: the computer brings a new file, %s: %s", path,
                      handle >= 0 ? "a place is made for it" : "its place could NOT be made");
    }
    void* obj = STOCK_HEAP_ALLOC(FS_FILE_BYTES);
    if (!obj) return 0;
    STOCK_FS_FILE_CTOR(obj, path);
    give_delete(obj);
    return obj;
}

typedef void* (*BehaviourFn)();
static const BehaviourFn folder_behaviour_vtable[6] = {
    (BehaviourFn)op1f_folders_noop,                                                 // the destructor
    (BehaviourFn)op1f_folders_noop,                                                 // the deleting destructor: nothing to free, the object is the base's
    (BehaviourFn)op1f_folders_noop,                                                 // +8
    (BehaviourFn)op1f_folders_zero,                                                 // +0xc
    (BehaviourFn)op1f_folders_zero,                                                 // +0x10: no folder inside
    (BehaviourFn)op1f_folders_lookup,                                               // +0x14: the files
};

// The base's root child maker (module/build_r2_disk.cpp), for every name it is handed: a shown folder's
// behaviour, or 0. On the thread that builds the disk mode's tree.
extern "C" void* op1f_folders_child(const char* name) OP1F_NAMED("op1f_folders_child");
extern "C" void* op1f_folders_child(const char* name) {
    if (name == 0 || !settled) return 0;
    for (u32 n = 0; n < folder_count; ++n) {
        if (!folders[n].shown || !folders[n].made || !same(name, folders[n].name)) continue;
        behaviours[n][0] = folder_behaviour_vtable;
        return (void*)behaviours[n];
    }
    return 0;
}

// The base's population: each shown folder made in the tree with its files.
extern "C" void op1f_folders_populate(void* root) OP1F_NAMED("op1f_folders_populate");
extern "C" void op1f_folders_populate(void* root) {
    if (!settled) {
        STOCK_LOG(2, LOG_FILE, 11, "folders: the disk mode's tree was built before the base's first frame: no folder of a pack is shown");
        return;
    }
    for (u32 n = 0; n < folder_count; ++n) {
        Folder* f = &folders[n];
        if (!f->shown || !f->made) continue;
        void* node = STOCK_MAKE_FOLDER(root, f->name);                              // through the root child maker, and op1f_folders_child
        char path[PATH];
        folder_path(path, f, 0);
        if (node == 0) {
            STOCK_LOG(1, LOG_FILE, 12, "folders: '%s' of '%s' is NOT in the disk mode's tree: the firmware's folder maker refused it", f->name, pack_name((int)f->slot));
            continue;
        }
        u32 count = 0, more = 0, inside = 0, other = 0;
        void* dir = STOCK_DIR_OPEN(path);
        if (dir) {                                                                  // the names first, the folder closed, then the files
            for (;;) {
                const char* entry = (const char*)STOCK_DIR_NEXT(dir);
                if (!entry) break;
                const char* name = entry + DIR_NAME_AT;
                if (!file_name(name)) {
                    if (name[0] != '.') ++other;
                } else if (count == (u32)FILES_MOST) {
                    ++more;
                } else {
                    u32 at = 0;
                    for (; name[at]; ++at) listed[count][at] = name[at];
                    listed[count++][at] = 0;
                }
            }
            STOCK_DIR_CLOSE(dir);
        }
        u32 files_added = 0, files_seen = 0;
        for (u32 i = 0; i < count; ++i) {
            char file[PATH];
            file_path(file, f, listed[i]);
            if (STOCK_IS_FOLDER(file)) { ++inside; continue; }
            ++files_seen;
            if (STOCK_ADD_FILE(node, listed[i])) ++files_added;                    // each through op1f_folders_lookup
        }
        STOCK_LOG(dir && files_added == files_seen ? 3 : 1, LOG_FILE, 13,
                  "folders: the disk mode shows '%s' with %u of the %u file(s) of %s/ (%s); %u more, %u folder(s) inside and %u other name(s) not shown",
                  f->name, files_added, files_seen, path, dir ? "listed" : "NOT listed", more, inside, other);
    }
}

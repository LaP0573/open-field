// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R10, the base pack (base.pack 34): the firmware's own log files in the disk mode. A unit of its own, new at
// R10. The firmware writes its log into the folder logs/ at the top of the volume, plain text, one file a start named
// log_<date>_<time>_<n>.txt and a new one each 512 KiB. This unit shows those files in the unit's normal disk mode as
// a folder `logs` at the top, beside synth, drum, tapes, mixdown and packs. Nothing is copied and nothing new is
// written to the disk: each file the computer sees is the firmware's own file in logs/, read where it lies.
//
//   op1f_logs_child(name): the folder's behaviour for the name `logs`, else 0. The base's root child maker
//     (module/build_r2_disk.cpp) asks it first, before the packs' folders and the firmware's own.
//   op1f_logs_populate(root): called by the base's population before the packs' folders. logs/ is listed, each name
//     held to the rule of a log file's name (log_ ... .txt, printable, no space, no slash, no backslash, no `..`),
//     sorted (the names are dates: the last is the newest) and the newest sixty-four kept when there are more; then
//     the folder `logs` is made in the tree through the firmware's folder maker (which reaches the root child maker,
//     and so op1f_logs_child) and each kept name added through its file adder. One line says what is shown.
//   The folder's behaviour answers a file by its name only for a name of that list, and once a tree (the file adder's
//     call): the filesystem-backed file object on logs/<name>, its delete ours. Any other name, a file the computer
//     brings among them or brings back after a delete, gets 0: the firmware refuses it and no new file is made in
//     logs/. No folder inside is made.
//   The delete: the newest listed file is the running start's log and is never removed (a line says the computer
//     asked and it was kept); any other is removed and its size asked again, which says what the disk holds, as the
//     packs' folders' delete does (module/build_r2_folders.cpp). The first sixteen deletes are said, and every one
//     that failed.
//
// The disk mode's tree is built on the thread that handles keys and the computer's requests come on the USB device's
// thread; nothing here is shared with a pack's frame function. The firmware's calls are those of
// module/build_r2_folders.cpp, at the same addresses: the folder maker, the file adder, the heap, the file class's
// constructor (and its empty delete, which ours takes the place of), the removal, the size, the folder listing (an
// entry's name at +11) and the logger. Each takes a C string. Nothing is divided (the module links no runtime).
// Compiled for the PC as it is, with OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for
// tests this tree does not hold).

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*MakeFolderFn)(void* parent, const char* name);              // 0x8fcaeb10: a folder of the disk mode's tree
typedef void* (*AddFileFn)(void* folder, const char* name);                  // 0x8fcaed68: a file of the disk mode's tree
typedef void* (*HeapAllocFn)(u32 bytes);                                     // 0x8d4e63bc
typedef void (*FsFileCtorFn)(void* obj, const char* path);                   // 0x8fcbabb8: the filesystem-backed file object (0x40 bytes)
typedef int (*FileSizeFn)(const char* path);                                 // 0x8c051f9c: -1 when there is no file
typedef int (*FileRemoveFn)(const char* path);                               // 0x8fcbefb4
typedef void* (*DirOpenFn)(const char* path);                                // 0x8c051e28: 0 when the folder cannot be opened
typedef void* (*DirNextFn)(void* handle);                                    // 0x8c0a4bd0: an entry, its name at +11; 0 at the end
typedef int (*DirCloseFn)(void* handle);                                     // 0x8c0a57d4

#ifndef OP1F_NAMED
#ifdef OP1F_HOST
#define OP1F_NAMED(x)
#else
#define OP1F_NAMED(x) asm(x)
#endif
#endif

#ifdef OP1F_HOST
#include "mock_files_r3.h"                             // the PC's stand-ins: the logger, a model of the disk and of the disk mode's tree
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MAKE_FOLDER ((MakeFolderFn)0x8fcaeb10u)
#define STOCK_ADD_FILE ((AddFileFn)0x8fcaed68u)
#define STOCK_HEAP_ALLOC ((HeapAllocFn)0x8d4e63bcu)
#define STOCK_FS_FILE_CTOR ((FsFileCtorFn)0x8fcbabb8u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_REMOVE ((FileRemoveFn)0x8fcbefb4u)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)
#define STOCK_FS_EMPTY_DELETE ((u32)0x8fcba894u)
#endif

static const char LOG_FILE[] = "../src/main/op1f_logs.cpp";
static const char FOLDER[] = "logs";                   // the firmware's folder at the top of the volume, and the tree's
static const char PREFIX[] = "logs/";

enum { MOST = 64, NAME_MOST = 48, PATH = sizeof PREFIX + NAME_MOST, SHOWN = 56, DIR_NAME_AT = 11, FS_FILE_BYTES = 0x40 };
enum { FS_FILE_SLOTS = 12, FS_DELETE_SLOT = 7, FS_PATH_OWN = 8, FS_PATH_ROOM = 0x1c, LINES_SAID = 16 };

static char listed[MOST][NAME_MOST + 1];               // the log files the tree shows, sorted: the last is the newest
static u32 listed_count;
static u8 given[MOST];                                 // a file object was made for the name in this tree
static char newest[PATH];                              // logs/ and the newest name (the running start's log), or empty
static u32 file_vtable[FS_FILE_SLOTS];                 // stock's twelve slots with the delete ours; made at the first file object
static const void* behaviour[2];                       // the folder's behaviour: its table, and a word of nothing
static struct {
    u32 deleted, refused;                              // since the start: a line for each of the first LINES_SAID
} said;

// ---------------------------------------------------------------- texts

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static int compare(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return (int)(u8)*a - (int)(u8)*b;
}

// The characters of a text, counted up to one past `most`.
static u32 length_of(const char* text, u32 most) {
    u32 n = 0;
    while (n <= most && text[n]) ++n;
    return n;
}

// Whether `count` characters are a log file's name: `log_`, `.txt`, plain, no slash, no backslash, no `..`.
static int log_name(const char* name, u32 count) {
    if (count < 8 || count > (u32)NAME_MOST) return 0;
    if (name[0] != 'l' || name[1] != 'o' || name[2] != 'g' || name[3] != '_') return 0;
    const char* end = name + count - 4;
    if (end[0] != '.' || end[1] != 't' || end[2] != 'x' || end[3] != 't') return 0;
    for (u32 i = 0; i < count; ++i) {
        u32 c = (u8)name[i];
        if (c <= 0x20 || c >= 0x7f || c == '/' || c == '\\') return 0;
        if (c == '.' && i + 1 < count && name[i + 1] == '.') return 0;
    }
    return 1;
}

// A name into a place of the list (both of NAME_MOST + 1 bytes).
static void copy(char* to, const char* from) {
    u32 i = 0;
    for (; i < (u32)NAME_MOST && from[i]; ++i) to[i] = from[i];
    to[i] = 0;
}

// `logs/` and the name: the one way a path of this unit is made, and only from a name log_name passed.
static void log_path(char* out, const char* name) {
    u32 at = 0;
    for (; PREFIX[at]; ++at) out[at] = PREFIX[at];
    for (u32 i = 0; i < (u32)NAME_MOST && name[i]; ++i) out[at++] = name[i];
    out[at] = 0;
}

// Whether a path is logs/ and a log file's name: what every file object of this unit stands on.
static int of_logs(const char* path) {
    u32 at = 0;
    for (; PREFIX[at]; ++at)
        if (path[at] != PREFIX[at]) return 0;
    return log_name(path + at, length_of(path + at, NAME_MOST));
}

// A text for a line: at most SHOWN characters, each plain or `?`, and `...` behind a longer one.
static void held(char* out, const char* text) {
    u32 n = 0;
    for (; n < (u32)SHOWN && text[n]; ++n) out[n] = (u8)text[n] < 0x20 || (u8)text[n] > 0x7e ? '?' : text[n];
    if (text[n]) {
        out[n++] = '.';
        out[n++] = '.';
        out[n++] = '.';
    }
    out[n] = 0;
}

// ---------------------------------------------------------------- the files

// The delete slot: (object). The object's path is the stock string at +4: its own buffer at +8 while the room word at
// +0x1c is under sixteen, else a pointer there (as module/build_r2_folders.cpp reads it).
extern "C" void op1f_logs_file_delete(void* obj) OP1F_NAMED("op1f_logs_file_delete");
extern "C" void op1f_logs_file_delete(void* obj) {
    const char* own = (const char*)obj + FS_PATH_OWN;
    const char* path = *(const u32*)((const char*)obj + FS_PATH_ROOM) >= 16 ? *(const char* const*)own : own;
    const char* kept = !of_logs(path) ? "it is no log file of logs/" : same(path, newest) ? "it is the log of the running start" : 0;
    if (kept) {
        char shown[SHOWN + 4];
        held(shown, path);
        STOCK_LOG(2, LOG_FILE, 4, "logs: the computer asked to delete %s: it was kept, %s", shown, kept);
        return;
    }
    int answer = STOCK_FILE_REMOVE(path);
    int left = STOCK_FILE_SIZE(path);                                               // what the disk holds now, whatever the removal answered
    if (left < 0 && ++said.deleted > (u32)LINES_SAID) return;
    STOCK_LOG(left < 0 ? 3 : 1, LOG_FILE, 5, "logs: %s deleted from the computer: %s (the removal answered %d, the size is %d)",
              path, left < 0 ? "the file is gone" : "the file is STILL there", answer, left);
}

// A file object fresh from stock's constructor gets the delete above.
static void give_delete(void* obj) {
    u32* stock = *(u32**)obj;
    if (!file_vtable[0]) {
        if (stock[FS_DELETE_SLOT] != STOCK_FS_EMPTY_DELETE) return;                 // not the class as it was read: left as stock made it
        for (int i = 0; i < FS_FILE_SLOTS; ++i) file_vtable[i] = stock[i];
        file_vtable[FS_DELETE_SLOT] = (u32)&op1f_logs_file_delete;
    }
    *(u32**)obj = file_vtable;
}

extern "C" void op1f_logs_noop(void* self) OP1F_NAMED("op1f_logs_noop");
extern "C" void op1f_logs_noop(void* self) { (void)self; }                          // the destructors (the object is static) and the +8 slot

extern "C" void* op1f_logs_zero(void* self, const char* name) OP1F_NAMED("op1f_logs_zero");
extern "C" void* op1f_logs_zero(void* self, const char* name) { (void)self; (void)name; return 0; }   // +0xc, and +0x10: no folder inside

// +0x14: a file of the folder by its name. A name of the list, the first time it is asked in this tree: stock's
// filesystem-backed file object on logs/<name>. Anything else: 0, and the firmware refuses it.
extern "C" void* op1f_logs_lookup(void* self, const char* name) OP1F_NAMED("op1f_logs_lookup");
extern "C" void* op1f_logs_lookup(void* self, const char* name) {
    (void)self;
    u32 at = 0;
    if (name != 0)
        while (at < listed_count && !same(name, listed[at])) ++at;
    if (name == 0 || at == listed_count || given[at]) {
        if (++said.refused <= (u32)LINES_SAID) {
            char shown[SHOWN + 4];
            held(shown, name ? name : "");
            STOCK_LOG(2, LOG_FILE, 6, "logs: the computer brings a file, '%s', to logs/: refused, logs/ takes no new file", shown);
        }
        return 0;
    }
    char path[PATH];
    log_path(path, listed[at]);
    void* obj = STOCK_HEAP_ALLOC(FS_FILE_BYTES);
    if (!obj) return 0;
    STOCK_FS_FILE_CTOR(obj, path);
    give_delete(obj);
    given[at] = 1;
    return obj;
}

typedef void* (*BehaviourFn)();
static const BehaviourFn behaviour_vtable[6] = {
    (BehaviourFn)op1f_logs_noop,                                                    // the destructor
    (BehaviourFn)op1f_logs_noop,                                                    // the deleting destructor: nothing to free, the object is the base's
    (BehaviourFn)op1f_logs_noop,                                                    // +8
    (BehaviourFn)op1f_logs_zero,                                                    // +0xc
    (BehaviourFn)op1f_logs_zero,                                                    // +0x10: no folder inside
    (BehaviourFn)op1f_logs_lookup,                                                  // +0x14: the files
};

// ---------------------------------------------------------------- the disk mode

// The base's root child maker (module/build_r2_disk.cpp) asks this first, for every name it is handed: the folder's
// behaviour for `logs`, else 0. On the thread that builds the disk mode's tree.
extern "C" void* op1f_logs_child(const char* name) OP1F_NAMED("op1f_logs_child");
extern "C" void* op1f_logs_child(const char* name) {
    if (name == 0 || !same(name, FOLDER)) return 0;
    behaviour[0] = (const void*)behaviour_vtable;
    return (void*)behaviour;
}

// The base's population, before the packs' folders: logs/ listed, the folder made in the tree with its files.
extern "C" void op1f_logs_populate(void* root) OP1F_NAMED("op1f_logs_populate");
extern "C" void op1f_logs_populate(void* root) {
    u32 more = 0, other = 0;
    listed_count = 0;
    newest[0] = 0;
    for (u32 i = 0; i < (u32)MOST; ++i) given[i] = 0;
    void* dir = STOCK_DIR_OPEN(FOLDER);
    if (dir) {                                                                      // the names first, sorted, the folder closed, then the files
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(dir);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (!log_name(name, length_of(name, NAME_MOST))) {
                if (name[0] != '.') ++other;
                continue;
            }
            u32 at = listed_count;
            if (at == (u32)MOST) {                                                  // full: the newest are kept
                ++more;
                if (compare(name, listed[0]) <= 0) continue;
                for (u32 i = 0; i + 1 < at; ++i) copy(listed[i], listed[i + 1]);
                --at;
            } else {
                ++listed_count;
            }
            while (at > 0 && compare(name, listed[at - 1]) < 0) {
                copy(listed[at], listed[at - 1]);
                --at;
            }
            copy(listed[at], name);
        }
        STOCK_DIR_CLOSE(dir);
    }
    if (listed_count) log_path(newest, listed[listed_count - 1]);
    void* node = STOCK_MAKE_FOLDER(root, FOLDER);                                   // through the root child maker, and op1f_logs_child
    if (node == 0) {
        STOCK_LOG(1, LOG_FILE, 1, "logs: 'logs' is NOT in the disk mode's tree: the firmware's folder maker refused it");
        return;
    }
    if (dir == 0) {
        STOCK_LOG(1, LOG_FILE, 2, "logs: the disk mode shows 'logs' with no file: logs/ could NOT be listed");
        return;
    }
    u32 added = 0;
    for (u32 i = 0; i < listed_count; ++i)
        if (STOCK_ADD_FILE(node, listed[i])) ++added;                               // each through op1f_logs_lookup
    STOCK_LOG(added == listed_count ? 3 : 1, LOG_FILE, 3,
              "logs: the disk mode shows 'logs' with %u of the %u log file(s) of logs/ (%u older not shown, %u other name(s) not shown)",
              added, listed_count + more, more, other);
}

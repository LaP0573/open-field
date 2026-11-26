// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step M1 image M1: the loader's "packs" folder in the disk mode.
//
// The words. From step M1 on the loader is our piece inside the firmware file, which every unit before this one
// calls "the base"; the base is the shared pack, the file base.pack. This unit is J5c's, which images J5c, L1 and L5
// carry, with the loader's words: its three log lines begin "loader:"
// and its tag is op1f_loader, as the lines of the loader's other unit do (module/build_m4_loader.cpp). Nothing else of
// its code is changed. Its functions keep their names (op1f_base_...): the assembly beside it and the PC's harness
// call them by those.
//
// What follows is J5c's own description, in the new words. Until J5b
// the folder had two slots, 01.pack and 02.pack, which always existed, and it was the loader's only when no pack
// ran: a pack took the disk mode's two sites for its own folders and showed "packs" among them. Since J5c
//
//   * the folder is the loader's whatever runs (build_m4_loader.cpp writes the two changes of build_l1_base_table.s
//     before any pack's table; a pack of the new kind has no entry at the two sites). A pack that has folders of its
//     own offers two functions by name, op1f_mtp_root_child and op1f_mtp_populate_fx; the loader asks the first for
//     every root name that is not "packs" and calls the second after "packs" is made. A pack of the old kind still
//     takes the two sites itself, and this unit is then not reached;
//   * the folder shows the pack files that are on the disk: user/pk is listed when the disk mode builds its tree, and
//     every name that ends in ".pack" is a file of the folder (op1f_base_pack_name in build_m4_loader.cpp has the
//     rule, the same one the load point uses);
//   * a name the computer brings that the folder does not have is taken when it is a pack file's name: the disk
//     mode's open by name (0x8fcbb400) asks the folder's behaviour for a file object on a miss, as the tree builder
//     does for each file it adds. The object stands on a file that exists, as since J3: an empty placeholder is
//     written first, and the computer's bytes replace it through stock's file class (a temporary and a rename);
//   * a delete removes the file. K6b's delete emptied a slot, since a slot's file always existed; a folder of files
//     named freely has no slots, so the file object's delete slot calls stock's removal 0x8fcbefb4(path)
//     (patch_import.cpp: the path as the open takes it, relative to the root folder's descriptor; 0 when it is
//     gone) and the line says what the disk holds afterwards.
//
// A path is now user/pk/ and a name of up to thirty-nine characters, more than the fifteen a stock string keeps in
// its own buffer. Stock's file class copies the path into a string of its own, which it frees (0x8fcbabb8; its
// open, close, size and time read the text behind the pointer when the room word says sixteen or more: 0x8fcb8ed0,
// 0x8fcb8fa8). The one string this unit makes itself, for stock's json writer, is freed as the clipboard unit frees
// its own (build_k_clip.cpp, on the unit since K3).
//
// One log line a disk mode start, one for a file the computer brings and one for a delete, from the disk mode's
// thread, as stock's own folder maker logs there.
//
// Two generations, as build_m4_loader.cpp has them: with no define this
// is generation 1, N1's unit byte for byte; a wrapper unit compiles it as generation 2, N2's, whose
// folder shows 128 names (MAX_FILES), as many as N2 looks at. Nothing else differs.

#ifndef OP1F_LOADER_GENERATION
#define OP1F_LOADER_GENERATION 1                            // no define: N1's unit
#endif
#if OP1F_LOADER_GENERATION != 1 && OP1F_LOADER_GENERATION != 2
#error "OP1F_LOADER_GENERATION is 1 or 2"
#endif

typedef unsigned int u32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*MakeFolderFn)(void* parent, const char* name);              // 0x8fcaeb10
typedef void* (*AddFileFn)(void* folder, const char* name);                  // 0x8fcaed68
typedef void* (*HeapAllocFn)(u32 bytes);                                     // 0x8d4e63bc
typedef void (*FreeFn)(void* memory);                                        // 0x8d4e63cc
typedef void (*FsFileCtorFn)(void* obj, const char* path);                   // 0x8fcbabb8: the filesystem-backed file object (0x40 bytes)
typedef void* (*RootChildFn)(void* behaviour, const char* name);             // 0x8fcacea0: stock's root child maker
typedef void (*PopulateFn)(void* root);
typedef int (*FileSizeFn)(const char* path);
typedef int (*FileRemoveFn)(const char* path);                               // 0x8fcbefb4
typedef int (*MakeFoldersFn)(const char* path);
typedef void (*JsonCtorFn)(void* json, int kind);
typedef void (*JsonDtorFn)(void* json);
typedef int (*JsonWriteFileFn)(void* path_string, void* json);
typedef void (*StringCtorFn)(void* out, const char* text);
typedef void* (*DirOpenFn)(const char* path);
typedef void* (*DirNextFn)(void* handle);
typedef int (*DirCloseFn)(void* handle);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MAKE_FOLDER ((MakeFolderFn)0x8fcaeb10u)
#define STOCK_ADD_FILE ((AddFileFn)0x8fcaed68u)
#define STOCK_HEAP_ALLOC ((HeapAllocFn)0x8d4e63bcu)
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
#define STOCK_FS_FILE_CTOR ((FsFileCtorFn)0x8fcbabb8u)
#define STOCK_ROOT_CHILD ((RootChildFn)0x8fcacea0u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_REMOVE ((FileRemoveFn)0x8fcbefb4u)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define STOCK_FS_EMPTY_DELETE ((u32)0x8fcba894u)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)

enum { FS_FILE_BYTES = 0x40, JSON_OBJECT_KIND = 7 };
enum { FS_FILE_SLOTS = 12, FS_DELETE_SLOT = 7, FS_PATH_OWN = 8, FS_PATH_ROOM = 0x1c };
#if OP1F_LOADER_GENERATION == 2
enum { FILES_MOST = 128 };                                  // N2: as many names as the loader looks at files (its MAX_PACKS)
#else
enum { FILES_MOST = 32 };
#endif
enum { MAX_FILES = FILES_MOST, FILE_CHARS = 40, PATH_CHARS = 48, DIR_NAME_AT = 11 };   // as build_m4_loader.cpp has them

static const char LOG_FILE[] = "../src/main/op1f_loader.cpp";
static const char FOLDER_NAME[] = "packs";
static const char EMMC_FOLDER[] = "user/pk/";              // with the trailing slash: the folder maker makes what a slash ends
static const char PACK_DIR[] = "user/pk";                  // as stock opens a folder: without the slash
static const char OLD_FIRST[] = "01.pack";                 // the two names of J4 to J5b, shown when the folder cannot be listed
static const char OLD_SPARE[] = "02.pack";

// Is this a pack file's name? The load point's rule (build_m4_loader.cpp).
extern "C" int op1f_base_pack_name(const char* name) asm("op1f_base_pack_name");

// A pack's two functions for folders of its own, found by name at the load point; 0 without such a pack.
extern "C" u32 op1f_base_more_child asm("op1f_base_more_child");
extern "C" u32 op1f_base_more_populate asm("op1f_base_more_populate");
u32 op1f_base_more_child;
u32 op1f_base_more_populate;

#ifdef OP1F_HOST
// On the PC a pack's code cannot be called: the tests' harness stands in for it.
void* pack_root_child(u32 function, void* behaviour, const char* name);
void pack_populate(u32 function, void* root);
#else
static inline void* pack_root_child(u32 function, void* behaviour, const char* name) { return ((RootChildFn)function)(behaviour, name); }
static inline void pack_populate(u32 function, void* root) { ((PopulateFn)function)(root); }
#endif

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// user/pk/ and the name: at most forty-seven characters and the end.
static void path_of(char* path, const char* name) {
    u32 at = 0;
    for (const char* s = EMMC_FOLDER; *s; ++s) path[at++] = *s;
    for (const char* s = name; *s && at < PATH_CHARS - 1; ++s) path[at++] = *s;
    path[at] = 0;
}

// ---------------------------------------------------------------- a file's place, empty

// An empty json object from stock's writer at this path, as the slots' placeholders were made since J3. The stock
// string is the writer's argument: +4 its text or, for sixteen characters or more, the pointer to it; +0x18 its room.
static int write_placeholder(const char* path) {
    u32 json[4];
    for (int i = 0; i < 4; ++i) json[i] = 0;
    STOCK_JSON_CTOR(json, JSON_OBJECT_KIND);
    u32 text[12];
    for (int i = 0; i < 12; ++i) text[i] = 0;
    STOCK_STRING_CTOR(text, path);
    int written = STOCK_JSON_WRITE_FILE(text, json);
    if (text[6] >= 16 && text[1]) STOCK_FREE((void*)text[1]);              // the text is on the heap
    STOCK_JSON_DTOR(json);
    return written;
}

static u32 file_vtable[FS_FILE_SLOTS];                      // stock's twelve slots with the delete the loader's; made at the first file object

// The delete slot: (object). The object's path is the stock string at +4: its own buffer at +8 while the room word
// at +0x1c is under sixteen, else a pointer there (as stock's own open, close, size and time virtuals read it).
extern "C" void op1f_base_file_delete(void* obj) asm("op1f_base_file_delete");
extern "C" void op1f_base_file_delete(void* obj) {
    const char* own = (const char*)obj + FS_PATH_OWN;
    const char* path = *(const u32*)((const char*)obj + FS_PATH_ROOM) >= 16 ? *(const char* const*)own : own;
    int answer = STOCK_FILE_REMOVE(path);
    int left = STOCK_FILE_SIZE(path);                                       // what the disk holds now, whatever the removal answered
    STOCK_LOG(left < 0 ? 3 : 1, LOG_FILE, 21, "loader: %s deleted from the computer: %s (the removal answered %d, the size is %d)",
              path, left < 0 ? "the file is gone" : "the file is STILL there", answer, left);
}

// A file object fresh from stock's constructor gets the delete above.
static void give_delete(void* obj) {
    u32* stock = *(u32**)obj;
    if (!file_vtable[0]) {
        if (stock[FS_DELETE_SLOT] != STOCK_FS_EMPTY_DELETE) return;         // not the class as it was read: left as stock made it
        for (int i = 0; i < FS_FILE_SLOTS; ++i) file_vtable[i] = stock[i];
        file_vtable[FS_DELETE_SLOT] = (u32)&op1f_base_file_delete;
    }
    *(u32**)obj = file_vtable;
}

// ---------------------------------------------------------------- the folder's behaviour

extern "C" void op1f_base_disk_noop(void* self) asm("op1f_base_disk_noop");
extern "C" void op1f_base_disk_noop(void* self) { (void)self; }             // the destructors (the object is static) and the +8 slot

extern "C" void* op1f_base_disk_zero(void* self, const char* name) asm("op1f_base_disk_zero");
extern "C" void* op1f_base_disk_zero(void* self, const char* name) { (void)self; (void)name; return 0; }   // +0xc, and +0x10: no child folders

extern "C" void* op1f_base_pack_lookup(void* self, const char* name) asm("op1f_base_pack_lookup");
extern "C" void* op1f_base_pack_lookup(void* self, const char* name) {     // +0x14: the pack file as stock's filesystem-backed file object
    (void)self;
    if (!name || !op1f_base_pack_name(name)) return 0;                      // not a pack file's name: refused, and stock says so
    char path[PATH_CHARS];
    path_of(path, name);
    if (STOCK_FILE_SIZE(path) < 0) {                                        // a name the computer brings: its file is made first
        int written = write_placeholder(path);
        STOCK_LOG(written ? 3 : 1, LOG_FILE, 29, "loader: the computer brings a new pack file, %s: %s", path,
                  written ? "a place is made for it" : "its place could NOT be made");
    }
    void* obj = STOCK_HEAP_ALLOC(FS_FILE_BYTES);
    if (!obj) return 0;
    STOCK_FS_FILE_CTOR(obj, path);
    give_delete(obj);
    return obj;
}

typedef void* (*BehaviourFn)();
static const BehaviourFn pack_behaviour_vtable[6] = {
    (BehaviourFn)op1f_base_disk_noop,                                       // the destructor
    (BehaviourFn)op1f_base_disk_noop,                                       // the deleting destructor: nothing to free, the object is the loader's
    (BehaviourFn)op1f_base_disk_noop,                                       // +8
    (BehaviourFn)op1f_base_disk_zero,                                       // +0xc
    (BehaviourFn)op1f_base_disk_zero,                                       // +0x10: no child folders
    (BehaviourFn)op1f_base_pack_lookup,                                     // +0x14: the files
};
static const void* pack_behaviour[2] = { pack_behaviour_vtable, 0 };

// ---------------------------------------------------------------- the two hooks

// The root behaviour's +0x10 (the word at 0x8490fc4c): "packs" is the loader's folder; every other name goes to a
// pack that has folders of its own, which answers its own and hands the rest to stock, or to stock.
extern "C" void* op1f_base_root_child(void* behaviour, const char* name) asm("op1f_base_root_child");
extern "C" void* op1f_base_root_child(void* behaviour, const char* name) {
    if (name && same(name, FOLDER_NAME)) return (void*)pack_behaviour;
    if (op1f_base_more_child) return pack_root_child(op1f_base_more_child, behaviour, name);
    return STOCK_ROOT_CHILD(behaviour, name);
}

static char listed[MAX_FILES][FILE_CHARS];                  // the folder's pack files as last listed; a node copies its name

// Called by op1f_base_populate_stub (build_k6_base_disk.s) with the root folder after stock made the mixdown folder.
extern "C" void op1f_base_populate(void* root) asm("op1f_base_populate");
extern "C" void op1f_base_populate(void* root) {
    STOCK_MAKE_FOLDERS(EMMC_FOLDER);                                       // user/pk/ on the eMMC
    u32 count = 0, more = 0, other = 0;
    void* folder = STOCK_DIR_OPEN(PACK_DIR);
    if (folder) {                                                          // the names first, the folder closed, then the files
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(folder);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (!op1f_base_pack_name(name)) {
                if (name[0] != '.') ++other;
            } else if (count == MAX_FILES) {
                ++more;
            } else {
                u32 at = 0;
                for (; name[at] && at < FILE_CHARS - 1; ++at) listed[count][at] = name[at];
                listed[count++][at] = 0;
            }
        }
        STOCK_DIR_CLOSE(folder);
    } else {                                                               // not listed: the two names of before, where they are files
        for (int i = 0; i < 2; ++i) {
            const char* name = i ? OLD_SPARE : OLD_FIRST;
            char path[PATH_CHARS];
            path_of(path, name);
            if (STOCK_FILE_SIZE(path) < 0) continue;
            u32 at = 0;
            for (; name[at]; ++at) listed[count][at] = name[at];
            listed[count++][at] = 0;
        }
    }
    u32 files = 0;
    void* packs = STOCK_MAKE_FOLDER(root, FOLDER_NAME);                    // through op1f_base_root_child
    if (packs) {
        for (u32 i = 0; i < count; ++i) {
            if (STOCK_ADD_FILE(packs, listed[i])) ++files;                 // each through op1f_base_pack_lookup
        }
    }
    STOCK_LOG(packs && files == count ? 3 : 1, LOG_FILE, 20,
              "loader: the disk mode's tree: the folder 'packs' %s with %u of the %u pack file(s) of user/pk (%s), %u more not shown, %u other name(s); a pack's folders after it: %s",
              packs ? "made" : "NOT made", files, count, folder ? "listed" : "NOT listed", more, other, op1f_base_more_populate ? "yes" : "no");
    if (op1f_base_more_populate) pack_populate(op1f_base_more_populate, root);
}

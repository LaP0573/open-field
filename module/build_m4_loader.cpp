// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// One source, two generations. The build define
// OP1F_LOADER_GENERATION is 1 or 2; where it is not defined the unit is generation 1, which compiles to N1's loader
// byte for byte (and to M4's for 1.7.0), so every build of this file that names it alone still makes N1. Generation 2
// is the loader N2, compiled from two small wrapper units this tree does not hold yet (the define as 2 and an
// include). Against generation 1 it
//
//   n1. loads packs into a room of its own, 16 MiB from 0x85000000 (PACK_BASE, PACK_ROOM), and its own block is the
//       top 128 KiB of our reservation (BASE_START, LOADER_BYTES), its end BASE_START + LOADER_BYTES
//       where generation 1 took PACK_BASE for it: the room no longer lies above the loader;
//   n2. looks at 128 pack files (MAX_PACKS; the disk mode's folder shows as many, MAX_FILES);
//   n3. answers a second mark, op1f_loader_2, with the address of a record of six words in its constant data
//       (op1f_loader_record: its bytes, the generation, the room's first address and bytes, the most files, the most
//       cards); a pack that needs it runs only under N2, and under N1 the name gives nothing;
//   n4. hands the cards over into a second list too, op1f_cards_2, of 128 slots, beside op1f_cards with its 32;
//   n5. no longer runs a pack of the old kind (format 2): it is left out with the line for a format the loader does
//       not know, and the old kind's run is not compiled;
//   n6. names itself N2 in its first line.
//
// Every other line, check and rule is generation 1's, and a pack file runs under N2 exactly as under N1: packs are
// still linked for N1's room and moved by the distance to their place.
//
// Step M4 image M4: the loader of step M1 with the start screen's new words: the main version, a dash, open-field,
// and the release of the base, "1.7.0 - open-field 0.3".
//
// It is M1's loader, with these changes and no other:
//
//   i. where pack files run, the start screen says the words " - open-field" and the release behind them, and no
//      name of a pack: "1.7.0 - open-field 0.3". Behind it, in brackets, the pack files left out, by M1's rule (h
//      below): "1.7.0 - open-field 0.3 (1 left out)", or the pack they want where no file of the folder holds it,
//      "(1 needs foo)". Where that does not fit its share of the screen, one count of the files left out; where even
//      that does not, the release is left out (words_for_the_set). The words of a start as stock firmware are M1's,
//      and so are those of a pack of the old kind;
//   j. its first line names it M4, so that a log says which firmware file ran.
//
// Every other line, every write and every rule is M1's, and a pack runs under this loader exactly as under M1's: so
// the loader answers for the same mark, op1f_loader_1, and for no other. M1's description follows as it was written;
// where it speaks of the word custom for a set of the new kind, i stands.
//
// Step M1 image M1: the loader, our piece inside the firmware file, alone in it (docs/packs.md says what it does).
//
// The words. From this step on the loader is this unit, which every unit and note before calls "the base" (with
// its lines "base: ..."), and the base is the shared pack every other pack relies on, the file base.pack, which was
// the pack `kit`. The functions here keep their names (op1f_base_..., op1f_boot_...): the assembly beside this unit
// and the PC's harness call them by those.
//
// It is made from the loader of image L5, with the code for a pack inside the image taken out, which leaves the
// loader of image L1. The shared pack is a file only, never inside the firmware file, so the image holds this
// loader and nothing else of ours, as image L1 did, and the loader is linked by linker/op1f-1.7.0-base.ld. Against
// L1's unit this one
//
//   a. says "loader:" where its elders say "base:", under the tag op1f_loader, and names itself M1 in its first
//      line. Every line of before keeps its text otherwise, and its number;
//   b. takes a pack's card. The last word of a pack's header, zero in every pack before, is the place of the pack's
//      card in its body, 0 for none. The place is held with the header (on a word, with eight bytes of the body
//      from it on), and the card's first two words after the checksum: its mark, and a size of whole words, eight
//      bytes at least, that ends inside the body. The loader reads nothing else of a card. A pack whose card is not
//      in order is left out, with three lines;
//   c. hands the cards over. A pack may offer a list under the name op1f_cards: two words and thirty-two slots of
//      three, in its zeroed data. When the packs are joined and their zeroed data is cleared, and before any table
//      is written, the loader writes into it, in the order of the files' names, for each pack that runs and has a
//      card: where the card stands, the pack's first address, and the address behind its last. The list is held to
//      lie whole in the pack that offers it, on a word. What a card says is that pack's to read;
//   d. allows a pack of the new kind whose table has no entry: a pack that changes no place of the firmware;
//   e. answers for a name of its own when the packs are joined, op1f_loader_1, with the address of its own first
//      byte. A pack that needs this loader or a later one names it as a name it needs, and an older loader leaves
//      such a pack out with its usual line. The loader keeps every name that begins op1f_loader_ to itself: a pack
//      that offers one is left out, so a pack that needs a later loader's mark finds nobody under this one;
//   f. writes a pack's table after the tables of the packs that lean on it: those that name it as a pack they need,
//      as before, and now also those that need a name it offers, each counted once;
//   g. says a line for a file that stands by because another file holds the same pack, and prints the place two
//      packs meet on as an address;
//   h. shows a release on the start screen. A pack may offer a short text under the name op1f_release, and it
//      stands behind the word custom ("1.7.0 custom 0.1 (8 tracks, fx train)"), or behind a plus sign where the
//      names would not fit otherwise. And where every file left out for want of a pack wants the same pack, and no
//      file of the folder holds that pack, the screen names it ("1.7.0 stock (4 need base)"): copying that file is
//      then the step to take. A pack that lies in the folder and cannot run is never named so.
//
// Where no pack has a card, a list or a release, no file stands by or is in want of a pack and no two packs meet,
// every line and every write is L1's but for the words and the first line's name.
//
// What the loader does, as J5c's and L1's descriptions have it, which still hold. It
//
//   1. lists the folder user/pk and takes every file whose name ends in ".pack", whatever it is called; what a pack
//      is stands inside the file (its name, a version, an interface number);
//   2. places the packs of the new kind (format 3) one behind the other in the room, in the order of their files'
//      names, and moves each to its place: the file lists the places that hold one of the pack's own addresses, and
//      the distance between where it was linked for and where it stands is added at each;
//   3. joins the packs by name: a pack lists the names it offers and the names it wants, needed or optional, and the
//      loader fills each wanted name's places with the address of the pack that offers it, or with zero for an
//      optional name nobody offers, which the pack's own code tests before it uses the name;
//   4. settles which packs run before anything of stock is written: a pack is left out when a pack it needs is not
//      there, when a pack it works with was made for another interface, when a needed name is offered by nobody,
//      when it offers a name an earlier pack offers, when its table does not pass, or when one of its changes of
//      stock meets a change of the loader's or of an earlier pack's. The others run;
//   5. writes each pack's table but for the entries marked "only when pack X is away" while X runs;
//   6. owns the disk mode's two sites and the folder "packs" whatever runs (build_m1_loader_disk.cpp), and hands the
//      root folder's other names to a pack that offers the two functions for it;
//   7. names what runs on the start screen.
//
// An entry of a table of stock changes is of one of two kinds, and a table may hold both. The old kind names the
// bytes its site holds in the stock firmware. The new kind (L1's) carries in their place the CRC-32 of the bytes the
// change replaces (four for a data word, else eight, or as many as its trampoline copies when that is more) and the
// address of the pack's room for a trampoline, or 0; its kind's low half is KIND_SUM and the kind of before, its
// high half the bytes the trampoline copies: 0, 8, 10, 12 or 14. Such an entry passes when the checksum of what
// stands at its site is the entry's, or, without a trampoline, when the site already holds what the entry writes.
// The loader makes the trampoline itself before it writes the hook: the displaced bytes copied from the running
// firmware into the room, a JUMP.a to the first instruction after them behind, the room written out of the data
// cache and dropped from the instruction cache as loaded code is. The room lies in the pack's zeroed data, which the
// loader has cleared before any table is written (run_new, op1f_base_load). The loader's own three changes of stock
// are entries of the new kind (build_l1_base_table.s, build_l1_base_text_table.s): it holds no stock bytes in a table.
//
// A pack of the old kind (format 2: K6b's, K5's) is linked for the room's first byte and holds everything. One runs
// only when no pack of the new kind does: the first that passes, in the order of the names, exactly as J5b ran it
// (the same functions, the same lines but for the words), and it keeps the disk mode's two sites it takes itself.
//
// The lists of a pack (tools/pack_build.py has the format) are read into the room right behind its body, where its
// zeroed data will be, used, and gone when that data is cleared: nothing of them stays in memory. So everything the
// loader needs afterwards (which entries of a table are left to another pack, where the list of cards stands, the
// release) is kept in its own records before.
//
// What stops the start-up is what stopped it before: a write of a table that does not take, or a site that is no
// longer what the check saw. Everything else leaves a pack out and goes on; with no pack left the unit starts as
// stock firmware.
//
// Stock functions used: L1's, none more. Those of J5b, all proven at the load point: the logger 0x8c053a20, the file
// size 0x8c051f9c, the open 0x8fc8a9d8 with stock's mode text "rb", the read 0x8fc8a9a8, the close 0x8fc8a994, the
// assert 0x8c053a50, the keyboard controller's request 0x8c0567f0. Since J5c, read at their callers (docs/methods.md
// says how): the folder's open 0x8c051e28(path) (a handle or 0; the path without the slash), its next entry
// 0x8c0a4bd0(handle) (0 at the end; the name is the text at the entry's +0xb) and its close 0x8c0a57d4(handle). The
// start screen's text is measured with the bitmap font's table of widths where stock keeps it (0x84019e64: a byte a
// character from 0x20, one column between two characters).
//
// The unit runs on the PC whole, in a harness of tests this tree does not hold: the stock memory behind the sites,
// the room, the loader's own tables and the font's widths are the harness's there, behind the names marked
// OP1F_HOST below.

#ifndef OP1F_LOADER_GENERATION
#define OP1F_LOADER_GENERATION 1                            // no define: N1's loader
#endif
#if OP1F_LOADER_GENERATION != 1 && OP1F_LOADER_GENERATION != 2
#error "OP1F_LOADER_GENERATION is 1 or 2"
#endif

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*FileSizeFn)(const char* path);
typedef int (*FileOpenFn)(const char* path, const char* mode);
typedef int (*FileReadFn)(void* buffer, int size, int count, int descriptor);
typedef int (*FileCloseFn)(int descriptor);
typedef void (*AssertFn)(const char* message);
typedef u32 (*KbRequestFn)(void* link, u32 type, const void* payload, u32 bytes, void* answer, u32 timeout_ms);
typedef void* (*DirOpenFn)(const char* path);
typedef void* (*DirNextFn)(void* handle);
typedef int (*DirCloseFn)(void* handle);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_OPEN ((FileOpenFn)0x8fc8a9d8u)
#define STOCK_FILE_READ ((FileReadFn)0x8fc8a9a8u)
#define STOCK_FILE_CLOSE ((FileCloseFn)0x8fc8a994u)
#define STOCK_READ_MODE ((const char*)0x848daec0u)
#define STOCK_ASSERT ((AssertFn)0x8c053a50u)
#define STOCK_KB_REQUEST ((KbRequestFn)0x8c0567f0u)
#define STOCK_KB_LINK ((void*)0x8497d844u)
#define STOCK_KB_LISTENER ((volatile u32*)0x849857bcu)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)

enum { HEADER_BYTES = 64, PACK_FORMAT = 2, PIECE_BYTES = 16384, ENTRY_BYTES = 20, MAX_ENTRIES = 4096 };
#if OP1F_LOADER_GENERATION == 2
enum { PACKS_MOST = 128 };                                  // N2: the pack files looked at
#else
enum { PACKS_MOST = 32 };
#endif
enum { HEADER3_BYTES = 96, PACK_FORMAT_LINKED = 3, MAX_PACKS = PACKS_MOST, FILE_CHARS = 40, PATH_CHARS = 48, MAX_SKIPS = 64, LIST_COUNTS = 6, PACK_ALIGN = 32 };
enum { DIR_NAME_AT = 11 };                                  // a folder entry's name: the text at its +0xb

// A pack's card and what goes with it (module/build_m1_card.h has them for the packs; a test holds the two files to
// each other). A card's first word is its mark and its second its size, which holds those two words at least. The
// list a pack offers for the cards is two words, the count of the slots filled and of the cards that found none, and
// CARDS_MOST slots of three words: where a card stands, its pack's first address, the address behind its pack's
// last. A release is a text in RELEASE_BYTES bytes, its end among them; so is the name of a pack a file is in want of.
enum { CARD_MARK = 0x44524143, CARD_LEAST = 8, CARDS_MOST = 32, SLOT_WORDS = 3, CARDS_WORDS = 2 + SLOT_WORDS * CARDS_MOST, RELEASE_BYTES = 16, MISSING_BYTES = 16 };
#if OP1F_LOADER_GENERATION == 2
// N2's second list, op1f_cards_2: the same two words and CARDS_MOST_2 slots of three, one for every pack file looked at.
enum { CARDS_MOST_2 = 128, CARDS_WORDS_2 = 2 + SLOT_WORDS * CARDS_MOST_2 };
#endif

// The first four bytes of the stock OS image's SHA-256 (f9 77 75 d6 ...), little-endian: the OS a pack is for.
static const u32 OS_ID = 0x1aeb34d1u;

#if OP1F_LOADER_GENERATION == 2
// N2: packs are loaded into the room our loader N2 takes for them, 16 MiB from 0x85000000; the loader's own block is
// the top LOADER_BYTES of our reservation, and it ends there. Packs are still linked for N1's room (linker/op1f-1.7.0-pack.ld)
// and moved by the distance to their place, which is now below where they were linked.
static const u32 PACK_BASE = 0x85000000u;
static const u32 PACK_ROOM = 0x01000000u;
static const u32 BASE_START = 0x8d4c4234u;
static const u32 LOADER_BYTES = 0x00020000u;
static const u32 LOADER_END = BASE_START + LOADER_BYTES;    // the end of the loader's own code and data
#else
// Where packs are loaded: the reservation's first byte plus the loader's 64 KiB, to the reservation's end
// (linker/op1f-1.7.0-pack.ld). The loader itself lies in the 64 KiB before it (linker/op1f-1.7.0-base.ld).
static const u32 PACK_BASE = 0x8d2f4234u;
static const u32 PACK_ROOM = 0x001f0000u;
static const u32 BASE_START = 0x8d2e4234u;
static const u32 LOADER_END = PACK_BASE;                    // the room's first byte is the end of the loader's own code
#endif

// Where a site may lie (tools/boot_table.py: L1_CODE and DDR).
static const u32 L1_CODE_START = 0x11a00000u;
static const u32 L1_CODE_END = 0x11a10000u;
static const u32 DDR_START = 0x80000000u;
static const u32 DDR_END = 0x90000000u;

static const char PACK_DIR[] = "user/pk";                   // as stock opens a folder: without the slash
static const char PACK_PREFIX[] = "user/pk/";
static const char OLD_FIRST[] = "01.pack";                  // the two names of J4 to J5b, tried when the folder cannot be listed
static const char OLD_SPARE[] = "02.pack";
static const char LOG_FILE[] = "../src/main/op1f_loader.cpp";

// Three names. The loader answers for the first itself, with BASE_START, its own first byte: its mark, which a pack
// names as a name it needs when it needs this loader or a later one. The two others are names a pack may offer and
// the loader looks for: the list the cards are handed over in, and the release text the start screen shows.
static const char NAME_LOADER[] = "op1f_loader_1";
static const char NAME_CARDS[] = "op1f_cards";
static const char NAME_RELEASE[] = "op1f_release";
#if OP1F_LOADER_GENERATION == 2
// N2's two names more: its second mark, which it answers with the address of its record, and the second list for
// the cards, which a pack may offer beside the first.
static const char NAME_LOADER_2[] = "op1f_loader_2";
static const char NAME_CARDS_2[] = "op1f_cards_2";
// The record behind op1f_loader_2, six words in the loader's constant data, never written: its own size in bytes, the
// loader's generation, the room's first address and its bytes, the most pack files looked at, the most cards handed
// over (op1f_cards_2's slots). A reader takes `bytes` first and only the fields it covers; a later loader adds words
// behind and never changes the meaning of one.
enum { RECORD_WORDS = 6 };
extern "C" const u32 op1f_loader_record[RECORD_WORDS] asm("op1f_loader_record");
const u32 op1f_loader_record[RECORD_WORDS] = { 4 * RECORD_WORDS, 2, PACK_BASE, PACK_ROOM, MAX_PACKS, CARDS_MOST_2 };
#endif
// And the family of the first: every name that begins so is a loader's mark, this one's or a later one's, and the
// loader keeps them all to itself. It answers for its own alone; a pack that offers any of them is left out, so that
// a pack that needs a later loader finds nobody under this one, whatever files lie in the folder.
static const char NAME_LOADERS[] = "op1f_loader_";

// The header tools/pack_build.py writes for a pack of the old kind (format 2): sixty-four bytes, little-endian.
struct PackHeader {
    char magic[8];          // "OP1FPACK"
    u32 format;             // PACK_FORMAT
    u32 header_bytes;       // HEADER_BYTES
    u32 os_id;              // OS_ID
    u32 load_address;       // where the body was linked for: PACK_BASE
    u32 body_bytes;         // code, constant data, initialised data
    u32 compare_bytes;      // of those, the code and constant data
    u32 zeroed_bytes;       // the memory after the body the pack expects cleared, to the end of its zeroed data
    u32 body_crc;           // CRC-32 of the body (the zlib one)
    char name[16];          // zero padded
    u32 table;              // the address of the pack's table of stock changes, in its constant data
    u32 table_end;          // and of its end
};

// And for a pack of the new kind (format 3): the same sixty-four bytes and eight words more.
struct Header3 {
    char magic[8];
    u32 format;             // PACK_FORMAT_LINKED
    u32 header_bytes;       // HEADER3_BYTES
    u32 os_id;
    u32 link_address;       // where the body was linked for; it stands where the loader places it
    u32 body_bytes;         // whole words
    u32 compare_bytes;
    u32 zeroed_bytes;
    u32 crc;                // CRC-32 of the body and the lists
    char name[16];          // the pack's own name: what it is, whatever its file is called
    u32 table;              // as linked
    u32 table_end;
    u32 lists_bytes;        // the lists behind the body
    u32 extra_bytes;        // what follows the lists in the file and is not loaded
    u32 version;            // of two files with the same pack the higher runs
    u32 interface_;         // two packs that work together agree on each other's
    u32 align;              // the largest alignment inside, a power of two up to PACK_ALIGN
    u32 flags;              // FLAG_*
    u32 path_place;         // the offset in the body of PATH_CHARS bytes for the file's path, 0 for none
    u32 card_place;         // the offset in the body of the pack's card, 0 for none (zero in every pack before M1)
};

enum { FLAG_UNNAMED = 1, FLAG_SUM = 2, NEEDED = 1 };
enum { PLACE_HIGH = 0, PLACE_LOW = 1, PLACE_WORD = 2, PLACE_CALL = 3 };

enum {
    LOAD_OK = 0,            // in place, the checksum right
    LOAD_NO_FILE = 1,       // nothing at the path
    LOAD_NOT_A_PACK = 2,    // a file without the magic: a slot's placeholder
    LOAD_BAD_HEADER = 3,    // the magic, but a field does not fit this loader
    LOAD_NOT_READ = 4,      // the open or a read failed
    LOAD_BAD_CRC = 5,       // read whole, the checksum differs
};

struct PackLoad {
    int code;               // LOAD_*
    int file_bytes;         // the file's size, -1 without a file
    int descriptor;         // what the open answered
    u32 read_bytes;         // body bytes read
    u32 crc;                // the checksum computed
    u32 read_cycles;        // the reads
    u32 check_cycles;       // the clearing and the checksum
    PackHeader header;      // as read (zeroed without one)
    char name[17];          // the header's name, ended
};

// One entry of a table of stock changes (tools/boot_table.py): five words. An entry of the old kind names the stock
// bytes of its site; one of the new kind (KIND_SUM) has their checksum and the room of its trampoline there.
struct BootPatch {
    u32 kind;               // KIND_*; the new kind: KIND_SUM and one of them, the trampoline's bytes in the high half
    u32 site;               // the stock address
    u32 target;             // the function of ours the word, the call, the jump or the load names
    u32 before_lo;          // the site's stock bytes: the first four, little-endian (a data word: the word)
                            // the new kind: the CRC-32 of the bytes the change replaces or its trampoline copies
    u32 before_hi;          // the next four (a data word: 0)
                            // the new kind: the address of the pack's room for the trampoline, 0 without one
};

enum { KIND_WORD = 0, KIND_CALL_L1 = 1, KIND_CALL_DDR = 2, KIND_JUMP_L1 = 3, KIND_JUMP_DDR = 4, KIND_LOAD_DDR = 5 };

// The new kind. A hook is eight bytes; the instructions under it end at its eighth byte or two, four or six bytes
// on (a 64-bit instruction may start at the hook's seventh byte), and a trampoline copies up to there. Behind the
// copy stands the jump back, eight bytes.
enum { KIND_SUM = 0x10, TRAMPOLINE_LEAST = 8, TRAMPOLINE_MOST = 14, JUMP_BYTES = 8 };

enum {
    TABLE_OK = 0,           // the check: every entry can be applied; the write: every entry is in
    TABLE_KIND = 1,         // a kind this loader does not know
    TABLE_SITE = 2,         // a site outside stock's memory, or off its alignment
    TABLE_TARGET = 3,       // a target outside the code the table came with
    TABLE_BYTES = 4,        // the site holds neither the stock bytes the entry names nor what the entry writes;
                            // the new kind: neither the bytes the entry's checksum was made from nor what it writes
    TABLE_SHARED = 5,       // two entries on the same bytes
    TABLE_CHANGED = 6,      // the write: the site is no longer what the check saw
    TABLE_NOT_TAKEN = 7,    // the write: read back, the site does not hold what was written
    TABLE_ROOM = 8,         // the new kind: a trampoline's room outside the zeroed data the table came with, or shared
    TABLE_FORM = 9,         // the new kind: a load entry on a site that holds no load of a register
};

// What a check or a write of a table found. The counts are the write's, but for `sums`, which both keep.
struct TableResult {
    int code;               // TABLE_*
    int index;              // the entry it is about, -1 without one
    u32 site;               // that entry's
    u32 kind;
    u32 found[2];           // what stood at the site
    u32 wanted[2];          // what the entry expected there; the new kind: the entry's checksum, then the site's
    u32 entries;            // entries gone through
    u32 words;              // data words written and read back
    u32 in_ddr;             // code sites in DDR
    u32 in_l1;              // code sites in L1
    u32 already;            // sites that already held what the entry writes
    u32 left;               // entries left to another pack: marked "only when it is away", and it runs
    u32 sums;               // entries of the new kind passed (the check) or made (the write)
    u32 trampolines;        // trampolines made
};

// A pack file of the folder, and what became of it.
enum {
    P_NONE = 0,             // no pack in the file
    P_OLD = 1,              // a pack of the old kind: runs only when no new one does
    P_NEW = 2,              // a pack of the new kind, not read yet
    P_IN = 3,               // in the room, moved to its place: runs unless the set says otherwise
    P_RUNS = 4,
    P_OUT = 5,              // left out: `why`
    P_SPARE = 6,            // another file holds the same pack and runs
};

enum {
    WHY_HEADER = 1,         // a field of its header does not fit this loader
    WHY_READ = 2,           // the open or a read failed
    WHY_SUM = 3,            // read whole, the checksum differs
    WHY_LISTS = 4,          // its lists are not in order
    WHY_ROOM = 5,           // no room is left for it
    WHY_NEEDS = 6,          // a pack it needs does not run
    WHY_INTERFACE = 7,      // a pack it works with has another interface number
    WHY_NAME = 8,           // it offers a name an earlier pack offers
    WHY_IMPORT = 9,         // a name it needs is offered by nobody
    WHY_REACH = 10,         // a direct call of it would not reach
    WHY_TABLE = 11,         // its table does not pass
    WHY_CLAIM = 12,         // one of its changes of stock meets the loader's or an earlier pack's
    WHY_CARD = 13,          // its card is not in order: where it stands, its mark or its size
};

struct Pack {
    char file[FILE_CHARS];  // the file's name in the folder
    int size;               // its size, -1 without a file
    int state;              // P_*
    int why;                // WHY_*
    Header3 h;              // its header as read (a pack of the old kind: the first sixty-four bytes)
    char name[17];          // the header's name, ended
    u32 place;              // where its body stands
    u32 foot;               // what it takes of the room: the body and the larger of the lists and the zeroed data
    u32 crc;                // the checksum computed
    u32 read_cycles;
    u32 check_cycles;
    u32 moved;              // places of its own changed
    u32 filled;             // places filled with another pack's address
    u32 names;              // names of others found
    u32 absent;             // optional names nobody offers
    u32 sum;                // FLAG_SUM: the CRC-32 of its body as linked
    u32 rank;               // how many running packs lean on it: the higher, the later its table is written
    u32 skip_count;         // entries of its table left to another pack
    u16 skips[MAX_SKIPS];
    // the lists, while they stand behind the body (until the zeroed data is cleared)
    const u32* works;       // name, interface, flags
    const u32* exports;     // name, offset
    const u32* imports;     // name, flags
    const u32* places;      // offset, kind and import, value
    const u32* marks;       // table entry, work
    const char* strings;
    u32 works_n, exports_n, imports_n, places_n, marks_n, strings_n;
};

// The safe start. The keyboard controller's request for its key map; stock's own timeout for a request to it; a
// message's size, which is what stock copies into the answer's buffer; a map's size; the help key's number.
enum { KEYS_REQUEST = 0x41, KEYS_TIMEOUT_MS = 100, MESSAGE_BYTES = 253, MAP_BYTES = 8, SAFE_KEY = 0 };

// What the keyboard handler's init leaves in the link's listener word: its key map handler, the object's +0x81f4.
static const u32 KB_KEY_HANDLER = 0x84985a34u;

enum {
    KEYS_OK = 0,            // the map is in low and high
    KEYS_NO_ANSWER = 1,     // the request came back without one
    KEYS_SHORT_ANSWER = 2,  // an answer with fewer bytes than a map has
};

struct KeysRead {
    int code;               // KEYS_*
    u32 type;               // the answer's type and its payload's length, 0 without one
    u32 bytes;
    u32 low;                // keys 0 to 31, bit n for key n
    u32 high;               // keys 32 to 63
    u32 cycles;             // the request's time
    u32 listener;           // what the link's listener word held
    u32 aside;              // 1 when it was the key map handler and was cleared for the request
};

extern "C" u32 op1f_boot_control() asm("op1f_boot_control");
extern "C" void op1f_boot_write_ddr(u32 site, u32 lo, u32 hi) asm("op1f_boot_write_ddr");
extern "C" u32 op1f_boot_write_l1(u32 site, u32 lo, u32 hi) asm("op1f_boot_write_l1");
extern "C" void op1f_boot_read_l1(u32 site, u32* out) asm("op1f_boot_read_l1");
extern "C" u32 op1f_boot_probe() asm("op1f_boot_probe");
extern "C" const u32 op1f_boot_probe_alt[2] asm("op1f_boot_probe_alt");   // the probe's other eight bytes, read as data
extern "C" void op1f_base_flush(u32 start, u32 end) asm("op1f_base_flush");

extern "C" int op1f_base_load(const char* path, u8* room, u32 room_bytes, u32 load_address, PackLoad* load) asm("op1f_base_load");
extern "C" void op1f_base_say(const char* path, const PackLoad* load) asm("op1f_base_say");
extern "C" int op1f_base_check(const BootPatch* table, const BootPatch* table_end, u32 code_start, u32 code_end, TableResult* result) asm("op1f_base_check");
extern "C" int op1f_base_write(const BootPatch* table, const BootPatch* table_end, TableResult* result) asm("op1f_base_write");
extern "C" int op1f_base_keys(KeysRead* keys) asm("op1f_base_keys");
extern "C" const char* op1f_base_screen_text(const char* stock_text) asm("op1f_base_screen_text");
extern "C" int op1f_base_pack_name(const char* name) asm("op1f_base_pack_name");
extern "C" void op1f_boot_apply() asm("op1f_boot_apply");

// The disk mode's root folder asks these for a name that is not "packs" and for folders after "packs"
// (build_m1_loader_disk.cpp): a pack's two functions, found by name; 0 without such a pack.
extern "C" u32 op1f_base_more_child asm("op1f_base_more_child");
extern "C" u32 op1f_base_more_populate asm("op1f_base_more_populate");
static const char NAME_MORE_CHILD[] = "op1f_mtp_root_child";
static const char NAME_MORE_POPULATE[] = "op1f_mtp_populate_fx";

// A function that stands by itself in the listing, where the compiler would fold it into its one caller.
#define OWN __attribute__((noinline))

static u32 header_words[HEADER3_BYTES / 4];                 // a header as read, word aligned
static u32 crc_table[256];
static u32 crc_table_made;
static u32 l1_announced;                                    // the line before the first access to L1 code, once a start
static u32 keys_message[64];                                // the keyboard controller's answer, whole: stock copies MESSAGE_BYTES
static Pack packs[MAX_PACKS];                               // the folder's pack files, in the order of their names
static u32 pack_count;
static u32 folder_more;                                     // pack files beyond MAX_PACKS, not looked at
static u32 folder_other;                                    // names of the folder that are no pack file's

// The start screen's text. Stock's own buffer for a version text holds SCREEN_STOCK_CHARS and an end. The words
// that follow it are chosen at the load point to fit WORDS_COLUMNS of the screen's 480 columns, which leaves room
// for a version text of ten characters; the whole is measured again when stock's text is known.
enum { SCREEN_COLUMNS = 440, WORDS_COLUMNS = 380, SCREEN_STOCK_CHARS = 23, SCREEN_WORDS_BYTES = 88, FONT_FIRST = 0x20, FONT_LAST = 0x7e };
static const u32 FONT_WIDTHS = 0x84019e64u;                 // bitmapfont.cpp: a byte a character from FONT_FIRST
static char screen_words[SCREEN_WORDS_BYTES];               // what follows stock's version text; empty: stock's text alone
static char screen_text[SCREEN_STOCK_CHARS + SCREEN_WORDS_BYTES + 1];

// What M1 keeps beside the list of pack files, whose entries are as they were (392 bytes each).
static char missing[MAX_PACKS][MISSING_BYTES];              // for a file left out for want of a pack: that pack's name, as the start screen says it; empty where the pack is in the folder
static u32 card_mark, card_bytes;                           // the first two words of the card load3 looked at last, for its line; 0 where it read none
static char release[RELEASE_BYTES];                         // the release text a pack offers, ended; empty without one
static u32 words_at;                                        // where the words being put together for the start screen end
static u32 words_cut;                                       // 1 once a piece of them found no room in the buffer
static u8 leaned_on[MAX_PACKS];                             // while count_leaning looks at one pack: 1 for each pack of the list it leans on

#ifdef OP1F_HOST
// On the PC the harness stands in for the clock, for stock's memory behind the sites, for the room packs are loaded
// into, for the loader's own tables and for the font's table of widths.
u32 cycles();
u32 site_word(u32 address);
void site_word_set(u32 address, u32 value);
u32 site_half(u32 address);
u8* pack_memory(u32 address);
const BootPatch* own_table(int end);
const BootPatch* text_table(int end);
u32 stock_byte(u32 address);
#else
static inline u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
static inline u32 site_word(u32 address) { return *(volatile u32*)address; }
static inline void site_word_set(u32 address, u32 value) { *(volatile u32*)address = value; }
static inline u32 site_half(u32 address) { return *(const volatile u16*)address; }
static inline u8* pack_memory(u32 address) { return (u8*)address; }
static inline u32 stock_byte(u32 address) { return *(const volatile u8*)address; }
// The loader's own two changes (build_l1_base_table.s, written by tools/boot_table.py: entries of the new kind).
extern "C" const BootPatch op1f_base_table[] asm("op1f_base_table");
extern "C" const BootPatch op1f_base_table_end[] asm("op1f_base_table_end");
static inline const BootPatch* own_table(int end) { return end ? op1f_base_table_end : op1f_base_table; }
// The loader's one change for the start screen's text (build_l1_base_text_table.s, the same).
extern "C" const BootPatch op1f_base_text_table[] asm("op1f_base_text_table");
extern "C" const BootPatch op1f_base_text_table_end[] asm("op1f_base_text_table_end");
static inline const BootPatch* text_table(int end) { return end ? op1f_base_text_table_end : op1f_base_text_table; }
#endif

// CRC-32 as zlib computes it: the reflected polynomial 0xEDB88320, the register all ones at the start and inverted
// at the end.
static void make_crc_table() {
    for (u32 i = 0; i < 256; ++i) {
        u32 c = i;
        for (int k = 0; k < 8; ++k) c = (c & 1) ? ((c >> 1) ^ 0xedb88320u) : (c >> 1);
        crc_table[i] = c;
    }
    crc_table_made = 1;
}

// The same sum over any bytes, for the packs of the new kind; the old loader below keeps its own loop, as proven.
static u32 crc_of(const u8* bytes, u32 count) {
    if (!crc_table_made) make_crc_table();
    u32 crc = 0xffffffffu;
    for (u32 i = 0; i < count; ++i) crc = crc_table[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
    return crc ^ 0xffffffffu;
}

static void clear(PackLoad* load) {
    u32* w = (u32*)load;
    for (u32 i = 0; i < sizeof(PackLoad) / 4; ++i) w[i] = 0;
}

static int finish(PackLoad* load, int code, int descriptor) {
    if (descriptor >= 0) STOCK_FILE_CLOSE(descriptor);
    load->code = code;
    return code;
}

// A pack of the old kind (J4's loader, unchanged). Read the pack at `path` into `room`, the memory packs are loaded
// into (`room_bytes` of it, standing at `load_address`), clear the zeroed data after its body and check its
// checksum. Nothing of the pack is run or looked at beyond its header; LOAD_OK means its bytes are in place as the
// header describes them.
int op1f_base_load(const char* path, u8* room, u32 room_bytes, u32 load_address, PackLoad* load) {
    clear(load);
    load->descriptor = -1;
    load->file_bytes = STOCK_FILE_SIZE(path);
    if (load->file_bytes < 0) return finish(load, LOAD_NO_FILE, -1);
    if (load->file_bytes < HEADER_BYTES) return finish(load, LOAD_NOT_A_PACK, -1);

    int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
    load->descriptor = descriptor;
    if (descriptor < 0) return finish(load, LOAD_NOT_READ, -1);
    if (STOCK_FILE_READ(header_words, 1, HEADER_BYTES, descriptor) != HEADER_BYTES) return finish(load, LOAD_NOT_READ, descriptor);
    PackHeader* h = &load->header;
    for (u32 i = 0; i < HEADER_BYTES / 4; ++i) ((u32*)h)[i] = header_words[i];
    static const char MAGIC[8] = { 'O', 'P', '1', 'F', 'P', 'A', 'C', 'K' };
    for (int i = 0; i < 8; ++i) {
        if (h->magic[i] != MAGIC[i]) return finish(load, LOAD_NOT_A_PACK, descriptor);
    }
    for (int i = 0; i < 16; ++i) load->name[i] = (h->name[i] >= 0x20 && h->name[i] < 0x7f) ? h->name[i] : 0;
    load->name[16] = 0;
    // What the loader knows by itself: the format, this OS, the one address, the room; and the header's own sums: the
    // body is the rest of the file, the table lies whole in the code and constant data, word aligned.
    if (h->format != PACK_FORMAT || h->header_bytes != HEADER_BYTES || h->os_id != OS_ID || h->load_address != load_address ||
        h->body_bytes == 0 || h->body_bytes != (u32)load->file_bytes - HEADER_BYTES ||
        h->body_bytes > room_bytes || h->zeroed_bytes > room_bytes - h->body_bytes ||
        h->compare_bytes == 0 || h->compare_bytes > h->body_bytes ||
        h->table < load_address || h->table_end < h->table || h->table_end - load_address > h->compare_bytes ||
        (h->table & 3) != 0 || h->table_end == h->table || h->table_end - h->table > MAX_ENTRIES * ENTRY_BYTES) {
        return finish(load, LOAD_BAD_HEADER, descriptor);
    }
    for (u32 at = h->table; at != h->table_end; at += ENTRY_BYTES) {    // a whole number of entries, without a division
        if (h->table_end - at < ENTRY_BYTES) return finish(load, LOAD_BAD_HEADER, descriptor);
    }

    u32 started = cycles();
    u32 offset = 0;
    while (offset < h->body_bytes) {
        u32 want = h->body_bytes - offset;
        if (want > PIECE_BYTES) want = PIECE_BYTES;
        if (STOCK_FILE_READ(room + offset, 1, (int)want, descriptor) != (int)want) {
            load->read_bytes = offset;
            load->read_cycles = cycles() - started;
            return finish(load, LOAD_NOT_READ, descriptor);
        }
        offset += want;
    }
    load->read_bytes = offset;
    load->read_cycles = cycles() - started;
    STOCK_FILE_CLOSE(descriptor);

    started = cycles();
    for (u32 i = 0; i < h->zeroed_bytes; ++i) room[h->body_bytes + i] = 0;
    if (!crc_table_made) make_crc_table();
    u32 crc = 0xffffffffu;
    for (u32 i = 0; i < h->body_bytes; ++i) crc = crc_table[(crc ^ room[i]) & 0xff] ^ (crc >> 8);
    load->crc = crc ^ 0xffffffffu;
    load->check_cycles = cycles() - started;
    return finish(load, load->crc == h->body_crc ? LOAD_OK : LOAD_BAD_CRC, -1);
}

// One line for each file of the old kind tried.
void op1f_base_say(const char* path, const PackLoad* load) {
    const PackHeader* h = &load->header;
    switch (load->code) {
    case LOAD_OK:
        STOCK_LOG(3, LOG_FILE, 11, "loader: %s '%s' loaded at 0x%08x: %u bytes read in %u cycles, %u cleared and checksum %08x ok in %u cycles",
                  path, load->name, h->load_address, load->read_bytes, load->read_cycles, h->zeroed_bytes, load->crc, load->check_cycles);
        break;
    case LOAD_NO_FILE:
        STOCK_LOG(2, LOG_FILE, 12, "loader: no file %s", path);
        break;
    case LOAD_NOT_A_PACK:
        STOCK_LOG(2, LOG_FILE, 13, "loader: %s holds no pack (%d bytes)", path, load->file_bytes);
        break;
    case LOAD_BAD_HEADER:
        STOCK_LOG(1, LOG_FILE, 14, "loader: %s '%s' does not fit this loader: format %u, header %u, os %08x, load 0x%08x, body %u, zeroed %u, table 0x%08x to 0x%08x, file %d",
                  path, load->name, h->format, h->header_bytes, h->os_id, h->load_address, h->body_bytes, h->zeroed_bytes, h->table, h->table_end, load->file_bytes);
        break;
    case LOAD_NOT_READ:
        STOCK_LOG(1, LOG_FILE, 15, "loader: %s could not be read: the open answered %d, %u body bytes read of %u, file %d bytes",
                  path, load->descriptor, load->read_bytes, h->body_bytes, load->file_bytes);
        break;
    default:
        STOCK_LOG(1, LOG_FILE, 16, "loader: %s '%s' read, %u bytes: its checksum is %08x and its header says %08x",
                  path, load->name, load->read_bytes, load->crc, h->body_crc);
        break;
    }
}

// The eight bytes an entry's site is to hold, as two little-endian words: `CALL.a target` (DE00) or `JUMP.a target`
// (DC00): the opcode, the address's high half, its low half, 0000; or `Rn = target`, the stock load's first
// half-word (DA0n) kept and its value replaced. A data word: the target.
static inline void after_words(const BootPatch* e, u32* out) {
    if (e->kind == KIND_WORD) {
        out[0] = e->target;
        out[1] = 0;
        return;
    }
    if (e->kind == KIND_LOAD_DDR) {
        out[0] = (e->before_lo & 0xffffu) | ((e->target >> 16) << 16);
    } else {
        u32 first = (e->kind == KIND_JUMP_L1 || e->kind == KIND_JUMP_DDR) ? 0xdc00u : 0xde00u;
        out[0] = first | ((e->target >> 16) << 16);
    }
    out[1] = e->target & 0xffffu;
}

static inline bool in_l1(u32 kind) {
    return kind == KIND_CALL_L1 || kind == KIND_JUMP_L1;
}

// What stands at an entry's site, as two words. A code site is only 2-byte aligned: half-word reads in DDR; L1 code
// is read under the control register's bit 16 (build_j_boot.s).
static void read_site(const BootPatch* e, u32* out) {
    if (e->kind == KIND_WORD) {
        out[0] = site_word(e->site);
        out[1] = 0;
    } else if (in_l1(e->kind)) {
        op1f_boot_read_l1(e->site, out);
    } else {
        out[0] = site_half(e->site) | (site_half(e->site + 2) << 16);
        out[1] = site_half(e->site + 4) | (site_half(e->site + 6) << 16);
    }
}

static void write_site(const BootPatch* e, const u32* after) {
    if (e->kind == KIND_WORD) {
        site_word_set(e->site, after[0]);
    } else if (in_l1(e->kind)) {
        op1f_boot_write_l1(e->site, after[0], after[1]);
    } else {
        op1f_boot_write_ddr(e->site, after[0], after[1]);
    }
}

static inline bool same(const u32* a, const u32* b) {
    return a[0] == b[0] && a[1] == b[1];
}

static inline u32 get16(const u8* at) { return at[0] | ((u32)at[1] << 8); }
static inline void put16(u8* at, u32 value) { at[0] = (u8)value; at[1] = (u8)(value >> 8); }

// What an entry is, whichever kind it is of.
struct Shape {
    u32 kind;               // KIND_WORD to KIND_LOAD_DDR
    u32 sum;                // 1: an entry of the new kind, with a checksum where stock's bytes were
    u32 trampoline;         // the bytes its trampoline copies, 0 without one
    u32 span;               // the bytes at its site it stands on: what it writes, or what its trampoline copies if that is more
};

// An entry's shape; false for a kind this loader does not know. An entry of the old kind is one of the six kinds and
// nothing else. One of the new kind has KIND_SUM and one of the six in its low half and a trampoline's bytes in its
// high half: none, or eight to fourteen, even, and only where the change is a call or a jump.
static bool shape_of(const BootPatch* e, Shape* shape) {
    bool known = true;
    shape->kind = e->kind;
    shape->sum = 0;
    shape->trampoline = 0;
    if (e->kind > KIND_LOAD_DDR) {
        shape->kind = e->kind & 0xfu;
        shape->sum = 1;
        shape->trampoline = e->kind >> 16;
        if ((e->kind & 0xfff0u) != KIND_SUM || shape->kind > KIND_LOAD_DDR) known = false;
        if (shape->trampoline != 0 && (shape->trampoline < TRAMPOLINE_LEAST || shape->trampoline > TRAMPOLINE_MOST || (shape->trampoline & 1) != 0 ||
                                       shape->kind == KIND_WORD || shape->kind == KIND_LOAD_DDR)) known = false;
    }
    shape->span = shape->kind == KIND_WORD ? 4 : shape->trampoline > 8 ? shape->trampoline : 8;
    return known;
}

static u32 span_of(const BootPatch* e) {
    Shape shape;
    shape_of(e, &shape);
    return shape.span;
}

// The first `count` bytes at an entry's site in the order of their addresses, for its checksum and its trampoline:
// the eight read_site read (a data word: four) and, where a trampoline copies more, the half-words behind them in
// DDR, or in L1 the last eight of them read under the control register's bit. `plain` is the entry with one of the
// six kinds as its kind.
OWN static void bytes_at(const BootPatch* plain, const u32* found, u32 count, u8* out) {
    for (u32 i = 0; i < count && i < 8; ++i) out[i] = (u8)(found[i >> 2] >> (8 * (i & 3)));
    if (count <= 8) return;
    if (in_l1(plain->kind)) {
        u32 last[2];
        op1f_boot_read_l1(plain->site + count - 8, last);
        for (u32 i = 8; i < count; ++i) {
            u32 k = i - (count - 8);
            out[i] = (u8)(last[k >> 2] >> (8 * (k & 3)));
        }
    } else {
        for (u32 i = 8; i < count; i += 2) {
            u32 half = site_half(plain->site + i);
            out[i] = (u8)half;
            out[i + 1] = (u8)(half >> 8);
        }
    }
}

// The trampoline of an entry, made in the pack's room before the hook is written: the bytes the hook displaces, as
// they stand at the site, and behind them a JUMP.a to the first instruction after them (DC00, the address's high
// half, its low half, 0000). The room is zeroed data of the pack, cleared when the pack was loaded and written by
// nobody since; it is written out of the data cache and dropped from the instruction cache as loaded code is.
OWN static void make_trampoline(u32 room_address, u32 site, u32 count, const u8* bytes) {
    u8* room = pack_memory(room_address);
    u32 back = site + count;
    for (u32 i = 0; i < count; ++i) room[i] = bytes[i];
    put16(room + count, 0xdc00u);
    put16(room + count + 2, back >> 16);
    put16(room + count + 4, back & 0xffffu);
    put16(room + count + 6, 0);
    op1f_base_flush(room_address, room_address + count + JUMP_BYTES);
}

static void clear(TableResult* result) {
    u32* w = (u32*)result;
    for (u32 i = 0; i < sizeof(TableResult) / 4; ++i) w[i] = 0;
    result->index = -1;
}

static int failed(TableResult* result, int code, int index, const BootPatch* e, const u32* found, const u32* wanted) {
    result->code = code;
    result->index = index;
    result->site = e->site;
    result->kind = e->kind;
    result->found[0] = found[0];
    result->found[1] = found[1];
    result->wanted[0] = wanted[0];
    result->wanted[1] = wanted[1];
    return code;
}

// Can this table be applied whole? Nothing is written. Each entry: a kind the loader knows; a site where its kind may
// lie, on its alignment; a target inside [code_start, code_end), the code the table came with; at the site the
// stock bytes the entry names, or already what it writes; and no byte shared with an entry before it.
// An entry of the new kind: at the site the bytes its checksum was made from, or, without a trampoline, already what
// it writes (a hook that is in has lost the bytes a trampoline would copy); a load on a site that holds a load of a
// register; a trampoline's room whole inside [code_end, zero_end), the zeroed data the table came with, on a code
// address, and shared with no entry before it; no room without a trampoline.
OWN static int check_table(const BootPatch* table, const BootPatch* table_end, u32 code_start, u32 code_end, u32 zero_end, TableResult* result) {
    clear(result);
    u32 found[2], wanted[2], after[2];
    u8 bytes[TRAMPOLINE_MOST + 2];
    Shape shape, theirs;
    BootPatch plain;
    int index = 0;
    for (const BootPatch* e = table; e != table_end; ++e, ++index) {
        wanted[0] = e->before_lo;
        wanted[1] = e->before_hi;
        found[0] = e->kind;
        found[1] = 0;
        if (!shape_of(e, &shape)) return failed(result, TABLE_KIND, index, e, found, wanted);
        u32 low = in_l1(shape.kind) ? L1_CODE_START : DDR_START;
        u32 high = in_l1(shape.kind) ? L1_CODE_END : DDR_END;
        found[0] = e->site;
        if (e->site < low || e->site > high - shape.span || (e->site & (shape.kind == KIND_WORD ? 3u : 1u)) != 0) {
            return failed(result, TABLE_SITE, index, e, found, wanted);
        }
        found[0] = e->target;
        if (e->target < code_start || e->target >= code_end) return failed(result, TABLE_TARGET, index, e, found, wanted);
        u32 room = shape.sum ? e->before_hi : 0;
        u32 need = shape.trampoline + JUMP_BYTES;
        found[0] = room;
        if (shape.sum && (shape.trampoline == 0 ? room != 0
                          : (zero_end < code_end || zero_end - code_end < need || room < code_end || room > zero_end - need || (room & 1) != 0))) {
            return failed(result, TABLE_ROOM, index, e, found, wanted);
        }
        int other = 0;
        for (const BootPatch* o = table; o != e; ++o, ++other) {
            shape_of(o, &theirs);
            found[1] = (u32)other;
            if (o->site < e->site + shape.span && e->site < o->site + theirs.span) {
                found[0] = o->site;
                return failed(result, TABLE_SHARED, index, e, found, wanted);
            }
            if (shape.trampoline != 0 && theirs.trampoline != 0 && o->before_hi < room + need && room < o->before_hi + theirs.trampoline + JUMP_BYTES) {
                return failed(result, TABLE_ROOM, index, e, found, wanted);
            }
        }
        if (in_l1(shape.kind) && !l1_announced) {
            l1_announced = 1;
            STOCK_LOG(3, LOG_FILE, 3, "loader: the first access to L1 code is next: entry %d, site 0x%08x", index, e->site);
        }
        plain.kind = shape.kind;
        plain.site = e->site;
        plain.target = e->target;
        plain.before_lo = e->before_lo;
        plain.before_hi = e->before_hi;
        read_site(&plain, found);
        if (!shape.sum) {
            after_words(&plain, after);
            if (same(found, after)) {
                ++result->already;
            } else if (!same(found, wanted)) {
                return failed(result, TABLE_BYTES, index, e, found, wanted);
            }
        } else {
            plain.before_lo = found[0];                     // a load keeps the first half-word the site has: its register
            after_words(&plain, after);
            bytes_at(&plain, found, shape.span, bytes);
            wanted[1] = crc_of(bytes, shape.span);          // the entry's checksum, and the site's
            if (shape.kind == KIND_LOAD_DDR && ((found[0] & 0xfff0u) != 0xda00u || (found[1] >> 16) != 0)) {
                return failed(result, TABLE_FORM, index, e, found, wanted);
            }
            if (shape.trampoline == 0 && same(found, after)) {
                ++result->already;
            } else if (wanted[1] != wanted[0]) {
                return failed(result, TABLE_BYTES, index, e, found, wanted);
            }
            ++result->sums;
        }
        ++result->entries;
    }
    return TABLE_OK;
}

// The check of a table that came with no zeroed data, so that none of its entries may have a trampoline: the
// loader's own.
int op1f_base_check(const BootPatch* table, const BootPatch* table_end, u32 code_start, u32 code_end, TableResult* result) {
    return check_table(table, table_end, code_start, code_end, code_end, result);
}

// Apply a table that passed the check: each site found as the check found it, written, read back. The first
// entry that is not so ends the pass with what was found; the entries before it are in. An entry whose number is in
// `skips` is left to another pack and not touched. An entry with a trampoline: the trampoline first, made from the
// bytes just read, then the hook.
OWN static int write_table(const BootPatch* table, const BootPatch* table_end, const u16* skips, u32 skip_count, TableResult* result) {
    clear(result);
    u32 found[2], wanted[2], after[2], back[2];
    u8 bytes[TRAMPOLINE_MOST + 2];
    Shape shape;
    BootPatch plain;
    int index = 0;
    for (const BootPatch* e = table; e != table_end; ++e, ++index) {
        bool skip = false;
        for (u32 s = 0; s < skip_count; ++s) skip = skip || skips[s] == (u16)index;
        ++result->entries;
        if (skip) {
            ++result->left;
            continue;
        }
        shape_of(e, &shape);
        plain.kind = shape.kind;
        plain.site = e->site;
        plain.target = e->target;
        plain.before_lo = e->before_lo;
        plain.before_hi = e->before_hi;
        wanted[0] = e->before_lo;
        wanted[1] = e->before_hi;
        read_site(&plain, found);
        if (shape.sum) {
            plain.before_lo = found[0];
            bytes_at(&plain, found, shape.span, bytes);
            wanted[1] = crc_of(bytes, shape.span);
        }
        after_words(&plain, after);
        if (shape.trampoline == 0 && same(found, after)) {
            ++result->already;
            continue;
        }
        if (shape.sum ? wanted[1] != wanted[0] : !same(found, wanted)) return failed(result, TABLE_CHANGED, index, e, found, wanted);
        if (shape.trampoline != 0) {
            make_trampoline(e->before_hi, e->site, shape.trampoline, bytes);
            ++result->trampolines;
        }
        write_site(&plain, after);
        read_site(&plain, back);
        if (!same(back, after)) return failed(result, TABLE_NOT_TAKEN, index, e, back, after);
        if (shape.kind == KIND_WORD) ++result->words;
        else if (in_l1(shape.kind)) ++result->in_l1;
        else ++result->in_ddr;
        result->sums += shape.sum;
    }
    return TABLE_OK;
}

int op1f_base_write(const BootPatch* table, const BootPatch* table_end, TableResult* result) {
    return write_table(table, table_end, 0, 0, result);
}

// Ask the keyboard controller which keys are down. Stock's request sends the message 0x41 with no payload (a null
// payload of no bytes is what stock itself hands over for its own requests without one), waits for the answer up to
// the timeout and copies it whole into the buffer: byte 2 its type, byte 3 its payload's length, the payload from
// byte 4, here the map with its low byte first. The answer is this request's by its reply id, so its type is not
// looked at, as te-boot does not look at it (the OS's own key map message has the type 0x42); it goes into the log.
// The buffer is not on the stack: an answer that came after the time was up would be copied into it all the same.
// The link's listener, the OS's key map handler, is out of the way while the request runs (J5a).
int op1f_base_keys(KeysRead* keys) {
    u32 started = cycles();
    keys->code = KEYS_NO_ANSWER;
    keys->type = 0;
    keys->bytes = 0;
    keys->low = 0;
    keys->high = 0;
    for (u32 i = 0; i < sizeof keys_message / 4; ++i) keys_message[i] = 0;
    keys->listener = *STOCK_KB_LISTENER;
    keys->aside = keys->listener == KB_KEY_HANDLER;
    if (keys->aside) *STOCK_KB_LISTENER = 0;
    u32 answered = STOCK_KB_REQUEST(STOCK_KB_LINK, KEYS_REQUEST, 0, 0, keys_message, KEYS_TIMEOUT_MS);
    if (keys->aside) *STOCK_KB_LISTENER = keys->listener;
    keys->cycles = cycles() - started;
    if (!answered) return keys->code;
    const u8* message = (const u8*)keys_message;
    keys->type = message[2];
    keys->bytes = message[3];
    if (keys->bytes < MAP_BYTES) return keys->code = KEYS_SHORT_ANSWER;
    keys->low = keys_message[1];
    keys->high = keys_message[2];
    return keys->code = KEYS_OK;
}

static const char* said(int code) {
    switch (code) {
    case TABLE_KIND: return "a kind this loader does not know";
    case TABLE_SITE: return "a site outside stock's memory or off its alignment";
    case TABLE_TARGET: return "a target outside the code it came with";
    case TABLE_BYTES: return "not the stock bytes at the site";
    case TABLE_SHARED: return "two entries on the same bytes";
    case TABLE_CHANGED: return "the site changed after the check";
    case TABLE_ROOM: return "a trampoline's room outside the zeroed data, or shared";
    case TABLE_FORM: return "a load entry on a site that holds no load of a register";
    default: return "the write did not take";
    }
}

// The loader's own two changes, the disk mode's folder "packs": checked whole, then written. TABLE_BYTES when a site
// is not stock's any more: a pack of the old kind has taken it for its own folders.
OWN static int add_folder(TableResult* result) {
    int own = op1f_base_check(own_table(0), own_table(1), BASE_START, LOADER_END, result);
    if (own == TABLE_OK) own = op1f_base_write(own_table(0), own_table(1), result);
    return own;
}

// Stop the start-up for good: the line, stock's assert, and a loop should the assert ever come back.
static void stop(const TableResult* r) {
    STOCK_LOG(1, LOG_FILE, 9, "loader: STOPPED, %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
              said(r->code), r->index, r->site, r->kind, r->found[0], r->found[1], r->wanted[0], r->wanted[1]);
    STOCK_ASSERT("op1f loader stopped the start-up");
    for (;;) {
    }
}

// ---------------------------------------------------------------- the start screen's text

// A text's width in the bitmap font's columns, as stock's own width function counts it: each character its width
// and one column more; a byte outside the font is drawn as a question mark.
OWN static u32 width_of(const char* text) {
    u32 columns = 0;
    for (; *text; ++text) {
        u32 c = (u8)*text;
        if (c < FONT_FIRST || c > FONT_LAST) c = '?';
        columns += stock_byte(FONT_WIDTHS + (c - FONT_FIRST)) + 1;
    }
    return columns;
}

static u32 put(char* out, u32 at, u32 room, const char* text) {
    for (; *text && at + 1 < room; ++text) out[at++] = *text;
    out[at] = 0;
    return at;
}

static u32 put_number(char* out, u32 at, u32 room, u32 value) {
    char digits[10];
    int n = 0;
    do {                                                    // no division: a count of packs is far below a hundred
        u32 tens = 0;
        while (value >= 10) { value -= 10; ++tens; }
        digits[n++] = (char)('0' + value);
        value = tens;
    } while (value && n < 10);
    while (n && at + 1 < room) out[at++] = digits[--n];
    out[at] = 0;
    return at;
}

// What the start screen is to say after stock's version text: three pieces put together, cut to the buffer.
static void screen_says(const char* a, const char* b, const char* c) {
    u32 at = 0;
    for (; *a && at < SCREEN_WORDS_BYTES - 1; ++a) screen_words[at++] = *a;
    for (; *b && at < SCREEN_WORDS_BYTES - 1; ++b) screen_words[at++] = *b;
    for (; *c && at < SCREEN_WORDS_BYTES - 1; ++c) screen_words[at++] = *c;
    screen_words[at] = 0;
}

// The loader's one change for the start screen, the CALL.a at stock's draw of the version text: checked, then written.
// Where the site is not stock's, the text stays stock's. A write that does not take stops the start-up, as a pack's
// does: the site is run in a moment.
OWN static void add_screen_text(TableResult* result) {
    int code = op1f_base_check(text_table(0), text_table(1), BASE_START, LOADER_END, result);
    if (code != TABLE_OK) {
        STOCK_LOG(2, LOG_FILE, 26, "loader: the start screen keeps stock's version text, %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                  said(code), result->index, result->site, result->kind, result->found[0], result->found[1], result->wanted[0], result->wanted[1]);
        return;
    }
    if (op1f_base_write(text_table(0), text_table(1), result) != TABLE_OK) stop(result);
    STOCK_LOG(3, LOG_FILE, 25, "loader: the start screen's version text is followed by '%s': %u in DDR code written and read back, %u already there",
              screen_words, result->in_ddr, result->already);
}

// Called by the stub at stock's draw of the version text (build_j5b_base.s), once a start, on the main thread:
// stock's text in, the text to draw out. Stock's own pointer goes back when there is nothing to add or the whole
// would not be sure to fit the screen.
const char* op1f_base_screen_text(const char* stock_text) {
    u32 stock_chars = 0;
    u32 words = 0;
    if (stock_text) {
        while (stock_chars <= SCREEN_STOCK_CHARS && stock_text[stock_chars]) ++stock_chars;
    }
    while (screen_words[words]) ++words;
    u32 columns = 0;
    if (stock_chars != 0 && stock_chars <= SCREEN_STOCK_CHARS) columns = width_of(stock_text) + width_of(screen_words);
    if (stock_chars == 0 || stock_chars > SCREEN_STOCK_CHARS || words == 0 || columns > SCREEN_COLUMNS) {
        STOCK_LOG(2, LOG_FILE, 28, "loader: the start screen keeps stock's version text: %u character(s) of stock's and %u of ours, %u columns of %u",
                  stock_chars, words, columns, (u32)SCREEN_COLUMNS);
        return stock_text;
    }
    u32 at = 0;
    for (u32 i = 0; i < stock_chars; ++i) screen_text[at++] = stock_text[i];
    for (u32 i = 0; i < words; ++i) screen_text[at++] = screen_words[i];
    screen_text[at] = 0;
    STOCK_LOG(3, LOG_FILE, 27, "loader: the start screen says '%s'", screen_text);
    return screen_text;
}

// ---------------------------------------------------------------- the folder's pack files

static bool same_text(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static bool before(const char* a, const char* b) {         // the order of the names: byte by byte
    while (*a && *a == *b) { ++a; ++b; }
    return (u8)*a < (u8)*b;
}

// A pack file's name: six to thirty-nine characters of the plain kind that end in ".pack" (in small letters or
// capitals), none of them one a computer's file names cannot hold, the first neither a dot nor a space. The disk
// mode's folder asks the same function (build_m1_loader_disk.cpp): what the folder shows and takes is what the load
// point reads.
int op1f_base_pack_name(const char* name) {
    u32 n = 0;
    while (name[n]) {
        u32 c = (u8)name[n];
        if (c < 0x20 || c > 0x7e || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') return 0;
        if (++n >= FILE_CHARS) return 0;
    }
    if (n < 6 || name[0] == '.' || name[0] == ' ') return 0;
    return name[n - 5] == '.' && (name[n - 4] | 0x20) == 'p' && (name[n - 3] | 0x20) == 'a' && (name[n - 2] | 0x20) == 'c' && (name[n - 1] | 0x20) == 'k';
}

static void path_of(const Pack* p, char* path) {            // PATH_CHARS bytes: the prefix's eight and a name of at most thirty-nine
    u32 at = put(path, 0, PATH_CHARS, PACK_PREFIX);
    put(path, at, PATH_CHARS, p->file);
}

static void add_file(const char* name) {
    if (pack_count == MAX_PACKS) {
        ++folder_more;
        return;
    }
    Pack* p = &packs[pack_count++];
    put(p->file, 0, FILE_CHARS, name);
}

static const char MAGIC[8] = { 'O', 'P', '1', 'F', 'P', 'A', 'C', 'K' };

// The folder's pack files, in the order of their names, each with its size and, for a file long enough, its header.
// Where the folder cannot be listed, or lists no pack file: the two names of J4 to J5b. Nothing of a pack is read
// into the room yet.
OWN static void look() {
    void* folder = STOCK_DIR_OPEN(PACK_DIR);
    if (folder) {
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(folder);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (op1f_base_pack_name(name)) add_file(name);
            else if (name[0] != '.') ++folder_other;
        }
        STOCK_DIR_CLOSE(folder);
        for (u32 i = 1; i < pack_count; ++i) {              // an insertion sort of the names: nothing else is filled in yet
            for (u32 k = i; k > 0 && before(packs[k].file, packs[k - 1].file); --k) {
                for (u32 c = 0; c < FILE_CHARS; ++c) {
                    char swap = packs[k].file[c];
                    packs[k].file[c] = packs[k - 1].file[c];
                    packs[k - 1].file[c] = swap;
                }
            }
        }
    }
    u32 listed = pack_count;
    if (listed == 0) {
        add_file(OLD_FIRST);
        add_file(OLD_SPARE);
    }
    u32 old = 0, fresh = 0, there = 0;
    char path[PATH_CHARS];
    for (u32 i = 0; i < pack_count; ++i) {
        Pack* p = &packs[i];
        path_of(p, path);
        p->state = P_NONE;
        p->size = STOCK_FILE_SIZE(path);
        if (p->size < 0) {
            STOCK_LOG(2, LOG_FILE, 12, "loader: no file %s", path);
            continue;
        }
        ++there;
        if (p->size < HEADER_BYTES) {
            STOCK_LOG(2, LOG_FILE, 13, "loader: %s holds no pack (%d bytes)", path, p->size);
            continue;
        }
        int want = HEADER3_BYTES;
        if (p->size < HEADER3_BYTES) want = HEADER_BYTES;
        for (u32 w = 0; w < HEADER3_BYTES / 4; ++w) header_words[w] = 0;
        int got = -1;
        int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
        if (descriptor >= 0) {
            got = STOCK_FILE_READ(header_words, 1, want, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
        if (got != want) {                                  // a file that cannot be read may be a pack: it counts as left out
            p->state = P_OUT;
            p->why = WHY_READ;
            STOCK_LOG(1, LOG_FILE, 37, "loader: %s is left out: its header could not be read (the open answered %d, %d bytes read of %d, file %d bytes)",
                      path, descriptor, got, want, p->size);
            continue;
        }
        for (u32 w = 0; w < HEADER3_BYTES / 4; ++w) ((u32*)&p->h)[w] = header_words[w];
        bool magic = true;
        for (int c = 0; c < 8; ++c) magic = magic && p->h.magic[c] == MAGIC[c];
        if (!magic) {
            STOCK_LOG(2, LOG_FILE, 13, "loader: %s holds no pack (%d bytes)", path, p->size);
            continue;
        }
        for (int c = 0; c < 16; ++c) p->name[c] = (p->h.name[c] >= 0x20 && p->h.name[c] < 0x7f) ? p->h.name[c] : 0;
        p->name[16] = 0;
#if OP1F_LOADER_GENERATION == 2
        // N2 runs no pack of the old kind: format 2 is a format this loader does not know (line 38), and line 30 counts 0 of it
        if (p->h.format == PACK_FORMAT_LINKED && want == HEADER3_BYTES) {
#else
        if (p->h.format == PACK_FORMAT) {
            p->state = P_OLD;
            ++old;
        } else if (p->h.format == PACK_FORMAT_LINKED && want == HEADER3_BYTES) {
#endif
            p->state = P_NEW;
            ++fresh;
        } else {
            p->state = P_OUT;                               // a pack for another loader
            p->why = WHY_HEADER;
            STOCK_LOG(1, LOG_FILE, 38, "loader: %s '%s' is left out: a pack of format %u, which this loader does not know (file %d bytes)",
                      path, p->name, p->h.format, p->size);
        }
    }
    STOCK_LOG(folder ? 3 : 2, LOG_FILE, 30, "loader: the folder %s %s: %u pack(s) of the new kind and %u of the old in %u file(s); %u more not looked at, %u other name(s)",
              PACK_DIR, !folder ? "cannot be listed, 01.pack and 02.pack tried" : listed ? "listed" : "lists no pack file, 01.pack and 02.pack tried",
              fresh, old, there, folder_more, folder_other);
}

// One place of a pack set to an address: the high or the low half of an address loaded into a register (sixteen
// bits), a data word, or the distance of a direct call or jump, whose instruction starts two bytes before the
// place: its first half-word keeps the opcode in its high byte and has the distance's top eight bits in its low
// byte, its second half-word has the low sixteen; the distance counts in steps of two bytes from the instruction.
OWN static void set_place(u8* body, u32 body_address, u32 offset, u32 kind, u32 address) {
    u8* at = body + offset;
    if (kind == PLACE_HIGH) {
        put16(at, address >> 16);
    } else if (kind == PLACE_LOW) {
        put16(at, address & 0xffffu);
    } else if (kind == PLACE_WORD) {
        put16(at, address & 0xffffu);
        put16(at + 2, address >> 16);
    } else {
        u32 steps = ((address - (body_address + offset - 2)) >> 1) & 0xffffffu;
        at[-2] = (u8)(steps >> 16);
        put16(at, steps & 0xffffu);
    }
}

static const char* text_at(const Pack* p, u32 offset) { return p->strings + offset; }

// A pack of the new kind read into the room at `place`: its header held against what the loader knows, its body and
// its lists read straight into place and their checksum checked, the lists held to their own counts, and the pack
// moved: the distance between where it was linked for and where it stands added at each place of its own. Answers 0,
// or why not; nothing of stock is touched.
// A table may have no entry (M1): a pack that changes no place of the firmware. And where the header's last word
// names a card, the card's place is held with the header and its first two words once the checksum has passed; they
// are kept in card_mark and card_bytes for the line of a pack that is left out for them.
OWN static int load3(Pack* p, u32 place, const char* path) {
    Header3* h = &p->h;
    u32 room_end = PACK_BASE + PACK_ROOM;
    bool align_ok = h->align == 1 || h->align == 2 || h->align == 4 || h->align == 8 || h->align == 16 || h->align == PACK_ALIGN;
    if (h->header_bytes != HEADER3_BYTES || h->os_id != OS_ID || (h->link_address & 3) != 0 || !align_ok ||
        h->body_bytes == 0 || (h->body_bytes & 3) != 0 || h->body_bytes > PACK_ROOM || h->zeroed_bytes > PACK_ROOM ||
        h->lists_bytes < 4 * LIST_COUNTS || (h->lists_bytes & 3) != 0 || h->lists_bytes > PACK_ROOM || h->extra_bytes > 0x40000000u ||
        (u32)p->size != HEADER3_BYTES + h->body_bytes + h->lists_bytes + h->extra_bytes ||
        h->compare_bytes == 0 || h->compare_bytes > h->body_bytes ||
        h->table < h->link_address || h->table_end < h->table || h->table_end - h->link_address > h->compare_bytes ||
        (h->table & 3) != 0 || h->table_end - h->table > MAX_ENTRIES * ENTRY_BYTES ||
        (h->path_place != 0 && ((h->path_place & 3) != 0 || h->body_bytes < PATH_CHARS || h->path_place > h->body_bytes - PATH_CHARS))) {
        return WHY_HEADER;
    }
    for (u32 at = h->table; at != h->table_end; at += ENTRY_BYTES) {    // a whole number of entries, without a division
        if (h->table_end - at < ENTRY_BYTES) return WHY_HEADER;
    }
    card_mark = 0;
    card_bytes = 0;
    if (h->card_place != 0 && ((h->card_place & 3) != 0 || h->body_bytes < CARD_LEAST || h->card_place > h->body_bytes - CARD_LEAST)) return WHY_CARD;
    u32 behind = h->lists_bytes > h->zeroed_bytes ? h->lists_bytes : h->zeroed_bytes;
    p->foot = h->body_bytes + behind;
    if (place > room_end || p->foot > room_end - place) return WHY_ROOM;

    u8* room = pack_memory(place);
    u32 total = h->body_bytes + h->lists_bytes;
    u32 started = cycles();
    int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
    if (descriptor < 0) return WHY_READ;
    if (STOCK_FILE_READ(header_words, 1, HEADER3_BYTES, descriptor) != HEADER3_BYTES) {    // the header once more: the file is read from its start
        STOCK_FILE_CLOSE(descriptor);
        return WHY_READ;
    }
    u32 offset = 0;
    while (offset < total) {
        u32 want = total - offset;
        if (want > PIECE_BYTES) want = PIECE_BYTES;
        if (STOCK_FILE_READ(room + offset, 1, (int)want, descriptor) != (int)want) {
            STOCK_FILE_CLOSE(descriptor);
            return WHY_READ;
        }
        offset += want;
    }
    STOCK_FILE_CLOSE(descriptor);
    p->read_cycles = cycles() - started;

    started = cycles();
    p->crc = crc_of(room, total);
    p->check_cycles = cycles() - started;
    if (p->crc != h->crc) return WHY_SUM;

    // the card: its mark, and a size of whole words that holds its first two and ends inside the body
    if (h->card_place != 0) {
        const u32* card = (const u32*)(room + h->card_place);
        card_mark = card[0];
        card_bytes = card[1];
        if (card_mark != CARD_MARK || (card_bytes & 3) != 0 || card_bytes < CARD_LEAST || card_bytes > h->body_bytes - h->card_place) return WHY_CARD;
    }

    // the lists: six counts, five lists of whole entries, the strings; together exactly what the header says
    const u32* counts = (const u32*)(room + h->body_bytes);
    p->works_n = counts[0];
    p->exports_n = counts[1];
    p->imports_n = counts[2];
    p->places_n = counts[3];
    p->marks_n = counts[4];
    p->strings_n = counts[5];
    u32 limit = h->lists_bytes;
    if (p->works_n > limit || p->exports_n > limit || p->imports_n > limit || p->places_n > limit || p->marks_n > limit || p->strings_n > limit ||
        p->imports_n > 0xfffeu) return WHY_LISTS;
    u32 fixed = 4 * LIST_COUNTS + 12 * p->works_n + 8 * p->exports_n + 8 * p->imports_n + 12 * p->places_n + 8 * p->marks_n;
    if (fixed > limit || fixed + p->strings_n != limit || (p->strings_n & 3) != 0) return WHY_LISTS;
    p->works = counts + LIST_COUNTS;
    p->exports = p->works + 3 * p->works_n;
    p->imports = p->exports + 2 * p->exports_n;
    p->places = p->imports + 2 * p->imports_n;
    p->marks = p->places + 3 * p->places_n;
    p->strings = (const char*)(p->marks + 2 * p->marks_n);
    bool names = p->works_n + p->exports_n + p->imports_n > 0;
    if (names && (p->strings_n == 0 || p->strings[p->strings_n - 1] != 0)) return WHY_LISTS;    // every name ends inside the strings
    for (u32 i = 0; i < p->works_n; ++i) if (p->works[3 * i] >= p->strings_n) return WHY_LISTS;
    for (u32 i = 0; i < p->exports_n; ++i) {                // a name it offers lies in the pack, its zeroed data included
        if (p->exports[2 * i] >= p->strings_n || p->exports[2 * i + 1] >= h->body_bytes + h->zeroed_bytes) return WHY_LISTS;
    }
    for (u32 i = 0; i < p->imports_n; ++i) if (p->imports[2 * i] >= p->strings_n) return WHY_LISTS;
    u32 entries = 0;
    for (u32 at = h->table; at != h->table_end; at += ENTRY_BYTES) ++entries;
    if (p->marks_n > MAX_SKIPS) return WHY_LISTS;
    for (u32 i = 0; i < p->marks_n; ++i) if (p->marks[2 * i] >= entries || p->marks[2 * i + 1] >= p->works_n) return WHY_LISTS;
    for (u32 i = 0; i < p->places_n; ++i) {
        u32 at = p->places[3 * i], kind = p->places[3 * i + 1] & 0xffu, wanted = p->places[3 * i + 1] >> 16;
        u32 size = kind == PLACE_WORD ? 4 : 2, low = kind == PLACE_CALL ? 2 : 0;
        if (kind > PLACE_CALL || (p->places[3 * i + 1] & 0xff00u) != 0 || (at & 1) != 0 || at < low || at > h->body_bytes - size) return WHY_LISTS;
        if (wanted > p->imports_n) return WHY_LISTS;
        if (kind == PLACE_CALL && (wanted == 0 || (p->imports[2 * (wanted - 1) + 1] & NEEDED) == 0)) return WHY_LISTS;    // a call of the pack's own needs no place; one of an optional name cannot be made safe
    }

    // moved: its own addresses are those it was linked with, further on by the distance
    u32 distance = place - h->link_address;
    p->moved = 0;
    for (u32 i = 0; i < p->places_n; ++i) {
        if (p->places[3 * i + 1] >> 16) continue;
        if (distance != 0) set_place(room, place, p->places[3 * i], p->places[3 * i + 1] & 0xffu, p->places[3 * i + 2] + distance);
        ++p->moved;
    }
    p->place = place;
    return 0;
}

static bool in_set(const Pack* p) { return p->state == P_IN || p->state == P_RUNS; }

OWN static Pack* pack_named(const char* name, const Pack* but) {
    for (u32 i = 0; i < pack_count; ++i) {
        if (&packs[i] != but && in_set(&packs[i]) && same_text(packs[i].name, name)) return &packs[i];
    }
    return 0;
}

// The address of a name among the packs of the set, but for `self`; 0 when none offers it. The loader's own mark
// (M1) is a name the loader answers for itself, before any pack is asked: its own first byte.
OWN static u32 address_of(const char* name, const Pack* self) {
    if (same_text(name, NAME_LOADER)) return BASE_START;
#if OP1F_LOADER_GENERATION == 2
    if (same_text(name, NAME_LOADER_2)) return (u32)op1f_loader_record;      // N2: its second mark, the record's address
#endif
    for (u32 i = 0; i < pack_count; ++i) {
        const Pack* q = &packs[i];
        if (q == self || !in_set(q)) continue;
        for (u32 e = 0; e < q->exports_n; ++e) {
            if (same_text(text_at(q, q->exports[2 * e]), name)) return q->place + q->exports[2 * e + 1];
        }
    }
    return 0;
}

// The pack of the set that offers a name, but for `self`: the first in the order of the files, as address_of takes
// it. Answers its number in the list and, through `at`, where the name stands; -1 when no pack offers it. The
// loader's own mark is no pack's.
OWN static int pack_with(const char* name, const Pack* self, u32* at) {
    for (u32 i = 0; i < pack_count; ++i) {
        const Pack* q = &packs[i];
        if (q == self || !in_set(q)) continue;
        for (u32 e = 0; e < q->exports_n; ++e) {
            if (same_text(text_at(q, q->exports[2 * e]), name)) {
                *at = q->place + q->exports[2 * e + 1];
                return (int)i;
            }
        }
    }
    return -1;
}

// Is this entry of the pack's table left to another pack: marked "only when it is away", and it is in the set?
OWN static bool left_to_another(const Pack* p, u32 entry) {
    for (u32 m = 0; m < p->marks_n; ++m) {
        if (p->marks[2 * m] == entry && pack_named(text_at(p, p->works[3 * p->marks[2 * m + 1]]), p)) return true;
    }
    return false;
}

static const BootPatch* table_of(const Pack* p, int end) {
    u32 distance = p->place - p->h.link_address;
    return (const BootPatch*)pack_memory((end ? p->h.table_end : p->h.table) + distance);
}

// An entry stands on the bytes it writes and on those its trampoline copies.
static bool meet(const BootPatch* a, const BootPatch* b) {
    return a->site < b->site + span_of(b) && b->site < a->site + span_of(a);
}

// Does a change of this pack meet one of the loader's own or of a pack before it in the set? Entries left to another
// pack are not made and meet nothing.
OWN static bool claims_meet(const Pack* p, u32 index, const Pack** other, u32* site) {
    u32 number = 0;
    for (const BootPatch* e = table_of(p, 0); e != table_of(p, 1); ++e, ++number) {
        if (left_to_another(p, number)) continue;
        for (int t = 0; t < 2; ++t) {
            const BootPatch* end = t ? text_table(1) : own_table(1);
            for (const BootPatch* o = t ? text_table(0) : own_table(0); o != end; ++o) {
                if (meet(e, o)) { *other = 0; *site = e->site; return true; }
            }
        }
        for (u32 i = 0; i < index; ++i) {
            const Pack* q = &packs[i];
            if (!in_set(q)) continue;
            u32 theirs = 0;
            for (const BootPatch* o = table_of(q, 0); o != table_of(q, 1); ++o, ++theirs) {
                if (!left_to_another(q, theirs) && meet(e, o)) { *other = q; *site = e->site; return true; }
            }
        }
    }
    return false;
}

static void say_out(const Pack* p, const char* what, const char* a, u32 x, u32 y) {
    STOCK_LOG(1, LOG_FILE, 33, "loader: %s '%s' is left out: %s %s (%u, %u)", p->file, p->name, what, a, x, y);
}

// Does a name begin as a loader's mark does? Such a name is no pack's to offer.
OWN static bool a_loaders_mark(const char* name) {
    const char* head = NAME_LOADERS;
    while (*head && *name == *head) { ++name; ++head; }
    return *head == 0;
}

// Is this pack in the folder, though it does not run? A file of the folder that was read as a pack of that name with
// nothing wrong of its own (its header, its checksum, its card and its lists in order): one that is in the set as
// it stands, one that stands by, or one left out when the set was settled, for a pack it needs, a name or a place of
// stock. A file that could not be read as that pack, damaged or cut short, is not: its pack is away.
OWN static bool in_the_folder(const char* name) {
    for (u32 i = 0; i < pack_count; ++i) {
        const Pack* q = &packs[i];
        bool read = q->state == P_IN || q->state == P_SPARE || (q->state == P_OUT && q->why >= WHY_NEEDS && q->why <= WHY_CLAIM);
        if (read && same_text(q->name, name)) return true;
    }
    return false;
}

// The name of the pack a file is left out for want of, kept for the start screen: fifteen characters of it at most,
// a question mark for one that is not plain. Nothing is kept, an empty name, where that pack is in the folder and
// does not run: the screen names a pack as needed only when copying its file is the step to take, and counts this
// file with the others left out.
OWN static void keep_missing(u32 index, const char* name) {
    u32 n = 0;
    if (!in_the_folder(name)) {
        for (; name[n] && n < MISSING_BYTES - 1; ++n) {
            u32 c = (u8)name[n];
            missing[index][n] = (c >= 0x20 && c <= 0x7e) ? (char)c : '?';
        }
    }
    missing[index][n] = 0;
}

// May this pack of the set run beside the others of the set? 0, or why not, with the line that says it: the lists
// are still in the room. Nothing is written.
OWN static int judge(Pack* p, u32 index, TableResult* result) {
    for (u32 w = 0; w < p->works_n; ++w) {
        const char* other = text_at(p, p->works[3 * w]);
        const Pack* q = pack_named(other, p);
        if (!q && (p->works[3 * w + 2] & NEEDED)) {
            say_out(p, "it needs the pack", other, 0, 0);
            keep_missing(index, other);
            return WHY_NEEDS;
        }
        if (q && q->h.interface_ != p->works[3 * w + 1]) {
            say_out(p, "it was made for another interface of the pack", other, p->works[3 * w + 1], q->h.interface_);   // (its number, the running one's)
            return WHY_INTERFACE;
        }
    }
    for (u32 e = 0; e < p->exports_n; ++e) {
        const char* name = text_at(p, p->exports[2 * e]);
        if (a_loaders_mark(name)) {                         // a loader's mark, this one's or a later one's, is nobody's to offer
            say_out(p, "it offers a name the loader keeps to itself,", name, 0, 0);
            return WHY_NAME;
        }
        for (u32 i = 0; i < index; ++i) {
            const Pack* q = &packs[i];
            if (!in_set(q)) continue;
            for (u32 o = 0; o < q->exports_n; ++o) {
                if (same_text(text_at(q, q->exports[2 * o]), name)) {
                    say_out(p, "an earlier pack offers the same name", name, i, 0);
                    return WHY_NAME;
                }
            }
        }
    }
    for (u32 i = 0; i < p->imports_n; ++i) {
        if ((p->imports[2 * i + 1] & NEEDED) && address_of(text_at(p, p->imports[2 * i]), p) == 0) {
            say_out(p, "nobody offers the name it needs,", text_at(p, p->imports[2 * i]), 0, 0);
            return WHY_IMPORT;
        }
    }
    for (u32 i = 0; i < p->places_n; ++i) {
        if ((p->places[3 * i + 1] & 0xffu) != PLACE_CALL) continue;
        const char* name = text_at(p, p->imports[2 * ((p->places[3 * i + 1] >> 16) - 1)]);
        i32 distance = (i32)(address_of(name, p) + p->places[3 * i + 2] - (p->place + p->places[3 * i] - 2));
        if ((distance & 1) != 0 || distance < -(1 << 24) || distance >= (1 << 24)) {
            say_out(p, "a direct call does not reach the name", name, p->places[3 * i], 0);
            return WHY_REACH;
        }
    }
    u32 body_end = p->place + p->h.body_bytes;              // where the pack's zeroed data starts: a trampoline's room lies there
    if (check_table(table_of(p, 0), table_of(p, 1), p->place, body_end, body_end + p->h.zeroed_bytes, result) != TABLE_OK) {
        STOCK_LOG(1, LOG_FILE, 42, "loader: '%s' is left out, %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                  p->name, said(result->code), result->index, result->site, result->kind,
                  result->found[0], result->found[1], result->wanted[0], result->wanted[1]);
        return WHY_TABLE;
    }
    const Pack* other = 0;
    u32 site = 0;
    if (claims_meet(p, index, &other, &site)) {             // the place as an address, which is how every other line names a site
        STOCK_LOG(1, LOG_FILE, 49, "loader: %s '%s' is left out: one of its changes of stock, at 0x%08x, meets one of %s",
                  p->file, p->name, site, other ? other->name : "the loader");
        return WHY_CLAIM;
    }
    return 0;
}

static const char* why_text(int why) {
    switch (why) {
    case WHY_HEADER: return "its header does not fit this loader";
    case WHY_READ: return "it could not be read";
    case WHY_SUM: return "its checksum is not its header's";
    case WHY_LISTS: return "its lists are not in order";
    case WHY_CARD: return "its card is not in order";
    default: return "no room is left for it";
    }
}

// ---------------------------------------------------------------- what M1 adds to the run of the packs

// How many packs of the set lean on each, for the order of the tables. A pack leans on a pack it names as one it
// needs, as before, and (M1) on the pack that offers a name it needs; a pack it leans on in both ways, or by several
// names, counts it once. The lists still stand.
OWN static void count_leaning() {
    for (u32 k = 0; k < pack_count; ++k) {
        const Pack* q = &packs[k];
        if (q->state != P_IN) continue;
        for (u32 i = 0; i < pack_count; ++i) leaned_on[i] = 0;
        for (u32 w = 0; w < q->works_n; ++w) {
            if (!(q->works[3 * w + 2] & NEEDED)) continue;
            for (u32 i = 0; i < pack_count; ++i) {
                if (i != k && packs[i].state == P_IN && same_text(text_at(q, q->works[3 * w]), packs[i].name)) leaned_on[i] = 1;
            }
        }
        for (u32 n = 0; n < q->imports_n; ++n) {
            u32 at = 0;
            if (!(q->imports[2 * n + 1] & NEEDED)) continue;
            int i = pack_with(text_at(q, q->imports[2 * n]), q, &at);
            if (i >= 0) leaned_on[i] = 1;
        }
        for (u32 i = 0; i < pack_count; ++i) packs[i].rank += leaned_on[i];
    }
}

// The release a pack offers under the name op1f_release, taken for the start screen while the lists stand: one to
// fifteen characters from '!' to '~', ended by a zero, in the body of the pack that offers it. Anything else is not
// shown, and one line says why.
OWN static void take_release() {
    u32 at = 0;
    int owner = pack_with(NAME_RELEASE, 0, &at);
    if (owner < 0) return;
    const Pack* q = &packs[owner];
    const u8* room = pack_memory(q->place);
    u32 offset = at - q->place;
    const char* why = 0;
    u32 n = 0;
    while (!why) {
        if (n == RELEASE_BYTES) {
            why = "it is not ended within sixteen bytes";
        } else if (offset + n >= q->h.body_bytes) {
            why = "it does not lie whole in the pack's body";
        } else {
            u32 c = room[offset + n];
            if (c == 0) break;
            if (c < 0x21 || c > 0x7e) why = "it holds a character that is not plain";
            else release[n++] = (char)c;
        }
    }
    if (!why && n == 0) why = "it is empty";
    if (why) n = 0;
    release[n] = 0;
    if (why) STOCK_LOG(2, LOG_FILE, 54, "loader: the release '%s' offers is not shown: %s", q->name, why);
}

// The cards handed over, once the packs' zeroed data is cleared and before any table is written. The list a pack
// offers under the name op1f_cards (`owner` its number in the list of pack files, -1 for none; `list_at` where the
// list stands) gets a slot for each pack that runs and has a card, in the order of the files: where the card stands,
// the pack's first address, and the address behind the last byte of its body and zeroed data. The list is held to
// lie whole in the pack that offers it, on a word. It holds CARDS_MOST slots, as many as the loader takes pack files;
// a card that finds none is counted in the list's second word. N2 calls it once for each list offered, op1f_cards
// with CARDS_MOST slots and op1f_cards_2 with CARDS_MOST_2, each list held to its own size; with neither offered,
// once with -1 for line 53.
#if OP1F_LOADER_GENERATION == 2
OWN static void hand_cards(int owner, u32 list_at, u32 most) {
    const u32 words = 2 + SLOT_WORDS * most;               // the list's words: its two, and three a slot
#else
OWN static void hand_cards(int owner, u32 list_at) {
    const u32 most = CARDS_MOST, words = CARDS_WORDS;
#endif
    if (owner < 0) {
        u32 carried = 0;
        for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_RUNS && packs[i].h.card_place) ++carried;
        if (carried) STOCK_LOG(2, LOG_FILE, 53, "loader: %u pack(s) that run carry a card, and no pack offers a list for them", carried);
        return;
    }
    const Pack* q = &packs[owner];
    u32 bytes = q->h.body_bytes + q->h.zeroed_bytes;        // the pack's memory from its first byte on
    if ((list_at & 3) != 0 || bytes < 4 * words || list_at - q->place > bytes - 4 * words) {
        STOCK_LOG(1, LOG_FILE, 52, "loader: the list of cards '%s' offers is not used: it does not lie whole in the pack, on a word (0x%08x)", q->name, list_at);
        return;
    }
    u32* list = (u32*)pack_memory(list_at);
    u32 count = 0, more = 0;
    for (u32 i = 0; i < pack_count; ++i) {
        const Pack* p = &packs[i];
        if (p->state != P_RUNS || !p->h.card_place) continue;
        if (count == most) {
            ++more;
            continue;
        }
        u32* slot = list + 2 + SLOT_WORDS * count++;
        slot[0] = p->place + p->h.card_place;
        slot[1] = p->place;
        slot[2] = p->place + p->h.body_bytes + p->h.zeroed_bytes;
    }
    for (u32 w = 2 + SLOT_WORDS * count; w < words; ++w) list[w] = 0;    // a slot that is not used holds nothing, wherever the list lies
    list[0] = count;
    list[1] = more;
    STOCK_LOG(3, LOG_FILE, 51, "loader: %u card(s) handed to '%s', its list at 0x%08x; %u more than the list holds", count, q->name, list_at, more);
}

// ---------------------------------------------------------------- the packs of the new kind

// Read every pack of the new kind into the room, settle which run, join them, and write their tables. Answers how
// many run; 0 leaves the room as it is found and nothing of stock written. With M1: a line for a file that stands
// by and for a card that is not in order; the release taken and the list for the cards found while the lists stand;
// the cards handed over between the clearing and the tables.
OWN static u32 run_new(TableResult* result, u32 started) {
    char path[PATH_CHARS];
    u32 next = PACK_BASE;
    // place and read: of the files that hold the same pack, the highest version; the others stand by
    for (u32 i = 0; i < pack_count; ++i) {
        Pack* p = &packs[i];
        while (p->state == P_NEW) {
            Pack* best = p;
            for (u32 k = i + 1; k < pack_count; ++k) {
                Pack* q = &packs[k];
                if (q->state == P_NEW && same_text(q->name, p->name) && q->h.version > best->h.version) best = q;
            }
            path_of(best, path);
            u32 place = next + ((best->h.link_address - next) & (PACK_ALIGN - 1));    // the next address a multiple of 32 from where it was linked for
            int why = load3(best, place, path);
            if (why == 0) {
                best->state = P_IN;
                next = best->place + ((best->foot + 3) & ~3u);
                STOCK_LOG(3, LOG_FILE, 31, "loader: %s '%s' version %u is placed at 0x%08x, %d from where it was linked: %u places of its own moved",
                          path, best->name, best->h.version, best->place, (i32)(best->place - best->h.link_address), best->moved);
                STOCK_LOG(3, LOG_FILE, 39, "loader: its %u bytes read in %u cycles, checksum %08x ok in %u cycles",
                          best->h.body_bytes + best->h.lists_bytes, best->read_cycles, best->crc, best->check_cycles);
                for (u32 k = 0; k < pack_count; ++k) {      // the other files with this pack are not needed: each stands by, and a line says so (M1)
                    Pack* other = &packs[k];
                    if (other == best || other->state != P_NEW || !same_text(other->name, best->name)) continue;
                    other->state = P_SPARE;
                    STOCK_LOG(3, LOG_FILE, 48, "loader: %s%s '%s' version %u stands by and is not read: %s%s holds this pack, version %u",
                              PACK_PREFIX, other->file, other->name, other->h.version, PACK_PREFIX, best->file, best->h.version);
                }
            } else {
                best->state = P_OUT;
                best->why = why;
                STOCK_LOG(1, LOG_FILE, 32, "loader: %s '%s' is left out: %s", path, best->name, why_text(why));
                STOCK_LOG(1, LOG_FILE, 40, "loader: its header: format %u, size %u, os %08x, body %u, lists %u, zeroed %u, file %d bytes; checksum %08x, its header's %08x",
                          best->h.format, best->h.header_bytes, best->h.os_id, best->h.body_bytes, best->h.lists_bytes, best->h.zeroed_bytes, best->size,
                          best->crc, best->h.crc);
                if (why == WHY_CARD) {
                    STOCK_LOG(1, LOG_FILE, 50, "loader: its card: at %u in a body of %u bytes, mark %08x, size %u", best->h.card_place, best->h.body_bytes, card_mark, card_bytes);
                }
            }
        }
    }

    // settle which run: until nothing changes
    bool changed = true;
    while (changed) {
        changed = false;
        for (u32 i = 0; i < pack_count; ++i) {
            Pack* p = &packs[i];
            if (p->state != P_IN) continue;
            int why = judge(p, i, result);
            if (why) {
                p->state = P_OUT;
                p->why = why;
                changed = true;
            }
        }
    }
    u32 running = 0;
    for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_IN) ++running;
    if (running == 0) return 0;

    // the set stands: what each leaves to another, how many lean on each, the two names the disk mode's folder asks
    // for, and (M1) the list for the cards and the release, each by its name
    for (u32 i = 0; i < pack_count; ++i) {
        Pack* p = &packs[i];
        if (p->state != P_IN) continue;
        u32 entry = 0;
        for (const BootPatch* e = table_of(p, 0); e != table_of(p, 1); ++e, ++entry) {
            if (left_to_another(p, entry) && p->skip_count < MAX_SKIPS) p->skips[p->skip_count++] = (u16)entry;
        }
    }
    count_leaning();
    op1f_base_more_child = address_of(NAME_MORE_CHILD, 0);
    op1f_base_more_populate = address_of(NAME_MORE_POPULATE, 0);
    u32 cards_at = 0;
    int cards_owner = pack_with(NAME_CARDS, 0, &cards_at);
#if OP1F_LOADER_GENERATION == 2
    u32 cards2_at = 0;
    int cards2_owner = pack_with(NAME_CARDS_2, 0, &cards2_at);
#endif
    take_release();

    // join: every wanted name's places filled with the address of the pack that offers it, or with nothing
    for (u32 i = 0; i < pack_count; ++i) {
        Pack* p = &packs[i];
        if (p->state != P_IN) continue;
        u8* room = pack_memory(p->place);
        for (u32 n = 0; n < p->imports_n; ++n) {
            if (address_of(text_at(p, p->imports[2 * n]), p)) ++p->names;
            else ++p->absent;
        }
        for (u32 k = 0; k < p->places_n; ++k) {
            u32 wanted = p->places[3 * k + 1] >> 16;
            if (wanted == 0) continue;
            u32 found = address_of(text_at(p, p->imports[2 * (wanted - 1)]), p);     // 0: an optional name nobody offers, and the place says so
            set_place(room, p->place, p->places[3 * k], p->places[3 * k + 1] & 0xffu, found + p->places[3 * k + 2]);
            if (found) ++p->filled;
        }
        if (p->h.path_place) {
            path_of(p, path);
            u8* to = room + p->h.path_place;
            u32 at = 0;
            for (; path[at] && at < PATH_CHARS - 1; ++at) to[at] = (u8)path[at];
            for (; at < PATH_CHARS; ++at) to[at] = 0;
        }
    }
    // the lists are done with: each pack's sum as linked where it asks for one, its zeroed data cleared, its memory
    // out of the data cache. From here on nothing reads a pack's lists.
    for (u32 i = 0; i < pack_count; ++i) {
        Pack* p = &packs[i];
        if (p->state != P_IN) continue;
        u8* room = pack_memory(p->place);
        if (p->h.flags & FLAG_SUM) p->sum = crc_of(room, p->h.body_bytes);
        for (u32 z = 0; z < p->h.zeroed_bytes; ++z) room[p->h.body_bytes + z] = 0;
        p->works = p->exports = p->imports = p->places = p->marks = 0;
        p->strings = 0;
        p->state = P_RUNS;
        op1f_base_flush(p->place, p->place + p->h.body_bytes + p->h.zeroed_bytes);
    }
#if OP1F_LOADER_GENERATION == 2
    // N2: each list offered gets the cards, op1f_cards its 32 slots and op1f_cards_2 its 128; line 53 only when neither is
    if (cards_owner >= 0 || cards2_owner < 0) hand_cards(cards_owner, cards_at, CARDS_MOST);
    if (cards2_owner >= 0) hand_cards(cards2_owner, cards2_at, CARDS_MOST_2);
#else
    hand_cards(cards_owner, cards_at);                      // the cards (M1): into zeroed data that is cleared, before anything of a pack can run
#endif

    // the loader's own folder first, then the tables: a pack other packs lean on after those that lean on it
    int own = add_folder(result);
    if (own == TABLE_OK) {
        STOCK_LOG(3, LOG_FILE, 34, "loader: the disk mode's folder 'packs' is the loader's: %u word, %u in DDR code written and read back; a pack's folders after it: %s",
                  result->words, result->in_ddr, op1f_base_more_populate ? "yes" : "no");
    } else {
        STOCK_LOG(1, LOG_FILE, 17, "loader: its disk mode has no folder 'packs', %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                  said(own), result->index, result->site, result->kind, result->found[0], result->found[1], result->wanted[0], result->wanted[1]);
    }
    for (u32 rank = 0; rank <= pack_count; ++rank) {
        for (u32 i = 0; i < pack_count; ++i) {
            Pack* p = &packs[i];
            if (p->state != P_RUNS || p->rank != rank) continue;
            if (write_table(table_of(p, 0), table_of(p, 1), p->skips, p->skip_count, result) != TABLE_OK) stop(result);
            STOCK_LOG(3, LOG_FILE, 35, "loader: '%s' runs: %u words, %u in DDR code, %u in L1 code written and read back, %u already there, %u left to another pack",
                      p->name, result->words, result->in_ddr, result->in_l1, result->already, result->left);
            STOCK_LOG(3, LOG_FILE, 41, "loader: '%s' is joined: %u names of other packs in %u places, %u optional names nobody offers; its sum as linked %08x",
                      p->name, p->names, p->filled, p->absent, p->sum);
            if (result->sums) {
                STOCK_LOG(3, LOG_FILE, 43, "loader: '%s' has %u of its changes held by a checksum in place of stock's bytes; %u trampolines made in its zeroed data",
                          p->name, result->sums, result->trampolines);
            }
        }
    }
    u32 out = 0;
    for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_OUT) ++out;
    STOCK_LOG(3, LOG_FILE, 36, "loader: done: %u pack(s) run, %u left out, control 0x%08x, %u cycles", running, out, op1f_boot_control(), cycles() - started);
    return running;
}

// ---------------------------------------------------------------- the words for the start screen (M1; M4's where packs run)

// A piece of the words that follow stock's version text, put behind what is there; a piece that finds no room in the
// buffer is cut, and words_cut says so.
OWN static void say(const char* text) {
    for (; *text; ++text) {
        if (words_at + 1 < SCREEN_WORDS_BYTES) screen_words[words_at++] = *text;
        else words_cut = 1;
    }
    screen_words[words_at] = 0;
}

// A number among the words. Its digits are counted here and not by put_number, which so keeps the two callers it has
// in L1's unit and is compiled as there.
OWN static void say_number(u32 value) {
    char digits[12];
    u32 n = sizeof digits - 1;
    digits[n] = 0;
    do {                                                    // no division, as in put_number: a count of pack files is far below a hundred
        u32 tens = 0;
        while (value >= 10) { value -= 10; ++tens; }
        digits[--n] = (char)('0' + value);
        value = tens;
    } while (value && n);
    say(digits + n);
}

static void say_from_the_start(const char* text) {
    words_at = 0;
    words_cut = 0;
    say(text);
}

// The pack files left out, as the start screen says them. Where at least one was left out for want of a pack, all
// of those want the same pack and no file of the folder holds it (keep_missing), that pack is named, "4 need kit" or
// "1 needs kit", and behind it the files left out for another reason, ", 1 left out". Otherwise, and where `plain`
// asks for it, one count of them all: "5 left out". Nothing is said where none was left out.
OWN static void say_left_out(bool plain) {
    u32 out = 0, in_want = 0;
    const char* pack = 0;
    bool one_pack = true;
    for (u32 i = 0; i < pack_count; ++i) {
        if (packs[i].state != P_OUT) continue;
        ++out;
        if (packs[i].why != WHY_NEEDS) continue;
        ++in_want;
        if (!pack) pack = missing[i];
        else if (!same_text(pack, missing[i])) one_pack = false;
    }
    if (out == 0) return;
    if (plain || in_want == 0 || !one_pack || !pack[0]) {
        say_number(out);
        say(" left out");
        return;
    }
    say_number(in_want);
    say(in_want == 1 ? " needs " : " need ");
    say(pack);
    if (out != in_want) {
        say(", ");
        say_number(out - in_want);
        say(" left out");
    }
}

// One form of the words put together (M4): " - open-field", the release where it is to be said and there is one,
// and in brackets the pack files left out, by say_left_out's rule; no brackets where none was left out. Answers
// whether the whole found room in the buffer and fits its share of the screen.
OWN static bool words_fit(bool with_release, bool plain) {
    u32 out = 0;
    for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_OUT) ++out;
    say_from_the_start(" - open-field");
    if (with_release && release[0]) {
        say(" ");
        say(release);
    }
    if (out) {
        say(" (");
        say_left_out(plain);
        say(")");
    }
    return !words_cut && width_of(screen_words) <= WORDS_COLUMNS;
}

// What the start screen says where pack files run (M4): the words " - open-field",
// the release, and the pack files left out. The names of the packs that run are not said, and neither is how many
// run. The first of these forms that fits the buffer and the words' share of the screen:
//
//   " - open-field 0.3 (2 need base, 1 left out)"    the files left out, and the pack they want where M1's rule names it
//   " - open-field 0.3 (3 left out)"                 one count of all the files left out
//   " - open-field (3 left out)"                     and without the release, which always fits
//
// With no file left out the first form is " - open-field 0.3", or " - open-field" where no pack offers a release.
// `running` is the caller's count of the packs that run, which the words no longer say: it is kept so that the one
// call of this function stands in op1f_boot_apply as in M1's unit.
OWN static void words_for_the_set(u32 running) {
    (void)running;
    if (words_fit(true, false)) return;
    if (words_fit(true, true)) return;
    words_fit(false, true);
}

// The words of a start as stock firmware where pack files were left out, by M1's rule: " stock (4 need kit)",
// " stock (4 need base, 1 left out)", " stock (2 left out)". The entry below is L1's to the letter but for its first
// line's name and its one call of this function: it puts L1's words together as before, a count of the files left
// out, and this function says them anew behind it. So the entry compiles to L1's code with one call more, which is
// worth three statements whose work is done twice. Where no file was left out it changes nothing: with the help key
// held no file was looked at, and with no pack file the words are L1's.
OWN static void stock_words() {
    u32 out = 0;
    for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_OUT) ++out;
    if (out == 0) return;
    say_from_the_start(" stock (");
    say_left_out(false);
    say(")");
}

void op1f_boot_apply() {
    u32 started = cycles();
    u32 control_before = op1f_boot_control();
    static PackLoad load;
    static TableResult result;
    // The loader's own three changes looked at before anything else, by the check they are checked with again when
    // they are made: the first line says how many of them pass, and how many of those by a checksum. Reads only.
    u32 changes = 0, passed = 0, by_sum = 0;
    for (const BootPatch* e = own_table(0); e != own_table(1); ++e) ++changes;
    for (const BootPatch* e = text_table(0); e != text_table(1); ++e) ++changes;
    op1f_base_check(own_table(0), own_table(1), BASE_START, LOADER_END, &result);
    passed += result.entries;
    by_sum += result.sums;
    op1f_base_check(text_table(0), text_table(1), BASE_START, LOADER_END, &result);
    passed += result.entries;
    by_sum += result.sums;
#if OP1F_LOADER_GENERATION == 2
    STOCK_LOG(3, LOG_FILE, 1, "loader: N2 at the load point: control 0x%08x, packs are loaded from 0x%08x, room %u bytes; "
#else
    STOCK_LOG(3, LOG_FILE, 1, "loader: M4 at the load point: control 0x%08x, packs are loaded from 0x%08x, room %u bytes; "
#endif
              "%u of its own %u changes of stock pass their check, %u by a checksum",
              control_before, PACK_BASE, PACK_ROOM, passed, changes, by_sum);

    // The self-test of a code write in DDR: a function that has just run, rewritten, run again.
    u32 before_test = op1f_boot_probe();
    op1f_boot_write_ddr((u32)&op1f_boot_probe, op1f_boot_probe_alt[0], op1f_boot_probe_alt[1]);
    u32 after_test = op1f_boot_probe();
    STOCK_LOG(3, LOG_FILE, 2, "loader: self-test of a code write in DDR: %u before, %u after", before_test, after_test);
    if (before_test != 1 || after_test != 2) {
        STOCK_LOG(1, LOG_FILE, 5, "loader: the self-test failed: no stock code is changed, and the unit starts as stock firmware");
        return;
    }

    // The safe start: which keys are down, asked of the keyboard controller as te-boot asks at every start. With the
    // help key among them no pack is read, and the unit starts as stock firmware with the loader's folder "packs".
    static KeysRead keys;
    u32 held = 0;
    if (op1f_base_keys(&keys) == KEYS_OK) {
        held = (keys.low >> SAFE_KEY) & 1;
        STOCK_LOG(held ? 2 : 3, LOG_FILE, 22, "loader: keys at the start %08x %08x (answer 0x%02x, %u bytes, %u cycles): the help key is %s",
                  keys.high, keys.low, keys.type, keys.bytes, keys.cycles, held ? "held, no pack is read" : "not held");
    } else {
        STOCK_LOG(2, LOG_FILE, 23, "loader: no key map from the keyboard controller (%s, type 0x%02x, %u bytes, %u cycles): no safe start, the packs are read",
                  keys.code == KEYS_NO_ANSWER ? "no answer" : "a short answer", keys.type, keys.bytes, keys.cycles);
    }
    if (!keys.aside) {
        STOCK_LOG(2, LOG_FILE, 24, "loader: the keyboard link's listener is 0x%08x, not the key map handler 0x%08x: it was left as it is",
                  keys.listener, KB_KEY_HANDLER);
    }

    char path[PATH_CHARS];
    if (!held) {
        look();
        bool fresh = false;
        for (u32 i = 0; i < pack_count; ++i) fresh = fresh || packs[i].state == P_NEW;
        // The packs of the new kind: several, each where the loader places it.
        if (fresh) {
            u32 running = run_new(&result, started);
            if (running) {
                words_for_the_set(running);
                add_screen_text(&result);
                return;
            }
        }
#if OP1F_LOADER_GENERATION == 1
        // A pack of the old kind, as J5b runs it: the first file that loads and whose table can be applied whole.
        // Nothing of stock is written before a table has passed. (Not in N2, which leaves such a file out in look.)
        const PackHeader* h = &load.header;
        for (u32 i = 0; i < pack_count; ++i) {
            if (packs[i].state != P_OLD) continue;
            path_of(&packs[i], path);
            int code = op1f_base_load(path, pack_memory(PACK_BASE), PACK_ROOM, PACK_BASE, &load);
            op1f_base_say(path, &load);
            if (code != LOAD_OK) {
                packs[i].state = P_OUT;                     // a pack file that does not run: the start screen counts it
                packs[i].why = code == LOAD_BAD_CRC ? WHY_SUM : code == LOAD_BAD_HEADER ? WHY_HEADER : WHY_READ;
                continue;
            }
            const BootPatch* table = (const BootPatch*)pack_memory(h->table);
            const BootPatch* table_end = (const BootPatch*)pack_memory(h->table_end);
            if (check_table(table, table_end, PACK_BASE, PACK_BASE + h->body_bytes, PACK_BASE + h->body_bytes + h->zeroed_bytes, &result) != TABLE_OK) {
                STOCK_LOG(1, LOG_FILE, 6, "loader: %s '%s' is not applied, %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                          path, load.name, said(result.code), result.index, result.site, result.kind,
                          result.found[0], result.found[1], result.wanted[0], result.wanted[1]);
                packs[i].state = P_OUT;
                packs[i].why = WHY_TABLE;
                continue;
            }
            STOCK_LOG(3, LOG_FILE, 7, "loader: %s '%s': its %u changes of stock checked, %u already there; nothing written yet",
                      path, load.name, result.entries, result.already);

            // The pack's code out of the data cache, then its table: every entry, or the start-up stops.
            op1f_base_flush(PACK_BASE, PACK_BASE + h->body_bytes + h->zeroed_bytes);
            if (op1f_base_write(table, table_end, &result) != TABLE_OK) stop(&result);
            STOCK_LOG(3, LOG_FILE, 4, "loader: done: pack '%s' from %s runs: %u words, %u in DDR code, %u in L1 code written and read back, "
                      "%u already there, control 0x%08x, %u cycles",
                      load.name, path, result.words, result.in_ddr, result.in_l1, result.already, op1f_boot_control(), cycles() - started);
            if (result.sums) {
                STOCK_LOG(3, LOG_FILE, 43, "loader: '%s' has %u of its changes held by a checksum in place of stock's bytes; %u trampolines made in its zeroed data",
                          load.name, result.sums, result.trampolines);
            }

            // A pack that left the disk mode's two sites alone gets the loader's folder beside it, so that it can be replaced.
            int own = add_folder(&result);
            if (own == TABLE_OK) {
                STOCK_LOG(3, LOG_FILE, 18, "loader: the pack leaves the disk mode alone: the loader's folder 'packs' is added, %u word, %u in DDR code written and read back",
                          result.words, result.in_ddr);
            } else if (own != TABLE_BYTES) {
                STOCK_LOG(1, LOG_FILE, 19, "loader: the loader's folder 'packs' could not be added, %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                          said(own), result.index, result.site, result.kind, result.found[0], result.found[1], result.wanted[0], result.wanted[1]);
            }

            // The start screen: the pack's name after stock's version text, and how many pack files were left out.
            u32 out = 0;
            for (u32 k = 0; k < pack_count; ++k) if (packs[k].state == P_OUT) ++out;
            u32 at = put(screen_words, 0, SCREEN_WORDS_BYTES, " custom");
            if (load.name[0]) {
                at = put(screen_words, at, SCREEN_WORDS_BYTES, " (");
                at = put(screen_words, at, SCREEN_WORDS_BYTES, load.name);
                if (out) {
                    at = put(screen_words, at, SCREEN_WORDS_BYTES, ", ");
                    at = put_number(screen_words, at, SCREEN_WORDS_BYTES, out);
                    at = put(screen_words, at, SCREEN_WORDS_BYTES, " left out");
                }
                put(screen_words, at, SCREEN_WORDS_BYTES, ")");
            }
            add_screen_text(&result);
            return;
        }
#endif
    }

    // No pack, or the help key held: the unit runs as stock. The loader's own two changes give the disk mode the
    // folder "packs"; when they cannot be made the start goes on all the same, without the folder.
    STOCK_LOG(2, LOG_FILE, 8, "loader: no pack runs: the unit starts as stock firmware");
    int own = add_folder(&result);
    if (own == TABLE_OK) {
        STOCK_LOG(3, LOG_FILE, 10, "loader: its disk mode shows the folder 'packs': %u word, %u in DDR code written and read back, "
                  "%u already there, control 0x%08x, %u cycles",
                  result.words, result.in_ddr, result.already, op1f_boot_control(), cycles() - started);
    } else {
        STOCK_LOG(1, LOG_FILE, 17, "loader: its disk mode has no folder 'packs', %s: entry %d, site 0x%08x, kind %u, found %08x %08x, wanted %08x %08x",
                  said(own), result.index, result.site, result.kind, result.found[0], result.found[1], result.wanted[0], result.wanted[1]);
    }
    u32 out = 0;
    for (u32 i = 0; i < pack_count; ++i) if (packs[i].state == P_OUT) ++out;
    if (held) {
        screen_says(" stock (", "help key", ")");
    } else if (out) {
        u32 at = put(screen_words, 0, SCREEN_WORDS_BYTES, " stock (");
        at = put_number(screen_words, at, SCREEN_WORDS_BYTES, out);
        put(screen_words, at, SCREEN_WORDS_BYTES, " left out)");
    } else {
        screen_says(" stock (", "no pack", ")");
    }
    stock_words();                                          // M1: where files were left out the words are said anew, with the pack they are in want of
    add_screen_text(&result);
}

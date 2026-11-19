// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step M3, the base pack: M1's picture loader (base.pack 13 to 15's, which this tree does not hold) with one call
// added, in op1f_picture: a document made for a pack's own picture is told to the cards unit (op1f_cards_made,
// module/build_m3_cards.cpp), which keeps the documents each card made, so that a page is opened and a text is
// written only on a document the pack made. Nothing else of the text is changed.
//
// What follows is M1's text.
//
// Step M1, the base pack: the picture loader that also serves the packs that carry a card (OS 1.7.0+0). This is
// the picture loader of the pack `kit` that went before, with lines added and none changed or taken out:
//
//   - The four packs this unit knows by name are asked first, in their order, as before. When one of them serves a
//     name, the cards are looked through for the same name, so that one line can say that two packs carry it
//     (op1f_cards_also); the pack that served it before serves it still.
//   - A name none of the four has is looked for among the cards (op1f_cards_picture), in the order of the packs'
//     file names: the first card whose pack has it serves it. The picture is then read from that pack's file and
//     checked, made or parsed by the same function as a picture of the four (from_file), with the same lines.
//   - A pack's own picture, for a page of its own: op1f_picture(card, name) looks into the directory of that card's
//     pack alone, by the name itself.
//
// The lookup among the cards is the cards' unit's (module/build_m3_cards.cpp). It answers a directory and an entry
// and has returned before a file is opened, so what stands on the stack under stock's parser is this unit's own, as
// before. With three callers the function that reads a picture (from_file) is a function of its own now, where the
// compiler had put it into its one caller: its instructions are the elder's own, the loader calls it and returns,
// and under stock's parser the two stand on 280 bytes of the stack with the entry's, where the elder stood on 228
// and stock's own loader stands on 340 (a test of the project's holds all three numbers and the instructions).
//
// What follows is the elder's text.
//
// Pictures step P3: the picture loader of P2 with room for eight stock pictures in one set (OS 1.7.0+0).
// This is P2's loader, which is kept as it is for the kit of P2,
// with two lines changed and two added: a set of the second kind may take stretches from eight stock pictures where
// it took them from four (STOCKS), and the kit offers a third mark, op1f_art_maker_3, which a pack names when one of
// its sets takes more than four. The sidechain's page does: the flow drawing and the meters (signalflow.svg), the
// letters (op_font_34.svg), the track digits (tape.svg), the parameter card (param_popup.svg) and the metronome
// (tempo.svg). The bound on what a set may ask of stock's heap is as it was: a megabyte for the picture and a
// megabyte for its stock pictures together. The instructions, their words and every refusal are P2's.
//
// What follows is P2's text.
//
// Pictures step P2: the picture loader that puts a picture together on the unit (OS 1.7.0+0).
// This is P1's loader, which is kept as it is for the kit of P1,
// with a second kind of instructions added. P1's kind is a stock picture with text of ours put in before its end.
// The second kind is a list of stretches: of the unit's own stock pictures as they are, of the same with their
// geometry mapped (moved or scaled: build_p2_map.cpp), and of text of ours, one behind the other. That is enough
// for every picture a pack shows in place of a stock one: "shrink the tape" is one mapped stretch, the whole
// picture; "draw that part of theirs here" is a stretch of the part, kept or mapped; a number set, a part doubled
// or taken out, a part put elsewhere are stretches in another order with text of ours between them. The pack
// carries offsets, lengths and what is ours; tools/picture_delta.py works them out on the PC.
//
// The second kind, one instruction a line (tools/picture_make.py is the same steps on the PC):
//   op1f picture 2                  the first line
//   size N                          the second: the bytes of the picture that is made
//   from NAME BYTES CRC             a stock picture the stretches come from: the unit's own content/display/NAME,
//                                   BYTES long with that CRC-32, or nothing is made; up to four, numbered from 0
//   c S OFFSET LENGTH               LENGTH bytes of the stock picture S from OFFSET, as they are
//   m S OFFSET LENGTH SCALE DX DY   the same stretch mapped: x' = SCALE/1000 x + DX/1000, y' = SCALE/1000 y + DY/1000
//   a BYTES                         the BYTES bytes behind this line (and a line feed), which are ours
// Each stock picture is read whole into a buffer of its own and given back when the picture is made or refused.
//
// op1f_art_maker_2 is the mark of a kit that knows the second kind; a pack whose pictures use it names the mark
// outright, as P1's packs name op1f_art_maker_1, so that an older kit never sees them.
//
// What follows is P1's text.
//
// Pictures step P1: the picture loader that makes a picture on the unit (OS 1.7.0+0).
// This is J5g's loader, which is kept as it is for the packs before P1, with one thing added: what a pack
// carries under a picture's name may be a set of instructions in place of the picture. The loader reads and checks
// it as it reads and checks a picture; when it begins with the line `op1f picture 1` the picture is made here, from
// the unit's own stock picture and what the instructions carry, and that is what stock's parser is handed. So a
// pack holds no picture made from a stock picture: the five LFO pages of `fx train` are the stock page and four
// (the midi page: sixteen) marker groups of ours, and the pack now carries the groups alone.
//
// The instructions, one a line (tools/picture_make.py is the same steps on the PC, and writes them):
//   op1f picture 1            the first line
//   size N                    the second: the bytes of the picture that is made; the buffer is asked for once
//   from NAME BYTES CRC       the unit's own content/display/NAME is the text to work on; it must be BYTES long and
//                             have that CRC-32, or nothing is made (a unit with another picture gets its own screen)
//   add end BYTES             the BYTES bytes behind this line go in before the picture's last closing tag
// Anything else is refused: a later kit knows more, and a pack that needs it names that kit's mark (below).
//
// The stock picture is opened as stock's own loader opens it: the path is the folder of stock's own format text
// (0x848daf5c, "content/display/%s") and the name, relative, with stock's open, read and close. Nothing is written.
// A failure answers 0 with one line that says why, as for a picture that cannot be read: stock then loads the
// device's own file for a stock screen's name, so the page shows without what the pack adds.
//
// op1f_art_maker_1 is the mark of a kit that makes pictures by these instructions. A pack whose pictures are
// instructions names it outright (module/build_p3_fx_train_needs.s does), and the base leaves a pack out when a name it
// needs is offered by nobody: an older kit, which would hand the instructions to stock's parser, never sees them.
//
// What follows is J5g's text.
//
// Build J packs J5g: the picture loader for packs that carry a picture in more than one form (OS 1.7.0+0).
// This is J5f's loader, which is kept as it is for J5f's kit, with
// one thing added: before the loader looks into a pack's pictures it asks the pack, if the pack offers a function
// for that, which of its pictures the name stands for. The sidechain answers its page for four tracks or its page
// for eight for the signal flow screen's name (build_j5g_duck_alone.cpp). A pack may also answer 0: nothing of
// mine for that name now, and the next pack is asked. A pack that offers no such function is asked by the name
// itself, as in J5f. What follows is J5f's text.
//
// J5f: the picture loader for packs that each carry their own pictures. `kit` carries no picture of
// its own, and the loader asks the feature packs' directories, each offered by its pack under a name this unit
// knows, in an order: the first that has a name serves it. A pack that does not run offers no directory, and its
// place in the list is empty.
//
// J5e: the pictures read from the pack's file.
//
// Until J5e the sixteen pictures the features draw on were bytes inside the pack (module/build_b_art.s and
// build_b_art.cpp, which are kept as they are for the packs before J5e): 1.18 MB of the 2 MB room, read into it at
// every start and never looked at again once each screen had made its document. Since J5e they stand in the pack's
// file behind the lists, a part the base does not load, and this unit reads one when a screen asks for it.
//
// What stock does, read whole for this unit (svg_loader.cpp load(name) 0x8fc8ab7c): the path made, te_fopen
// 0x8fc8a9d8(path, "rb") answering a descriptor, negative on a failure, with an error line of its own; the file's
// size by three calls of the seek 0x8c0a3630(descriptor, offset low, offset high, from) with from 2, 1 and 0 (the
// end, where am I, the start), whose answer is the new place's low word; a buffer of the size and one byte from
// 0x8fcc7560; te_fread 0x8fc8a9a8(buffer, 1, bytes, descriptor) answering the count read; te_fclose 0x8fc8a994; a
// zero byte behind the text; parse_svg_from_ram 0x8c040298; the buffer freed with 0x8fcc6c30; the document
// answered, 0 with an error line on every failure (the caller asserts on 0).
//
// What this unit does for a name a pack carries a picture for: the pack's file opened by the path the base wrote
// into the pack at the start; the file's header read and held against what the running pack is (its name, its
// version, the size of its pictures' part), since the file may have been replaced from the computer while the unit
// ran; the place of the picture gone to, which is behind the header, the body and the lists; the picture read into
// the buffer stock's loader would have taken; its CRC-32 held against the directory's (a picture that is not what
// was built is never handed to the parser, which hangs on what it does not know); then stock's parse and the
// buffer freed. Any failure answers 0 with one line that says why: for a stock screen's name stock then loads the
// device's own file, and for a screen of ours the caller keeps its screen off.
//
// A directory is data of its pack, made by tools/pack_build.py from the recipe's pictures (the files
// module/build_j5f_*_pictures.s): a head with the pack's name and version and 48 bytes for the file's path, then
// one entry a picture. The picture loader's entry is op1f_svg_load in build_j5e_art.s.
//
// Freestanding like the other units. Called on the main thread when a stock screen is made and from the frame
// hook for the screens of ours; both read files with stock's own functions today.

#include "build_m1_card.h"

typedef unsigned int u32;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*AllocFn)(u32 bytes);                       // 0x8fcc7560: svg_loader's buffer, 0 when the heap is out
typedef void (*FreeFn)(void* block);                       // 0x8fcc6c30
typedef void* (*ParseFn)(char* text);                      // 0x8c040298 parse_svg_from_ram: the document, 0 on failure
typedef int (*FileOpenFn)(const char* path, const char* mode);              // 0x8fc8a9d8: a descriptor, negative on a failure
typedef int (*FileReadFn)(void* buffer, int size, int count, int descriptor);   // 0x8fc8a9a8: the count read
typedef int (*FileCloseFn)(int descriptor);                                 // 0x8fc8a994
typedef int (*FileSeekFn)(int descriptor, u32 offset_low, u32 offset_high, int from);   // 0x8c0a3630: the new place's low word; from 0 is the file's start

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_ALLOC ((AllocFn)0x8fcc7560u)
#define STOCK_FREE ((FreeFn)0x8fcc6c30u)
#define STOCK_PARSE_SVG ((ParseFn)0x8c040298u)
#define STOCK_FILE_OPEN ((FileOpenFn)0x8fc8a9d8u)
#define STOCK_FILE_READ ((FileReadFn)0x8fc8a9a8u)
#define STOCK_FILE_CLOSE ((FileCloseFn)0x8fc8a994u)
#define STOCK_FILE_SEEK ((FileSeekFn)0x8c0a3630u)
#define STOCK_READ_MODE ((const char*)0x848daec0u)         // the text "rb", which te_fopen compares its mode with
#define STOCK_PICTURE_PATH_FORMAT ((const char*)0x848daf5cu)   // P1: svg_loader.cpp's own "content/display/%s"; the folder is read from it

static const char LOG_FILE[] = "../src/main/ctls/op1f_tape_art.cpp";

enum {
    ART_MAGIC = 0x31545241,     // "ART1"
    ART_NAME = 24,
    PACK_NAME = 16,
    PATH_CHARS = 48,
    HEADER_BYTES = 96,          // a pack file's header, format 3 (tools/pack_build.py)
    HEADER_WORDS = HEADER_BYTES / 4,
    MAGIC_LOW = 0x4631504f,     // "OP1F"
    MAGIC_HIGH = 0x4b434150,    // "PACK"
    H_FORMAT = 2, H_HEADER = 3, H_BODY = 6, H_NAME = 10, H_LISTS = 16, H_PICTURES = 17, H_VERSION = 18,    // the header's words
};

struct ArtEntry {
    char name[ART_NAME];        // the picture's name as a screen asks for it, zero padded
    u32 offset;                 // its place in the pictures' part of the file
    u32 bytes;
    u32 crc;                    // its CRC-32 (zlib's)
};

struct ArtDir {
    u32 magic;
    u32 count;
    u32 total;                  // the pictures' part of the file, bytes: the header's word of what follows the lists
    u32 version;                // the pack's version
    char pack[PACK_NAME];       // the pack's name, as the file's header has it
    char path[PATH_CHARS];      // the file's path: the base writes it at the start
    ArtEntry entries[1];        // `count` of them
};

// The feature packs' directories, each optional: a pack that does not run offers none, and the base then leaves 0.
extern "C" ArtDir __attribute__((weak)) op1f_art_sidechain asm("op1f_art_sidechain");
extern "C" ArtDir __attribute__((weak)) op1f_art_lfo asm("op1f_art_lfo");
extern "C" ArtDir __attribute__((weak)) op1f_art_fx_train asm("op1f_art_fx_train");
extern "C" ArtDir __attribute__((weak)) op1f_art_8_tracks asm("op1f_art_8_tracks");

// J5g: a pack's own choice among its pictures, optional like its directory: the name to look for in the pack's
// directory when a screen asks for `name`, or 0 for none of this pack's.
typedef const char* (*PickFn)(const char* name);
extern "C" const char* __attribute__((weak)) op1f_art_pick_sidechain(const char* name) asm("op1f_art_pick_sidechain");
extern "C" const char* __attribute__((weak)) op1f_art_pick_lfo(const char* name) asm("op1f_art_pick_lfo");
extern "C" const char* __attribute__((weak)) op1f_art_pick_fx_train(const char* name) asm("op1f_art_pick_fx_train");
extern "C" const char* __attribute__((weak)) op1f_art_pick_8_tracks(const char* name) asm("op1f_art_pick_8_tracks");

extern "C" u32 op1f_svg_served asm("op1f_svg_served");                 // documents parsed from a pack's pictures
u32 op1f_svg_served;
extern "C" u32 op1f_svg_refused asm("op1f_svg_refused");               // pictures a pack carries that could not be served
u32 op1f_svg_refused;
extern "C" u32 op1f_svg_picked asm("op1f_svg_picked");                 // J5g: names a pack answered with another of its pictures
u32 op1f_svg_picked;
extern "C" u32 op1f_svg_made asm("op1f_svg_made");                     // P1: pictures made here from a pack's instructions
u32 op1f_svg_made;
extern "C" const u32 op1f_art_maker_1 asm("op1f_art_maker_1");         // P1: the mark of a kit that makes pictures by instructions of the first kind
const u32 op1f_art_maker_1 = 1;
extern "C" const u32 op1f_art_maker_2 asm("op1f_art_maker_2");         // P2: and of the second kind
const u32 op1f_art_maker_2 = 2;
extern "C" const u32 op1f_art_maker_3 asm("op1f_art_maker_3");         // P3: and of a set of the second kind that takes up to eight stock pictures
const u32 op1f_art_maker_3 = 3;
// P2: a stretch with its geometry mapped (build_p2_map.cpp): the bytes it makes, more than `room` when it did not fit.
extern "C" u32 op1f_map_range(const char* data, u32 bytes, u32 scale, i32 dx, i32 dy, char* out, u32 room, const char** why) asm("op1f_map_range");
// Step M1 (module/build_m3_cards.cpp): the packs that carry a card, looked through by the cards' unit, which
// answers before a file is opened.
//   op1f_cards_picture  the directory and the entry the cards have for a name: 0 for none; 1 for a picture of that
//                       name; 2 for another picture of the pack's, by the pack's own choice. With `only`, a card:
//                       that card's own directory alone, by the name itself.
//   op1f_cards_also     a name one of the four packs below serves, looked for among the cards too: for the one line
//                       that says two packs carry it.
extern "C" int op1f_cards_picture(const char* name, const Op1fCard* only, const ArtDir** dir, const ArtEntry** entry) asm("op1f_cards_picture");
extern "C" void op1f_cards_also(const char* name, const char* pack) asm("op1f_cards_also");

// The packs' directories in the order they are asked, each with the pack's own choice: the first that has the name
// serves it. The signal flow screen's picture with the duck's page (sidechain) comes before the one with the eight
// meters alone (8 tracks).
struct Source { ArtDir* dir; PickFn pick; };
static const Source SOURCES[] = {
    { &op1f_art_sidechain, op1f_art_pick_sidechain },
    { &op1f_art_lfo, op1f_art_pick_lfo },
    { &op1f_art_fx_train, op1f_art_pick_fx_train },
    { &op1f_art_8_tracks, op1f_art_pick_8_tracks },
};

static u32 crc_table[256];
static u32 crc_table_made;

static u32 crc_of(const u8* bytes, u32 count) {
    if (!crc_table_made) {
        for (u32 i = 0; i < 256; ++i) {
            u32 c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
            crc_table[i] = c;
        }
        crc_table_made = 1;
    }
    u32 crc = 0xffffffffu;
    for (u32 i = 0; i < count; ++i) crc = crc_table[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
    return ~crc;
}

static inline u32 cycles() {
#ifdef OP1F_HOST
    return 0;
#else
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
#endif
}

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static void* refused(const char* name, const ArtDir* dir, const char* why, u32 a, u32 b) {
    ++op1f_svg_refused;
    STOCK_LOG(1, LOG_FILE, 2, "artwork %s: not served from %s, %s (%u, %u)", name, dir->path, why, a, b);
    return 0;
}

// ---------------------------------------------------------------------------------------------------------
// P1: a picture made on the unit from a pack's instructions.

enum {
    MARK_BYTES = 15,            // "op1f picture 1" and its line feed
    LARGEST = 1 << 20,          // the largest picture made here
    STOCK_NAME = 32,            // a stock picture's name with its ending, the zero byte counted
    FOLDER_ROOM = 40,           // the folder of stock's format text, at most
    WORDS = 8,                  // the words of an instruction's line that are kept (P1: 4; the second kind's `m` has seven)
    STOCKS = 8,                 // the stock pictures one set of the second kind takes stretches from, at most (P2: 4; P3: 8)
};

static const char MARK[] = "op1f picture 1\n";
static const char MARK2[] = "op1f picture 2\n";            // P2: the second kind

struct Lines { const char* at; const char* end; };
struct Word { const char* text; u32 count; };

// 0 a picture, 1 a set of instructions of the first kind, 2 of the second.
static int is_instructions(const char* text, u32 bytes) {
    if (bytes < MARK_BYTES) return 0;
    for (u32 i = 0; i < MARK_BYTES - 2; ++i) if (text[i] != MARK[i]) return 0;
    if (text[MARK_BYTES - 1] != '\n') return 0;
    return text[MARK_BYTES - 2] == '1' ? 1 : text[MARK_BYTES - 2] == '2' ? 2 : 0;
}

// The next line cut at its single spaces: the count of its words, of which the first WORDS are kept, or -1 when the
// text ends inside the line.
static int next_line(Lines* l, Word* words) {
    int count = 0;
    const char* start = l->at;
    for (const char* p = l->at; p < l->end; ++p) {
        if (*p != ' ' && *p != '\n') continue;
        if (count < WORDS) {
            words[count].text = start;
            words[count].count = (u32)(p - start);
        }
        ++count;
        start = p + 1;
        if (*p == '\n') {
            l->at = p + 1;
            return count;
        }
    }
    return -1;
}

// A word of a line against a word of ours. It is kept a function of its own: put in line at its four places, the
// compiler ran out of pointer registers in make() and counted with an index register (`I0 += M0`), which counts
// straight on only while its length register is zero. That holds wherever compiled code is called, but nothing of
// ours has leaned on it at stock's picture loader before; out of line the question does not come up, and
// a test of the project's reads the listing for it.
static int __attribute__((noinline)) is_word(const Word* w, const char* text) {
    u32 i = 0;
    while (i < w->count && text[i] && w->text[i] == text[i]) ++i;
    return i == w->count && text[i] == 0;
}

// One to nine decimal digits.
static int decimal(const Word* w, u32* out) {
    if (w->count == 0 || w->count > 9) return 0;
    u32 value = 0;
    for (u32 i = 0; i < w->count; ++i) {
        char c = w->text[i];
        if (c < '0' || c > '9') return 0;
        value = value * 10 + (u32)(c - '0');
    }
    *out = value;
    return 1;
}

// Eight hex digits, the letters small.
static int hex8(const Word* w, u32* out) {
    if (w->count != 8) return 0;
    u32 value = 0;
    for (u32 i = 0; i < 8; ++i) {
        char c = w->text[i];
        u32 digit;
        if (c >= '0' && c <= '9') digit = (u32)(c - '0');
        else if (c >= 'a' && c <= 'f') digit = (u32)(c - 'a') + 10;
        else return 0;
        value = (value << 4) | digit;
    }
    *out = value;
    return 1;
}

// A stock picture's name: plain characters, no folder in it.
static int plain_name(const Word* w) {
    if (w->count == 0 || w->count >= STOCK_NAME) return 0;
    for (u32 i = 0; i < w->count; ++i) {
        char c = w->text[i];
        if (c <= 0x20 || c >= 0x7f || c == '/' || c == '\\') return 0;
    }
    return 1;
}

static char* not_made(const char* name, const ArtDir* dir, char* picture, const char* why, u32 a, u32 b) {
    if (picture) STOCK_FREE(picture);
    refused(name, dir, why, a, b);
    return 0;
}

// P2: the two makers are kept functions of their own, so that what they need of the stack (the line's words, the
// path, the stock pictures) is given back before stock's parser runs: the loader then stands on less of the stack
// under the parser than stock's own loader does (0x8fc8ab7c: 288 bytes and its saved registers).
static char* make(const char* name, const ArtDir* dir, const char* text, u32 bytes, u32* made, u32* own) __attribute__((noinline));

// The picture a set of instructions makes, in a buffer of its size and one byte: or 0 with a line that says why.
// `text` is the instructions as read from the pack's file and checked; `own` is given the bytes of them that went
// into the picture.
static char* make(const char* name, const ArtDir* dir, const char* text, u32 bytes, u32* made, u32* own) {
    Lines l = { text + MARK_BYTES, text + bytes };
    Word w[WORDS];
    u32 size = 0;
    int count = next_line(&l, w);
    if (count < 0) return not_made(name, dir, 0, "its instructions end inside a line", 0, 0);
    if (count != 2 || !is_word(&w[0], "size")) return not_made(name, dir, 0, "its instructions do not begin with the picture's size", (u32)count, 0);
    if (!decimal(&w[1], &size)) return not_made(name, dir, 0, "its instructions name a number that is none", 0, 0);
    if (size == 0 || size > LARGEST) return not_made(name, dir, 0, "its instructions name a size the unit does not make", size, LARGEST);
    char* picture = (char*)STOCK_ALLOC(size + 1);
    if (picture == 0) return not_made(name, dir, 0, "no buffer for the picture", size, 0);
    u32 length = 0, taken = 0, added = 0;
    while (l.at < l.end) {
        count = next_line(&l, w);
        if (count < 0) return not_made(name, dir, picture, "its instructions end inside a line", 0, 0);
        if (count == 4 && is_word(&w[0], "from")) {
            u32 stock_bytes = 0, stock_crc = 0;
            if (!plain_name(&w[1]) || !hex8(&w[3], &stock_crc)) return not_made(name, dir, picture, "its instructions name a stock picture badly", w[1].count, w[3].count);
            if (!decimal(&w[2], &stock_bytes)) return not_made(name, dir, picture, "its instructions name a number that is none", 0, 0);
            if (taken) return not_made(name, dir, picture, "its instructions take a stock picture twice", 0, 0);
            if (stock_bytes > size) return not_made(name, dir, picture, "its instructions name a stock picture larger than the result", stock_bytes, size);
            char path[FOLDER_ROOM + STOCK_NAME];
            u32 n = 0;
            for (const char* f = STOCK_PICTURE_PATH_FORMAT; *f && *f != '%' && n < FOLDER_ROOM; ++f) path[n++] = *f;
            for (u32 i = 0; i < w[1].count; ++i) path[n++] = w[1].text[i];
            path[n] = 0;
            int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
            if (descriptor < 0) return not_made(name, dir, picture, "the unit's own picture could not be opened", (u32)descriptor, 0);
            int got = stock_bytes ? STOCK_FILE_READ(picture, 1, (int)stock_bytes, descriptor) : 0;
            char one_more = 0;
            int more = (u32)got == stock_bytes ? STOCK_FILE_READ(&one_more, 1, 1, descriptor) : 0;
            STOCK_FILE_CLOSE(descriptor);
            if ((u32)got != stock_bytes || more != 0) return not_made(name, dir, picture, "the unit's own picture is not as long as the instructions say", (u32)got + (u32)more, stock_bytes);
            u32 crc = crc_of((const u8*)picture, stock_bytes);
            if (crc != stock_crc) return not_made(name, dir, picture, "the unit's own picture is not the one the instructions were made for", crc, stock_crc);
            length = stock_bytes;
            taken = 1;
        } else if (count == 3 && is_word(&w[0], "add") && is_word(&w[1], "end")) {
            u32 block = 0;
            if (!decimal(&w[2], &block)) return not_made(name, dir, picture, "its instructions name a number that is none", 0, 0);
            if (block + 1 > (u32)(l.end - l.at) || l.at[block] != '\n') return not_made(name, dir, picture, "its instructions end inside what they add", block, (u32)(l.end - l.at));
            if (!taken) return not_made(name, dir, picture, "its instructions add to a picture before they take one", 0, 0);
            u32 at = length;                                    // the last closing tag, "</svg"
            for (u32 p = length; p >= 5; --p) {
                const char* q = picture + p - 5;
                if (q[0] == '<' && q[1] == '/' && q[2] == 's' && q[3] == 'v' && q[4] == 'g') {
                    at = p - 5;
                    break;
                }
            }
            if (at == length) return not_made(name, dir, picture, "the picture has no closing tag to add before", length, 0);
            if (length + block > size) return not_made(name, dir, picture, "its instructions make more than the size they name", length + block, size);
            for (u32 k = length; k > at; --k) picture[k - 1 + block] = picture[k - 1];      // what stands behind makes room
            for (u32 k = 0; k < block; ++k) picture[at + k] = l.at[k];
            length += block;
            added += block;
            l.at += block + 1;
        } else {
            return not_made(name, dir, picture, "its instructions hold one this kit does not know", (u32)count, w[0].count);
        }
    }
    if (!taken || length != size) return not_made(name, dir, picture, "its instructions do not make the size they name", length, size);
    *made = length;
    *own = added;
    return picture;
}

// ---------------------------------------------------------------------------------------------------------
// P2: the second kind of instructions.

struct Stock { char* data; u32 bytes; };

// A whole number that may have a minus before it: an offset.
static int signed_decimal(const Word* w, i32* out) {
    Word digits = *w;
    int negative = w->count > 0 && w->text[0] == '-';
    if (negative) {
        ++digits.text;
        --digits.count;
    }
    u32 value = 0;
    if (!decimal(&digits, &value)) return 0;
    *out = negative ? -(i32)value : (i32)value;
    return 1;
}

// A set that is refused: the picture and every stock picture read so far are given back.
static char* not_made2(const char* name, const ArtDir* dir, char* picture, Stock* stocks, u32 count, const char* why, u32 a, u32 b) {
    for (u32 i = 0; i < count; ++i) STOCK_FREE(stocks[i].data);
    return not_made(name, dir, picture, why, a, b);
}

// The stretch a `c` or an `m` line names, in the words 1 to 3: 0, or the words it is refused with.
static const char* stretch(const Word* w, const Stock* stocks, u32 count, u32* number, u32* offset, u32* length) {
    if (!decimal(&w[1], number) || !decimal(&w[2], offset) || !decimal(&w[3], length)) return "its instructions name a number that is none";
    if (*number >= count) return "its instructions name a stock picture they did not take";
    if (*offset > stocks[*number].bytes || *length > stocks[*number].bytes - *offset) return "its instructions name a stretch outside the stock picture";
    return 0;
}

// The picture a set of instructions of the second kind makes, in a buffer of its size and one byte: or 0 with a line
// that says why. `own` is given the bytes of the instructions that went into the picture, `taken` the stock pictures.
static char* __attribute__((noinline)) make2(const char* name, const ArtDir* dir, const char* text, u32 bytes, u32* made, u32* own, u32* taken) {
    Lines l = { text + MARK_BYTES, text + bytes };
    Word w[WORDS];
    Stock stocks[STOCKS];
    u32 size = 0, count_of = 0, held = 0;
    int count = next_line(&l, w);
    if (count < 0) return not_made(name, dir, 0, "its instructions end inside a line", 0, 0);
    if (count != 2 || !is_word(&w[0], "size")) return not_made(name, dir, 0, "its instructions do not begin with the picture's size", (u32)count, 0);
    if (!decimal(&w[1], &size)) return not_made(name, dir, 0, "its instructions name a number that is none", 0, 0);
    if (size == 0 || size > LARGEST) return not_made(name, dir, 0, "its instructions name a size the unit does not make", size, LARGEST);
    char* picture = (char*)STOCK_ALLOC(size + 1);
    if (picture == 0) return not_made(name, dir, 0, "no buffer for the picture", size, 0);
    u32 length = 0, added = 0;
    while (l.at < l.end) {
        count = next_line(&l, w);
        if (count < 0) return not_made2(name, dir, picture, stocks, count_of, "its instructions end inside a line", 0, 0);
        if (count == 4 && is_word(&w[0], "from")) {
            u32 stock_bytes = 0, stock_crc = 0;
            if (!plain_name(&w[1]) || !hex8(&w[3], &stock_crc)) return not_made2(name, dir, picture, stocks, count_of, "its instructions name a stock picture badly", w[1].count, w[3].count);
            if (!decimal(&w[2], &stock_bytes)) return not_made2(name, dir, picture, stocks, count_of, "its instructions name a number that is none", 0, 0);
            if (count_of == STOCKS) return not_made2(name, dir, picture, stocks, count_of, "its instructions take more stock pictures than the unit holds", count_of, 0);
            // What a set may ask of stock's heap is held to a bound, because stock's buffer function does not answer
            // "none": with no room its allocator logs an error and aborts (0x8d4e373c, which 0x8fcc7560
            // reaches through 0x8d4e63bc). So the tests for no buffer are never met on the unit, and a length with a digit
            // too many would stop the start. A picture is a megabyte at most and the stock pictures of a set a megabyte
            // together, where the heap had 4,911 KiB free at its top after a start (2026-11-13).
            if (stock_bytes > LARGEST - held) return not_made2(name, dir, picture, stocks, count_of, "its instructions take stock pictures larger than the unit holds at once", stock_bytes, LARGEST - held);
            char path[FOLDER_ROOM + STOCK_NAME];
            u32 n = 0;
            for (const char* f = STOCK_PICTURE_PATH_FORMAT; *f && *f != '%' && n < FOLDER_ROOM; ++f) path[n++] = *f;
            for (u32 i = 0; i < w[1].count; ++i) path[n++] = w[1].text[i];
            path[n] = 0;
            int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
            if (descriptor < 0) return not_made2(name, dir, picture, stocks, count_of, "the unit's own picture could not be opened", (u32)descriptor, 0);
            char* data = (char*)STOCK_ALLOC(stock_bytes + 1);
            if (data == 0) {
                STOCK_FILE_CLOSE(descriptor);
                return not_made2(name, dir, picture, stocks, count_of, "no buffer for the unit's own picture", stock_bytes, 0);
            }
            stocks[count_of].data = data;                       // from here on it is given back with the others
            stocks[count_of].bytes = stock_bytes;
            ++count_of;
            held += stock_bytes;
            int got = stock_bytes ? STOCK_FILE_READ(data, 1, (int)stock_bytes, descriptor) : 0;
            char one_more = 0;
            int more = (u32)got == stock_bytes ? STOCK_FILE_READ(&one_more, 1, 1, descriptor) : 0;
            STOCK_FILE_CLOSE(descriptor);
            if ((u32)got != stock_bytes || more != 0) return not_made2(name, dir, picture, stocks, count_of, "the unit's own picture is not as long as the instructions say", (u32)got + (u32)more, stock_bytes);
            u32 crc = crc_of((const u8*)data, stock_bytes);
            if (crc != stock_crc) return not_made2(name, dir, picture, stocks, count_of, "the unit's own picture is not the one the instructions were made for", crc, stock_crc);
        } else if (count == 4 && is_word(&w[0], "c")) {
            u32 number = 0, offset = 0, bytes_of = 0;
            const char* why = stretch(w, stocks, count_of, &number, &offset, &bytes_of);
            if (why) return not_made2(name, dir, picture, stocks, count_of, why, offset, bytes_of);
            if (length + bytes_of > size) return not_made2(name, dir, picture, stocks, count_of, "its instructions make more than the size they name", length + bytes_of, size);
            const char* from = stocks[number].data + offset;
            for (u32 k = 0; k < bytes_of; ++k) picture[length + k] = from[k];
            length += bytes_of;
        } else if (count == 7 && is_word(&w[0], "m")) {
            u32 number = 0, offset = 0, bytes_of = 0, scale = 0;
            i32 dx = 0, dy = 0;
            if (!decimal(&w[4], &scale) || !signed_decimal(&w[5], &dx) || !signed_decimal(&w[6], &dy)) return not_made2(name, dir, picture, stocks, count_of, "its instructions name a number that is none", 0, 0);
            const char* why = stretch(w, stocks, count_of, &number, &offset, &bytes_of);
            if (why) return not_made2(name, dir, picture, stocks, count_of, why, offset, bytes_of);
            u32 mapped = op1f_map_range(stocks[number].data + offset, bytes_of, scale, dx, dy, picture + length, size - length, &why);
            if (why) return not_made2(name, dir, picture, stocks, count_of, why, offset, bytes_of);
            if (mapped > size - length) return not_made2(name, dir, picture, stocks, count_of, "its instructions make more than the size they name", length + mapped, size);
            length += mapped;
        } else if (count == 2 && is_word(&w[0], "a")) {
            u32 block = 0;
            if (!decimal(&w[1], &block)) return not_made2(name, dir, picture, stocks, count_of, "its instructions name a number that is none", 0, 0);
            if (block + 1 > (u32)(l.end - l.at) || l.at[block] != '\n') return not_made2(name, dir, picture, stocks, count_of, "its instructions end inside what they add", block, (u32)(l.end - l.at));
            if (length + block > size) return not_made2(name, dir, picture, stocks, count_of, "its instructions make more than the size they name", length + block, size);
            for (u32 k = 0; k < block; ++k) picture[length + k] = l.at[k];
            length += block;
            added += block;
            l.at += block + 1;
        } else {
            return not_made2(name, dir, picture, stocks, count_of, "its instructions hold one this kit does not know", (u32)count, w[0].count);
        }
    }
    if (length != size) return not_made2(name, dir, picture, stocks, count_of, "its instructions do not make the size they name", length, size);
    for (u32 i = 0; i < count_of; ++i) STOCK_FREE(stocks[i].data);
    *made = length;
    *own = added;
    *taken = count_of;
    return picture;
}

// A pack's instructions, read and checked by from_file: the picture made, parsed, and both buffers given back.
// The drawings of other packs' wagons (module/op1f_wagon.h), added to the train's picture for build_h_train.cpp: each
// four times as it is and four times dim (every colour the dim grey, the near-black fills black), as the groups
// fxpcar<i>_<copy> and fxpdim<i>_<copy>, just before the lamp so that they stand where the train's own characters
// stand. Only while op1f_svg_train asks; a picture without the lamp is left as it is.
static const char* const* wagon_art;
static u32 wagon_arts;
static const char WAGON_ANCHOR[] = "<g id=\"fxlamp\"";

static u32 text_length(const char* t) { u32 n = 0; while (t[n]) ++n; return n; }

// A drawing with an arc in a path is left out: the firmware's reader of pictures has no arcs, and one hangs the unit
// at the start (docs/base.md, "What a picture may hold"). Only the path data is looked at: d="..." with an A or a.
static int has_arc(const char* t) {
    for (; *t; ++t) {
        if (t[0] != ' ' || t[1] != 'd' || t[2] != '=' || t[3] != '"') continue;
        for (t += 4; *t && *t != '"'; ++t) if (*t == 'A' || *t == 'a') return 1;
        if (!*t) return 0;
    }
    return 0;
}

static char* put_group(char* p, const char* kind, u32 i, u32 copy, const char* art, int dim) {
    const char* h = "<g id=\"fxp";
    while (*h) *p++ = *h++;
    while (*kind) *p++ = *kind++;
    *p++ = (char)('0' + i); *p++ = '_'; *p++ = (char)('0' + copy); *p++ = '"'; *p++ = '>';
    for (const char* a = art; *a; ++a) {
        if (dim && a[0] == '#' && a[1] && a[2] && a[3] && a[4] && a[5] && a[6]) {   // #rrggbb: near-black stays black, the rest dim grey
            const int dark = a[1] == '0' && (a[2] == '0' || a[2] == 'E' || a[2] == 'e');
            const char* c = dark ? "#000005" : "#5B6670";
            for (int k = 0; k < 7; ++k) *p++ = c[k];
            a += 6;
            continue;
        }
        *p++ = *a;
    }
    *p++ = '<'; *p++ = '/'; *p++ = 'g'; *p++ = '>';
    return p;
}

static char* with_wagons(char* text, u32* length) {
    if (wagon_arts == 0) return text;
    char* at = 0;
    for (char* p = text; *p && !at; ++p) {
        u32 k = 0;
        while (WAGON_ANCHOR[k] && p[k] == WAGON_ANCHOR[k]) ++k;
        if (!WAGON_ANCHOR[k]) at = p;
    }
    if (at == 0) return text;
    u32 extra = 0;
    for (u32 i = 0; i < wagon_arts; ++i) if (wagon_art[i] && !has_arc(wagon_art[i])) extra += 8u * (text_length(wagon_art[i]) + 24u);
    char* out = (char*)STOCK_ALLOC(*length + extra + 1);
    if (out == 0) return text;
    char* p = out;
    for (char* q = text; q < at; ++q) *p++ = *q;
    for (u32 i = 0; i < wagon_arts; ++i) {
        if (!wagon_art[i] || has_arc(wagon_art[i])) continue;
        for (u32 c = 0; c < 4; ++c) p = put_group(p, "car", i, c, wagon_art[i], 0);
        for (u32 c = 0; c < 4; ++c) p = put_group(p, "dim", i, c, wagon_art[i], 1);
    }
    for (char* q = at; q < text + *length; ++q) *p++ = *q;
    STOCK_FREE(text);
    *length = (u32)(p - out);
    return out;
}

static void* made_from(const char* name, const ArtDir* dir, const ArtEntry* e, char* text, u32 started, int kind) {
    u32 length = 0, own = 0, taken = 1;
    char* picture = kind == 2 ? make2(name, dir, text, e->bytes, &length, &own, &taken) : make(name, dir, text, e->bytes, &length, &own);
    STOCK_FREE(text);
    if (picture == 0) return 0;
    u32 made_cycles = cycles() - started;
    picture[length] = 0;
    picture = with_wagons(picture, &length);
    void* document = STOCK_PARSE_SVG(picture);
    STOCK_FREE(picture);
    if (document == 0) return refused(name, dir, "the picture made here did not parse", length, own);
    ++op1f_svg_served;
    ++op1f_svg_made;
    if (kind == 2) STOCK_LOG(3, LOG_FILE, 6, "artwork %s: put together on the unit, %u bytes: %u from %u of the unit's own picture(s) and %u from %s, in %u cycles", name, length, length - own, taken, own, dir->path, made_cycles);
    else STOCK_LOG(3, LOG_FILE, 5, "artwork %s: made on the unit, %u bytes: %u of the unit's own picture and %u from %s, in %u cycles", name, length, length - own, own, dir->path, made_cycles);
    return document;
}

// A picture of a pack, read from the pack's file and parsed: the document, or 0 with a line that says why.
static void* from_file(const char* name, const ArtDir* dir, const ArtEntry* e) {
    u32 started = cycles();
    if (dir->path[0] == 0) return refused(name, dir, "the base wrote no path for the pack's file", 0, 0);
    if (e->offset > dir->total || e->bytes > dir->total - e->offset) return refused(name, dir, "its place lies outside the pictures' part", e->offset, e->bytes);
    int descriptor = STOCK_FILE_OPEN(dir->path, STOCK_READ_MODE);
    if (descriptor < 0) return refused(name, dir, "the file could not be opened", (u32)descriptor, 0);
    u32 head[HEADER_WORDS];
    int got = STOCK_FILE_READ(head, 1, HEADER_BYTES, descriptor);
    const char* named = (const char*)&head[H_NAME];
    int same_pack = got == HEADER_BYTES;
    for (u32 i = 0; same_pack && i < PACK_NAME; ++i) same_pack = named[i] == dir->pack[i];
    if (!same_pack || head[0] != MAGIC_LOW || head[1] != MAGIC_HIGH || head[H_FORMAT] != 3 || head[H_HEADER] != HEADER_BYTES ||
        head[H_VERSION] != dir->version || head[H_PICTURES] != dir->total) {
        STOCK_FILE_CLOSE(descriptor);
        return refused(name, dir, "the file is not the pack that runs (bytes read, its version)", (u32)got, got == HEADER_BYTES ? head[H_VERSION] : 0);
    }
    u32 place = HEADER_BYTES + head[H_BODY] + head[H_LISTS] + e->offset;
    int at = STOCK_FILE_SEEK(descriptor, place, 0, 0);
    if ((u32)at != place) {
        STOCK_FILE_CLOSE(descriptor);
        return refused(name, dir, "its place in the file could not be gone to", place, (u32)at);
    }
    char* buffer = (char*)STOCK_ALLOC(e->bytes + 1);
    if (buffer == 0) {
        STOCK_FILE_CLOSE(descriptor);
        return refused(name, dir, "no buffer for it", e->bytes, 0);
    }
    got = STOCK_FILE_READ(buffer, 1, (int)e->bytes, descriptor);
    STOCK_FILE_CLOSE(descriptor);
    if ((u32)got != e->bytes) {
        STOCK_FREE(buffer);
        return refused(name, dir, "it could not be read whole", (u32)got, e->bytes);
    }
    u32 crc = crc_of((const u8*)buffer, e->bytes);
    if (crc != e->crc) {
        STOCK_FREE(buffer);
        return refused(name, dir, "its checksum is not the one it was built with", crc, e->crc);
    }
    int kind = is_instructions(buffer, e->bytes);
    if (kind) return made_from(name, dir, e, buffer, started, kind);                           // P1, P2: the picture is made here
    u32 read_cycles = cycles() - started;
    buffer[e->bytes] = 0;
    u32 length = e->bytes;
    buffer = with_wagons(buffer, &length);
    void* document = STOCK_PARSE_SVG(buffer);
    STOCK_FREE(buffer);
    if (document == 0) return refused(name, dir, "it did not parse", e->bytes, 0);
    ++op1f_svg_served;
    STOCK_LOG(3, LOG_FILE, 3, "artwork %s: %u bytes served from %s at %u, read and checked in %u cycles", name, e->bytes, dir->path, place, read_cycles);
    return document;
}

// Called by op1f_svg_load with the name the loader was given, and by the screens of ours. A document, or 0: stock
// then loads the device's file for a name of its own.
extern "C" void* op1f_svg_embedded(const char* name) asm("op1f_svg_embedded");
extern "C" void* op1f_svg_embedded(const char* name) {
    if (name == 0) return 0;
    for (u32 s = 0; s < sizeof(SOURCES) / sizeof(SOURCES[0]); ++s) {
        const ArtDir* dir = SOURCES[s].dir;
        if (dir == 0 || dir->magic != ART_MAGIC) continue;
        const char* wanted = name;
        if (SOURCES[s].pick != 0) {                         // J5g: the pack says which of its pictures the name stands for
            wanted = SOURCES[s].pick(name);
            if (wanted == 0) continue;
        }
        for (u32 i = 0; i < dir->count; ++i) {
            if (!same(wanted, dir->entries[i].name)) continue;
            if (wanted != name) {
                ++op1f_svg_picked;
                STOCK_LOG(3, LOG_FILE, 4, "artwork %s: the pack %s serves it as its picture %s", name, dir->pack, wanted);
            }
            op1f_cards_also(name, dir->pack);               // Step M1: a card that carries the name too is said, once; this pack serves it as before
            return from_file(name, dir, &dir->entries[i]);
        }
    }
    const ArtDir* dir = 0;                                  // Step M1: none of the four has it: the cards, in the order of the packs' file names
    const ArtEntry* entry = 0;
    int found = op1f_cards_picture(name, 0, &dir, &entry);
    if (found == 2) {                                       // by the pack's own choice: said as for one of the four
        ++op1f_svg_picked;
        STOCK_LOG(3, LOG_FILE, 4, "artwork %s: the pack %s serves it as its picture %s", name, dir->pack, entry->name);
    }
    if (found) return from_file(name, dir, entry);
    return 0;
}

// The train's picture with the wagons of other packs drawn in (up to ten; 0 for one without a drawing). Display thread.
extern "C" void* op1f_svg_train(const char* const* arts, u32 count) asm("op1f_svg_train");
extern "C" void* op1f_svg_train(const char* const* arts, u32 count) {
    wagon_art = arts;
    wagon_arts = count > 10 ? 10 : count;
    void* document = op1f_svg_embedded("fxtrain.svg");
    wagon_arts = 0;
    return document;
}

// Step M1: a pack's own picture, for a page of its own. It is looked for in the directory of that card's pack alone
// and by the name itself, whatever another pack carries under the name; the card is one the base has in use. A
// document, or 0: with a line where the pack has the picture and it could not be served.
extern "C" void op1f_cards_made(const Op1fCard* card, void* document) asm("op1f_cards_made");   // M3: the cards unit keeps it

extern "C" void* op1f_picture(const Op1fCard* card, const char* name) {
    const ArtDir* dir = 0;
    const ArtEntry* entry = 0;
    if (card == 0 || name == 0 || !op1f_cards_picture(name, card, &dir, &entry)) return 0;
    void* document = from_file(name, dir, entry);
    if (document != 0) op1f_cards_made(card, document);    // M3
    return document;
}

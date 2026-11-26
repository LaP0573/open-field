// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Base 33 (R9), the base pack: base 30's text unit, edited in place (base 30's text, base 32's too, is open-field
// 0.14's), with one call added for a page that holds its picture: where a part is made a text and where a text
// is laid out, the cards unit is told that the document changed (op1f_cards_changed), so that a page under 77 that
// holds its picture is drawn whole again. What base 30 said follows and holds.
//
// Base 30 (R6d), the base pack: the text unit of base 19 (which stays as it was for every
// base since) with one entry more, for the base itself and offered to no pack:
//
//   op1f_text_base_set  op1f_text_base with an alignment, OP1F_LEFT, OP1F_CENTRE or OP1F_RIGHT, handed to the
//                  firmware's label maker as its third argument when the part's label is made, as op1f_text_set
//                  hands a pack's. The label keeps it for good: a later call with another one sets the text, keeps the
//                  alignment and says so (the first sixteen), as for a pack. Any other value answers 0 before anything.
//                  Its texts count under the base's own slot, as op1f_text_base's. Its first user is the base's own
//                  page for an engine whose pack is away (the cards unit), whose words stand centred on the page.
//
// op1f_text_base is as it was (its labels from the left, for the door's words). What R1's text said follows and holds.
//
// Step R1, the base pack (base.pack 19): text on a pack's page at level 4. A sibling of base 18's text unit
// (base.pack 18), which stays as it was; that unit is M3's (M3's text unit) with op1f_text_base added for a
// document of the base's own. What R1 changes:
//
//   the room       512 texts for all packs together, at most 256 for one pack (base 18: sixty-four for all). A text
//                  counts for the pack whose card made its document; the base's own texts (op1f_text_base) count
//                  under a slot of their own, behind the 128 cards' slots (R20), in the same 512;
//   op1f_text_set  op1f_text with an alignment, OP1F_LEFT, OP1F_CENTRE or OP1F_RIGHT, handed to the firmware's label
//                  maker as its third argument when the part's label is made. The label keeps it for good, so a
//                  part's alignment is its first use's: a later call with another one sets the text, keeps the
//                  alignment and says so (the first sixteen). Any other value answers 0. op1f_text is op1f_text_set
//                  with OP1F_LEFT;
//   op1f_text_width  the laid-out text's width in pixels, from the label's box as the firmware's own function answers
//                  it: a structure of four words returned by value, so the compiler hands the place for the answer as
//                  that function expects it (P0 on the unit). 0 for an empty text, -1 when no text of that part was
//                  made;
//   op1f_text_node the label the part became, a part like any other for op1f_move and op1f_show; 0 when none.
//
// What holds from M3: op1f_text(document, part, text, width) writes a text into a part of a document a pack made
// through op1f_picture, in the firmware's own letters: the part, a placeholder group of the pack's picture, becomes a
// label object of the firmware's at its first use, and the label's text is set when it changes.
//
// The firmware's calls, each as our code meets it:
//   the lookup 0x8fc25cc0(screen, id, 1) and the label maker 0x8fc25abc(screen, node, align) read the screen at +4
//     and nowhere else, so a document no screen holds is handed in four words of which the second is the document,
//     as op1f_node does since M1. The label maker stops the unit for a part whose paint is 0 or lies past the
//     document's table of paints ([document + 0x84] to [document + 0x88], twenty bytes an entry), so the paint is
//     asked here first, through the same function of the node's table, and a part whose paint would stop the unit is
//     not handed on: the answer is 0 and one line says so. Its third argument: 0 left, 1 centred, 2 right;
//   the text 0x8fc3aa3c(label, text, width) lays the glyphs out from the firmware's font, shortened with a mark while
//     wider than `width` pixels. It allocates, so it is called only when the text changed. The font's capitals stand
//     under the small letters' codes: a text is handed on in small letters. The font's lookup takes a character as a
//     signed byte and stops the unit for one past its table, which a byte from 0x80 up is: every character outside
//     the plain ones is handed on as a question mark;
//   the box 0x8fc3820c(label) answers the laid-out glyphs' box in 1024ths of a pixel (x min, x max, y min, y max),
//     by value. It is the glyphs' geometry without their stroke: a text of one upright stroke measures 0.
//
// From a frame function only (the cards unit says when it is calling one, op1f_cards_calling): a label is made and
// laid out on the display thread, where the document is drawn. A document is one a card made (op1f_cards_document).
// Lines: the first sixteen texts made, the first sixteen refusals, the first sixteen alignments kept. Nothing is
// divided (the module links no runtime). Compiled for the PC as it is, with OP1F_HOST defined: the stand-ins come in
// through the PC's own header of the firmware's calls (for tests this tree does not hold).

#include "build_r1_card.h"

typedef unsigned int u32;
typedef int i32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void* (*FindNodeFn)(void* screen, const char* id, int optional);    // 0x8fc25cc0: 0 when optional and missing
typedef void* (*LabelFromNodeFn)(void* screen, void* node, int align);      // 0x8fc25abc: the node replaced by a label object in the document, the label returned
typedef void (*LabelTextFn)(void* label, const char* text, int width);      // 0x8fc3aa3c: the text laid out, shortened with a mark while wider than `width` pixels
typedef u32 (*PaintFn)(void* node);                                         // the node's table +0x14: its paint, an index into the document's paints from 1
struct LabelBox { i32 x_min, x_max, y_min, y_max; };                       // the laid-out glyphs' box, in 1024ths of a pixel
typedef LabelBox (*LabelBoundsFn)(void* label);                            // 0x8fc3820c: the box returned by value, the place for it handed as the function expects

#ifdef OP1F_HOST
#include "mock_files_r1.h"                             // the PC's stand-ins: the logger, a model of the documents, their parts and the labels
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FIND_NODE ((FindNodeFn)0x8fc25cc0u)
#define STOCK_LABEL_FROM_NODE ((LabelFromNodeFn)0x8fc25abcu)
#define STOCK_LABEL_TEXT ((LabelTextFn)0x8fc3aa3cu)
#define STOCK_LABEL_BOUNDS ((LabelBoundsFn)0x8fc3820cu)
#define RD32(a) (*(volatile u32*)(a))
#endif

// R20 (base 42): the cards unit's second list and two of its names (module/build_r3_base.h, which this unit's card
// header does not take): the same declarations.
#ifndef OP1F_BUILD_R3_BASE_H
enum { OP1F_SLOTS_MOST = 128 };
struct Op1fCards2 { unsigned count, more; Op1fCardSlot slots[OP1F_SLOTS_MOST]; };
extern "C" Op1fCards2 op1f_cards_2 OP1F_NAMED("op1f_cards_2");
extern "C" void op1f_cards_lists() OP1F_NAMED("op1f_cards_lists");
extern "C" int op1f_cards_in_use(unsigned slot) OP1F_NAMED("op1f_cards_in_use");
#endif

enum { LABELS = 512, PACK_LABELS = 256, BASE_SLOT = OP1F_SLOTS_MOST, SLOTS = OP1F_SLOTS_MOST + 1, PART_KEPT = 24, TEXT_KEPT = 40, SCREEN_WIDTH = 480,
       LINES_SAID = 16, ALIGNMENTS = 3, Q10 = 10 };
enum { NODE_PAINT = 5, DOCUMENT_PAINTS = 0x84 / 4, DOCUMENT_PAINTS_END = 0x88 / 4, PAINT_BYTES = 20 };   // the node's table's word +0x14; the document's paints

static const char LOG_FILE[] = "../src/main/op1f_text.cpp";
static const char* const ALIGNED[ALIGNMENTS] = { "left", "centre", "right" };

extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
extern "C" int op1f_cards_document(const void* document) OP1F_NAMED("op1f_cards_document");

// R9 (base 33): the cards unit's mark of a change to a document (module/build_r6d_cards.cpp): a part made a text and a
// text laid out change what the document draws, so a page that holds its picture is drawn whole again. On the PC the
// unit is built beside an elder cards unit, which has no such word: a stand-in passes it by.
#ifdef OP1F_HOST
static void op1f_cards_changed(const void*) {}
#else
extern "C" void op1f_cards_changed(const void* document) OP1F_NAMED("op1f_cards_changed");
#endif

struct Text {
    void* document;
    char part[PART_KEPT];                              // the part's name, whole: a name of more than 23 characters takes no text
    void* label;
    char text[TEXT_KEPT];                              // the text as last laid out, in small letters
    u32 laid;                                          // the text was laid out at least once
    u8 slot;                                           // R1: the slot of the card whose document it is; BASE_SLOT for the base's own (R20: 0 to 128, a byte holds it)
    u8 align;                                          // R1: the alignment the label was made with, for good
};

static Text texts[LABELS];
static u32 text_count, lines_made, lines_refused, lines_kept;
static u32 held_by[SLOTS];                             // R1: the texts each slot holds

static int same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// A line, the first sixteen texts made and the first sixteen refusals, each counted apart.
static void said(int level, int line, const char* format, const char* pack, const char* part, const char* why) {
    u32* lines = level == 3 ? &lines_made : &lines_refused;
    if (*lines >= (u32)LINES_SAID) return;
    ++*lines;
    STOCK_LOG(level, LOG_FILE, line, format, pack, part, why);
}

// The text a document's part became, or 0: nothing is made.
static Text* found(void* document, const char* part) {
    for (u32 i = 0; i < text_count; ++i) {
        if (texts[i].document == document && same(part, texts[i].part)) return &texts[i];
    }
    return 0;
}

static const char* const MADE_ALIGNED[ALIGNMENTS] = { ", aligned left", ", aligned centre", ", aligned right" };

// The text of a document's part made, once: the part looked up, its paint held, the label made with its alignment.
// 0 when it cannot be. A part made before keeps its alignment; asked another, it says so.
static Text* made(void* document, const char* part, const char* pack, u32 slot, u32 align) {
    u32 n = 0;
    while (part[n]) {
        if (++n == (u32)PART_KEPT) {
            said(2, 1, "text: '%s' has no part %s that can take a text: %s", pack, "?", "its name is longer than 23 characters");
            return 0;
        }
    }
    Text* known = found(document, part);
    if (known) {
        if (known->align != align && lines_kept < (u32)LINES_SAID) {
            ++lines_kept;
            STOCK_LOG(2, LOG_FILE, 4, "text: '%s' asked the part %s aligned %s: it keeps %s, as its text was first made", pack, part, ALIGNED[align], ALIGNED[known->align]);
        }
        return known;
    }
    if (held_by[slot] >= (u32)PACK_LABELS) {
        said(2, 2, "text: '%s' asked for the part %s: %s", pack, part, "its 256 texts are taken");
        return 0;
    }
    if (text_count >= (u32)LABELS) {
        said(2, 2, "text: '%s' asked for the part %s: %s", pack, part, "the base's 512 texts are taken");
        return 0;
    }
    u32 held[4] = { 0, (u32)document, 0, 0 };                                     // no screen holds it: the lookup and the label maker read the word at +4 and nothing else
    void* node = STOCK_FIND_NODE(held, part, 1);
    if (node == 0) {
        said(2, 1, "text: '%s' has no part %s that can take a text: %s", pack, part, "its picture has no part of that name");
        return 0;
    }
    u32 paint = ((PaintFn)((*(u32**)node)[NODE_PAINT]))(node);                    // as the label maker asks it, before it stops the unit for a 0
    const u32* doc = (const u32*)document;
    u32 paints = doc[DOCUMENT_PAINTS_END] - doc[DOCUMENT_PAINTS];                 // the bytes of the document's paints
    if (paint == 0 || (paint - 1u) * (u32)PAINT_BYTES >= paints) {
        said(2, 1, "text: '%s' has no part %s that can take a text: %s", pack, part, "it is no group with a stroke");
        return 0;
    }
    void* label = STOCK_LABEL_FROM_NODE(held, node, (int)align);                  // at the part's left, its middle or its right, for good
    op1f_cards_changed(document);                                                 // R9 (base 33): the part replaced in the document, whatever the maker answers
    if (label == 0) return 0;
    Text* t = &texts[text_count++];
    ++held_by[slot];
    t->document = document;
    for (u32 i = 0; i <= n; ++i) t->part[i] = part[i];
    t->label = label;
    t->text[0] = 0;
    t->laid = 0;
    t->slot = (u8)slot;
    t->align = (u8)align;
    said(3, 3, "text: '%s' made a text of the part %s of its picture%s", pack, part, MADE_ALIGNED[align]);
    return t;
}

// The text laid out in small letters, when it changed.
static int lay(Text* t, const char* text, int width) {
    char small[TEXT_KEPT];
    u32 n = 0;
    for (; n < (u32)TEXT_KEPT - 1 && text[n]; ++n) {
        u8 c = (u8)text[n];
        if (c < 0x20 || c > 0x7e) c = '?';                                        // the font's lookup reads a character as signed: a byte from 0x80 up lands past its table, and the unit stops
        small[n] = c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : (char)c;          // the font's capitals stand under the small letters
    }
    small[n] = 0;
    if (t->laid && same(small, t->text)) return 1;                                // the same text: nothing laid out again
    STOCK_LABEL_TEXT(t->label, small, width > 0 ? width : (int)SCREEN_WIDTH);
    op1f_cards_changed(t->document);                                              // R9 (base 33): the document draws another text
    for (u32 i = 0; i <= n; ++i) t->text[i] = small[i];
    t->laid = 1;
    return 1;
}

extern "C" int op1f_text_set(void* document, const char* part, const char* text, int width, unsigned align) {
    if (op1f_cards_calling() == 0 || document == 0 || part == 0 || text == 0) return 0;   // from a frame function only
    if (align >= (unsigned)ALIGNMENTS) return 0;
    int slot = op1f_cards_document(document);
    if (slot < 0) return 0;                                                       // a document a card made, and no other
    Text* t = made(document, part, op1f_cards_2.slots[slot].card->name, (u32)slot, align);
    if (t == 0) return 0;
    return lay(t, text, width);
}

extern "C" int op1f_text(void* document, const char* part, const char* text, int width) {
    return op1f_text_set(document, part, text, width, OP1F_LEFT);
}

// The text a pack's document's part became, asked from a frame function: 0 when there is none.
static Text* asked(void* document, const char* part) {
    if (document == 0 || part == 0 || op1f_cards_calling() == 0) return 0;       // from a frame function only
    if (op1f_cards_document(document) < 0) return 0;
    return found(document, part);
}

extern "C" int op1f_text_width(void* document, const char* part) {
    Text* t = asked(document, part);
    if (t == 0) return -1;
    if (t->text[0] == 0) return 0;                                                // no glyph laid out: the box is not asked
    LabelBox box = STOCK_LABEL_BOUNDS(t->label);                                  // by value
    i32 width = (box.x_max - box.x_min) >> Q10;
    return width > 0 ? (int)width : 0;
}

extern "C" void* op1f_text_node(void* document, const char* part) {
    Text* t = asked(document, part);
    return t ? t->label : 0;
}

// S1: a text into a part of a document of the base's own (not one a card made), from the base's own frame work on
// the display thread. The rest is op1f_text's; R1: its texts count under the base's own slot.
extern "C" int op1f_text_base(void* document, const char* part, const char* text, int width) OP1F_NAMED("op1f_text_base");
extern "C" int op1f_text_base(void* document, const char* part, const char* text, int width) {
    if (document == 0 || part == 0 || text == 0) return 0;
    Text* t = made(document, part, "base", (u32)BASE_SLOT, (u32)OP1F_LEFT);
    if (t == 0) return 0;
    return lay(t, text, width);
}

// R6d: a text into a part of a document of the base's own, with an alignment (OP1F_LEFT, OP1F_CENTRE, OP1F_RIGHT), from
// the base's own frame work on the display thread. The rest is op1f_text_base's: its texts count under the base's own
// slot. Not offered to packs.
extern "C" int op1f_text_base_set(void* document, const char* part, const char* text, int width, unsigned align) OP1F_NAMED("op1f_text_base_set");
extern "C" int op1f_text_base_set(void* document, const char* part, const char* text, int width, unsigned align) {
    if (document == 0 || part == 0 || text == 0) return 0;
    if (align >= (unsigned)ALIGNMENTS) return 0;
    Text* t = made(document, part, "base", (u32)BASE_SLOT, (u32)align);
    if (t == 0) return 0;
    return lay(t, text, width);
}

// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Base 33 (R9): one helper more for packs, op1f_paint_copy, which copies a block of this frame's picture out in the
// layout op1f_paint_block takes back, under the helpers' guards, so that a page that holds its picture (module/
// build_r6e_card.h, "Base 33") can put back what lay under something it moves. The other helpers are as they were.
//
// Base 29, its second build (R6b): the paint unit of R1 (which stays as it was for every
// base before), with one function added for the base itself: op1f_paint_text_base, the text in the base's own font
// for a page of the base's own, painted by the cards unit from its frame work while no pack's frame function runs (the
// page that says an engine's pack is not on the unit: base 29's cards unit). It is no name packs are offered.
// Every helper of R1's is here as it was, and its guards hold for every pack.
//
// Step R1, the base pack: painting on the frame's picture, and a font of the base's own (OS 1.7.0+0). A unit of its
// own, new at R1.
//
// The five helpers paint into the frame's canvas, the picture that has just been drawn and that the display is about
// to show: op1f_paint_box, op1f_paint_line, op1f_paint_block, op1f_paint_mask and op1f_paint_text. The canvas is the
// frame's buffer:
// 480 columns of 256 pixels of sixteen bits (RGB565, red in the high bits), the column of x = 479 first, so that the
// pixel at (x, y) is the half-word at canvas + 2 x ((479 - x) x 256 + y), y from 0 to 223 shown. A column's pixels lie
// next to each other, so a box is painted column by column. Everything is cut to 480 by 224.
//
// The guards. A helper paints only while the base is calling a pack's frame function on the display thread
// (op1f_cards_calling, the slot + 1, not 0) and only into this frame's canvas (op1f_cards_canvas, not 0 while a
// frame function runs, the canvas the frame was handed). A call that fails either paints nothing and is counted as
// refused, by the pack whose frame function runs (or apart, when none runs on this thread); a call that paints is
// counted as painted. The cards unit says the counts at the period (op1f_paint_take): no helper here says a line,
// waits or calls anything of the firmware's. A canvas of 0, which a frame hands when the base offers none, paints
// nothing and is not counted.
//
// The font is the base's own: 5 by 7 pixels a glyph, one column of space after it, 95 glyphs from 0x20 to 0x7e,
// drawn for R1 from the grid alone, each as seven rows of '#' and '.' in the comment above its bytes (a row's byte
// holds the row in its low five bits, bit 4 the left pixel). A test of the PC's holds the bytes to the
// drawings, and holds that no glyph is one of the unit's own.
//
// No division: nothing here divides but by a power of two, written as a shift. Compiled for the PC as it is, with
// OP1F_HOST defined: the stand-ins come in through a header of the PC's own (for tests this tree does not hold).

#include "build_r1_card.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#ifdef OP1F_HOST
#include "mock_paint_r1.h"                             // the PC's stand-ins: the two words of the cards unit, a count of the pixels stored
#else
#define PUT(at, value) (*(at) = (value))
#endif

enum { WIDE = 480, TALL = 224, COLUMN = 256, LAST_X = WIDE - 1 };
enum { PEN_MOST = 16, SCALE_MOST = 8, TEXT_MOST = 128, BLOCK_MOST = 480, REACH = 32767 };
enum { GLYPH_WIDE = 5, GLYPH_TALL = 7, ADVANCE = 6, FIRST_GLYPH = 0x20, LAST_GLYPH = 0x7e, GLYPHS = LAST_GLYPH - FIRST_GLYPH + 1 };
enum { SLOTS = OP1F_CARDS_MOST, NONE = OP1F_CARDS_MOST };   // the counts of each slot, and one for a call from no frame function

// The cards unit's (base 19's cards unit): the slot + 1 whose frame function the base is calling on this thread, 0
// otherwise; and this frame's canvas while a frame function runs, 0 otherwise.
extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
extern "C" void* op1f_cards_canvas() OP1F_NAMED("op1f_cards_canvas");

static u32 painted[SLOTS + 1], refused[SLOTS + 1];

// The counts of a slot (0 to 31; 32: calls from no frame function) since the last time they were taken, and taken:
// for the cards unit's line at the period, from the display thread. A count missed by a refusal on another thread
// at that instant is lost; nothing else is.
extern "C" void op1f_paint_take(u32 slot, u32* painted_now, u32* refused_now) OP1F_NAMED("op1f_paint_take");
extern "C" void op1f_paint_take(u32 slot, u32* painted_now, u32* refused_now) {
    if (slot > (u32)NONE) slot = NONE;
    if (painted_now) *painted_now = painted[slot];
    if (refused_now) *refused_now = refused[slot];
    painted[slot] = 0;
    refused[slot] = 0;
}

// The canvas a helper may paint into, or 0: this frame's, from a pack's frame function. Each call counted.
static u16* may_paint(void* canvas) {
    if (canvas == 0) return 0;
    u32 calling = op1f_cards_calling();
    void* now = op1f_cards_canvas();
    if (calling == 0 || calling > (u32)SLOTS || now == 0 || canvas != now) {
        ++refused[calling == 0 || calling > (u32)SLOTS ? (u32)NONE : calling - 1u];
        return 0;
    }
    ++painted[calling - 1u];
    return (u16*)canvas;
}

// A box already held to the canvas: x, y, w, h inside 480 by 224, w and h more than 0.
static void fill(u16* canvas, int x, int y, int w, int h, u16 colour) {
    for (int cx = x; cx < x + w; ++cx) {
        u16* at = canvas + (((u32)(LAST_X - cx)) << 8) + (u32)y;
        for (int j = 0; j < h; ++j) PUT(at + j, colour);
    }
}

// A box cut to the canvas; 0 when nothing of it is left. Written so that no sum can pass an int's room.
static int cut(int* x, int* y, int* w, int* h) {
    if (*w <= 0 || *h <= 0) return 0;
    if (*x < 0) { *w += *x; *x = 0; if (*w <= 0) return 0; }
    if (*y < 0) { *h += *y; *y = 0; if (*h <= 0) return 0; }
    if (*x >= (int)WIDE || *y >= (int)TALL) return 0;
    if (*w > (int)WIDE - *x) *w = (int)WIDE - *x;
    if (*h > (int)TALL - *y) *h = (int)TALL - *y;
    return 1;
}

static void box(u16* canvas, int x, int y, int w, int h, u16 colour) {
    if (cut(&x, &y, &w, &h)) fill(canvas, x, y, w, h, colour);
}

extern "C" void op1f_paint_box(void* canvas, int x, int y, int w, int h, unsigned colour) {
    u16* to = may_paint(canvas);
    if (to) box(to, x, y, w, h, (u16)colour);
}

static int far(int v) {
    return v < -(int)REACH - 1 || v > (int)REACH;
}

// Bresenham's line, the pen a square of `width` whose top-left stands (width - 1) / 2 above and left of the point.
extern "C" void op1f_paint_line(void* canvas, int x0, int y0, int x1, int y1, int width, unsigned colour) {
    if (width < 1 || width > (int)PEN_MOST || far(x0) || far(y0) || far(x1) || far(y1)) return;
    u16* to = may_paint(canvas);
    if (to == 0) return;
    int back = (width - 1) >> 1;
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, sx = x0 < x1 ? 1 : -1;
    int dy = y1 > y0 ? y0 - y1 : y1 - y0, sy = y0 < y1 ? 1 : -1;   // dy is minus the rise
    int error = dx + dy;
    for (;;) {
        box(to, x0 - back, y0 - back, width, width, (u16)colour);
        if (x0 == x1 && y0 == y1) break;
        int twice = error << 1;
        if (twice >= dy) { error += dy; x0 += sx; }
        if (twice <= dx) { error += dx; y0 += sy; }
    }
}

extern "C" void op1f_paint_block(void* canvas, int x, int y, int w, int h, const unsigned short* pixels, int stride, int key) {
    if (w <= 0 || h <= 0 || w > (int)BLOCK_MOST || h > (int)BLOCK_MOST || pixels == 0 || key < -1 || key > 0xffff) return;
    u16* to = may_paint(canvas);
    if (to == 0) return;
    if (x <= -(int)BLOCK_MOST || y <= -(int)BLOCK_MOST) return;  // nothing of it on the canvas
    int left = x < 0 ? -x : 0, top = y < 0 ? -y : 0;           // the block's first column and row that lie on the canvas
    int cx = x + left, cy = y + top, cw = w - left, ch = h - top;
    if (!cut(&cx, &cy, &cw, &ch)) return;
    for (int c = 0; c < cw; ++c) {
        u16* at = to + (((u32)(LAST_X - (cx + c))) << 8) + (u32)cy;
        const u16* from = pixels + (left + c) + top * stride;
        for (int r = 0; r < ch; ++r) {
            u16 p = from[r * stride];
            if ((int)p != key) PUT(at + r, p);
        }
    }
}

// R9 (base 33): a block of this frame's picture copied out, the way op1f_paint_block puts one in: `w` by `h` pixels from
// (x, y), row by row from the top, `stride` pixels a row, cut as op1f_paint_block's; the block's pixels that lie outside
// the picture are not written. The guards are the helpers' (may_paint): a refused call copies nothing and is counted.
// 1 when anything was copied.
extern "C" int op1f_paint_copy(void* canvas, int x, int y, int w, int h, unsigned short* pixels, int stride) OP1F_NAMED("op1f_paint_copy");
extern "C" int op1f_paint_copy(void* canvas, int x, int y, int w, int h, unsigned short* pixels, int stride) {
    if (w <= 0 || h <= 0 || w > (int)BLOCK_MOST || h > (int)BLOCK_MOST || pixels == 0) return 0;
    const u16* from = may_paint(canvas);
    if (from == 0) return 0;
    if (x <= -(int)BLOCK_MOST || y <= -(int)BLOCK_MOST) return 0;  // nothing of it on the canvas
    int left = x < 0 ? -x : 0, top = y < 0 ? -y : 0;           // the block's first column and row that lie on the canvas
    int cx = x + left, cy = y + top, cw = w - left, ch = h - top;
    if (!cut(&cx, &cy, &cw, &ch)) return 0;
    for (int c = 0; c < cw; ++c) {
        const u16* at = from + (((u32)(LAST_X - (cx + c))) << 8) + (u32)cy;
        u16* to = pixels + (left + c) + top * stride;
        for (int r = 0; r < ch; ++r) to[r * stride] = at[r];
    }
    return 1;
}

// A colour laid over what is there, by a block of cover: each byte says how much of the colour lies over its pixel, 0
// none and 255 all. Each of the pixel's three parts is mixed by itself, with a multiply and a shift and no division:
// the part + (((the colour's part - the part) x weight) >> 8), the weight being the cover and its top bit
// (cover + (cover >> 7), 0 to 256), so that 0 leaves the pixel as it is and 255 gives the colour exactly. A pixel whose
// cover is 0 is not stored. The block is cut as op1f_paint_block's.
extern "C" void op1f_paint_mask(void* canvas, int x, int y, int w, int h, const unsigned char* cover, int stride, unsigned colour) {
    if (w <= 0 || h <= 0 || w > (int)BLOCK_MOST || h > (int)BLOCK_MOST || cover == 0) return;
    u16* to = may_paint(canvas);
    if (to == 0) return;
    if (x <= -(int)BLOCK_MOST || y <= -(int)BLOCK_MOST) return;  // nothing of it on the canvas
    int left = x < 0 ? -x : 0, top = y < 0 ? -y : 0;           // the block's first column and row that lie on the canvas
    int cx = x + left, cy = y + top, cw = w - left, ch = h - top;
    if (!cut(&cx, &cy, &cw, &ch)) return;
    int red = (int)((colour >> 11) & 0x1fu), green = (int)((colour >> 5) & 0x3fu), blue = (int)(colour & 0x1fu);
    for (int c = 0; c < cw; ++c) {
        u16* at = to + (((u32)(LAST_X - (cx + c))) << 8) + (u32)cy;
        const u8* from = cover + (left + c) + top * stride;
        for (int r = 0; r < ch; ++r) {
            int weight = from[r * stride];
            if (weight == 0) continue;
            weight += weight >> 7;
            int p = at[r];
            int pr = p >> 11, pg = (p >> 5) & 0x3f, pb = p & 0x1f;
            pr += ((red - pr) * weight) >> 8;
            pg += ((green - pg) * weight) >> 8;
            pb += ((blue - pb) * weight) >> 8;
            PUT(at + r, (u16)((pr << 11) | (pg << 5) | pb));
        }
    }
}

// The font: a glyph a character from 0x20 to 0x7e, seven rows of five pixels, bit 4 the left one.
static const u8 FONT[GLYPHS][GLYPH_TALL] = {
    // 0x20 space
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // 0x21 '!'
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   .....
    //   ..#..
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 },
    // 0x22 '"'
    //   .#.#.
    //   .#.#.
    //   #.#..
    //   .....
    //   .....
    //   .....
    //   .....
    { 0x0a, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x00 },
    // 0x23 '#'
    //   .#.#.
    //   .#.#.
    //   #####
    //   .#.#.
    //   #####
    //   .#.#.
    //   .#.#.
    { 0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a },
    // 0x24 '$'
    //   ..#..
    //   .####
    //   #.#..
    //   .###.
    //   ..#.#
    //   ####.
    //   ..#..
    { 0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04 },
    // 0x25 '%'
    //   ##...
    //   ##..#
    //   ...#.
    //   ..#..
    //   .#...
    //   #..##
    //   ...##
    { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03 },
    // 0x26 '&'
    //   .##..
    //   #..#.
    //   #.#..
    //   .#...
    //   #.#.#
    //   #..#.
    //   .##.#
    { 0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d },
    // 0x27 '''
    //   ..#..
    //   ..#..
    //   ..#..
    //   .....
    //   .....
    //   .....
    //   .....
    { 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00 },
    // 0x28 '('
    //   ...#.
    //   ..#..
    //   .#...
    //   .#...
    //   .#...
    //   ..#..
    //   ...#.
    { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 },
    // 0x29 ')'
    //   .#...
    //   ..#..
    //   ...#.
    //   ...#.
    //   ...#.
    //   ..#..
    //   .#...
    { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 },
    // 0x2a '*'
    //   .....
    //   ..#..
    //   #.#.#
    //   .###.
    //   #.#.#
    //   ..#..
    //   .....
    { 0x00, 0x04, 0x15, 0x0e, 0x15, 0x04, 0x00 },
    // 0x2b '+'
    //   .....
    //   .....
    //   ..#..
    //   .###.
    //   ..#..
    //   .....
    //   .....
    { 0x00, 0x00, 0x04, 0x0e, 0x04, 0x00, 0x00 },
    // 0x2c ','
    //   .....
    //   .....
    //   .....
    //   .....
    //   ..##.
    //   ..##.
    //   .#...
    { 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x08 },
    // 0x2d '-'
    //   .....
    //   .....
    //   .....
    //   .###.
    //   .....
    //   .....
    //   .....
    { 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00 },
    // 0x2e '.'
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    //   .##..
    //   .##..
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c },
    // 0x2f '/'
    //   .....
    //   ....#
    //   ...#.
    //   ..#..
    //   .#...
    //   #....
    //   .....
    { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00 },
    // 0x30 '0'
    //   .###.
    //   #...#
    //   #..##
    //   #.#.#
    //   ##..#
    //   #...#
    //   .###.
    { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e },
    // 0x31 '1'
    //   ..#..
    //   .##..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   .###.
    { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e },
    // 0x32 '2'
    //   .###.
    //   #...#
    //   ....#
    //   ...#.
    //   ..#..
    //   .#...
    //   #####
    { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f },
    // 0x33 '3'
    //   ####.
    //   ....#
    //   ....#
    //   .###.
    //   ....#
    //   ....#
    //   ####.
    { 0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e },
    // 0x34 '4'
    //   ...#.
    //   ..##.
    //   .#.#.
    //   #..#.
    //   #####
    //   ...#.
    //   ...#.
    { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 },
    // 0x35 '5'
    //   #####
    //   #....
    //   ####.
    //   ....#
    //   ....#
    //   #...#
    //   .###.
    { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e },
    // 0x36 '6'
    //   ..##.
    //   .#...
    //   #....
    //   ####.
    //   #...#
    //   #...#
    //   .###.
    { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e },
    // 0x37 '7'
    //   #####
    //   ....#
    //   ...#.
    //   ..#..
    //   .#...
    //   .#...
    //   .#...
    { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },
    // 0x38 '8'
    //   .###.
    //   #...#
    //   #...#
    //   .###.
    //   #...#
    //   #...#
    //   .###.
    { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e },
    // 0x39 '9'
    //   .###.
    //   #...#
    //   #...#
    //   .####
    //   ....#
    //   ...#.
    //   .##..
    { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c },
    // 0x3a ':'
    //   .....
    //   .##..
    //   .##..
    //   .....
    //   .##..
    //   .##..
    //   .....
    { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00 },
    // 0x3b ';'
    //   .....
    //   .##..
    //   .##..
    //   .....
    //   .##..
    //   ..#..
    //   .#...
    { 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x04, 0x08 },
    // 0x3c '<'
    //   ....#
    //   ...#.
    //   ..#..
    //   .#...
    //   ..#..
    //   ...#.
    //   ....#
    { 0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01 },
    // 0x3d '='
    //   .....
    //   .....
    //   .####
    //   .....
    //   .####
    //   .....
    //   .....
    { 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00 },
    // 0x3e '>'
    //   #....
    //   .#...
    //   ..#..
    //   ...#.
    //   ..#..
    //   .#...
    //   #....
    { 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10 },
    // 0x3f '?'
    //   .###.
    //   #...#
    //   ....#
    //   ...#.
    //   ..#..
    //   .....
    //   ..#..
    { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 },
    // 0x40 '@'
    //   .###.
    //   #...#
    //   #.###
    //   #.#.#
    //   #.###
    //   #....
    //   .###.
    { 0x0e, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0e },
    // 0x41 'A'
    //   .###.
    //   #...#
    //   #...#
    //   #####
    //   #...#
    //   #...#
    //   #...#
    { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 },
    // 0x42 'B'
    //   ####.
    //   #...#
    //   #...#
    //   ####.
    //   #...#
    //   #...#
    //   ####.
    { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e },
    // 0x43 'C'
    //   .####
    //   #....
    //   #....
    //   #....
    //   #....
    //   #....
    //   .####
    { 0x0f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0f },
    // 0x44 'D'
    //   ###..
    //   #..#.
    //   #...#
    //   #...#
    //   #...#
    //   #..#.
    //   ###..
    { 0x1c, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1c },
    // 0x45 'E'
    //   #####
    //   #....
    //   #....
    //   ####.
    //   #....
    //   #....
    //   #####
    { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f },
    // 0x46 'F'
    //   #####
    //   #....
    //   #....
    //   ####.
    //   #....
    //   #....
    //   #....
    { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 },
    // 0x47 'G'
    //   .###.
    //   #...#
    //   #....
    //   #.###
    //   #...#
    //   #...#
    //   .####
    { 0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f },
    // 0x48 'H'
    //   #...#
    //   #...#
    //   #...#
    //   #####
    //   #...#
    //   #...#
    //   #...#
    { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 },
    // 0x49 'I'
    //   .###.
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   .###.
    { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e },
    // 0x4a 'J'
    //   ..###
    //   ...#.
    //   ...#.
    //   ...#.
    //   ...#.
    //   #..#.
    //   .##..
    { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c },
    // 0x4b 'K'
    //   #...#
    //   #..#.
    //   #.#..
    //   ##...
    //   #.#..
    //   #..#.
    //   #...#
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 },
    // 0x4c 'L'
    //   #....
    //   #....
    //   #....
    //   #....
    //   #....
    //   #....
    //   #####
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f },
    // 0x4d 'M'
    //   #...#
    //   ##.##
    //   #.#.#
    //   #.#.#
    //   #...#
    //   #...#
    //   #...#
    { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 },
    // 0x4e 'N'
    //   #...#
    //   #...#
    //   ##..#
    //   #.#.#
    //   #..##
    //   #...#
    //   #...#
    { 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11 },
    // 0x4f 'O'
    //   #####
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   #####
    { 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f },
    // 0x50 'P'
    //   ####.
    //   #...#
    //   #...#
    //   ####.
    //   #....
    //   #....
    //   #....
    { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 },
    // 0x51 'Q'
    //   .###.
    //   #...#
    //   #...#
    //   #...#
    //   #.#.#
    //   #..#.
    //   .##.#
    { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d },
    // 0x52 'R'
    //   ####.
    //   #...#
    //   #...#
    //   ####.
    //   #.#..
    //   #..#.
    //   #...#
    { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 },
    // 0x53 'S'
    //   .####
    //   #....
    //   #....
    //   .###.
    //   ....#
    //   ....#
    //   ####.
    { 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e },
    // 0x54 'T'
    //   #####
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
    // 0x55 'U'
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   .###.
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e },
    // 0x56 'V'
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   #...#
    //   .#.#.
    //   ..#..
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 },
    // 0x57 'W'
    //   #...#
    //   #...#
    //   #...#
    //   #.#.#
    //   #.#.#
    //   #.#.#
    //   .#.#.
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a },
    // 0x58 'X'
    //   #...#
    //   #...#
    //   .#.#.
    //   ..#..
    //   .#.#.
    //   #...#
    //   #...#
    { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 },
    // 0x59 'Y'
    //   #...#
    //   #...#
    //   .#.#.
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 },
    // 0x5a 'Z'
    //   #####
    //   ....#
    //   ...#.
    //   ..#..
    //   .#...
    //   #....
    //   #####
    { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f },
    // 0x5b '['
    //   .###.
    //   .#...
    //   .#...
    //   .#...
    //   .#...
    //   .#...
    //   .###.
    { 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e },
    // 0x5c '\'
    //   .....
    //   #....
    //   .#...
    //   ..#..
    //   ...#.
    //   ....#
    //   .....
    { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00 },
    // 0x5d ']'
    //   .###.
    //   ...#.
    //   ...#.
    //   ...#.
    //   ...#.
    //   ...#.
    //   .###.
    { 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e },
    // 0x5e '^'
    //   ..#..
    //   .#.#.
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    { 0x04, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // 0x5f '_'
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    //   .....
    //   ####.
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e },
    // 0x60 '`'
    //   .#...
    //   ..#..
    //   ...#.
    //   .....
    //   .....
    //   .....
    //   .....
    { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 },
    // 0x61 'a'
    //   .....
    //   .....
    //   .###.
    //   ....#
    //   .####
    //   #...#
    //   .####
    { 0x00, 0x00, 0x0e, 0x01, 0x0f, 0x11, 0x0f },
    // 0x62 'b'
    //   #....
    //   #....
    //   #.##.
    //   ##..#
    //   #...#
    //   #...#
    //   ####.
    { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1e },
    // 0x63 'c'
    //   .....
    //   .....
    //   .###.
    //   #....
    //   #....
    //   #...#
    //   .###.
    { 0x00, 0x00, 0x0e, 0x10, 0x10, 0x11, 0x0e },
    // 0x64 'd'
    //   ....#
    //   ....#
    //   .##.#
    //   #..##
    //   #...#
    //   #...#
    //   .####
    { 0x01, 0x01, 0x0d, 0x13, 0x11, 0x11, 0x0f },
    // 0x65 'e'
    //   .....
    //   .....
    //   .###.
    //   #...#
    //   #####
    //   #....
    //   .###.
    { 0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0e },
    // 0x66 'f'
    //   ..##.
    //   .#..#
    //   .#...
    //   ###..
    //   .#...
    //   .#...
    //   .#...
    { 0x06, 0x09, 0x08, 0x1c, 0x08, 0x08, 0x08 },
    // 0x67 'g'
    //   .....
    //   .####
    //   #...#
    //   #...#
    //   .####
    //   ....#
    //   .###.
    { 0x00, 0x0f, 0x11, 0x11, 0x0f, 0x01, 0x0e },
    // 0x68 'h'
    //   #....
    //   #....
    //   #.##.
    //   ##..#
    //   #...#
    //   #...#
    //   #...#
    { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11 },
    // 0x69 'i'
    //   ..#..
    //   .....
    //   .##..
    //   ..#..
    //   ..#..
    //   ..#..
    //   .###.
    { 0x04, 0x00, 0x0c, 0x04, 0x04, 0x04, 0x0e },
    // 0x6a 'j'
    //   ...#.
    //   .....
    //   ..##.
    //   ...#.
    //   ...#.
    //   #..#.
    //   .##..
    { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c },
    // 0x6b 'k'
    //   #....
    //   #....
    //   #..#.
    //   #.#..
    //   ##...
    //   #.#..
    //   #..#.
    { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12 },
    // 0x6c 'l'
    //   .##..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   .###.
    { 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e },
    // 0x6d 'm'
    //   .....
    //   .....
    //   ##.#.
    //   #.#.#
    //   #.#.#
    //   #...#
    //   #...#
    { 0x00, 0x00, 0x1a, 0x15, 0x15, 0x11, 0x11 },
    // 0x6e 'n'
    //   .....
    //   .....
    //   #.##.
    //   ##..#
    //   #...#
    //   #...#
    //   #...#
    { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11 },
    // 0x6f 'o'
    //   .....
    //   .....
    //   .###.
    //   #...#
    //   #...#
    //   #...#
    //   .###.
    { 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e },
    // 0x70 'p'
    //   .....
    //   .....
    //   ####.
    //   #...#
    //   ####.
    //   #....
    //   #....
    { 0x00, 0x00, 0x1e, 0x11, 0x1e, 0x10, 0x10 },
    // 0x71 'q'
    //   .....
    //   .....
    //   .##.#
    //   #..##
    //   .####
    //   ....#
    //   ....#
    { 0x00, 0x00, 0x0d, 0x13, 0x0f, 0x01, 0x01 },
    // 0x72 'r'
    //   .....
    //   .....
    //   #.##.
    //   ##..#
    //   #....
    //   #....
    //   #....
    { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10 },
    // 0x73 's'
    //   .....
    //   .....
    //   .###.
    //   #....
    //   .###.
    //   ....#
    //   ####.
    { 0x00, 0x00, 0x0e, 0x10, 0x0e, 0x01, 0x1e },
    // 0x74 't'
    //   .#...
    //   .#...
    //   ###..
    //   .#...
    //   .#...
    //   .#..#
    //   ..##.
    { 0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06 },
    // 0x75 'u'
    //   .....
    //   .....
    //   #...#
    //   #...#
    //   #...#
    //   #..##
    //   .##.#
    { 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d },
    // 0x76 'v'
    //   .....
    //   .....
    //   #...#
    //   #...#
    //   #...#
    //   .#.#.
    //   ..#..
    { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04 },
    // 0x77 'w'
    //   .....
    //   .....
    //   #...#
    //   #...#
    //   #.#.#
    //   #.#.#
    //   .#.#.
    { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0a },
    // 0x78 'x'
    //   .....
    //   .....
    //   #...#
    //   .#.#.
    //   ..#..
    //   .#.#.
    //   #...#
    { 0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11 },
    // 0x79 'y'
    //   .....
    //   .....
    //   #...#
    //   #...#
    //   .####
    //   ....#
    //   .###.
    { 0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0x0e },
    // 0x7a 'z'
    //   .....
    //   .....
    //   #####
    //   ...#.
    //   ..#..
    //   .#...
    //   #####
    { 0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f },
    // 0x7b '{'
    //   ...##
    //   ..#..
    //   ..#..
    //   .#...
    //   ..#..
    //   ..#..
    //   ...##
    { 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x03 },
    // 0x7c '|'
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    //   ..#..
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
    // 0x7d '}'
    //   ##...
    //   ..#..
    //   ..#..
    //   ...#.
    //   ..#..
    //   ..#..
    //   ##...
    { 0x18, 0x04, 0x04, 0x02, 0x04, 0x04, 0x18 },
    // 0x7e '~'
    //   .....
    //   .....
    //   .#...
    //   #.#.#
    //   ...#.
    //   .....
    //   .....
    { 0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00 },
};

extern "C" int op1f_paint_text(void* canvas, int x, int y, const char* text, unsigned colour, int scale) {
    if (text == 0 || scale < 1 || scale > (int)SCALE_MOST) return 0;
    u32 n = 0;
    while (n < (u32)TEXT_MOST && text[n]) ++n;
    if (n == 0) return 0;
    int taken = ((int)n * (int)ADVANCE - 1) * scale;
    u16* to = may_paint(canvas);
    if (to == 0) return taken;
    if (x < -(int)REACH || x > (int)REACH || y < -(int)REACH || y > (int)REACH) return taken;
    for (u32 i = 0; i < n; ++i) {
        u32 c = (u8)text[i];
        if (c < (u32)FIRST_GLYPH || c > (u32)LAST_GLYPH) c = '?';
        const u8* rows = FONT[c - FIRST_GLYPH];
        int gx = x + (int)i * (int)ADVANCE * scale;
        for (int r = 0; r < (int)GLYPH_TALL; ++r) {
            for (int b = 0; b < (int)GLYPH_WIDE; ++b) {
                if (rows[r] & (0x10u >> b)) box(to, gx + b * scale, y + r * scale, scale, scale, (u16)colour);
            }
        }
    }
    return taken;
}

// R6b: the base's own words on a page of its own, from the cards unit's frame work on the display thread, while no
// pack's frame function runs (op1f_cards_calling 0): as op1f_paint_text, into the canvas the cards unit hands, which is
// this frame's. A canvas of 0 paints nothing and is not counted; else the call is counted with the calls from no
// frame function: as painted, or as refused when a pack's frame function runs. The width is answered either way.
extern "C" int op1f_paint_text_base(void* canvas, int x, int y, const char* text, unsigned colour, int scale) OP1F_NAMED("op1f_paint_text_base");
extern "C" int op1f_paint_text_base(void* canvas, int x, int y, const char* text, unsigned colour, int scale) {
    if (text == 0 || scale < 1 || scale > (int)SCALE_MOST) return 0;
    u32 count = 0;
    while (count < (u32)TEXT_MOST && text[count]) ++count;
    if (count == 0) return 0;
    int wide = ((int)count * (int)ADVANCE - 1) * scale;                         // the last glyph's column of space not counted
    if (canvas == 0) return wide;
    if (op1f_cards_calling() != 0) {                                            // only from the base's own frame work
        ++refused[NONE];
        return wide;
    }
    if (x < -(int)REACH || x > (int)REACH || y < -(int)REACH || y > (int)REACH) return wide;
    ++painted[NONE];
    u16* into = (u16*)canvas;
    for (u32 k = 0; k < count; ++k) {
        u32 code = (u8)text[k];
        if (code < (u32)FIRST_GLYPH || code > (u32)LAST_GLYPH) code = '?';     // a glyph the font lacks: the question mark
        const u8* glyph = FONT[code - FIRST_GLYPH];
        int left = x + (int)k * (int)ADVANCE * scale;
        for (int row = 0; row < (int)GLYPH_TALL; ++row) {
            for (int bit = 0; bit < (int)GLYPH_WIDE; ++bit) {
                if (glyph[row] & (0x10u >> bit)) box(into, left + bit * scale, y + row * scale, scale, scale, (u16)colour);
            }
        }
    }
    return wide;
}

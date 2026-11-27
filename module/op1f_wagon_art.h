// A wagon's picture of a pack's effect, for a train that draws it (hasis's snowtrain; Faust's fx train draws the
// effect it rides). An effect pack that wants its effect drawn
// on the train as itself, and not as the unit's effect it rides, names one in its effect's ask ("wagon": the symbol of
// an Op1fWagonArt, docs/level-8.md). The pack tool then writes the ask's table 184 bytes long, the picture's address
// at +180 (OP1F_WAGON_ART_AT), behind the 180 bytes of level 8. A base of level 8 takes the longer table as it is, and
// a host that does not know the field never reads it.
//
// Such a train paints the picture on the frame's canvas (level 4, op1f_paint_block) where the wagon's character would
// stand, on the wagon's chassis, with the wagon's moves, a gentle sway on the line and every colour dim on the siding.
// Where an effect names none, or one out of the limits below, the wagon is drawn as the effect it rides, as before.
//
// The picture: `width` x `height` bytes, row by row from the top, each an index into `colours` (RGB565, red in the
// high bits). Index 0 is see-through, whatever colours[0] holds; an index at or past `count` too. Draw it as the
// train's own characters are drawn (line art, stroke #A2A4A6, near-black #000005 fills that hide what lies behind,
// #CDD0C2 for small light parts, a knob colour's dot or two), its soft edges already mixed on black.
#ifndef OP1F_WAGON_ART_H
#define OP1F_WAGON_ART_H

enum { OP1F_WAGON_ART_AT = 180, OP1F_EFFECT_WAGON_BYTES = 184, OP1F_WAGON_ART_BYTES = 24 };
// The limits a host holds a picture to: at most 80 by 64 pixels, its top-left between x -8 and 72 of the wagon's
// frame (the wagon's left at 0, its right at 70) and between y 80 and 144 of the train's page (the floor at 144).
enum { OP1F_WAGON_ART_WIDE = 80, OP1F_WAGON_ART_TALL = 64 };

struct Op1fWagonArt {
    unsigned bytes;                     // OP1F_WAGON_ART_BYTES: the size of this head
    short x, y;                         // its top-left: x in the wagon's frame, y on the train's page
    unsigned short width, height;       // 1 to OP1F_WAGON_ART_WIDE, 1 to OP1F_WAGON_ART_TALL
    unsigned short count, reserved;     // the colours, 2 to 256; reserved 0
    const unsigned short* colours;      // count colours, RGB565
    const unsigned char* pixels;        // width x height indices
};   // 24 bytes

#endif

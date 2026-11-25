# A page that holds its picture (bases 33 and 36)

At every frame the unit clears the picture to black and draws your page's document into it, and then the packs paint over that. A page with many parts pays for that drawing at every frame, even when nothing on it moved. On the maintainer's unit a test page of 510 circles ran at 28 frames a second drawn whole, and at 58 once it held its picture.

Two parts of the base let a page skip that work:

- **Base 33**: the page asks to keep its picture. The unit draws nothing of the page in the next frame, and your frame function repaints only what changed. For a page whose document stays as it is and whose moving things you paint yourself: a few numbers, a dot, a bar.
- **Base 36**: the page keeps its picture and names the rectangles that changed and the parts of its document that lie there. The unit draws only those. For a page whose moving things are parts of its own picture, moved, shown or hidden, or texts set.

Neither is a level. The level stays 10, and each part has a mark of its own: `op1f_hold_1` from base 33, `op1f_parts_1` from base 36. A page that asks neither is drawn as before.

The words of the interface are in the header `module/build_r6e_card.h`, the sections "Base 33" and "Base 36". Where this page and the header disagree, the header is right.

## What a frame function is handed, from base 33

Base 33 hands a frame function 48 bytes and says so in `bytes`: level 4's 40, then two words. `Op1fFrame` keeps its size, so a pack that compares `bytes` with it keeps its meaning. Read the two words with the helpers, which answer 0 on an older base:

| Helper | What it answers |
| --- | --- |
| `op1f_frame_held(now)` | 1 when the unit drew nothing of the page in this frame: the canvas holds the picture of two frames before, with everything painted into it then. 0 when the page was drawn whole |
| `op1f_frame_picture(now)` | Which of the display's two pictures this frame's canvas is, 0 or 1. It alternates frame by frame |

**The display has two pictures and draws into them in turn.** So a held canvas is the picture of the last frame with the same `picture`, two frames back. What you painted into the other picture in the frame between is not in it.

## Holding the picture (base 33)

```cpp
int op1f_page_hold(const Op1fCard* card, unsigned redraw);
```

- **Call it at every frame the hold should go on**, from your card's frame function, while your page is the one shown (the frame's `page` is your card). It asks for the next frame. A frame without the ask is followed by a whole picture.
- **Read `held` at every frame.** 0: the page was drawn whole, so paint all of your own, as on base 32. 1: repaint what changed since the frame with the same `picture`. What you painted stays until you paint over it.
- **`redraw` not 0** asks for the next two frames at least to be drawn whole: for a change the base cannot see (below).
- It answers 1 when the ask is taken, 0 when not: another page shown, or a call from anywhere but your card's frame function.

**The base draws the page whole by itself, for the next three frames,** whenever the picture could differ from the one kept:

- the page comes onto the screen, or another document comes under it;
- the base changes the document: `op1f_show` and `op1f_move` where they change a part, `op1f_text` and `op1f_text_set`, a picture made with `op1f_picture`;
- one of the unit's popups, the base's notice or its top page lies over your page, or the power popup is up;
- the base paints the ask's name on a stand-in's blank page;
- you asked it with `redraw`.

You need to know none of these: read `held`. Anything you change in your picture by other means than those helpers, say it with `redraw`.

**Something that moves leaves its old place painted.** Keep what lies under it with `op1f_paint_copy` before you paint it, and put that back with `op1f_paint_block` and a `key` of -1 in the next frame with the same `picture`:

```cpp
int op1f_paint_copy(void* canvas, int x, int y, int w, int h, unsigned short* pixels, int stride);
```

It copies `w` by `h` pixels of this frame's picture from (x, y) into `pixels`, row by row from the top, `stride` pixels from one row's start to the next: the layout `op1f_paint_block` takes back. Pixels of the block that lie outside the picture are not written. The painting helpers' rules hold ([level 4](level-4.md), "Painting"): only from your own frame function and with this frame's canvas. Nothing for a `w` or an `h` of 0 or less or more than 480, or for `pixels` 0. It answers 1 when anything was copied.

A dot that moves over a held page, one copy of what lies under it for each picture:

```cpp
static unsigned short under[2][DOT * DOT];
static int under_x[2], under_y[2], kept[2];

extern "C" void dot_frame(const Op1fFrame* now) {
    if (now->page != &op1f_card_dot) return;
    op1f_page_hold(&op1f_card_dot, 0);                   // at every frame the hold goes on
    unsigned p = op1f_frame_picture(now);
    if (op1f_frame_held(now) && kept[p])                 // the dot of two frames ago is in this picture
        op1f_paint_block(now->canvas, under_x[p], under_y[p], DOT, DOT, under[p], DOT, -1);
    int x = dot_x(now->ms), y = dot_y(now->ms);
    kept[p] = op1f_paint_copy(now->canvas, x, y, DOT, DOT, under[p], DOT);
    under_x[p] = x;
    under_y[p] = y;
    op1f_paint_box(now->canvas, x, y, DOT, DOT, RED);
}
```

On a canvas drawn whole there is nothing to put back: the copy of that picture is simply taken again. A number that changes is the same idea in a smaller form: paint a box of the ground's colour behind it, then the number.

**The mark.** A pack that calls `op1f_page_hold` or `op1f_paint_copy` needs base 33. The two helpers are names your pack needs, so under an older base the loader leaves it out with `nobody offers the name it needs`. Name `op1f_hold_1` in your code too (a line in the log that prints it will do), and the loader's line names the mark instead.

## Redrawing only the parts that changed (base 36)

A page held with `op1f_page_hold` may not change its document: any change makes the base draw it whole. A page whose own parts keep moving asks instead with `op1f_page_parts`, and in the same frame names each rectangle of its picture that changed and the parts of its document that lie there, with `op1f_page_redraw`.

```cpp
int op1f_page_parts(const Op1fCard* card);
int op1f_page_redraw(const Op1fCard* card, int x, int y, int w, int h, void* const* parts, unsigned count);
```

In the next frame, when the base holds the page, the unit draws, for each rectangle in turn, your document cut to that rectangle with only the named parts shown: each named part with everything under it as its own display says, and the groups that hold it. Everything else of the document is left out of that drawing. Outside the rectangles the picture stays as it was two frames before, as with `op1f_page_hold`. When the base draws whole instead, for one of the reasons above, the names are not used.

So the page's look is the unit's own. A frame costs what the named parts cost to draw (about the same as in a whole picture, part for part: the cut saves little), plus a little for each rectangle. On the maintainer's unit a busy page whose few moving parts were named went from 29 frames a second to 55 to 59.

What to name:

- **What changed since the frame with the same `picture`**: where a part was and where it is now. Two frames, not one.
- **Everything that lies in a rectangle, its ground first among them.** The base does not clear a rectangle, and a part not named is missing there for that frame.
- **Only parts of your page's own document**, as `op1f_node` or `op1f_text_node` answered them, and none twice in one rectangle.

The rules:

- `op1f_page_parts` takes the place of `op1f_page_hold`. Call it from your card's frame function while your page is shown, at every frame the asking should go on. With no rectangle named, the picture is held as it is. It answers 1 when the ask is taken, 0 when not.
- `op1f_page_redraw` names a rectangle of `w` by `h` pixels from (x, y), cut to the picture, and the `count` parts drawn in it. Call it from the same place, before or after `op1f_page_parts` in the same frame. It answers 1 when taken, and 0 when not: another page shown, a call from elsewhere, a rectangle of nothing once cut to the picture, no parts, a part that is 0, or more than `OP1F_REDRAWS_MOST` rectangles (8) or `OP1F_REDRAW_PARTS_MOST` parts (160, all rectangles together) in this frame. Then the next frame is drawn whole.
- In a frame where the page asks `op1f_page_parts`, the changes your card makes from its frame function (`op1f_show`, `op1f_move`, and `op1f_text` and `op1f_text_set` on the page's document) no longer make the base draw whole. Everything else in base 33's list still does, and so does a rectangle refused, or rectangles named in a frame that then does not ask.
- **A page that asks changes its document only from its frame function.** Around each drawing the base hides and shows parts itself, and puts back what it found.
- The base also draws the frame whole when it cannot work out what to leave out: more than 2,048 parts to leave out in all the frame's rectangles, groups nested deeper than 24, or a document in which a part is reached by two ways. The line below counts those frames as `plans failed`.

A needle that turns on a dial, its ground and its old and new places in one rectangle:

```cpp
static void* ground;                                     // op1f_node(document, "dial_ground"), once
static void* needle;                                     // op1f_node(document, "dial_needle"), once
static int box[2][4];                                    // where the needle lay at each picture

extern "C" void dial_frame(const Op1fFrame* now) {
    if (now->page != &op1f_card_dial) return;
    unsigned p = op1f_frame_picture(now);
    op1f_move(needle, needle_matrix(angle()));
    int x0, y0, x1, y1;
    union_of(box[p], needle_box(angle()), &x0, &y0, &x1, &y1);   // its place two frames ago and now
    void* const parts[2] = { ground, needle };
    op1f_page_redraw(&op1f_card_dial, x0, y0, x1 - x0, y1 - y0, parts, 2);
    op1f_page_parts(&op1f_card_dial);
    keep_box(box[p], needle_box(angle()));
}
```

**The mark.** `op1f_parts_1`. A pack that must have base 36 names it. A pack that also runs on an older base declares `op1f_page_parts` and `op1f_page_redraw` weak and calls them only when their address is not 0, and holds or draws whole otherwise:

```cpp
extern "C" int __attribute__((weak)) op1f_page_parts(const Op1fCard* card) OP1F_NAMED("op1f_page_parts");
extern "C" int __attribute__((weak)) op1f_page_redraw(const Op1fCard* card, int x, int y, int w, int h, void* const* parts,
                                                      unsigned count) OP1F_NAMED("op1f_page_redraw");
```

## The lines the base writes

The tag is `op1f_cards`.

| Line begins | What it says |
| --- | --- |
| `page: '<pack>' holds its picture from frame to frame; <n> time(s) since the start` | A hold began. For the first sixteen holds of a start |
| `page: '<pack>' stopped holding its picture: <n> frame(s) held, <n> drawn whole since it began` | It ended: the page stopped asking, went, or another document came under it |
| `page: '<pack>' parts, second <n>: <n> frames, <n> held, <n> rectangles drawn, <n> hidden, <n> us in all, <n> most; <n> refused, <n> plans failed` | Base 36, once a second for the first 30 seconds of a page's asking, for its first sixteen runs: the frames, how many were held, the rectangles drawn and the parts left out of them, the time the drawings took, and the rectangles refused |

A held frame against one drawn whole shows in `frame cost:` ([the base](base.md), "The lines the base writes into the log").

## Limits, in one place

| | |
| --- | --- |
| From | Your card's frame function, while its page is the one shown |
| A hold | Asked again at every frame; after anything in base 33's list, three frames drawn whole before the next hold |
| A held canvas | The picture of two frames before |
| Rectangles a frame | 8 (`OP1F_REDRAWS_MOST`) |
| Parts a frame | 160 in all rectangles together (`OP1F_REDRAW_PARTS_MOST`) |
| The marks | `op1f_hold_1` (base 33), `op1f_parts_1` (base 36) |

Both have run on the maintainer's unit with test pages of his own: base 33's hold with a page of 510 circles, a square moved with `op1f_paint_copy` and `op1f_paint_block` and no trail left, and texts that changed; base 36's rectangles with a busy page, each held picture compared after a whole redraw and found the same. No example in this repository uses them yet.

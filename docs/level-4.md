# Level 4: what a page needs

Levels 1 to 3 get a page onto the screen. Level 4 is for the pack that wants to draw on it properly. Your pages can stack, one over another. There's a black page of the base's that you paint yourself. Text can be set left, centred or to the right, measured, and moved like any part. You can paint boxes, lines, pixels and text straight onto the frame. You can delete a file in your folder. And your input function can watch a key without taking it away from anyone, and it's told which thread it was called on.

Level 4 also gives the card its list of **asks**: things a pack asks of the base. Level 4 itself defines no kind of ask. Levels 5 to 9 fill the list ([level 5](level-5.md), [level 6](level-6.md), [level 7](level-7.md), [level 8](level-8.md), [level 9](level-9.md)).

The words of the interface are in the header `module/build_r6e_card.h`, which your pack includes. It names no address of the firmware. Where this page and the header disagree, the header is right.

## The example: `sketch`

[`sketch`](../examples/sketch/README.md) is a drawing board. Hold **shift** and press the **metronome** key, in any mode, and its page comes up: a black board with a small blue cross in the middle and a title at the top. Turn the **blue** knob and the cross moves left and right, the **second** knob moves it up and down, and every move leaves a line behind. Press the **red** knob to wipe the board. Press the **third** knob to show the same board on the base's blank page, and again to go back. Shift and the metronome key close it.

It keeps your drawing in `sketch.bin`, in a folder of its own that you can see in the disk mode. Wiping the board deletes the file. The code quoted below is `sketch`'s own.

Shift and the metronome key open three of the examples: `dub`, `sketch` and `yoyo`. The pack whose file sorts first gets the keys, so with `dub` on your unit the other two have no way in, and with `sketch` there `yoyo` has none. Keep one of the three on your unit at a time.

## The card at level 4

A card of level 4 is 64 bytes. Behind the folders of level 3 it has two fields more, the list of asks and their count, and you never write either by hand. This is `sketch`'s card, in its recipe:

```json
"card": { "level": 4, "frame": "sketch_frame", "pick": null, "pictures": true, "input": "sketch_input", "folders": [ {} ], "asks": [] }
```

The card's source makes `op1f_level_4` a name your pack needs. On a base that doesn't offer it, the loader leaves your pack out and the log says which mark was missing. A base of level 4 offers the marks of levels 1 to 3 as well, so a pack built for an earlier level runs on it as it is.

What a frame function is handed grows to 40 bytes at level 4 (`bytes` says so), with one field more, `canvas` (below). What an input function is handed grows to 40 bytes too, with `thread` (below). Read `bytes` before you read a field a level added.

## Pages that stack

Level 2's `op1f_page_open` shows one page in place of everything else. Level 4 adds `op1f_page_show`, which puts your page **over** whatever page is there.

| Helper | What it does | From where |
| --- | --- | --- |
| `op1f_page_show(card, document, flags)` | Your page shown over the page that's there, in the mode the unit is in. `document` is one your card made with `op1f_picture`, or 0 for the base's blank page. `flags` is 0, or `OP1F_PAGE_STAYS` ([level 7](level-7.md), "A page that stays with its mode"). Answers 1 when the base took the request | Your frame function or your input function |
| `op1f_page_close(card)` | Your page closed. The page under it shows again | The same |

What happens to a stack:

- **The page on top is the one shown**, and the frame's `page` names its card. Compare it with your own card to know your page is up. As with level 2, a page asked for at one frame shows from the frame after.
- **Closing the top page shows the one under it again.** Four pages can lie under the shown one. When a fifth would go under, the lowest is dropped, with a line in the log.
- **A mode change closes them all**, unless the top page was shown with `OP1F_PAGE_STAYS`.
- A page opened the old way, with `op1f_page_open`, still behaves the old way. A page shown over it closes it.
- **The base's own top page can come over yours** for a moment. It's the page that asks you to confirm something, such as the packs a computer sent over USB MIDI. While it's up, the frame's `page` still names your card, and no key or knob reaches any pack until it's gone. From base 38, `canvas` is 0 from the frame it's asked for until it's gone, so nothing you paint can hide it.
- `op1f_page_show` answers 0 for a document your card didn't make, for a card the base doesn't have in use, and for a flag the base doesn't know. Taken is not the same as shown. The frame's `page` is what tells you your page is up.

`sketch` opens and closes its page on one key:

```cpp
if (e->what == (u32)OP1F_DOWN_SHIFT && e->key == (u32)KEY_METRONOME) {     // the way in, in any mode
    void* d = document;
    if (d == 0) return 0;                                                  // no page could be made: the keys stay the firmware's
    ++keys;
    if (e->mine) op1f_page_close(&op1f_card_sketch);
    else op1f_page_show(&op1f_card_sketch, blank ? 0 : d, 0);
    return 1;
}
```

## The blank page

Hand `op1f_page_show` a document of 0 and you get the base's **blank page**: black, the whole screen, the same for every pack. Its text isn't yours to set. It's there for the pack that paints its whole page itself, so you don't need a picture at all. `dub` ([level 6](level-6.md)), `echo` ([level 5](level-5.md)) and the four examples of levels 8 and 9 all draw their pages this way.

The base makes the blank page once, at the first request that needs it. If it can't be made, a page asked for with no document isn't shown, and the log says so once.

## The unit's own popups over your page

Some of the unit's small popups come up over whatever screen is showing: the octave when an octave key is pressed, the count-in, undo, the sequencer-off notice, the countdown to switching off, and the others. **Over a pack's page they come up as they do over the unit's own screens, and you have nothing to do for them.** The base draws them itself, after every pack has painted, in the unit's own order: the sequencer-off notice first, which darkens the page under it ([level 5](level-5.md), "When the unit says SEQ OFF"), then the others over it, and the countdown on top of everything.

So on your own page the popups lie over your painting. When you paint over one of the unit's own screens instead, your painting lies over everything on it, the popups included. Keep what you paint there small.

## Painting

Every frame the screen is drawn whole, and then the base calls the packs' frame functions. The frame's `canvas` is that frame's picture, already drawn. Whatever you paint on it lies over the screen for that one frame (under the unit's own popups, on a pack's page) and is gone at the next. **So you paint again every frame.** From base 38, `canvas` is 0 while the base's own top page is asked for or up: paint nothing then.

The picture is 480 pixels wide and 224 tall, x from the left and y from the top. Anything that runs off an edge is cut. A colour is sixteen bits, made with `OP1F_COLOUR(r, g, b)` from three numbers of 0 to 255. Take your colours from your unit's own `content/display/palette.svg` so your page looks like the rest of the device. The knobs, for instance, are blue `OP1F_COLOUR(0x3c, 0x60, 0x87)`, ochre `OP1F_COLOUR(0x8a, 0x65, 0x3c)`, grey `OP1F_COLOUR(0xa2, 0xa4, 0xa6)` and red `OP1F_COLOUR(0xe5, 0x28, 0x17)`.

| Helper | What it paints |
| --- | --- |
| `op1f_paint_box(canvas, x, y, w, h, colour)` | A filled box, its top-left at (x, y) |
| `op1f_paint_line(canvas, x0, y0, x1, y1, width, colour)` | A line between two points, both ends painted, with a square pen 1 to 16 pixels wide |
| `op1f_paint_block(canvas, x, y, w, h, pixels, stride, key)` | A block of your own pixels, row by row. A pixel equal to `key` is left out so what's under it shows through. A `key` of -1 leaves nothing out |
| `op1f_paint_mask(canvas, x, y, w, h, cover, stride, colour)` | One colour laid over what's there through a block of cover bytes, 0 none and 255 all of it. This is how you paint shapes and letters with soft edges |
| `op1f_paint_text(canvas, x, y, text, colour, scale)` | Text in the base's own small font, 5 by 7 pixels a character, `scale` 1 to 8. It answers the width the text takes, painted or not, so it's also how you measure a text before you place it |

Only your own frame function can paint, and only on the `canvas` it was handed in that same call. Any other call paints nothing and is counted, and the base names your pack in the log with the count. A `canvas` of 0 means the frame offers none: nothing is painted and nothing is counted.

Painting costs time on the display thread, so keep an eye on your frame call in the log. `sketch` repaints up to 256 lines, the bar under its title and its cross at every frame:

```cpp
static void paint(void* canvas, u32 on_blank) {
    for (u32 i = 0; i < state.count; ++i) {
        const Line* l = &state.lines[(state.first + i) & (LINES_MOST - 1)];
        op1f_paint_line(canvas, l->x0, l->y0, l->x1, l->y1, PEN, LIGHT_GREY);
    }
    if (!on_blank && document != 0) {
        int wide = op1f_text_width(document, TITLE);
        if (wide > 0) op1f_paint_box(canvas, TITLE_MIDDLE - (wide >> 1), BAR_Y, wide, BAR_TALL, OCHRE);
    }
    op1f_paint_mask(canvas, state.x - CROSS_HALF, state.y - CROSS_HALF, CROSS, CROSS, CROSS_COVER, CROSS, LIGHT_BLUE);
}
```

Its cross is an 11 by 11 block of cover bytes with soft edges, painted in the field palette's light blue.

## Text, placed and measured

Level 3's `op1f_text` sets a text left, at its placeholder. Level 4 adds three helpers, all from your frame function only.

| Helper | What it does |
| --- | --- |
| `op1f_text_set(document, part, text, width, align)` | `op1f_text` with an alignment: `OP1F_LEFT` (the text starts at the placeholder's left), `OP1F_CENTRE` (centred on it) or `OP1F_RIGHT` (it ends at its right) |
| `op1f_text_width(document, part)` | How wide the part's text is, in pixels, as it was laid out. 0 for an empty text, -1 when no text of that part was made |
| `op1f_text_node(document, part)` | The text as a part, so `op1f_move` can move it and `op1f_show` can hide it |

A part keeps the alignment of its first use. Set it again with another alignment and the text changes, the alignment doesn't, and the log says so. If you want one part in two alignments, give your picture two placeholders.

A base of level 4 has room for 512 texts for all packs together, and 256 for one pack.

`sketch` centres its title and measures it, so the ochre bar under it is as wide as the words:

```cpp
u32 at = put(state.title, 0, TITLE_ROOM, "sketch ");
put_number(state.title, at, TITLE_ROOM, state.count);
op1f_text_set(d, TITLE, state.title, 0, OP1F_CENTRE);
```

## Deleting a file

| Helper | What it does |
| --- | --- |
| `op1f_remove(card, folder, name)` | A file of one of your folders taken off the disk. Answers 0 when it's gone |

It answers `OP1F_NO_FILE` when there was no such file, and `OP1F_NOT_NOW` when the call doesn't come from your frame function or while the disk mode is open. Like level 3's other file helpers, it's your frame function's alone. If the disk mode is open, wait for `disk` to go back to 0 and try again. `sketch` does exactly that when the board is wiped:

```cpp
static void remove_saved() {
    int answer = op1f_remove(&op1f_card_sketch, FOLDER, SAVED);
    if (answer == OP1F_NOT_NOW) {
        ++state.waits;
        return;
    }
    state.to_remove = 0;
    ...
}
```

## Watching a key

At level 2 an input function either takes an event or lets it go. At level 4 it has a third answer, **`OP1F_SEEN`**. The event goes on to the other packs and to the firmware as if you'd answered 0, and you get told of that key's up as well, before the up goes where its down went. For a turn, `OP1F_SEEN` is the same as 0.

That's how you show something while a key is held and still let the key do its normal job. A sequencer pack, for instance, can watch the octave keys and let the unit move the octave ([level 5](level-5.md)).

A base of level 3 takes any answer that isn't 0 as "taken", so a pack that answers `OP1F_SEEN` must need `op1f_level_4`. It does when its card says level 4, so there's nothing to add.

## The thread an event came on

What an input function is handed says which thread it's on, in `thread`:

| `thread` | Which events | What your function may do |
| --- | --- | --- |
| `OP1F_KEY_THREAD` | Every turn, and every key outside 13 to 53: the keys under the screen, the mode keys, the help and metronome keys, the knobs' presses and clicks | What the mode's own code does on the key thread |
| 0 | The keys 13 to 53: the keys that play notes, and shift, which is 38 | **Only what the audio thread may do**: no line in the log, no waiting, nothing of the firmware's, back quickly. The firmware may hand these in on the audio thread |

Get this wrong and the sound clicks or the unit stops. When in doubt, treat the call as if it were on the audio thread: count, and let your frame function do the rest.

## The lines the base writes

| Line begins | What it says |
| --- | --- |
| `page: '<pack>' covered by '<pack>' in mode <n>` | Your page lies under another one. It shows again when that one closes |
| `page: '<pack>' dropped from under the pages of mode <n>` | More than four pages lay under the shown one, and yours was the lowest |
| `page: '<pack>' shown again in mode <n>` | Yours is back on top |
| `page: the base's blank page made from blank.svg` | Once, at the first page that needed it |
| `text: '<pack>' made a text of the part <part> of its picture` | A placeholder became a text |
| `text: '<pack>' asked the part <part> aligned <how>: it keeps <how>` | A second alignment for a part, which keeps its first |
| `text: '<pack>' asked for the part <part>: <why>` | A text the base didn't make, and why, for instance `its 256 texts are taken` |
| `input: '<pack>' watched <n> down, told <n> up since the start` | Your `OP1F_SEEN` answers |
| `paint: '<pack>' painted <n> time(s) this period, <n> refused` | Only when something was refused: a paint call from outside your frame function, or with a canvas that isn't this frame's |

## Limits, in one place

| | |
| --- | --- |
| A card | 64 bytes, up to 16 asks of at most 4,096 bytes each |
| Pages under the shown one | 4 |
| The picture you paint on | 480 by 224 pixels, sixteen bits a pixel |
| A line's pen | 1 to 16 pixels |
| The base's font | 5 by 7 pixels, scale 1 to 8, up to 128 characters a call |
| A block of pixels or cover | Up to 480 pixels wide |
| Texts | 512 for all packs, 256 for one |

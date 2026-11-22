// Made for OS 1.7.3.
// Step R8, the base pack (base.pack 31): options. A unit of its own; no unit of base 30
// changes but the cards unit, which calls this one's frame work and counts its kind of ask as known.
//
// A card of level 6 or later may carry one ask of kind OP1F_ASK_OPTIONS (module/build_r6e_card.h, "Base 31"): up to
// eight options, each a switch or a list of up to eight choices, and where each starts. This unit
//
//   keeps the values for the whole unit in user/pf/.options, one line `<pack>/<option>=<choice>` each, read at the
//     first frame and written when the page closes with a value changed. Lines of packs not on the unit this start
//     are written back as they were, so a pack taken off and put back finds its values again;
//   answers op1f_option(card, i) from any thread: the byte of the value, or the option's `first` before the file was
//     read;
//   adds a row "packs" to the unit's own settings list. One entry of the base's table (profile group "base") puts a
//     jump at the entry of the function the settings controller adds a row with, 0x8c0d57a8 (the controller, a row's
//     controller). The firmware adds its six rows at the start, before the first frame; ours lets each through the
//     trampoline and, behind the sixth (the row whose controller's table is 0x84013740), adds a row of ours: a
//     controller of one word, its table of eight functions, and a screen of two words, its table of two functions and a
//     node the firmware's own node maker makes (0x8d4e63bc, 204 bytes, then 0x8fc80288), as the firmware's battery
//     row makes its own. What the settings code asks of a row, where our code meets it:
//       the controller's table: +0x00 a turn of the second to fourth knob (the controller, the knob, the amount),
//       +0x08 a knob's click while the row is chosen (the controller, the knob 0 to 3), +0x10 the row is chosen (also
//       once as it is added), +0x14 its screen; +0x04 and +0x0c are called around the keys under the screen and are
//       nothing here. The screen's table: +0x00 the row's name (a text that lives on), +0x04 draw (the screen, the
//       firmware's drawing context): nothing is drawn here; it notes that the row is the one on the screen this frame.
//       The screen's second word is a node of the firmware's whose word at +0x44 the keys under the screen set bits in.
//       The settings screen shows nine rows; the firmware fills six;
//   opens its page at a click of any knob while the row is chosen: the base's top page (the cards unit's
//     op1f_top_open, with the base's blank page) takes every key and knob while it is up; this unit paints it at its
//     frame work, straight into the frame's buffer, in OP-1 line art (module/build_r8_line_art.h): the first knob
//     moves from option to option, the second knob or a click changes the one chosen (a switch toggles, a list goes to
//     its next choice), the help key, T1 or a mode key leaves. Words and shapes carry every state (ON / OFF in a filled
//     or hollow box, the choice's word, a ring for the cursor), no colour alone. While the row is chosen and the page is
//     not up, a line under the settings' own says how to open it;
//   tells the pack: `changed(option, value)` on the display thread, at the frame the change is taken, before any frame
//     function.
//
// The firmware's calls: the log 0x8c053a20, the size 0x8c051f9c, open 0x8fcba988, read 0x8fcba954, close 0x8fcba8ac
// (module/build_r2_folders.cpp says how each was read), the allocator 0x8d4e63bc and the node maker 0x8fc80288 (read in
// the battery row's screen getter), and the word [0x0801b804], this frame's buffer (the cards unit's FRAME_BUFFER).
// The write is the folders unit's op1f_folders_write_base. Freestanding: no library, no division but by constants that
// are powers of two. Compiled for the PC as it is, with OP1F_HOST defined, for its test
// (tools/tests/host/options_test.cpp, its stand-ins in tools/tests/host/mock_options_r8.h).

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*FileSizeFn)(const char* path);
typedef int (*OpenFn)(const char* path, const char* mode);
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);
typedef void (*CloseFn)(int handle);

#ifdef OP1F_HOST
#include "mock_options_r8.h"
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)
#define STOCK_READ ((ReadFn)0x8fcba954u)
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)
#define RD32(a) (*(volatile u32*)(a))
#define BARRIER() asm volatile("" : : : "memory")
#endif

static inline void px(u16* fb, int x, int y, u16 c) {
    if ((u32)x < 480u && (u32)y < 224u) fb[y + ((479 - x) << 8)] = c;
}
#include "build_r8_line_art.h"

enum { MOST = OP1F_CARDS_MOST, OPTIONS = OP1F_OPTIONS_MOST, TABLE_BYTES = 708, TEXT_MOST = 4096, STEP = 1024,
       KEY_HELP = 0, KEY_T1 = 6, KEY_MODE_FIRST = 2, KEY_MODE_LAST = 5, CLICK_FIRST = 54, CLICK_LAST = 57,
       ROWS_SHOWN = 6, ROW_TALL = 26, ROWS_Y = 40 };
static const u32 FRAME_BUFFER = 0x0801b804u;
static const char LOG_FILE[] = "../src/main/op1f_options.cpp";
static const char PATH[] = "user/pf/.options";
static const char READ_MODE[] = "rb";
static const char BLANK[] = "blank.svg";

extern "C" Op1fCards op1f_cards OP1F_NAMED("op1f_cards");
extern "C" u32 op1f_cards_used() OP1F_NAMED("op1f_cards_used");
extern "C" void op1f_top_open(void* document, int (*keys)(const Op1fInput* e)) OP1F_NAMED("op1f_top_open");
extern "C" void op1f_top_close() OP1F_NAMED("op1f_top_close");
extern "C" void* op1f_base_picture(const char* name) OP1F_NAMED("op1f_base_picture");
extern "C" int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) OP1F_NAMED("op1f_folders_write_base");

extern "C" const u32 op1f_options_1 OP1F_NAMED("op1f_options_1");
const u32 op1f_options_1 = 1;

static u8 value[MOST][OPTIONS];                        // the display thread writes, any thread reads
static volatile u32 read_done;
static char text[TEXT_MOST + 1];                       // the file as read: lines of packs not here are written back
static u32 text_bytes;
static u32 dirty;

// The keys while the page is up, from the key thread: counts only it writes; the frame keeps how far it took them.
static volatile u32 moves_up, moves_down, changes_up, changes_down, closes, opens;
static u32 moves_up_taken, moves_down_taken, changes_up_taken, changes_down_taken, closes_taken, opens_taken;
static int turned[2];                                  // the key thread's: a knob's turn short of a step
static volatile u32 row_drawn;                         // the settings draw our row's screen: this frame's
static u32 page_up, cursor, first_shown;
static void* blank;
static u32 row_added, row_node_failed;

// A card's ask of this kind, or 0: a card of level 6 or later, of level 4's size, a table of this kind's bytes with 1
// to 8 options, each in order (a switch of two choices, a list of 2 to 8, a first below its count).
static const Op1fOptions* ask_of(const Op1fCard* card) {
    if (card == 0 || card->bytes < (u32)OP1F_CARD_LEVEL_6_BYTES || card->level < 6u || card->asks == 0) return 0;
    for (u32 i = 0; i < card->ask_count && i < (u32)OP1F_ASKS_MOST; ++i) {
        const Op1fAsk* a = &card->asks[i];
        if (a->kind != (u32)OP1F_ASK_OPTIONS || a->bytes < (u32)TABLE_BYTES || a->what == 0) continue;
        const Op1fOptions* o = (const Op1fOptions*)a->what;
        if (o->count == 0 || o->count > (u32)OPTIONS) return 0;
        for (u32 n = 0; n < o->count; ++n) {
            const Op1fOption* p = &o->options[n];
            if (p->kind > 1u || p->count < 2u || p->count > (u32)OP1F_CHOICES_MOST || (p->kind == 0u && p->count != 2u) || p->first >= p->count) return 0;
        }
        return o;
    }
    return 0;
}

static int slot_of(const Op1fCard* card) {
    u32 seen = op1f_cards.count > (u32)MOST ? (u32)MOST : op1f_cards.count;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards.slots[i].card == card) return (int)i;
    return -1;
}

extern "C" int op1f_option(const Op1fCard* card, unsigned option) {
    const Op1fOptions* o = ask_of(card);
    int slot = slot_of(card);
    if (o == 0 || slot < 0 || option >= o->count) return OP1F_BAD_NAME;
    return read_done ? (int)value[slot][option] : (int)o->options[option].first;
}

// ---------------------------------------------------------------- the file

static int same_to(const char* a, const char* b, u32 n) {       // the first n bytes of a are b, whole
    u32 i = 0;
    for (; i < n && b[i] && a[i] == b[i]; ++i) {}
    return i == n && b[i] == 0;
}

// The choice the file holds for <pack>/<option>, or -1.
static int kept(const char* pack, const char* option) {
    for (u32 at = 0; at < text_bytes;) {
        u32 end = at, slash = 0, equal = 0;
        while (end < text_bytes && text[end] != '\n') {
            if (text[end] == '/' && slash == 0) slash = end;
            if (text[end] == '=') equal = end;
            ++end;
        }
        if (slash > at && equal > slash + 1 && equal + 1 < end && same_to(text + at, pack, slash - at) && same_to(text + slash + 1, option, equal - slash - 1)) {
            int v = 0;
            for (u32 i = equal + 1; i < end; ++i) {
                if (text[i] < '0' || text[i] > '9' || v > 99) return -1;
                v = v * 10 + (text[i] - '0');
            }
            return v;
        }
        at = end + 1;
    }
    return -1;
}

static void read_file() {
    text_bytes = 0;
    int size = STOCK_FILE_SIZE(PATH);
    if (size > 0 && size <= (int)TEXT_MOST) {
        int h = STOCK_OPEN(PATH, READ_MODE);
        if (h >= 0) {
            int got = STOCK_READ(text, 1, (u32)size, h);
            STOCK_CLOSE(h);
            if (got == size) text_bytes = (u32)size;
        }
    }
    text[text_bytes] = 0;
    u32 used = op1f_cards_used(), packs = 0, found = 0;
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const Op1fOptions* o = (used >> slot) & 1u ? ask_of(op1f_cards.slots[slot].card) : 0;
        if (o == 0) continue;
        ++packs;
        for (u32 n = 0; n < o->count; ++n) {
            int v = kept(op1f_cards.slots[slot].card->name, o->options[n].name);
            if (v >= 0 && v < (int)o->options[n].count) ++found;
            value[slot][n] = (u8)(v >= 0 && v < (int)o->options[n].count ? v : o->options[n].first);
        }
    }
    BARRIER();
    read_done = 1;
    STOCK_LOG(3, LOG_FILE, 1, "options: %u pack(s) with options, %u value(s) read from %s (%d bytes); the settings row %s",
              packs, found, PATH, size, row_added ? "is in the list" : row_node_failed ? "could NOT be made" : "is not in the list");
}

static u32 put(char* out, u32 at, const char* s) {
    while (*s && at < (u32)TEXT_MOST) out[at++] = *s++;
    return at;
}

static int here(const char* pack, u32 n) {                      // a pack of that name (its first n bytes) has options now
    u32 used = op1f_cards_used();
    for (u32 slot = 0; slot < (u32)MOST; ++slot)
        if (((used >> slot) & 1u) && ask_of(op1f_cards.slots[slot].card) && same_to(pack, op1f_cards.slots[slot].card->name, n)) return 1;
    return 0;
}

static void write_file() {
    static char out[TEXT_MOST + 1];
    u32 at = 0, used = op1f_cards_used();
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const Op1fOptions* o = (used >> slot) & 1u ? ask_of(op1f_cards.slots[slot].card) : 0;
        for (u32 n = 0; o && n < o->count; ++n) {
            char digits[4] = { (char)('0' + value[slot][n]), '\n', 0, 0 };
            at = put(out, at, op1f_cards.slots[slot].card->name);
            at = put(out, at, "/");
            at = put(out, at, o->options[n].name);
            at = put(out, at, "=");
            at = put(out, at, digits);
        }
    }
    for (u32 from = 0; from < text_bytes;) {                    // the lines of packs not here, as they were
        u32 end = from, slash = 0;
        while (end < text_bytes && text[end] != '\n') { if (text[end] == '/' && slash == 0) slash = end; ++end; }
        if (slash > from && !here(text + from, slash - from))
            for (u32 i = from; i <= end && i < text_bytes && at < (u32)TEXT_MOST; ++i) out[at++] = text[i];
        from = end + 1;
    }
    if (at && out[at - 1] != '\n' && at < (u32)TEXT_MOST) out[at++] = '\n';
    int answer = op1f_folders_write_base(PATH, out, at);
    if (answer == OP1F_NOT_NOW) return;                          // the disk mode: at a later frame
    dirty = 0;
    if (answer == (int)at) {
        for (u32 i = 0; i < at; ++i) text[i] = out[i];
        text_bytes = at;
        text[at] = 0;
    }
    STOCK_LOG(answer == (int)at ? 3 : 1, LOG_FILE, 2, "options: %s %s, %u bytes (%d)", answer == (int)at ? "wrote" : "could NOT write", PATH, at, answer);
}

// ---------------------------------------------------------------- the page

struct Row { u8 slot, option; };                                  // option 0xff: the pack's heading
static Row rows[MOST * (OPTIONS + 1)];
static u32 row_count;

static void list_rows() {
    row_count = 0;
    u32 used = op1f_cards_used();
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const Op1fOptions* o = (used >> slot) & 1u ? ask_of(op1f_cards.slots[slot].card) : 0;
        if (o == 0) continue;
        rows[row_count].slot = (u8)slot; rows[row_count++].option = 0xff;
        for (u32 n = 0; n < o->count; ++n) { rows[row_count].slot = (u8)slot; rows[row_count++].option = (u8)n; }
    }
}

static int options_keys(const Op1fInput* e) {                    // the top page's: the key thread
    if (e->what == (u32)OP1F_TURN && e->key < 2u) {
        int t = turned[e->key] + e->turn;
        while (t >= (int)STEP) { t -= STEP; if (e->key == 0) moves_down = moves_down + 1; else changes_up = changes_up + 1; }
        while (t <= -(int)STEP) { t += STEP; if (e->key == 0) moves_up = moves_up + 1; else changes_down = changes_down + 1; }
        turned[e->key] = t;
        return 1;
    }
    if (e->what != (u32)OP1F_DOWN && e->what != (u32)OP1F_DOWN_SHIFT) return 1;
    if (e->key >= (u32)CLICK_FIRST && e->key <= (u32)CLICK_LAST) changes_up = changes_up + 1;
    else if (e->key == (u32)KEY_HELP || e->key == (u32)KEY_T1 || (e->key >= (u32)KEY_MODE_FIRST && e->key <= (u32)KEY_MODE_LAST)) closes = closes + 1;
    return 1;
}

static u32 next_option_row(u32 from, int way) {                  // the nearest row of an option from `from` that way, or from
    for (int r = (int)from + way; r >= 0 && r < (int)row_count; r += way)
        if (rows[r].option != 0xff) return (u32)r;
    return from;
}

static void change(int way) {
    if (cursor >= row_count || rows[cursor].option == 0xff) return;
    const Row* r = &rows[cursor];
    const Op1fOptions* o = ask_of(op1f_cards.slots[r->slot].card);
    if (o == 0) return;
    u32 count = o->options[r->option].count, v = value[r->slot][r->option];
    v = way > 0 ? (v + 1u == count ? 0u : v + 1u) : (v == 0u ? count - 1u : v - 1u);
    value[r->slot][r->option] = (u8)v;
    dirty = 1;
    if (o->changed) o->changed(r->option, v);
}

static const u16 BLACK = 0x0000, WHITE = 0xffff;

// Text in capitals: the stroke font keeps a lowercase b for the flat sign.
static const char* caps(const char* s, char* out, u32 room) {
    u32 n = 0;
    for (; s[n] && n + 1 < room; ++n) out[n] = s[n] >= 'a' && s[n] <= 'z' ? (char)(s[n] - 32) : s[n];
    out[n] = 0;
    return out;
}

static void paint_row(u16* fb, u32 r, int y, int chosen) {
    const Row* row = &rows[r];
    const Op1fCard* card = op1f_cards.slots[row->slot].card;
    char big[OP1F_OPTION_NAME + 8];
    if (row->option == 0xff) {                                   // the pack's heading: its name, small, and a rule
        caps(card->name, big, sizeof big);
        vtext(fb, 16, y + 8, big, 6, LAV, 1);
        ln(fb, 24 + vtext_w(big, 6), y + 12, 464, y + 12, DIMG, 1);
        return;
    }
    const Op1fOption* p = &ask_of(card)->options[row->option];
    u32 v = value[row->slot][row->option];
    if (chosen) { rbox(fb, 8, y, 472, y + ROW_TALL - 4, KC[0], 2); vtext(fb, 16, y + 5, ">", 8, KC[0], 2); }
    vtext(fb, 36, y + 5, caps(p->name, big, sizeof big), 8, WHITE, 2);
    if (p->kind == (u8)OP1F_OPTION_SWITCH) {                     // OFF and ON, the value's word in a filled box, the other hollow and dim
        static const char* const WORDS[2] = { "OFF", "ON" };
        for (u32 k = 0; k < 2u; ++k) {
            int x0 = k ? 400 : 330, x1 = k ? 460 : 390;
            int w = vtext_w(WORDS[k], 8);
            if (k == v) {
                for (int x = x0 + 2; x <= x1 - 2; ++x) for (int yy = y + 2; yy <= y + ROW_TALL - 6; ++yy) px(fb, x, yy, k ? KC[1] : LAV);
                vtext(fb, (x0 + x1 - w) >> 1, y + 5, WORDS[k], 8, BLACK, 2);
            } else {
                rbox(fb, x0, y + 1, x1, y + ROW_TALL - 5, DIMG, 1);
                vtext(fb, (x0 + x1 - w) >> 1, y + 5, WORDS[k], 8, DIMG, 2);
            }
        }
        return;
    }
    const char* word = p->choices[v];                            // a list: < word >, and a dot a choice, the value's filled
    char shown[OP1F_CHOICE_NAME + 1];
    u32 n = 0;
    for (; n < (u32)OP1F_CHOICE_NAME && word[n]; ++n) shown[n] = word[n];
    shown[n] = 0;
    caps(shown, shown, sizeof shown);
    int w = vtext_w(shown, 8);
    vtext(fb, 330, y + 5, "<", 8, KC[1], 2);
    vtext(fb, 452, y + 5, ">", 8, KC[1], 2);
    vtext(fb, (346 + 450 - w) >> 1, y + 3, shown, 8, WHITE, 2);
    int dots = 4 + (int)p->count * 8, x = (346 + 450 - dots) >> 1;
    for (u32 k = 0; k < p->count; ++k, x += 8) {
        if (k == v) dotw(fb, x + 2, y + 18, 3, KC[1]);
        else px(fb, x + 3, y + 19, DIMG);
    }
}

static void paint_page(u16* fb) {
    for (int x = 0; x < 480; ++x) for (int y = 0; y < 224; ++y) fb[y + ((479 - x) << 8)] = BLACK;
    vtext(fb, 16, 10, "PACK OPTIONS", 10, LAV, 2);
    ln(fb, 16, 32, 464, 32, LAV, 1);
    if (row_count == 0) {
        vtext(fb, 16, 96, "NO PACK HAS OPTIONS", 8, DIMG, 2);
    } else {
        if (cursor < first_shown + 1u && cursor > 0u) first_shown = cursor - 1u;               // the heading above stays in sight
        if (cursor < first_shown) first_shown = cursor;
        if (cursor >= first_shown + (u32)ROWS_SHOWN) first_shown = cursor + 1u - (u32)ROWS_SHOWN;
        for (u32 i = 0; i < (u32)ROWS_SHOWN && first_shown + i < row_count; ++i)
            paint_row(fb, first_shown + i, ROWS_Y + (int)i * ROW_TALL, first_shown + i == cursor);
        if (first_shown > 0u) vtext(fb, 466, 36, "^", 6, LAV, 1);
        if (first_shown + (u32)ROWS_SHOWN < row_count) vtext(fb, 466, 192, "V", 6, LAV, 1);
    }
    // the knobs' words at the foot: a ring in the knob's colour and the word
    arc(fb, 22, 212, 5, 0, 256, KC[0], 2);
    vtext(fb, 32, 206, "MOVE", 6, KC[0], 1);
    arc(fb, 102, 212, 5, 0, 256, KC[1], 2);
    vtext(fb, 112, 206, "CHANGE", 6, KC[1], 1);
    vtext(fb, 360, 206, "HELP BACK", 6, LAV, 1);
}

// While our row is the one the settings show and the page is not up: how to open it, under the settings' own words.
static void paint_hint(u16* fb) {
    vtext(fb, 200, 180, "CLICK A KNOB", 6, LAV, 1);
    vtext(fb, 200, 194, "FOR PACK OPTIONS", 6, LAV, 1);
}

static void open_page() {
    if (blank == 0) blank = op1f_base_picture(BLANK);
    if (blank == 0) { STOCK_LOG(1, LOG_FILE, 3, "options: the page could NOT be opened: no blank page"); return; }
    list_rows();
    cursor = row_count ? next_option_row(0, 1) : 0u;
    first_shown = 0;
    moves_up_taken = moves_up; moves_down_taken = moves_down; changes_up_taken = changes_up; changes_down_taken = changes_down; closes_taken = closes;
    op1f_top_open(blank, options_keys);
    page_up = 1;
    STOCK_LOG(3, LOG_FILE, 4, "options: the page is up, %u row(s)", row_count);
}

static void close_page() {
    op1f_top_close();
    page_up = 0;
    if (dirty) write_file();
    STOCK_LOG(3, LOG_FILE, 5, "options: the page is closed");
}

// Once a frame, on the display thread, before any frame function (the cards unit's frame work, behind the values' unit's).
extern "C" void op1f_options_frame() OP1F_NAMED("op1f_options_frame");
extern "C" void op1f_options_frame() {
    if (!read_done) read_file();
    if (dirty && !page_up) write_file();                       // a write the disk mode held back
    u32 asked = opens;
    if (asked != opens_taken) { opens_taken = asked; if (!page_up) open_page(); }
    u16* fb = (u16*)RD32(FRAME_BUFFER);
    if (!page_up) {
        if (row_drawn && fb) paint_hint(fb);
        row_drawn = 0;
        return;
    }
    row_drawn = 0;
    u32 c = closes;
    if (c != closes_taken) { closes_taken = c; close_page(); return; }
    for (u32 n = moves_down; moves_down_taken != n; ++moves_down_taken) cursor = next_option_row(cursor, 1);
    for (u32 n = moves_up; moves_up_taken != n; ++moves_up_taken) cursor = next_option_row(cursor, -1);
    for (u32 n = changes_up; changes_up_taken != n; ++changes_up_taken) change(1);
    for (u32 n = changes_down; changes_down_taken != n; ++changes_down_taken) change(-1);
    if (fb) paint_page(fb);
}

// ---------------------------------------------------------------- the row in the unit's settings

static const char ROW_NAME[] = "packs";
static void row_nothing(void*) {}
static void row_click(void*, u32) { opens = opens + 1; }        // the key thread: a knob's click while our row is chosen
struct RowScreen { const void* const* table; void* node; };
static RowScreen row_screen;
static void* row_screen_of(void*) { return &row_screen; }
static const char* row_name(void*) { return ROW_NAME; }
static void row_draw(void*, void*) { row_drawn = 1; }           // the display thread, in the settings' draw
static const void* const ROW_SCREEN_TABLE[2] = { (const void*)row_name, (const void*)row_draw };
static const void* const ROW_TABLE[8] = { (const void*)row_nothing, (const void*)row_nothing, (const void*)row_click, (const void*)row_nothing,
                                          (const void*)row_nothing, (const void*)row_screen_of, (const void*)row_nothing, (const void*)row_nothing };
struct RowController { const void* const* table; };
static RowController row_controller = { ROW_TABLE };

#ifndef OP1F_HOST
typedef void* (*NewFn)(u32 bytes);
typedef void (*NodeFn)(void* node);
typedef void (*AddFn)(void* owner, void* controller);
static const u32 LAST_STOCK_ROW = 0x84013740u;                   // the table of the sixth row's controller the firmware adds
static const u32 NODE_BYTES = 204;
// The room (zeroed data): the eight bytes the jump displaced, copied there by the loader with a jump back behind them.
extern "C" u32 op1f_options_add_room[4] OP1F_NAMED("op1f_options_add_room");
u32 op1f_options_add_room[4];

// At the start, on the main thread, before the first frame: each of the firmware's rows through the trampoline; ours
// behind the sixth. No line here: read_file says it.
extern "C" void op1f_options_add(void* owner, void* controller) OP1F_NAMED("op1f_options_add");
extern "C" void op1f_options_add(void* owner, void* controller) {
    ((AddFn)(void*)op1f_options_add_room)(owner, controller);
    if (row_added || controller == 0 || *(const u32*)controller != LAST_STOCK_ROW) return;
    void* node = ((NewFn)0x8d4e63bcu)(NODE_BYTES);
    if (node == 0) { row_node_failed = 1; return; }
    ((NodeFn)0x8fc80288u)(node);
    row_screen.table = ROW_SCREEN_TABLE;
    row_screen.node = node;
    row_added = 1;
    ((AddFn)(void*)op1f_options_add_room)(owner, &row_controller);
}
#else
// The test's ways in: the settings' calls of our row.
static const char* options_row_name() { return ((const char* (*)(void*))ROW_SCREEN_TABLE[0])(&row_screen); }
static void options_row_click() { ((void (*)(void*, u32))ROW_TABLE[2])(&row_controller, 0); }
static void options_row_draw() { ((void (*)(void*, void*))ROW_SCREEN_TABLE[1])(&row_screen, 0); }
#endif

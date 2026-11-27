// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R8, the base pack (base.pack 32): options, first written by hasis (open-field pull request 20, for OS 1.7.3 on
// base 30), reworked for base 32 to the maintainer's design: a pack's options live in the unit's own settings, drawn and
// turned as the unit's own items are. A unit of its own; no unit of base 31 changes but the cards unit, which calls this
// one's frame work and counts its kind of ask as known.
//
// Base 44 (hasis): options of words (module/build_r6e_card.h, "Base 44"), behind the table's count where a base 43
// does not read: their words from the pack's function at run time (the display thread only), up to 32; the file keeps
// the chosen word, and each frame a word kept that is not the value's is looked for among the words (found: its
// number taken, `changed` told). They show on their pack's page under PACKS only.
//
// A card of level 6 or later may carry one ask of kind OP1F_ASK_OPTIONS (module/build_r6e_card.h, "Base 32"). This unit
//
//   keeps the values for the whole unit in user/pf/.options, one line `<pack>/<option>=<value>` each (the value a
//     switch's 0 or 1, a list's word from 0, a number's distance from its low), read at the first frame (lines that
//     end in CR LF too) and written about a second and a half after the last change. Lines of packs not on the unit
//     this start are written back as they were. A file over 4,096 bytes is neither read nor written over;
//   answers op1f_option(card, i) from any thread: a position, 0 to 255, for every kind (a number its distance from its
//     low), so that no real option's answer is below 0 where the error codes are;
//   takes op1f_option_set(card, i, position) from the pack's own frame function only (a pack with `open` sets its
//     options from its own page with it): the value kept and the file written as after a change in the settings, a
//     category's line that shows it written again while the settings show that category, `changed` not told;
//   places each pack's options at the first frame, in the order of the packs' file names: an option of KEYBOARD, SYSTEM
//     or MIDI after the unit's own lines while the category has room (seven lines in all: 4, 1 and 2 of ours), any
//     other on its pack's page under PACKS (one line on the log for a pack whose category was full);
//   adds a row "packs" to the unit's own settings list. One entry of the base's table puts a jump at the entry of the
//     function the settings controller adds a row with, 0x8c0d57a8 (the settings controller, a row's controller). The
//     firmware adds its six rows at the start, before the first frame, and never again; ours lets each through the
//     trampoline and, behind the sixth (the last, "monitor": the row whose controller's table is 0x84013740), adds a
//     row of ours: a controller of one
//     word and its table, and a screen the unit's own category screen maker makes (0x8fc64c68 on 0x2c bytes of the
//     allocator 0x8d4e63bc, named "packs"), as KEYBOARD, SYSTEM and MIDI make theirs. Its table is ours: its name, and
//     a draw that puts our lines into the shared seven names and values (0x8fc64c2c, 0x8fc64c08) and sets the count
//     [screen + 0xc] and the chosen line [screen + 8] before it calls the unit's own draw (0x8fc64de0), so the right
//     side is the unit's own letters and colours. It writes the shared seven only while the settings' chosen row
//     (0x84c9b928, an index into the list of the rows' screens 0x84c9b97c) is ours, and reads it again behind them:
//     moved on meanwhile (a fast blue turn: the next row's chosen runs on the key thread), it does not draw, and that
//     row's chosen fills the lines. What the settings ask of a row of ours, by the words of its table:
//     +0x00 a turn of knobs 1 to 3 (the controller, the knob, the amount: the knob's 16-bit count over
//     32768 as a float), +0x04 a release of T1 to T4 (0 to 3), +0x08 a knob's click at its release (0 to 3), +0x0c
//     for every row from the settings controller's own, +0x10 the row is chosen (also once as it is added), +0x14 its
//     screen. Under PACKS: the list of packs with options on their page or a page of their own; the ochre knob picks a
//     line (four detents a line, as the unit's own picker), its click opens: `open` called at the next frame work, or
//     the pack's options in the same right side, where the ochre knob picks and the red knob changes (a switch and a
//     list four detents a step, a number one, as the unit's own items; no wrap: they stop at their ends); T1 back to the
//     list (see op1f_options_down), and the row chosen again shows the list. More than seven lines: the seven around
//     the chosen one;
//   offers six functions that another entry of the table puts into the data words +0x10 (chosen) and +0x18 (the red
//     knob on a line) of the KEYBOARD, SYSTEM and MIDI rows' tables: chosen calls the unit's own (0x8c0d44b0,
//     0x8c0d4d0c, 0x8c0d484c), then puts our lines behind its three, six or five, sets the row's count [row + 0x2c] and
//     the screen's, raises the line picker's top (the CtlKnob at row + 4, 0x8c0b68b0) and gives the picker back the
//     place the unit's own function held it from; red hands a line of the unit's to the unit's own function
//     (0x8c0d45ce, 0x8c0d5018, 0x8c0d49d0) and changes ours, writing its value again;
//   tells the pack: `changed(option, value)` on the display thread, at the frame the change is taken, before any frame
//     function.
//
// The threads: the settings call a row on the key thread. There this unit only counts (our row) or does what the
// unit's own function does in its place (the categories: a byte of ours and the line's text); the display thread does
// the rest at the frame work, and the draw fills the lines on the display thread.
//
// The firmware's calls besides: the log 0x8c053a20, the size 0x8c051f9c, open 0x8fcba988, read 0x8fcba954, close
// 0x8fcba8ac (module/build_r2_folders.cpp says how each was read). The write is the folders unit's
// op1f_folders_write_base. Freestanding: no library, no division but by powers of two (numbers to text by subtraction),
// no float. Compiled for the PC as it is, with OP1F_HOST defined, for tests this tree does not hold.

#include "build_r6e_card.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*FileSizeFn)(const char* path);
typedef int (*OpenFn)(const char* path, const char* mode);
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);
typedef void (*CloseFn)(int handle);
typedef void* (*NewFn)(u32 bytes);
typedef void (*ScreenMakeFn)(void* screen, const char* name);
typedef void (*ScreenDrawFn)(void* screen, void* context);
typedef void (*LineFn)(void* screen, u32 line, const char* text);
typedef void (*TopFn)(void* knob, u32 top);
typedef void (*ChosenFn)(void* row);
typedef void (*RedFn)(void* row, u32 line, u32 amount);       // the amount a float, handed on in its register as it came
typedef void (*AddFn)(void* owner, void* controller);

#ifdef OP1F_HOST
#include "mock_options_r8.h"
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)
#define STOCK_READ ((ReadFn)0x8fcba954u)
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)
#define STOCK_NEW ((NewFn)0x8d4e63bcu)                       // the allocator
#define STOCK_SCREEN_MAKE ((ScreenMakeFn)0x8fc64c68u)        // the category screen's maker
#define STOCK_SCREEN_DRAW ((ScreenDrawFn)0x8fc64de0u)        // its draw
#define STOCK_LINE_NAME ((LineFn)0x8fc64c2cu)                // a line's name into the shared seven
#define STOCK_LINE_VALUE ((LineFn)0x8fc64c08u)               // a line's value into the shared seven
#define STOCK_KNOB_TOP ((TopFn)0x8c0b68b0u)                  // CtlKnob's new maximum
#define STOCK_CHOSEN_KEYBOARD ((ChosenFn)0x8c0d44b0u)
#define STOCK_CHOSEN_SYSTEM ((ChosenFn)0x8c0d4d0cu)
#define STOCK_CHOSEN_MIDI ((ChosenFn)0x8c0d484cu)
#define STOCK_RED_KEYBOARD ((RedFn)0x8c0d45ceu)
#define STOCK_RED_SYSTEM ((RedFn)0x8c0d5018u)
#define STOCK_RED_MIDI ((RedFn)0x8c0d49d0u)
#define LAST_STOCK_ROW 0x84013740u                           // the table of the sixth row's controller
#define TABLE_KEYBOARD 0x840134c8u                           // the KEYBOARD, SYSTEM and MIDI rows' tables
#define TABLE_SYSTEM 0x840134a4u
#define TABLE_MIDI 0x840134ecu
#define SETTINGS_CHOSEN 0x84c9b928u                          // the settings' chosen row, an index into their list
#define SETTINGS_LIST 0x84c9b97cu                            // that list, a vector of the rows' screens: +4 first, +8 end
#define STOCK_ADD(owner, controller) ((AddFn)(void*)op1f_options_add_room)(owner, controller)
#define BARRIER() asm volatile("" : : : "memory")
// The room (zeroed data): the eight bytes the jump displaced, copied there by the loader with a jump back behind them.
extern "C" u32 op1f_options_add_room[4] OP1F_NAMED("op1f_options_add_room");
u32 op1f_options_add_room[4];
#endif
#define RD32(a) (*(volatile u32*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))

// R20 (base 42): the cards unit's second list and two of its names (module/build_r3_base.h, which this unit's card
// header does not take): the same declarations.
#ifndef OP1F_BUILD_R3_BASE_H
enum { OP1F_SLOTS_MOST = 128 };
struct Op1fCards2 { unsigned count, more; Op1fCardSlot slots[OP1F_SLOTS_MOST]; };
extern "C" Op1fCards2 op1f_cards_2 OP1F_NAMED("op1f_cards_2");
extern "C" void op1f_cards_lists() OP1F_NAMED("op1f_cards_lists");
extern "C" int op1f_cards_in_use(unsigned slot) OP1F_NAMED("op1f_cards_in_use");
#endif

enum { MOST = OP1F_SLOTS_MOST, OPTIONS = OP1F_OPTIONS_MOST, TEXT_MOST = 4096, WORDS_MOST = OP1F_WORDS_MOST, LIVE_MOST = 8, NAME = OP1F_OPTION_NAME, LINES = 7, LINE_ROOM = 32, SHOWN_MOST = 15,
       KNOB_OCHRE = 1, KNOB_RED = 3, KEY_T1 = 6, LINE_SHIFT = 2, WRITE_AFTER_MS = 1500, VALUE_MOST = 255 };
// The list row of KEYBOARD, SYSTEM and MIDI, and its line picker, a CtlKnob; the category screen.
enum { ROW_PICKER = 0x04, ROW_COUNT = 0x2c, ROW_SCREEN = 0x30, KNOB_VALUE = 0x10, KNOB_TOP = 0x18,
       SCREEN_LINE = 0x08, SCREEN_COUNT = 0x0c, SCREEN_BYTES = 0x2c };
enum { IN_PACKS = 0, IN_KEYBOARD = 1, IN_SYSTEM = 2, IN_MIDI = 3, CATEGORIES = 4, ROOM_MOST = 4 };
enum { SHOWS_LIST = 0, SHOWS_ITEMS = 1 };
static const u32 STOCK_LINES[CATEGORIES] = { 0, 3, 6, 5 };     // the unit's own lines
static const u32 ROOM[CATEGORIES] = { 0, 4, 1, 2 };            // ours behind them: seven in all
static const char LOG_FILE[] = "../src/main/op1f_options.cpp";
static const char PATH[] = "user/pf/.options";
static const char READ_MODE[] = "rb";
static const char ROW_NAME[] = "packs";

extern "C" u32 op1f_cards_calling() OP1F_NAMED("op1f_cards_calling");
extern "C" int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) OP1F_NAMED("op1f_folders_write_base");

extern "C" const u32 op1f_options_1 OP1F_NAMED("op1f_options_1");
const u32 op1f_options_1 = 1;

typedef void (*OpenPageFn)();

static struct State {
    // the display thread's, written before read_done and read by any thread after it
    volatile u32 read_done;
    const Op1fOptions* table[MOST];                     // each used card's ask, in order, or 0
    OpenPageFn open[MOST];
    u8 on_page[MOST];                                   // bit n: option n shows on its pack's page under PACKS
    u8 listed[MOST];                                    // the packs under PACKS, in order (R20: slots 0 to 127, a byte holds them)
    u32 listed_count;
    u8 cat_slot[CATEGORIES][ROOM_MOST], cat_option[CATEGORIES][ROOM_MOST];   // R20: a slot of the 128 fits a byte
    u32 cat_count[CATEGORIES];
    // the display thread's
    u32 too_big, dirty, changed_ms, text_bytes;
    u32 shows, list_raw, list_first, items_slot, items_count, items_raw, items_first, red_line;
    int red_raw;
    u8 items[OPTIONS];
    u32 open_asked;                                     // a slot + 1
    u32 chosens_taken, backs_taken, presses_taken, ochre_taken, red_taken, drawn_seen;
    u8 told[CATEGORIES][ROOM_MOST];
    // the key thread's: counts only it writes (our row), and its work on the categories' lines
    volatile u32 chosens, backs, presses, ochre_turned, red_turned;
    volatile u8 bumped[CATEGORIES][ROOM_MOST];
    int cat_raw[CATEGORIES];
    u32 cat_raw_line[CATEGORIES];                       // the line + 1 cat_raw was taken on
    // the display thread's, read by the key thread
    volatile u32 drawn, items_up;
    // the row
    void* screen;
    u32 row_added, row_failed;
    void* cat_row[CATEGORIES];                          // KEYBOARD's, SYSTEM's and MIDI's rows, noted at the start
    // base 44, the display thread's: each option of words with the word it keeps (the file's, then the owner's)
    // ponytail: eight on the unit in all; one more is not kept over a start (a line says so)
    struct { u8 slot, option; char word[NAME]; } live[LIVE_MOST];
    u32 live_count;
} s;
static u8 value[MOST][OPTIONS];                         // the display thread writes, and the key thread for the categories' lines; any reads
static char text[TEXT_MOST + 1];                        // the file as read: lines of packs not here are written back

// ---------------------------------------------------------------- an option

// Base 44: an option of words, its function (in its choices' first four bytes) and how many words it gives now.
static Op1fOptionWord word_fn(const Op1fOption* p) { return *(const Op1fOptionWord*)(const void*)p->choices; }
static u32 words_of(const Op1fOption* p) {
    Op1fOptionWord f = word_fn(p);
    u32 n = 0;
    while (n < (u32)WORDS_MOST && f(n)) ++n;
    return n;
}

static u32 top_of(const Op1fOption* p) {                // the highest value: a switch 1, a list its count - 1, a number high - low
    if (p->kind == (u8)OP1F_OPTION_WORDS) { u32 n = words_of(p); return n ? n - 1u : 0u; }      // base 44: the words now
    if (p->kind == (u8)OP1F_OPTION_NUMBER) return (u32)((int)(signed char)p->choices[0][1] - (int)(signed char)p->choices[0][0]);
    return (u32)p->count - 1u;
}

// The value v of a number as the owner sees it: its low + v. (op1f_option and `changed` hand v itself, a position from
// 0, for every kind: the pack adds its own low.)
static int number_of(const Op1fOption* p, u32 v) {
    return (int)(signed char)p->choices[0][0] + (int)v;
}

static u32 shift_of(const Op1fOption* p) {              // the detents a step, as a shift: a number one, a switch and a list four
    return p->kind == (u8)OP1F_OPTION_NUMBER ? 0u : 2u;
}

static int in_order(const Op1fOption* p) {
    u32 n = 0;
    while (n < (u32)OP1F_OPTION_NAME && p->name[n]) ++n;
    if (n == 0 || n >= (u32)OP1F_OPTION_NAME) return 0;
    if (p->kind == (u8)OP1F_OPTION_SWITCH) return p->count == 2u && p->first < 2u;
    if (p->kind == (u8)OP1F_OPTION_LIST) return p->count >= 2u && p->count <= (u32)OP1F_CHOICES_MOST && p->first < p->count;
    if (p->kind == (u8)OP1F_OPTION_NUMBER) {
        int low = (signed char)p->choices[0][0], high = (signed char)p->choices[0][1];
        return p->count == 0u && low < high && (int)p->first <= high - low;
    }
    return 0;
}

static int pack_function(const Op1fCardSlot* slot, const void* f) {     // 0, or a function of the pack
    u32 a = (u32)f;
    return a == 0 || (a >= slot->first && a < slot->end && (a & 1u) == 0);
}

// Base 44: the options of a table, `count` and the options of words behind them (a base 43 reads only `count`).
static u32 all_of(const Op1fOptions* o) {
    u32 n = o->count;
    while (n < (u32)OPTIONS && o->options[n].kind == (u8)OP1F_OPTION_WORDS) ++n;
    return n;
}

static int words_in_order(const Op1fCardSlot* slot, const Op1fOption* p) {
    u32 n = 0;
    while (n < (u32)OP1F_OPTION_NAME && p->name[n]) ++n;
    const void* f = (const void*)word_fn(p);
    return n > 0 && n < (u32)OP1F_OPTION_NAME && p->count == 0u && p->first < (u32)WORDS_MOST && f != 0 && pack_function(slot, f);
}

// A slot's ask of this kind, in order, or 0 (and why, for a line): a card of level 6 or later, of level 4's size, its
// first ask of this kind with a table of at least 708 bytes, 1 to 8 options each in order, `changed` and `open` the
// pack's. Reads only: from any thread.
static const Op1fOptions* table_of(u32 slot, const char** why, OpenPageFn* open) {
    const Op1fCardSlot* place = &op1f_cards_2.slots[slot];
    const Op1fCard* card = place->card;
    if (card == 0 || card->bytes < (u32)OP1F_CARD_LEVEL_6_BYTES || card->level < 6u || card->asks == 0) return 0;
    for (u32 i = 0; i < card->ask_count && i < (u32)OP1F_ASKS_MOST; ++i) {
        const Op1fAsk* a = &card->asks[i];
        if (a->kind != (u32)OP1F_ASK_OPTIONS || a->bytes < (u32)OP1F_OPTIONS_BYTES_FIRST || a->what == 0) continue;
        const Op1fOptions* o = (const Op1fOptions*)a->what;
        OpenPageFn f = a->bytes >= (u32)OP1F_OPTIONS_BYTES ? o->open : 0;
        const char* fault = 0;
        if (o->count > (u32)OPTIONS || all_of(o) == 0) fault = "it lists no option, or more than eight";     // base 44: words only is fine
        for (u32 n = 0; fault == 0 && n < o->count; ++n)
            if (!in_order(&o->options[n])) fault = "an option is not in order";
        for (u32 n = o->count; fault == 0 && n < all_of(o); ++n)                // base 44
            if (!words_in_order(place, &o->options[n])) fault = "an option of words is not in order";
        if (fault == 0 && !pack_function(place, (const void*)o->changed)) fault = "its changed is not a function of the pack";
        if (fault == 0 && !pack_function(place, (const void*)f)) fault = "its open is not a function of the pack";
        if (fault) { if (why) *why = fault; return 0; }
        if (open) *open = f;
        return o;
    }
    return 0;
}

static int slot_of(const Op1fCard* card) {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)MOST ? (u32)MOST : op1f_cards_2.count;
    for (u32 i = 0; i < seen; ++i)
        if (op1f_cards_2.slots[i].card == card) return (int)i;
    return -1;
}

static int used(u32 slot) { return op1f_cards_in_use(slot); }   // R20: any of the 128

extern "C" int op1f_option(const Op1fCard* card, unsigned option) {
    int slot = card ? slot_of(card) : -1;
    if (slot < 0 || !used((u32)slot)) return OP1F_BAD_NAME;
    int done = s.read_done != 0;
    const Op1fOptions* o = done ? s.table[slot] : table_of((u32)slot, 0, 0);
    if (o == 0 || option >= all_of(o)) return OP1F_BAD_NAME;
    return done ? (int)value[slot][option] : (int)o->options[option].first;      // a position, 0 to 255
}

// ---------------------------------------------------------------- texts

// A name or a word as the unit's letters show it: small letters (the font keeps its capitals at their codes), `_` a
// space; at most `most` characters, ended.
static void shown(char* out, const char* in, u32 most) {
    u32 n = 0;
    for (; n < most && in[n]; ++n) {
        char c = in[n];
        out[n] = c >= 'A' && c <= 'Z' ? (char)(c + 32) : c == '_' ? ' ' : c;
    }
    out[n] = 0;
}

static u32 number_text(char* out, int number) {        // by subtraction: -128 to 999
    static const int TENS[3] = { 100, 10, 1 };
    u32 at = 0, started = 0;
    if (number < 0) { out[at++] = '-'; number = -number; }
    for (u32 i = 0; i < 3u; ++i) {
        int digit = 0;
        while (number >= TENS[i]) { number -= TENS[i]; ++digit; }
        if (digit || started || i == 2u) { out[at++] = (char)('0' + digit); started = 1; }
    }
    out[at] = 0;
    return at;
}

static void value_text(char* out, const Op1fOption* p, u32 v) {
    if (p->kind == (u8)OP1F_OPTION_SWITCH) shown(out, v ? "on" : "off", SHOWN_MOST);
    else if (p->kind == (u8)OP1F_OPTION_LIST) shown(out, p->choices[v < (u32)OP1F_CHOICES_MOST ? v : 0u], (u32)OP1F_CHOICE_NAME);
    else if (p->kind == (u8)OP1F_OPTION_WORDS) { const char* w = v < (u32)WORDS_MOST ? word_fn(p)(v) : 0; shown(out, w ? w : "-", SHOWN_MOST); }
    else number_text(out, number_of(p, v));
}

// A line of a category's screen: the option's name and its value, into the shared seven.
static void put_line(void* screen, u32 line, u32 slot, u32 option) {
    char name[LINE_ROOM], said[LINE_ROOM];
    const Op1fOption* p = &s.table[slot]->options[option];
    shown(name, p->name, SHOWN_MOST);
    value_text(said, p, value[slot][option]);
    STOCK_LINE_NAME(screen, line, name);
    STOCK_LINE_VALUE(screen, line, said);
}

// The detents of a turn the settings hand a row: the knob's 16-bit count over 32768, as a float, made the count again
// from its bits (no float library), then shifted right by ten as the unit's own item takes it (CtlKnob's turn: a
// detent is 1024 of the count), the remainder dropped toward minus as there.
static int detents(u32 bits) {
    u32 e = (bits >> 23) & 0xffu;
    if (e < 112u) return 0;                                     // below one count
    u32 m = (bits & 0x7fffffu) | 0x800000u;
    int count = e > 127u ? 32767 : (int)(m >> (135u - e));      // the value is m x 2^(e - 150); the count that x 2^15
    if (bits >> 31) count = -count;
    return count >> 10;
}

// One turn on a value, as the unit's own item takes it (CtlKnob): the raw count moves by the detents and is held
// between 0 and the top (no wrap), the value is the raw count over the detents a step.
static u32 stepped(int* raw, int turn, u32 top, u32 shift) {
    int r = *raw + turn, most = (int)(top << shift);
    if (r < 0) r = 0;
    if (r > most) r = most;
    *raw = r;
    return (u32)r >> shift;
}

// ---------------------------------------------------------------- the file

static int same_to(const char* a, const char* b, u32 n) {       // the first n bytes of a are b, whole
    u32 i = 0;
    for (; i < n && b[i] && a[i] == b[i]; ++i) {}
    return i == n && b[i] == 0;
}

// The value the file holds for <pack>/<option>, or -1. A line ends at LF; a CR before it is not part of it. Base 44:
// `word`, when not 0, takes the text behind the `=` instead (up to 15 characters), for an option of words.
static int kept(const char* pack, const char* option, char* word = 0) {
    for (u32 at = 0; at < s.text_bytes;) {
        u32 end = at, slash = 0, equal = 0;
        while (end < s.text_bytes && text[end] != '\n') ++end;
        u32 stop = end > at && text[end - 1] == '\r' ? end - 1 : end;
        for (u32 i = at; i < stop; ++i) {
            if (text[i] == '/' && slash == 0) slash = i;
            if (text[i] == '=') equal = i;
        }
        if (slash > at && equal > slash + 1 && equal + 1 < stop && same_to(text + at, pack, slash - at) && same_to(text + slash + 1, option, equal - slash - 1)) {
            if (word) {
                u32 n = 0;
                for (u32 i = equal + 1; i < stop && n + 1u < (u32)NAME; ++i) word[n++] = text[i];
                word[n] = 0;
                return 0;
            }
            int v = 0;
            for (u32 i = equal + 1; i < stop; ++i) {
                if (text[i] < '0' || text[i] > '9' || v > 99) return -1;
                v = v * 10 + (text[i] - '0');
            }
            return v > (int)VALUE_MOST ? -1 : v;
        }
        at = end + 1;
    }
    return -1;
}

// Where each option shows: KEYBOARD, SYSTEM and MIDI while they have room, in the packs' order; else the pack's page.
static void place() {
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const Op1fOptions* o = s.table[slot];
        if (o == 0) continue;
        u32 sent = 0, full = 0;                                 // full: a bit for each category found full
        for (u32 n = 0; n < all_of(o); ++n) {
            u32 cat = o->options[n].kind == (u8)OP1F_OPTION_WORDS ? 0u : (u32)(o->options[n].where & (u8)OP1F_OPTION_CATEGORY) >> 4;     // base 44: words under PACKS
            if (cat >= (u32)IN_KEYBOARD && cat <= (u32)IN_MIDI && s.cat_count[cat] < ROOM[cat]) {
                s.cat_slot[cat][s.cat_count[cat]] = (u8)slot;
                s.cat_option[cat][s.cat_count[cat]] = (u8)n;
                ++s.cat_count[cat];
                continue;
            }
            if (cat >= (u32)IN_KEYBOARD && cat <= (u32)IN_MIDI) { ++sent; full |= 1u << cat; }
            s.on_page[slot] = (u8)(s.on_page[slot] | (1u << n));
        }
        if (sent)
            STOCK_LOG(2, LOG_FILE, 6, "options: '%s': %u option(s) on its page under PACKS, full:%s%s%s", op1f_cards_2.slots[slot].card->name, sent,
                      (full >> IN_KEYBOARD) & 1u ? " keyboard" : "", (full >> IN_SYSTEM) & 1u ? " system" : "", (full >> IN_MIDI) & 1u ? " midi" : "");
        if (s.open[slot] || s.on_page[slot]) s.listed[s.listed_count++] = (u8)slot;
    }
}

static void read_file() {
    s.text_bytes = 0;
    int size = STOCK_FILE_SIZE(PATH);
    if (size > (int)TEXT_MOST) s.too_big = 1;
    if (size > 0 && size <= (int)TEXT_MOST) {
        int h = STOCK_OPEN(PATH, READ_MODE);
        if (h >= 0) {
            int got = STOCK_READ(text, 1, (u32)size, h);
            STOCK_CLOSE(h);
            if (got == size) s.text_bytes = (u32)size;
        }
    }
    text[s.text_bytes] = 0;
    u32 packs = 0, found = 0;
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const char* why = 0;
        OpenPageFn open = 0;
        const Op1fOptions* o = used(slot) ? table_of(slot, &why, &open) : 0;
        if (why) STOCK_LOG(1, LOG_FILE, 7, "options: '%s' asks for options not in order (%s): left aside", op1f_cards_2.slots[slot].card->name, why);
        if (o == 0) continue;
        s.table[slot] = o;
        s.open[slot] = open;
        ++packs;
        for (u32 n = 0; n < all_of(o); ++n) {
            if (o->options[n].kind == (u8)OP1F_OPTION_WORDS) {          // base 44: its first now, its word looked for at each frame
                value[slot][n] = o->options[n].first;
                if (s.live_count >= (u32)LIVE_MOST) {
                    STOCK_LOG(1, LOG_FILE, 8, "options: '%s/%s': more than %u options of words on the unit: its choice is NOT kept over a start", op1f_cards_2.slots[slot].card->name, o->options[n].name, (u32)LIVE_MOST);
                    continue;
                }
                u32 k = s.live_count++;
                s.live[k].slot = (u8)slot;
                s.live[k].option = (u8)n;
                s.live[k].word[0] = 0;
                if (kept(op1f_cards_2.slots[slot].card->name, o->options[n].name, s.live[k].word) == 0) ++found;
                continue;
            }
            int v = kept(op1f_cards_2.slots[slot].card->name, o->options[n].name);
            if (v >= 0 && (u32)v <= top_of(&o->options[n])) ++found;
            value[slot][n] = (u8)(v >= 0 && (u32)v <= top_of(&o->options[n]) ? (u32)v : (u32)o->options[n].first);
        }
    }
    place();
    BARRIER();
    s.read_done = 1;
    STOCK_LOG(3, LOG_FILE, 1, "options: %u pack(s) with options, %u value(s) read from %s (%d bytes%s); %u under PACKS, %u %u %u in keyboard, system, midi; the row %s",
              packs, found, PATH, size, s.too_big ? ", over 4096: not read, not written over" : "", s.listed_count,
              s.cat_count[IN_KEYBOARD], s.cat_count[IN_SYSTEM], s.cat_count[IN_MIDI],
              s.row_added ? "is in the list" : s.row_failed ? "could NOT be made" : "is not in the list");
}

// Base 44: the word an option of words keeps, or 0 (none kept for it).
static char* word_of(u32 slot, u32 option) {
    for (u32 k = 0; k < s.live_count; ++k)
        if (s.live[k].slot == slot && s.live[k].option == option) return s.live[k].word;
    return 0;
}

static int same_word(const char* a, const char* b) {           // the first 15 characters
    u32 i = 0;
    for (; i + 1u < (u32)NAME && a[i] && a[i] == b[i]; ++i) {}
    return i + 1u == (u32)NAME || a[i] == b[i];
}

// The option's word now, kept as what the owner chose (a change from the settings or the pack).
static void word_again(u32 slot, u32 option) {
    char* w = word_of(slot, option);                           // 0 for every option but one of words
    if (w == 0) return;
    const char* now = value[slot][option] < (u32)WORDS_MOST ? word_fn(&s.table[slot]->options[option])(value[slot][option]) : 0;
    if (now == 0) return;
    u32 n = 0;
    for (; n + 1u < (u32)NAME && now[n]; ++n) w[n] = now[n];
    w[n] = 0;
}

// Once a frame: each option of words whose word kept is not its value's word looks for it among the words now, and
// found, takes its number and tells the pack. None kept yet: the value's word is kept.
static void words_follow() {
    for (u32 k = 0; k < s.live_count; ++k) {
        u32 slot = s.live[k].slot, option = s.live[k].option;
        if (s.table[slot] == 0 || !used(slot)) continue;
        Op1fOptionWord f = word_fn(&s.table[slot]->options[option]);
        u32 v = value[slot][option];
        const char* now = v < (u32)WORDS_MOST ? f(v) : 0;
        if (s.live[k].word[0] == 0) { if (now) word_again(slot, option); continue; }
        if (now && same_word(now, s.live[k].word)) continue;
        for (u32 i = 0; i < (u32)WORDS_MOST; ++i) {
            const char* w = f(i);
            if (w == 0) break;
            if (!same_word(w, s.live[k].word)) continue;
            value[slot][option] = (u8)i;
            const Op1fOptions* o = s.table[slot];
            if (o->changed) o->changed(option, i);
            break;
        }
    }
}

static u32 put(char* out, u32 at, const char* t, u32* over) {
    for (; *t; ++t) {
        if (at >= (u32)TEXT_MOST) { *over = 1; return at; }
        out[at++] = *t;
    }
    return at;
}

static int here(const char* pack, u32 n) {                     // a pack of that name (its first n bytes) has options now
    for (u32 slot = 0; slot < (u32)MOST; ++slot)
        if (s.table[slot] && used(slot) && same_to(pack, op1f_cards_2.slots[slot].card->name, n)) return 1;
    return 0;
}

static void write_file() {
    static char out[TEXT_MOST + 1];
    if (s.too_big) {
        s.dirty = 0;
        STOCK_LOG(1, LOG_FILE, 2, "options: %s is over 4096 bytes: NOT written over; the values hold until the unit is switched off", PATH);
        return;
    }
    u32 at = 0, over = 0;
    for (u32 slot = 0; slot < (u32)MOST; ++slot) {
        const Op1fOptions* o = s.table[slot];
        if (o == 0 || !used(slot)) continue;
        for (u32 n = 0; n < all_of(o); ++n) {
            char digits[NAME + 2];
            u32 d = 0;
            if (o->options[n].kind == (u8)OP1F_OPTION_WORDS) {          // base 44: the word kept, or no line
                const char* w = word_of((u32)slot, n);
                while (w && w[d] && d + 1u < (u32)NAME) { digits[d] = w[d]; ++d; }
                if (d == 0) continue;
            } else d = number_text(digits, (int)value[slot][n]);
            digits[d] = '\n';
            digits[d + 1] = 0;
            at = put(out, at, op1f_cards_2.slots[slot].card->name, &over);
            at = put(out, at, "/", &over);
            at = put(out, at, o->options[n].name, &over);
            at = put(out, at, "=", &over);
            at = put(out, at, digits, &over);
        }
    }
    for (u32 from = 0; from < s.text_bytes;) {                 // the lines of packs not here, as they were
        u32 end = from, slash = 0;
        while (end < s.text_bytes && text[end] != '\n') { if (text[end] == '/' && slash == 0) slash = end; ++end; }
        if (slash > from && !here(text + from, slash - from)) {
            for (u32 i = from; i <= end && i < s.text_bytes; ++i) {
                if (at >= (u32)TEXT_MOST) { over = 1; break; }
                out[at++] = text[i];
            }
            if (end >= s.text_bytes && at < (u32)TEXT_MOST) out[at++] = '\n';
            else if (end >= s.text_bytes) over = 1;
        }
        from = end + 1;
    }
    if (over) {
        s.dirty = 0;
        STOCK_LOG(1, LOG_FILE, 2, "options: %s would pass 4096 bytes: NOT written; the values hold until the unit is switched off", PATH);
        return;
    }
    int answer = op1f_folders_write_base(PATH, out, at);
    if (answer == OP1F_NOT_NOW) return;                          // the disk mode: at a later frame
    s.dirty = 0;
    if (answer == (int)at) {
        for (u32 i = 0; i < at; ++i) text[i] = out[i];
        s.text_bytes = at;
        text[at] = 0;
    }
    STOCK_LOG(answer == (int)at ? 3 : 1, LOG_FILE, 2, "options: %s %s, %u bytes (%d)", answer == (int)at ? "wrote" : "could NOT write", PATH, at, answer);
}

// ---------------------------------------------------------------- the display thread's work

static void tell(u32 slot, u32 option) {                       // a value changed: the pack told, the file to write
    const Op1fOptions* o = s.table[slot];
    s.dirty = 1;
    s.changed_ms = op1f_ms();
    word_again(slot, option);                                    // base 44: an option of words keeps its word
    if (o && o->changed && used(slot)) o->changed(option, (u32)value[slot][option]);
}

static void show_items(u32 slot) {                            // the pack's page under PACKS: its options there
    s.items_slot = slot;
    s.items_count = 0;
    for (u32 n = 0; n < all_of(s.table[slot]); ++n)
        if ((s.on_page[slot] >> n) & 1u) s.items[s.items_count++] = (u8)n;
    s.items_raw = 0;
    s.items_first = 0;
    s.red_line = 0xffffffffu;
    s.shows = SHOWS_ITEMS;
}

static void take_keys() {
    u32 c = s.chosens;
    if (c != s.chosens_taken) { s.chosens_taken = c; s.shows = SHOWS_LIST; }
    u32 t = s.ochre_turned;
    int turn = (int)(t - s.ochre_taken);
    s.ochre_taken = t;
    if (turn && s.shows == SHOWS_LIST && s.listed_count) {
        int raw = (int)s.list_raw;
        stepped(&raw, turn, s.listed_count - 1u, LINE_SHIFT);
        s.list_raw = (u32)raw;
    } else if (turn && s.shows == SHOWS_ITEMS && s.items_count) {
        int raw = (int)s.items_raw;
        stepped(&raw, turn, s.items_count - 1u, LINE_SHIFT);
        s.items_raw = (u32)raw;
    }
    u32 b = s.backs;
    if (b != s.backs_taken) { s.backs_taken = b; s.shows = SHOWS_LIST; }
    u32 p = s.presses;
    if (p != s.presses_taken) {
        s.presses_taken = p;
        u32 line = s.list_raw >> LINE_SHIFT;
        if (s.shows == SHOWS_LIST && line < s.listed_count) {
            u32 slot = s.listed[line];
            if (s.open[slot]) s.open_asked = slot + 1u;
            else show_items(slot);
        }
    }
    t = s.red_turned;
    turn = (int)(t - s.red_taken);
    s.red_taken = t;
    u32 line = s.items_raw >> LINE_SHIFT;
    if (turn && s.shows == SHOWS_ITEMS && line < s.items_count) {
        u32 slot = s.items_slot, option = s.items[line];
        const Op1fOption* o = &s.table[slot]->options[option];
        u32 shift = shift_of(o), v = value[slot][option];
        if (s.red_line != line || ((u32)s.red_raw >> shift) != v) { s.red_raw = (int)(v << shift); s.red_line = line; }
        u32 w = stepped(&s.red_raw, turn, top_of(o), shift);
        if (w != v) { value[slot][option] = (u8)w; tell(slot, option); }
    }
}

// Once a frame, on the display thread, before any frame function (the cards unit's frame work, behind the values' unit's).
extern "C" void op1f_options_frame() OP1F_NAMED("op1f_options_frame");
extern "C" void op1f_options_frame() {
    if (!s.read_done) read_file();
    words_follow();                                             // base 44
    take_keys();
    for (u32 cat = (u32)IN_KEYBOARD; cat <= (u32)IN_MIDI; ++cat)        // the categories' lines the key thread changed
        for (u32 k = 0; k < s.cat_count[cat]; ++k) {
            u8 b = s.bumped[cat][k];
            if (b != s.told[cat][k]) { s.told[cat][k] = b; tell(s.cat_slot[cat][k], s.cat_option[cat][k]); }
        }
    if (s.open_asked) {
        u32 slot = s.open_asked - 1u;
        s.open_asked = 0;
        if (s.open[slot] && used(slot)) {
            STOCK_LOG(3, LOG_FILE, 3, "options: '%s' opens its page", op1f_cards_2.slots[slot].card->name);
            s.open[slot]();
        }
    }
    if (s.dirty && op1f_ms() - s.changed_ms >= (u32)WRITE_AFTER_MS) write_file();
    u32 d = s.drawn;                                            // our row's screen drawn since the last frame: a pack's options up
    s.items_up = s.shows == SHOWS_ITEMS && d != s.drawn_seen ? 1u : 0u;
    s.drawn_seen = d;
}

// The screen of the settings' chosen row, or 0: the chosen row is an index into the list of the rows' screens, which
// the settings move on the key thread. The shared seven names and values are written only while it is the
// screen they are written for.
static u32 chosen_screen() {
    u32 i = RD32(SETTINGS_CHOSEN), first = RD32(SETTINGS_LIST + 4u), end = RD32(SETTINGS_LIST + 8u);
    if (end < first || i >= (end - first) >> 2) return 0;
    return RD32(first + (i << 2));
}

// The window of seven that follows the chosen line.
static u32 window(u32* first, u32 line, u32 count) {
    if (line < *first) *first = line;
    if (line >= *first + (u32)LINES) *first = line + 1u - (u32)LINES;
    if (*first + (u32)LINES > count) *first = count > (u32)LINES ? count - (u32)LINES : 0u;
    return *first;
}

// Our screen's draw, on the display thread, for the row the settings show: our lines into the shared seven, then the
// unit's own draw. Only while our row is the settings' chosen one, before and after the writing: a fast blue turn to
// another row runs that row's chosen on the key thread meanwhile, and it fills the shared seven for its own draw.
static void packs_draw(void* screen, void* context) {
    if (chosen_screen() != (u32)screen) return;
    u32 count = 0, line = 0, first = 0;
    if (s.read_done && s.shows == SHOWS_ITEMS) {
        count = s.items_count;
        line = s.items_raw >> LINE_SHIFT;
        first = window(&s.items_first, line, count);
        for (u32 i = 0; i < (u32)LINES && first + i < count; ++i) put_line(screen, i, s.items_slot, s.items[first + i]);
    } else if (s.read_done) {
        count = s.listed_count;
        line = s.list_raw >> LINE_SHIFT;
        first = window(&s.list_first, line, count);
        for (u32 i = 0; i < (u32)LINES && first + i < count; ++i) {
            char name[LINE_ROOM];
            shown(name, op1f_cards_2.slots[s.listed[first + i]].card->name, SHOWN_MOST);
            STOCK_LINE_NAME(screen, i, name);
            STOCK_LINE_VALUE(screen, i, "");
        }
    }
    BARRIER();
    if (chosen_screen() != (u32)screen) return;                // moved on while we wrote: the other row's chosen fills them
    s.drawn = s.drawn + 1u;
    WR32((u32)screen + SCREEN_LINE, count ? line - first : 0u);
    WR32((u32)screen + SCREEN_COUNT, count > (u32)LINES ? (u32)LINES : count);
    STOCK_SCREEN_DRAW(screen, context);
}

static const char* packs_name(void*) { return ROW_NAME; }
static const void* const PACKS_SCREEN_TABLE[2] = { (const void*)packs_name, (const void*)packs_draw };

// ---------------------------------------------------------------- a pack sets its own option

// The line of KEYBOARD, SYSTEM or MIDI that shows this option, written again while the settings show that category (the
// display thread). A line under PACKS needs nothing: packs_draw writes its lines at every draw.
static void line_again(u32 slot, u32 option) {
    for (u32 cat = (u32)IN_KEYBOARD; cat <= (u32)IN_MIDI; ++cat)
        for (u32 k = 0; k < s.cat_count[cat]; ++k) {
            if (s.cat_slot[cat][k] != slot || s.cat_option[cat][k] != option) continue;
            u32 row = (u32)s.cat_row[cat], line = STOCK_LINES[cat] + k;
            u32 screen = row ? RD32(row + ROW_SCREEN) : 0u;
            if (screen == 0) return;
            char said[LINE_ROOM];
            value_text(said, &s.table[slot]->options[option], value[slot][option]);
            if (chosen_screen() == screen && RD32(screen + SCREEN_COUNT) > line) STOCK_LINE_VALUE((void*)screen, line, said);
            return;
        }
}

// From the pack's own frame function only (op1f_cards_calling is its slot + 1 there, and 0 or another's elsewhere).
extern "C" int op1f_option_set(const Op1fCard* card, unsigned option, unsigned position) {
    int slot = card ? slot_of(card) : -1;
    if (slot < 0 || !used((u32)slot)) return OP1F_BAD_NAME;
    if (op1f_cards_calling() != (u32)slot + 1u || !s.read_done) return OP1F_NOT_NOW;
    const Op1fOptions* o = s.table[slot];
    if (o == 0 || option >= all_of(o) || position > top_of(&o->options[option])) return OP1F_BAD_NAME;
    if (position == (u32)value[slot][option]) return 0;
    value[slot][option] = (u8)position;
    word_again((u32)slot, option);                              // base 44: an option of words keeps its word
    s.dirty = 1;                                                // the file, as after a change in the settings
    s.changed_ms = op1f_ms();
    line_again((u32)slot, option);
    return 0;
}

// ---------------------------------------------------------------- our row, on the key thread: counts only

static void row_turn(void*, u32 knob, u32 amount) {
    int turn = detents(amount);
    if (knob == (u32)KNOB_OCHRE) s.ochre_turned = s.ochre_turned + (u32)turn;
    else if (knob == (u32)KNOB_RED) s.red_turned = s.red_turned + (u32)turn;
}
static void row_key(void*, u32 key) { if (key == 0) s.backs = s.backs + 1u; }          // T1's release: back to the list
static void row_click(void*, u32 knob) { if (knob == (u32)KNOB_OCHRE) s.presses = s.presses + 1u; }
static void row_nothing(void*) {}
static void row_chosen(void*) { s.chosens = s.chosens + 1u; }
static void* row_screen_of(void*) { return s.screen; }
static const void* const ROW_TABLE[8] = { (const void*)row_turn, (const void*)row_key, (const void*)row_click, (const void*)row_nothing,
                                          (const void*)row_chosen, (const void*)row_screen_of, (const void*)row_nothing, (const void*)row_nothing };
struct RowController { const void* const* table; };
static RowController row_controller = { ROW_TABLE };

// T1's press leaves the settings: the settings controller's own key down leaves them at T1's press, before the release
// could reach our +0x04. This is for the cards unit's key down, before the firmware's: not
// 0 when it took T1, while a pack's options show under PACKS (the key thread; it only counts).
extern "C" int op1f_options_down(unsigned key) OP1F_NAMED("op1f_options_down");
extern "C" int op1f_options_down(unsigned key) {
    if (key != (u32)KEY_T1 || !s.items_up) return 0;
    s.backs = s.backs + 1u;
    return 1;
}

// At the start, on the main thread, before the first frame: each of the firmware's rows through the trampoline; ours
// behind the sixth. KEYBOARD's, SYSTEM's and MIDI's rows noted by their tables, for a line op1f_option_set writes
// again (their screens are read at that time). No line here: read_file says it.
extern "C" void op1f_options_add(void* owner, void* controller) OP1F_NAMED("op1f_options_add");
extern "C" void op1f_options_add(void* owner, void* controller) {
    STOCK_ADD(owner, controller);
    u32 table = controller ? RD32(controller) : 0u;
    if (table == TABLE_KEYBOARD) s.cat_row[IN_KEYBOARD] = controller;
    if (table == TABLE_SYSTEM) s.cat_row[IN_SYSTEM] = controller;
    if (table == TABLE_MIDI) s.cat_row[IN_MIDI] = controller;
    if (s.row_added || s.row_failed || controller == 0 || RD32(controller) != LAST_STOCK_ROW) return;
    void* screen = STOCK_NEW(SCREEN_BYTES);
    if (screen == 0) { s.row_failed = 1; return; }
    STOCK_SCREEN_MAKE(screen, ROW_NAME);
    WR32(screen, (u32)PACKS_SCREEN_TABLE);
    s.screen = screen;
    s.row_added = 1;
    STOCK_ADD(owner, &row_controller);
}

// ---------------------------------------------------------------- KEYBOARD, SYSTEM, MIDI: the rows' table words

// Chosen: the unit's own first; then our lines behind its, the counts, the picker's top, and the picker where it was
// (the unit's own function holds it to its own lines). The key thread, as the unit's own.
static void chosen_in(void* row, u32 cat, ChosenFn stock) {
    u32 picker = (u32)row + ROW_PICKER;
    u32 was = RD32(picker + KNOB_VALUE);
    stock(row);
    u32 ours = s.read_done ? s.cat_count[cat] : 0u, n = STOCK_LINES[cat];
    if (ours == 0 || RD32((u32)row + ROW_COUNT) != n) return;
    u32 screen = RD32((u32)row + ROW_SCREEN);
    if (screen == 0) return;
    for (u32 k = 0; k < ours; ++k) put_line((void*)screen, n + k, s.cat_slot[cat][k], s.cat_option[cat][k]);
    WR32((u32)row + ROW_COUNT, n + ours);
    WR32(screen + SCREEN_COUNT, n + ours);
    STOCK_KNOB_TOP((void*)picker, n + ours - 1u);
    if (was <= RD32(picker + KNOB_TOP)) WR32(picker + KNOB_VALUE, was);
}

// The red knob on a line: the unit's own lines to its own function; ours changed here and its line written again.
static void red_in(void* row, u32 line, u32 amount, u32 cat, RedFn stock) {
    u32 n = STOCK_LINES[cat];
    if (line < n) { stock(row, line, amount); return; }
    u32 k = line - n;
    if (!s.read_done || k >= s.cat_count[cat]) return;
    u32 slot = s.cat_slot[cat][k], option = s.cat_option[cat][k];
    const Op1fOption* p = &s.table[slot]->options[option];
    u32 shift = shift_of(p), v = value[slot][option];
    if (s.cat_raw_line[cat] != line + 1u || ((u32)s.cat_raw[cat] >> shift) != v) { s.cat_raw[cat] = (int)(v << shift); s.cat_raw_line[cat] = line + 1u; }
    u32 w = stepped(&s.cat_raw[cat], detents(amount), top_of(p), shift);
    if (w == v) return;
    value[slot][option] = (u8)w;
    u32 screen = RD32((u32)row + ROW_SCREEN);
    if (screen) {
        char said[LINE_ROOM];
        value_text(said, p, w);
        STOCK_LINE_VALUE((void*)screen, line, said);
    }
    s.bumped[cat][k] = (u8)(s.bumped[cat][k] + 1u);
}

extern "C" void op1f_settings_chosen_keyboard(void* row) OP1F_NAMED("op1f_settings_chosen_keyboard");
extern "C" void op1f_settings_chosen_keyboard(void* row) { chosen_in(row, IN_KEYBOARD, STOCK_CHOSEN_KEYBOARD); }
extern "C" void op1f_settings_chosen_system(void* row) OP1F_NAMED("op1f_settings_chosen_system");
extern "C" void op1f_settings_chosen_system(void* row) { chosen_in(row, IN_SYSTEM, STOCK_CHOSEN_SYSTEM); }
extern "C" void op1f_settings_chosen_midi(void* row) OP1F_NAMED("op1f_settings_chosen_midi");
extern "C" void op1f_settings_chosen_midi(void* row) { chosen_in(row, IN_MIDI, STOCK_CHOSEN_MIDI); }
extern "C" void op1f_settings_red_keyboard(void* row, u32 line, u32 amount) OP1F_NAMED("op1f_settings_red_keyboard");
extern "C" void op1f_settings_red_keyboard(void* row, u32 line, u32 amount) { red_in(row, line, amount, IN_KEYBOARD, STOCK_RED_KEYBOARD); }
extern "C" void op1f_settings_red_system(void* row, u32 line, u32 amount) OP1F_NAMED("op1f_settings_red_system");
extern "C" void op1f_settings_red_system(void* row, u32 line, u32 amount) { red_in(row, line, amount, IN_SYSTEM, STOCK_RED_SYSTEM); }
extern "C" void op1f_settings_red_midi(void* row, u32 line, u32 amount) OP1F_NAMED("op1f_settings_red_midi");
extern "C" void op1f_settings_red_midi(void* row, u32 line, u32 amount) { red_in(row, line, amount, IN_MIDI, STOCK_RED_MIDI); }

#ifdef OP1F_HOST
// The test's way in: a fresh start (the row stays as added).
static void options_restart() {
    void* screen = s.screen;
    u32 added = s.row_added;
    void* rows[CATEGORIES];
    for (u32 c = 0; c < (u32)CATEGORIES; ++c) rows[c] = s.cat_row[c];
    for (u32 i = 0; i < (u32)sizeof s; ++i) ((u8*)&s)[i] = 0;
    for (u32 i = 0; i < (u32)sizeof value; ++i) ((u8*)value)[i] = 0;
    s.screen = screen;
    s.row_added = added;
    for (u32 c = 0; c < (u32)CATEGORIES; ++c) s.cat_row[c] = rows[c];
}
#endif

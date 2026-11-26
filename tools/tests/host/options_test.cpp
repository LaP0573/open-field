// Host test of the base's options (module/build_r8_options.cpp, base 32 and base 43's options of words): the values
// read from the base's file, the row "packs" and a pack's page under it, the 4th knob changing a value, `changed`
// told, op1f_option and op1f_option_set, the file written (by word for an option of words) with the lines of packs not
// here kept. Built for a 32-bit target (the card header holds four-byte addresses): emscripten and node,
//   em++ -std=c++11 -DOP1F_HOST -I module -I tools/tests/host tools/tests/host/options_test.cpp -o <out>.js && node <out>.js
// tools/tests/test_options.py runs it where both are at hand. VERBOSE=1 prints the base's lines.
// OLD_UNIT=1 (with -DOLD_UNIT -include <base 38's build_r8_options.cpp>) runs the part an older base must pass: the
// options of words behind `count` unseen, the others as before.
#include <cassert>
#ifndef OLD_UNIT
#include "../../../module/build_r8_options.cpp"
#endif

static unsigned told[64][2], told_count;
static void changed_v(unsigned option, unsigned v) { told[told_count][0] = option; told[told_count++][1] = v; }
// The words the packs give now: vu's looks (registered at their packs' first frame) and themes' themes.
static std::vector<std::string> looks, themes;
static unsigned asked;                                  // how many words were asked for
static const char* look_word(unsigned c) { ++asked; return c < looks.size() ? looks[c].c_str() : 0; }
static const char* theme_word(unsigned c) { return c < themes.size() ? themes[c].c_str() : 0; }
// The base takes a pack's function only at an even address (a Blackfin instruction's); here a function's address is
// its number in the table, so each is picked among four of the same.
template <int N> static const char* look_n(unsigned c) { return look_word(c); }
template <int N> static const char* theme_n(unsigned c) { return theme_word(c); }
template <int N> static void changed_n(unsigned o, unsigned v) { changed_v(o, v); }
template <class F> static F even(F a, F b, F c, F d) { F all[4] = { a, b, c, d }; for (F f : all) if (!((unsigned)(unsigned long)f & 1u)) return f; assert(0); return a; }

static Op1fOptions VU_OPTIONS, THEMES_OPTIONS, OLD_OPTIONS, BAD_OPTIONS;
static Op1fAsk VU_ASKS[1], THEMES_ASKS[1], OLD_ASKS[1], BAD_ASKS[1];
static Op1fCard VU, THEMES, OLD, BAD;
Op1fCards op1f_cards;
static unsigned calling, now_ms;
u32 op1f_cards_used() { return 15; }
#ifndef OLD_UNIT
// base 42: the second list, 128 slots; under N1 the base copies the first into it (op1f_cards_lists)
Op1fCards2 op1f_cards_2;
void op1f_cards_lists() { op1f_cards_2.count = op1f_cards.count; op1f_cards_2.more = op1f_cards.more;
    for (unsigned i = 0; i < (unsigned)OP1F_CARDS_MOST; ++i) op1f_cards_2.slots[i] = op1f_cards.slots[i]; }
int op1f_cards_in_use(unsigned slot) { return slot < 32 && ((op1f_cards_used() >> slot) & 1u); }
#else
static void op1f_cards_lists() {}
#endif
u32 op1f_cards_calling() { return calling; }
unsigned op1f_ms() { return now_ms; }
int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) { mock_disk[path] = std::string((const char*)data, bytes); return (int)bytes; }

static void option(Op1fOption* o, const char* name, unsigned kind, unsigned count, unsigned first, const char* const* choices) {
    memset(o, 0, sizeof *o); strncpy(o->name, name, 15); o->kind = (unsigned char)kind; o->count = (unsigned char)count; o->first = (unsigned char)first;
    for (unsigned i = 0; choices && i < count; ++i) strncpy(o->choices[i], choices[i], 7);
}
static void words(Op1fOption* o, const char* name, unsigned first, const char* (*f)(unsigned)) {
    option(o, name, 3, 0, first, 0); memcpy(o->choices, &f, 4);                  // 3: OP1F_OPTION_WORDS, base 43
}
static void card(Op1fCard* c, const char* name, Op1fAsk* asks, Op1fOptions* o) {
    memset(c, 0, sizeof *c); c->mark = OP1F_CARD_MARK; c->bytes = OP1F_CARD_LEVEL_7_BYTES; c->level = 7;
    strncpy(c->name, name, sizeof c->name - 1); c->asks = asks; c->ask_count = 1;
    asks[0].kind = OP1F_ASK_OPTIONS; asks[0].bytes = sizeof *o; asks[0].what = o;
}
static unsigned bits(float f) { unsigned b; memcpy(&b, &f, 4); return b; }
typedef void (*TurnFn)(void*, unsigned, unsigned);
typedef void (*KnobFn)(void*, unsigned);
typedef void (*DrawFn)(void*, void*);
static const void* const* row_table() { return *(const void* const* const*)mock_rows.back(); }
static void turn(unsigned knob, int detents) { ((TurnFn)row_table()[0])(mock_rows.back(), knob, bits(detents * 1024 / 32768.f)); }
static void click(unsigned knob) { ((KnobFn)row_table()[2])(mock_rows.back(), knob); }
static void chosen() { ((void (*)(void*))row_table()[4])(mock_rows.back()); }
static void draw() { void* screen = (void*)(unsigned long)mock_screens[6]; ((DrawFn)(*(const void* const* const*)screen)[1])(screen, 0); }
static void frame(unsigned ms) { now_ms = ms; op1f_options_frame(); }
static bool has(const char* line) { return mock_disk["user/pf/.options"].find(line) != std::string::npos; }

int main() {
    static const char* const STYLE[] = { "bars", "dots", "lines" };
    static const char* const SNAP[] = { "off", "1/16", "1/8" };
    // vu: two options every base sees, and `look`, an option of words behind them
    VU_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(VU_OPTIONS.head.name, "vu"); VU_OPTIONS.count = 2;
    VU_OPTIONS.changed = even(changed_n<0>, changed_n<1>, changed_n<2>, changed_n<3>);
    option(&VU_OPTIONS.options[0], "EQ knobs", 0, 2, 1, 0);
    option(&VU_OPTIONS.options[1], "classic", 1, 3, 0, STYLE);
    words(&VU_OPTIONS.options[2], "look", 0, even(look_n<0>, look_n<1>, look_n<2>, look_n<3>));
    // themes: an option of words only (an older base leaves the ask aside: it lists no option for it)
    THEMES_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(THEMES_OPTIONS.head.name, "themes"); THEMES_OPTIONS.count = 0;
    words(&THEMES_OPTIONS.options[0], "theme", 0, even(theme_n<0>, theme_n<1>, theme_n<2>, theme_n<3>));
    OLD_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(OLD_OPTIONS.head.name, "old"); OLD_OPTIONS.count = 1;
    option(&OLD_OPTIONS.options[0], "snap", 1, 3, 0, SNAP);
    // bad: an option of words without its function: the whole ask left aside on base 43
    BAD_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(BAD_OPTIONS.head.name, "bad"); BAD_OPTIONS.count = 1;
    option(&BAD_OPTIONS.options[0], "x", 0, 2, 0, 0);
    words(&BAD_OPTIONS.options[1], "y", 0, 0);
    card(&VU, "vu", VU_ASKS, &VU_OPTIONS); card(&THEMES, "themes", THEMES_ASKS, &THEMES_OPTIONS);
    card(&OLD, "old", OLD_ASKS, &OLD_OPTIONS); card(&BAD, "bad", BAD_ASKS, &BAD_OPTIONS);
    op1f_cards.count = 4;
    op1f_cards.slots[0].card = &BAD; op1f_cards.slots[1].card = &OLD; op1f_cards.slots[2].card = &THEMES; op1f_cards.slots[3].card = &VU;
    for (unsigned i = 0; i < 4; ++i) { op1f_cards.slots[i].first = 0; op1f_cards.slots[i].end = 0xfffffff0u; }
    op1f_cards_lists();
    mock_disk["user/pf/.options"] = "vu/EQ knobs=0\r\nvu/look=ANDY\ngone/x=3\nthemes/theme=CONSOLE\nold/snap=2\n";

    // the start: the unit's six rows, ours behind the sixth
    static unsigned tables[6] = { 0x6000u, TABLE_KEYBOARD, TABLE_SYSTEM, TABLE_MIDI, 0x6005u, LAST_STOCK_ROW };
    for (unsigned i = 0; i < 6; ++i) op1f_options_add(0, &tables[i]);
    assert(mock_rows.size() == 7);
    mock_screens[6] = (unsigned)(unsigned long)s.screen;
    mock_settings_list[1] = (unsigned)(unsigned long)mock_screens; mock_settings_list[2] = mock_settings_list[1] + 4 * 7;

    looks = { "OFF" };                                                           // vu's own: no look pack has registered yet
    frame(10);
    assert(op1f_option(&VU, 0) == 0 && op1f_option(&VU, 1) == 0);               // the file's (CR LF too), classic's first
    assert(op1f_option(&OLD, 0) == 2);
#ifdef OLD_UNIT
    // an older base: the options behind `count` are not there, the ask that lists none is left aside, the bad one runs
    assert(op1f_option(&VU, 2) < 0 && op1f_option(&THEMES, 0) < 0 && op1f_option(&BAD, 0) == 0);
    assert(asked == 0);
    puts("options old-base test: ok");
    return 0;
#else
    assert(op1f_option(&VU, 2) == 0);                                           // ANDY not there yet: look's first
    assert(op1f_option(&THEMES, 0) == 0 && op1f_option(&BAD, 0) < 0 && op1f_option(&VU, 3) < 0);
    assert(told_count == 0);
    // the looks' packs register at their first frame; the next frame's work finds ANDY
    looks = { "OFF", "SPECTRUM", "ANDY", "BELLA" };
    themes = { "off", "89MPH", "CONSOLE" };
    frame(20);
    assert(op1f_option(&VU, 2) == 2 && op1f_option(&THEMES, 0) == 2);
    assert(told_count == 1 && told[0][0] == 2 && told[0][1] == 2);              // vu told (themes has no changed)
    frame(30);
    assert(told_count == 1);                                                    // found once: nothing more
    assert(mock_disk["user/pf/.options"].find("ANDY") != std::string::npos);    // nothing written: nothing changed

    // settings > PACKS: the list (bad left aside, old's on its page), vu's page, `look` its word
    mock_settings_chosen = 6;
    chosen(); frame(40); draw();
    assert(mock_name[0] == "old" && mock_name[1] == "themes" && mock_name[2] == "vu");
    turn(1, 8); click(1); frame(50); draw();                                    // the 2nd knob down two lines, its click opens vu
    assert(mock_name[0] == "eq knobs" && mock_value[0] == "off" && mock_name[1] == "classic" && mock_value[1] == "bars");
    assert(mock_name[2] == "look" && mock_value[2] == "andy");
    turn(1, 8); frame(60);                                                      // down to `look`
    turn(3, 4); frame(70); draw();                                              // the 4th knob a step: BELLA
    assert(op1f_option(&VU, 2) == 3 && mock_value[2] == "bella");
    assert(told_count == 2 && told[1][0] == 2 && told[1][1] == 3);
    turn(3, 40); frame(80);                                                     // past the last word: it stops there
    assert(op1f_option(&VU, 2) == 3 && told_count == 2);
    turn(3, -12); frame(90); draw();                                            // three back: OFF
    assert(op1f_option(&VU, 2) == 0 && mock_value[2] == "off");
    frame(2000);                                                                // a second and a half after: written
    assert(has("vu/look=OFF\n") && has("vu/EQ knobs=0\n") && has("vu/classic=0\n") && has("gone/x=3\n") && has("themes/theme=CONSOLE\n") && has("old/snap=2\n"));

    // the pack sets it itself (the mixer key's round): from its own frame function only, the word kept
    assert(op1f_option_set(&VU, 2, 1) == OP1F_NOT_NOW);
    calling = 4;
    assert(op1f_option_set(&VU, 2, 4) == OP1F_BAD_NAME);                         // past the last word
    assert(op1f_option_set(&VU, 2, 1) == 0 && op1f_option(&VU, 2) == 1);
    calling = 0;
    frame(2100); frame(4000);
    assert(has("vu/look=SPECTRUM\n") && told_count == 3);                       // not told: the pack set it
    unsigned before = told_count;

    // a start with SPECTRUM's pack away: look stays at its first, its word kept in the file; back, it is found
    options_restart();
    looks = { "OFF" };
    frame(10);
    looks = { "OFF", "ANDY", "BELLA" };
    frame(20); frame(30);
    assert(op1f_option(&VU, 2) == 0 && told_count == before);
    calling = 0;
    mock_settings_chosen = 6; chosen(); frame(40);                               // the owner changes another option: the file is written
    turn(1, 8); click(1); frame(50); turn(3, 4); frame(60);                     // vu's page, EQ knobs on
    assert(op1f_option(&VU, 0) == 1);
    frame(3000);
    assert(has("vu/look=SPECTRUM\n") && has("vu/EQ knobs=1\n"));                 // SPECTRUM's line kept while it is away
    looks = { "OFF", "ANDY", "BELLA", "SPECTRUM" };                              // a pack that registers late
    frame(3100);
    assert(op1f_option(&VU, 2) == 3);

    // more than the base reads: at most 32 words asked for
    looks.assign(40, "W"); looks[0] = "OFF"; asked = 0;
    calling = 4; assert(op1f_option_set(&VU, 2, 31) == 0 && op1f_option_set(&VU, 2, 32) == OP1F_BAD_NAME); calling = 0;
    puts("options host test: ok");
    return 0;
#endif
}

// Host test of the base's options (module/build_r8_options.cpp, base 31): the values read from the base's file, the
// page's keys changing them, `changed` told, the file written with the lines of packs not here kept, op1f_option's
// answers. Built for a 32-bit target (the card header holds four-byte addresses): emscripten and node,
//   em++ -std=c++11 -DOP1F_HOST -I module -I tools/tests/host tools/tests/host/options_test.cpp -o <out>.js && node <out>.js
// tools/tests/test_options.py runs it where both are at hand. VERBOSE=1 prints the base's lines. OPTIONS_PAGE=<file>
// writes the page as painted, 480 x 224 RGB565 rows top to bottom, for a picture.
#include <cassert>
#include "../../../module/build_r8_options.cpp"

static unsigned told[16][2], told_count;
static void lanes_changed(unsigned option, unsigned v) { told[told_count][0] = option; told[told_count++][1] = v; }
static Op1fOptions LANES_OPTIONS, MARKERS_OPTIONS;
static Op1fAsk LANES_ASKS[1], MARKERS_ASKS[1];
static Op1fCard LANES, MARKERS, OLD;
Op1fCards op1f_cards;
static int top_wanted; static int (*top_keys)(const Op1fInput*);
u32 op1f_cards_used() { return 7; }
void op1f_top_open(void* d, int (*k)(const Op1fInput*)) { assert(d); top_wanted = 1; top_keys = k; }
void op1f_top_close() { top_wanted = 0; }
void* op1f_base_picture(const char*) { static int doc; return &doc; }
int op1f_folders_write_base(const char* path, const void* data, unsigned bytes) { mock_disk[path] = std::string((const char*)data, bytes); return (int)bytes; }

static void option(Op1fOption* o, const char* name, unsigned kind, unsigned count, unsigned first, const char* const* choices) {
    memset(o, 0, sizeof *o); strncpy(o->name, name, 15); o->kind = (unsigned char)kind; o->count = (unsigned char)count; o->first = (unsigned char)first;
    for (unsigned i = 0; choices && i < count; ++i) strncpy(o->choices[i], choices[i], 7);
}
static void card(Op1fCard* c, const char* name, unsigned level, Op1fAsk* asks, unsigned count) {
    memset(c, 0, sizeof *c); c->mark = OP1F_CARD_MARK; c->bytes = OP1F_CARD_LEVEL_7_BYTES; c->level = level;
    strncpy(c->name, name, sizeof c->name - 1); c->asks = asks; c->ask_count = count;
}
static void key(unsigned what, unsigned k, int turn) {
    Op1fInput e; memset(&e, 0, sizeof e); e.bytes = sizeof e; e.what = what; e.key = k; e.turn = turn; top_keys(&e);
}

int main() {
    static const char* const SNAP[] = { "off", "1/16", "1/8", "1/4" };
    LANES_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(LANES_OPTIONS.head.name, "lanes"); LANES_OPTIONS.changed = lanes_changed; LANES_OPTIONS.count = 1;
    option(&LANES_OPTIONS.options[0], "LCARS lanes", OP1F_OPTION_SWITCH, 2, 1, 0);
    MARKERS_OPTIONS.head.bytes = sizeof(Op1fOptions); strcpy(MARKERS_OPTIONS.head.name, "markers"); MARKERS_OPTIONS.count = 2;
    option(&MARKERS_OPTIONS.options[0], "snap", OP1F_OPTION_LIST, 4, 0, SNAP);
    option(&MARKERS_OPTIONS.options[1], "debug keys", OP1F_OPTION_SWITCH, 2, 0, 0);
    LANES_ASKS[0] = { OP1F_ASK_OPTIONS, sizeof(Op1fOptions), &LANES_OPTIONS };
    MARKERS_ASKS[0] = { OP1F_ASK_OPTIONS, sizeof(Op1fOptions), &MARKERS_OPTIONS };
    card(&LANES, "lanes", 6, LANES_ASKS, 1); card(&MARKERS, "markers", 10, MARKERS_ASKS, 1); card(&OLD, "old", 5, MARKERS_ASKS, 1);   // old: level 5, not read
    op1f_cards.count = 3; op1f_cards.slots[0].card = &LANES; op1f_cards.slots[1].card = &MARKERS; op1f_cards.slots[2].card = &OLD;

    assert(op1f_option(&LANES, 0) == 1 && op1f_option(&MARKERS, 0) == 0);           // before the file: the firsts
    assert(op1f_option(&OLD, 0) == OP1F_BAD_NAME && op1f_option(&LANES, 1) == OP1F_BAD_NAME);
    mock_disk["user/pf/.options"] = "lanes/LCARS lanes=0\nmarkers/snap=2\nmarkers/debug keys=9\ngone/thing=1\n";
    op1f_options_frame();
    assert(op1f_option(&LANES, 0) == 0 && op1f_option(&MARKERS, 0) == 2 && op1f_option(&MARKERS, 1) == 0);   // 9 is no choice: the first

    assert(strcmp(options_row_name(), "packs") == 0);
    options_row_draw(); op1f_options_frame();                                       // the hint, no page
    assert(!top_wanted);
    options_row_click(); op1f_options_frame();
    assert(top_wanted && page_up && cursor == 1);                                   // the cursor on lanes' first option
    key(OP1F_DOWN, 54, 0); op1f_options_frame();                                    // a click: LCARS lanes back on
    assert(op1f_option(&LANES, 0) == 1 && told_count == 1 && told[0][0] == 0 && told[0][1] == 1);
    key(OP1F_TURN, 0, 1024); op1f_options_frame();                                  // the first knob: past markers' heading to snap
    assert(rows[cursor].slot == 1 && rows[cursor].option == 0);
    key(OP1F_TURN, 1, 700); key(OP1F_TURN, 1, 700); op1f_options_frame();           // the second knob, one step in two turns
    assert(op1f_option(&MARKERS, 0) == 3);
    key(OP1F_TURN, 1, 1024); op1f_options_frame();                                  // a list goes round
    assert(op1f_option(&MARKERS, 0) == 0);
    key(OP1F_TURN, 1, -1400); op1f_options_frame();                                  // 376 left from before: one step back
    assert(op1f_option(&MARKERS, 0) == 3);
    key(OP1F_TURN, 0, 1024); key(OP1F_TURN, 0, 1024); op1f_options_frame();         // the last option holds the cursor
    assert(rows[cursor].slot == 1 && rows[cursor].option == 1);
    if (const char* out = getenv("OPTIONS_PAGE")) {                                 // the page as painted
        key(OP1F_TURN, 0, -1024); op1f_options_frame();
        FILE* f = fopen(out, "wb");
        for (int y = 0; y < 224; ++y) for (int x = 0; x < 480; ++x) fwrite(&mock_frame[y + ((479 - x) << 8)], 2, 1, f);
        fclose(f);
    }
    key(OP1F_DOWN, 0, 0); op1f_options_frame();                                     // help: closed, written
    assert(!top_wanted && !page_up);
    const std::string& kept_text = mock_disk["user/pf/.options"];
    assert(kept_text == "lanes/LCARS lanes=1\nmarkers/snap=3\nmarkers/debug keys=0\ngone/thing=1\n");
    printf("options host test: ok\n");
    return 0;
}

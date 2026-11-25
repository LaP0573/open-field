// Host test of the base's working memory (module/build_r5_memory.cpp): base 39 takes 32 memory asks of the packs in
// all (16 before), the thirty-third and on refused with their line; the base's own asks (op1f_memory_more) stay at 16.
//   em++ -std=c++11 -DOP1F_HOST -I module -I tools/tests/host tools/tests/host/memory_test.cpp -o <out>.js && node <out>.js
// tools/tests/test_memory.py runs it. VERBOSE=1 prints the base's lines.
#include <cassert>
#include <string>
#include <vector>
#include "../../../module/build_r5_memory.cpp"

static bool verbose = getenv("VERBOSE") != 0;
static std::vector<std::string> lines;
void op1f_log(int, const char*, int, const char* format, ...) {
    char b[300]; va_list a; va_start(a, format); vsnprintf(b, sizeof b, format, a); va_end(a);
    lines.push_back(b); if (verbose) fprintf(stderr, "%s\n", b);
}
Op1fCards op1f_cards;
int op1f_cards_verdict(unsigned) { return 0; }

enum { PACKS = 3, ASKS = 12 };                           // 36 asks: felt's 4 and bows' 2 would have been 6 of them
static Op1fCard cards[PACKS];
static Op1fAsk asks[PACKS][ASKS];
static Op1fMemory tables[PACKS][ASKS];

static unsigned count(const char* part) { unsigned n = 0; for (auto& l : lines) n += l.find(part) != std::string::npos; return n; }

int main() {
    for (unsigned p = 0; p < PACKS; ++p) {
        Op1fCard* c = &cards[p];
        c->mark = OP1F_CARD_MARK; c->bytes = OP1F_CARD_LEVEL_6_BYTES; c->level = 6;
        snprintf(c->name, sizeof c->name, "pack%u", p); c->asks = asks[p]; c->ask_count = ASKS;
        for (unsigned k = 0; k < ASKS; ++k) {
            Op1fMemory* t = &tables[p][k];
            t->head.bytes = sizeof *t; snprintf(t->head.name, OP1F_KIND_NAME, "m%u", k); t->bytes = 4096; t->align = 32;
            asks[p][k].kind = OP1F_ASK_MEMORY; asks[p][k].bytes = sizeof *t; asks[p][k].what = t;
        }
        op1f_cards.slots[p].card = c;
    }
    op1f_cards.count = PACKS;
    op1f_memory_frame();                                 // the registration
    op1f_memory_frame();                                 // the clearing: 32 x 4 KiB fit in one frame's quarter megabyte
    assert(count("is given 4096 bytes") == 32);
    assert(count("it would be the thirty-third") == 4);
    assert(count("seventeenth") == 0);
    unsigned given = 0;
    for (unsigned p = 0; p < PACKS; ++p)
        for (unsigned k = 0; k < ASKS; ++k) {
            void* m = op1f_memory(&cards[p], k);
            assert(((unsigned long)m & 31) == 0);
            given += m != 0;
            assert((m != 0) == (p * ASKS + k < 32));     // the first 32 in the cards' order, the rest refused
        }
    assert(given == 32);
    assert(op1f_memory_state(2, 7) == 1 && op1f_memory_state(2, 8) == -1);
    for (unsigned i = 0; i < 16; ++i) assert(op1f_memory_more(1024, 4, "test") > 0);   // the base's own: 16, as before
    assert(op1f_memory_more(1024, 4, "test") == OP1F_FAILED);
    assert(count("the base has asked sixteen blocks already") == 1);
    printf("memory host test: ok\n");
    return 0;
}

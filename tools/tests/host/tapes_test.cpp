// Host test of the base's files per tape (module/build_r7_tapes.cpp, base 44): the unit's tape copy and a tape made anew carried
// into the folder a card's ask of kind OP1F_ASK_TAPES names, and the ask's `changed` told.
// The card header holds the sizes of four-byte addresses, so it is built for a 32-bit target: emscripten and node,
//   em++ -std=c++11 -DOP1F_HOST -I module -I tools/tests/host tools/tests/host/tapes_test.cpp -o <out>.js && node <out>.js
// tools/tests/test_tapes.py runs it so where both are at hand. VERBOSE=1 prints the base's lines.
#include <cassert>
#include "../../../module/build_r7_tapes.cpp"

static unsigned told[32][3], told_count;
static void a_changed(unsigned how, unsigned from, unsigned to) { told[told_count][0] = how; told[told_count][1] = from; told[told_count++][2] = to; }
static const Op1fTapes A_TAPES = { { sizeof(Op1fTapes), "markers", 0, 0 }, 0, a_changed };
static const Op1fTapes B_TAPES = { { sizeof(Op1fTapes), "trains", 0, 0 }, 1, 0 };
static const Op1fTapes C_TAPES = { { sizeof(Op1fTapes), "old", 0, 0 }, 0, a_changed };
static const Op1fAsk A_ASKS[] = { { OP1F_ASK_TAPES, sizeof(Op1fTapes), &A_TAPES } };
static const Op1fAsk B_ASKS[] = { { OP1F_ASK_KEEPS, 4, 0 }, { OP1F_ASK_TAPES, sizeof(Op1fTapes), &B_TAPES } };
static const Op1fAsk C_ASKS[] = { { OP1F_ASK_TAPES, sizeof(Op1fTapes), &C_TAPES } };
static Op1fCard A, B, C;
Op1fCards op1f_cards;
u32 op1f_cards_used() { return 7; }
// base 42: the second list, 128 slots; under N1 the base copies the first into it (op1f_cards_lists)
Op1fCards2 op1f_cards_2;
void op1f_cards_lists() { op1f_cards_2.count = op1f_cards.count; op1f_cards_2.more = op1f_cards.more;
    for (unsigned i = 0; i < (unsigned)OP1F_CARDS_MOST; ++i) op1f_cards_2.slots[i] = op1f_cards.slots[i]; }
int op1f_cards_in_use(unsigned slot) { return slot < 32 && ((op1f_cards_used() >> slot) & 1u); }
// The folders unit's answer: a/0 user/pf/markers/, b/0 songs (asks nothing), b/1 trains; c has none.
int op1f_path(const Op1fCard* card, unsigned folder, const char* name, char* out, unsigned room) {
    const char* p = card == &A && folder == 0 ? "user/pf/markers/" : card == &B && folder == 1 ? "user/pf/trains/" : 0;
    if (!p || name) return OP1F_NO_FOLDER;
    strncpy(out, p, room); return (int)strlen(p);
}

static void card(Op1fCard* c, const char* name, unsigned level, const Op1fAsk* asks, unsigned count) {
    memset(c, 0, sizeof *c); c->mark = OP1F_CARD_MARK; c->bytes = OP1F_CARD_LEVEL_7_BYTES; c->level = level;
    strncpy(c->name, name, sizeof c->name - 1); c->asks = asks; c->ask_count = count;
}

int main() {
    card(&A, "a", 7, A_ASKS, 1); card(&B, "b", 10, B_ASKS, 2); card(&C, "c", 6, C_ASKS, 1);    // c: level 6, the ask is not read
    op1f_cards.count = 3; op1f_cards.slots[0].card = &A; op1f_cards.slots[1].card = &B; op1f_cards.slots[2].card = &C;
    mock_make_folders("user/pf/markers/"); mock_make_folders("user/pf/trains/"); mock_make_folders("user/pf/songs/");
    std::string big(50000, 'x'); big[49999] = 'e';                                  // miditracks' notes: many chunks
    mock_disk["user/pf/markers/tape1.txt"] = "op1f markers 1\n";
    mock_disk["user/pf/markers/tape1.mtk"] = std::string("MTK1\0\0bin", 9);
    mock_disk["user/pf/markers/tape10.txt"] = "ten";
    mock_disk["user/pf/markers/song.mid"] = "m";
    mock_disk["user/pf/markers/tape3.txt"] = "stale";
    mock_disk["user/pf/markers/tape3.old"] = "stale";
    mock_make_folders("user/pf/markers/tape1.dir/");                               // a folder inside: left alone
    mock_disk["user/pf/trains/tape1.txt"] = big;
    mock_disk["user/pf/songs/tape1.txt"] = "not asked for";
    op1f_tapes_frame();
    assert(told_count == 0);

    op1f_tapes_note(CLONE, 0, 2);                                                   // tape 1 -> tape 3
    op1f_tapes_frame();
    assert(mock_disk["user/pf/markers/tape3.txt"] == "op1f markers 1\n");
    assert(mock_disk["user/pf/markers/tape3.mtk"] == mock_disk["user/pf/markers/tape1.mtk"] && mock_disk["user/pf/markers/tape3.mtk"].size() == 9);
    assert(!mock_disk.count("user/pf/markers/tape3.old"));                        // no source: removed
    assert(mock_disk["user/pf/markers/tape10.txt"] == "ten" && !mock_disk.count("user/pf/markers/tape30.txt"));
    assert(mock_disk["user/pf/trains/tape3.txt"] == big);
    assert(!mock_disk.count("user/pf/songs/tape3.txt"));                          // a folder no ask names
    assert(!mock_disk.count("tmp/op1f_tape.part") && !mock_is_folder("user/pf/markers/tape3.dir"));
    assert(told_count == 1 && told[0][0] == OP1F_TAPES_COPIED && told[0][1] == 1 && told[0][2] == 3);   // a told, b has no changed, c not read
    op1f_tapes_frame();
    assert(told_count == 1);                                                        // nothing new

    mock_mounted = 2; op1f_tapes_made();                                            // the copy loaded (flag 1): no delete
    op1f_tapes_frame();
    assert(told_count == 1 && mock_disk.count("user/pf/markers/tape3.txt"));

    mock_disk_open = 1; mock_mounted = 0; op1f_tapes_made();                        // tape 1 made anew in the disk mode: waits
    op1f_tapes_frame();
    assert(told_count == 1 && mock_disk.count("user/pf/markers/tape1.txt"));
    mock_disk_open = 0;
    op1f_tapes_frame();
    assert(told_count == 2 && told[1][0] == OP1F_TAPES_DELETED && told[1][1] == 1 && told[1][2] == 1);
    assert(!mock_disk.count("user/pf/markers/tape1.txt") && !mock_disk.count("user/pf/markers/tape1.mtk") && !mock_disk.count("user/pf/trains/tape1.txt"));
    assert(mock_disk["user/pf/markers/tape10.txt"] == "ten" && mock_disk.count("user/pf/markers/tape3.txt") && mock_disk.count("user/pf/songs/tape1.txt"));

    mock_disk["user/pf/markers/tape3.old"] = "stale";                             // tape 3 made anew later (the copy long carried)
    mock_mounted = 2; op1f_tapes_made();
    op1f_tapes_frame();
    assert(told_count == 3 && told[2][0] == OP1F_TAPES_DELETED && told[2][1] == 3 && !mock_disk.count("user/pf/markers/tape3.old"));
    mock_mounted = -1; op1f_tapes_made();                                           // no tape mounted: nothing noted
    op1f_tapes_frame();
    assert(told_count == 3);

    op1f_tapes_note(CLONE, 9, 1);                                                   // out of range: not noted
    op1f_tapes_frame();
    assert(told_count == 3);
    told_count = 0;
    for (int i = 0; i < 20; ++i) op1f_tapes_note(DELETE, 4, 4);                     // more than the ring: 4 passed over
    op1f_tapes_frame();
    assert(carried == noted && noted == 23 && told_count == RING);
    puts("tapes host test: ok");
}

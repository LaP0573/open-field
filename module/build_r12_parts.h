// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R12, base.pack 36: which parts of a held page's document to hide while the firmware draws each rectangle the
// page named again (the cards unit, module/build_r6d_cards.cpp, includes this file).
//
// A page that asks op1f_page_parts names rectangles of its picture and, for each, the parts of its document that lie
// there (op1f_page_redraw, module/build_r6e_card.h, "Base 36"). For each rectangle the firmware's render draws the
// document once, cut to that rectangle; around that drawing the base hides every part that is not to be drawn there
// (it keeps each one's display byte, writes 0, lets the render draw, and puts the bytes back). This file works out
// those parts, for all the rectangles in one walk of the document.
//
// A document is a tree of nodes. What this file reads of a node: its children, a vector of node pointers, its begin
// at +0x48 and its end at +0x4c; its display byte at +0x5c, 0 for hidden (the node and everything under it are not
// drawn).
//
// For one rectangle the drawing shows its named parts, each with everything under it as its own display bytes say,
// and the groups that hold them. So under each node that holds a named part of the rectangle below it, the plan hides
// every child that is shown, is not a named part of the rectangle and holds none below it:
//   - under a named part nothing is hidden for that rectangle, so a part named under another one adds nothing;
//   - the root is never hidden, and is walked whatever its own display byte (when it is 0 nothing is drawn anyway);
//   - a named part is found only under shown nodes: one under a hidden node is not found; one that is hidden itself
//     is, and the groups that hold it are drawn;
//   - a rectangle none of whose named parts is found has every shown child of the root hidden;
//   - a child already hidden is not listed (it is not drawn either way), and no node is listed twice for a rectangle.
//
// Plain integer code with nothing called and nothing divided. The named parts are only compared as pointers, never
// read, so a pointer that is not a node does no harm. Which rectangles name a node is a small table of pointers (512
// slots for at most 160 parts, a mask of the rectangles for each), so one walk serves all the rectangles: for each
// node it meets, the walk looks it up once and reads its display byte and its children's two words. The walk keeps
// its own stack (no recursion) and enters only shown nodes with children.

#ifndef OP1F_BUILD_R12_PARTS_H
#define OP1F_BUILD_R12_PARTS_H

// The most a frame may name (module/build_r6e_card.h, "Base 36"), named here where that header is not read.
#ifndef OP1F_BUILD_R6E_CARD_H
enum { OP1F_REDRAWS_MOST = 8, OP1F_REDRAW_PARTS_MOST = 160 };
#endif

// A node, read as bytes. The firmware's words are 32-bit pointers: the plan reads them as pointers of its own, so it
// is built for a 32-bit target only (the module, and the PC's tests).
typedef const unsigned char* Op1fPlanNode;
typedef char op1f_plan_on_32_bits[sizeof(void*) == 4 ? 1 : -1];

enum { OP1F_PLAN_BEGIN = 0x48, OP1F_PLAN_END = 0x4c, OP1F_PLAN_DISPLAY = 0x5c };

static inline Op1fPlanNode const* op1f_plan_begin(Op1fPlanNode n) { return *(Op1fPlanNode const* const*)(n + OP1F_PLAN_BEGIN); }
static inline Op1fPlanNode const* op1f_plan_end(Op1fPlanNode n) { return *(Op1fPlanNode const* const*)(n + OP1F_PLAN_END); }
static inline unsigned op1f_plan_shown(Op1fPlanNode n) { return n[OP1F_PLAN_DISPLAY]; }

// The limits: hides in all the rectangles together; groups on the walk's stack, the root the first; the table's slots
// (a power of two, 2 ^ OP1F_PLAN_SLOT_BITS).
enum { OP1F_PLAN_HIDES_MOST = 2048, OP1F_PLAN_DEPTH = 24, OP1F_PLAN_SLOT_BITS = 9, OP1F_PLAN_SLOTS = 512 };

// What op1f_plan_parts answers. Anything but OP1F_PLAN_MADE: the plan is not to be used, and the caller draws the page
// whole.
//   OP1F_PLAN_MADE     the plan is made
//   OP1F_PLAN_ASKED    not an ask the plan takes: no root, more than OP1F_REDRAWS_MOST rectangles or
//                      OP1F_REDRAW_PARTS_MOST parts, a rectangle's parts past the parts named, a part that is 0
//   OP1F_PLAN_MANY     more than OP1F_PLAN_HIDES_MOST hides in all the rectangles together
//   OP1F_PLAN_DEEP     a shown group to walk lies deeper than OP1F_PLAN_DEPTH groups (a document whose children lead
//                      back to a node above them ends here too)
//   OP1F_PLAN_TANGLED  more named parts found than were named: a node reached by two ways, not a tree
enum { OP1F_PLAN_MADE = 0, OP1F_PLAN_ASKED = 1, OP1F_PLAN_MANY = 2, OP1F_PLAN_DEEP = 3, OP1F_PLAN_TANGLED = 4 };

// A rectangle the page named.
//   x0, x1, y0, y1  in: the rectangle in pixels, cut to the picture, ends exclusive (the caller's; the plan does not
//                   read them)
//   first, count    in: its named parts, part[first] to part[first + count - 1] of the plan
//   hide, hides     out: the nodes to hide for its drawing, hide[hide] to hide[hide + hides - 1] of the plan, in the
//                   order the walk found them
struct Op1fPlanRect {
    unsigned x0, x1, y0, y1;
    unsigned first, count;
    unsigned hide, hides;
};

// A group on the walk's stack.
//   at, begin, end  its children's vector: the next child's slot, the first, the end
//   active          the rectangles walked under it: none of them names it or a group above it
//   below           of those, the rectangles with a named part found under it
//   named           the rectangles that name it, of those walked under its parent
//   found           the length of the plan's `found` when it was entered: its own children's entries follow
struct Op1fPlanFrame {
    Op1fPlanNode const* at;
    Op1fPlanNode const* begin;
    Op1fPlanNode const* end;
    unsigned active, below, named, found;
};

// A child kept for some rectangles, until its group is done: its slot in the group's vector, and the rectangles for
// which it is drawn (it is named, or a named part was found under it).
struct Op1fPlanFound {
    Op1fPlanNode const* slot;
    unsigned keep;
};

// The plan: the rectangles and their parts in, the nodes to hide out, and the walk's own room. About 17 KB.
//   rects, rect    in: the rectangles named, at most OP1F_REDRAWS_MOST
//   parts, part    in: their named parts, at most OP1F_REDRAW_PARTS_MOST, each rectangle a run of them
//   hides, hide    out: the nodes to hide, for all the rectangles together (each rectangle's run in `rect`)
//   the rest       the walk's own: the table (`key`, `mask`), each hide's rectangle and then its place (`tag`), the
//                  stack, the children kept
struct Op1fPlan {
    unsigned rects;
    Op1fPlanRect rect[OP1F_REDRAWS_MOST];
    unsigned parts;
    const void* part[OP1F_REDRAW_PARTS_MOST];
    unsigned hides;
    Op1fPlanNode hide[OP1F_PLAN_HIDES_MOST];
    unsigned key[OP1F_PLAN_SLOTS];
    unsigned char mask[OP1F_PLAN_SLOTS];
    unsigned short tag[OP1F_PLAN_HIDES_MOST];
    Op1fPlanFrame frame[OP1F_PLAN_DEPTH];
    Op1fPlanFound found[OP1F_REDRAW_PARTS_MOST];
};

// A pointer's first slot in the table: the top bits of its product with 2 ^ 32 / the golden ratio.
static inline unsigned op1f_plan_slot(unsigned word) {
    return (word * 0x9e3779b1u) >> (32 - OP1F_PLAN_SLOT_BITS);
}

// The rectangles that name the node at `word` (not 0): their mask, 0 for none.
static inline unsigned op1f_plan_named(const Op1fPlan* p, unsigned word) {
    unsigned s = op1f_plan_slot(word);
    for (;;) {
        unsigned k = p->key[s];
        if (k == word) return p->mask[s];
        if (k == 0) return 0;
        s = (s + 1) & (unsigned)(OP1F_PLAN_SLOTS - 1);
    }
}

// Makes the plan for the document under `root` from the rectangles and parts in `p`: fills `hides`, `hide` and each
// rectangle's `hide` and `hides`. Reads the document, writes nothing into it. Answers OP1F_PLAN_MADE or a failure.
static inline unsigned op1f_plan_parts(Op1fPlan* p, Op1fPlanNode root) {
    p->hides = 0;
    unsigned rects = p->rects, parts = p->parts;
    if (root == 0 || rects > (unsigned)OP1F_REDRAWS_MOST || parts > (unsigned)OP1F_REDRAW_PARTS_MOST) return OP1F_PLAN_ASKED;

    // The table: each named part with the mask of the rectangles that name it.
    for (unsigned s = 0; s < (unsigned)OP1F_PLAN_SLOTS; ++s) {
        p->key[s] = 0;
        p->mask[s] = 0;
    }
    for (unsigned r = 0; r < rects; ++r) {
        Op1fPlanRect* q = &p->rect[r];
        q->hide = 0;
        q->hides = 0;
        if (q->first > parts || q->count > parts - q->first) return OP1F_PLAN_ASKED;
        for (unsigned i = q->first; i < q->first + q->count; ++i) {
            unsigned word = (unsigned)p->part[i];
            if (word == 0) return OP1F_PLAN_ASKED;
            unsigned s = op1f_plan_slot(word);
            while (p->key[s] != 0 && p->key[s] != word) s = (s + 1) & (unsigned)(OP1F_PLAN_SLOTS - 1);
            p->key[s] = word;
            p->mask[s] = (unsigned char)(p->mask[s] | (1u << r));
        }
    }

    // The walk. A rectangle that names the root draws it whole: nothing hidden for it.
    unsigned active = ((1u << rects) - 1u) & ~op1f_plan_named(p, (unsigned)root);
    if (active == 0) return OP1F_PLAN_MADE;
    unsigned n = 0, found = 0, depth = 1;
    Op1fPlanFrame* f = &p->frame[0];
    f->at = f->begin = op1f_plan_begin(root);
    f->end = op1f_plan_end(root);
    f->active = active;
    f->below = 0;
    f->named = 0;
    f->found = 0;
    for (;;) {
        if (f->at != f->end) {
            // The group's next child: entered when it is shown, has children and some rectangle walked here does not
            // name it; else kept for the rectangles that name it.
            Op1fPlanNode const* slot = f->at++;
            Op1fPlanNode c = *slot;
            if (c == 0) continue;
            unsigned named = op1f_plan_named(p, (unsigned)c) & f->active;
            if (op1f_plan_shown(c) != 0) {
                unsigned under = f->active & ~named;
                Op1fPlanNode const* b = op1f_plan_begin(c);
                Op1fPlanNode const* e = op1f_plan_end(c);
                if (under != 0 && b != e) {
                    if (depth == (unsigned)OP1F_PLAN_DEPTH) return OP1F_PLAN_DEEP;
                    ++depth;
                    ++f;
                    f->at = f->begin = b;
                    f->end = e;
                    f->active = under;
                    f->below = 0;
                    f->named = named;
                    f->found = found;
                    continue;
                }
            }
            if (named != 0) {
                if (found == (unsigned)OP1F_REDRAW_PARTS_MOST) return OP1F_PLAN_TANGLED;
                p->found[found].slot = slot;
                p->found[found].keep = named;
                ++found;
                f->below |= named;
            }
            continue;
        }

        // The group is walked. For each rectangle it holds a part of (the root: each rectangle walked), its shown
        // children not kept for that rectangle are hidden.
        unsigned need = depth == 1 ? f->active : f->below;
        if (need != 0) {
            unsigned k = f->found;
            for (Op1fPlanNode const* slot = f->begin; slot != f->end; ++slot) {
                unsigned keep = 0;
                if (k < found && p->found[k].slot == slot) keep = p->found[k++].keep;
                Op1fPlanNode c = *slot;
                unsigned hide = need & ~keep;
                if (c == 0 || hide == 0 || op1f_plan_shown(c) == 0) continue;
                for (unsigned r = 0; hide != 0; ++r, hide >>= 1) {
                    if ((hide & 1u) == 0) continue;
                    if (n == (unsigned)OP1F_PLAN_HIDES_MOST) return OP1F_PLAN_MANY;
                    p->hide[n] = c;
                    p->tag[n] = (unsigned short)r;
                    ++n;
                }
            }
        }
        found = f->found;
        if (depth == 1) break;
        unsigned keep = f->named | f->below;
        --depth;
        --f;
        if (keep != 0) {
            if (found == (unsigned)OP1F_REDRAW_PARTS_MOST) return OP1F_PLAN_TANGLED;
            p->found[found].slot = f->at - 1;
            p->found[found].keep = keep;
            ++found;
            f->below |= keep;
        }
    }

    // Each rectangle's hides together, in the walk's order: counted, given their places (`tag` from the rectangle to
    // the place), then moved there in place, a cycle at a time.
    for (unsigned i = 0; i < n; ++i) ++p->rect[p->tag[i]].hides;
    unsigned at = 0;
    for (unsigned r = 0; r < rects; ++r) {
        p->rect[r].hide = at;
        at += p->rect[r].hides;
        p->rect[r].hides = 0;
    }
    for (unsigned i = 0; i < n; ++i) {
        Op1fPlanRect* q = &p->rect[p->tag[i]];
        p->tag[i] = (unsigned short)(q->hide + q->hides);
        ++q->hides;
    }
    for (unsigned i = 0; i < n; ++i) {
        while ((unsigned)p->tag[i] != i) {
            unsigned j = p->tag[i];
            Op1fPlanNode c = p->hide[j];
            p->hide[j] = p->hide[i];
            p->hide[i] = c;
            unsigned short t = p->tag[j];
            p->tag[j] = p->tag[i];
            p->tag[i] = t;
        }
    }
    p->hides = n;
    return OP1F_PLAN_MADE;
}

#endif

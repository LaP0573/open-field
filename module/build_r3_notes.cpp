// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R3, the base pack (base.pack 20): the notes heard, level 5's second kind of ask (OS 1.7.0+0). A unit of its
// own, new at R3.
//
// Any pack may ask, through its card, to be told every note the instrument is asked to begin and to end, whoever
// asks: the keyboard, MIDI, a sequencer of the firmware's or of a pack's (OP1F_ASK_NOTES, its table an Op1fNotes,
// module/build_r3_card.h). The firmware's two note functions begin with a jump to the two entries of
// module/build_r3_notes.s, placed by the base's table; an entry hands its note to op1f_notes_heard_on or
// op1f_notes_heard_off below, and the firmware's function then runs as it always did.
//
// The registration, once, at the base's first frame (op1f_notes_frame, from the cards unit, on the display thread):
// every card the base uses (op1f_cards_verdict), in the list's order, is looked through for asks of this kind, each
// in its card's order. An ask is held to its form: its table's bytes at least an Op1fNotes' and the same as its
// head says, its name one to fifteen characters of a-z, 0-9, '-' and '_' with its end, no flag, and its `note` a
// function of its own pack (inside the pack, on two bytes). An ask not in order is left aside with a line, and its
// card is still used. Eight asks in all are heard; a ninth is left aside with a line. The listeners' table is
// written whole before the word that says how many there are, op1f_notes_listening, which the entries read: until
// it is written, nobody listens, and a note costs an entry a load and a test.
//
// A note heard: a note outside 0 to 127 is dropped (the firmware would stop at it). The instrument the unit is on is
// read from the firmware's own word (0 the synth, 1 the drum, another number while it is on neither, when the note
// goes out over MIDI only). A begin's velocity of -1 is the firmware's 127, any other is held to 1 to 127; an end's
// is 0. Each listener is called in the order of the registration with an Op1fNote of its own, inside a record of
// the cards unit's of kind OP1F_CALL_LISTEN, so that a listener's call of a helper that plays notes is refused. The
// two functions run on the thread that asks for the note (the key thread, the sequencer's, a MIDI handler's and
// others), and one may interrupt another: they keep nothing but what lies on their own stack, share nothing but the
// counts of each listener's notes (each counted with interrupts masked), call nothing of the firmware's and say no
// line. The display thread reads and clears the counts each period, and says one line for each listener that heard
// anything, through a quiet gate of this unit's own.
//
// Nothing is divided (the module links no runtime). Compiled for the PC as it is, with OP1F_HOST defined: the
// stand-ins come in through a header of the PC's own (for tests this tree does not hold).

#include "build_r3_base.h"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"     // the gate's half for the power popup: this unit is handed no frame to see it by
#endif
#include "build_q_quiet.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

typedef unsigned int u32;
typedef unsigned char u8;

#ifdef OP1F_HOST
#include "mock_notes_r3.h"                             // the PC's stand-ins: the word read, the masks
#define MASK_OFF(mask) ((void)((mask) = mock_mask()))
#define MASK_ON(mask) mock_unmask(mask)
#define BARRIER() ((void)0)                            // the PC's run is one thread at a time
#else
#define RD32(a) (*(volatile u32*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
// The compiler moves no load or store across this: the table is stored before the word that says how much of it
// there is, and read after it.
#define BARRIER() asm volatile("" : : : "memory")
#endif

static const u32 INSTRUMENT = 0x84d21088u;             // the instrument the unit is on: 0 the synth, 1 the drum, another number on neither

enum { MOST = OP1F_NOTES_MOST, CARDS = OP1F_SLOTS_MOST, NAME = OP1F_KIND_NAME, TABLE_BYTES = 32, CARD_4_BYTES = OP1F_CARD_LEVEL_4_BYTES,
       NOTE_MOST = 127, VELOCITY_MOST = 127, PERIOD_FRAMES = 256 };
static_assert(sizeof(Op1fNotes) == TABLE_BYTES, "the table of the notes heard is 32 bytes at level 5");

static const char LOG_FILE[] = "../src/main/op1f_notes.cpp";

// R20 (base 42): the cards unit's list is op1f_cards_2 (module/build_r3_base.h), up to OP1F_SLOTS_MOST slots,
// read past op1f_cards_lists (or a name of the cards unit's that calls it).

// How many listeners the table holds: 0 until the registration, then written once. The entries read it first.
extern "C" volatile u32 op1f_notes_listening OP1F_NAMED("op1f_notes_listening");
volatile u32 op1f_notes_listening = 0;

struct Listener {
    void (*note)(const Op1fNote* event);
    u32 slot;                                          // the card's slot in op1f_cards
    char name[NAME];                                   // the ask's name, for the lines
};

static Listener listeners[MOST];
static volatile u32 begun[MOST], ended[MOST];          // each listener's notes since the period began, counted masked

static struct {
    u32 enrolled;                                      // the registration was made
    int countdown;                                     // frames to the period's end
    QuietGate gate;
} notes;

// ---------------------------------------------------------------- a note heard, from any thread

// The note handed on to every listener. `begins` is OP1F_NOTE_BEGINS or OP1F_NOTE_ENDS.
static void heard(u32 note, int velocity, u32 no_midi, u32 begins) {
    u32 count = op1f_notes_listening;
    BARRIER();                                                                      // the table is read after the word
    if (note > (u32)NOTE_MOST || count == 0) return;                                // below 0 too, as an unsigned number
    if (count > (u32)MOST) count = MOST;
    u32 host = RD32(INSTRUMENT);
    u32 strength = 0;
    if (begins) strength = velocity == -1 ? (u32)VELOCITY_MOST : velocity < 1 ? 1u : velocity > VELOCITY_MOST ? (u32)VELOCITY_MOST : (u32)velocity;
    u32 flags = no_midi != 0 ? (u32)OP1F_NOTE_NO_MIDI : 0u;
    for (u32 i = 0; i < count; ++i) {
        Op1fNote event;                                                             // each listener its own: none sees what another did to it
        event.bytes = sizeof(Op1fNote);
        event.what = begins;
        event.host = host;
        event.note = note;
        event.velocity = strength;
        event.flags = flags;
        Op1fCallKept kept;
        op1f_cards_call_in(OP1F_CALL_LISTEN, listeners[i].slot, &kept);
        listeners[i].note(&event);
        op1f_cards_call_out(OP1F_CALL_LISTEN, &kept);
        u32 mask;
        MASK_OFF(mask);
        if (begins) ++begun[i]; else ++ended[i];
        MASK_ON(mask);
    }
}

// From the note on's entry: (note, velocity, the "no MIDI" word), as the firmware's note on was handed them.
extern "C" void op1f_notes_heard_on(u32 note, int velocity, u32 no_midi) OP1F_NAMED("op1f_notes_heard_on");
extern "C" void op1f_notes_heard_on(u32 note, int velocity, u32 no_midi) {
    heard(note, velocity, no_midi, OP1F_NOTE_BEGINS);
}

// From the note off's entry: the same three, as the firmware's note off was handed them.
extern "C" void op1f_notes_heard_off(u32 note, int velocity, u32 no_midi) OP1F_NAMED("op1f_notes_heard_off");
extern "C" void op1f_notes_heard_off(u32 note, int velocity, u32 no_midi) {
    heard(note, velocity, no_midi, OP1F_NOTE_ENDS);
}

// ---------------------------------------------------------------- the registration, on the display thread

// An ask's name: one to fifteen characters of a-z, 0-9, '-' and '_', ended within its sixteen bytes.
static int plain_name(const char* name) {
    u32 n = 0;
    for (; n < (u32)NAME && name[n]; ++n) {
        char c = name[n];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return n != 0 && n < (u32)NAME;
}

// Why an ask of this kind is left aside, or 0; `a` and `b` for the line when its table is not in order:
//   (the ask's bytes, the bytes of an Op1fNotes)     its table is too small, or there is none
//   (the bytes its head says, the ask's bytes)       the two differ
//   (its flags, 0)                                   a flag is set
//   (its function's place from the pack's first byte, the pack's bytes)   `note` is not a function of its pack
// `plain` says whether its name may be shown.
enum { IN_ORDER = 0, NOT_IN_ORDER = 1, NOT_A_NAME = 2 };
static int refused(const Op1fAsk* ask, const Op1fCardSlot* slot, u32* a, u32* b, int* plain) {
    *plain = 0;
    const Op1fNotes* table = (const Op1fNotes*)ask->what;
    if (table == 0 || ask->bytes < (u32)TABLE_BYTES) {                              // the card's check holds the table in its pack, with its bytes
        *a = ask->bytes;
        *b = TABLE_BYTES;
        return NOT_IN_ORDER;
    }
    if (!plain_name(table->head.name)) return NOT_A_NAME;
    *plain = 1;
    if (table->head.bytes != ask->bytes) {
        *a = table->head.bytes;
        *b = ask->bytes;
        return NOT_IN_ORDER;
    }
    if (table->head.flags != 0) {
        *a = table->head.flags;
        *b = 0;
        return NOT_IN_ORDER;
    }
    u32 note = (u32)table->note;
    if (note < slot->first || note >= slot->end || (note & 1u) != 0) {
        *a = note - slot->first;
        *b = slot->end - slot->first;
        return NOT_IN_ORDER;
    }
    return IN_ORDER;
}

static void enrol() {
    op1f_cards_lists();                                                             // R20 (base 42)
    u32 seen = op1f_cards_2.count > (u32)CARDS ? (u32)CARDS : op1f_cards_2.count;
    u32 n = 0;
    for (u32 i = 0; i < seen; ++i) {
        if (op1f_cards_verdict(i) != 0) continue;                                   // a card the base does not use
        const Op1fCardSlot* slot = &op1f_cards_2.slots[i];
        const Op1fCard* card = slot->card;
        if (card->bytes < (u32)CARD_4_BYTES || card->level < 4u || card->asks == 0) continue;
        for (u32 k = 0; k < card->ask_count; ++k) {
            const Op1fAsk* ask = &card->asks[k];
            if (ask->kind != (u32)OP1F_ASK_NOTES) continue;
            u32 a = 0, b = 0;
            int plain = 0, why = refused(ask, slot, &a, &b, &plain);
            const char* name = plain ? ((const Op1fNotes*)ask->what)->head.name : "?";
            if (why == NOT_A_NAME) {
                op1f_log(2, LOG_FILE, 2, "notes: '%s' asks to hear the notes as '%s', which %s: left aside", card->name, name, "is not a name the log can show");
                continue;
            }
            if (why == NOT_IN_ORDER) {
                op1f_log(2, LOG_FILE, 3, "notes: '%s' asks to hear the notes as '%s', which %s (%u, %u): left aside", card->name, name, "its table is not in order", a, b);
                continue;
            }
            if (n == (u32)MOST) {
                op1f_log(2, LOG_FILE, 2, "notes: '%s' asks to hear the notes as '%s', which %s: left aside", card->name, name, "would be the ninth");
                continue;
            }
            const Op1fNotes* table = (const Op1fNotes*)ask->what;
            Listener* l = &listeners[n++];
            l->note = table->note;
            l->slot = i;
            for (u32 c = 0; c < (u32)NAME; ++c) l->name[c] = table->head.name[c];
            op1f_log(3, LOG_FILE, 1, "notes: '%s' of '%s' hears the notes", l->name, card->name);
        }
    }
    BARRIER();                                                                      // the table is whole before the word that says so
    op1f_notes_listening = n;
}

// Once a frame, from the cards unit: the registration at the first call; each period, the lines.
extern "C" void op1f_notes_frame() {
    if (!notes.enrolled) {
        notes.enrolled = 1;
        notes.countdown = PERIOD_FRAMES;
        enrol();
        return;
    }
    u32 count = op1f_notes_listening;
    if (count == 0 || --notes.countdown > 0) return;
    notes.countdown = PERIOD_FRAMES;
    u32 b[MOST], e[MOST];
    int activity = 0;
    for (u32 i = 0; i < count; ++i) {                                               // read and cleared, whatever the gate says
        u32 mask;
        MASK_OFF(mask);
        b[i] = begun[i];
        e[i] = ended[i];
        begun[i] = 0;
        ended[i] = 0;
        MASK_ON(mask);
        if (b[i] != 0 || e[i] != 0) activity = 1;
    }
    if (!quiet_period(&notes.gate, quiet_mix(quiet_start(), count), activity)) return;
    for (u32 i = 0; i < count; ++i) {
        if (b[i] == 0 && e[i] == 0) continue;
        op1f_log(3, LOG_FILE, 4, "notes: '%s' this period: %u note(s) begun, %u ended", listeners[i].name, b[i], e[i]);
    }
}

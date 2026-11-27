// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R17, base 39: the checks of a drop into a synth sound's sample, as plain code, and the bound on what is put back
// at the start. The sample unit (module/build_r17_sample.cpp) fills a scene from the firmware's words and from its own,
// at the call (the key thread) and again in the unit's turn (the worker), and asks op1f_sample_check what stands in the
// way; the restore asks op1f_sample_restore_bytes how much of a kept file it may read. Nothing here reads the firmware,
// calls anything or divides: the same text is compiled for the unit and by the PC's compiler (for tests this tree does
// not hold).
//
// The order of the checks is the answers' table's (module/build_r6e_card.h, "Base 39"): not now, a bad name, not
// here, busy, empty, too large, failed; but for two places where a later check needs an earlier one to have passed:
// the list of lifted tracks is held to be in order before it is counted (a list out of order fails before "no
// track"), and the slot's object is held to be at its own pool before its capacity and channels are read for the
// bound (an object elsewhere fails before "too large").

#ifndef OP1F_BUILD_R17_SAMPLE_H
#define OP1F_BUILD_R17_SAMPLE_H

// The answers, by the header's numbers (module/build_r6e_card.h: OP1F_NOT_NOW, OP1F_BAD_NAME, OP1F_NOT_HERE,
// OP1F_BUSY, OP1F_EMPTY, OP1F_TOO_LARGE, OP1F_FAILED). Named in a struct of their own so that a unit that reads either
// header may include this one; the sample unit holds them to the header's.
struct Op1fSampleAnswer {
    enum { NOT_NOW = -3, BAD_NAME = -5, FAILED = -6, NOT_HERE = -8, BUSY = -9, EMPTY = -10, TOO_LARGE = -2 };
};

// The memory of a slot's sample, in halfwords: a synth sound's pool and a drum sound's (the drum's is for a later base).
// A slot holds its sample from its pool's first byte; the unit's file of a kept sample holds it from 16 bytes in.
enum { OP1F_SAMPLE_POOL = 529200, OP1F_SAMPLE_DRUM_POOL = 1764000, OP1F_SAMPLE_KEPT_FROM = 16 };

// How many lifted tracks the list may hold before the base holds it to be out of order (the tape has four, eight with
// `8 tracks`).
enum { OP1F_SAMPLE_TRACKS_MOST = 64 };

// A lifted track the unit's own renders take: 0 to 3. One of 4 or more they cannot take, and the base refuses it, unless
// a pack holds the import's entry (its first eight bytes then no longer check as the firmware's own).
enum { OP1F_SAMPLE_TRACKS_STOCK = 4 };

// The checksum of the import entry's (0x8f004204) first eight bytes as the firmware holds them, the same in OS 1.7.0 and
// 1.7.3: zlib's CRC-32 (op1f_sample_crc8), as the base's table holds a site's. The base holds the checksum, not the
// bytes, which are the firmware's own.
static const unsigned OP1F_SAMPLE_IMPORT_CRC = 0x6e415f42u;

// zlib's CRC-32 of eight bytes, as the base's table holds a site's: the reflected polynomial 0xedb88320, the register
// all ones at the start and inverted at the end; bit by bit, with no table, nothing divided.
static inline unsigned op1f_sample_crc8(const unsigned char* bytes) {
    unsigned crc = 0xffffffffu;
    for (unsigned i = 0; i < 8u; ++i) {
        crc ^= bytes[i];
        for (unsigned k = 0; k < 8u; ++k) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return crc ^ 0xffffffffu;
}

// Why a drop is refused, or 0: it may go on. Each has one answer (op1f_sample_answer) and its words for the log
// (op1f_sample_why_words).
enum {
    OP1F_SAMPLE_PASSED = 0,
    OP1F_SAMPLE_NOT_INPUT = 1,     // not inside the card's own input function, for an event on the key thread
    OP1F_SAMPLE_DISK = 2,          // the disk mode is open
    OP1F_SAMPLE_NO_PLACE = 3,      // a place that is no sound's
    OP1F_SAMPLE_DRUM = 4,          // a drum sound's place on a base that drops into synth sounds only (39); from base 43 a
                                   // drum place is taken when the ask is the card's drum engine (kind 12)
    OP1F_SAMPLE_NO_ASK = 5,        // no ask of kind engine with that number (of kind drum engine for a drum place, base 43)
    OP1F_SAMPLE_RESTORING = 6,     // the place's sample is being put back after the start
    OP1F_SAMPLE_NOT_RIDING = 7,    // the place does not ride that engine now
    OP1F_SAMPLE_UNTOLD = 8,        // a sound loaded there is not told yet
    OP1F_SAMPLE_TICKET = 9,        // a drop is under way (any pack's)
    OP1F_SAMPLE_QUEUE = 10,        // the unit's queue of clipboard work is not empty
    OP1F_SAMPLE_SOUND = 11,        // a copied sound where the unit would paste it: at most one item and a sound held
    OP1F_SAMPLE_LIST = 12,         // the list of lifted tracks is not in order
    OP1F_SAMPLE_NO_TRACK = 13,     // nothing lifted
    OP1F_SAMPLE_OBJECT = 14,       // the slot's sample is not at its own pool
    OP1F_SAMPLE_LARGE = 15,        // the cut, as the slot takes it, passes the pool
    OP1F_SAMPLE_TRACK = 16,        // a lifted track of 4 or more while the import's entry is the firmware's own
    OP1F_SAMPLE_POST = 17,         // the unit did not take the job (its queue's write place did not move)
    OP1F_SAMPLE_WHYS = 18
};

// What a place's ride says (the scene's `ride`).
enum { OP1F_SAMPLE_RIDES_NOT = 0, OP1F_SAMPLE_RIDES = 1, OP1F_SAMPLE_RIDES_UNTOLD = 2 };

// What the checks read, as plain values.
//   turn         0 at the call (the key thread); 1 in the unit's turn (the worker): `keyed` and `engine` are then taken
//                as 1, and the ticket and the queue are not looked at (the queue holds the drop itself)
//   keyed        1 when the call comes from inside the card's own input function, for an event on the key thread
//   disk         not 0 while the disk mode is open
//   place        as asked: 0 to 7 a synth sound, 8 to 15 a drum sound
//   engine       1 when the card's ask of that number is of kind engine
//   ride         OP1F_SAMPLE_RIDES when the place rides that ask's engine; OP1F_SAMPLE_RIDES_UNTOLD when it does but a
//                sound loaded there is not told yet; else OP1F_SAMPLE_RIDES_NOT
//   restoring    not 0 while the place's sample is being put back after the start
//   ticket       not 0 while a drop is under way (any pack's), queued, running or not yet said
//   queue_read, queue_write   the two places of the unit's queue of clipboard work: the same when it is empty
//   many         the clipboard's item test, the firmware's: not 0 when it holds more than one item
//   patch        the clipboard's word of a copied sound's size: not 0 when one is held
//   tracks, tracks_end   the list of lifted tracks: a word a track (0 to 3 the tape's, 4 to 7 `8 tracks`')
//   head         the import entry's first eight bytes as they stand now
//   data, pool   the slot object's first word (its sample) and the slot's own pool; data 0 when no object was found
//   capacity     the object's frames (+0xd768)
//   stereo       1 when its channels byte (+0xd770) is not 0, else 0
//   length       the clipboard's length in frames
//   drums        base 43: 1 when a drum place may be dropped into (the base has drum engines); `engine` and `ride` are
//                then the drum engine's for a drum place (kind 12, the drum unit's ride), and the bound is the drum's pool
struct Op1fSampleScene {
    unsigned turn, keyed, disk, place, engine, ride, restoring, ticket;
    unsigned queue_read, queue_write, many, patch;
    const unsigned* tracks;
    const unsigned* tracks_end;
    unsigned char head[8];
    unsigned data, pool, capacity, stereo, length;
    unsigned drums;
};

// The frames the unit's render writes into the slot: the cut's length, cut at the slot's capacity.
static inline unsigned op1f_sample_frames(unsigned length, unsigned capacity) {
    return length < capacity ? length : capacity;
}

// The frames a synth slot's pool holds at that channel count: 529,200, or 264,600 stereo.
static inline unsigned op1f_sample_room(unsigned stereo) {
    return (unsigned)OP1F_SAMPLE_POOL >> (stereo ? 1 : 0);
}

// The bound: 1 when what the render writes, min(length, capacity) frames at that channel count, stays inside the pool;
// 0 when it would pass it. No division: the frames shifted by the channels against the pool's halfwords, the frames held
// to the pool first so that the shift cannot carry out.
static inline int op1f_sample_fits(unsigned length, unsigned capacity, unsigned stereo) {
    unsigned n = op1f_sample_frames(length, capacity);
    if (n > (unsigned)OP1F_SAMPLE_POOL) return 0;
    return (n << (stereo ? 1 : 0)) <= (unsigned)OP1F_SAMPLE_POOL ? 1 : 0;
}

// 1 when the import's entry still checks as the firmware's own (no pack holds the place): its eight bytes' checksum is
// OP1F_SAMPLE_IMPORT_CRC. Anything else there (`8 tracks`' jump) checks otherwise.
static inline int op1f_sample_head_is_stock(const unsigned char* head) {
    return op1f_sample_crc8(head) == OP1F_SAMPLE_IMPORT_CRC ? 1 : 0;
}

// Base 43: the same bound against a pool of `pool` halfwords (a drum slot's, 1,764,000).
static inline int op1f_sample_fits_pool(unsigned length, unsigned capacity, unsigned stereo, unsigned pool) {
    unsigned n = op1f_sample_frames(length, capacity);
    if (n > pool) return 0;
    return (n << (stereo ? 1 : 0)) <= pool ? 1 : 0;
}

// The checks, in the table's order (above): why the drop cannot go on, or OP1F_SAMPLE_PASSED.
static inline unsigned op1f_sample_check(const Op1fSampleScene* s) {
    if (!s->turn && !s->keyed) return OP1F_SAMPLE_NOT_INPUT;
    if (s->disk) return OP1F_SAMPLE_DISK;
    if (s->place >= 16u) return OP1F_SAMPLE_NO_PLACE;
    if (s->place >= 8u && !s->drums) return OP1F_SAMPLE_DRUM;
    if (!s->turn && !s->engine) return OP1F_SAMPLE_NO_ASK;
    if (s->restoring) return OP1F_SAMPLE_RESTORING;
    if (s->ride == (unsigned)OP1F_SAMPLE_RIDES_UNTOLD) return OP1F_SAMPLE_UNTOLD;
    if (s->ride != (unsigned)OP1F_SAMPLE_RIDES) return OP1F_SAMPLE_NOT_RIDING;
    if (!s->turn && s->ticket) return OP1F_SAMPLE_TICKET;
    if (!s->turn && s->queue_read != s->queue_write) return OP1F_SAMPLE_QUEUE;
    if (s->many == 0 && s->patch != 0) return OP1F_SAMPLE_SOUND;
    if (s->tracks_end < s->tracks || (unsigned)(s->tracks_end - s->tracks) > (unsigned)OP1F_SAMPLE_TRACKS_MOST) return OP1F_SAMPLE_LIST;
    if (s->tracks_end == s->tracks) return OP1F_SAMPLE_NO_TRACK;
    if (s->data == 0 || s->data != s->pool) return OP1F_SAMPLE_OBJECT;
    if (!op1f_sample_fits_pool(s->length, s->capacity, s->stereo, s->place >= 8u ? (unsigned)OP1F_SAMPLE_DRUM_POOL : (unsigned)OP1F_SAMPLE_POOL))
        return OP1F_SAMPLE_LARGE;
    if (op1f_sample_head_is_stock(s->head))
        for (const unsigned* t = s->tracks; t != s->tracks_end; ++t)
            if (*t >= (unsigned)OP1F_SAMPLE_TRACKS_STOCK) return OP1F_SAMPLE_TRACK;
    return OP1F_SAMPLE_PASSED;
}

// The answer of a refusal: what op1f_sample_drop answers at the call, or `answer` holds after the unit's turn.
static inline int op1f_sample_answer(unsigned why) {
    switch (why) {
    case OP1F_SAMPLE_NOT_INPUT: case OP1F_SAMPLE_DISK: case OP1F_SAMPLE_RESTORING: return Op1fSampleAnswer::NOT_NOW;
    case OP1F_SAMPLE_NO_PLACE: case OP1F_SAMPLE_DRUM: case OP1F_SAMPLE_NO_ASK: return Op1fSampleAnswer::BAD_NAME;
    case OP1F_SAMPLE_NOT_RIDING: case OP1F_SAMPLE_UNTOLD: return Op1fSampleAnswer::NOT_HERE;
    case OP1F_SAMPLE_TICKET: case OP1F_SAMPLE_QUEUE: return Op1fSampleAnswer::BUSY;
    case OP1F_SAMPLE_SOUND: case OP1F_SAMPLE_NO_TRACK: return Op1fSampleAnswer::EMPTY;
    case OP1F_SAMPLE_LARGE: return Op1fSampleAnswer::TOO_LARGE;
    case OP1F_SAMPLE_LIST: case OP1F_SAMPLE_OBJECT: case OP1F_SAMPLE_TRACK: case OP1F_SAMPLE_POST: return Op1fSampleAnswer::FAILED;
    default: return 0;
    }
}

// The words the log says for a refusal (each under 64 characters).
static inline const char* op1f_sample_why_words(unsigned why) {
    switch (why) {
    case OP1F_SAMPLE_NOT_INPUT: return "not from its input function, for an event on the key thread";
    case OP1F_SAMPLE_DISK: return "the disk mode is open";
    case OP1F_SAMPLE_NO_PLACE: return "no sound has that place";
    case OP1F_SAMPLE_DRUM: return "a drum sound: this base drops into synth sounds only";
    case OP1F_SAMPLE_NO_ASK: return "no ask of kind engine (drum engine) with that number";
    case OP1F_SAMPLE_RESTORING: return "its sample is being put back after the start";
    case OP1F_SAMPLE_NOT_RIDING: return "the sound does not play that engine now";
    case OP1F_SAMPLE_UNTOLD: return "a sound loaded there is not told yet";
    case OP1F_SAMPLE_TICKET: return "a drop is under way";
    case OP1F_SAMPLE_QUEUE: return "the unit has clipboard work waiting";
    case OP1F_SAMPLE_SOUND: return "the clipboard holds a copied sound, no lifted cut";
    case OP1F_SAMPLE_LIST: return "the list of lifted tracks is not in order";
    case OP1F_SAMPLE_NO_TRACK: return "no cut of the tape is lifted";
    case OP1F_SAMPLE_OBJECT: return "the sound's sample is not where it should be";
    case OP1F_SAMPLE_LARGE: return "the cut, as the sound takes it, passes its memory";
    case OP1F_SAMPLE_TRACK: return "a lifted track of 5 to 8 that nothing renders";
    case OP1F_SAMPLE_POST: return "the unit did not take the job";
    default: return "";
    }
}

// The bytes the restore may read from a kept file into the pool, written from the pool's 16th byte on: the
// file's own bytes, never more than the pool holds behind those 16 bytes (529,200 - 8 halfwords, 1,058,384 bytes),
// and whole halfwords. A file of 1,058,416 bytes (a never-loaded slot's, stereo) gives 1,058,384.
static inline unsigned op1f_sample_restore_bytes(unsigned file_bytes) {
    unsigned most = ((unsigned)OP1F_SAMPLE_POOL - (unsigned)OP1F_SAMPLE_KEPT_FROM / 2u) * 2u;
    unsigned n = file_bytes < most ? file_bytes : most;
    return n & ~1u;
}

// A drop's request lost from the unit's queue. The ticket's states, as the sample unit keeps them:
// free; queued (armed, and posted once the call saw the queue take it); running (the base's job took it); ended (the
// job wrote the end, which the frame says and frees).
enum { OP1F_SAMPLE_FREE = 0, OP1F_SAMPLE_QUEUED = 1, OP1F_SAMPLE_RUNNING = 2, OP1F_SAMPLE_ENDED = 3 };

// 1 when the request of ticket `number` is lost, from what the frame reads in one masked instant, in this order: the
// ticket the call saw the queue take (`posted`, 0 while the post is still on its way), the queue's read and write
// places, the ticket's state. The unit's queue holds a request until its job is over, and the base's job moves the
// ticket from queued to running at its start, so an empty queue under a posted ticket still queued means the request
// was lost from the queue.
static inline int op1f_sample_lost(unsigned posted, unsigned number, unsigned queue_read, unsigned queue_write, unsigned state) {
    return posted != 0 && posted == number && queue_read == queue_write && state == (unsigned)OP1F_SAMPLE_QUEUED ? 1 : 0;
}

#endif

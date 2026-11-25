// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R17, base 39: the restore unit, new. A cut a pack dropped into a synth sound's sample is still there after the
// unit is switched off and on.
//
// At each drop the sample unit makes the firmware's mark, and the unit keeps the slot's sample as it keeps its
// sampler's: in user/audio/synth/synth_N.raw, N the slot from 0 (synth 1 is synth_0.raw), the sample from its 16th
// byte on. The unit puts that file back at the start only into a sound that plays its sampler; here the base reads it
// back for a sound that rides a pack's engine.
//
// What it does, all on the display thread:
//   the drop's end (op1f_sample_kept, from the sample unit's frame, which says the end): the slot's kept cut, its frames
//       and channels, handed to the values unit (op1f_keeps_sample_set), which keeps it in its store beside the ride and
//       writes the store as it writes it for a value (module/build_r6b_keeps.cpp); the engines unit's count of the
//       slot's loads noted. Nothing is written from the worker
//   a frame (op1f_sample_restore_frame, from the sample unit's frame, after its lines):
//     * at the first: each synth slot's kept cut, as the values unit read it from its store at its own first frame
//       (the same frame, before), and the loads counted so far
//     * the start's window, as the door's (module/build_r4c_door.cpp, base 31): from the second call (the first frame
//       is over) until 4 s after it, no file is read
//     * a sound loaded onto a slot (its loads count moved) ends its kept cut, before, during or after the put-back
//     * once the window is over, one slot at a time, synth 1 to 8: a slot whose cut is kept and that rides a pack's
//       engine (op1f_engines_ride answers 1; 2 waits for a later frame), its object at its own pool with the kept
//       channels and its file long enough for the cut: op1f_sample_restore_begin (0: a drop there is under way, a later
//       frame), the file opened and read into pool + 16 bytes, 64 KB a frame, never past the pool (the .h's bound,
//       op1f_sample_restore_bytes: at most 529,200 - 8 halfwords, whatever the file's length), then
//       op1f_sample_restore_end with the kept frames (0 when it was not put back whole). Otherwise the pool is left as
//       it is and the line says why. The disk mode opened during a read closes the file and the slot waits for it to
//       close, then is read again from the start. Before each 64 KB, what the read began on is looked at again: the slot
//       still rides that card's engine and is told, its loads count, its object, the object's sample at its pool, its
//       channels and its capacity as they were; any of them changed (a sound loaded there, which the firmware may read
//       into the pool) stops it: the file closed, nothing more written, op1f_sample_restore_end with 0, the kept cut
//       kept unless a load ended it
//   a drop that ended on a slot whose cut still waits: that slot's put-back is done (the pool holds the newer cut)
//
// Lines, at most two a frame, in the unit's own numbering (synth 1 to 8) and with the file's own name:
//   sample: synth <n>: put back <frames> frames from synth_<n - 1>.raw
//   sample: synth <n>: not put back (<why>)
//   sample: synth <n>: put back stopped: <what changed>
//   sample: synth <n>: the cut is kept no longer: a sound was loaded there
//   sample: synth <n>: the put-back waits: the disk mode opened
//
// The module links no runtime: nothing is divided. Compiled for the unit only.

#include "build_r6d_base.h"                            // base 29's words, the base's shared names, level 10's
#include "build_r17_sample.h"                          // R17: the pool, the bound of a kept file

typedef unsigned int u32;
typedef unsigned char u8;

extern "C" void op1f_sample_kept(unsigned slot, unsigned frames, unsigned stereo) OP1F_NAMED("op1f_sample_kept");
extern "C" void op1f_sample_restore_frame() OP1F_NAMED("op1f_sample_restore_frame");

// ---- from the sample unit ----
extern "C" int op1f_sample_restore_begin(unsigned slot) OP1F_NAMED("op1f_sample_restore_begin");
extern "C" void op1f_sample_restore_end(unsigned slot, unsigned frames) OP1F_NAMED("op1f_sample_restore_end");

// ---- from the values unit ----
extern "C" int op1f_keeps_sample_get(unsigned slot, unsigned* frames, unsigned* stereo) OP1F_NAMED("op1f_keeps_sample_get");
extern "C" void op1f_keeps_sample_set(unsigned slot, unsigned frames, unsigned stereo) OP1F_NAMED("op1f_keeps_sample_set");

// ---- from the engines unit, weak: away, no slot rides an engine and nothing is put back ----
extern "C" unsigned op1f_engines_ride(unsigned slot, unsigned* card, unsigned* ask, unsigned* loads) OP1F_NAMED("op1f_engines_ride") __attribute__((weak));

#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define RESTORE_APART __attribute__((noinline))

typedef int (*FileSizeFn)(const char* path);                                 // 0x8c051f9c: -1 when there is no file
typedef int (*OpenFn)(const char* path, const char* mode);                   // 0x8fcba988: a handle, -1 when it cannot be opened
typedef int (*ReadFn)(void* buffer, u32 size, u32 count, int handle);        // 0x8fcba954: the count read
typedef void (*CloseFn)(int handle);                                         // 0x8fcba8ac
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_OPEN ((OpenFn)0x8fcba988u)
#define STOCK_READ ((ReadFn)0x8fcba954u)
#define STOCK_CLOSE ((CloseFn)0x8fcba8acu)

// The firmware's words, as the sample unit names them.
enum : u32 {
    DISK_OPEN = 0x84d25bccu,            // not 0 while the disk mode is open
    SYNTH_HOLDERS = 0x0802f73cu,        // a synth slot's holder, a word a slot: its first word is the slot's object
    SYNTH_POOLS = 0x840b0becu,          // synth slot 0's pool; slot i's SYNTH_STRIDE x i further
    SYNTH_STRIDE = 0x102660u,           // 529,200 halfwords
    CAPACITY_AT = 0xd768u,              // the object's frames
    CHANNELS_AT = 0xd770u               // the object's channels byte: 0 mono
};

static const char LOG_FILE[] = "../src/main/op1f_sample_restore.cpp";
static const char READ_MODE[] = "rb";
static const char FOLDER[] = "user/audio/synth/";      // the file's folder, then its name: synth_<slot>.raw
static const char NAME_HEAD[] = "synth_";

enum { SLOTS = 8, CHUNK = 65536, LINES_A_FRAME = 2, START_SETTLE_MS = 4000, PATH = 40, NAME_AT = sizeof(FOLDER) - 1 };
enum { NONE = 0, WAITING = 1, READING = 2, DONE = 3 };

static struct {
    u32 calls;                                         // op1f_sample_restore_frame's calls
    u32 taken;                                         // the kept cuts were taken from the values unit
    u32 first_over_ms;                                 // op1f_ms at the second call: the first frame was over
    u32 settled;                                       // the start's window is over
    u32 state[SLOTS];                                  // NONE, WAITING (to be put back), READING, DONE
    u32 held[SLOTS];                                   // the slot's cut is kept (in the values unit's store)
    u32 loads[SLOTS];                                  // the engines unit's count of the slot's loads when it was kept
    u32 frames[SLOTS], stereo[SLOTS];                  // the kept cut
    u32 reading;                                       // the slot read now + 1, 0 none
    int handle;
    u32 done, total;                                   // the bytes read so far, the bytes to read
    u32 card, ask, loads_at, object, capacity, channels;   // what the read began on: the ride, the loads, the object and its words
    u32 lines;                                         // the lines said this frame
} unit;

// ---------------------------------------------------------------- small helpers

static int plausible(u32 at) {
    return (at & 3u) == 0 && ((at >> 24) == 8u || (at >> 28) == 8u);
}

static u32 object_of(u32 slot) {
    u32 holder = RD32((u32)SYNTH_HOLDERS + 4u * slot);
    if (!plausible(holder)) return 0;
    u32 object = RD32(holder);
    return plausible(object) ? object : 0u;
}

static u32 pool_of(u32 slot) {
    return (u32)SYNTH_POOLS + (u32)SYNTH_STRIDE * slot;
}

// The slot's ride (0, 1 or 2), its card and ask, and its loads.
static u32 ride_whole(u32 slot, u32* card, u32* ask, u32* loads) {
    u32 c = 0, a = 0, l = 0, r = 0;
    if (&op1f_engines_ride != 0) r = op1f_engines_ride(slot, &c, &a, &l);
    *card = c;
    *ask = a;
    *loads = l;
    return r;
}

static u32 ride_of(u32 slot, u32* loads) {
    u32 c = 0, a = 0;
    return ride_whole(slot, &c, &a, loads);
}

// user/audio/synth/synth_<slot>.raw, the slot from 0; its name is at path + NAME_AT.
static void path_of(u32 slot, char* path) {
    u32 n = 0;
    for (u32 i = 0; FOLDER[i] != 0; ++i) path[n++] = FOLDER[i];
    for (u32 i = 0; NAME_HEAD[i] != 0; ++i) path[n++] = NAME_HEAD[i];
    path[n++] = (char)('0' + slot);
    path[n++] = '.';
    path[n++] = 'r';
    path[n++] = 'a';
    path[n++] = 'w';
    path[n] = 0;
}

// Room for a line this frame (at most LINES_A_FRAME); each line said counts itself.
static int room() {
    return unit.lines < (u32)LINES_A_FRAME;
}

static void not_put_back(u32 slot, const char* why) {
    ++unit.lines;
    op1f_log(2, LOG_FILE, 1, "sample: synth %u: not put back (%s)", slot + 1u, why);
}

// The slot's put-back over: DONE; its file closed when it was open.
static void finish(u32 slot) {
    if (unit.reading == slot + 1u) {
        if (unit.handle >= 0) STOCK_CLOSE(unit.handle);
        unit.handle = -1;
        unit.reading = 0;
    }
    unit.state[slot] = DONE;
}

// ---------------------------------------------------------------- the drop's end

// From the sample unit's frame, when it says a drop that rendered: the cut kept for the next start; a put-back that still
// waited for this slot is done (the pool holds the newer cut).
extern "C" RESTORE_APART void op1f_sample_kept(unsigned slot, unsigned frames, unsigned stereo) {
    if (slot >= (u32)SLOTS) return;
    u32 loads = 0;
    ride_of(slot, &loads);
    u32 s = stereo ? 1u : 0u;
    op1f_keeps_sample_set(slot, frames, s);
    unit.held[slot] = frames != 0 ? 1u : 0u;
    unit.loads[slot] = loads;
    unit.frames[slot] = frames;
    unit.stereo[slot] = s;
    if (unit.state[slot] == (u32)WAITING) unit.state[slot] = DONE;
}

// ---------------------------------------------------------------- the put-back

// A slot whose cut waits, with room for a line: checked, and its file opened (READING); or DONE with its line; or left
// WAITING for a later frame.
static void begin(u32 slot) {
    u32 card = 0, ask = 0, loads = 0;
    u32 ride = ride_whole(slot, &card, &ask, &loads);
    if (ride == 2u) return;                                                         // a sound loaded there is not told yet: a later frame
    if (ride != 1u) {
        not_put_back(slot, "the sound plays no pack's engine now");
        finish(slot);
        return;
    }
    u32 object = object_of(slot);
    if (object == 0 || RD32(object) != pool_of(slot)) {
        not_put_back(slot, "the sound's sample is not where it should be");
        finish(slot);
        return;
    }
    u32 stereo = RD8(object + (u32)CHANNELS_AT) != 0 ? 1u : 0u;
    if (stereo != unit.stereo[slot]) {
        not_put_back(slot, stereo ? "the sound is stereo now, the cut was kept mono" : "the sound is mono now, the cut was kept stereo");
        finish(slot);
        return;
    }
    char path[PATH];
    path_of(slot, path);
    int size = STOCK_FILE_SIZE(path);
    if (size < 0) {
        ++unit.lines;
        op1f_log(2, LOG_FILE, 2, "sample: synth %u: not put back (no file %s)", slot + 1u, path + NAME_AT);
        finish(slot);
        return;
    }
    u32 total = op1f_sample_restore_bytes((u32)size);
    u32 halves = unit.frames[slot] << unit.stereo[slot];                           // the cut's halfwords from the pool's first
    u32 skipped = (u32)OP1F_SAMPLE_KEPT_FROM / 2u;                                  // the halfwords the file does not hold
    u32 needed = halves > skipped ? (halves - skipped) * 2u : 0u;
    if (total < needed) {
        ++unit.lines;
        op1f_log(2, LOG_FILE, 3, "sample: synth %u: not put back (%s holds %d bytes, the cut %u)", slot + 1u, path + NAME_AT, size, needed);
        finish(slot);
        return;
    }
    if (!op1f_sample_restore_begin(slot)) return;                                   // a drop there is under way: a later frame
    int handle = STOCK_OPEN(path, READ_MODE);
    if (handle < 0) {
        op1f_sample_restore_end(slot, 0);
        ++unit.lines;
        op1f_log(2, LOG_FILE, 4, "sample: synth %u: not put back (%s could not be opened)", slot + 1u, path + NAME_AT);
        finish(slot);
        return;
    }
    unit.handle = handle;
    unit.reading = slot + 1u;
    unit.done = 0;
    unit.total = total;
    unit.card = card;
    unit.ask = ask;
    unit.loads_at = loads;
    unit.object = object;
    unit.capacity = RD32(object + (u32)CAPACITY_AT);
    unit.channels = RD8(object + (u32)CHANNELS_AT);
    unit.state[slot] = READING;
}

// What changed since the read began, in the line's words; 0 when nothing did. `loaded`: 1 when it is the loads count.
static const char* changed(u32 slot, u32* loaded) {
    u32 card = 0, ask = 0, loads = 0;
    u32 ride = ride_whole(slot, &card, &ask, &loads);
    *loaded = 0;
    if (loads != unit.loads_at) {
        *loaded = 1;
        return "a sound was loaded there";
    }
    if (ride == 2u) return "a sound loaded there is not told yet";
    if (ride != 1u) return "the sound plays no pack's engine now";
    if (card != unit.card || ask != unit.ask) return "the sound plays another engine now";
    u32 object = object_of(slot);
    if (object != unit.object) return "the sound's object changed";
    if (RD32(object) != pool_of(slot)) return "the sound's sample is not at its pool now";
    if (RD8(object + (u32)CHANNELS_AT) != unit.channels) return "the sound's channels changed";
    if (RD32(object + (u32)CAPACITY_AT) != unit.capacity) return "the sound's capacity changed";
    return 0;
}

// The put-back stopped: the file closed, nothing more written, its end with 0 frames; the kept cut ended only by a load.
static void stop(u32 slot, const char* what, u32 loaded) {
    op1f_sample_restore_end(slot, 0);
    if (loaded) {
        unit.held[slot] = 0;
        op1f_keeps_sample_set(slot, 0, 0);
    }
    ++unit.lines;
    op1f_log(2, LOG_FILE, 9, "sample: synth %u: put back stopped: %s", slot + 1u, what);
    finish(slot);
}

// The slot read now, with room for a line: 64 KB more into its pool, from 16 bytes in; at the end its line.
static void read_on(u32 slot) {
    char path[PATH];
    u32 loaded = 0;
    const char* what = changed(slot, &loaded);                                     // before each chunk, the first too
    if (what) {
        stop(slot, what, loaded);
        return;
    }
    u32 left = unit.total - unit.done;
    u32 chunk = left < (u32)CHUNK ? left : (u32)CHUNK;
    if (chunk != 0) {
        u8* to = (u8*)(pool_of(slot) + (u32)OP1F_SAMPLE_KEPT_FROM + unit.done);   // unit.done + chunk <= the bound: never past the pool
        int got = STOCK_READ(to, 1, chunk, unit.handle);
        if (got != (int)chunk) {
            op1f_sample_restore_end(slot, 0);
            path_of(slot, path);
            ++unit.lines;
            op1f_log(2, LOG_FILE, 5, "sample: synth %u: not put back (%s was not read whole: %u of %u bytes)", slot + 1u, path + NAME_AT,
                     unit.done + (got > 0 ? (u32)got : 0u), unit.total);
            finish(slot);
            return;
        }
        unit.done += chunk;
    }
    if (unit.done < unit.total) return;
    op1f_sample_restore_end(slot, unit.frames[slot]);
    path_of(slot, path);
    ++unit.lines;
    op1f_log(3, LOG_FILE, 6, "sample: synth %u: put back %u frames from %s", slot + 1u, unit.frames[slot], path + NAME_AT);
    finish(slot);
}

// ---------------------------------------------------------------- once a frame

// From the sample unit's frame (module/build_r17_sample.cpp, op1f_sample_frame), once a frame on the display thread.
extern "C" RESTORE_APART void op1f_sample_restore_frame() {
    unit.lines = 0;
    u32 call = unit.calls + 1u;
    unit.calls = call;
    if (call == 1u) unit.handle = -1;
    if (call == 2u) unit.first_over_ms = op1f_ms();                                 // the first frame is over
    if (!unit.settled && call > 2u && op1f_ms() - unit.first_over_ms >= (u32)START_SETTLE_MS) unit.settled = 1;
    if (!unit.taken) {                                                              // the values unit read its store at its first frame
        if (op1f_keeps_sample_get(0, 0, 0) < 0) return;                             // not yet: the next frame
        for (u32 slot = 0; slot < (u32)SLOTS; ++slot) {
            u32 frames = 0, stereo = 0, loads = 0;
            int got = op1f_keeps_sample_get(slot, &frames, &stereo);
            ride_of(slot, &loads);
            unit.loads[slot] = loads;
            unit.held[slot] = got == 1 ? 1u : 0u;
            unit.frames[slot] = frames;
            unit.stereo[slot] = stereo ? 1u : 0u;
            unit.state[slot] = got == 1 ? (u32)WAITING : (u32)NONE;
        }
        unit.taken = 1;
    }
    for (u32 slot = 0; slot < (u32)SLOTS && room(); ++slot) {                       // a sound loaded onto a slot ends its kept cut
        if (!unit.held[slot]) continue;
        u32 loads = 0;
        ride_of(slot, &loads);
        if (loads == unit.loads[slot]) continue;
        u32 state = unit.state[slot];
        if (state == (u32)READING) {                                                // stopped: nothing more written
            stop(slot, "a sound was loaded there", 1);
            continue;
        }
        unit.held[slot] = 0;
        op1f_keeps_sample_set(slot, 0, 0);
        if (state == (u32)WAITING) {
            not_put_back(slot, "a sound was loaded there");
            finish(slot);
        } else {
            ++unit.lines;
            op1f_log(3, LOG_FILE, 7, "sample: synth %u: the cut is kept no longer: a sound was loaded there", slot + 1u);
        }
    }
    if (!unit.settled || !room()) return;
    if (RD32(DISK_OPEN) != 0) {
        if (unit.reading) {                                                         // the disk mode opened: the file closed, the slot waits
            u32 slot = unit.reading - 1u;
            op1f_sample_restore_end(slot, 0);
            finish(slot);
            unit.state[slot] = WAITING;
            ++unit.lines;
            op1f_log(2, LOG_FILE, 8, "sample: synth %u: the put-back waits: the disk mode opened", slot + 1u);
        }
        return;
    }
    for (u32 slot = 0; slot < (u32)SLOTS && !unit.reading && room(); ++slot)
        if (unit.state[slot] == (u32)WAITING) begin(slot);
    if (unit.reading && room()) read_on(unit.reading - 1u);
}

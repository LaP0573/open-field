// Build N: the pack engines, new synth engines as packs of their own (module/op1f_engine.h). An engine pack exports
// one description, op1f_engine_<name>; this pack owns the places of the firmware every engine needs, so that each
// engine is a file an owner puts on the unit or leaves out. The places (profiles/op1f-1.7.3.json, group engines):
//   * the firmware's lookups of a synth engine by its name (two jumps): an engine's name answers the stock engine it
//     rides on (its host: dsynth or voltage), and is valid; every other name goes on to the firmware's own;
//   * in set-patch, the call of that lookup (a call): notes which synth slot's patch is which engine's;
//   * the start-up load and the save of content/op1_factory.json (two calls): a synth_types entry for each engine
//     that is there goes in after the load and stays out of the save, so the file on the disk stays stock (the
//     method of seajay's garden.pack, for its sequencer);
//   * in the synth's voice loop, in L1 (a jump): a voice of the host engine on an engine's slot renders through the
//     engine; every other voice, the firmware's own render;
//   * in the synth's audio stage, in L1, the voices summed onto the bus and the effect stage about to run (a jump): a
//     bus engine adds its sound there, before the T3 effect and fx train;
//   * the funnels every synth note goes through (two jumps), and right before the display commit (a call): every note
//     to the slot's engine, and its faceplate over the host's screen. These three are marked away for cherd: a pack
//     that takes those places for itself hands each note and the frame buffer to op1f_engines_note and
//     op1f_engines_face, which this pack offers.
// At its first frame it writes the engines' presets that are missing (version 2 of the description) into
// user/synth_presets/<engine>/: a file under a spare name, renamed over the real one, and never one that is there.
//
// Threads: the lookups, set-patch and the factory file run on the main thread; the voice loop and the bus on the
// audio thread; the note funnels on the thread that handles keys or MIDI; the faces on the display thread. They share
// single words: a slot's engine, the block counter, shift.

#include "build_m3_card.h"
#include "op1f_engine.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

extern "C" {   // libgcc's division, which a pack has no copy of (the voice index divides by the host's stride)
u32 eng_udiv(u32 a, u32 b) asm("___udivsi3");
u32 eng_udiv(u32 a, u32 b) { u32 q = 0, r = 0; for (int i = 31; i >= 0; --i) { r = (r << 1) | ((a >> i) & 1u); if (r >= b) { r -= b; q |= 1u << i; } } return q; }
u32 eng_umod(u32 a, u32 b) asm("___umodsi3");
u32 eng_umod(u32 a, u32 b) { return a - eng_udiv(a, b) * b; }
}

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*StringIsFn)(const void* string, const char* text);          // 0 when the text object holds the C text
typedef int (*PatchSlotFn)(void* patch);                                   // the synth slot (0..7) a patch is loaded into
typedef int (*JsonLoadFn)(const char* path, void* json);
typedef int (*JsonSaveFn)(void* path_string, void* json);
typedef void* (*JsonFindFn)(void* json, const char* key);
typedef void* (*JsonKeyFn)(void* json, const char* key);
typedef int (*JsonSizeFn)(void* json);
typedef void* (*JsonAtFn)(void* json, int index);
typedef void (*JsonPushFn)(void* array, void* value);
typedef void (*JsonMakeFn)(void* json, int kind);
typedef void (*JsonAssignFn)(void* json, const void* from);
typedef void (*JsonDestroyFn)(void* json);
typedef int (*FileSizeFn)(const char* path);
typedef int (*MakeFoldersFn)(const char* path);
typedef i32 (*FOpenFn)(const char* path, const char* mode);
typedef i32 (*FWriteFn)(const void* source, i32 size, i32 count, i32 file);
typedef void (*FCloseFn)(i32 file);
typedef void (*RenameFn)(const char* old_path, const char* new_path);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_STRING_IS ((StringIsFn)0x8fca7a38u)
#define STOCK_PATCH_SLOT ((PatchSlotFn)0x8d4f3d6cu)
#define STOCK_JSON_LOAD ((JsonLoadFn)0x8c0534d0u)
#define STOCK_JSON_SAVE ((JsonSaveFn)0x8c053168u)
#define STOCK_JSON_FIND ((JsonFindFn)0x8c084f74u)
#define STOCK_JSON_KEY ((JsonKeyFn)0x8c085e84u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_MAKE ((JsonMakeFn)0x8c085ba8u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DESTROY ((JsonDestroyFn)0x8c0844e0u)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_FOPEN ((FOpenFn)0x8fc22180u)
#define STOCK_FWRITE ((FWriteFn)0x8fc220b8u)
#define STOCK_FCLOSE ((FCloseFn)0x8fc220a0u)
#define STOCK_RENAME ((RenameFn)0x8fc222f4u)

// Words of the firmware this pack reads.
#define TRACK_WORD 0x84a01524u          // the synth slot shown (0..7)
#define PLAY_MODE_BYTE 0x84d21088u      // 0 while the keys play the synth, 1 the drums
#define LIVE_PARAMS 0x119035e0u         // per slot, 0x20 bytes: its 8 knobs as set (Q31)
#define MOD_PARAMS 0x11904774u          // the shown slot's 8 knobs with the LFO applied (Q31)
#define PITCH_WORD 0x11903c04u          // the pitch word of the voice being rendered
#define VOICE_POOL 0x119062d8u          // the synth's six voices, one engine's at a time, each at its engine's stride
#define DSYNTH_VTABLE 0x848cc9acu
#define VOLTAGE_VTABLE 0x848cbd20u
#define BUS_LEFT 0x11902ba0u            // the synth bus: 64 samples a side, Q31
#define BUS_RIGHT 0x11902ca0u

static const char LOG_FILE[] = "op1f/engines.cpp";       // the log shows "engines"

extern "C" const Op1fCard op1f_card_engines OP1F_NAMED("op1f_card_engines");

// ---------------------------------------------------------------- the engines there are
// Optional names: an engine whose pack is away is simply not offered. One line an engine; examples/drone is the
// example to start from.
extern "C" const Op1fEngine __attribute__((weak)) op1f_engine_drone OP1F_NAMED("op1f_engine_drone");
extern "C" const Op1fEngine __attribute__((weak)) op1f_engine_lilith OP1F_NAMED("op1f_engine_lilith");
extern "C" const Op1fEngine __attribute__((weak)) op1f_engine_clarinet OP1F_NAMED("op1f_engine_clarinet");
extern "C" const Op1fEngine __attribute__((weak)) op1f_engine_trumpet OP1F_NAMED("op1f_engine_trumpet");
static const Op1fEngine* const ENGINES[] = { &op1f_engine_drone, &op1f_engine_lilith, &op1f_engine_clarinet, &op1f_engine_trumpet };
enum { ENGINE_COUNT = sizeof(ENGINES) / sizeof(ENGINES[0]) };

static const Op1fEngine* engine_at(int i) {
    const Op1fEngine* e = ENGINES[i];
    return e && e->magic == OP1F_ENGINE_MAGIC && e->version >= 1 && e->version <= OP1F_ENGINE_VERSION && e->name ? e : 0;
}
static int engine_of(const void* string) {                  // 1 + the engine whose name the text object holds, or 0
    for (int i = 0; i < ENGINE_COUNT; ++i) { const Op1fEngine* e = engine_at(i); if (e && STOCK_STRING_IS(string, e->name) == 0) return i + 1; }
    return 0;
}

// ---------------------------------------------------------------- names and slots (main thread)
static volatile u8 slot_engine_of[8];                       // per synth slot: 1 + its engine, or 0 for the firmware's own
static inline const Op1fEngine* slot_engine() {
    const u32 s = *(volatile u32*)TRACK_WORD;
    return s < 8 && slot_engine_of[s] ? engine_at(slot_engine_of[s] - 1) : 0;
}
extern "C" int eng_orig_name_to_type(const void* string);   // the rooms of the two trampolines (build_n_engines.s)
extern "C" int eng_orig_is_valid(const void* string);
extern "C" int eng_name_to_type(const void* string) { const int i = engine_of(string); return i ? (int)engine_at(i - 1)->host_type : eng_orig_name_to_type(string); }
extern "C" int eng_is_valid(const void* string) { return engine_of(string) ? 1 : eng_orig_is_valid(string); }
extern "C" int eng_set_type(const void* string, void* patch) {   // set-patch's lookup; the entry hands the patch in
    const int i = engine_of(string);
    const u32 slot = (u32)STOCK_PATCH_SLOT(patch);
    if (slot < 8) slot_engine_of[slot] = (u8)i;
    return i ? (int)engine_at(i - 1)->host_type : eng_orig_name_to_type(string);
}

// ---------------------------------------------------------------- the firmware's engine list (main thread)
// A JSON value of the firmware is 12 bytes (kept in 16, as the base keeps its own); its kind stands in the byte at +4.
struct Json { u32 w[4]; };
enum { J_INT = 1, J_TEXT = 4, J_ARRAY = 6, J_OBJECT = 7 };
static inline u8 kind_of(const Json* v) { return ((const u8*)v)[4]; }
static u32 given_id[ENGINE_COUNT];                          // the ids our entries got (0: not added)
static volatile u32 added, factory_why;

static void set_int(void* slot, int n) { Json v; STOCK_JSON_MAKE(&v, J_INT); v.w[0] = (u32)n; STOCK_JSON_ASSIGN(slot, &v); STOCK_JSON_DESTROY(&v); }
static int id_of(Json* item) {
    Json* id = item && kind_of(item) == J_OBJECT ? (Json*)STOCK_JSON_FIND(item, "id") : 0;
    return id && (kind_of(id) == 1 || kind_of(id) == 2) ? (int)id->w[0] : -1;
}
extern "C" int eng_factory_load(const char* path, Json* root) {
    const int answer = STOCK_JSON_LOAD(path, root);
    if (!root || kind_of(root) != J_OBJECT) { factory_why = 1; return answer; }
    Json* list = (Json*)STOCK_JSON_FIND(root, "synth_types");
    if (!list || kind_of(list) != J_ARRAY) { factory_why = 2; return answer; }
    const int n = STOCK_JSON_SIZE(list);
    int top = 0;
    for (int k = 0; k < n && k < 255; ++k) { const int id = id_of((Json*)STOCK_JSON_AT(list, k)); if (id > top) top = id; }
    for (int i = 0; i < ENGINE_COUNT; ++i) {
        const Op1fEngine* e = engine_at(i);
        if (!e) continue;
        Json entry, text, params;
        STOCK_JSON_MAKE(&entry, J_OBJECT);
        given_id[i] = (u32)++top;
        set_int(STOCK_JSON_KEY(&entry, "id"), top);
        STOCK_JSON_MAKE(&text, J_TEXT); text.w[0] = (u32)(unsigned long)e->name; STOCK_JSON_ASSIGN(STOCK_JSON_KEY(&entry, "type"), &text); STOCK_JSON_DESTROY(&text);
        STOCK_JSON_MAKE(&params, J_ARRAY);
        for (int k = 0; k < 8; ++k) { Json v; STOCK_JSON_MAKE(&v, J_INT); v.w[0] = (u32)e->defaults[k]; STOCK_JSON_PUSH(&params, &v); STOCK_JSON_DESTROY(&v); }
        STOCK_JSON_ASSIGN(STOCK_JSON_KEY(&entry, "default_params"), &params); STOCK_JSON_DESTROY(&params);
        STOCK_JSON_PUSH(list, &entry); STOCK_JSON_DESTROY(&entry);
        added = added + 1;
    }
    return answer;
}
static bool ours(Json* item) { const int id = id_of(item); for (int i = 0; i < ENGINE_COUNT; ++i) if (given_id[i] && (int)given_id[i] == id) return true; return false; }
extern "C" int eng_factory_save(void* path, Json* root) {   // path: the firmware's text object, handed on as it came
    Json* list = added && root && kind_of(root) == J_OBJECT ? (Json*)STOCK_JSON_FIND(root, "synth_types") : 0;
    if (!list || kind_of(list) != J_ARRAY) return STOCK_JSON_SAVE(path, root);
    Json copy, kept;
    STOCK_JSON_MAKE(&copy, 0); STOCK_JSON_ASSIGN(&copy, root); STOCK_JSON_MAKE(&kept, J_ARRAY);
    const int n = STOCK_JSON_SIZE(list);
    for (int k = 0; k < n; ++k) { Json* item = (Json*)STOCK_JSON_AT(list, k); if (!ours(item)) STOCK_JSON_PUSH(&kept, item); }
    STOCK_JSON_ASSIGN(STOCK_JSON_KEY(&copy, "synth_types"), &kept); STOCK_JSON_DESTROY(&kept);
    const int answer = STOCK_JSON_SAVE(path, &copy);
    STOCK_JSON_DESTROY(&copy);
    return answer;
}

// ---------------------------------------------------------------- the voices and the bus (audio thread)
static volatile u32 block;                                  // one step an audio block (the bus)
struct Host { u32 type, vtable, stride; };                  // a stock engine an engine may ride on: its voices in the pool
static const Host HOSTS[2] = { { 12, DSYNTH_VTABLE, 0x48 }, { 13, VOLTAGE_VTABLE, 0x34 } };
static int voice_index(const Op1fEngine* e, u32 voice) {    // 0..5: the voice in its host's pool, or -1
    for (int i = 0; i < 2; ++i) {
        const Host& h = HOSTS[i];
        if (e->host_type != h.type) continue;
        const u32 at = voice - VOICE_POOL;
        return at < 6u * h.stride && *(volatile u32*)voice == h.vtable ? (int)(at / h.stride) : -1;
    }
    return -1;
}
extern "C" int eng_voice_is(u32 voice) {                    // 1 when the voice renders through its slot's engine
    const Op1fEngine* e = slot_engine();
    return e && voice_index(e, voice) >= 0;
}
extern "C" void eng_voice_render(int* out, u32 voice) {
    const Op1fEngine* e = slot_engine();
    const volatile int* knobs = e && (e->flags & OP1F_ENGINE_LIVE_KNOBS)
        ? (const volatile int*)(LIVE_PARAMS + (*(volatile int*)TRACK_WORD) * 0x20) : (const volatile int*)MOD_PARAMS;
    const int k = e ? voice_index(e, voice) : -1;
    if (e && e->render && k >= 0) e->render(out, knobs, *(volatile u32*)PITCH_WORD, block, k);
    else for (int i = 0; i < 128; ++i) out[i] = 0;
}
extern "C" void __attribute__((weak)) op1f_autotune_bus(int* left, int* right) OP1F_NAMED("op1f_autotune_bus");   // a live input on the bus, when a pack offers one
extern "C" void eng_bus() {
    block = block + 1;
    const Op1fEngine* e = slot_engine();
    if (e && (e->flags & OP1F_ENGINE_BUS) && e->bus) e->bus((int*)BUS_LEFT, (int*)BUS_RIGHT, block);
    if (&op1f_autotune_bus) op1f_autotune_bus((int*)BUS_LEFT, (int*)BUS_RIGHT);
}

// ---------------------------------------------------------------- notes and faces
extern "C" void op1f_engines_note(int note, int on, int velocity) OP1F_NAMED("op1f_engines_note");
extern "C" void op1f_engines_note(int note, int on, int velocity) {
    if (*(volatile u8*)PLAY_MODE_BYTE != 0) return;         // synth notes only
    const Op1fEngine* e = slot_engine();
    if (e && e->note) e->note(note, on, velocity);
}
extern "C" void eng_orig_on(int, int, int, int, int, int, int);
extern "C" void eng_orig_off(int, int, int, int, int, int, int);
extern "C" void eng_on(int note, int velocity, int a, int b, int c, int d, int f) { op1f_engines_note(note, 1, velocity); eng_orig_on(note, velocity, a, b, c, d, f); }
extern "C" void eng_off(int note, int a, int b, int c, int d, int f, int g) { op1f_engines_note(note, 0, 0); eng_orig_off(note, a, b, c, d, f, g); }

static volatile u32 shift, frames;
static inline void put_pixel(u16* fb, int x, int y, u16 c) { if ((unsigned)x < 480u && (unsigned)y < 216u) fb[y + ((479 - x) << 8)] = c; }
static inline bool is_marker(u16 s) { return ((s >> 11) & 0x1f) >= 0x1d && ((s >> 5) & 0x3f) <= 2 && (s & 0x1f) >= 0x1d; }
// The host's screen carries our marker, a magenta square at its top-left (pictures/engines): on an engine's slot its
// face draws over the screen; on the host's own patch the marker is painted over with the screen's background.
extern "C" void op1f_engines_face(u16* fb) OP1F_NAMED("op1f_engines_face");
extern "C" void op1f_engines_face(u16* fb) {
    frames = frames + 1;
    if (!is_marker(fb[1 + ((479 - 1) << 8)])) return;
    const Op1fEngine* e = slot_engine();
    if (e && e->face) { e->face(fb, (const volatile int*)(LIVE_PARAMS + (*(volatile int*)TRACK_WORD) * 0x20), shift != 0, frames); return; }
    const u16 background = fb[12 + ((479 - 1) << 8)];
    for (int y = 0; y < 5; ++y) for (int x = 0; x < 5; ++x) put_pixel(fb, x, y, background);
}

// ---------------------------------------------------------------- presets (display thread, at the first frame)
static char* join(char* p, const char* s, const char* end) { while (*s && p < end) *p++ = *s++; *p = 0; return p; }
static void write_presets(const Op1fEngine* e, u32& written, u32& there, u32& failed) {
    if (e->version < 2 || !e->presets) return;
    char folder[64], path[128], spare[136];
    const char* fend = folder + sizeof(folder) - 1;
    if (join(join(join(folder, "user/synth_presets/", fend), e->name, fend), "/", fend) >= fend) return;
    bool made = false;
    for (u32 k = 0; k < e->preset_count; ++k) {
        const Op1fPreset& p = e->presets[k];
        const char* pend = path + sizeof(path) - 1;
        if (join(join(join(path, folder, pend), p.name, pend), ".json", pend) >= pend) { failed = failed + 1; continue; }
        if (STOCK_FILE_SIZE(path) >= 0) { there = there + 1; continue; }
        if (!made) { STOCK_MAKE_FOLDERS(folder); made = true; }
        join(join(spare, path, spare + sizeof(spare) - 1), ".part", spare + sizeof(spare) - 1);
        const i32 f = STOCK_FOPEN(spare, "w+b");               // the firmware's modes: w+b, not wb
        if (f < 0) { failed = failed + 1; continue; }
        u32 n = 0; while (p.json[n]) ++n;
        const i32 w = STOCK_FWRITE(p.json, 1, (i32)n, f);
        STOCK_FCLOSE(f);
        if (w != (i32)n) { failed = failed + 1; continue; }
        STOCK_RENAME(spare, path);
        written = written + 1;
    }
}

// ---------------------------------------------------------------- the card's frame
extern "C" void engines_frame(const Op1fFrame* f) OP1F_NAMED("engines_frame");
extern "C" void engines_frame(const Op1fFrame* f) {
    static u32 said, said_added;
    if (!f || f->bytes < 28) return;
    shift = f->shift;
    if (!said) {
        said = 1;
        for (int i = 0; i < ENGINE_COUNT; ++i) {
            const Op1fEngine* e = engine_at(i);
            if (!e) continue;
            u32 written = 0, there = 0, failed = 0;
            write_presets(e, written, there, failed);
            STOCK_LOG(3, LOG_FILE, 1, "engines: '%s' is here, on type %u; presets: %u written, %u already there, %u failed",
                      e->name, e->host_type, written, there, failed);
        }
    }
    if (added != said_added) { said_added = added; STOCK_LOG(3, LOG_FILE, 2, "engines: %u engine(s) in the firmware's engine list", added); }
    if (factory_why) { STOCK_LOG(2, LOG_FILE, 3, "engines: the factory file was not as expected (%u): no engine added", factory_why); factory_why = 0; }
}

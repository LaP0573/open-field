// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Build H images H1 to H3: the chain's engine, the train's knobs and keys and the persistence
// (analysis/fx_chain_study.md, sections 3, 6, 9 to 15, 17 and 18). H1 to H1g painted a provisional train page as text and
// boxes over the finished frame; since H2 the page is a screen of the GUI's own, drawn from the chain's view by
// build_h_train.cpp (op1f_chain_view, below), and this unit paints nothing. H3 adds the bus peak the train runs on,
// the refused pick's type for the page, shift + T3 on the train as the whole-chain bypass (every wagon onto its siding,
// the couplings remembered in the file) and the file's tag 92 with the remembered couplings as a 48th integer (a tag-91
// file of 47 integers still loads). H3b arms the hold only on a clean press (no turn of the knob in the quarter second
// before it: a knob pressed while it is turned fired holds the owner did not mean) and logs every hold, every press
// that armed none and every follow of the mirror's fx_active byte, the three paths that move a wagon by themselves.
// H4b (section 20.6) times the hold and the clean window by the core's cycle counter as milliseconds (every timer
// had counted GUI frames, and the train page's frame rate is not 58 a second for sure: the owner found the hold
// long), shortens the window to 120 ms and logs the frame rate (the `chain clock:` line) for the read-out.
// H5 (section 21) adds the chain presets: a list in the stock browser's shape under shift + T3 (the whole-chain
// bypass of H3 gave up the key), a hold of shift + T3 saving the chain as a preset, the list loading, renaming and
// deleting; the store travels in the module's file as a tag-93 element; the screen is build_h_presets.cpp's.
// H5b (section 21.8): every text in the font's lowercase (the OS font's table is indexed by the character code and
// holds the letters at the lowercase codes; H5's capitals drew icons and question marks), the default name's maker
// no longer compares the name with its own slot (H5 named every preset chain 99), and the save shows: stock's save
// screen redrawn at the id 38 with the host's box and the preset's number, "saving" with the clock while shift + T3
// is held (from 200 ms; the save at 700 ms), "saved!" with the check for a second after any save.
// H6c (section 22.10): the record's document follows the mirrored wagon whenever the module puts another effect on
// stock's slot (mirror_document: fx_type, fx_params, fx_active and the type the chain knows the document by; H6b
// left the document at the browser's last effect, so the slots file and the effect page showed that effect's
// parameters), and the frame's follow of the slot's block is counted and, soon after a document write, logged.
// H6 (section 22) makes the presets files: thirty-two slots user/fx/01.json .. 32.json, one JSON object each (the
// name, the order, the four wagons with the effect's name, its coupling and its eight parameters; {} for an empty
// slot), written by stock's json file writer and read by its reader, scanned at the first file load and at every
// list open (a file whose time or size changed is read again, a missing file is made), the disk mode showing them
// as fx/NN.json (build_h_disk.cpp); the H5 presets of op1f.json's tag-93 element are moved into the first free
// slots once, and the element is written empty from then on. H6b (section 22.7): the folder is made with its
// trailing slash (createFolderRecursive makes the components that end in a slash: H6's user/fx made user alone, and
// every write failed its rename into the missing folder), every write's result is checked and counted, the H5
// element stays in op1f.json until its presets have landed in files, and the slots are sixty-four.
// H4 lifts the one-wagon-per-type rule (section 20): every module-built instance gets a private state region (512 KB
// of C heap 0, eight regions for the eight wagon slots of the two hosts) and the fixed state the H0 census found is
// redirected to it around the instance's process call on the audio thread: the delay's and cwo's pointer-table
// entries (0x84ad0700[0], 0x84ad06f8[0], read inside their process), the spring's and mother's region bases through
// the module words op1f_spring_base / op1f_mother_base that the three CALL.a stubs of build_h_regions.s load in place
// of stock's immediates, and nitro's three static words at 0x080266e8 copied in and out; the L1 buffers 0x11904004 and
// 0x11904604 are scratch filled and consumed within one call and need nothing, and grid's and phone's "addresses" of
// the census were fixed-point constants. Stock's own instances (the mirrored wagon's producer, the master effect's at
// index 1) keep stock's regions. Without the pool (the allocation refused) the H1 rule stands as before.
//
// The model (study section 3): four wagons, each bound to one knob for good (the colour travels with the wagon); a
// wagon carries one effect or none, coupled (its effect runs) or uncoupled (skipped, costing nothing); the chain
// runs in position order after the engine, inside the instrument's second stage, where stock ran its one effect.
//
// What stock does, as read for H1 (study sections 6 and 9):
//   * Both sound controllers (synth, vtable 0x8401364c; drum, 0x8400e88c) share the key-down handler 0x8c0cfe7c and
//     the key-up 0x8c0cf364; their shifted key-down (0x8c0cf904 / 0x8c0b6e1e) and encoder (0x8c0d5c10 / 0x8c0b7634)
//     handlers differ. The host is the word at this + 0xc (0 synth, else drum), the page at this + 0x28 (0 engine,
//     1 envelope, 2 effect, 3 lfo). The dispatchers 0x8c0b1ae8 (key-down), 0x8c0b1b58 (shifted key-down) and
//     0x8c0b1b8c (key-up) call the current controller's virtual with (this, event*) or (this, key), the key the
//     first word of the event (the handlers read nothing else of it), and run a fallback when the handler returns
//     0. The keys: T1..T4 = 6..9 (T3 = 8 sets page 2, and on page 2 toggles fx_active through the controller's
//     virtual at vtable + 0x50); the encoder handler gets (this, index + 4 while shift is held, delta). A knob's
//     physical press (blue to orange) arrives at key-down as the key 61..64 (at the shifted key-down while shift is
//     held) and its release at key-up as the same key; then, when the knob was not turned while pressed, the key
//     handler 0x8c0b1468 synthesizes a click: the key 54..57 at key-down (shifted by shift at the release) and at
//     key-up (section 10.3; H1 had the press and the click confused). Shift + T3 calls the virtual at vtable + 0x3c
//     (this, slot): the effect browser.
//   * Stock's effect is one slot per host: the type word 0x11904794[slot] (synth) / 0x119047b4[slot] (drum), the
//     eight parameter words at 0x119036e0 + 0x20 slot / 0x119039e0 + 0x20 slot (copied every block into the ctx
//     0x11904754 the effect reads), the fx_active byte at 0x84a0154c + slot / 0x84a0155c + slot, the producer
//     object in the slot 0x84a03c84, shared by the synth and the drum: 0x8d505390(0, type) nulls the slot pointer,
//     builds the type's object through the factory 0x8f00f598 into the fixed storage 0x11906918, calls its init
//     (vtable + 0xc)(this, 0) and publishes the pointer; the second stages test the pointer for null, then the
//     fx_active byte, then call process (vtable + 8)(this, L bus 0x11902ba0, R bus 0x11902ca0, ctx). The GUI's pick
//     path 0x8f007bf8(index, type) is that build and nothing else.
//   * A sound slot's settings are a document the firmware keeps in memory: a record of 52 bytes for each slot
//     (0x080235dc + 52 slot for the synth, 0x0802377c + 52 slot for the drum) with the slot's number in its second
//     word and the JSON object in its third. The firmware's setters write a key of the document and see to it that
//     the slot's file is written: 0x8d4f3b9c(record, eight words) for the effect's parameters, each stored as the
//     word shifted right by sixteen, and 0x8d4f3c24(record, value) for the effect's on-off. A record's document is
//     put onto the sound by the fifth function of the record's table (0x8d4f635c for the synth and 0x8d4f4e2c for
//     the drum, in the words 0x8401f894 and 0x8401f870, at which this unit's two wrappers stand): the effect's type
//     from its name, the parameter words, the on-off byte and, for the slot that is played, the effect's object
//     built anew. The effect browser and the loading of a preset go the same way. A key the firmware does not know
//     is not kept in such a document from one start to the next, which is why the chain has a file of its own
//     (below). The effects' names come from the firmware's table at 0x8490e288, in the order 0 delay, 1 grid,
//     2 punch, 3 phone, 4 spring, 5 mother, 6 nitro, 7 cwo, 8 vocoder, 9 terminal, 10 fazer.
//
// H1's design:
//   * The module owns the whole chain. The start stubs (build_h_fx.s) hand stock a null producer, so stock's own
//     call is skipped; the end stubs call op1f_fx_chain_run(host), which runs the coupled wagons in position order
//     on the two buses. Wagon instances are built by the module through each type's constructor and init into its
//     own memory, except the "mirrored" wagon: the one stock's slot shows and edits (its type word, parameters and
//     fx_active byte, its effect page, its browser), whose instance is stock's own producer object. Tapping a wagon
//     makes it the mirror (stock's slot takes its type and parameters, stock's producer is rebuilt as the browser
//     does it) and opens stock's effect page for it, so every effect keeps its real page and knobs. Until the
//     frame hook has put a slot's chain together, the end stubs run stock's producer as stock would.
//   * One wagon per effect type (study 6.3: eight of the types keep state in fixed memory), and the delay and nitro
//     exclude each other (they share the L1 state 0x11904004): the H1 rule, lifted in H4 (section 20; the shared L1
//     word turned out to be scratch). A pick that would push the chain over the ceiling
//     (the summed per-type maxima of H1a, 200,000 cycles) is refused and shown.
//   * The chain is saved in the module's own file, user/op1f.json (H1e): an array of the tag and the sixteen slots'
//     chains (the mirror, the document's effect type the chain was last reconciled with, the order, then per knob
//     the type, the coupling and the eight parameters as stock's integers), read at the first reconcile and written
//     half a second after the last change, or at once when the shutdown countdown begins. The preset documents
//     cannot carry it: the boot sets each record's document from the slots file whole and then rebuilds it from the
//     known keys (0x8d4f787c, run by 0x8d4f5544 after every set), which the H1d read-out showed as `key none` at
//     every apply while the file held the key. The chain is reconciled against the record's document (H1d): after
//     stock applied a document (the wrappers on the two records' apply vtable words, H1b), when the module first
//     sees a slot and when the slot's type word changes under the module; the document's own fx_type is compared
//     with the type the chain was last reconciled with: equal means a stale document re-applied, and the chain
//     stands, its mirrored wagon written back into the slot; different means the browser picked (or a preset was
//     loaded), and the mirrored wagon takes the document's effect. A slot without a chain of its own gives the
//     document's effect on the blue wagon, the others empty. Each reconcile logs one line (the first forty).
//   * The train: T3 opens it (from any page, when the train unit's screen exists: op1f_train_available), T3 on it
//     opens the mirrored wagon's effect page, T3 on the effect page opens it again; a knob's turn picks the wagon's effect (the label at once, the instance after a 150 ms
//     rest), shift + turn moves the wagon, a tap (the click stock synthesizes at a release without a turn) opens
//     its page (the browser for an empty wagon), a hold (400 ms from the press) couples or uncouples it, with the
//     click after it swallowed, shift + tap randomizes its parameters with stock's own random-preset value function
//     (H1c), shift + hold resets them; a turn while pressed is a pick and cancels the hold, and a press within
//     120 ms of a turn arms no hold (H3b: a quarter second; H4b: the window and the hold in milliseconds of the core
//     clock, not frames); shift + T3 on the train opens the presets list with a tap and saves the chain as a new
//     preset with a hold (H5; H3's whole-chain bypass under that key is gone). The train closes when another page
//     key is pressed or the mode leaves the host.
//
// GUI thread: the four handlers, op1f_chain_frame (per frame), op1f_fx_patch_apply, op1f_chain_view. Audio thread: op1f_fx_chain_run
// (no logging, no division, no floats). Shared state is written by the GUI thread in whole words; the audio thread
// reads it once per block and tolerates a block of staleness.

#include "build_h_chain.h"
#include "build_i_lfo4.h"
#include "build_i_patch.h"
#include "build_q_quiet.h"
#include "op1f_wagon.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int i32;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*KeyEventFn)(void* self, const int* event);
typedef int (*KeyUpFn)(void* self, int key);
typedef int (*EncoderFn)(void* self, int index, int delta);
typedef void (*BrowserFn)(void* self, int slot);
typedef void (*ApplyFn)(void* descriptor);
typedef void (*ProcessFn)(void* self, int* left, int* right, void* ctx);
typedef void (*InitFn)(void* self, int index);
typedef void (*CtorFn)(void* storage);
typedef void (*ArrayCtorFn)(void* storage, int count, int size, void* ctor, void* dtor);
typedef void (*BuildProducerFn)(int index, int type);
typedef void (*SetActiveDocFn)(void* record, int value);
typedef void (*SetParamsDocFn)(void* record, const u32* words);
typedef int (*RandomParamFn)(int type, int index, int bias);     // 0x8d51233c: stock's random value for an effect parameter, the JSON integer in the low half
typedef void (*JsonToStringFn)(void* out_string, void* json);     // 0x8c084782: the json's text into a std::string (the data pointer at +0)
typedef int (*FxTypeOfNameFn)(void* string);                       // 0x8fca7b08: the effect type of a name, as the apply uses it
typedef int (*JsonBoolFn)(void* json);                              // 0x8c084b94
typedef int (*JsonSizeFn)(void* json);                              // 0x8c084c22
typedef void* (*JsonLookupFn)(void* json, const char* key);          // 0x8c085e84: the member, inserted when missing
typedef int (*JsonHasFn)(void* json, const char* key);               // 0x8c085068
typedef void* (*JsonGetFn)(void* json, const char* key);             // 0x8c084f74
typedef void* (*JsonAtFn)(void* json, int index);                   // 0x8c084ee0
typedef int (*JsonIntFn)(void* json);                                // 0x8c084840
typedef void (*JsonCtorFn)(void* json, int kind);                    // 0x8c085ba8(json, kind): a fresh value of that kind; 0 is null, which the first push turns into an array
typedef void (*JsonFromIntFn)(void* json, int value);                // 0x8c083b1c
typedef void (*JsonPushFn)(void* array, void* value);                // 0x8c085d54
typedef void (*JsonAssignFn)(void* target, void* value);             // 0x8c0845b8
typedef void (*JsonDtorFn)(void* json);                              // 0x8c0844e0
typedef int (*JsonReadFileFn)(const char* path, void* json);         // 0x8c0534d0: the file parsed into the json, non-zero on success
typedef int (*JsonWriteFileFn)(void* path_string, void* json);       // 0x8c053168: the json written to the path, a stock string object
typedef void (*StringCtorFn)(void* out, const char* text);           // 0x8c03e434: a stock string object from text

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_BUILD_PRODUCER ((BuildProducerFn)0x8d505390u)
#define STOCK_SET_ACTIVE_DOC ((SetActiveDocFn)0x8d4f3c24u)
#define STOCK_SET_PARAMS_DOC ((SetParamsDocFn)0x8d4f3b9cu)
#define STOCK_RANDOM_PARAM ((RandomParamFn)0x8d51233cu)
#define STOCK_ARRAY_CTOR ((ArrayCtorFn)0x8fcc76bcu)
#define STOCK_PATCH_APPLY ((ApplyFn)0x8d4e9724u)
#define STOCK_APPLY_SYNTH ((ApplyFn)0x8d4f635cu)
#define STOCK_APPLY_DRUM ((ApplyFn)0x8d4f4e2cu)
#define STOCK_JSON_LOOKUP ((JsonLookupFn)0x8c085e84u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_AT ((JsonAtFn)0x8c084ee0u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_FROM_INT ((JsonFromIntFn)0x8c083b1cu)
#define STOCK_JSON_PUSH ((JsonPushFn)0x8c085d54u)
#define STOCK_JSON_ASSIGN ((JsonAssignFn)0x8c0845b8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_TO_STRING ((JsonToStringFn)0x8c084782u)
#define STOCK_FX_TYPE_OF ((FxTypeOfNameFn)0x8fca7b08u)
#define STOCK_JSON_BOOL ((JsonBoolFn)0x8c084b94u)
#define STOCK_JSON_SIZE ((JsonSizeFn)0x8c084c22u)
#define STOCK_JSON_READ_FILE ((JsonReadFileFn)0x8c0534d0u)
#define STOCK_JSON_WRITE_FILE ((JsonWriteFileFn)0x8c053168u)
#define STOCK_STRING_CTOR ((StringCtorFn)0x8c03e434u)
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define WR32(a, v) (*(volatile u32*)(a) = (u32)(v))
#define WR8(a, v) (*(volatile u8*)(a) = (u8)(v))

static const char LOG_FILE[] = "../src/main/op1f_chain.cpp";
static const char FILE_PATH[] = "user/op1f.json";             // 14 characters: short enough to live inside the string object, nothing to free
static const char FX_TYPE_KEY[] = "fx_type";
static const char FX_PARAMS_KEY[] = "fx_params";
static const char FX_ACTIVE_KEY[] = "fx_active";

enum { HOSTS = 2, SLOTS = 8, WAGONS = 4, TYPES = 11, NONE = -1, PARAMS = 8, CHAIN_TAG = 91, CHAIN_TAG_V2 = 92 };
enum { CHAIN_INTS = 3 + WAGONS + WAGONS * (2 + PARAMS) };                  // 47: the H1e to H2 chain (the tag 91)
enum { CHAIN_INTS_V2 = CHAIN_INTS + 1 };                                   // 48: H3 adds the bypass's remembered couplings (the tag 92)
enum { BUS_SAMPLES = 64 };
enum { KEY_T1 = 6, KEY_T3 = 8, KEY_CLICK0 = 54, KEY_PRESS0 = 61, PAGE_FX = 2, PAGE_LFO = 3 };
// H6h (analysis/lfo_train_study.md 5.5): the LFO's type words and parameter blocks beside the effect's, and the two descriptor
// tables the converter 0x8d510b64 reads as (min, max, step), six bytes per parameter, 48 per type; the static tables read as
// (0, 0, step) for every knob parameter, which the knobs' behaviour contradicts, so every knob turn on the effect and LFO pages
// is logged with the entry and the word before and after (H6h: the first twenty-four lines; H6i: a budget per knob and
// a range line when the knob rests, see stock_knob_turn).
static const u32 LFO_TYPE_ARRAY[HOSTS] = { 0x119035a0u, 0x119035c0u }, LFO_BLOCK[HOSTS] = { 0x119037e0u, 0x11903ae0u };
static const u32 FX_DESCRIPTORS = 0x840afe60u, LFO_DESCRIPTORS = 0x840af940u;
// The six LFO kinds' names are stock's own and are read where stock keeps them (packs L3; until then this unit carried
// them, in stock's order): six entries of eight bytes at 0x84d256c4, the name's address and the kind. That table is not in the image: one of the
// constructors the start code runs before main fills it (0x8fca6224, the word at 0x08002424 of the constructors' table;
// the walk is 0x8fcc4dd0, called at 0x11a00152 right before main). Stock's own lookup of a kind by its name, 0x8fca618c,
// walks the same six entries. This
// table says where each name's address stands. The effects' names come from stock's table 0x8490e288 (fx_name).
#define LFO_NAME_ENTRY(kind) ((const char* const*)(0x84d256c4u + 8u * (kind)))
static const char* const* const LFO_NAMES[6] = { LFO_NAME_ENTRY(0), LFO_NAME_ENTRY(1), LFO_NAME_ENTRY(2), LFO_NAME_ENTRY(3), LFO_NAME_ENTRY(4), LFO_NAME_ENTRY(5) };
#define RD16(a) (*(volatile u16*)(a))
enum { CEILING_CYCLES = 200000, BLOCK_CYCLES = 580500, REST_FRAMES = 9, SAVE_DELAY_FRAMES = 30, FLASH_FRAMES = 60, PRINT_FRAMES = 232 };
enum { HOLD_MS = 400, CLEAN_MS = 120, CYCLES_PER_MS = 400000 };
// H5 (section 21): the chain presets, a list at the screen id 37 (85 until H6c; H6d moved it below the power popup's 82) under shift + T3.
enum { PRESETS = 64, NAME_CHARS = OP1F_PRESET_NAME, PRESET_INTS = NAME_CHARS + CHAIN_INTS_V2, ROOT_TAG_V3 = 93, LIST_ROWS = OP1F_PRESET_ROWS, FLASH_MS = 1000 };   // 64 slots (H6b; H5 and H6 had 32); the file's root tag 93 carries the presets as the seventeenth element (91: none)
enum { FLASH_NONE = 0, FLASH_SAVED = 1, FLASH_DELETED = 2, FLASH_FULL = 3 };
enum { SAVE_SHOW_MS = 200, SAVE_HOLD_MS = 700, SAVED_MS = 1000 };                 // H5b: shift + T3 held: the save screen from 200 ms, the save at 700 ms, the confirmation for a second
enum { SAVE_NONE = OP1F_SAVE_NONE, SAVE_SAVING = OP1F_SAVE_SAVING, SAVE_SAVED = OP1F_SAVE_SAVED };   // the save screen's states (the header's, for the presets unit)
// H6 (section 22): the presets as files, one per slot, in the module's folder (the path user/fx/NN.json has 15 characters: inside a stock string's own buffer, nothing to free).
enum { PRESET_FORMAT = 1, JSON_OBJECT_KIND = 7, JSON_STRING_KIND = 4, JSON_BOOL_KIND = 5 };
static const char FX_FOLDER[] = "user/fx/";                                       // with the trailing slash: createFolderRecursive (0x8c05206c) makes a component when a slash ends it (H6 passed user/fx and only user was made)
static const char FX_PATH_PREFIX[] = "user/fx/";
static const char KEY_FORMAT[] = "op1f_fx_preset", KEY_NAME[] = "name", KEY_ORDER[] = "order", KEY_WAGONS[] = "wagons", KEY_FX[] = "fx", KEY_ON[] = "on", KEY_PARAMS[] = "params";
typedef int (*MakeFoldersFn)(const char* path);                                   // 0x8c05206c: createFolderRecursive (FileManager.cpp), every component made when missing
typedef int (*FileSizeFn)(const char* path);                                      // 0x8c051f9c: the file's size, -1 when it is not there
typedef u32 (*FileMtimeFn)(const char* path);                                     // 0x8fcbc350: the file's modification time, 0 when it is not there
typedef void (*JsonFromCstrFn)(void* json, const char* text);                     // 0x8c084430: the value becomes a string copied from the text (the kind 4)
typedef void (*JsonFromBoolFn)(void* json, int value);                            // 0x8c083aec: the value becomes a bool (the kind 5)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_FILE_MTIME ((FileMtimeFn)0x8fcbc350u)
#define STOCK_JSON_FROM_CSTR ((JsonFromCstrFn)0x8c084430u)
#define STOCK_JSON_FROM_BOOL ((JsonFromBoolFn)0x8c083aecu)
extern "C" u32 __attribute__((weak)) op1f_disk_folders asm("op1f_disk_folders");      // build_h_disk.cpp (H6): the disk mode's fx folder and files
extern "C" u32 __attribute__((weak)) op1f_disk_files asm("op1f_disk_files");
extern "C" u32 __attribute__((weak)) op1f_disk_lookups asm("op1f_disk_lookups");
extern "C" u32 __attribute__((weak)) op1f_disk_refused asm("op1f_disk_refused");
enum { KNOB_BLUE = 0, KNOB_OCHRE = 1, KNOB_GREY = 2, KNOB_RED = 3 };
static const char ALPHABET[] = " abcdefghijklmnopqrstuvwxyz0123456789-.";          // the rename editor's letters, in the font's lowercase (the OS font keeps the capitals' shapes at the lowercase codes; the uppercase codes are icons: H5b)
enum { ALPHABET_COUNT = 39 };
typedef void (*RequestScreenFn)();                                                   // 0x8c0b2554: the GUI asks the current controller's screen-id virtual and queues the screen (it runs after key events; the module calls it after a change at a key release)
#define STOCK_REQUEST_SCREEN ((RequestScreenFn)0x8c0b2554u)
// J5i: stock makes the request on the thread that handles key events, and the request's queue takes one writer at a
// time: its push (0x8fc232b4) stops the OS with "already pushing" when a second one enters (docs/07, "The screen
// request's queue"). A request from a frame function, which runs on the display thread, is made with interrupts
// masked: the key thread cannot be inside the push while this thread runs, and cannot enter it before the mask is
// lifted. Nothing in the request waits (0x8c0bc5cc and 0x8c0bc20c are getters, the controller's screen-id virtual
// answers from state, 0x8fc234f4 looks at the queue's last entry and pushes).
#define STOCK_IRQ_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define STOCK_IRQ_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
extern "C" int __attribute__((weak)) op1f_presets_available() asm("op1f_presets_available");   // build_h_presets.cpp (H5): the list screen exists
                  // H4b: the hold and the clean window in milliseconds of the core clock (400 MHz, the measurement unit's time base; H1 to H4 counted 23 frames for the hold and H3b 15 for the window)
enum { SWALLOW_FRAMES = 10, INIT_WAIT_FRAMES = 300 };                                // the click after a hold; the wait for a record's document at boot
enum { OBJECT_BYTES = 512 };
// H4 (section 20): the fixed state outside a producer's object, per type, and the private region a module instance gets.
enum { REGION_BYTES = 512 * 1024, REGION_COUNT = HOSTS * WAGONS, REGION_ALIGN = 32, REGION_HEAP = 0 };   // 4 MiB of C heap 0 (32 MiB, 1.4 MiB in stock's use)
enum { STATE_NONE = 0, STATE_TABLE_DELAY = 1, STATE_TABLE_CWO = 2, STATE_BASE_SPRING = 3, STATE_BASE_MOTHER = 4, STATE_STATIC_NITRO = 5 };
static const u8 STATE_KIND[TYPES] = { STATE_TABLE_DELAY, STATE_NONE, STATE_NONE, STATE_NONE, STATE_BASE_SPRING, STATE_BASE_MOTHER, STATE_STATIC_NITRO, STATE_TABLE_CWO, STATE_NONE, STATE_NONE, STATE_NONE };
static const u32 REGION_USED[TYPES] = { 176400, 0, 0, 0, 58272, 0x42318, 0, 262144, 0, 0, 0 };   // the bytes of its region a type writes, cleared at the build: the delay's 44,100-word line, the spring's 58,272, the mother's 0x42318, cwo's 128K halfwords
static const u32 DELAY_TABLE = 0x84ad0700u, CWO_TABLE = 0x84ad06f8u;             // the pointer tables the two read inside process (entry 0; stock's entries point at its two 512 KB regions 0x84ad02c4 / 0x84b502c4)
static const u32 NITRO_STATIC = 0x080266e8u;                                      // the static struct nitro's inner process keeps in L2: the words 0..2 move (the envelope follower), 4 and 5 are constants set once
static const u32 MOTHER_REGION_CTOR = 0x8d507338u;                                // constructs a mother region as the boot constructs stock's two (two sub-blocks: the positions and a 32 KB clear each)
static const u32 SPRING_BASE_STOCK = 0x84ab3fb0u, MOTHER_BASE_STOCK = 0x8402960cu;
enum { NITRO_STATIC_WORDS = 3 };
typedef void* (*HeapMallocFn)(int heap_index, u32 size);                          // 0x8fcc5a68 (build_b_cache.cpp)
typedef void (*RegionCtorFn)(void* region);
#define STOCK_HEAP_MALLOC ((HeapMallocFn)0x8fcc5a68u)
typedef void (*FreeFn)(void* block);                                               // 0x8d4e63cc: the match of 0x8d4e63bc, the allocator stock's strings use
#define STOCK_FREE ((FreeFn)0x8d4e63ccu)
extern "C" volatile u32 op1f_spring_base asm("op1f_spring_base");                  // read by the stubs of build_h_regions.s in place of stock's immediates
extern "C" volatile u32 op1f_mother_base asm("op1f_mother_base");
volatile u32 op1f_spring_base = SPRING_BASE_STOCK;
volatile u32 op1f_mother_base = MOTHER_BASE_STOCK;
static const u32 MODE_GUI = 0x84d21088u, MODE_AUDIO = 0x11902da4u, FX_PRODUCER = 0x84a03c84u, STOCK_CTX = 0x11904754u;
static const u32 L_BUS = 0x11902ba0u, R_BUS = 0x11902ca0u, FX_NAME_TABLE = 0x8490e288u;
static const u32 POWER_POPUP_SHOWN = 0x84c9b3e4u;             // the power popup's byte: 1 while the countdown popup is on the screen (set by 0x8fc6144c, cleared by 0x8fc61438)

struct HostAddr {
    u32 slot_word, built_slot, type_array, param_block, fx_active, patch_table;
    u32 key_down, key_shift, key_up, encoder;                 // stock's handlers, called for what the module leaves alone
};
static const HostAddr HOST[HOSTS] = {
    { 0x11904704u, 0x84a01524u, 0x11904794u, 0x119036e0u, 0x84a0154cu, 0x080235dcu, 0x8c0cfe7cu, 0x8c0cf904u, 0x8c0cf364u, 0x8c0d5c10u },
    { 0x11904708u, 0x84a01528u, 0x119047b4u, 0x119039e0u, 0x84a0155cu, 0x0802377cu, 0x8c0cfe7cu, 0x8c0b6e1eu, 0x8c0cf364u, 0x8c0b7634u },
};

// The knob walks the browser's order; the vocoder is not offered.
static const signed char KNOB_ORDER[] = { NONE, 1, 2, 0, 3, 4, 7, 6, 5, 9, 10 };
enum { KNOB_ORDER_COUNT = 11, TYPE_DELAY = 0, TYPE_NITRO = 6, TYPE_TERMINAL = 9 };
static const u32 CTOR[TYPES] = { 0x8d5061fcu, 0x8d508984u, 0x8d50f8f0u, 0x8d511ac8u, 0x8d517c84u, 0x11a071bau, 0x8d50822cu, 0x8d50256cu, 0, 0, 0x8d510616u };
static const u32 COST_MAX[TYPES] = { 22400, 53000, 83100, 47400, 49600, 44300, 40800, 44500, 0, 36300, 16800 };   // H1a, cycles per block
static const short DEFAULTS[TYPES][PARAMS] = {                                                                     // content/op1_factory.json, fx_types[].default_params
    { 8000, 8000, 8000, 8000, 0, 0, 0, 0 }, { 8000, 8000, 18000, 18000, 8000, 8000, 8000, 8000 }, { 6000, 15000, 20000, 28000, 8000, 8000, 8000, 8000 },
    { 8000, 8000, 8000, 16000, 8000, 8000, 8000, 8000 }, { 8000, 15000, 2000, 20000, 8000, 8000, 8000, 8000 }, { 17000, 0, 17000, 17000, 0, 0, 0, 0 },
    { 500, 0, 10500, 16000, 0, 0, 0, 0 }, { 5000, 8192, 16384, 32767, 0, 0, 0, 0 }, { 16384, 16384, 16384, 11253, 0, 0, 0, 0 },
    { 18203, 13235, 8091, 32767, 0, 0, 0, 0 }, { 7802, 20560, 9322, 21695, 0, 0, 0, 0 } };

// Wagons offered by other packs (op1f_wagon.h): each under an optional name of this list, one line a wagon. The ones
// that are there are the types TYPES + their place in the list, at the end of every knob's list. The files keep such a
// wagon by its name (wagon_code): an fx train that does not know it, or one whose pack is away, reads an empty wagon.
extern "C" const Op1fWagon __attribute__((weak)) op1f_wagon_autotune asm("op1f_wagon_autotune");
extern "C" const Op1fWagon __attribute__((weak)) op1f_wagon_juneaux asm("op1f_wagon_juneaux");
extern "C" const Op1fWagon __attribute__((weak)) op1f_wagon_tape asm("op1f_wagon_tape");
extern "C" const Op1fWagon __attribute__((weak)) op1f_wagon_gate asm("op1f_wagon_gate");
extern "C" const Op1fWagon __attribute__((weak)) op1f_wagon_freeze asm("op1f_wagon_freeze");
static const Op1fWagon* const OFFERED[] = { &op1f_wagon_autotune, &op1f_wagon_juneaux, &op1f_wagon_tape, &op1f_wagon_gate, &op1f_wagon_freeze };
enum { PLUGS = sizeof(OFFERED) / sizeof(OFFERED[0]), ALL_TYPES = TYPES + PLUGS };

static const Op1fWagon* wagon_of(int type) {                     // the offered wagon of a type, or 0 (a stock type, none, a pack away)
    if (type < TYPES || type >= ALL_TYPES) return 0;
    const Op1fWagon* w = OFFERED[type - TYPES];
    return w && w->magic == OP1F_WAGON_MAGIC && (w->version == 1 || w->version == OP1F_WAGON_VERSION) && w->name && w->process && w->bytes <= REGION_BYTES ? w : 0;
}

static int wagon_code(const char* name) {                         // the name's first four characters as one integer: above every stock type
    u32 c = 0;
    for (int i = 0; i < 4; ++i) { c = c << 8 | (u8)*name; if (*name) ++name; }
    return (int)c;
}

static int type_stored(int type) { const Op1fWagon* w = wagon_of(type); return w ? wagon_code(w->name) : type; }

static int type_loaded(int value) {                               // a stored type: a stock one, an offered wagon's code, else none
    if (value >= 0 && value < TYPES) return value;
    for (int t = TYPES; t < ALL_TYPES; ++t) if (wagon_of(t) && wagon_code(wagon_of(t)->name) == value) return t;
    return NONE;
}

static int default_param(int type, int i) { const Op1fWagon* w = wagon_of(type); return w ? w->defaults[i] : DEFAULTS[type][i]; }
static u32 cost_of(int type) { const Op1fWagon* w = wagon_of(type); return w ? w->cost : (type >= 0 && type < TYPES ? COST_MAX[type] : 0u); }

struct Wagon { i32 type; i32 coupled; u32 params[PARAMS]; };
struct Stat { u32 sum, count, max; };
struct Live {                                                  // a module-built instance, one per host and knob
    void* volatile obj;                                        // the audio thread reads this once per block
    u32 ctx[PARAMS];                                           // the eight parameter words the instance reads
    i32 type;                                                  // the type built, or NONE
    u32 which;                                                 // the buffer in use
    u32 buf[2][OBJECT_BYTES / 4];
    u8* region;                                                // H4: the instance's private state region (REGION_BYTES), or 0 without the pool
    u32 nitro_state[NITRO_STATIC_WORDS];                       // H4: a nitro instance's copy of the static words, swapped in around its call
    volatile int kind;                                         // H4: the state kind of the built type (written before obj; the audio thread reads it after obj)
};
struct Json { u32 words[4]; };                                 // a stock json value is 12 bytes; 16 for alignment

static Wagon wagons[HOSTS][SLOTS][WAGONS];
static volatile u32 order_word[HOSTS][SLOTS];                  // byte p = the knob at position p; written whole
static volatile u8 mirror[HOSTS][SLOTS];                       // the knob stock's slot shows
static i32 doc_type[HOSTS][SLOTS];                             // the document's effect type the chain was last reconciled with
static u8 known[HOSTS][SLOTS];                                 // the slot's chain is the module's own (from the file, or made in this session)
static int file_loaded;
static int keep_h5_presets;                                    // H6b: the H5 presets element is written back as read while its presets could not become files
static Json h5_presets;
enum { JSON_ARRAY_KIND = 6 };                                  // the kind byte (json + 4) of an array: the list walk of the size getter's table (H1g; H1f had learnt 0, the null a fresh value starts as)
static u8 initialised[HOSTS][SLOTS];
static u8 bypass_mask[HOSTS][SLOTS];                           // H3: the couplings shift + T3 parked, 1 << knob; always 0 since H5 (a file's mask re-couples at the load)
static struct {                                                // H6: the presets store, one slot per file user/fx/NN.json, shared by both hosts
    u8 used[PRESETS];                                          // the slot's file holds a preset
    char name[PRESETS][NAME_CHARS + 1];                        // per slot, in the font's lowercase
    int values[PRESETS][CHAIN_INTS_V2];                        // per slot: a tag-92 chain (chain_values); the mirror and the document type are the slot's own at the load
    u32 mtime[PRESETS];                                        // the file's time at the last read
    int size[PRESETS];                                         // and its size (a changed file is read again at the next scan)
    int order[PRESETS];                                        // the used slots in slot order: the list's rows
    int count;                                                 // used slots
    int scanned;                                               // the folder was scanned once (the first file load)
} presets;
static struct {                                                // H5: the list's state
    int open[HOSTS], cursor[HOSTS], first[HOSTS], editing[HOSTS], caret[HOSTS];
    int loaded_slot[HOSTS];                                    // the slot the chain was loaded from or saved as, or -1 (H6: a slot, not a row)
    char buffer[HOSTS][NAME_CHARS + 1];                        // the rename editor's text
    int flash_kind, flash_slot, flash_until_ms;                // one row's word for a second: saved, deleted, list full (the row of that slot)
    int pending_delete_slot;                                   // the slot emptied when its deleted flash ends, or -1
    int t3_pressed[HOSTS], t3_ms[HOSTS], t3_acted[HOSTS];      // shift + T3 held: the save after HOLD_MS, else the tap at the release
    int pressed[HOSTS][WAGONS], press_ms[HOSTS][WAGONS], acted[HOSTS][WAGONS], swallow_until[HOSTS][WAGONS];   // the knobs on the list (the red hold deletes)
    int keep_loaded;                                           // mark_dirty leaves `loaded` alone while a preset is saved, loaded, renamed or deleted
    int save_state, save_host, save_slot, save_ms;             // H5b: the save screen: SAVING from 200 ms of the hold (save_ms the press), SAVED for a second after a save (save_ms the save); the slot shown as its number
    char row_text[LIST_ROWS][NAME_CHARS + 1];
    char wagon_text[WAGONS][NAME_CHARS + 6];
    char cell_text[NAME_CHARS][2];
    u32 opens, saves, loads, deletes, renames;
    u32 mirror_moves;                                          // H6g: loads that moved the mirror to a loaded effect (the mirrored knob's wagon was empty)
    u32 scans, rereads, writes, write_failures, migrated, folder_made;   // H6: the files (H6b: the failed writes counted)
} ls;
static volatile u32 bus_peak[HOSTS];                           // H3: the instrument bus's largest sample since the frame last read it (the audio thread writes, the frame clears)
static i32 stock_type_seen[HOSTS][SLOTS];                      // the slot's type word at the last look
static u32 memory[HOSTS][SLOTS][WAGONS][ALL_TYPES][PARAMS];     // each wagon's parameters per effect it carried
static u8 memory_set[HOSTS][SLOTS][WAGONS][ALL_TYPES];
static Live live[HOSTS][WAGONS];
static u8* region_pool;                                        // H4: the heap-0 block the regions are cut from (0 when refused)
static int regions_ready;
static volatile int chain_ready[HOSTS];
static volatile Stat chain_stat[HOSTS];
static volatile Stat wagon_stat[HOSTS][WAGONS];

static struct {
    u32 frames;
    int print_countdown;
    int train_open[HOSTS];
    int current_slot[HOSTS];                                   // the slot whose chain is live, or -1
    u32 ms, last_cycles, cycle_acc;                            // H4b: milliseconds of the core clock since the first frame, from the CYCLES deltas between frames
    u32 frames_at_clock, ms_at_clock;                          // H4b: the last `chain clock:` line's frame count and time
    int pressed[HOSTS][WAGONS], press_ms[HOSTS][WAGONS], press_shift[HOSTS][WAGONS], acted[HOSTS][WAGONS];   // H4b: the press timed in milliseconds
    int swallow_until[HOSTS][WAGONS];                          // the click that follows a hold is not a tap
    int hold_armed[HOSTS][WAGONS];                             // H3b: the press was clean (no turn of the knob in the quarter second before it)
    int last_turn_ms[HOSTS][WAGONS];                           // H3b: the knob's last encoder step on the train (H4b: in milliseconds)
    int pending_type[HOSTS][WAGONS], pending_since[HOSTS][WAGONS];   // a pick waiting for the knob's rest
    int push_pending[HOSTS];                                   // the mirror's pick waiting for the rest
    int dirty[HOSTS], dirty_since[HOSTS];
    int flash_until[HOSTS];
    int turn_knob[HOSTS], turn_frame[HOSTS];                   // the knob last turned on the train, for the page's card (H2)
    int turn_move[HOSTS];                                      // that turn was a move (H3: the page's hop)
    int refused_knob[HOSTS], refused_type[HOSTS];              // the pick that did not fit, shown during the flash (H3)
    u32 picks, moves, taps, holds, randoms, resets, refusals, saves, loads, adopts, builds, page_opens, train_opens, mirror_syncs, bypasses;
    u32 inits, swallowed, reconciles, restores;
    u32 turn_presses, follows;                                 // H3b: presses that armed no hold; the mirror's coupling following stock's fx_active byte
    u32 diag_lines;                                            // reconcile log lines printed (the first forty)
    u32 doc_writes, doc_lines, doc_ms;                         // H6c: the record's document rewritten for the mirror (the lines printed, the first twenty-four; the time of the last)
    u32 knob_turns, turn_lines[2], range_lines, row_mask[2];   // H6h, H6i: knob turns on the effect and LFO pages passed to stock; the per-turn lines printed (effect, LFO), the range lines, the descriptor rows printed (a bit per type)
    u8 probes_full;                                            // H6i: the table of knobs is full and its line is printed
    u32 clamps, ghosts, value_lines, rows_next;                // H6j: stored parameters held to their knob's range, ghost delays taken as empty, their lines (the first twenty-four), the next descriptor row of the boot's print
    u32 param_follows, early_follows, param_lines;             // H6c: the mirrored wagon's parameters following the slot's block; those within two seconds of a document write (the first twenty-four logged)
    u32 lfo4_carried;                                          // J5f: sounds whose LFOs the chain file held while the LFO unit was away, carried to the next save
    char line[200];
    QuietGate quiet;                                           // Q1: the gate of the periodic lines (build_q_quiet.h); the last field, so that no other field's place moved
} st;

extern "C" u32 op1f_fx_t0[HOSTS] asm("op1f_fx_t0");          // the start stubs' stamps (build_h_fx.cpp)
extern "C" u32 __attribute__((weak)) op1f_fx_voices asm("op1f_fx_voices");   // Q1 (build_h_fx.cpp): what the synth stage's own records counted in the last period, 0 while no synth voice sounds
extern "C" void __attribute__((weak)) op1f_lfopage_frame() asm("op1f_lfopage_frame");     // build_i_lfopage.cpp (I2, I3): the LFO pages' wagon marker, every frame
extern "C" void __attribute__((weak)) op1f_lfo4_block(int host) asm("op1f_lfo4_block");   // build_i_lfo4.cpp (I5, I6): the three other LFOs' fx-side calls, first in the chain's run
extern "C" void __attribute__((weak)) op1f_lfo4_frame() asm("op1f_lfo4_frame");           // build_i_lfo4.cpp: its frame work on the GUI thread (the instances built, the holds, the log)
// I6: the LFO train on T4 (build_i_lfo4.cpp). The key functions answer -1 for a key that is not theirs.
extern "C" int __attribute__((weak)) op1f_lfotrain_key_down(void* self, const int* event, int shifted) asm("op1f_lfotrain_key_down");
extern "C" void __attribute__((weak)) op1f_lfotrain_key_up(int key) asm("op1f_lfotrain_key_up");
extern "C" int __attribute__((weak)) op1f_lfotrain_encoder(void* self, int index, int delta) asm("op1f_lfotrain_encoder");
extern "C" void __attribute__((weak)) op1f_lfo4_store_load(int slot, const int* values) asm("op1f_lfo4_store_load");   // a synth slot's three other LFOs and its lamp, from the chain file
extern "C" int __attribute__((weak)) op1f_lfo4_store_save(int slot, int* values) asm("op1f_lfo4_store_save");         // the same for the file; 0 while the slot has nothing but stock's one LFO on the first wagon
extern "C" int __attribute__((weak)) op1f_train_available() asm("op1f_train_available");   // build_h_train.cpp (H2): the train screen exists
// I8: the LFO presets (build_i_lfopresets.cpp), a library of its own beside this unit's: its folder and files at the first file load, its frame work
extern "C" void __attribute__((weak)) op1f_lfopresets_scan() asm("op1f_lfopresets_scan");
extern "C" void __attribute__((weak)) op1f_lfopresets_frame() asm("op1f_lfopresets_frame");
// I9: an instrument carries its train and its LFOs (build_i_patch.cpp): what a patch loaded from a file brought for its slot, and that unit's frame work
extern "C" int __attribute__((weak)) op1f_patch_pending(int host, int slot, int* out) asm("op1f_patch_pending");
extern "C" void __attribute__((weak)) op1f_patch_frame() asm("op1f_patch_frame");

static u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c));
    return c;
}

static u32 udiv(u32 n, u32 d) {
    if (d == 0) return 0;
    u32 q = 0, r = 0;
    for (int i = 31; i >= 0; --i) {
        r = (r << 1) | ((n >> i) & 1u);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    return q;
}

static char* put_text(char* p, const char* s) {
    while (*s) *p++ = *s++;
    return p;
}

static char* put_u32(char* p, u32 v) {
    char digits[10];
    int n = 0;
    do {
        u32 q = udiv(v, 10);
        digits[n++] = (char)('0' + (v - q * 10));
        v = q;
    } while (v);
    while (n) *p++ = digits[--n];
    return p;
}

static const char* fx_name(int type) {
    if (type < 0) return "none";
    if (type >= TYPES) return wagon_of(type) ? wagon_of(type)->name : "?";
    return *(const char* const*)(FX_NAME_TABLE + 8u * (u32)type);
}

static int host_of(void* self) { return *(int*)((u8*)self + 0xc) ? 1 : 0; }
static int* page_of(void* self) { return (int*)((u8*)self + 0x28); }
static u32 gui_slot(int host) { return RD32(HOST[host].slot_word) & 7u; }
static u32 patch_record(int host, u32 slot) { return HOST[host].patch_table + 52u * slot; }
static int knob_at(int host, u32 slot, int pos) { return (int)((order_word[host][slot] >> (8 * pos)) & 3u); }
static int in_own_mode(int host) { return (int)RD32(MODE_GUI) == host && (int)RD32(MODE_AUDIO) == host; }
static u32 word_of(int value) { return (u32)value << 16; }
static int int_of(u32 word) { return (int)word >> 16; }

static void set_order(int host, u32 slot, const int* knobs) {
    u32 w = 0;
    for (int p = 0; p < WAGONS; ++p) w |= (u32)(knobs[p] & 3) << (8 * p);
    order_word[host][slot] = w;
}

static int conflicts(int a, int b) {                            // the H1 rule: kept for the fallback without regions only (H4)
    if (regions_ready) return 0;
    if (a < 0 || b < 0) return 0;
    if (a == b) return 1;
    return (a == TYPE_DELAY && b == TYPE_NITRO) || (a == TYPE_NITRO && b == TYPE_DELAY);
}

// ---------------------------------------------------------------- the instances

// An offered wagon's instance, in the shape the run calls every instance: a table whose [2] is process and [3] init,
// then the wagon and its state (the Live's region).
static void plug_process(void* obj, int* left, int* right, void* ctx) {
    u32* o = (u32*)obj;
    ((const Op1fWagon*)o[1])->process((void*)o[2], left, right, (const volatile unsigned*)ctx);
}

static void plug_init(void* obj, int) {
    u32* o = (u32*)obj;
    const Op1fWagon* w = (const Op1fWagon*)o[1];
    if (w->init) w->init((void*)o[2]);
}

static const struct { void* none[2]; ProcessFn process; InitFn init; } PLUG_TABLE = { { 0, 0 }, plug_process, plug_init };

static void construct(void* storage, int type, u8* region) {
    if (const Op1fWagon* w = wagon_of(type)) {
        u32* o = (u32*)storage;
        o[0] = (u32)&PLUG_TABLE;
        o[1] = (u32)w;
        o[2] = (u32)region;
        return;
    }
    if (type == TYPE_TERMINAL) {                               // the factory's own sequence (producer_factory.cpp)
        u8* b = (u8*)storage;
        *(u32*)b = 0x840ae684u;
        STOCK_ARRAY_CTOR(b + 0x0c, 4, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x2c, 4, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x4c, 2, 8, (void*)0x8f011052u, 0);
        STOCK_ARRAY_CTOR(b + 0x64, 2, 16, (void*)0x8f011038u, 0);
        STOCK_ARRAY_CTOR(b + 0x84, 2, 16, (void*)0x8f011038u, 0);
        return;
    }
    ((CtorFn)CTOR[type])(storage);
}

static int buildable(int type) { return type >= 0 && (type < TYPES ? CTOR[type] != 0 || type == TYPE_TERMINAL : wagon_of(type) && regions_ready); }

// Builds the wagon's instance of `type` in the spare buffer and publishes it; anything else unpublishes. H4: an
// instance whose type keeps state outside its object gets the Live's private region first (cleared over the bytes the
// type writes; a mother region constructed as the boot constructs stock's two), and a Live that still holds an
// instance is only unpublished this frame: the build follows at the next frame's call, when the audio thread has left
// the old instance and its region may be cleared (the frame is 17 ms, the block 1.45 ms).
static void build_live(int host, int knob, int type) {
    Live* l = &live[host][knob];
    if (!buildable(type)) {
        l->obj = 0;
        l->type = NONE;
        l->kind = STATE_NONE;
        return;
    }
    if (l->obj) {
        l->obj = 0;
        l->type = NONE;                                            // the pending type differs from NONE: the caller comes back next frame
        return;
    }
    int kind = regions_ready && type < TYPES ? STATE_KIND[type] : STATE_NONE;
    const Op1fWagon* plug = wagon_of(type);
    if (kind != STATE_NONE || plug) {                              // an offered wagon's state is its region, cleared over the bytes it asked for
        u32* r = (u32*)l->region;
        u32 words = (plug ? plug->bytes + 3u : REGION_USED[type]) / 4;
        for (u32 i = 0; i < words; ++i) r[i] = 0;
        if (kind == STATE_BASE_MOTHER) ((RegionCtorFn)MOTHER_REGION_CTOR)(l->region);
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) l->nitro_state[i] = 0;
    }
    u32 which = l->which ^ 1u;
    u32* buf = l->buf[which];
    for (int i = 0; i < OBJECT_BYTES / 4; ++i) buf[i] = 0;
    construct(buf, type, l->region);
    u32* vtable = *(u32**)buf;
    ((InitFn)vtable[3])(buf, 0);
    l->which = which;
    l->type = type;
    l->kind = kind;
    asm volatile("" ::: "memory");                                 // the kind and the region before the pointer the audio thread reads
    l->obj = buf;
    ++st.builds;
    STOCK_LOG(3, LOG_FILE, 10, "chain build: %s knob %d %s, state kind %d, region %08x", host ? "drum" : "synth", knob, fx_name(type), kind, (u32)l->region);
}

// ---------------------------------------------------------------- the audio thread

static void run_one(int host, int knob, void* obj, void* ctx) {
    u32* vtable = *(u32**)obj;
    if (vtable == 0) return;
    u32 t = cycles();
    ((ProcessFn)vtable[2])(obj, (int*)L_BUS, (int*)R_BUS, ctx);
    u32 dt = cycles() - t;
    volatile Stat* s = &wagon_stat[host][knob];
    s->sum += dt;
    s->count += 1;
    if (dt > s->max) s->max = dt;
}

// H4: a module instance's call with its fixed state pointed at its private region (section 20): the pointer-table
// entry the delay or cwo reads inside process, the base word the spring's or mother's stubs load, or nitro's three
// static words copied in and out; stock's values are put back after the call, so stock's own instances (the mirror,
// the master effect) never see the module's regions. The kind is read after obj (build_live publishes it before).
static void run_live(int host, int knob, void* obj, void* ctx) {         // I2: ctx is the instance's own words, or the copy stock's LFO turned this block
    Live* l = &live[host][knob];
    switch (l->kind) {
    case STATE_TABLE_DELAY: {
        volatile u32* entry = (volatile u32*)DELAY_TABLE;
        u32 saved = entry[0];
        entry[0] = (u32)l->region;
        run_one(host, knob, obj, ctx);
        entry[0] = saved;
        break;
    }
    case STATE_TABLE_CWO: {
        volatile u32* entry = (volatile u32*)CWO_TABLE;
        u32 saved = entry[0];
        entry[0] = (u32)l->region;
        run_one(host, knob, obj, ctx);
        entry[0] = saved;
        break;
    }
    case STATE_BASE_SPRING: {
        u32 saved = op1f_spring_base;
        op1f_spring_base = (u32)l->region;
        run_one(host, knob, obj, ctx);
        op1f_spring_base = saved;
        break;
    }
    case STATE_BASE_MOTHER: {
        u32 saved = op1f_mother_base;
        op1f_mother_base = (u32)l->region;
        run_one(host, knob, obj, ctx);
        op1f_mother_base = saved;
        break;
    }
    case STATE_STATIC_NITRO: {
        volatile u32* words = (volatile u32*)NITRO_STATIC;
        u32 saved[NITRO_STATIC_WORDS];
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) { saved[i] = words[i]; words[i] = l->nitro_state[i]; }
        run_one(host, knob, obj, ctx);
        for (int i = 0; i < NITRO_STATIC_WORDS; ++i) { l->nitro_state[i] = words[i]; words[i] = saved[i]; }
        break;
    }
    default:
        run_one(host, knob, obj, ctx);
        break;
    }
}

// ---------------------------------------------------------------- I2: the stock LFO's effect destination on any wagon
//
// The owner's rule (analysis/lfo_train_study.md section 10): the LFO is stock's own, with every target stock has;
// the one thing added is that its effect destination can turn the knobs of any effect wagon of the train, not
// only the wagon under the lamp (stock's slot effect).
//
// How stock reaches its slot effect (sections 5.3, 8.3, 10.1): once a block the second stage copies the slot's
// eight effect words into the scratch 0x11904754, then calls the fx-side LFO object's apply,
// object->vtable[2](object, the slot's LFO block, 2 for the synth or 5 for the drum), which adds its offset to the
// scratch through stock's converter with the descriptor row of the slot's effect type (the word at type_array +
// 4 x slot), then the effect runs on the scratch.
//
// What the module does, with stock's own code doing every part of the LFO: a stub in place of the 8-byte load
// before that call (0x11a05500 for the synth, 0x11a0404e for the drum; build_i_lfofx.s) hands stock a table whose
// third word is the wrapper below, so stock calls the wrapper with its own arguments. For a wagon other than the
// lamp's, the wrapper puts that wagon's eight words into the scratch and its effect type into the slot's type word,
// calls stock's apply, keeps what the scratch holds after it for the wagon's run of this block, and puts the lamp's
// words and type back. The audio thread runs a block to its end before the GUI thread runs again (one core), so
// nothing else sees the two swapped for the length of the call. The wagon's own words, which the knobs, the chain
// file and the preset files hold, are never written.
//
// Which wagon: one choice per sound slot, 0 for the wagon under the lamp (stock's own way, and what every sound has
// until the destination knob is turned inside the effect's range) or 1 to 4 for the wagon of a knob. A chosen wagon
// that carries no effect counts as no choice: the LFO turns the lamp's effect, as stock. The choice is kept in the
// chain file after a chain's 48 integers (the tag 96 and the choice), not in stock's document.
//
// Where the swap is made: only for the LFO types whose apply has been read through for what it reads for the effect
// (the slot from 0x8d503078 / 0x8d503064, the effect type from the slot's type word, that type's flag byte and
// descriptor row, the scratch), told by the class of the fx-side object, and only while the type's destination is
// the effect as its own apply decodes the destination word. I2: the value LFO (the apply 0x8d50c308, the vtable
// 0x840ae3bc, the word's top four bits 2). I3: the random, element and velocity LFOs (the applies 0x8d50bbc0,
// 0x8d50a8b0, 0x8d50cf14, the vtables 0x840ae258, 0x840ae0b0, 0x840ae550, the word's top five bits 2 through the
// decoders 0x8d50b8a6, 0x8d50a44c, 0x8d50cc2e; what else they read is their own: the random's generator and seed,
// the element's and the velocity's source buffers at 0x080268a4 and 0x08026aa8, none of it the scratch or the
// type word). Every other class and destination passes to stock's apply untouched.
//
// I4: the midi LFO (the apply 0x8d50a23c, the vtable 0x840adf4c) has four destinations of its own, the block's words
// 4 to 7 (the same four classes as the three above), each with its parameter in the word four before it and its
// value in the four words at 0x0802e3c4; the apply walks the four in one call and keeps nothing between calls. Each
// destination has a choice of wagon of its own, so the wrapper calls stock's apply once for the lamp's effect with
// the other wagons' destinations masked (the class 0, which this call's class does nothing with), then once per
// other wagon with only that wagon's destinations left, that wagon's words and type swapped in as above. With no
// destination on another wagon than the lamp's it is one call of stock's apply, untouched.
enum { LFO_TYPE_VALUE = 1, LFO_TYPE_RANDOM = 2, LFO_TYPE_ELEMENT = 3, LFO_TYPE_MIDI = 4, LFO_TYPE_VELOCITY = 5, LFO_DEST_KNOB = 2, LFO_DEST_WORD = 2, LFO_DEST_FX = 2, LFOFX_TAG = 96, LFOFX_LINES = 48, LFOFX_PERIOD_LINES = 64 };
// I4: the midi LFO's four destinations: the block's words 4 to 7, set by the shifted knobs (the handler's indexes 4 to 7); the tag of their choices in the chain file
enum { MIDI_DESTS = 4, LFO_MIDI_DEST_WORD = 4, LFO_MIDI_DEST_KNOB = 4, LFOMIDI_TAG = 97 };
// I6: a synth slot's three other LFOs and its lamp ride after the two pairs: the tag 98 and the LFO unit's integers (build_i_lfo4.h)
enum { LFO4_TAG = 98, LFO4_INTS = OP1F_LFO4_STORE_INTS, LFO4_MIN = OP1F_LFO4_STORE_MIN, LFO4_AT = CHAIN_INTS_V2 + 4 };   // I7: one more integer than I6 wrote (the order on the page); a file of I6's is read with it missing
// the value LFO's destination word, as the H6i read-out and the apply 0x8d50c308 give it: the value's top four bits
// the destination (2 the effect), bit 11 the form shared by all voices (the page's FREE); the knob runs from 1024 to 11264
enum { LFO_FX_LO = 8192, LFO_FX_SHARED = 10240, LFO_FX_HI = 11264, LFO_FX_PARK = 9216, LFO_FX_PARK_SHARED = 10752, LFO_UNDER_FX = 7168 };
// I3: the random, element and velocity LFOs' destination word, as their decoders read it with the class of the call:
// the value's top five bits, four classes of 2048 (0 the engine, 1 the envelope, 2 the effect, 3 the three direct
// ones), no shared form; the knob runs from 1024 to 7168 (the I2 read-out's rows). The parked values are the middles.
enum { LFO4_FX_LO = 4096, LFO4_FX_HI = 6144, LFO4_FX_PARK = 5120, LFO4_UNDER_FX = 3072, LFO4_OVER_FX = 6656 };
static const u32 VALUE_LFO_FX_VTABLE = 0x840ae3bcu;           // the value LFO's fx-side class (one object at 0x11906ef8 for both hosts)
static const u32 RANDOM_LFO_FX_VTABLE = 0x840ae258u, ELEMENT_LFO_FX_VTABLE = 0x840ae0b0u, VELOCITY_LFO_FX_VTABLE = 0x840ae550u;   // I3: the three's fx-side classes (the same object, built again by the type's init)
static const u32 MIDI_LFO_FX_VTABLE = 0x840adf4cu;            // I4: the midi LFO's
static const u32 LFO_FX_OBJECT[HOSTS] = { 0x84a01570u, 0x84a01574u };   // the hosts' fx-side LFO object pointers (the stages read them at 0x11a051be and 0x11a03d6e), for the log
typedef int (*LfoApplyFn)(void* object, u32* block, int cls);

static u8 lfo_fx_pick[HOSTS][SLOTS];                           // 0 the wagon under the lamp; 1..4 the wagon of knob 0..3
static u8 lfo_midi_pick[HOSTS][SLOTS][MIDI_DESTS];             // I4: the same choice for each of the midi LFO's four destinations

// J5f: the LFO unit's integers of each synth slot, carried while the LFO unit is away. The pack `lfo` is a file of its
// own since J5f, and with it away this unit would write the chain file, and an instrument's file, without the three
// other LFOs of each sound: the next save would drop them (analysis/pack_loader_study.md 17.2). So what the chain file
// or an instrument held is kept as it was read and written back as it was, and the LFOs are there again when the pack
// is. With the LFO unit there nothing here is used.
// lfo4-carry-begin
static int lfo4_kept[SLOTS][LFO4_INTS];
static u8 lfo4_kept_has[SLOTS];

static void lfo4_keep(u32 slot, const int* more) {
    for (int i = 0; i < LFO4_INTS; ++i) lfo4_kept[slot][i] = more[i];
    lfo4_kept_has[slot] = 1;
}

static void lfo4_drop(u32 slot) { lfo4_kept_has[slot] = 0; }

static int lfo4_carried(u32 slot, int* more) {
    if (!lfo4_kept_has[slot]) return 0;
    for (int i = 0; i < LFO4_INTS; ++i) more[i] = lfo4_kept[slot][i];
    return 1;
}
// lfo4-carry-end
struct FxMod {                                                 // the audio thread's: a wagon's words as stock's LFO left them this block
    u32 words[PARAMS];
    void* obj;                                                 // the instance they were made for: another instance on the knob runs on its own words
    int valid;
};
static FxMod fx_mod[HOSTS][WAGONS];                            // by the wagon's knob (I4: the midi LFO can turn several wagons in a block; I2 and I3 kept one set a host)
static volatile u32 lfofx_turned[HOSTS], lfofx_idle[HOSTS], lfofx_stock[HOSTS];   // blocks: a wagon's words turned; the LFO kept off the lamp's effect with its wagon parked or not built; stock's own way
extern "C" volatile u32 op1f_lfofx_calls[HOSTS] asm("op1f_lfofx_calls");
volatile u32 op1f_lfofx_calls[HOSTS];                           // I5: stock's fx-side LFO applies the wrapper has seen, by host (build_i_lfo4.cpp calls its twin once for each)
static u32 lfofx_lines, lfofx_period_lines;                    // the knob's lines and the periodic lines printed

// The wagon the LFO's effect destination turns: the chosen one while it carries an effect, else the one under the lamp.
static int lfo_fx_knob_of(int host, u32 slot) {
    int pick = lfo_fx_pick[host][slot];
    return (pick && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// I4: the wagon the midi LFO's destination i turns, by the same rule.
static int lfo_midi_knob_of(int host, u32 slot, int i) {
    int pick = lfo_midi_pick[host][slot][i];
    return (pick && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// Is the effect this class's destination, as its own apply decodes the block's destination word. 0 for a class
// whose apply has not been read through: stock's way.
static int lfo_class_to_fx(u32 vtable, const u32* block) {
    i32 word = (i32)block[LFO_DEST_WORD];
    if (vtable == VALUE_LFO_FX_VTABLE) return (word >> 28) == (i32)LFO_DEST_FX;
    if (vtable == RANDOM_LFO_FX_VTABLE || vtable == ELEMENT_LFO_FX_VTABLE || vtable == VELOCITY_LFO_FX_VTABLE) return (word >> 27) == (i32)LFO_DEST_FX;
    return 0;
}

// I4: the midi LFO. target[i] is the knob of the wagon destination i turns, or -1 when it is not the effect.
static int lfofx_apply_midi(int host, void* object, u32* block, int cls, LfoApplyFn real, u32 slot) {
    int target[MIDI_DESTS];
    int lamp = mirror[host][slot];
    int other = 0;
    for (int i = 0; i < MIDI_DESTS; ++i) {
        target[i] = ((i32)block[LFO_MIDI_DEST_WORD + i] >> 27) == (i32)LFO_DEST_FX ? lfo_midi_knob_of(host, slot, i) : -1;
        if (target[i] >= 0 && target[i] != lamp) other = 1;
    }
    if (!other || !chain_ready[host]) {
        lfofx_stock[host] = lfofx_stock[host] + 1;             // every effect destination on the lamp's wagon, or none: stock's own way, untouched
        return real(object, block, cls);
    }
    u32 part[PARAMS];
    for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
    for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] >= 0 && target[i] != lamp) part[LFO_MIDI_DEST_WORD + i] = 0;
    int result = real(object, part, cls);                      // the lamp's effect, by the destinations that are its own
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];         // the scratch as that call left it: the lamp's effect runs on it
    u32 kept_type = *type_word;
    for (int knob = 0; knob < WAGONS; ++knob) {
        if (knob == lamp) continue;
        int mine = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] == knob) mine = 1;
        if (!mine) continue;
        Live* l = &live[host][knob];
        const Wagon* w = &wagons[host][slot][knob];
        void* obj = l->obj;
        i32 type = l->type;
        if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
            lfofx_idle[host] = lfofx_idle[host] + 1;           // parked or not built this block: nothing to turn, and the apply keeps no state to run on
            continue;
        }
        for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] != knob) part[LFO_MIDI_DEST_WORD + i] = 0;
        for (int i = 0; i < PARAMS; ++i) ctx[i] = l->ctx[i];
        *type_word = (u32)type;
        real(object, part, cls);
        FxMod* m = &fx_mod[host][knob];
        for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
        *type_word = kept_type;
        m->obj = obj;
        m->valid = 1;
        lfofx_turned[host] = lfofx_turned[host] + 1;
    }
    return result;                                             // stock's answer for the lamp's effect: its page copy as stock makes it
}

static int lfofx_apply(int host, void* object, u32* block, int cls) {
    op1f_lfofx_calls[host] = op1f_lfofx_calls[host] + 1;       // I5
    LfoApplyFn real = (LfoApplyFn)(*(u32**)object)[2];
    u32 slot = RD32(HOST[host].built_slot) & 7u;               // the slot stock's apply reads the effect type of (0x8d503078 / 0x8d503064)
    if (*(u32*)object == MIDI_LFO_FX_VTABLE) return lfofx_apply_midi(host, object, block, cls, real, slot);   // I4
    int knob = lfo_fx_knob_of(host, slot);
    if (knob == mirror[host][slot] || !chain_ready[host] || !lfo_class_to_fx(*(u32*)object, block)) {
        lfofx_stock[host] = lfofx_stock[host] + 1;             // the wagon under the lamp, another LFO type, another destination: stock's own way, untouched
        return real(object, block, cls);
    }
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];
    Live* l = &live[host][knob];
    const Wagon* w = &wagons[host][slot][knob];
    void* obj = l->obj;
    i32 type = l->type;
    if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
        real(object, block, cls);                               // the LFO runs on (its phase); its wagon is parked or not built this block, and the lamp's effect is not its target
        for (int i = 0; i < PARAMS; ++i) ctx[i] = kept[i];
        lfofx_idle[host] = lfofx_idle[host] + 1;
        return 0;
    }
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept_type = *type_word;
    for (int i = 0; i < PARAMS; ++i) ctx[i] = l->ctx[i];
    *type_word = (u32)type;                                    // the descriptor row stock's converter holds the result to is this wagon's
    real(object, block, cls);
    FxMod* m = &fx_mod[host][knob];
    for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
    *type_word = kept_type;
    m->obj = obj;
    m->valid = 1;
    lfofx_turned[host] = lfofx_turned[host] + 1;
    return 0;                                                  // the lamp's effect did not move: no copy for its page (0x8d504854)
}

extern "C" int op1f_lfofx_apply_synth(void* object, u32* block, int cls) asm("op1f_lfofx_apply_synth");
extern "C" int op1f_lfofx_apply_synth(void* object, u32* block, int cls) { return lfofx_apply(0, object, block, cls); }
extern "C" int op1f_lfofx_apply_drum(void* object, u32* block, int cls) asm("op1f_lfofx_apply_drum");
extern "C" int op1f_lfofx_apply_drum(void* object, u32* block, int cls) { return lfofx_apply(1, object, block, cls); }
// the tables the stubs hand stock in place of the object's vtable: stock reads the word at +8 and calls it
extern "C" const LfoApplyFn op1f_lfofx_table_synth[3] asm("op1f_lfofx_table_synth");
extern "C" const LfoApplyFn op1f_lfofx_table_synth[3] = { 0, 0, op1f_lfofx_apply_synth };
extern "C" const LfoApplyFn op1f_lfofx_table_drum[3] asm("op1f_lfofx_table_drum");
extern "C" const LfoApplyFn op1f_lfofx_table_drum[3] = { 0, 0, op1f_lfofx_apply_drum };

// ---------------------------------------------------------------- I6: the fx side of the three other LFOs
//
// Four LFOs per sound (analysis/lfo_train_study.md section 18): the one under the LFO train's lamp is stock's slot
// LFO, served by the wrapper above; the three others are objects of stock's LFO classes that build_i_lfo4.cpp keeps
// and calls. Their fx-side apply comes here, from that unit, first thing in the chain's run: after stock's own LFO
// has had its call and before any wagon reads its words. Each of them has an effect wagon of its own choice, as
// stock's LFO has since I2 (0 the wagon under the effect train's lamp, 1 to 4 the wagon of a knob), and the midi
// kind one for each of its four destinations.
//
// What differs from the wrapper above. The apply is called once a block whatever the destination, as stock calls
// its own (the kinds advance their state in it; the value kind's fx-side call advances the oscillator its voices
// read in the FREE form). A wagon's words start from what stock's LFO or an earlier one of the three left them at
// this block (fx_mod), so several LFOs on one wagon add up as their offsets add up on the scratch for the lamp's
// effect. On the lamp's effect the call is stock's apply on the scratch as it stands, and its answer goes back to
// the caller, which sends stock's page its copy.
static volatile u32 lfox_turned[HOSTS], lfox_idle[HOSTS], lfox_stock[HOSTS];   // blocks, as lfofx_turned, lfofx_idle and lfofx_stock, for the three other LFOs

// The wagon a choice turns: the chosen one while it carries an effect, else the one under the lamp.
static int extra_knob_of(int host, u32 slot, int pick) {
    return (pick > 0 && pick <= WAGONS && wagons[host][slot][pick - 1].type >= 0) ? pick - 1 : mirror[host][slot];
}

// One wagon that is not the lamp's, turned: its words into the scratch and its type into the slot's type word, the
// apply, the result kept for the wagon's run of this block, the scratch and the type word put back. A wagon that is
// parked or not built gets nothing; the kinds that keep a phase still have their call (call_parked), on a scratch
// that is put back.
static void extra_turn(int host, u32 slot, int knob, void* object, u32* block, int cls, LfoApplyFn real, int call_parked) {
    volatile u32* ctx = (volatile u32*)STOCK_CTX;
    u32 kept[PARAMS];
    for (int i = 0; i < PARAMS; ++i) kept[i] = ctx[i];
    Live* l = &live[host][knob];
    const Wagon* w = &wagons[host][slot][knob];
    void* obj = l->obj;
    i32 type = l->type;
    if (obj == 0 || type != w->type || type < 0 || type >= TYPES || !w->coupled) {
        if (call_parked) {
            real(object, block, cls);
            for (int i = 0; i < PARAMS; ++i) ctx[i] = kept[i];
        }
        lfox_idle[host] = lfox_idle[host] + 1;
        return;
    }
    FxMod* m = &fx_mod[host][knob];
    const u32* from = (m->valid && m->obj == obj) ? m->words : l->ctx;   // as an LFO before this one left them at this block, else the wagon's own
    volatile u32* type_word = (volatile u32*)(HOST[host].type_array + 4u * slot);
    u32 kept_type = *type_word;
    for (int i = 0; i < PARAMS; ++i) ctx[i] = from[i];
    *type_word = (u32)type;
    real(object, block, cls);
    for (int i = 0; i < PARAMS; ++i) { m->words[i] = ctx[i]; ctx[i] = kept[i]; }
    *type_word = kept_type;
    m->obj = obj;
    m->valid = 1;
    lfox_turned[host] = lfox_turned[host] + 1;
}

// Audio thread. Non-zero when the lamp's effect was moved on the scratch (stock's answer).
extern "C" int op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) asm("op1f_lfofx_extra");
extern "C" int op1f_lfofx_extra(int host, void* object, u32* block, int pick, const u8* midi_pick) {
    LfoApplyFn real = (LfoApplyFn)(*(u32**)object)[2];
    int cls = host ? 5 : 2;
    if (!chain_ready[host]) {
        lfox_stock[host] = lfox_stock[host] + 1;
        return real(object, block, cls);
    }
    u32 slot = RD32(HOST[host].built_slot) & 7u;
    u32 vtable = *(u32*)object;
    int lamp = mirror[host][slot];
    if (vtable == MIDI_LFO_FX_VTABLE) {                        // four destinations, each with its wagon: a call for the lamp's effect, one for each other wagon (as lfofx_apply_midi)
        int target[MIDI_DESTS];
        int other = 0;
        for (int i = 0; i < MIDI_DESTS; ++i) {
            target[i] = ((i32)block[LFO_MIDI_DEST_WORD + i] >> 27) == (i32)LFO_DEST_FX ? extra_knob_of(host, slot, midi_pick[i]) : -1;
            if (target[i] >= 0 && target[i] != lamp) other = 1;
        }
        if (!other) {
            lfox_stock[host] = lfox_stock[host] + 1;
            return real(object, block, cls);
        }
        u32 part[PARAMS];
        for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
        for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] >= 0 && target[i] != lamp) part[LFO_MIDI_DEST_WORD + i] = 0;
        int result = real(object, part, cls);
        for (int knob = 0; knob < WAGONS; ++knob) {
            if (knob == lamp) continue;
            int mine = 0;
            for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] == knob) mine = 1;
            if (!mine) continue;
            for (int i = 0; i < PARAMS; ++i) part[i] = block[i];
            for (int i = 0; i < MIDI_DESTS; ++i) if (target[i] != knob) part[LFO_MIDI_DEST_WORD + i] = 0;
            extra_turn(host, slot, knob, object, part, cls, real, 0);
        }
        return result;
    }
    int knob = extra_knob_of(host, slot, pick);
    if (knob == lamp || !lfo_class_to_fx(vtable, block)) {     // the lamp's effect, another destination, a class not read through: stock's apply on the scratch as it stands
        lfox_stock[host] = lfox_stock[host] + 1;
        return real(object, block, cls);
    }
    extra_turn(host, slot, knob, object, block, cls, real, 1);
    return 0;
}

// What the LFO unit's log line says of the fx side: the three counts since the last call.
extern "C" void op1f_lfofx_extra_counts(u32* out) asm("op1f_lfofx_extra_counts");
extern "C" void op1f_lfofx_extra_counts(u32* out) {
    out[0] = lfox_turned[0]; out[1] = lfox_idle[0]; out[2] = lfox_stock[0];
    lfox_turned[0] = 0; lfox_idle[0] = 0; lfox_stock[0] = 0;
}

extern "C" void op1f_fx_chain_run(int host) asm("op1f_fx_chain_run");
extern "C" void op1f_fx_chain_run(int host) {
    if (&op1f_lfo4_block) op1f_lfo4_block(host);               // I5: the second LFO's fx-side call, before any wagon reads the scratch
    u32 slot = RD32(HOST[host].built_slot) & 7u;
    if (chain_ready[host]) {
        Wagon* ws = wagons[host][slot];
        u32 ord = order_word[host][slot];
        int mir = mirror[host][slot];
        for (int pos = 0; pos < WAGONS; ++pos) {
            int knob = (int)((ord >> (8 * pos)) & 3u);
            Wagon* w = &ws[knob];
            if (w->type < 0) continue;
            if (knob == mir) {
                if (!RD8(HOST[host].fx_active + slot)) continue;
                void* obj = (void*)RD32(FX_PRODUCER);
                if (obj) run_one(host, knob, obj, (void*)STOCK_CTX);
            } else {
                if (!w->coupled) continue;
                void* obj = live[host][knob].obj;
                if (obj) run_live(host, knob, obj, (fx_mod[host][knob].valid && fx_mod[host][knob].obj == obj) ? (void*)fx_mod[host][knob].words : (void*)live[host][knob].ctx);   // H4: the instance's fixed state on its own region; I2: the words stock's LFO turned this block, when it aims at this wagon
            }
        }
    } else {                                                   // as stock: the slot's producer while fx_active
        void* obj = (void*)RD32(FX_PRODUCER);
        if (obj && RD8(HOST[host].fx_active + slot)) run_one(host, mirror[host][slot], obj, (void*)STOCK_CTX);
    }
    for (int k = 0; k < WAGONS; ++k) fx_mod[host][k].valid = 0;   // I2: the turned words were this block's
    {                                                          // H3: the bus's largest sample, for the page (the train runs while the instrument sounds)
        const int* bus = (const int*)L_BUS;
        int peak = 0;
        for (int i = 0; i < BUS_SAMPLES; ++i) { int v = bus[i]; if (v < 0) v = -v; if (v > peak) peak = v; }
        if ((u32)peak > bus_peak[host]) bus_peak[host] = (u32)peak;
    }
    u32 total = cycles() - op1f_fx_t0[host];
    volatile Stat* c = &chain_stat[host];
    c->sum += total;
    c->count += 1;
    if (total > c->max) c->max = total;
}

// ---------------------------------------------------------------- the wagons' values

static void params_default(int type, u32* out) {
    for (int i = 0; i < PARAMS; ++i) out[i] = type >= 0 ? word_of(default_param(type, i)) : 0;
}

static void remember(int host, u32 slot, int knob, const Wagon* w) {
    if (w->type < 0) return;
    for (int i = 0; i < PARAMS; ++i) memory[host][slot][knob][w->type][i] = w->params[i];
    memory_set[host][slot][knob][w->type] = 1;
}

static void slot_write_params(int host, u32 slot, const u32* params) {
    for (int i = 0; i < PARAMS; ++i) WR32(HOST[host].param_block + 0x20u * slot + 4u * i, params[i]);
}

static void slot_read_params(int host, u32 slot, u32* params) {
    for (int i = 0; i < PARAMS; ++i) params[i] = RD32(HOST[host].param_block + 0x20u * slot + 4u * i);
}

static int slot_type(int host, u32 slot) {
    int type = (int)RD32(HOST[host].type_array + 4u * slot);
    return (type >= 0 && type < TYPES) ? type : NONE;
}

// Clears every other wagon whose type conflicts with `type` (the one-wagon-per-type rule after an adoption; nothing
// with the regions of H4, where conflicts() is 0).
static void dedupe(int host, u32 slot, int keep_knob, int type) {
    for (int k = 0; k < WAGONS; ++k) {
        if (k == keep_knob) continue;
        Wagon* w = &wagons[host][slot][k];
        if (conflicts(w->type, type)) {
            remember(host, slot, k, w);
            w->type = NONE;
            for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
            if (st.current_slot[host] == (int)slot) { st.pending_type[host][k] = NONE; st.pending_since[host][k] = (int)st.frames - REST_FRAMES; }
        }
    }
}

static void set_text(void* obj, const char* key, const char* text);       // the json helpers, below
static int doc_fx_active(void* doc);

// H6c: the record's document follows the effect the module puts on stock's slot. Stock keeps the slot's effect in
// the document as well (fx_type, fx_params, fx_active): the slots file carries it, the boot and a slot switch
// apply it, and the effect page's widgets read it, so a document left at the browser's last effect while the
// module had put another on the slot (a pick on the mirror knob, a preset load) showed that effect's parameters
// and saved them to the slots file: the H6b finding (synth_slots.json said delay with fazer's defaults while the
// slot ran nitro, then fazer). The type word, the block and the byte were right; the document is written here
// with them, and the type the chain knows the document by follows, so the next apply finds the chain standing.
// fx_type has no stock setter (its seven references are reads): the name goes in through the json string setter
// on the key's entry; the fx_params setter queues the record's write job (the changed virtual), so the slots
// file follows as after a knob turn on the page. Every write is logged (the first twenty-four).
static void mirror_document(int host, u32 slot, const Wagon* w, const char* why) {
    void* record = (void*)patch_record(host, slot);
    void* doc = (void*)(patch_record(host, slot) + 8);
    int had = doc_type[host][slot];
    set_text(doc, FX_TYPE_KEY, fx_name(w->type));
    STOCK_SET_PARAMS_DOC(record, w->params);
    STOCK_SET_ACTIVE_DOC(record, w->coupled ? 1 : 0);
    doc_type[host][slot] = w->type;
    ++st.doc_writes;
    st.doc_ms = st.ms;
    if (st.doc_lines < 24) {
        ++st.doc_lines;
        STOCK_LOG(3, LOG_FILE, 22, "chain document: %s s%u (%s): %s onto the slot and the document (the document had %s), %s, params %d %d %d %d; %u documents written in all",
                  host ? "drum" : "synth", slot, why, fx_name(w->type), fx_name(had), w->coupled ? "coupled" : "uncoupled",
                  int_of(w->params[0]), int_of(w->params[1]), int_of(w->params[2]), int_of(w->params[3]), st.doc_writes);
    }
}

// Stock's slot and the mirrored wagon agree. When the slot already runs the wagon's effect, stock's values are the
// authority (its document carries the knob turns made on the effect page, its fx_active byte the toggles), and the
// wagon takes them (H1f; H1e had written the wagon's values over them, zeros for every slot it had just met). When the
// slot runs another effect, it takes the wagon's type, parameters and coupling, and the producer is rebuilt as the
// browser does it, in the host's own mode only (the producer storage is shared by the synth and the drum; stock
// rebuilds from the type word otherwise).
static void mirror_push(int host, u32 slot) {
    int knob = mirror[host][slot];
    Wagon* w = &wagons[host][slot][knob];
    if (w->type >= 0) {
        if (slot_type(host, slot) == w->type) {
            slot_read_params(host, slot, w->params);
            w->coupled = RD8(HOST[host].fx_active + slot) ? 1 : 0;
        } else {
            WR32(HOST[host].type_array + 4u * slot, w->type);
            slot_write_params(host, slot, w->params);
            WR8(HOST[host].fx_active + slot, w->coupled ? 1 : 0);
            mirror_document(host, slot, w, "push");                                 // H6c: the document too
            if (in_own_mode(host) && gui_slot(host) == slot) STOCK_BUILD_PRODUCER(0, w->type);
        }
    } else {
        WR8(HOST[host].fx_active + slot, 0);
        if (doc_fx_active((void*)(patch_record(host, slot) + 8))) STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), 0);   // H6c: an empty mirror wagon is an inactive effect in the document as well
    }
    stock_type_seen[host][slot] = slot_type(host, slot);
}

static void mark_dirty(int host) {
    st.dirty[host] = 1;
    st.dirty_since[host] = (int)st.frames;
    if (!ls.keep_loaded) ls.loaded_slot[host] = -1;            // H5: the chain changed, the lamp goes out
}

static u32 chain_cost(int host, u32 slot, int except_knob, int with_type) {
    u32 sum = 0;
    for (int k = 0; k < WAGONS; ++k) {
        const Wagon* w = &wagons[host][slot][k];
        int type = (k == except_knob) ? with_type : w->type;
        int coupled = (k == except_knob) ? 1 : w->coupled;
        if (type >= 0 && coupled) sum += cost_of(type);
    }
    return sum;
}

static int type_in_use(int host, u32 slot, int type, int except_knob) {
    for (int k = 0; k < WAGONS; ++k)
        if (k != except_knob && conflicts(wagons[host][slot][k].type, type)) return 1;
    return 0;
}

static int order_type(int i) { return i < KNOB_ORDER_COUNT ? KNOB_ORDER[i] : TYPES + (i - KNOB_ORDER_COUNT); }   // the offered wagons after stock's list

static int order_index_of(int type) {
    for (int i = 0; i < KNOB_ORDER_COUNT + PLUGS; ++i) if (order_type(i) == type) return i;
    return 0;
}

// A turn of the wagon's knob: the next or previous effect in the browser's order, skipping the picks over the ceiling
// (and, without the regions of H4, the types another wagon carries); the label changes at once, the instance after
// the knob's rest.
static void pick(int host, int knob, int delta) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    int index = order_index_of(w->type);
    int step = delta > 0 ? 1 : -1;
    for (int tries = 0; tries < KNOB_ORDER_COUNT + PLUGS; ++tries) {
        index += step;
        if (index < 0 || index >= KNOB_ORDER_COUNT + PLUGS) return;         // the ends of the list
        int type = order_type(index);
        if (type >= TYPES && (!buildable(type) || knob == mirror[host][slot])) continue;   // an offered wagon that is there, never under the lamp
        if (type >= 0 && type_in_use(host, slot, type, knob)) continue;
        if (type >= 0 && chain_cost(host, slot, knob, type) > CEILING_CYCLES) {
            ++st.refusals;
            st.flash_until[host] = (int)st.frames + FLASH_FRAMES;
            st.refused_knob[host] = knob;                                        // the page sags this wagon and names the pick in red (H3)
            st.refused_type[host] = type;
            return;
        }
        remember(host, slot, knob, w);
        w->type = type;
        if (type >= 0) {
            if (memory_set[host][slot][knob][type]) for (int i = 0; i < PARAMS; ++i) w->params[i] = memory[host][slot][knob][type][i];
            else params_default(type, w->params);
        } else {
            for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
        }
        st.pending_type[host][knob] = type;
        st.pending_since[host][knob] = (int)st.frames;
        if (knob == mirror[host][slot]) st.push_pending[host] = 1;
        ++st.picks;
        mark_dirty(host);
        return;
    }
}

static void move_wagon(int host, int knob, int delta) {
    u32 slot = gui_slot(host);
    int knobs[WAGONS];
    int pos = 0;
    for (int p = 0; p < WAGONS; ++p) {
        knobs[p] = knob_at(host, slot, p);
        if (knobs[p] == knob) pos = p;
    }
    int to = pos + (delta > 0 ? 1 : -1);
    if (to < 0 || to >= WAGONS) return;
    knobs[pos] = knobs[to];
    knobs[to] = knob;
    set_order(host, slot, knobs);
    ++st.moves;
    mark_dirty(host);
}

static void set_coupled(int host, int knob, int coupled) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    w->coupled = coupled ? 1 : 0;
    if (knob == mirror[host][slot]) {
        WR8(HOST[host].fx_active + slot, w->coupled);
        STOCK_SET_ACTIVE_DOC((void*)patch_record(host, slot), w->coupled);
    }
    mark_dirty(host);
}

// H3's whole-chain bypass under shift + T3 (toggle_bypass) is gone since H5: the key opens the presets list, and a
// chain whose file remembers parked wagons (the tag-92 mask) gets them back on the line at the load.

static void apply_params(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (knob == mirror[host][slot]) {
        slot_write_params(host, slot, w->params);
        STOCK_SET_PARAMS_DOC((void*)patch_record(host, slot), w->params);   // stock's own key follows its slot
    } else {
        for (int i = 0; i < PARAMS; ++i) live[host][knob].ctx[i] = w->params[i];
    }
    mark_dirty(host);
}

// Shift + tap: the four knob parameters from stock's own random-preset value function 0x8d51233c(type, index, bias),
// the one its "random preset" job (0x8d4e9724, at 0x8d4e9076) calls for each effect parameter with the descriptor table
// 0x840afe60 behind it, so every value is one stock would write itself (H1b drew 8 to 85 percent of a fixed 0..32767
// scale, which is outside the delay's own ranges: the delay crashed on it); the other four stay at the defaults.
static void randomize(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (w->type < 0 || w->type >= TYPES) return;                            // stock's random values are for stock's effects
    for (int i = 0; i < PARAMS; ++i) {
        int value = i < 4 ? (int)(short)STOCK_RANDOM_PARAM(w->type, i, 0) : DEFAULTS[w->type][i];
        w->params[i] = word_of(value);
    }
    ++st.randoms;
    apply_params(host, knob);
}

static void reset_params(int host, int knob) {
    u32 slot = gui_slot(host);
    Wagon* w = &wagons[host][slot][knob];
    if (w->type < 0) return;
    params_default(w->type, w->params);
    ++st.resets;
    apply_params(host, knob);
}

// The tap: the wagon becomes the mirror and stock shows its page (the browser for an empty wagon).
static void open_wagon_page(void* self, int host, int knob) {
    u32 slot = gui_slot(host);
    if (const Op1fWagon* plug = wagon_of(wagons[host][slot][knob].type)) {   // an offered wagon: its own page, the lamp stays
        if (plug->page) {
            st.train_open[host] = 0;
            ++st.page_opens;
            plug->page(wagons[host][slot][knob].params);
        }
        return;
    }
    int old = mirror[host][slot];
    if (old != knob) {
        mirror[host][slot] = (u8)knob;
        live[host][knob].obj = 0;                                            // the mirror runs through stock's producer
        live[host][knob].type = NONE;
        st.pending_type[host][knob] = NONE;
        st.push_pending[host] = 0;
        mirror_push(host, slot);
        st.pending_type[host][old] = wagons[host][slot][old].type;          // the former mirror needs a module instance now
        st.pending_since[host][old] = (int)st.frames - REST_FRAMES;
        mark_dirty(host);
    }
    st.train_open[host] = 0;
    ++st.page_opens;
    u32* vtable = *(u32**)self;
    if (wagons[host][slot][knob].type < 0) {
        ((BrowserFn)vtable[0x3c / 4])(self, (int)slot);                     // the effect browser, as shift + T3
        return;
    }
    if (*page_of(self) == PAGE_FX) *page_of(self) = 0;                       // T3 on page 2 would toggle; from another page it shows the effect
    int event[4] = { KEY_T3, 0, 0, 0 };
    ((KeyEventFn)HOST[host].key_down)(self, event);
}

// ---------------------------------------------------------------- the handlers (vtable words)

// H5: the presets list's handlers (defined after the persistence, which they use)
static void t3_press(int host);
static void t3_release(int host);
static void list_release(int host, int knob);
static void list_turn(int host, int knob, int delta);
static int list_key_down(void* self, int host, const int* event, int shifted);
static void list_frame(int host);

// A knob's physical press reaches the controller as the key 61 + knob at key-down (through the shifted key-down
// while shift is held), its release as the same key at key-up; then, when the knob was not turned while pressed,
// the key handler 0x8c0b1468 synthesizes a click, the key 54 + knob at key-down (shifted by shift at the release)
// and at key-up. On the train the press starts the hold timer (frame_host fires it), the release ends it, and the
// click is the tap unless a hold fired just before it.
static void knob_press(int host, int knob, int shifted) {
    int gap = (int)st.ms - st.last_turn_ms[host][knob];
    st.pressed[host][knob] = 1;
    st.press_ms[host][knob] = (int)st.ms;
    st.press_shift[host][knob] = shifted;
    st.acted[host][knob] = 0;
    st.hold_armed[host][knob] = gap >= CLEAN_MS;                             // H3b: a press right after a turn arms no hold (the click after it is still a tap); H4b: 120 ms
    if (!st.hold_armed[host][knob]) {
        ++st.turn_presses;
        STOCK_LOG(3, LOG_FILE, 7, "chain press: knob %d turned %d ms before the press, no hold armed", knob, gap);
    }
}

static void knob_release(int host, int knob) {
    if (st.pressed[host][knob] && st.acted[host][knob]) st.swallow_until[host][knob] = (int)st.frames + SWALLOW_FRAMES;
    st.pressed[host][knob] = 0;
}

static int knob_click(void* self, int host, int knob, int shifted) {
    if ((int)st.frames < st.swallow_until[host][knob]) {                   // the click after a hold
        st.swallow_until[host][knob] = 0;
        ++st.swallowed;
        return 1;
    }
    if (shifted) randomize(host, knob);
    else { ++st.taps; open_wagon_page(self, host, knob); }
    return 1;
}

extern "C" int op1f_fx_key_down(void* self, const int* event) asm("op1f_fx_key_down");
extern "C" int op1f_fx_key_down(void* self, const int* event) {
    int host = host_of(self);
    int key = event[0];
    if (host == 0 && &op1f_lfotrain_key_down && !(st.train_open[0] && ls.open[0])) {   // I6: the LFO train on T4 and its keys while it is open; the presets list keeps every key
        int taken = op1f_lfotrain_key_down(self, event, 0);
        if (taken >= 0) return taken;
    }
    if (!st.train_open[host]) {
        if (key == KEY_T3 && (&op1f_train_available == 0 || op1f_train_available())) {   // no page to show: T3 runs stock
            st.train_open[host] = 1;
            ++st.train_opens;
            return 1;
        }
        return ((KeyEventFn)HOST[host].key_down)(self, event);
    }
    if (ls.open[host]) return list_key_down(self, host, event, 0);         // H5: the list over the train takes every key
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { knob_press(host, key - KEY_PRESS0, 0); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) return knob_click(self, host, key - KEY_CLICK0, 0);
    if (key == KEY_T3) {
        open_wagon_page(self, host, mirror[host][gui_slot(host)]);
        return 1;
    }
    if (key >= KEY_T1 && key < KEY_T1 + 4) st.train_open[host] = 0;         // another page: the train closes
    return ((KeyEventFn)HOST[host].key_down)(self, event);
}

extern "C" int op1f_fx_key_shift(void* self, const int* event) asm("op1f_fx_key_shift");
extern "C" int op1f_fx_key_shift(void* self, const int* event) {
    int host = host_of(self);
    int key = event[0];
    if (host == 0 && &op1f_lfotrain_key_down && !(st.train_open[0] && ls.open[0])) {   // I6
        int taken = op1f_lfotrain_key_down(self, event, 1);
        if (taken >= 0) return taken;
    }
    if (!st.train_open[host]) return ((KeyEventFn)HOST[host].key_shift)(self, event);
    if (ls.open[host]) return list_key_down(self, host, event, 1);         // H5
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { knob_press(host, key - KEY_PRESS0, 1); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) return knob_click(self, host, key - KEY_CLICK0, 1);
    if (key == KEY_T3) { t3_press(host); return 1; }                          // H5: a tap opens the presets list at the release, a hold saves the chain as a preset (H3's bypass is gone)
    return ((KeyEventFn)HOST[host].key_shift)(self, event);
}

extern "C" int op1f_fx_key_up(void* self, int key) asm("op1f_fx_key_up");
extern "C" int op1f_fx_key_up(void* self, int key) {
    int host = host_of(self);
    if (host == 0 && &op1f_lfotrain_key_up) op1f_lfotrain_key_up(key);       // I6: the LFO train sees its knobs released; stock's bookkeeping runs below as ever
    if (st.train_open[host] && key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) {
        if (ls.open[host]) list_release(host, key - KEY_PRESS0);              // H5
        else knob_release(host, key - KEY_PRESS0);
    }
    if (key == KEY_T3 && ls.t3_pressed[host]) t3_release(host);             // H5: the tap of shift + T3 (the hold has acted already)
    return ((KeyUpFn)HOST[host].key_up)(self, key);                          // stock's release bookkeeping either way
}

// H6h: a knob turn stock handles on the effect page or the LFO page, with the descriptor entry the converter uses for it
// (the type word's row, the knob's column; the shift layer is the same column with the knob's value divided by 16) and
// the slot's parameter words before and after, for the read-out: the static tables read as (0, 0, step), and the
// device shows whether the clamp fields are filled at run time or the converter reads otherwise (the LFO study, 5.5).
// H6i: H6h printed one line a turn and twenty-four in all, which the first page's knobs alone would have used up (every
// detent is a turn). Each knob now has an entry of its own (the host, the page, the type, the index with its shift
// bit): the slot's whole eight-word block is read before and after stock's handler, the first two turns and the first
// turns that changed the block print a line each (eight per effect knob, sixteen per LFO knob, whose destination steps
// are worth following one by one), and a knob left alone for a second and a half prints one line with what it reached:
// the lowest and the highest value of its own word (the clamps, once the knob was turned to both ends), the smallest
// and the largest step, the deltas, the words it changed. A type's descriptor row (eight entries of three half-words)
// is printed once, when its first knob is turned. The budgets are counted against the checklist that spends them
// (tools/tests/test_fx_chain_presets.py): twenty-four effect knobs, sixteen LFO knobs.
// H6j: the probe did its work (analysis/build_h6i_device_result.md: the rows, the law of a turn, the LFO's words) and is
// switched off: left on, it would write up to some 860 lines a boot on the two pages the owner plays most.
// KNOB_PROBE = 1 brings it back as it ran in H6i.
enum { KNOB_PROBE = 0 };
enum { PROBES = 64, PROBE_IDLE_MS = 1500, BLOCK_WORDS = 8, FX_TURN_LINES = 8, LFO_TURN_LINES = 16, FX_TURN_LINES_ALL = 240, LFO_TURN_LINES_ALL = 320, RANGE_LINES_ALL = 300 };
struct KnobProbe {
    u32 key;                                                   // 0 free; else 1 | host << 1 | lfo << 2 | type << 4 | index << 8
    u32 desc;                                                  // the descriptor entry the converter reads for this knob
    u32 slot;                                                  // the sound slot of the last turn
    u32 turns, moved;                                          // the turns passed to stock; those that changed a word of the block
    i32 delta_lo, delta_hi;                                    // the encoder deltas seen
    i32 lo, hi;                                                // the knob's own word, the lowest and the highest seen, before or after a turn
    u32 step_lo, step_hi;                                      // the smallest and the largest change of the own word in one turn
    u32 changed;                                               // bit n: a turn changed word n of the block
    u32 lines;                                                 // the per-turn lines printed for this knob
    u32 dirty_ms;                                              // the time of the last turn
    u8 dirty;                                                  // turned since its last range line
};
static KnobProbe probes[PROBES];

static const char* probe_name(int lfo, u32 type) { return lfo ? (type < 6u ? *LFO_NAMES[type] : "?") : fx_name((int)type); }

static KnobProbe* probe_for(int host, int lfo, u32 type, int index) {
    u32 key = 1u | ((u32)host << 1) | ((u32)lfo << 2) | (type << 4) | ((u32)(index & 7) << 8);
    for (int i = 0; i < PROBES; ++i) {
        if (probes[i].key == key) return &probes[i];
        if (probes[i].key == 0) { probes[i].key = key; return &probes[i]; }   // the entries fill in order; a fresh one is all zeros
    }
    if (!st.probes_full) {
        st.probes_full = 1;
        STOCK_LOG(2, LOG_FILE, 27, "knob probe: the table of %d knobs is full: no line for a further knob", (int)PROBES);
    }
    return 0;
}

static void print_row(int lfo, u32 type) {
    if (st.row_mask[lfo] & (1u << type)) return;
    st.row_mask[lfo] |= 1u << type;
    u32 row = (lfo ? LFO_DESCRIPTORS : FX_DESCRIPTORS) + 48u * type;
    for (u32 half = 0; half < 2u; ++half) {
        u32 e = row + 24u * half;
        STOCK_LOG(3, LOG_FILE, 25, "knob row: %s %s (type %u) at %08x, entries %u to %u, three half-words each: %u %u %u, %u %u %u, %u %u %u, %u %u %u",
                  lfo ? "lfo" : "fx", probe_name(lfo, type), type, row, half * 4u, half * 4u + 3u,
                  (u32)RD16(e), (u32)RD16(e + 2u), (u32)RD16(e + 4u), (u32)RD16(e + 6u), (u32)RD16(e + 8u), (u32)RD16(e + 10u),
                  (u32)RD16(e + 12u), (u32)RD16(e + 14u), (u32)RD16(e + 16u), (u32)RD16(e + 18u), (u32)RD16(e + 20u), (u32)RD16(e + 22u));
    }
}

static int stock_knob_turn(void* self, int host, int index, int delta) {
    int page = *page_of(self);
    int lfo = page == PAGE_LFO;
    if (!KNOB_PROBE || (page != PAGE_FX && !lfo)) return ((EncoderFn)HOST[host].encoder)(self, index, delta);
    u32 slot = gui_slot(host);
    int own = index & 3;                                                     // the knob's own word of the block (the shifted knob adjusts the same word)
    u32 type_word = RD32((lfo ? LFO_TYPE_ARRAY[host] : HOST[host].type_array) + 4u * slot);
    u32 block = (lfo ? LFO_BLOCK[host] : HOST[host].param_block) + 0x20u * slot;
    u32 before[BLOCK_WORDS], after[BLOCK_WORDS];
    for (int w = 0; w < BLOCK_WORDS; ++w) before[w] = RD32(block + 4u * (u32)w);
    int result = ((EncoderFn)HOST[host].encoder)(self, index, delta);
    u32 mask = 0;
    for (int w = 0; w < BLOCK_WORDS; ++w) {
        after[w] = RD32(block + 4u * (u32)w);
        if (after[w] != before[w]) mask |= 1u << w;
    }
    ++st.knob_turns;
    if (type_word >= 16u) return result;                                     // no descriptor row for such a word
    KnobProbe* p = probe_for(host, lfo, type_word, index);
    if (p == 0) return result;
    u32 desc = (lfo ? LFO_DESCRIPTORS : FX_DESCRIPTORS) + 48u * type_word + 6u * (u32)own;
    i32 b = (i32)before[own], a = (i32)after[own];
    if (p->turns == 0) { p->lo = b; p->hi = b; p->delta_lo = delta; p->delta_hi = delta; p->step_lo = 0xffffffffu; p->step_hi = 0; }
    ++p->turns;
    if (delta < p->delta_lo) p->delta_lo = delta;
    if (delta > p->delta_hi) p->delta_hi = delta;
    if (b < p->lo) p->lo = b;
    if (a < p->lo) p->lo = a;
    if (b > p->hi) p->hi = b;
    if (a > p->hi) p->hi = a;
    if (a != b) {
        u32 step = a > b ? (u32)a - (u32)b : (u32)b - (u32)a;
        if (step < p->step_lo) p->step_lo = step;
        if (step > p->step_hi) p->step_hi = step;
    }
    if (mask) ++p->moved;
    p->changed |= mask;
    p->slot = slot;
    p->desc = desc;
    p->dirty = 1;
    p->dirty_ms = st.ms;
    print_row(lfo, type_word);
    u32 per_knob = lfo ? (u32)LFO_TURN_LINES : (u32)FX_TURN_LINES, all = lfo ? (u32)LFO_TURN_LINES_ALL : (u32)FX_TURN_LINES_ALL;
    if ((mask || p->turns <= 2u) && p->lines < per_knob && st.turn_lines[lfo] < all) {   // the first two turns whatever they did, then the turns that changed the block
        ++p->lines;
        ++st.turn_lines[lfo];
        int shown = own;                                                     // the knob's own word, or the lowest word that changed when the own one did not
        if (mask && !(mask & (1u << own))) for (shown = 0; !(mask & (1u << shown)); ++shown) {}
        STOCK_LOG(3, LOG_FILE, 24, "knob turn: %s s%u %s %s knob %d (index %d, delta %d): word %d %08x -> %08x (%d -> %d), changed mask %u; descriptor %08x (%u %u %u); turn %u",
                  host ? "drum" : "synth", slot, lfo ? "lfo" : "fx", probe_name(lfo, type_word), own, index, delta, shown, before[shown], after[shown],
                  int_of(before[shown]), int_of(after[shown]), mask, desc, (u32)RD16(desc), (u32)RD16(desc + 2u), (u32)RD16(desc + 4u), p->turns);
    }
    return result;
}

// H6j: every type's descriptor row printed once, some ten seconds after the first frame, a type a frame (two lines):
// the eleven effects, then the six LFOs, so one read-out holds the whole of both tables as the boot's initialiser
// left them (H6i printed the rows of the types whose knobs were turned: the delay, the terminal, the value LFO).
enum { ROWS_AT_FRAME = 600, LFO_TYPES = 6, ROW_TYPES = TYPES + LFO_TYPES };
static void rows_frame() {
    if (st.rows_next >= (u32)ROW_TYPES || st.frames < (u32)ROWS_AT_FRAME) return;
    u32 n = st.rows_next++;
    if (n < (u32)TYPES) print_row(0, n); else print_row(1, n - (u32)TYPES);
}

// H6i: a knob that rested for a second and a half says what it reached, two lines a frame at most.
static void probes_frame() {
    if (!KNOB_PROBE) return;
    int printed = 0;
    for (int i = 0; i < PROBES && printed < 2; ++i) {
        KnobProbe* p = &probes[i];
        if (p->key == 0) break;                                              // the entries fill in order
        if (!p->dirty || st.ms - p->dirty_ms < (u32)PROBE_IDLE_MS) continue;
        p->dirty = 0;
        if (st.range_lines >= (u32)RANGE_LINES_ALL) continue;
        ++st.range_lines;
        ++printed;
        int host = (int)((p->key >> 1) & 1u), lfo = (int)((p->key >> 2) & 1u), index = (int)((p->key >> 8) & 7u);
        u32 type = (p->key >> 4) & 15u;
        STOCK_LOG(3, LOG_FILE, 26, "knob range: %s s%u %s %s knob %d (index %d): %u turns, %u moved, delta %d..%d; word %08x..%08x (%d..%d), step %08x..%08x; changed mask %u; descriptor %08x (%u %u %u)",
                  host ? "drum" : "synth", p->slot, lfo ? "lfo" : "fx", probe_name(lfo, type), index & 3, index, p->turns, p->moved, p->delta_lo, p->delta_hi,
                  (u32)p->lo, (u32)p->hi, int_of((u32)p->lo), int_of((u32)p->hi), p->step_hi ? p->step_lo : 0u, p->step_hi, p->changed,
                  p->desc, (u32)RD16(p->desc), (u32)RD16(p->desc + 2u), (u32)RD16(p->desc + 4u));
    }
}

// I2: the value LFO's destination knob (index 2, no shift; stock's handler 0x8c0cec20 moves the block's word 2 by 352
// a detent between 1024 and 11264). Below the effect's range the knob is stock's. Inside it one detent is one effect
// wagon: the wagons that carry an effect, in knob order, first in the form of its own phase, then in the form shared
// by all voices (the page's FREE); the word is parked in the middle of the form's range meanwhile and changes only
// when the form changes or the knob leaves the range downwards. With no effect on any wagon the knob is stock's.
//
// Stock's handler is still called for every turn the module takes, with a turn of nothing (a delta of 0 moves no
// value: the converter adds (speed x delta) >> 15): it writes the slot's LFO block into the record's document
// (0x8d4f3c84, which queues the record's write job) and does what stock does after any turn on this page (the
// controller's encoder handler 0x8c0d5c10 / 0x8c0b7634: its hint when the LFO is off, its own bookkeeping).
static u32 lfo_type_word(int host, u32 slot) { return RD32(LFO_TYPE_ARRAY[host] + 4u * slot); }
static u32* lfo_block_of(int host, u32 slot) { return (u32*)(LFO_BLOCK[host] + 0x20u * slot); }

static int next_fx_knob(int host, u32 slot, int from, int dir) {         // the next wagon with an effect after `from` in the direction, or -1
    for (int k = from + dir; k >= 0 && k < WAGONS; k += dir) if (wagons[host][slot][k].type >= 0) return k;
    return -1;
}

static void lfofx_log_at(int host, u32 slot, const char* what, int value, int knob, int dest) {   // dest: 0 the LFO's one destination; 1 to 4 the midi LFO's (I4)
    if (lfofx_lines >= (u32)LFOFX_LINES) return;
    ++lfofx_lines;
    STOCK_LOG(3, LOG_FILE, 31, "lfo fx: %s s%u %s: the destination word %d of lfo type %u (destination %d), the effect wagon of knob %d (%s)%s",
              host ? "drum" : "synth", slot, what, value, lfo_type_word(host, slot), dest, knob, fx_name(wagons[host][slot][knob].type), knob == mirror[host][slot] ? ", the one under the lamp" : "");
}
static void lfofx_log(int host, u32 slot, const char* what, int value) { lfofx_log_at(host, slot, what, value, lfo_fx_knob_of(host, slot), 0); }

// I3: the random, element and velocity LFOs, whose destination word has four classes and no shared form
static int lfo_type_four(u32 type) { return type == (u32)LFO_TYPE_RANDOM || type == (u32)LFO_TYPE_ELEMENT || type == (u32)LFO_TYPE_VELOCITY; }

// Their destination knob: outside the effect's range stock's own step; a step that lands inside starts at the near
// end of the wagons (the first from below, the last from above) with the word parked in the range's middle; inside,
// a detent is a wagon, and a detent past the last or before the first leaves the range for the middle of the class
// next door, the choice kept.
// I4: the word and the choice are the caller's: the block's word 2 and the slot's one choice for the three LFOs, the
// word 4 + n and the destination's own choice for the midi LFO's destination n (dest n + 1 on the log's line).
static int lfo_dest_turn_four(void* self, int host, u32 slot, int index, int delta, int* result, int first, int last, int word_index, u8* pick, int dest) {
    EncoderFn stock = (EncoderFn)HOST[host].encoder;
    u32* word = &lfo_block_of(host, slot)[word_index];
    int value = int_of(*word);
    if (value < LFO4_FX_LO || value >= LFO4_FX_HI) {
        *result = stock(self, index, delta);
        int landed = int_of(*word);
        if (landed >= LFO4_FX_LO && landed < LFO4_FX_HI) {
            *pick = (u8)((value < LFO4_FX_LO ? first : last) + 1);
            *word = word_of(LFO4_FX_PARK);
            *result = stock(self, index, 0);
            mark_dirty(host);
            lfofx_log_at(host, slot, value < LFO4_FX_LO ? "the knob entered the effect's range from below" : "the knob entered the effect's range from above", LFO4_FX_PARK, (int)*pick - 1, dest);
        }
        return 1;
    }
    int dir = delta > 0 ? 1 : -1;
    int at = (*pick && wagons[host][slot][*pick - 1].type >= 0) ? *pick - 1 : mirror[host][slot];   // the wagon it turns now (lfo_fx_knob_of's rule)
    int next = next_fx_knob(host, slot, at, dir);
    const char* what;
    int now = value;
    if (next >= 0) {                                                         // the next wagon, the word where it is
        *pick = (u8)(next + 1);
        at = next;
        what = dir > 0 ? "a detent up" : "a detent down";
    } else if (dir > 0) {                                                    // past the last wagon: out upwards
        now = LFO4_OVER_FX;
        what = "up out of the effect's range";
    } else {                                                                 // before the first: out downwards
        now = LFO4_UNDER_FX;
        what = "down out of the effect's range";
    }
    if (now != value) *word = word_of(now);
    *result = stock(self, index, 0);
    mark_dirty(host);
    lfofx_log_at(host, slot, what, now, at, dest);
    return 1;
}

static int lfo_dest_turn(void* self, int host, int index, int delta, int* result) {   // 1: the module took the turn (*result is stock's answer)
    if (*page_of(self) != PAGE_LFO || delta == 0) return 0;
    u32 slot = gui_slot(host);
    u32 kind = lfo_type_word(host, slot);
    if (!initialised[host][slot]) return 0;
    int midi = kind == (u32)LFO_TYPE_MIDI && index >= LFO_MIDI_DEST_KNOB && index < LFO_MIDI_DEST_KNOB + MIDI_DESTS;   // I4: a shifted knob on the midi LFO's page, one of its four destinations
    if (!midi && (index != LFO_DEST_KNOB || (kind != (u32)LFO_TYPE_VALUE && !lfo_type_four(kind)))) return 0;
    int first = next_fx_knob(host, slot, -1, 1), last = next_fx_knob(host, slot, WAGONS, -1);
    if (first < 0 || first == last) return 0;                                // I3: fewer than two wagons with an effect, nothing to choose: stock's knob
    if (midi) return lfo_dest_turn_four(self, host, slot, index, delta, result, first, last, index, &lfo_midi_pick[host][slot][index - LFO_MIDI_DEST_KNOB], index - LFO_MIDI_DEST_KNOB + 1);
    if (kind != (u32)LFO_TYPE_VALUE) return lfo_dest_turn_four(self, host, slot, index, delta, result, first, last, LFO_DEST_WORD, &lfo_fx_pick[host][slot], 0);
    EncoderFn stock = (EncoderFn)HOST[host].encoder;
    u32* word = &lfo_block_of(host, slot)[LFO_DEST_WORD];
    int value = int_of(*word);
    if (value < LFO_FX_LO) {
        *result = stock(self, index, delta);                                // stock's own step; one that lands in the effect's range starts at the first wagon
        if (int_of(*word) >= LFO_FX_LO) {
            lfo_fx_pick[host][slot] = (u8)(first + 1);
            *word = word_of(LFO_FX_PARK);
            *result = stock(self, index, 0);
            mark_dirty(host);
            lfofx_log(host, slot, "the knob entered the effect's range", LFO_FX_PARK);
        }
        return 1;
    }
    int dir = delta > 0 ? 1 : -1;
    int shared = value >= LFO_FX_SHARED;
    int next = next_fx_knob(host, slot, lfo_fx_knob_of(host, slot), dir);
    const char* what = 0;
    int now = value;
    if (next >= 0) {                                                         // the next wagon, the word where it is
        lfo_fx_pick[host][slot] = (u8)(next + 1);
        what = dir > 0 ? "a detent up" : "a detent down";
    } else if (dir > 0 && !shared) {                                         // past the last wagon: the shared form, at the first
        lfo_fx_pick[host][slot] = (u8)(first + 1);
        now = LFO_FX_PARK_SHARED;
        what = "up into the shared form";
    } else if (dir < 0 && shared) {                                          // before the first wagon: the form of its own phase, at the last
        lfo_fx_pick[host][slot] = (u8)(last + 1);
        now = LFO_FX_PARK;
        what = "down into the form of its own phase";
    } else if (dir < 0) {                                                    // below the first wagon: out of the effect's range, the choice kept
        now = LFO_UNDER_FX;
        what = "down out of the effect's range";
    }                                                                        // else the last wagon in the shared form: the end of the knob's travel
    if (now != value) *word = word_of(now);
    *result = stock(self, index, 0);
    if (what) {
        mark_dirty(host);
        lfofx_log(host, slot, what, now);
    }
    return 1;
}

// what the LFO pages are told (build_i_lfopage.cpp)
extern "C" void op1f_lfofx_view(int host, Op1fLfoFxView* out) asm("op1f_lfofx_view");
extern "C" void op1f_lfofx_view(int host, Op1fLfoFxView* out) {
    u32 slot = gui_slot(host);
    int knob = lfo_fx_knob_of(host, slot);
    u32 kind = lfo_type_word(host, slot);
    int value = int_of(lfo_block_of(host, slot)[LFO_DEST_WORD]);
    out->known = initialised[host][slot];
    out->lfo_type = (int)kind;
    out->steps = kind == (u32)LFO_TYPE_VALUE || lfo_type_four(kind);
    out->to_fx = kind == (u32)LFO_TYPE_VALUE ? value >= LFO_FX_LO : (lfo_type_four(kind) && value >= LFO4_FX_LO && value < LFO4_FX_HI);
    out->knob = knob;
    out->type = wagons[host][slot][knob].type;
    out->mirror = mirror[host][slot];
}

// I4: what the midi LFO's page is told: for each of its four destinations, whether it is the effect and which wagon it turns
extern "C" void op1f_lfofx_midi_view(int host, Op1fLfoMidiView* out) asm("op1f_lfofx_midi_view");
extern "C" void op1f_lfofx_midi_view(int host, Op1fLfoMidiView* out) {
    u32 slot = gui_slot(host);
    out->known = initialised[host][slot];
    out->midi = lfo_type_word(host, slot) == (u32)LFO_TYPE_MIDI;
    for (int i = 0; i < MIDI_DESTS; ++i) {
        int value = int_of(lfo_block_of(host, slot)[LFO_MIDI_DEST_WORD + i]);
        int knob = lfo_midi_knob_of(host, slot, i);
        out->to_fx[i] = value >= LFO4_FX_LO && value < LFO4_FX_HI;
        out->knob[i] = knob;
        out->type[i] = wagons[host][slot][knob].type;
    }
}

extern "C" int op1f_fx_encoder(void* self, int index, int delta) asm("op1f_fx_encoder");
extern "C" int op1f_fx_encoder(void* self, int index, int delta) {
    int host = host_of(self);
    if (host == 0 && &op1f_lfotrain_encoder) {                               // I6: a knob turned on the LFO train picks the kind for its wagon
        int taken = op1f_lfotrain_encoder(self, index, delta);
        if (taken >= 0) return taken;
    }
    if (!st.train_open[host]) {
        int result = 0;
        if (lfo_dest_turn(self, host, index, delta, &result)) return result;  // I2, I3: the LFO's destination knob, a wagon a detent inside the effect's range
        return stock_knob_turn(self, host, index, delta);                     // H6h: logged on the effect and LFO pages (off since H6j)
    }
    if (delta == 0) return 1;
    if (ls.open[host]) { list_turn(host, index & 3, delta); return 1; }      // H5: the list's scroll, caret and letter
    int knob = index & 3;
    if (st.pressed[host][knob]) st.acted[host][knob] = 1;                   // a turn while pressed: no hold (stock sends no click either)
    st.last_turn_ms[host][knob] = (int)st.ms;                               // H3b: a press in the next 120 ms arms no hold (H4b)
    if (index >= 0 && index < WAGONS) pick(host, index, delta);
    else if (index >= 4 && index < 4 + WAGONS) move_wagon(host, index - 4, delta);
    st.turn_knob[host] = knob;                                              // the page's card (H2)
    st.turn_frame[host] = (int)st.frames;
    st.turn_move[host] = index >= 4;                                        // the page's hop (H3)
    return 1;
}

// ---------------------------------------------------------------- the persistence: the module's own file

// user/op1f.json: an array of the tag 91 and the sixteen slots' chains (synth 0 to 7, then drum 0 to 7), each the 47
// integers of the chain (the tag, the mirror's knob, the document's effect type the chain was last reconciled with,
// the order as four knobs by position, then per knob the type, the coupling and the eight parameters as stock's
// integers, word >>> 16) or an empty array for a slot the module has not seen. Written by stock's json file writer
// as the slots files are, read by its reader at the first reconcile (the boot's first apply, 1.8 s in, on the main
// thread); a missing or damaged file leaves every slot to its document. A value's kind is its byte at +4: 0 null (what
// 0x8c085ba8(json, 0) makes; the first push turns it into an array), 1 an integer as 0x8c083b1c makes it, 2 unsigned,
// 3 float, 6 an array, 7 an object. An array is checked for by its kind before the size read (the size getter's
// table walks the list for 6 and returns the kind byte for the rest); the integer getter 0x8c084840 needs no check:
// it converts the kinds 1, 2 and 3 (a parsed number is a 2 or a 3, so H1e's check against a fresh integer's kind lost
// every number read) and returns 0 with a log line for anything else. H1f had taken the array's kind from a fresh
// value, which is the null 0, so no parsed array passed and the file loaded nothing.

static int is_array(void* j) { return j != 0 && *((u8*)j + 4) == JSON_ARRAY_KIND; }
static void name_trim(char* name);                                           // H5, below
static char lower_of(char c);                                                // H5b, below
static void fx_scan(int force);                                              // H6, below
static int free_slot();
static int fx_write_slot(int slot);
static void rebuild_order();
static void default_name(char* out, int slot);
static int int_at(void* array, int i) { void* e = STOCK_JSON_AT(array, i); return e ? STOCK_JSON_INT(e) : 0; }

static void chain_values(int host, u32 slot, int* values) {                // CHAIN_INTS_V2 integers
    int n = 0;
    values[n++] = CHAIN_TAG_V2;
    values[n++] = mirror[host][slot];
    values[n++] = doc_type[host][slot];
    for (int p = 0; p < WAGONS; ++p) values[n++] = knob_at(host, slot, p);
    for (int k = 0; k < WAGONS; ++k) {
        const Wagon* w = &wagons[host][slot][k];
        values[n++] = type_stored(w->type);                                   // an offered wagon by its name
        values[n++] = w->coupled;
        for (int i = 0; i < PARAMS; ++i) values[n++] = int_of(w->params[i]);
    }
    values[n++] = bypass_mask[host][slot];                                     // H3: the 48th integer
}

// H6j: the wagon H6 to H6i made of a file's "none" (json_fx_type): an uncoupled delay with eight zero parameters. No knob
// reaches that state (the delay's first parameter starts at 1024, its second at 3276), so it is read as empty, here
// for the chains the unit saved while it ran those images and in the file reader for the presets saved from them.
static int ghost_delay(int type, int coupled, const int* params) {
    if (type != TYPE_DELAY || coupled) return 0;
    for (int i = 0; i < PARAMS; ++i) if (params[i]) return 0;
    return 1;
}

// H6j: a stored knob parameter held to the knob's own range, the type's descriptor row as it stands in memory: the
// minimum a signed half-word, the maximum the next (the H6i read-out: the rows are filled at boot by a static
// initialiser, and a knob turn clamps to them; a delay time above 11264 crashed the delay in H1b). The four knob
// parameters only: the other four words are not a knob's. An entry not filled (the image has 0, 0 there) holds nothing.
static int clamp_param(int host, u32 slot, int knob, int type, int i, int value) {
    if (i >= 4 || type >= TYPES) return value;
    u32 e = FX_DESCRIPTORS + 48u * (u32)type + 6u * (u32)i;
    int lo = (int)(short)RD16(e), hi = (int)(short)RD16(e + 2u);
    if (hi <= lo || (value >= lo && value <= hi)) return value;
    int held = value < lo ? lo : hi;
    ++st.clamps;
    if (st.value_lines < 24) {
        ++st.value_lines;
        STOCK_LOG(3, LOG_FILE, 28, "chain values: %s s%u knob %d (%s) parameter %d: %d held to %d (the knob's range %d..%d)", host ? "drum" : "synth", slot, knob, fx_name(type), i, value, held, lo, hi);
    }
    return held;
}

static void chain_from_values(int host, u32 slot, const int* values) {       // CHAIN_INTS_V2 integers (a tag-91 chain padded with 0)
    int n = 1;
    mirror[host][slot] = (u8)(values[n++] & 3);
    doc_type[host][slot] = values[n++];
    int knobs[WAGONS];
    for (int p = 0; p < WAGONS; ++p) knobs[p] = values[n++] & 3;
    for (int k = 0; k < WAGONS; ++k) {
        Wagon* w = &wagons[host][slot][k];
        int type = values[n++];
        int coupled = values[n++] ? 1 : 0;
        const int* stored = values + n;
        n += PARAMS;
        type = type_loaded(type);
        if (ghost_delay(type, coupled, stored)) {                           // H6j
            type = NONE;
            coupled = 1;
            ++st.ghosts;
            if (st.value_lines < 24) {
                ++st.value_lines;
                STOCK_LOG(3, LOG_FILE, 29, "chain values: %s s%u knob %d: an uncoupled delay with eight zero parameters (a file's none as H6 to H6i read it) taken as empty", host ? "drum" : "synth", slot, k);
            }
        }
        w->type = type;
        w->coupled = (u8)coupled;
        for (int i = 0; i < PARAMS; ++i) w->params[i] = type >= 0 ? word_of(clamp_param(host, slot, k, type, i, stored[i])) : 0;
    }
    int seen = 0;
    for (int p = 0; p < WAGONS; ++p) seen |= 1 << knobs[p];
    if (seen != 0xf) for (int p = 0; p < WAGONS; ++p) knobs[p] = p;
    set_order(host, slot, knobs);
    u32 parked = (u32)values[CHAIN_INTS] & 0xfu;                            // H3's bypass parked these: back on the line (H5 has no bypass key)
    if (parked) for (int k = 0; k < WAGONS; ++k) if ((parked & (1u << k)) && wagons[host][slot][k].type >= 0) wagons[host][slot][k].coupled = 1;
    bypass_mask[host][slot] = 0;
    for (int p = 0; p < WAGONS && wagon_of(wagons[host][slot][mirror[host][slot]].type); ++p)   // the lamp off an offered wagon, onto the first that can carry it
        if (!wagon_of(wagons[host][slot][knob_at(host, slot, p)].type)) mirror[host][slot] = (u8)knob_at(host, slot, p);
    if (wagon_of(wagons[host][slot][mirror[host][slot]].type)) wagons[host][slot][mirror[host][slot]].type = NONE;   // four of them: the lamp's is emptied
}

static void load_file() {
    file_loaded = 1;
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(FILE_PATH, &root);
    int tag = int_at(&root, 0);
    int shape = read && is_array(&root) && STOCK_JSON_SIZE(&root) >= 1 + HOSTS * SLOTS && (tag == CHAIN_TAG || tag == ROOT_TAG_V3);   // H5: the root tag 93 adds the presets
    if (shape) {
        for (int h = 0; h < HOSTS; ++h) {
            for (u32 sl = 0; sl < SLOTS; ++sl) {
                void* e = STOCK_JSON_AT(&root, 1 + h * SLOTS + (int)sl);
                if (!is_array(e)) continue;
                int tag = int_at(e, 0);
                int count = tag == CHAIN_TAG_V2 ? CHAIN_INTS_V2 : (tag == CHAIN_TAG ? CHAIN_INTS : 0);   // the H3 chain, or an H1e to H2 one
                if (count == 0 || STOCK_JSON_SIZE(e) < count) continue;
                int values[CHAIN_INTS_V2];
                for (int i = 0; i < CHAIN_INTS_V2; ++i) values[i] = i < count ? int_at(e, i) : 0;
                chain_from_values(h, sl, values);
                if (count == CHAIN_INTS_V2 && STOCK_JSON_SIZE(e) >= CHAIN_INTS_V2 + 2 && int_at(e, CHAIN_INTS_V2) == LFOFX_TAG) {   // I2: the LFO's effect wagon
                    int pick = int_at(e, CHAIN_INTS_V2 + 1);
                    lfo_fx_pick[h][sl] = (u8)(pick >= 0 && pick <= WAGONS ? pick : 0);
                    if (STOCK_JSON_SIZE(e) >= CHAIN_INTS_V2 + 4 && int_at(e, CHAIN_INTS_V2 + 2) == LFOMIDI_TAG) {   // I4: the midi LFO's four, four bits each
                        int packed = int_at(e, CHAIN_INTS_V2 + 3);
                        for (int i = 0; i < MIDI_DESTS; ++i) {
                            int one = (packed >> (4 * i)) & 15;
                            lfo_midi_pick[h][sl][i] = (u8)(one <= WAGONS ? one : 0);
                        }
                        if (h == 0 && STOCK_JSON_SIZE(e) >= LFO4_AT + 1 + LFO4_MIN && int_at(e, LFO4_AT) == LFO4_TAG) {   // I6: the three other LFOs and the lamp's place
                            int more[LFO4_INTS];
                            int have = STOCK_JSON_SIZE(e) - (LFO4_AT + 1);
                            for (int i = 0; i < LFO4_INTS; ++i) more[i] = i < have ? int_at(e, LFO4_AT + 1 + i) : -1;   // I7: what a file of I6's does not have reads as -1
                            if (&op1f_lfo4_store_load) op1f_lfo4_store_load((int)sl, more);
                            else { lfo4_keep(sl, more); ++st.lfo4_carried; }           // J5f: the LFO unit is away: carried to the next save
                        }
                    }
                }
                known[h][sl] = 1;
                ++st.loads;
            }
        }
    }
    fx_scan(1);                                                              // H6: the presets' folder and files (made when missing)
    if (&op1f_lfopresets_scan) op1f_lfopresets_scan();                       // I8: the LFO presets' folder and files, at the same moment
    int migrated = 0, failed = 0;
    if (shape && tag == ROOT_TAG_V3 && STOCK_JSON_SIZE(&root) >= 2 + HOSTS * SLOTS) {    // the H5 presets of the seventeenth element, moved into the first free slots
        void* pa = STOCK_JSON_AT(&root, 1 + HOSTS * SLOTS);
        if (is_array(pa)) {
            int n = STOCK_JSON_SIZE(pa);
            for (int j = 0; j < n; ++j) {
                void* e = STOCK_JSON_AT(pa, j);
                if (!is_array(e) || STOCK_JSON_SIZE(e) < PRESET_INTS) continue;
                int s = free_slot();
                if (s < 0) break;
                for (int c = 0; c < NAME_CHARS; ++c) {
                    int code = int_at(e, c);
                    presets.name[s][c] = (code >= 32 && code < 127) ? lower_of((char)code) : ' ';
                }
                presets.name[s][NAME_CHARS] = 0;
                name_trim(presets.name[s]);
                if (!presets.name[s][0]) default_name(presets.name[s], s);
                for (int v = 0; v < CHAIN_INTS_V2; ++v) presets.values[s][v] = int_at(e, NAME_CHARS + v);
                presets.used[s] = 1;
                if (fx_write_slot(s)) ++migrated;                                   // H6b: counted only when the file landed
                else { presets.used[s] = 0; ++failed; }
            }
            if (migrated) { rebuild_order(); ls.migrated += (u32)migrated; }
            if (migrated && !failed) { st.dirty[0] = 1; st.dirty_since[0] = (int)st.frames; }   // the element is written empty at the next save, once every preset is a file (H6b: H6 dropped it while the writes failed)
            else if (failed) keep_h5_presets = 1;                                 // the element is written back as it was
        }
    }
    if (keep_h5_presets && shape && tag == ROOT_TAG_V3 && STOCK_JSON_SIZE(&root) >= 2 + HOSTS * SLOTS) {   // H6b: the element kept whole while a file could not be written
        STOCK_JSON_CTOR(&h5_presets, 0);
        STOCK_JSON_ASSIGN(&h5_presets, STOCK_JSON_AT(&root, 1 + HOSTS * SLOTS));
    }
    STOCK_LOG(3, LOG_FILE, 8, "chain file: %s, kind %u, %s, %u chains, tag %d, %d presets migrated into the files, %d not (kept in the file), %d slots used", read ? "read" : "not read", *((u8*)&root + 4), shape ? "shape ok" : "no shape", st.loads, tag, migrated, failed, presets.count);
    if (st.lfo4_carried) STOCK_LOG(2, LOG_FILE, 34, "chain file: the LFO pack is away: the LFOs of %u sound(s) are carried as the file has them and written back untouched", st.lfo4_carried);
    STOCK_JSON_DTOR(&root);
}

static void save_file() {
    Json root, chain, item;
    STOCK_JSON_CTOR(&root, 0);
    STOCK_JSON_FROM_INT(&item, ROOT_TAG_V3);                                  // H5: the root carries the presets after the chains
    STOCK_JSON_PUSH(&root, &item);
    STOCK_JSON_DTOR(&item);
    for (int h = 0; h < HOSTS; ++h) {
        for (u32 sl = 0; sl < SLOTS; ++sl) {
            STOCK_JSON_CTOR(&chain, 0);
            if (known[h][sl]) {
                int values[CHAIN_INTS_V2];
                chain_values(h, sl, values);
                int tail[4] = { (int)LFOFX_TAG, (int)lfo_fx_pick[h][sl], (int)LFOMIDI_TAG, 0 };
                for (int i = 0; i < MIDI_DESTS; ++i) tail[3] |= (int)lfo_midi_pick[h][sl][i] << (4 * i);
                int more[LFO4_INTS];
                int have = h != 0 ? 0 : (&op1f_lfo4_store_save ? op1f_lfo4_store_save((int)sl, more) : lfo4_carried(sl, more));   // I6: nothing for a sound that has only stock's LFO, on the first wagon; J5f: what was read, while the LFO unit is away
                int extra = (tail[3] || have) ? 4 : 2;                        // I4: the midi LFO's four choices ride after I2's pair only when one is made: a sound that never chose keeps I2's fifty integers; I6: both pairs stand before the tag 98
                for (int i = 0; i < CHAIN_INTS_V2 + extra; ++i) {              // I2: after the 48 integers the tag 96 and the LFO's effect wagon (0 the lamp's, 1 to 4 a knob's); an older image reads the first 48
                    STOCK_JSON_FROM_INT(&item, i < CHAIN_INTS_V2 ? values[i] : tail[i - CHAIN_INTS_V2]);
                    STOCK_JSON_PUSH(&chain, &item);
                    STOCK_JSON_DTOR(&item);
                }
                for (int i = 0; have && i < 1 + LFO4_INTS; ++i) {              // I6: the tag 98 and the LFO unit's integers; an older image reads what stands before them
                    STOCK_JSON_FROM_INT(&item, i == 0 ? (int)LFO4_TAG : more[i - 1]);
                    STOCK_JSON_PUSH(&chain, &item);
                    STOCK_JSON_DTOR(&item);
                }
            }
            STOCK_JSON_PUSH(&root, &chain);
            STOCK_JSON_DTOR(&chain);
        }
    }
    Json list;                                                               // H6: the seventeenth element stays, empty: the presets live in user/fx/ (an H5 reader finds no preset here); H6b: the H5 presets written back while their files could not be made
    STOCK_JSON_CTOR(&list, 0);
    if (keep_h5_presets) STOCK_JSON_ASSIGN(&list, &h5_presets);
    STOCK_JSON_PUSH(&root, &list);
    STOCK_JSON_DTOR(&list);
    u32 path[12];
    for (int i = 0; i < 12; ++i) path[i] = 0;
    STOCK_STRING_CTOR(path, FILE_PATH);
    STOCK_JSON_WRITE_FILE(path, &root);
    STOCK_JSON_DTOR(&root);
    ++st.saves;
}

static void save_pending() {
    if (!st.dirty[0] && !st.dirty[1]) return;
    st.dirty[0] = st.dirty[1] = 0;
    save_file();
}

// ---------------------------------------------------------------- the presets (H5, section 21; the files H6, section 22)

// The store is thirty-two slots, one file each in the module's folder user/fx/ (01.json .. 32.json), shared by both
// hosts (a chain fits either). A used slot's file is a JSON object: op1f_fx_preset (the format, 1), name (the font's
// lowercase, twelve characters at most), order (the knob at each position), wagons (four objects: fx, stock's name
// of the effect or "none", on, params, the eight integers as stock keeps them); an empty slot's file is {}. The
// files are written by stock's json file writer and read by its reader; the folder is scanned at the first file
// load and at every list open (a file whose time or size changed since the last read is read again, a missing
// file is made), so a file the host wrote through the disk mode (build_h_disk.cpp shows the folder as fx/) is a
// preset at the next list open. The list is a screen of the GUI's own (build_h_presets.cpp, the id 37 since H6d) dressed
// from op1f_presets_view; the keys below route to it while it is open.
//   * shift + T3 on the train: a tap (the release before the save) opens the list, a hold saves the current chain
//     into the lowest free slot as chain NN (the slot's number), the save screen showing the number from 200 ms,
//     the save at 700 ms, saved! for a second, then the list on the new preset; on the list the same: a tap closes
//     it, a hold saves.
//   * On the list: the blue knob scrolls, its tap (or T3) loads the highlighted preset into the current slot (the
//     types, the order, the couplings and the parameters; the mirror stays on its knob and takes its new effect,
//     the other instances rebuild) and closes the list; the ochre knob's tap opens the rename editor; the grey
//     knob's tap saves (as the hold of shift + T3); the red knob held HOLD_MS empties the highlighted preset's slot
//     after a deleted flash of a second (its file becomes {}); T1, T2 and T4 close the list and the train and run
//     stock.
//   * The editor: the edited preset is scrolled to the window's first row and its name shows in twelve cells; the
//     blue knob moves the caret, the ochre knob steps the letter (space, a to z, 0 to 9, dash, dot), the grey knob's
//     tap or T3 keeps the name (all spaces: chain NN), the red knob's tap cancels.
//   * The lamp marks the slot the chain was loaded from or saved as until the chain changes (any mark_dirty of the
//     chain clears it; the presets' own writes keep it).
// Every save, load, rename, delete, scan and migration logs a line (presets:); the counts go on the presets: line.

static int alphabet_index(char c) {
    for (int i = 0; i < ALPHABET_COUNT; ++i) if (ALPHABET[i] == c) return i;
    return 0;
}

static void name_copy(char* dst, const char* src) {                          // at most NAME_CHARS characters, NUL-terminated
    int i = 0;
    for (; i < NAME_CHARS && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

static void name_trim(char* name) {                                          // the trailing spaces off
    int n = 0;
    while (name[n]) ++n;
    while (n > 0 && name[n - 1] == ' ') name[--n] = 0;
}

static char lower_of(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }   // H5b: the font's letters are the lowercase codes

static void default_name(char* out, int slot) {                             // chain NN, the slot's number (the file's)
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    char* p = put_text(out, "chain ");
    *p++ = (char)('0' + tens);
    *p++ = (char)('0' + n);
    *p = 0;
}

static void fx_path(int slot, char* out) {                                  // user/fx/NN.json
    int n = slot + 1, tens = 0;
    while (n >= 10) { n -= 10; ++tens; }
    char* p = put_text(out, FX_PATH_PREFIX);
    *p++ = (char)('0' + tens);
    *p++ = (char)('0' + n);
    p = put_text(p, ".json");
    *p = 0;
}

static int free_slot() {
    for (int s = 0; s < PRESETS; ++s) if (!presets.used[s]) return s;
    return -1;
}

static void rebuild_order() {                                               // the used slots in slot order
    int n = 0;
    for (int s = 0; s < PRESETS; ++s) if (presets.used[s]) presets.order[n++] = s;
    presets.count = n;
}

static int row_of_slot(int slot) {                                          // the list row of a slot, or -1
    for (int i = 0; i < presets.count; ++i) if (presets.order[i] == slot) return i;
    return -1;
}

// a json string's characters: a stock string object has its data inline at +4 up to fifteen characters, else a
// pointer there; the length at +0x14 (0x8c03e434)
static void json_text(void* entry, char* out, int max) {
    u32 str[8];
    for (int i = 0; i < 8; ++i) str[i] = 0;
    STOCK_JSON_TO_STRING(str, entry);
    u32 len = str[5], cap = str[6];
    const char* data = cap >= 16 ? (const char*)str[1] : (const char*)&str[1];
    int n = 0;
    if (data) for (; n < max && (u32)n < len && data[n]; ++n) out[n] = data[n];
    out[n] = 0;
    if (cap >= 16 && str[1]) STOCK_FREE((void*)str[1]);                      // a long text lives on the heap
}

static int text_equal(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

// "delay" .. "fazer" by stock's own names; "none" and anything else none. H6 to H6i asked stock's lookup 0x8fca7b08,
// which answers a name it does not know with type 0, the delay, and an error line (`unknown fx name: 'none'`): every
// empty wagon of a file came back as an uncoupled delay with zero parameters (the H6i read-out: the fifteen library
// files' twenty-one empty wagons). The name is compared here, and stock's lookup is left to stock's own documents.
static int json_fx_type(void* entry) {
    if (!entry || *((u8*)entry + 4) != JSON_STRING_KIND) return NONE;
    char name[16];
    json_text(entry, name, 15);
    for (int type = 0; type < ALL_TYPES; ++type) if ((type < TYPES || wagon_of(type)) && text_equal(name, fx_name(type))) return type;
    return NONE;
}

static int json_truth(void* entry) {                                        // true/false, or an integer
    if (!entry) return 0;
    if (*((u8*)entry + 4) == JSON_BOOL_KIND) return STOCK_JSON_BOOL(entry) ? 1 : 0;
    return STOCK_JSON_INT(entry) != 0;
}

static void set_int(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_FROM_INT(&item, value);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_text(void* obj, const char* key, const char* text) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_CSTR(&item, text);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_bool(void* obj, const char* key, int value) {
    Json item;
    STOCK_JSON_CTOR(&item, 0);
    STOCK_JSON_FROM_BOOL(&item, value ? 1 : 0);
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &item);
    STOCK_JSON_DTOR(&item);
}

static void set_ints(void* obj, const char* key, const int* values, int count) {
    Json arr, item;
    STOCK_JSON_CTOR(&arr, 0);
    for (int i = 0; i < count; ++i) {
        STOCK_JSON_FROM_INT(&item, values[i]);
        STOCK_JSON_PUSH(&arr, &item);
        STOCK_JSON_DTOR(&item);
    }
    STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(obj, key), &arr);
    STOCK_JSON_DTOR(&arr);
}

// The slot's file: {} for an empty slot, else the object above. The time and size are noted so the next scan does
// not read the module's own write back. 1 when the file landed (H6b: the writer's result and the file's size checked).
static int fx_write_slot(int slot) {
    Json root;
    STOCK_JSON_CTOR(&root, JSON_OBJECT_KIND);
    if (presets.used[slot]) {
        const int* v = presets.values[slot];
        set_int(&root, KEY_FORMAT, PRESET_FORMAT);
        set_text(&root, KEY_NAME, presets.name[slot]);
        int order[WAGONS];
        for (int p = 0; p < WAGONS; ++p) order[p] = v[3 + p] & 3;
        set_ints(&root, KEY_ORDER, order, WAGONS);
        Json wagons, w;
        STOCK_JSON_CTOR(&wagons, 0);
        for (int k = 0; k < WAGONS; ++k) {
            const int* wv = v + 3 + WAGONS + k * (2 + PARAMS);
            STOCK_JSON_CTOR(&w, JSON_OBJECT_KIND);
            set_text(&w, KEY_FX, fx_name(type_loaded(wv[0])));
            set_bool(&w, KEY_ON, wv[1]);
            set_ints(&w, KEY_PARAMS, wv + 2, PARAMS);
            STOCK_JSON_PUSH(&wagons, &w);
            STOCK_JSON_DTOR(&w);
        }
        STOCK_JSON_ASSIGN(STOCK_JSON_LOOKUP(&root, KEY_WAGONS), &wagons);
        STOCK_JSON_DTOR(&wagons);
    }
    char path[16];
    fx_path(slot, path);
    u32 str[12];
    for (int i = 0; i < 12; ++i) str[i] = 0;
    STOCK_STRING_CTOR(str, path);
    int written = STOCK_JSON_WRITE_FILE(str, &root);                             // the writer writes tmp/NN.json and renames it over the slot's file; 0 when that fails (H6b)
    STOCK_JSON_DTOR(&root);
    presets.mtime[slot] = STOCK_FILE_MTIME(path);
    presets.size[slot] = STOCK_FILE_SIZE(path);
    ++ls.writes;
    if (!written || presets.size[slot] < 0) {
        ++ls.write_failures;
        if (ls.write_failures <= 4) STOCK_LOG(1, LOG_FILE, 20, "presets: the write of %s failed (writer %d, size %d): the folder is missing or the disk is full", path, written, presets.size[slot]);
        return 0;
    }
    return 1;
}

// The slot's file read: a preset when it carries four wagons, else an empty slot (a host's file with an effect
// name stock does not know gets an empty wagon there; a missing name gets chain NN).
static void fx_read_slot(int slot) {
    char path[16];
    fx_path(slot, path);
    Json root;
    STOCK_JSON_CTOR(&root, 0);
    int read = STOCK_JSON_READ_FILE(path, &root);
    presets.used[slot] = 0;
    if (read && *((u8*)&root + 4) == JSON_OBJECT_KIND && STOCK_JSON_HAS(&root, KEY_WAGONS)) {
        void* wagons = STOCK_JSON_GET(&root, KEY_WAGONS);
        if (is_array(wagons) && STOCK_JSON_SIZE(wagons) >= WAGONS) {
            int* v = presets.values[slot];
            for (int i = 0; i < CHAIN_INTS_V2; ++i) v[i] = 0;
            v[0] = CHAIN_TAG_V2;
            for (int p = 0; p < WAGONS; ++p) v[3 + p] = p;
            if (STOCK_JSON_HAS(&root, KEY_ORDER)) {
                void* o = STOCK_JSON_GET(&root, KEY_ORDER);
                if (is_array(o) && STOCK_JSON_SIZE(o) >= WAGONS) for (int p = 0; p < WAGONS; ++p) v[3 + p] = int_at(o, p) & 3;
            }
            int seen = 0;
            for (int p = 0; p < WAGONS; ++p) seen |= 1 << v[3 + p];
            if (seen != 0xf) for (int p = 0; p < WAGONS; ++p) v[3 + p] = p;
            for (int k = 0; k < WAGONS; ++k) {
                int* wv = v + 3 + WAGONS + k * (2 + PARAMS);
                void* w = STOCK_JSON_AT(wagons, k);
                int type = NONE, on = 1;
                if (w && *((u8*)w + 4) == JSON_OBJECT_KIND) {
                    if (STOCK_JSON_HAS(w, KEY_FX)) type = json_fx_type(STOCK_JSON_GET(w, KEY_FX));
                    if (STOCK_JSON_HAS(w, KEY_ON)) on = json_truth(STOCK_JSON_GET(w, KEY_ON));
                }
                wv[0] = type_stored(type);
                wv[1] = type >= 0 ? (on ? 1 : 0) : 1;                          // H6j: an empty wagon is coupled, as a fresh chain's: a later pick sounds at once
                if (type >= 0) {
                    for (int i = 0; i < PARAMS; ++i) wv[2 + i] = default_param(type, i);
                    if (STOCK_JSON_HAS(w, KEY_PARAMS)) {
                        void* params = STOCK_JSON_GET(w, KEY_PARAMS);
                        if (is_array(params)) {
                            int n = STOCK_JSON_SIZE(params);
                            for (int i = 0; i < PARAMS && i < n; ++i) wv[2 + i] = int_at(params, i);
                        }
                    }
                }
                if (ghost_delay(wv[0], wv[1], wv + 2)) { wv[0] = NONE; wv[1] = 1; }   // H6j: a file saved from a chain that carried H6's ghost
            }
            char name[NAME_CHARS + 1];
            name[0] = 0;
            if (STOCK_JSON_HAS(&root, KEY_NAME)) {
                void* e = STOCK_JSON_GET(&root, KEY_NAME);
                if (e && *((u8*)e + 4) == JSON_STRING_KIND) json_text(e, name, NAME_CHARS);
            }
            for (int c = 0; name[c]; ++c) name[c] = (name[c] >= 32 && name[c] < 127) ? lower_of(name[c]) : ' ';
            name_trim(name);
            if (!name[0]) default_name(name, slot);
            name_copy(presets.name[slot], name);
            presets.used[slot] = 1;
        }
    }
    STOCK_JSON_DTOR(&root);
}

// The folder's thirty-two files: the folder made when missing, a missing file made as {}, a file whose time or
// size changed since the last read (or every file when forced) read again.
static void fx_scan(int force) {
    if (!presets.scanned) {
        int made = STOCK_MAKE_FOLDERS(FX_FOLDER);                              // user/ and user/fx/ when missing
        if (made) ++ls.folder_made;
        STOCK_LOG(3, LOG_FILE, 21, "presets: folder %s %s", FX_FOLDER, made ? "made or present" : "NOT made");
    }
    int reread = 0, made = 0;
    for (int s = 0; s < PRESETS; ++s) {
        char path[16];
        fx_path(s, path);
        int size = STOCK_FILE_SIZE(path);
        if (size < 0) {                                                        // no file: the empty slot's placeholder
            presets.used[s] = 0;
            fx_write_slot(s);
            ++made;
            continue;
        }
        u32 mtime = STOCK_FILE_MTIME(path);
        if (force || !presets.scanned || mtime != presets.mtime[s] || size != presets.size[s]) {
            fx_read_slot(s);
            presets.mtime[s] = mtime;
            presets.size[s] = size;
            ++reread;
        }
    }
    presets.scanned = 1;
    ++ls.scans;
    ls.rereads += (u32)reread;
    rebuild_order();
    STOCK_LOG(3, LOG_FILE, 19, "presets: scan %u of %s: %d files read, %d made, %d slots used, %u writes failed in all", ls.scans, FX_FOLDER, reread, made, presets.count, ls.write_failures);
}

static void flash(int kind, int slot) {
    ls.flash_kind = kind;
    ls.flash_slot = slot;
    ls.flash_until_ms = (int)st.ms + FLASH_MS;
}

static void list_clamp(int host) {                                          // the cursor inside the list, the window around the cursor
    int n = presets.count;
    if (ls.cursor[host] >= n) ls.cursor[host] = n - 1;
    if (ls.cursor[host] < 0) ls.cursor[host] = 0;
    if (ls.editing[host]) { ls.first[host] = ls.cursor[host]; return; }   // the edited preset on the first row
    int max_first = n > LIST_ROWS ? n - LIST_ROWS : 0;
    if (ls.first[host] > max_first) ls.first[host] = max_first;
    if (ls.first[host] < 0) ls.first[host] = 0;
    if (ls.cursor[host] < ls.first[host]) ls.first[host] = ls.cursor[host];
    if (ls.cursor[host] >= ls.first[host] + LIST_ROWS) ls.first[host] = ls.cursor[host] - LIST_ROWS + 1;
}

static void list_open(int host) {
    if (&op1f_presets_available == 0 || !op1f_presets_available()) return;  // no list screen: shift + T3 does nothing
    fx_scan(0);                                                              // H6: a file the host changed through the disk mode is a preset now
    ls.open[host] = 1;
    ls.editing[host] = 0;
    int row = row_of_slot(ls.loaded_slot[host]);
    if (row >= 0) ls.cursor[host] = row;
    list_clamp(host);
    for (int k = 0; k < WAGONS; ++k) { ls.pressed[host][k] = 0; ls.acted[host][k] = 0; ls.swallow_until[host][k] = 0; }
    ++ls.opens;
}

static void list_close(int host) {
    ls.open[host] = 0;
    ls.editing[host] = 0;
}

// The mirrored wagon onto stock's slot with the wagon authoritative (a preset load): mirror_push reads the slot back
// into the wagon when the types agree, which is right for the knobs on the effect page and wrong here.
static void mirror_write(int host, u32 slot) {
    int knob = mirror[host][slot];
    Wagon* w = &wagons[host][slot][knob];
    void* record = (void*)patch_record(host, slot);
    if (w->type >= 0) {
        if (slot_type(host, slot) != w->type) {
            WR32(HOST[host].type_array + 4u * slot, w->type);
            slot_write_params(host, slot, w->params);
            WR8(HOST[host].fx_active + slot, w->coupled ? 1 : 0);
            mirror_document(host, slot, w, "load");                                 // H6c: the document too (H6b left it at the browser's last effect)
            if (in_own_mode(host) && gui_slot(host) == slot) STOCK_BUILD_PRODUCER(0, w->type);
        } else {
            slot_write_params(host, slot, w->params);
            STOCK_SET_PARAMS_DOC(record, w->params);
            WR8(HOST[host].fx_active + slot, w->coupled ? 1 : 0);
            STOCK_SET_ACTIVE_DOC(record, w->coupled);
        }
    } else {
        WR8(HOST[host].fx_active + slot, 0);
    }
    stock_type_seen[host][slot] = slot_type(host, slot);
}

static void preset_save(int host) {
    u32 slot = gui_slot(host);
    if (!initialised[host][slot]) return;
    int s = free_slot();
    if (s < 0) { flash(FLASH_FULL, presets.count ? presets.order[ls.cursor[host]] : -1); return; }
    default_name(presets.name[s], s);
    chain_values(host, slot, presets.values[s]);
    presets.used[s] = 1;
    fx_write_slot(s);
    rebuild_order();
    ls.cursor[host] = row_of_slot(s);
    ls.editing[host] = 0;
    list_clamp(host);
    ls.keep_loaded = 1;
    ls.loaded_slot[host] = s;
    ls.keep_loaded = 0;
    flash(FLASH_SAVED, s);
    ls.save_state = SAVE_SAVED;                                                // H5b: "saved!" with the check for a second (the save screen), then the list
    ls.save_host = host;
    ls.save_slot = s;
    ls.save_ms = (int)st.ms;
    ++ls.saves;
    STOCK_LOG(3, LOG_FILE, 14, "presets: saved slot %d as %s from %s s%u (%s, %s, %s, %s)", s + 1, presets.name[s], host ? "drum" : "synth", slot,
              fx_name(wagons[host][slot][0].type), fx_name(wagons[host][slot][1].type), fx_name(wagons[host][slot][2].type), fx_name(wagons[host][slot][3].type));
}

static void preset_load(int host, int row) {
    u32 slot = gui_slot(host);
    if (row < 0 || row >= presets.count || !initialised[host][slot]) return;
    int s = presets.order[row];
    int values[CHAIN_INTS_V2];
    for (int n = 0; n < CHAIN_INTS_V2; ++n) values[n] = presets.values[s][n];
    values[1] = mirror[host][slot];                                            // the mirror stays on its knob
    values[2] = doc_type[host][slot];                                          // the document has not changed: the next reconcile finds the chain standing
    chain_from_values(host, slot, values);
    int moved_from = -1;
    if (wagons[host][slot][mirror[host][slot]].type < 0) {                        // H6g: a file whose wagons leave the mirrored knob empty (a two- or three-effect chain made on the PC fills the
        for (int p = 0; p < WAGONS; ++p) {                                        // blue, ochre and grey knobs): the mirror moves to the first effect in signal order, so stock's page shows an
            int k = knob_at(host, slot, p);                                        // effect of the chain, on, instead of the previous effect switched off (the owner's first listen)
            if (wagons[host][slot][k].type >= 0 && !wagon_of(wagons[host][slot][k].type)) { moved_from = mirror[host][slot]; mirror[host][slot] = (u8)k; ++ls.mirror_moves; break; }
        }
    }
    st.push_pending[host] = 0;
    mirror_write(host, slot);                                                  // stock's slot takes the mirrored wagon's effect, parameters and coupling
    st.current_slot[host] = -1;                                                // the live instances follow at the next frame (as after a reconcile)
    ls.keep_loaded = 1;
    mark_dirty(host);                                                          // the chain's own file
    ls.keep_loaded = 0;
    ls.loaded_slot[host] = s;
    ++ls.loads;
    u32 cost = chain_cost(host, slot, -1, NONE);                                  // H6f: a file the host wrote is not held to the ceiling the picks are; said in the log
    STOCK_LOG(3, LOG_FILE, 15, "presets: loaded slot %d (%s) onto %s s%u: %s, %s, %s, %s; %u of %u cycles a block%s; mirror knob %d%s", s + 1, presets.name[s], host ? "drum" : "synth", slot,
              fx_name(wagons[host][slot][0].type), fx_name(wagons[host][slot][1].type), fx_name(wagons[host][slot][2].type), fx_name(wagons[host][slot][3].type),
              cost, (u32)CEILING_CYCLES, cost > (u32)CEILING_CYCLES ? " (OVER THE BUDGET: the block may overrun)" : "",
              mirror[host][slot], moved_from >= 0 ? " (moved from an empty wagon)" : "");
}

static void preset_delete_now(int s) {
    if (s < 0 || s >= PRESETS || !presets.used[s]) return;
    STOCK_LOG(3, LOG_FILE, 16, "presets: deleted slot %d (%s)", s + 1, presets.name[s]);
    presets.used[s] = 0;
    fx_write_slot(s);                                                          // {}
    rebuild_order();
    for (int h = 0; h < HOSTS; ++h) {
        if (ls.loaded_slot[h] == s) ls.loaded_slot[h] = -1;
        list_clamp(h);
    }
    ++ls.deletes;
}

static void delete_begin(int host) {
    if (!presets.count || ls.pending_delete_slot >= 0 || ls.editing[host]) return;
    int s = presets.order[ls.cursor[host]];
    ls.pending_delete_slot = s;
    flash(FLASH_DELETED, s);                                                   // the row reads deleted for a second, then the list closes up
}

static void rename_begin(int host) {
    if (!presets.count) return;
    ls.editing[host] = 1;
    ls.caret[host] = 0;
    const char* name = presets.name[presets.order[ls.cursor[host]]];
    int c = 0;
    for (; c < NAME_CHARS && name[c]; ++c) ls.buffer[host][c] = name[c];
    for (; c < NAME_CHARS; ++c) ls.buffer[host][c] = ' ';
    ls.buffer[host][NAME_CHARS] = 0;
    list_clamp(host);
}

static void rename_keep(int host) {
    if (!ls.editing[host]) return;
    ls.editing[host] = 0;
    if (ls.cursor[host] < 0 || ls.cursor[host] >= presets.count) { list_clamp(host); return; }
    int s = presets.order[ls.cursor[host]];
    char name[NAME_CHARS + 1];
    name_copy(name, ls.buffer[host]);
    name_trim(name);
    if (name[0] == 0) default_name(name, s);                                   // all spaces: the slot's name
    name_copy(presets.name[s], name);
    fx_write_slot(s);
    list_clamp(host);
    ++ls.renames;
    STOCK_LOG(3, LOG_FILE, 17, "presets: renamed slot %d to %s", s + 1, presets.name[s]);
}

static void rename_cancel(int host) {
    ls.editing[host] = 0;
    list_clamp(host);
}

static void t3_press(int host) {
    ls.t3_pressed[host] = 1;
    ls.t3_ms[host] = (int)st.ms;
    ls.t3_acted[host] = 0;
}

static void t3_release(int host) {
    ls.t3_pressed[host] = 0;
    if (!ls.t3_acted[host]) {                                                  // the tap (any release before the save): the list opens or closes
        if (ls.save_state == SAVE_SAVING && ls.save_host == host) ls.save_state = SAVE_NONE;   // H5b: "saving" was showing: cancelled
        if (ls.open[host]) list_close(host); else list_open(host);
    }
    STOCK_REQUEST_SCREEN();                                                    // the GUI asks the screen-id virtual now (a release alone may not)
}

static void list_press(int host, int knob) {
    ls.pressed[host][knob] = 1;
    ls.press_ms[host][knob] = (int)st.ms;
    ls.acted[host][knob] = 0;
}

static void list_release(int host, int knob) {
    if (ls.pressed[host][knob] && ls.acted[host][knob]) ls.swallow_until[host][knob] = (int)st.frames + SWALLOW_FRAMES;
    ls.pressed[host][knob] = 0;
}

static void list_click(int host, int knob, int shifted) {
    if ((int)st.frames < ls.swallow_until[host][knob]) {                    // the click after the red hold
        ls.swallow_until[host][knob] = 0;
        return;
    }
    if (shifted) return;
    if (ls.editing[host]) {
        if (knob == KNOB_GREY) rename_keep(host);
        else if (knob == KNOB_RED) rename_cancel(host);
        return;
    }
    if (knob == KNOB_BLUE) { preset_load(host, ls.cursor[host]); list_close(host); }
    else if (knob == KNOB_OCHRE) rename_begin(host);
    else if (knob == KNOB_GREY) preset_save(host);
}

static void list_turn(int host, int knob, int delta) {
    int step = delta > 0 ? 1 : -1;
    if (ls.editing[host]) {
        if (knob == KNOB_BLUE) {
            int c = ls.caret[host] + step;
            if (c >= 0 && c < NAME_CHARS) ls.caret[host] = c;
        } else if (knob == KNOB_OCHRE) {
            char* ch = &ls.buffer[host][ls.caret[host]];
            int i = alphabet_index(*ch) + step;
            if (i < 0) i = ALPHABET_COUNT - 1;
            if (i >= ALPHABET_COUNT) i = 0;
            *ch = ALPHABET[i];
        }
        return;
    }
    if (knob == KNOB_BLUE && presets.count) {
        ls.cursor[host] += step;
        list_clamp(host);
    }
}

static int list_key_down(void* self, int host, const int* event, int shifted) {
    int key = event[0];
    if (key >= KEY_PRESS0 && key < KEY_PRESS0 + WAGONS) { list_press(host, key - KEY_PRESS0); return 1; }
    if (key >= KEY_CLICK0 && key < KEY_CLICK0 + WAGONS) { list_click(host, key - KEY_CLICK0, shifted); return 1; }
    if (key == KEY_T3) {
        if (shifted) { t3_press(host); return 1; }                              // the tap closes the list at the release, the hold saves
        if (ls.editing[host]) rename_keep(host); else preset_load(host, ls.cursor[host]);
        list_close(host);                                                       // back to the train (the GUI re-asks the screen after this key)
        return 1;
    }
    if (key >= KEY_T1 && key < KEY_T1 + 4) { list_close(host); st.train_open[host] = 0; }   // another page: the list and the train close
    else if (shifted) return 1;                                                 // shift + another key on the list: nothing
    return ((KeyEventFn)(shifted ? HOST[host].key_shift : HOST[host].key_down))(self, event);
}

// J5i: the screen request from the frame call (the display thread), behind the mask the binding's note asks for.
// The requests at key releases (t3_release) run on the key thread, as stock's own, and stay plain.
static void request_screen_from_frame() {
    u32 mask;
    STOCK_IRQ_OFF(mask);
    STOCK_REQUEST_SCREEN();
    STOCK_IRQ_ON(mask);
}

static void list_frame(int host) {
    int now = (int)st.ms;
    if (ls.t3_pressed[host] && !ls.t3_acted[host]) {
        int held = now - ls.t3_ms[host];
        if (held >= SAVE_HOLD_MS) {                                            // the hold: the save (its screen says saved!)
            ls.t3_acted[host] = 1;
            preset_save(host);
            if (!ls.open[host]) list_open(host);                               // the list shows the new preset after the confirmation
            request_screen_from_frame();
        } else if (held >= SAVE_SHOW_MS && ls.save_state == SAVE_NONE) {       // H5b: "saving" with the clock filling towards the save
            ls.save_state = SAVE_SAVING;
            ls.save_host = host;
            ls.save_slot = free_slot();                                        // the number the save would take (-1 when the list is full)
            ls.save_ms = ls.t3_ms[host];
            request_screen_from_frame();
        }
    }
    if (ls.save_state == SAVE_SAVED && ls.save_host == host && now - ls.save_ms >= SAVED_MS) {   // H5b: the confirmation ends, the list (or the train) is back
        ls.save_state = SAVE_NONE;
        request_screen_from_frame();
    }
    if (ls.open[host] && ls.pressed[host][KNOB_RED] && !ls.acted[host][KNOB_RED] && now - ls.press_ms[host][KNOB_RED] >= HOLD_MS) {
        ls.acted[host][KNOB_RED] = 1;
        delete_begin(host);
    }
    if (ls.flash_kind != FLASH_NONE && now - ls.flash_until_ms >= 0) {
        if (ls.flash_kind == FLASH_DELETED && ls.pending_delete_slot >= 0) preset_delete_now(ls.pending_delete_slot);
        ls.pending_delete_slot = -1;
        ls.flash_kind = FLASH_NONE;
    }
}

extern "C" int op1f_chain_list_open(int host) {
    return (host == 0 || host == 1) ? ls.open[host] : 0;
}

extern "C" int op1f_chain_screen_wanted(int host) {                          // H5b: the save screen over everything, the list over the train
    if (host != 0 && host != 1) return 0;
    if (ls.save_state != SAVE_NONE && ls.save_host == host) return OP1F_SAVE_SCREEN_ID;
    if (ls.open[host]) return OP1F_PRESETS_SCREEN_ID;
    return 0;
}

// The list's view (build_h_presets.cpp dresses the screens from it every frame).
extern "C" void op1f_presets_view(int host, Op1fPresetsView* out) {
    out->open = ls.open[host];
    out->count = presets.count;
    out->first = ls.first[host];
    out->cursor = ls.cursor[host];
    out->loaded = row_of_slot(ls.loaded_slot[host]);
    out->editing = ls.editing[host];
    out->caret = ls.caret[host];
    for (int r = 0; r < LIST_ROWS; ++r) {
        int i = ls.first[host] + r;
        int s = i < presets.count ? presets.order[i] : -1;
        const char* text = s >= 0 ? presets.name[s] : "";
        if (ls.flash_kind != FLASH_NONE && s >= 0 && ls.flash_slot == s) text = ls.flash_kind == FLASH_SAVED ? "saved" : (ls.flash_kind == FLASH_DELETED ? "deleted" : "list full");
        if (ls.flash_kind == FLASH_FULL && ls.flash_slot < 0 && r == 0) text = "list full";
        if (presets.count == 0 && r == 0 && ls.flash_kind == FLASH_NONE) text = "no presets";
        if (ls.editing[host] && r == 0) text = "";                             // the cells show the name
        name_copy(ls.row_text[r], text);
        out->row[r] = ls.row_text[r];
    }
    const int* v = presets.count ? presets.values[presets.order[ls.cursor[host] < presets.count ? ls.cursor[host] : 0]] : 0;
    for (int k = 0; k < WAGONS; ++k) {
        int type = v ? type_loaded(v[3 + WAGONS + k * (2 + PARAMS)]) : NONE;
        int coupled = v ? v[3 + WAGONS + k * (2 + PARAMS) + 1] : 1;
        char* p = ls.wagon_text[k];
        if (type >= 0) {
            p = put_text(p, fx_name(type));                                    // stock's own lowercase names
            if (!coupled) p = put_text(p, " off");
        } else {
            *p++ = '-';
        }
        *p = 0;
        out->wagon[k] = ls.wagon_text[k];
    }
    for (int c = 0; c < NAME_CHARS; ++c) {
        ls.cell_text[c][0] = ls.buffer[host][c] ? ls.buffer[host][c] : ' ';
        ls.cell_text[c][1] = 0;
        out->cell[c] = ls.cell_text[c];
    }
    out->save_state = ls.save_state;                                           // H5b
    out->save_host = ls.save_host;
    out->save_number = ls.save_slot + 1;
    int held = (int)st.ms - ls.save_ms;
    int spokes = ls.save_state == SAVE_SAVING ? (int)udiv((u32)(held < 0 ? 0 : held) * (u32)OP1F_SAVE_SPOKES, (u32)SAVE_HOLD_MS) : OP1F_SAVE_SPOKES;
    out->save_progress = spokes > OP1F_SAVE_SPOKES ? OP1F_SAVE_SPOKES : spokes;
}

// ---------------------------------------------------------------- the document

// The document's own effect: fx_type's name through stock's own lookup (the apply does the same).
static int doc_fx_type(void* doc) {
    if (!STOCK_JSON_HAS(doc, FX_TYPE_KEY)) return NONE;
    void* entry = STOCK_JSON_GET(doc, FX_TYPE_KEY);
    if (entry == 0) return NONE;
    u32 str[8];
    for (int i = 0; i < 8; ++i) str[i] = 0;
    STOCK_JSON_TO_STRING(str, entry);                              // a short name stays inside the object: nothing to free
    int type = STOCK_FX_TYPE_OF(str);
    return (type >= 0 && type < TYPES) ? type : NONE;
}

static void doc_fx_params(void* doc, u32* out) {
    for (int i = 0; i < PARAMS; ++i) out[i] = 0;
    if (!STOCK_JSON_HAS(doc, FX_PARAMS_KEY)) return;
    void* array = STOCK_JSON_GET(doc, FX_PARAMS_KEY);
    if (!is_array(array)) return;
    int n = STOCK_JSON_SIZE(array);
    for (int i = 0; i < PARAMS && i < n; ++i) out[i] = word_of(int_at(array, i));
}

static int doc_fx_active(void* doc) {
    if (!STOCK_JSON_HAS(doc, FX_ACTIVE_KEY)) return 0;
    void* entry = STOCK_JSON_GET(doc, FX_ACTIVE_KEY);
    return entry && STOCK_JSON_BOOL(entry) ? 1 : 0;
}

// The document's effect becomes the blue wagon; the others empty.
static void chain_defaults(int host, u32 slot, void* doc, int dt) {
    for (int k = 0; k < WAGONS; ++k) {
        Wagon* w = &wagons[host][slot][k];
        w->type = NONE;
        w->coupled = 1;
        for (int i = 0; i < PARAMS; ++i) w->params[i] = 0;
    }
    int knobs[WAGONS] = { 0, 1, 2, 3 };
    set_order(host, slot, knobs);
    mirror[host][slot] = 0;
    Wagon* first = &wagons[host][slot][0];
    first->type = dt;
    first->coupled = doc_fx_active(doc);
    if (dt >= 0) doc_fx_params(doc, first->params);
    doc_type[host][slot] = dt;
}

// I9 (analysis/lfo_train_study.md section 22): a new instrument was loaded from a file onto the slot (build_i_patch.cpp
// saw stock's job put it on the record). Its file carried the slot's integers as this unit's own file holds them
// for a slot, and they are read as load_file reads them; or it carried none, an instrument as stock alone knows
// it, and the slot is forgotten: the reconcile that follows then makes the chain of a slot seen for the first time
// (chain_defaults: the document's effect on the blue wagon, the three others empty). Either way the LFOs' effect
// wagons and the LFO unit's three other LFOs are the instrument's, never the sound key's of before.
static int instrument_lines;

static void instrument_event(int host, u32 slot) {
    if (!&op1f_patch_pending) return;
    static int v[OP1F_PATCH_INTS];                                      // the GUI thread only
    int n = op1f_patch_pending(host, (int)slot, v);
    if (n < 0) return;
    lfo_fx_pick[host][slot] = 0;
    for (int i = 0; i < MIDI_DESTS; ++i) lfo_midi_pick[host][slot][i] = 0;
    int own = n >= CHAIN_INTS_V2 && v[0] == CHAIN_TAG_V2, lfos = 0;
    if (own) {
        chain_from_values(host, slot, v);
        if (n >= CHAIN_INTS_V2 + 2 && v[CHAIN_INTS_V2] == LFOFX_TAG) {
            int pick = v[CHAIN_INTS_V2 + 1];
            lfo_fx_pick[host][slot] = (u8)(pick >= 0 && pick <= WAGONS ? pick : 0);
            if (n >= CHAIN_INTS_V2 + 4 && v[CHAIN_INTS_V2 + 2] == LFOMIDI_TAG) {
                int packed = v[CHAIN_INTS_V2 + 3];
                for (int i = 0; i < MIDI_DESTS; ++i) {
                    int one = (packed >> (4 * i)) & 15;
                    lfo_midi_pick[host][slot][i] = (u8)(one <= WAGONS ? one : 0);
                }
                if (host == 0 && n >= LFO4_AT + 1 + LFO4_MIN && v[LFO4_AT] == LFO4_TAG) {
                    int more[LFO4_INTS];
                    int have = n - (LFO4_AT + 1);
                    for (int i = 0; i < LFO4_INTS; ++i) more[i] = i < have ? v[LFO4_AT + 1 + i] : -1;
                    if (&op1f_lfo4_store_load) op1f_lfo4_store_load((int)slot, more);
                    else lfo4_keep(slot, more);                         // J5f: the LFO unit is away: the instrument's four are carried for the slot
                    lfos = 1;
                }
            }
        }
        known[host][slot] = 1;
    } else {
        known[host][slot] = 0;                                          // the reconcile below: chain_defaults
    }
    if (host == 0 && !lfos && !&op1f_lfo4_store_load) lfo4_drop(slot);  // J5f: the LFO unit is away and the instrument brings no LFOs of ours: nothing is carried for the slot
    if (host == 0 && !lfos && &op1f_lfo4_store_load) {                  // the LFO page as stock alone has it: the sound's own LFO on the blue knob, the three others empty, the knobs' own order
        int plain[LFO4_INTS];
        for (int i = 0; i < LFO4_INTS; ++i) plain[i] = 0;
        plain[1] = 1;                                                   // the first wagon is not emptied (under the lamp the kind's sign alone counts)
        plain[LFO4_MIN] = -1;
        op1f_lfo4_store_load((int)slot, plain);
    }
    mark_dirty(host);
    if (instrument_lines < 60) {
        ++instrument_lines;
        STOCK_LOG(3, LOG_FILE, 31, "chain instrument: %s s%u: %s (%d integers); its LFOs: %s", host ? "drum" : "synth", slot,
                  own ? "the file's own train" : "none in the file, the instrument's one effect on the blue wagon", n,
                  host ? "stock's" : (lfos ? "the file's four" : "the instrument's one, on the blue knob"));
    }
}

// The record's document against the chain. Run after stock applied a document to the slot (the hooks), when the
// module first sees a slot and when the slot's type word changes under the module. The type the chain was last
// reconciled with tells a stale document re-applied (the chain stands, its mirror written back) from a browser
// pick or a preset loaded (the mirror takes the document's effect).
static void reconcile(int host, u32 slot, const char* why) {
    if (!file_loaded) load_file();
    instrument_event(host, slot);                                    // I9: a new instrument on this sound key brings its own chain, or none
    void* doc = (void*)(patch_record(host, slot) + 8);
    int dt = doc_fx_type(doc);
    int applied = slot_type(host, slot);
    const char* action;
    if (!known[host][slot]) {
        chain_defaults(host, slot, doc, dt);
        known[host][slot] = 1;
        action = "defaults";
    } else if (dt == doc_type[host][slot]) {                         // the document the chain knows: the chain stands
        action = "stands";
        ++st.restores;
    } else {                                                         // another effect in the document: the mirror takes it
        int m = mirror[host][slot];
        Wagon* mw = &wagons[host][slot][m];
        remember(host, slot, m, mw);
        mw->type = dt;
        mw->coupled = doc_fx_active(doc);
        if (dt >= 0) doc_fx_params(doc, mw->params); else for (int i = 0; i < PARAMS; ++i) mw->params[i] = 0;
        dedupe(host, slot, m, dt);
        doc_type[host][slot] = dt;
        mark_dirty(host);
        action = "adopts";
        ++st.adopts;
    }
    initialised[host][slot] = 1;
    mirror_push(host, slot);                                         // the slot holds the mirrored wagon
    if (st.current_slot[host] == (int)slot) st.current_slot[host] = -1;    // the live instances follow at the next frame
    ++st.reconciles;
    if (st.diag_lines < 40) {
        ++st.diag_lines;
        STOCK_LOG(3, LOG_FILE, 6, "chain reconcile: %s s%u (%s): doc %s, applied %s -> %s; mirror %d order %d%d%d%d: %s, %s, %s, %s",
                  host ? "drum" : "synth", slot, why, fx_name(dt), fx_name(applied), action,
                  mirror[host][slot], knob_at(host, slot, 0), knob_at(host, slot, 1), knob_at(host, slot, 2), knob_at(host, slot, 3),
                  fx_name(wagons[host][slot][0].type), fx_name(wagons[host][slot][1].type), fx_name(wagons[host][slot][2].type), fx_name(wagons[host][slot][3].type));
    }
}

// The vtable word 0x8401e90c (H1): a preset job's apply (load a preset by step); it changes the record and applies
// it through the record's own virtual below, so this wrapper only reconciles once more. Kept as H1 left it.
extern "C" void op1f_fx_patch_apply(void* descriptor) asm("op1f_fx_patch_apply");
extern "C" void op1f_fx_patch_apply(void* descriptor) {
    u32 slot = *(u32*)((u8*)descriptor + 4) & 7u;
    int host = *((u8*)descriptor + 8) ? 1 : 0;
    STOCK_PATCH_APPLY(descriptor);
    reconcile(host, slot, "job");
}

// The records' apply virtual (vtable + 0x10; the words 0x8401f894 and 0x8401f870, H1b): every application of a
// record's document to its slot passes here: the boot's applies of all sixteen records (1.8 s in, after the slots
// files are loaded and the documents rebuilt), the slot switch, the browser's pick, the preset jobs.
static void record_applied(int host, void* record) {
    u32 slot = *(u32*)((u8*)record + 4) & 7u;
    ((ApplyFn)(host ? STOCK_APPLY_DRUM : STOCK_APPLY_SYNTH))(record);
    reconcile(host, slot, "apply");
}

extern "C" void op1f_fx_apply_synth(void* record) asm("op1f_fx_apply_synth");
extern "C" void op1f_fx_apply_synth(void* record) { record_applied(0, record); }

extern "C" void op1f_fx_apply_drum(void* record) asm("op1f_fx_apply_drum");
extern "C" void op1f_fx_apply_drum(void* record) { record_applied(1, record); }

// A slot the module has not seen: once its record holds a document, or after five seconds, the chain is reconciled
// with it.
static void init_slot(int host, u32 slot) {
    void* doc = (void*)(patch_record(host, slot) + 8);
    if (STOCK_JSON_HAS(doc, FX_TYPE_KEY) || st.frames > (u32)INIT_WAIT_FRAMES) {
        reconcile(host, slot, "init");
        ++st.inits;
    }
}

// I9: the slot's integers for an instrument's file, as save_file writes them for the slot (the chain's 48, the two
// pairs of the LFOs' effect wagons, then the tag 98 and the LFO unit's integers when the sound has more than stock's
// one LFO). 0 for a slot the module does not know and for one that is as stock alone has it: the lamp on the blue
// knob, the knobs' own order, nothing on the three other wagons, no LFO aimed at a wagon of its choice, no added LFO.
extern "C" int op1f_chain_slot_values(int host, int slot, int* out) {
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS || !known[host][slot]) return 0;
    int values[CHAIN_INTS_V2];
    chain_values(host, (u32)slot, values);
    int tail[4] = { (int)LFOFX_TAG, (int)lfo_fx_pick[host][slot], (int)LFOMIDI_TAG, 0 };
    for (int i = 0; i < MIDI_DESTS; ++i) tail[3] |= (int)lfo_midi_pick[host][slot][i] << (4 * i);
    int more[LFO4_INTS];
    int have = host != 0 ? 0 : (&op1f_lfo4_store_save ? op1f_lfo4_store_save(slot, more) : lfo4_carried((u32)slot, more));   // J5f: what was read, while the LFO unit is away
    int plain = !have && tail[1] == 0 && tail[3] == 0 && mirror[host][slot] == 0;
    for (int p = 0; p < WAGONS; ++p) if (knob_at(host, (u32)slot, p) != p) plain = 0;
    for (int k = 1; k < WAGONS; ++k) if (wagons[host][slot][k].type >= 0) plain = 0;
    if (plain) return 0;
    int n = 0;
    for (int i = 0; i < CHAIN_INTS_V2; ++i) out[n++] = values[i];
    for (int i = 0; i < 4; ++i) out[n++] = tail[i];
    if (have) {
        out[n++] = (int)LFO4_TAG;
        for (int i = 0; i < LFO4_INTS; ++i) out[n++] = more[i];
    }
    return n;
}

// I9: the slot's record against its chain once more, asked by build_i_patch.cpp for an instrument stock did not apply.
extern "C" void op1f_chain_instrument(int host, int slot) {
    if ((host != 0 && host != 1) || slot < 0 || slot >= SLOTS) return;
    reconcile(host, (u32)slot, "instrument");
}

// ---------------------------------------------------------------- the frame

// An offered wagon's name for the train's label (build_h_train.cpp), or 0 for a stock type and none.
extern "C" const char* op1f_chain_wagon_name(int type) asm("op1f_chain_wagon_name");
extern "C" const char* op1f_chain_wagon_name(int type) { return wagon_of(type) ? wagon_of(type)->name : 0; }

// Its picture for the train (version 2), or 0: build_h_train.cpp asks once, when it makes the train's picture.
extern "C" const char* op1f_chain_wagon_art(int type) asm("op1f_chain_wagon_art");
extern "C" const char* op1f_chain_wagon_art(int type) { const Op1fWagon* w = wagon_of(type); return w && w->version >= 2 ? w->art : 0; }

// The page's view of the chain (H2): build_h_train.cpp dresses the train screen from it every frame.
extern "C" void op1f_chain_view(int host, Op1fChainView* out) {
    u32 slot = gui_slot(host);
    out->open = st.train_open[host];
    out->known = initialised[host][slot];
    out->slot = (int)slot;
    for (int p = 0; p < WAGONS; ++p) out->order[p] = knob_at(host, slot, p);
    for (int k = 0; k < WAGONS; ++k) {
        const Wagon* w = &wagons[host][slot][k];
        out->type[k] = w->type;
        out->coupled[k] = w->coupled;
    }
    out->mirror = mirror[host][slot];
    out->full = (int)st.frames < st.flash_until[host];
    out->refused_knob = out->full ? st.refused_knob[host] : -1;
    out->refused_type = out->full ? st.refused_type[host] : -1;
    out->cost_percent = (int)udiv(chain_cost(host, slot, -1, NONE) * 100u, (u32)CEILING_CYCLES);
    out->turn_knob = st.turn_frame[host] ? st.turn_knob[host] : -1;
    out->turn_age = st.turn_frame[host] ? (int)st.frames - st.turn_frame[host] : 0x7fffffff;
    out->turn_move = st.turn_move[host];
    out->bypassed = bypass_mask[host][slot] != 0;
    out->peak = bus_peak[host];                                                // the audio thread's maximum since the last frame
    bus_peak[host] = 0;
}

extern "C" int op1f_chain_train_open(int host) {
    return (host == 0 || host == 1) ? st.train_open[host] : 0;
}

static void frame_host(int host) {
    u32 slot = gui_slot(host);
    if (!initialised[host][slot]) init_slot(host, slot);
    if (!initialised[host][slot]) return;                                   // the record not loaded yet: stock's producer runs as stock's (chain_ready stays 0)
    // a new current slot: its chain goes live
    if (st.current_slot[host] != (int)slot) {
        st.current_slot[host] = (int)slot;
        chain_ready[host] = 0;
        for (int k = 0; k < WAGONS; ++k) {
            Wagon* w = &wagons[host][slot][k];
            if (k == mirror[host][slot]) { live[host][k].obj = 0; live[host][k].type = NONE; st.pending_type[host][k] = NONE; continue; }
            build_live(host, k, w->type);
            for (int i = 0; i < PARAMS; ++i) live[host][k].ctx[i] = w->params[i];
            st.pending_type[host][k] = w->type;
        }
        st.push_pending[host] = 0;
        chain_ready[host] = 1;
    }
    int m = mirror[host][slot];
    Wagon* mw = &wagons[host][slot][m];
    // the mirror's own pick, after the knob's rest
    if (st.push_pending[host] && (int)st.frames - st.pending_since[host][m] >= REST_FRAMES) {
        st.push_pending[host] = 0;
        mirror_push(host, slot);
    }
    // the mirrored wagon follows stock's slot (the knobs on the effect page, a toggle); another effect in the slot
    // word means a document applied behind the module's back: reconcile
    if (!st.push_pending[host]) {
        int stock_type = slot_type(host, slot);
        if (stock_type != stock_type_seen[host][slot]) {
            stock_type_seen[host][slot] = stock_type;
            if (stock_type != mw->type) {
                ++st.mirror_syncs;
                reconcile(host, slot, "type word");
            }
        }
        m = mirror[host][slot];
        mw = &wagons[host][slot][m];
        if (mw->type >= 0 && slot_type(host, slot) == mw->type) {
            u32 current[PARAMS];
            slot_read_params(host, slot, current);
            int changed = 0;
            for (int i = 0; i < PARAMS; ++i) if (current[i] != mw->params[i]) changed = 1;
            if (changed) {                                                              // H6c: counted; one soon after a document write is something rewriting the block under the module
                ++st.param_follows;
                int early = st.doc_writes && st.ms - st.doc_ms < 2000u;
                if (early) ++st.early_follows;
                if (early && st.param_lines < 24) {
                    ++st.param_lines;
                    STOCK_LOG(3, LOG_FILE, 23, "chain params: %s s%u knob %d %s: the slot's block changed %u ms after a document write: %d %d %d %d -> %d %d %d %d",
                              host ? "drum" : "synth", slot, m, fx_name(mw->type), st.ms - st.doc_ms,
                              int_of(mw->params[0]), int_of(mw->params[1]), int_of(mw->params[2]), int_of(mw->params[3]),
                              int_of(current[0]), int_of(current[1]), int_of(current[2]), int_of(current[3]));
                }
                for (int i = 0; i < PARAMS; ++i) mw->params[i] = current[i];
                mark_dirty(host);
            }
            int active = RD8(HOST[host].fx_active + slot) ? 1 : 0;
            if (active != mw->coupled) {                                            // H3b: logged, the one path besides the hold and the bypass that moves a wagon
                ++st.follows;
                STOCK_LOG(3, LOG_FILE, 9, "chain follow: %s s%u knob %d fx_active %d under the module, the wagon %s", host ? "drum" : "synth", slot, m, active, mw->coupled ? "coupled" : "uncoupled");
                mw->coupled = active;
                mark_dirty(host);
            }
        }
    }
    // the other wagons' picks after the knob's rest
    for (int k = 0; k < WAGONS; ++k) {
        if (k == m) continue;
        int type = st.pending_type[host][k];
        if (type != live[host][k].type && (int)st.frames - st.pending_since[host][k] >= REST_FRAMES) {
            build_live(host, k, type);
            for (int i = 0; i < PARAMS; ++i) live[host][k].ctx[i] = wagons[host][slot][k].params[i];
        }
    }
    // an offered wagon's page writes its values into the wagon (the key thread): the instance follows, and the file
    for (int k = 0; k < WAGONS; ++k) {
        Wagon* w = &wagons[host][slot][k];
        if (k == m || !wagon_of(w->type) || live[host][k].type != w->type) continue;
        int changed = 0;
        for (int i = 0; i < PARAMS; ++i) if (live[host][k].ctx[i] != w->params[i]) { live[host][k].ctx[i] = w->params[i]; changed = 1; }
        if (changed) mark_dirty(host);
    }
    // the holds
    for (int k = 0; k < WAGONS; ++k) {
        if (st.pressed[host][k] && st.hold_armed[host][k] && !st.acted[host][k] && (int)st.ms - st.press_ms[host][k] >= HOLD_MS) {
            st.acted[host][k] = 1;
            if (st.press_shift[host][k]) reset_params(host, k);
            else {
                ++st.holds;
                int coupled = !wagons[host][slot][k].coupled;
                set_coupled(host, k, coupled);
                int gap = st.press_ms[host][k] - st.last_turn_ms[host][k];         // H3b: every hold logged with the turn before it, for the read-out (H4b: in ms)
                STOCK_LOG(3, LOG_FILE, 8, "chain hold: knob %d %s, the last turn %d ms before the press", k, coupled ? "coupled" : "uncoupled", gap > 99999 ? 99999 : gap);
            }
        }
    }
    // the save, a moment after the last change (the whole file)
    if (st.dirty[host] && (int)st.frames - st.dirty_since[host] >= SAVE_DELAY_FRAMES) save_pending();
}

// Q1: what the periodic lines state, as one number for the gate (build_q_quiet.h): the wagons, their coupling and
// what is built for them, the counts of events (the knob turns on the effect and LFO pages among them, which this
// unit has counted since H6h), the presets' list. Not what the lines measure (cycles a block, the time of a period:
// those differ at every period by themselves), but for the frame rate's eighth, which moves when a page becomes
// heavy (58 frames a second at rest, 26 to 29 on the UFOs' page while playing).
static u32 lines_signature(u32 rate_whole) {
    u32 s = quiet_start();
    for (int host = 0; host < HOSTS; ++host) {
        u32 slot = gui_slot(host);
        s = quiet_mix(s, slot);
        s = quiet_mix(s, (u32)mirror[host][slot]);
        for (int i = 0; i < WAGONS; ++i) s = quiet_mix(s, (u32)knob_at(host, slot, i));
        for (int k = 0; k < WAGONS; ++k) {
            const Wagon* w = &wagons[host][slot][k];
            s = quiet_mix(s, (u32)w->type);
            s = quiet_mix(s, w->coupled ? 1u : 0u);
            s = quiet_mix(s, live[host][k].obj != 0 ? 1u : 0u);
        }
        s = quiet_mix(s, (u32)chain_ready[host]);
        s = quiet_mix(s, (u32)doc_type[host][slot]);
        s = quiet_mix(s, ls.open[host] ? 1u : 0u);
        s = quiet_mix(s, (u32)ls.cursor[host]);
        s = quiet_mix(s, (u32)ls.loaded_slot[host]);
    }
    s = quiet_mix(s, st.picks); s = quiet_mix(s, st.moves); s = quiet_mix(s, st.taps); s = quiet_mix(s, st.holds); s = quiet_mix(s, st.randoms);
    s = quiet_mix(s, st.resets); s = quiet_mix(s, st.refusals); s = quiet_mix(s, st.saves); s = quiet_mix(s, st.loads); s = quiet_mix(s, st.adopts);
    s = quiet_mix(s, st.restores); s = quiet_mix(s, st.reconciles); s = quiet_mix(s, st.builds); s = quiet_mix(s, st.page_opens); s = quiet_mix(s, st.train_opens);
    s = quiet_mix(s, st.mirror_syncs); s = quiet_mix(s, st.inits); s = quiet_mix(s, st.swallowed); s = quiet_mix(s, st.bypasses); s = quiet_mix(s, st.turn_presses);
    s = quiet_mix(s, st.follows);
    s = quiet_mix(s, (u32)presets.count); s = quiet_mix(s, ls.opens); s = quiet_mix(s, ls.saves); s = quiet_mix(s, ls.loads); s = quiet_mix(s, ls.deletes);
    s = quiet_mix(s, ls.renames); s = quiet_mix(s, ls.scans); s = quiet_mix(s, ls.rereads); s = quiet_mix(s, ls.writes); s = quiet_mix(s, ls.migrated);
    s = quiet_mix(s, &op1f_disk_folders ? op1f_disk_folders : 0u); s = quiet_mix(s, &op1f_disk_files ? op1f_disk_files : 0u);
    s = quiet_mix(s, &op1f_disk_lookups ? op1f_disk_lookups : 0u); s = quiet_mix(s, &op1f_disk_refused ? op1f_disk_refused : 0u);
    s = quiet_mix(s, st.ghosts); s = quiet_mix(s, st.clamps); s = quiet_mix(s, ls.mirror_moves);
    s = quiet_mix(s, st.doc_writes); s = quiet_mix(s, st.param_follows); s = quiet_mix(s, st.early_follows);
    s = quiet_mix(s, st.knob_turns);
    s = quiet_mix(s, rate_whole >> 3);
    return s;
}

// The periodic lines. Q1: the wagons' and the section's statistics are read and cleared at every call as before;
// the lines go to the log when the gate says so (`why` is the power popup's reason when the frame call asks for the
// lines out of turn, else 0 and the gate is asked: a change in what the lines state, synth voices sounding, the two
// periods after either, the first three of the start, or five minutes without a line).
static void print_lines(int why) {
    u32 clock_frames = st.frames - st.frames_at_clock, clock_ms = st.ms - st.ms_at_clock;      // H4b: the frame rate over the period
    u32 rate10 = udiv(clock_frames * 10000u, clock_ms), rate_whole = udiv(rate10, 10u);
    st.frames_at_clock = st.frames;
    st.ms_at_clock = st.ms;
    if (!why) why = quiet_period(&st.quiet, lines_signature(rate_whole), &op1f_fx_voices && op1f_fx_voices != 0);
    int speak = why != 0;
    for (int host = 0; host < HOSTS; ++host) {
        u32 slot = gui_slot(host);
        int aimed = lfo_fx_knob_of(host, slot);
        if (lfo_type_word(host, slot) == (u32)LFO_TYPE_MIDI) {                 // I4: the midi LFO's four destinations this period, while one of them turned another wagon than the lamp's
            if ((lfofx_turned[host] || lfofx_idle[host]) && lfofx_period_lines < (u32)LFOFX_PERIOD_LINES) {
                ++lfofx_period_lines;
                u32 turned = lfofx_turned[host], idle = lfofx_idle[host], stock = lfofx_stock[host];
                lfofx_turned[host] = 0;
                lfofx_idle[host] = 0;
                u32 object = RD32(LFO_FX_OBJECT[host]);
                const u32* b = lfo_block_of(host, slot);
                STOCK_LOG(3, LOG_FILE, 33, "lfo fx: %s s%u midi: destinations %d %d %d %d on knobs %d %d %d %d, lamp on knob %d: %u wagon turns, %u parked or not built, %u blocks stock's way; object class %08x",
                          host ? "drum" : "synth", slot, int_of(b[4]), int_of(b[5]), int_of(b[6]), int_of(b[7]),
                          lfo_midi_knob_of(host, slot, 0), lfo_midi_knob_of(host, slot, 1), lfo_midi_knob_of(host, slot, 2), lfo_midi_knob_of(host, slot, 3),
                          (int)mirror[host][slot], turned, idle, stock, object ? RD32(object) : 0u);
            }
        } else if ((lfofx_turned[host] || lfofx_idle[host] || (aimed != mirror[host][slot] && lfofx_stock[host])) && lfofx_period_lines < (u32)LFOFX_PERIOD_LINES) {   // I2: the stock LFO's effect destination this period, while another wagon than the lamp's is chosen (the first sixty-four lines)
            ++lfofx_period_lines;
            u32 turned = lfofx_turned[host], idle = lfofx_idle[host], stock = lfofx_stock[host];
            lfofx_turned[host] = 0;
            lfofx_idle[host] = 0;
            u32 object = RD32(LFO_FX_OBJECT[host]);
            STOCK_LOG(3, LOG_FILE, 32, "lfo fx: %s s%u aims at knob %d (%s), choice %d, lamp on knob %d: %u blocks turned, %u parked or not built, %u stock's way; lfo type %u, destination %d, object class %08x",
                      host ? "drum" : "synth", slot, aimed, fx_name(wagons[host][slot][aimed].type), (int)lfo_fx_pick[host][slot], (int)mirror[host][slot], turned, idle, stock,
                      lfo_type_word(host, slot), int_of(lfo_block_of(host, slot)[LFO_DEST_WORD]), object ? RD32(object) : 0u);
        }
        lfofx_stock[host] = 0;
        char* p = put_text(st.line, host ? "chain drum: s" : "chain synth: s");
        p = put_u32(p, slot);
        p = put_text(p, " m");
        p = put_u32(p, mirror[host][slot]);
        p = put_text(p, " order ");
        for (int i = 0; i < WAGONS; ++i) p = put_u32(p, (u32)knob_at(host, slot, i));
        for (int k = 0; k < WAGONS; ++k) {
            const Wagon* w = &wagons[host][slot][k];
            p = put_text(p, " | k");
            p = put_u32(p, (u32)k);
            *p++ = ' ';
            p = put_text(p, fx_name(w->type));
            p = put_text(p, w->coupled ? " on" : " off");
            volatile Stat* s = &wagon_stat[host][k];
            u32 count = s->count, sum = s->sum, max = s->max;
            s->sum = 0; s->count = 0; s->max = 0;
            if (count) {
                *p++ = ' ';
                p = put_u32(p, udiv(sum, count));
                *p++ = '/';
                p = put_u32(p, max);
            }
        }
        *p = 0;
        if (speak) STOCK_LOG(3, LOG_FILE, 1 + host, "%s", st.line);
        volatile Stat* c = &chain_stat[host];
        u32 count = c->count, sum = c->sum, max = c->max;
        c->sum = 0; c->count = 0; c->max = 0;
        if (count && speak)
            STOCK_LOG(3, LOG_FILE, 3 + host, "chain %s section: %u/%u cycles x%u, ready %d, live %d%d%d%d, doc type %d",
                      host ? "drum" : "synth", udiv(sum, count), max, count, chain_ready[host],
                      live[host][0].obj != 0, live[host][1].obj != 0, live[host][2].obj != 0, live[host][3].obj != 0, doc_type[host][slot]);
    }
    if (!speak) return;
    STOCK_LOG(3, LOG_FILE, 5, "chain: %u picks, %u moves, %u taps, %u holds, %u randoms, %u resets, %u refused, %u saves, %u loads, %u adopts, %u stands, %u reconciles, %u builds, %u pages, %u trains, %u syncs, %u inits, %u swallowed, %u bypasses, %u turn presses, %u follows",
              st.picks, st.moves, st.taps, st.holds, st.randoms, st.resets, st.refusals, st.saves, st.loads, st.adopts, st.restores, st.reconciles, st.builds, st.page_opens, st.train_opens, st.mirror_syncs, st.inits, st.swallowed, st.bypasses, st.turn_presses, st.follows);
    STOCK_LOG(3, LOG_FILE, 18, "presets: %d slots used, %u opens, %u saves, %u loads, %u deletes, %u renames, %u scans, %u files read, %u written, %u migrated; synth list %s cursor %d loaded slot %d; drum list %s cursor %d loaded slot %d",
              presets.count, ls.opens, ls.saves, ls.loads, ls.deletes, ls.renames, ls.scans, ls.rereads, ls.writes, ls.migrated, ls.open[0] ? "open" : "closed", ls.cursor[0], ls.loaded_slot[0], ls.open[1] ? "open" : "closed", ls.cursor[1], ls.loaded_slot[1]);
    // H6j: a line of their own for the disk's counts, which the logger cut off the line above at its 260 characters
    // (every read-out since H6), with the values' counts: the ghosts taken as empty, the parameters held, the mirror's moves
    STOCK_LOG(3, LOG_FILE, 30, "presets disk: %u fx folders, %u files, %u lookups, %u refused; values: %u ghost delays taken as empty, %u parameters held to their range, %u mirror moves",
              &op1f_disk_folders ? op1f_disk_folders : 0u, &op1f_disk_files ? op1f_disk_files : 0u, &op1f_disk_lookups ? op1f_disk_lookups : 0u, &op1f_disk_refused ? op1f_disk_refused : 0u,
              st.ghosts, st.clamps, ls.mirror_moves);
    // H4b: the frame rate over the period, for the read-out (the hold no longer depends on it)
    STOCK_LOG(3, LOG_FILE, 13, "chain clock: %u frames in %u ms, %u.%u a second; the hold %d ms, the clean window %d ms; %u documents written, %u param follows (%u within 2 s of a write)",
              clock_frames, clock_ms, rate_whole, rate10 - rate_whole * 10u, (int)HOLD_MS, (int)CLEAN_MS, st.doc_writes, st.param_follows, st.early_follows);
    if (st.quiet.silent_before)                                    // Q1: how long the lines were held back, and what brought them out
        STOCK_LOG(3, LOG_FILE, 35, "chain quiet: these lines after %u periods without (%s); %u periods printed and %u not since the start",
                  st.quiet.silent_before, quiet_why(why), st.quiet.spoken, st.quiet.skipped);
}

// H4: the eight private regions, one block of C heap 0 at the first frame (B0a's allocator; heap 0 is stock's own
// heap, 1.4 MiB of its 32 MiB in use); refused, the H1 rule stays and a level-1 line says so.
static void allocate_regions() {
    u32 bytes = (u32)REGION_BYTES * (u32)REGION_COUNT + (u32)REGION_ALIGN;
    region_pool = (u8*)STOCK_HEAP_MALLOC(REGION_HEAP, bytes);
    if (region_pool == 0) {
        STOCK_LOG(1, LOG_FILE, 11, "chain regions: heap %d refused %u bytes: one wagon per effect type, as before H4", (int)REGION_HEAP, bytes);
        return;
    }
    u32 base = ((u32)region_pool + REGION_ALIGN - 1u) & ~(u32)(REGION_ALIGN - 1);
    for (int h = 0; h < HOSTS; ++h)
        for (int k = 0; k < WAGONS; ++k) live[h][k].region = (u8*)(base + (u32)REGION_BYTES * (u32)(h * WAGONS + k));
    regions_ready = 1;
    STOCK_LOG(3, LOG_FILE, 12, "chain regions: %d x %u bytes at %08x (heap %d block %08x): the same effect may ride on several wagons", (int)REGION_COUNT, (u32)REGION_BYTES, base, (int)REGION_HEAP, (u32)region_pool);
}

extern "C" void op1f_chain_frame(u16* fb) asm("op1f_chain_frame");
extern "C" void op1f_chain_frame(u16* fb) {
    if (st.frames == 0) {
        allocate_regions();                                        // H4: before any instance is built
        for (int h = 0; h < HOSTS; ++h) {
            st.current_slot[h] = -1;
            for (int k = 0; k < WAGONS; ++k) st.last_turn_ms[h][k] = -100000;     // H3b: no turn yet
            ls.loaded_slot[h] = -1;                                 // H5
        }
        ls.pending_delete_slot = -1;
        ls.flash_slot = -1;
        ls.save_slot = -1;
    }
    ++st.frames;
    u32 now = cycles();                                            // H4b: the clock, whatever the frame rate
    if (st.frames > 1) {
        st.cycle_acc += now - st.last_cycles;
        while (st.cycle_acc >= (u32)CYCLES_PER_MS) { st.cycle_acc -= (u32)CYCLES_PER_MS; ++st.ms; }
    }
    st.last_cycles = now;
    for (int h = 0; h < HOSTS; ++h)
        for (u32 s = 0; s < SLOTS; ++s) if (!initialised[h][s]) init_slot(h, s);
    int mode = (int)RD32(MODE_GUI);
    for (int host = 0; host < HOSTS; ++host) {
        if (st.train_open[host] && mode != host) { st.train_open[host] = 0; list_close(host); if (ls.save_host == host) ls.save_state = SAVE_NONE; }
        frame_host(host);
        list_frame(host);                                          // H5: the T3 hold, the red hold, the flash
    }
    (void)fb;                                                      // H2: nothing is painted over the frame; the train is the GUI's own screen (build_h_train.cpp)
    if (RD8(POWER_POPUP_SHOWN) != 0) save_pending();               // the countdown popup is on the frame: save now
    probes_frame();                                                // H6i: the knobs that rested say what they reached (off since H6j)
    if (&op1f_lfopage_frame) op1f_lfopage_frame();                 // I2: the value LFO's page, its wagon marker (build_i_lfopage.cpp)
    if (&op1f_lfo4_frame) op1f_lfo4_frame();                       // I5: the second LFO's log lines (build_i_lfo4.cpp)
    if (&op1f_lfopresets_frame) op1f_lfopresets_frame();           // I8: the LFO presets' list: the hold of shift + T4, the red hold, the flash (build_i_lfopresets.cpp)
    if (&op1f_patch_frame) op1f_patch_frame();                     // I9: an instrument loaded from a file that stock did not apply, and that unit's counts (build_i_patch.cpp)
    rows_frame();                                                  // H6j: the descriptor rows, once
    if (quiet_popup(&st.quiet, RD8(POWER_POPUP_SHOWN) != 0)) print_lines(QUIET_WHY_POPUP);   // Q1: the lines once more before the unit is off
    if (--st.print_countdown <= 0) {
        st.print_countdown = PRINT_FRAMES;
        print_lines(0);                                            // Q1: printed when the gate says so (build_q_quiet.h)
    }
}

// ---------------------------------------------------------------- I6: what the LFO unit (build_i_lfo4.cpp) asks of this one

// The millisecond clock of the frame call (H4b), for its holds.
extern "C" u32 op1f_chain_ms() asm("op1f_chain_ms");
extern "C" u32 op1f_chain_ms() { return st.ms; }

// J5h: this unit has read its file, so what that file holds for the LFO unit is handed over. The LFOs' own file
// (build_j5h_lfo_alone.cpp) holds its copy against this unit's only after that.
extern "C" int op1f_chain_file_loaded() asm("op1f_chain_file_loaded");
extern "C" int op1f_chain_file_loaded() { return file_loaded; }

// The chain file is written a moment after the last change (the LFO unit's store rides in it).
extern "C" void op1f_chain_touch(int host) asm("op1f_chain_touch");
extern "C" void op1f_chain_touch(int host) {
    if (host != 0 && host != 1) return;
    st.dirty[host] = 1;                                         // not mark_dirty: the effect chain did not change, the presets list keeps its lamp
    st.dirty_since[host] = (int)st.frames;
}

// The effect train leaves the screen: the LFO train takes its place on T4.
extern "C" void op1f_chain_close_train(int host) asm("op1f_chain_close_train");
extern "C" void op1f_chain_close_train(int host) { if (host == 0 || host == 1) st.train_open[host] = 0; }

// The effect wagon stock's slot LFO turns (I2) and its midi kind's four (I4): read, or written when the lamp of the
// LFO train moves and another LFO becomes stock's.
extern "C" void op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) asm("op1f_chain_lfo_picks");
extern "C" void op1f_chain_lfo_picks(int host, u32 slot, int write, u8* pick, u8* midi) {
    if ((host != 0 && host != 1) || slot >= (u32)SLOTS) return;
    if (write) {
        lfo_fx_pick[host][slot] = (u8)(*pick <= WAGONS ? *pick : 0);
        for (int i = 0; i < MIDI_DESTS; ++i) lfo_midi_pick[host][slot][i] = (u8)(midi[i] <= WAGONS ? midi[i] : 0);
    } else {
        *pick = lfo_fx_pick[host][slot];
        for (int i = 0; i < MIDI_DESTS; ++i) midi[i] = lfo_midi_pick[host][slot][i];
    }
}

// I8: the knob (0..3) of the effect wagon a choice turns now: the chosen one while it carries an effect, else the
// one under the effect train's lamp (extra_knob_of, the rule the fx side runs by). An LFO preset writes this knob
// in place of a choice of 0, so it does not depend on where the lamp stood when it was saved.
extern "C" int op1f_chain_lfo_knob(int host, u32 slot, int pick) asm("op1f_chain_lfo_knob");
extern "C" int op1f_chain_lfo_knob(int host, u32 slot, int pick) {
    if ((host != 0 && host != 1) || slot >= (u32)SLOTS) return 0;
    return extra_knob_of(host, slot, pick);
}

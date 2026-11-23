// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// The start's window (2026-11-23): from the start until START_SETTLE_MS after the start's first frame was over (the
// second frame call: the door's call comes first in the cards unit's frame, so by then every pack has had its first
// frame), every message that reads or writes the disk (list, hash, begin, data, end, remove, commit, and abort 01) is
// answered status 13 with no payload and does nothing, as a message that comes before the start looked for its note.
// Why: twice the unit froze when a PC asked a pack's SHA-256 about 6 s into a start, before the start's first frame
// was over (what causes it is not known). The wire is protocol 3 unchanged and
// hello still says build 5: outside the window every answer is base 30's byte for byte. The clients already wait on
// 13 with no payload: the install page asks list, hash and remove again 40 times 250 ms apart, begin and abort 01
// ten times a second apart; a client of your own should do the same. A start that finishes an apply still answers 14.
//
// Step R4c of piece 12, base.pack 27: the door's build 5 (the door's unit of base 25, base 25's and base 26's, as it is)
// with its test stop behind a build switch, OP1F_DOOR_TEST_STOP. Switched on (1) this file is build 5's code; switched
// off (0, base 27's, the default below) `commit 02 k` is answered as the door answers a payload it does not know
// (status 1, the message not read): no confirm page, no note, nothing applied. The public page never sends commit 02;
// the test page sends it in its visit mode only. Every other line below is build 5's.
#ifndef OP1F_DOOR_TEST_STOP
#define OP1F_DOOR_TEST_STOP 0                               // base 27: the test stop (commit 02) is off
#endif
//
// Step S4a of piece 11, the base pack: the door, build 5, protocol 3 (OS 1.7.0+0; the wire format's protocol 3
// unchanged). The sibling of the door's unit of base 23 (build 4, proven on a unit as base 23), which stays as
// it was: every message is answered as build 4 answers it, with build 5's number in hello and in the ready line. What
// build 5 changes, each where it stands below:
//
//   the card      the confirm page is drawn as "the card": the unit's own
//                 question card widened, the words in its red band, a red key 1 and a grey key 2, the row of dots to
//                 the bolt while it works, the band grey for `nothing changed`, the red warning triangle over the dot
//                 where it stopped. The picture door.svg is made on the unit from instructions that name the unit's own
//                 pictures. Its parts are shown, hidden and moved with
//                 the base's op1f_node, op1f_show and op1f_move; the words are labels the door makes itself with the
//                 firmware's label maker and an alignment (card_label below), since the base's op1f_text_base makes
//                 every label from the left and level 4's op1f_text_set takes only a document a card made; the full
//                 counts are measured by the firmware's own box of the laid-out glyphs at the page's first frame and
//                 give way to the short form when wider than the card's line (counts_measure);
//   T-2           every complete SysEx message whose byte 5 is 0x66 is counted for the `0x66` guard, from whichever of
//                 the firmware handler's three callers it comes: the USB poll's buffer, the second parser, and
//                 Bluetooth MIDI (0x8fca1f40);
//   the note      `note_looked` is set at the end of each path of the start's look for the note, not at its first line
//                 (the review of base 23, S-2's left-over), so a message that comes while the start reads its note
//                 answers 13;
//   list          with the third byte's bit 1 (added on 2026-11-20), each
//                 entry also gives its file's header: the pack's version, its firmware id and its pack name, read from
//                 the 96 bytes of the header and nothing else, at most HEADER_CYCLES_MOST of header reading a message
//                 (fewer entries then; the page pages on). Without that bit every answer is build 4's byte for byte.
//
// What follows is build 4's description, which holds.
//
// Step S3 of piece 11, the base pack: the door, build 4, protocol 3 (OS 1.7.0+0; the wire format's protocols 1, 2 and
// 3). The sibling of the door's unit of base 22 (build 3, proven on
// a unit): every message of protocols 1 and 2 is answered as build 3 answers it, with build 4's
// values (the piece 16,384, the build 4, `abort 01` three files a message, every restart 1,500 ms after the answer with
// status 10); protocol 3 adds `remove` (a pack of user/pk noted for removal), `commit 01` (the files that wait and the
// removals noted, applied) and `commit 02 k` (the same as a test that stops after k changes), and a start that finishes
// an apply cut short.
//
// Where it runs: as build 3. The firmware's handler of complete SysEx messages, 0x8fc8daae, carries a jump to the stub
// (the door's stubs of base 23, the base's table entry with a trampoline of 8); the stub hands buffer and length to
// op1f_door_sysex. Every message runs on main_task, one at a time; `commit 01` reads the headers of user/pk there and
// changes nothing. The display thread's frame call (op1f_door_frame, from the cards unit) shows the confirm page,
// looks for the note of an apply cut short at the start, empties user/pkin/ at the start, ends a transfer left open,
// and, after T1 on an apply's page, applies by itself (the plan's 4.1): one step a frame, a slice of 16,384 bytes
// checked again, or the note written, or one removal, or one move, so that an apply goes on whatever the page does.
//
// What it writes: under user/pkin/, as build 3, and the note user/pkin/.apply; under user/pk/, only the removal of a
// pack file that is not base.pack (pk_remove) and the move of a waiting file into user/pk/ by the firmware's rename
// (0x8c051f88, from apply_move: user/pkin/<name> to user/pk/<name>, one name that passed the loader's rule; yaffs's
// rename replaces its target in one step); and the note set aside within user/pkin/ (note_aside). The firmware's
// answers are never trusted: the sizes before and after say what the disk holds. base.pack is moved last.
//
// The frame and a message (the plan's 4.3): a message sets `busy` for its whole length and the frame does no file work
// while it is set. While the door applies or could not finish (states 6 and 7) a message makes no file call at all:
// `hello` and `status` read words, every other command answers status 14. So the files have one user at a time.
//
// The state is one word as in build 3 (IDLE, HASHING, ASKS, RESTARTS, and APPLYING, UNFINISHED) and the transfer's
// another, each change a look and a store with interrupts masked. Lines: as build 3, and for each apply asked, refused,
// begun, checked, each removal and move, its end, and the start's note; never a line a message. Nothing here runs on
// the audio thread but door_keys, which counts. Nothing is divided (the module links no runtime). Compiled for the PC
// as it is, with OP1F_HOST (for tests this tree does not hold).

#include "build_m3_card.h"
#include "build_q_quiet.h"

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef void (*MidiOutFn)(int index, const void* packets, u32 bytes);           // 0x8d4e5cb8: USB MIDI packets into the output ring
typedef void (*RestartFn)(int code, int delay_ms);                              // 0x8c0df868: records a code and a delay; the main loop restarts
typedef int (*FileOpenFn)(const char* path, const char* mode);                  // 0x8fc8a9d8: a descriptor, negative on a failure
typedef int (*FileReadFn)(void* buffer, int size, int count, int descriptor);   // 0x8fc8a9a8: the count read
typedef int (*FileCloseFn)(int descriptor);                                     // 0x8fc8a994
typedef int (*FileSeekFn)(int descriptor, u32 offset_low, u32 offset_high, int from);   // 0x8c0a3630: the new place's low word
typedef int (*FileSizeFn)(const char* path);                                    // 0x8c051f9c: -1 when there is no file
typedef void* (*DirOpenFn)(const char* path);                                   // 0x8c051e28: 0 when it cannot be opened
typedef void* (*DirNextFn)(void* handle);                                       // 0x8c0a4bd0: an entry, its name at +11; 0 at the end
typedef int (*DirCloseFn)(void* handle);                                        // 0x8c0a57d4
typedef int (*IsFolderFn)(const char* path);                                    // 0x8c052054: 1 when the path opens as a folder
typedef int (*FsOpenFn)(const char* path, const char* mode);                    // 0x8fcba988: a handle, -1 with a level-1 line
typedef int (*FsWriteFn)(const void* data, u32 size, u32 count, int handle);    // 0x8fcba8c0: the count written
typedef void (*FsCloseFn)(int handle);                                          // 0x8fcba8ac: the file flushed
typedef int (*FileRemoveFn)(const char* path);                                  // 0x8fcbefb4: yaffs's unlink's answer, never trusted
typedef int (*MakeFoldersFn)(const char* path);                                 // 0x8c05206c: the last part made only when a slash ends it
typedef long long (*FreeSpaceFn)(const char* path);                             // 0x8c0a4730: free bytes in R1:R0, -1 on a failure
typedef int (*RenameFn)(const char* from, const char* to);                      // 0x8c051f88: 1 when the rename was made, 0 when not
// Build 5, the card's words (base 19's text unit binds the same two and reads a part's paint the same way)
typedef void* (*LabelFromNodeFn)(void* screen, void* node, int align);          // 0x8fc25abc: the node replaced by a label, 0 left, 1 centred, 2 right
typedef void (*LabelTextFn)(void* label, const char* text, int width);          // 0x8fc3aa3c: the text laid out, shortened with a mark past `width`
typedef u32 (*PaintFn)(void* node);                                             // a node's table +0x14: its paint, an index into the document's paints from 1
struct LabelBox { int x_min, x_max, y_min, y_max; };                            // the laid-out glyphs' box, in 1024ths of a pixel
typedef LabelBox (*LabelBoundsFn)(void* label);                                 // 0x8fc3820c: the box returned by value (base 19's text unit)

#ifdef OP1F_HOST
#include "mock_door4.h"                             // the PC's stand-ins: the memory the door reads, the firmware's calls, a clock
#define MASK_OFF(mask) ((void)(mask = 0))
#define MASK_ON(mask) ((void)(mask))
#define DOOR_APART
#else
#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_MIDI_OUT ((MidiOutFn)0x8d4e5cb8u)
#define STOCK_RESTART ((RestartFn)0x8c0df868u)
#define STOCK_FILE_OPEN ((FileOpenFn)0x8fc8a9d8u)
#define STOCK_FILE_READ ((FileReadFn)0x8fc8a9a8u)
#define STOCK_FILE_CLOSE ((FileCloseFn)0x8fc8a994u)
#define STOCK_FILE_SEEK ((FileSeekFn)0x8c0a3630u)
#define STOCK_READ_MODE ((const char*)0x848daec0u)         // the text "rb", which te_fopen compares its mode with
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
#define STOCK_DIR_OPEN ((DirOpenFn)0x8c051e28u)
#define STOCK_DIR_NEXT ((DirNextFn)0x8c0a4bd0u)
#define STOCK_DIR_CLOSE ((DirCloseFn)0x8c0a57d4u)
#define STOCK_IS_FOLDER ((IsFolderFn)0x8c052054u)
#define STOCK_FS_OPEN ((FsOpenFn)0x8fcba988u)
#define STOCK_FS_WRITE ((FsWriteFn)0x8fcba8c0u)
#define STOCK_FS_CLOSE ((FsCloseFn)0x8fcba8acu)
#define STOCK_FILE_REMOVE ((FileRemoveFn)0x8fcbefb4u)
#define STOCK_MAKE_FOLDERS ((MakeFoldersFn)0x8c05206cu)
#define STOCK_FREE_SPACE ((FreeSpaceFn)0x8c0a4730u)
#define STOCK_RENAME ((RenameFn)0x8c051f88u)
#define STOCK_LABEL_FROM_NODE ((LabelFromNodeFn)0x8fc25abcu)
#define STOCK_LABEL_TEXT ((LabelTextFn)0x8fc3aa3cu)
#define STOCK_LABEL_BOUNDS ((LabelBoundsFn)0x8fc3820cu)
// A part's paint as the label maker asks it (the node's table, word 5), and the bytes of the document's paints
// ([document + 0x84] to [document + 0x88], twenty bytes an entry): the label maker stops the unit for a paint of 0 or
// past them, so the door asks first (base 19's text unit `made`, proven by step R1).
#define NODE_PAINT_OF(node) (((PaintFn)((*(u32**)(node))[5]))(node))
#define DOCUMENT_PAINT_BYTES(document) (((const u32*)(document))[0x88 / 4] - ((const u32*)(document))[0x84 / 4])
#define RD32(a) (*(volatile u32*)(a))
#define RD8(a) (*(volatile u8*)(a))
#define BYTES(a) ((const volatile u8*)(a))
#define MASK_OFF(mask) asm volatile("cli %0;" : "=d"(mask) : : "memory")
#define MASK_ON(mask) asm volatile("sti %0;" : : "d"(mask) : "memory")
// Out of line: the entry's fast path stays short for every other program's message, and each part is read alone.
#define DOOR_APART __attribute__((noinline))
// A time stamp is pinned where it is written: the read carries a memory clobber, which nothing is moved across.
static inline u32 cycles() {
    u32 c;
    asm volatile("%0 = CYCLES;" : "=d"(c) : : "memory");
    return c;
}
// The core's cycle counter, its 64 bits in units of 2^24 cycles (41.9 ms at 400 MHz): a clock that runs whether or not
// the display thread makes its frame call. Reading CYCLES first makes CYCLES2 the high word of that same moment.
static inline u32 slow_clock() {
    u32 low, high;
    asm volatile("%0 = CYCLES; %1 = CYCLES2;" : "=d"(low), "=d"(high) : : "memory");
    return high << 8 | low >> 24;
}
#endif

// The firmware's addresses (an enum of their own: they do not fit a signed number).
enum : u32 {
    USB_SYSEX_BUFFER = 0x848ea444u,     // where the USB parser collects a SysEx message, 0x20000 bytes
    USB_MODE = 0x8401bd68u,             // 1: the unit is a USB device
    MIDI_RING = 0x849a947cu,            // the output ring of the device mode
    KEY_MAP = 0x84985a7cu,              // the keyboard controller's map of the keys held, keys 0 to 31: its object 0x8497d840 + 0x823c
    KEY_MAP_HIGH = 0x84985a80u,         // keys 32 to 63, the object + 0x8240 (build 2 had the wrong sum 0x8497f634: result 6c)
    TRANSPORT = 0x11905a04u,            // the tape's transport word, 0 stopped (play, record, wind otherwise: inferred)
    RECORDING = 0x848ce538u,            // the L1 record function's "recording" byte
    CURRENT_MODE = 0x8400f978u,         // the mode (for the commit's line)
    CURRENT_SCREEN = 0x848d2540u,       // the screen shown (for the commit's line)
    DISK_OPEN = 0x84d25bccu,            // not 0 while the disk mode is open (the cards unit reads it too)
    POWER_POPUP_SHOWN = 0x84c9b3e4u,    // the power popup's byte (the cards unit reads it too): the totals said once more
    DOOR_OS_ID = 0x1aeb34d1u,           // the firmware's id, as a pack's header carries it (the first four bytes of the stock image's SHA-256)
    // Build 4: where the loader places the packs, the room's first byte (build_m4_loader.cpp PACK_BASE: our own
    // reservation's first byte plus the loader's 64 KiB; 1.7.3's by the port's map), and the room's bytes (PACK_ROOM).
    // The door works out where the loader would place every pack of a set, as the loader does (commit_room).
    PACK_BASE = 0x8d2f4234u,
    PACK_ROOM = 0x001f0000u
};

enum {
    RING_TAKEN_AT = 4,                  // the ring's words: [ring + 4] taken by the USB, [ring + 0x8014] written (free = [+4] + 0x2000 - [+0x8014])
    RING_WRITTEN_AT = 0x8014,
    RING_WORDS = 0x2000,
    RING_KEEP = 512,                    // words left for the firmware's own MIDI

    PROTOCOL = 1, PROTOCOL_2 = 2, PROTOCOL_3 = 3, MARK = 1, DOOR_BUILD = 5,   // build 5 (S4a): the card, T-2, the note's window
    START_SETTLE_MS = 4000,             // the start's window: no disk message taken until this long after its first frame was over
    TOTALS_MS = 300000,                 // the totals line's period: five minutes, through the quiet gate
    LONG_VISIT_MS = 60000,              // a visit that has gone on this long says so once
    KEY_EVENTS = 8,                     // T1 and T2 events taken while the page asks, kept for the frame's lines
    LARGEST = 131072,                   // the receive buffer: a message of more is lost whole
    SINK_MOST = 114681,                 // (131072 - 7) packed bytes: 16383 groups of 8, 7 bytes each
    OTHER_MOST = 64,                    // the largest message of any other command of protocol 1, on the wire
    COPY_MOST = 128,                    // protocol 2: the largest message of any command but sink and data (begin's longest is 93)
    PIECE = 16384,                      // build 4: the most bytes one `data` carries after its offset (build 3 8,192)
    DATA_MOST = 7 + (4 + PIECE) + (4 + PIECE + 6) / 7,  // a `data` message at the piece on the wire: 18,736 bytes
    LARGEST_FILE = 4194304, MARGIN = 8388608, WAITING_MOST = 32,
    START_REMOVALS = 4,                 // the start's emptying: removals a frame
    PAGE_REMOVALS = 3,                  // build 4: `abort 01`, removals a message (build 3 eight)
    REMOVALS_MOST = 32,                 // build 4: the removals that may be noted
    HEADER3 = 96, PACK_ALIGN = 32, PACKS_MOST = 32, KEPT_MOST = 64, LIST_COUNTS = 6, ENTRY_BYTES = 20, ENTRIES_MOST = 4096,
    NOTE_MOST = 24 + 40 * REMOVALS_MOST + 76 * WAITING_MOST + 4,  // the note's largest, 3,740 bytes
    NOTE_LEAST = 24 + 4, NOTE_FORMAT = 2,
    NOTE_HEAD = 24,                     // the review's N-3: the head carries the header sums of the base that wrote it and of the one it moves
    BASE_NAMES_MOST = 96, BASE_SITES_MOST = 64, MEETS_NAMES_MOST = 256, CARD_MARK = 0x44524143, MARKS_MOST = 64,
    PICTURE_TRIES = 10, PICTURE_RETRY_MS = 500, TUNNEL_SAID = 8,
    STOPPED_SHOWN_MS = 3000,            // `nothing changed` on an apply's page before it closes
    AFTER_CHANGE_MS = 5000,             // after an apply's change, no message within this: the unit restarts all the same
    TAPE_HELD_MS = 10000,               // the tape moving this long during the second check stops the apply (outcome 8)
    START_NAMES = 32,                   // names taken from the folder at a time by the start's emptying
    SLICE = 16384,                      // a hash slice (build 2: 32,768 held main_task 23 ms on the unit)
    LIST_PAGE = 8, LIST_MOST = 64, RAW_MOST = 96, RAW_NAME = 64, NAME_MOST = 39, NAME_LEAST = 6,
    PACKET_WORDS = 224,                 // build 5: the longest answer, 653 bytes on the wire, is 218 packets (build 4: 160, 424 bytes)
    OUT_BYTES = 564,                    // build 5: the longest answer's payload (list with the header: 4 + 8 x 70; build 4's list 4 + 8 x 45)
    // Build 5, list with the header (section 14): an entry's most, 6 + 39 + 4 + 4 + 1 + 16 bytes; the header reading a
    // message may take, 8 ms at 400 MHz (a small file's open, read and close is some 0.45 ms on the unit: S3's try, line
    // 35's one-slice reads of 452 and 454 us), after which the answer ends with the entries it has
    LIST_HEADER_MOST = 70, HEADER_CYCLES_MOST = 3200000,

    CMD_HELLO = 0x20, CMD_LIST = 0x21, CMD_HASH = 0x22, CMD_BEGIN = 0x23, CMD_DATA = 0x24, CMD_END = 0x25,
    CMD_REMOVE = 0x26, CMD_ABORT = 0x27, CMD_COMMIT = 0x28, CMD_STATUS = 0x29, CMD_SINK = 0x2a,
    ANSWER = 0x40,
    DONE = 0, BAD = 1, REFUSED = 2, NO_ROOM = 3, NO_SUCH = 4, ORDER = 5, DIFFERS = 6, ASKING = 7, CANCELLED = 8, MORE = 9,
    RESTARTING = 10, UNREADABLE = 11, TAPE = 12, NOT_READY = 13, BUSY_APPLYING = 14, NOT_TAKEN = 15,

    IDLE = 0, HASHING = 1, ASKS = 2, RESTARTS = 3,                 // the states
    APPLYING = 6, UNFINISHED = 7,                                  // build 4: the apply runs (T1, or a start that finishes); a change failed
    // The apply's own state as `status 03` says it, and its outcome
    A_NONE = 0, A_ASKING = 1, A_CHECKING = 2, A_CHANGING = 3, A_DONE = 4, A_STOPPED = 5, A_UNFINISHED = 6,
    P_NONE = 0, P_DONE = 1, P_T2 = 2, P_TIME = 3, P_ABORT = 4, P_NO_PAGE = 5, P_DIFFERS = 6, P_POWER = 7, P_TAPE = 8,
    P_NOTE = 9, P_FAILED = 10, P_TEST = 11,
    // What this start found of an apply cut short (hello field 64)
    N_NONE = 0, N_FINISHING = 1, N_ALL_MADE = 2, N_OTHER_BASE = 3, N_UNREADABLE = 4, N_FAILED = 5,
    // The apply's steps on the display thread
    STEP_NONE = 0, STEP_CHECK = 1, STEP_NOTE = 2, STEP_REMOVE = 3, STEP_MOVE = 4, STEP_END = 5,
    T_NONE = 0, T_RECEIVING = 4, T_CHECKING = 5, T_ENDING = 6,     // the transfer's states (6 is the frame's, never said: 0)
    // How a transfer ended
    X_NONE = 0, X_KEPT = 1, X_DIFFERS = 2, X_SHORT = 3, X_UNREADABLE = 4, X_WRITE = 5, X_ABORT = 6, X_BEGIN = 7, X_COMMIT = 8,
    X_VISIT = 9, X_DISK = 10, X_POWER = 11,
    // The kinds of refusal, each with its own eight lines
    R_NAME = 0, R_SIZE, R_COUNT, R_ROOM, R_FREE, R_OPEN, R_TAPE, R_DISK, R_READY, R_ORDER, R_POWER, R_KINDS,
    // Build 4: the kinds of an apply's refusal (line 37), each with its own eight lines
    Q_POWER = 0, Q_READY, Q_DISK, Q_TAPE, Q_NOTHING, Q_NOT_TAKEN, Q_COUNT, Q_ROOM, Q_FREE, Q_TUNNEL, Q_ENTRIES, Q_BASE_OUT, Q_FOLDER, Q_KINDS,
    // The waiting folder at the start
    F_EMPTYING = 0, F_READY = 1, F_FAILED = 2,
    ENDED_T1 = 1, ENDED_T2 = 2, ENDED_TIME = 3, ENDED_ABORT = 4, ENDED_NO_PAGE = 5, ENDED_NO_MESSAGE = 6, ENDED_NO_RESTART = 7,

    VISIT_MS = 10000, ARMED_MS = 500, ASK_MS = 60000, AFTER_T1_MS = 10000, CANCELLED_SHOWN_MS = 1000, RESTART_WAIT_MS = 15000,
    // The guard in the SysEx entry (mend 3), on slow_clock(): 75 s of asking and 20 s after the restart was asked, in
    // units of 2^24 cycles at 400 MHz (75 x 400,000,000 / 16,777,216 = 1788.1; 20 s = 476.8), so the frame call's own
    // limits (60 s, 15 s) normally come first.
    GUARD_ASK_UNITS = 1789, GUARD_RESTART_UNITS = 477,
    BIT_T1 = 1 << 6, BIT_T2 = 1 << 7, KEY_T1 = 6,
    RESTART_CODE = 20, RESTART_DELAY_MS = 1500,     // build 4: every restart 1,500 ms after the answer with status 10 (build 3 200)
    FAULTS_SAID = 8, DIR_NAME_AT = 11
};
static const u32 LONGEST = 0x0fffffffu;                // a time is counted as two thirds of a second at most

static const char LOG_FILE[] = "../src/main/op1f_door.cpp";
static const char FOLDER[] = "user/pk";                // listed without the slash, as the firmware's folder calls take it
static const char PREFIX[] = "user/pk/";
static const char PICTURE[] = "door.svg";
static const char PKIN_FOLDER[] = "user/pkin";          // the waiting folder, listed without the slash
static const char PKIN_MAKE[] = "user/pkin/";           // made with the slash: the folder maker makes the last part only so
static const char PKIN_PREFIX[] = "user/pkin/";
static const char WRITE_MODE[] = "w+b";                 // the file class's open compares it with its own table: a new file, written
static const char FREE_PATH[] = "/yaffs2";              // the device both stock callers of the free space hand it (0x84012340's text)
// Build 4: the note of an apply, in the waiting folder (its name begins with a dot: `list` skips it, `begin` refuses
// such a name, the loader never reads user/pkin), and the two names it is set aside under.
static const char NOTE_NAME[] = ".apply";
static const char NOTE_PATH[] = "user/pkin/.apply";
static const char NOTE_BAD_PATH[] = "user/pkin/.apply-bad";
static const char NOTE_FAILED_PATH[] = "user/pkin/.apply-failed";
static const char BASE_FILE[] = "base.pack";
static const char BASE_NAME[] = "base";

// The mark of a base with the door (study section 11): no pack needs it today; a page or a pack of later steps can.
extern "C" const u32 op1f_door_1 OP1F_NAMED("op1f_door_1");
const u32 op1f_door_1 = 1;

extern "C" const char op1f_release[16] OP1F_NAMED("op1f_release");                  // the set's release (base 18's release text)
// The base this door was built into: its version and the recipe it was made from, written into a source of the base's
// own from its recipe, so that they are the recipe's and not this unit's.
extern "C" const u32 op1f_door_base_version OP1F_NAMED("op1f_door_base_version");
extern "C" const char op1f_door_base_from[] OP1F_NAMED("op1f_door_base_from");
extern "C" unsigned op1f_ms() OP1F_NAMED("op1f_ms");                                // the cards unit's clock, milliseconds
extern "C" void* op1f_base_picture(const char* name) OP1F_NAMED("op1f_base_picture");  // base 18's picture loader
// Build 5: the card's parts are the base's helpers op1f_node, op1f_show and op1f_move (build_m3_card.h, the cards unit),
// which take any document, the door's own too.
extern "C" void op1f_top_open(void* document, int (*keys)(const Op1fInput* e)) OP1F_NAMED("op1f_top_open");    // base 18's cards unit
extern "C" void op1f_top_close() OP1F_NAMED("op1f_top_close");

// Build 2, mend A: the door's own copy of the keyboard controller's map, written by the stub at the map handler's store
// (the door's stubs of base 18, op1f_door_keymap_entry): [0] keys 0 to 31, [1] keys 32 to 63, [2] the object the firmware
// stores at (P1: the map at P1 + 0x823c), [3] a count of the stores the stub saw. Written on the thread that handles
// the controller's report (the audio thread, mostly), read here a word at a time. Before the first report of a start
// the count is 0 and the copy is not known: nothing is taken from it then.
extern "C" volatile u32 op1f_door_keys[4] OP1F_NAMED("op1f_door_keys");
enum { KEYS_LOW = 0, KEYS_HIGH = 1, KEYS_OBJECT = 2, KEYS_COUNT = 3, MAP_AT_OBJECT = 0x823c };

// ---------------------------------------------------------------- the state shared by the two threads

static volatile u32 state;                              // IDLE, HASHING, ASKS, RESTARTS
static volatile u32 outcome;                            // how the last commit ended, kept until the next commit
static volatile u32 outcome_unsaid;                     // 1 until a status or hello said a cancel
static volatile u32 last_message_ms;                    // the clock at the last message of the door's
static volatile u32 visit_on;                           // a visit was begun and its end not said
static volatile u32 visit_started_ms;                   // build 2: the clock when the visit began
static u32 long_said;                                   // build 2: the visit that runs has said its minute
static volatile u32 t1_ms;                              // the clock when T1 was seen
static volatile u32 seconds_left;                       // while asking, for the status answer

// A change of state from `from` to `to`, made only when the state is still `from`: 1 when it was made.
static int change(u32 from, u32 to) {
    u32 mask;
    int made = 0;
    MASK_OFF(mask);
    if (state == from) {
        state = to;
        made = 1;
    }
    MASK_ON(mask);
    return made;
}

// ---------------------------------------------------------------- counts, for the status answer and the visit's lines

static struct {
    u32 messages, answers, unsent, dropped;            // since the start (the status answer)
    u32 not_f0, visits;                                 // build 2: for the totals line
    u32 last_command, last_number, last_length;
} total;

static struct {
    u32 messages, hello, list, hash, slices, files, sink, sink_bytes, commit, status, abort;
    u32 sent, unsent, longest, slowest_us, slowest_command, dropped, not_f0, refused;
} visit;

static volatile u32 keys_taken, turns_taken, t1_not_held;   // door_keys, while the page is up (the input threads)
// Build 2: the T1 and T2 events door_keys took, with the door's copy at that moment, for the frame's lines (written
// by the input threads, the count last; read and printed by the display thread).
static volatile u32 ev_key[KEY_EVENTS], ev_what[KEY_EVENTS], ev_word[KEY_EVENTS], ev_reports[KEY_EVENTS];
static volatile u32 ev_count;
static u32 faults_unsent = FAULTS_SAID, faults_unread = FAULTS_SAID, faults_hash = FAULTS_SAID;

static inline u32 microseconds(u32 c) {
    if (c > LONGEST) c = LONGEST;
    return ((c >> 4) * 41u) >> 10;
}

// ---------------------------------------------------------------- the packing (7 in 8) and the answer

static u32 packets[PACKET_WORDS];
static u32 packet_count, held[3], held_count;
static u8 out[OUT_BYTES];

static void put(u32 byte) {
    if (held_count == 3) {
        if (packet_count < PACKET_WORDS) packets[packet_count++] = 4u | held[0] << 8 | held[1] << 16 | held[2] << 24;
        held_count = 0;
    }
    held[held_count++] = byte;
}

static void put_end() {
    u32 word = 4u + held_count;                          // 5, 6 or 7: the message's last one, two or three bytes
    for (u32 i = 0; i < held_count; ++i) word |= held[i] << (8 * (i + 1));
    if (packet_count < PACKET_WORDS) packets[packet_count++] = word;
    held_count = 0;
}

static void store32(u8* at, u32 value) {
    at[0] = (u8)value;
    at[1] = (u8)(value >> 8);
    at[2] = (u8)(value >> 16);
    at[3] = (u8)(value >> 24);
}

static u32 load32(const u8* at) {
    return at[0] | (u32)at[1] << 8 | (u32)at[2] << 16 | (u32)at[3] << 24;
}

static u32 load32_be(const u8* at) {                    // a fingerprint's first four bytes, as a line prints them
    return (u32)at[0] << 24 | (u32)at[1] << 16 | (u32)at[2] << 8 | at[3];
}

// The answer: the head, the status, `count` bytes of `out` packed, the end; handed to the output when the ring has
// room for it and the firmware's share. 1 when it was handed over.
static int answer(u32 command, u32 number, u32 status, u32 count) {
    packet_count = 0;
    held_count = 0;
    put(0xf0); put(0x7d); put(0x4f); put(0x46);
    put(command | ANSWER);
    put(number);
    put(status);
    for (u32 at = 0; at < count; at += 7) {
        u32 group = count - at < 7 ? count - at : 7, tops = 0;
        for (u32 i = 0; i < group; ++i) tops |= (u32)(out[at + i] >> 7) << i;
        put(tops);
        for (u32 i = 0; i < group; ++i) put(out[at + i] & 0x7f);
    }
    put(0xf7);
    put_end();
    u32 room = RD32(MIDI_RING + RING_TAKEN_AT) + RING_WORDS - RD32(MIDI_RING + RING_WRITTEN_AT);
    if (RD32(USB_MODE) != 1 || room > RING_WORDS || packet_count + RING_KEEP >= room) {
        ++total.unsent;
        ++visit.unsent;
        if (faults_unsent) {
            --faults_unsent;
            STOCK_LOG(1, LOG_FILE, 9, "door: an answer of %u words is not sent: %u words free in the MIDI output, USB mode %u (%u not sent)",
                      packet_count, room, RD32(USB_MODE), total.unsent);
        }
        return 0;
    }
    STOCK_MIDI_OUT(0, packets, packet_count * 4);           // the glue copies and starts the transfer itself, as for the firmware's own answers
    ++total.answers;
    ++visit.sent;
    return 1;
}

// A packed payload unpacked: the bytes, or -1 when it is not packed bytes (a byte with its top bit; a head with a
// bit for a byte its group has not; a head alone at the end) or more than `room`.
static int unpack(const volatile u8* in, u32 count, u8* to, u32 room) {
    u32 at = 0;
    while (count > 0) {
        u32 tops = in[0];
        u32 group = count - 1 < 7 ? count - 1 : 7;
        if (group == 0 || (tops & 0x80) || (tops >> group) != 0) return -1;
        for (u32 i = 0; i < group; ++i) {
            u32 byte = in[1 + i];
            if ((byte & 0x80) || at >= room) return -1;
            to[at++] = (u8)(byte | (((tops >> i) & 1) << 7));
        }
        in += 1 + group;
        count -= 1 + group;
    }
    return (int)at;
}

// ---------------------------------------------------------------- CRC-32 (zlib's) and SHA-256, of our own

static u32 crc_table[256];
static u32 crc_made;

static void crc_make() {
    for (u32 n = 0; n < 256; ++n) {
        u32 c = n;
        for (u32 k = 0; k < 8; ++k) c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
        crc_table[n] = c;
    }
    crc_made = 1;
}

static const u32 SHA_K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

struct Sha {
    u32 h[8];
    u8 block[64];
    u32 used;                                           // bytes in `block`
    u32 bytes;                                          // bytes hashed in all
};

static inline u32 rotr(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }

static void sha_start(Sha* s) {
    static const u32 H0[8] = { 0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au, 0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u };
    for (u32 i = 0; i < 8; ++i) s->h[i] = H0[i];
    s->used = 0;
    s->bytes = 0;
}

static u32 sha_w[64];                                   // main_task's, and the frame's in the apply's states, when no message hashes (4.3)

static DOOR_APART void sha_block(Sha* s, const u8* p) {
    for (u32 i = 0; i < 16; ++i) sha_w[i] = (u32)p[4 * i] << 24 | (u32)p[4 * i + 1] << 16 | (u32)p[4 * i + 2] << 8 | p[4 * i + 3];
    for (u32 i = 16; i < 64; ++i) {
        u32 a = sha_w[i - 15], b = sha_w[i - 2];
        u32 s0 = rotr(a, 7) ^ rotr(a, 18) ^ (a >> 3), s1 = rotr(b, 17) ^ rotr(b, 19) ^ (b >> 10);
        sha_w[i] = sha_w[i - 16] + s0 + sha_w[i - 7] + s1;
    }
    u32 a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4], f = s->h[5], g = s->h[6], h = s->h[7];
    for (u32 i = 0; i < 64; ++i) {
        u32 t1 = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + SHA_K[i] + sha_w[i];
        u32 t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d; s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

static void sha_add(Sha* s, const u8* p, u32 n) {
    s->bytes += n;
    while (n > 0) {
        if (s->used == 0 && n >= 64) {
            sha_block(s, p);
            p += 64;
            n -= 64;
            continue;
        }
        u32 take = 64 - s->used < n ? 64 - s->used : n;
        for (u32 i = 0; i < take; ++i) s->block[s->used + i] = p[i];
        s->used += take;
        p += take;
        n -= take;
        if (s->used == 64) {
            sha_block(s, s->block);
            s->used = 0;
        }
    }
}

static void sha_end(Sha* s, u8* digest) {
    u32 bits_high = s->bytes >> 29, bits_low = s->bytes << 3;
    s->block[s->used++] = 0x80;
    if (s->used > 56) {
        while (s->used < 64) s->block[s->used++] = 0;
        sha_block(s, s->block);
        s->used = 0;
    }
    while (s->used < 56) s->block[s->used++] = 0;
    for (u32 i = 0; i < 4; ++i) s->block[56 + i] = (u8)(bits_high >> (24 - 8 * i));
    for (u32 i = 0; i < 4; ++i) s->block[60 + i] = (u8)(bits_low >> (24 - 8 * i));
    sha_block(s, s->block);
    for (u32 i = 0; i < 8; ++i)
        for (u32 k = 0; k < 4; ++k) digest[4 * i + k] = (u8)(s->h[i] >> (24 - 8 * k));
}

// ---------------------------------------------------------------- build 4: a pack file checked as the loader checks it

// zlib's CRC-32 register carried over `n` bytes (started at 0xffffffff, inverted at the end).
static u32 crc_add(u32 crc, const u8* p, u32 n) {
    for (u32 i = 0; i < n; ++i) crc = crc_table[(crc ^ p[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

// What a file is checked for, read slice by slice from its first byte: its SHA-256, its first 96 bytes (a pack's header
// of format 3, build_m4_loader.cpp Header3) and the CRC-32 over its body and lists, the bytes the loader sums.
enum {
    H_FORMAT = 2, H_HEADER = 3, H_OS = 4, H_LINK = 5, H_BODY = 6, H_COMPARE = 7, H_ZEROED = 8, H_CRC = 9, H_NAME = 10,
    H_TABLE = 14, H_TABLE_END = 15, H_LISTS = 16, H_EXTRA = 17, H_VERSION = 18, H_ALIGN = 20, H_PATH = 22, H_CARD = 23
};
struct Check {
    Sha sha;
    u32 crc;
    u32 head[HEADER3 / 4];
    u32 have_head;
    u8 card[8], counts[24], last;                       // the review's S-4: the card's two words, the lists' six counts, the strings' last byte
    u32 got_card, got_counts, got_last;
};

static void check_start(Check* c) {
    if (!crc_made) crc_make();
    sha_start(&c->sha);
    c->crc = 0xffffffffu;
    c->have_head = 0;
    c->got_card = c->got_counts = c->got_last = 0;
}

// The bytes of [from, from + n) of a file that lie in the `len` bytes read at `at`, kept; `got` counts them.
static void grab(u8* to, u32* got, u32 from, u32 n, const u8* p, u32 at, u32 len) {
    u32 lo = from > at ? from : at, end_a = from + n, end_b = at + len, hi = end_a < end_b ? end_a : end_b;
    for (u32 x = lo; x < hi; ++x) {
        to[x - from] = p[x - at];
        ++*got;
    }
}

// `n` bytes of the file, read from `at`, added: hashed, the header kept from the first slice, the CRC-32 summed over the
// part of them that lies in [96, 96 + body + lists).
static void check_add(Check* c, const u8* p, u32 at, u32 n) {
    sha_add(&c->sha, p, n);
    if (at == 0 && n >= (u32)HEADER3) {
        for (u32 w = 0; w < (u32)HEADER3 / 4; ++w) c->head[w] = load32(p + 4 * w);
        c->have_head = 1;
    }
    if (!c->have_head) return;
    u32 from = HEADER3, to = HEADER3 + c->head[H_BODY] + c->head[H_LISTS];
    if (to < from || c->head[H_BODY] > to || c->head[H_LISTS] > to) to = 0xffffffffu;   // a sum past 4 GB: as far as the file goes
    u32 lo = at > from ? at : from, hi = at + n < to ? at + n : to;
    if (hi > lo) c->crc = crc_add(c->crc, p + (lo - at), hi - lo);
    u32 body = c->head[H_BODY], lists = c->head[H_LISTS];
    if (c->head[H_CARD] != 0) grab(c->card, &c->got_card, HEADER3 + c->head[H_CARD], 8, p, at, n);
    grab(c->counts, &c->got_counts, HEADER3 + body, 24, p, at, n);
    if (lists != 0) grab(&c->last, &c->got_last, HEADER3 + body + lists - 1, 1, p, at, n);
}

// The review's S-4: what load3 holds of a pack's card and lists once it has read them (build_m4_loader.cpp: WHY_CARD,
// WHY_LISTS), as far as the card's two words, the six counts and the strings' last byte say: 1 when they hold.
static int card_and_lists_hold(const Check* c) {
    const u32* h = c->head;
    u32 body = h[H_BODY], lists = h[H_LISTS];
    if (h[H_CARD] != 0) {
        if (c->got_card != 8) return 0;
        u32 mark = load32(c->card), bytes = load32(c->card + 4);
        if (mark != (u32)CARD_MARK || (bytes & 3) != 0 || bytes < 8 || bytes > body - h[H_CARD]) return 0;
    }
    if (c->got_counts != 24) return 0;
    u32 works = load32(c->counts), exports = load32(c->counts + 4), imports = load32(c->counts + 8);
    u32 places = load32(c->counts + 12), marks = load32(c->counts + 16), strings = load32(c->counts + 20);
    if (works > lists || exports > lists || imports > lists || places > lists || marks > lists || strings > lists || imports > 0xfffeu) return 0;
    u32 fixed = 4 * LIST_COUNTS + 12 * works + 8 * exports + 8 * imports + 12 * places + 8 * marks;
    if (fixed > lists || fixed + strings != lists || (strings & 3) != 0) return 0;
    if (works + exports + imports > 0 && (strings == 0 || c->got_last != 1 || c->last != 0)) return 0;
    return marks <= (u32)MARKS_MOST;
}

static int same_name(const char* a, const char* b);

// The pack's own name from a header, as the loader takes it (printable bytes, the others zero, sixteen at most).
static void pack_name_of(const u32* head, char* name) {
    for (u32 i = 0; i < 16; ++i) {
        u8 c = (u8)(head[H_NAME + (i >> 2)] >> (8 * (i & 3)));
        name[i] = c >= 0x20 && c < 0x7f ? (char)c : 0;
    }
    name[16] = 0;
}

// What a pack takes of the room, the loader's foot: its body and the larger of its lists and its zeroed data.
static u32 foot_of(const u32* head) {
    u32 behind = head[H_LISTS] > head[H_ZEROED] ? head[H_LISTS] : head[H_ZEROED];
    return head[H_BODY] + behind;
}

// The header's fields as the loader holds them before it reads a pack (build_m4_loader.cpp look and load3): 1 when the
// loader would read the file as a pack of format 3 for this firmware, of the size the header gives.
static int header_takes(const u32* head, u32 size) {
    if (size < (u32)HEADER3 || head[0] != 0x4631504fu || head[1] != 0x4b434150u) return 0;      // "OP1FPACK"
    if (head[H_FORMAT] != 3 || head[H_HEADER] != (u32)HEADER3 || head[H_OS] != DOOR_OS_ID) return 0;
    return size == (u32)HEADER3 + head[H_BODY] + head[H_LISTS] + head[H_EXTRA];
}

// The rest of load3's tests of a header, which need nothing but the header: 1 when they pass.
static int header_holds(const u32* head) {
    u32 align = head[H_ALIGN], body = head[H_BODY], lists = head[H_LISTS];
    int align_ok = align == 1 || align == 2 || align == 4 || align == 8 || align == 16 || align == (u32)PACK_ALIGN;
    if ((head[H_LINK] & 3) != 0 || !align_ok || body == 0 || (body & 3) != 0 || body > (u32)PACK_ROOM || head[H_ZEROED] > (u32)PACK_ROOM ||
        lists < 4u * LIST_COUNTS || (lists & 3) != 0 || lists > (u32)PACK_ROOM || head[H_EXTRA] > 0x40000000u ||
        head[H_COMPARE] == 0 || head[H_COMPARE] > body ||
        head[H_TABLE] < head[H_LINK] || head[H_TABLE_END] < head[H_TABLE] || head[H_TABLE_END] - head[H_LINK] > head[H_COMPARE] ||
        (head[H_TABLE] & 3) != 0 || head[H_TABLE_END] - head[H_TABLE] > (u32)ENTRIES_MOST * ENTRY_BYTES ||
        (head[H_PATH] != 0 && ((head[H_PATH] & 3) != 0 || body < 48u || head[H_PATH] > body - 48u)))
        return 0;
    for (u32 at = head[H_TABLE]; at != head[H_TABLE_END]; at += ENTRY_BYTES) {    // a whole number of entries, without a division
        if (head[H_TABLE_END] - at < (u32)ENTRY_BYTES) return 0;
    }
    u32 card = head[H_CARD];
    if (card != 0 && ((card & 3) != 0 || body < 8u || card > body - 8u)) return 0;
    return 1;
}

// Why the loader would not take a file that waits (status 15): 0 when it would, else the
// reason: 1 not a pack of format 3 (its magic, its format, its header's size, or another of the loader's tests of its
// header), 2 made for another firmware, 3 its size not the header's, 4 its header's sum does not hold, 5 the pack name
// `base` on another file, 6 base.pack not named `base`, 7 its card or its lists would not load (the review's S-4).
// `name` is the file's name, `c` the whole file checked.
static u32 take_reason(const char* name, u32 size, const Check* c, u32 crc) {
    const u32* h = c->head;
    if (!c->have_head || size < (u32)HEADER3 || h[0] != 0x4631504fu || h[1] != 0x4b434150u || h[H_FORMAT] != 3 || h[H_HEADER] != (u32)HEADER3) return 1;
    if (h[H_OS] != DOOR_OS_ID) return 2;
    if (size != (u32)HEADER3 + h[H_BODY] + h[H_LISTS] + h[H_EXTRA]) return 3;
    if (!header_holds(h)) return 1;
    if (crc != h[H_CRC]) return 4;
    if (!card_and_lists_hold(c)) return 7;
    char pack[17];
    pack_name_of(h, pack);
    int base_name = same_name(pack, BASE_NAME), base_file = same_name(name, BASE_FILE);
    if (base_name && !base_file) return 5;
    if (base_file && !base_name) return 6;
    return 0;
}

// ---------------------------------------------------------------- names (the loader's rule, build_m4_loader.cpp op1f_base_pack_name)

static int pack_name(const char* name) {
    u32 n = 0;
    while (name[n]) {
        u32 c = (u8)name[n];
        if (c < 0x20 || c > 0x7e || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') return 0;
        if (++n > NAME_MOST) return 0;
    }
    if (n < NAME_LEAST || name[0] == '.' || name[0] == ' ') return 0;
    return name[n - 5] == '.' && (name[n - 4] | 0x20) == 'p' && (name[n - 3] | 0x20) == 'a' && (name[n - 2] | 0x20) == 'c' && (name[n - 1] | 0x20) == 'k';
}

// A prefix and a name of at most RAW_NAME - 1 bytes, ended: `path` holds PATH_BYTES bytes.
enum { PATH_BYTES = 16 + RAW_NAME };
static void path_in(char* path, const char* prefix, const char* name) {
    u32 at = 0;
    for (; prefix[at]; ++at) path[at] = prefix[at];
    for (u32 i = 0; name[i] && i < RAW_NAME - 1; ++i) path[at++] = name[i];
    path[at] = 0;
}

static void path_of(char* path, const char* name) { path_in(path, PREFIX, name); }   // user/pk/: read only

// The one way to a path the door writes, removes or makes a file at: user/pkin/ and one name of 1 to RAW_NAME - 1
// bytes with no separator, not "." or "..". 1 when the path was made; 0 (and an empty path) when the name is not one.
static DOOR_APART int pkin_path(char* path, const char* name) {
    path[0] = 0;
    u32 n = 0;
    for (; name[n]; ++n) {
        if (n >= RAW_NAME - 1 || name[n] == '/' || name[n] == '\\' || name[n] == ':') return 0;
    }
    if (n == 0 || (name[0] == '.' && (n == 1 || (n == 2 && name[1] == '.')))) return 0;
    path_in(path, PKIN_PREFIX, name);
    return 1;
}

// ---------------------------------------------------------------- hash

static u8 slice[SLICE];                                 // main_task's, and the frame's in the apply's states, when no message reads one
static struct {
    char name[NAME_MOST + 1];
    char path[PATH_BYTES];
    u32 size, done;
    Sha sha;
} hashing;

static void hash_fault(const char* why, u32 now) {
    if (faults_hash) {
        --faults_hash;
        STOCK_LOG(1, LOG_FILE, 10, "door: the hash of %s stopped: %s (%u at the start, %u now, %u done)", hashing.path, why, hashing.size, now, hashing.done);
    }
}

// One slice: read from `done` and hashed. The status: MORE, DONE (with the digest), or UNREADABLE. `out` holds the
// payload; `bytes` its length.
static DOOR_APART u32 hash_slice(u32* bytes) {
    int size = STOCK_FILE_SIZE(hashing.path);
    u32 read_cycles = 0, hash_cycles = 0;
    if (size < 0 || (u32)size != hashing.size) {
        hash_fault(size < 0 ? "it is gone" : "its size changed", (u32)size);
        change(HASHING, IDLE);
        store32(out, hashing.size);
        store32(out + 4, hashing.done);
        *bytes = 8;
        return UNREADABLE;
    }
    u32 want = hashing.size - hashing.done < SLICE ? hashing.size - hashing.done : SLICE;
    if (want > 0) {
        u32 started = cycles();
        int descriptor = STOCK_FILE_OPEN(hashing.path, STOCK_READ_MODE);
        int got = -1;
        if (descriptor >= 0) {
            int at = STOCK_FILE_SEEK(descriptor, hashing.done, 0, 0);
            if ((u32)at == hashing.done) got = STOCK_FILE_READ(slice, 1, (int)want, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
        read_cycles = cycles() - started;
        if (got < 0 || (u32)got != want) {
            hash_fault(descriptor < 0 ? "it could not be opened" : "a slice could not be read whole", (u32)size);
            change(HASHING, IDLE);
            store32(out, hashing.size);
            store32(out + 4, hashing.done);
            *bytes = 8;
            return UNREADABLE;
        }
        started = cycles();
        sha_add(&hashing.sha, slice, want);
        hash_cycles = cycles() - started;
        hashing.done += want;
    }
    store32(out, hashing.size);
    store32(out + 4, hashing.done);
    store32(out + 8, read_cycles);
    store32(out + 12, hash_cycles);
    *bytes = 16;
    ++visit.slices;
    if (hashing.done < hashing.size) return MORE;
    sha_end(&hashing.sha, out + 16);
    *bytes = 48;
    change(HASHING, IDLE);
    return DONE;
}

static DOOR_APART u32 hash(const u8* args, int count, u32* bytes) {
    *bytes = 0;
    if (count < 1) return BAD;
    if (args[0] == 0) {                                 // start: the name to the payload's end
        u32 n = (u32)count - 1;
        if (n > NAME_MOST) return REFUSED;
        char name[NAME_MOST + 1];
        for (u32 i = 0; i < n; ++i) name[i] = (char)args[1 + i];
        name[n] = 0;
        for (u32 i = 0; i < n; ++i) {
            if (name[i] == 0) return REFUSED;           // a zero inside: not a name the loader takes
        }
        if (!pack_name(name)) return REFUSED;
        for (u32 i = 0; i <= n; ++i) hashing.name[i] = name[i];
        path_of(hashing.path, name);
        if (state == HASHING) change(HASHING, IDLE);    // a new start ends the hash in progress
        hashing.done = 0;
        int size = STOCK_FILE_SIZE(hashing.path);
        ++visit.files;
        if (size < 0) {
            hashing.size = 0xffffffffu;
            store32(out, 0xffffffffu);
            store32(out + 4, 0);
            *bytes = 8;
            return UNREADABLE;
        }
        hashing.size = (u32)size;
        sha_start(&hashing.sha);
        if (!change(IDLE, HASHING)) return ASKING;     // the state moved meanwhile (it cannot from main_task's own view)
        return hash_slice(bytes);
    }
    if (args[0] == 1) {                                 // next: the bytes hashed so far
        if (count != 5) return BAD;
        if (state != HASHING) return ORDER;
        u32 offset = load32(args + 1);
        if (offset != hashing.done) {
            store32(out, hashing.size);
            store32(out + 4, hashing.done);
            store32(out + 8, 0);
            store32(out + 12, 0);
            *bytes = 16;
            return ORDER;
        }
        return hash_slice(bytes);
    }
    return BAD;
}

// ---------------------------------------------------------------- the waiting files and the transfer (protocol 2)

// The files received and checked since the start: they wait in user/pkin/. Written on main_task, and read by the frame
// only in the apply's states, when no message touches them. Build 4 keeps with each its SHA-256, its header and the
// reason the loader would not take it (0: it would), all from the check that kept it.
struct Waiting {
    char name[NAME_MOST + 1];
    u32 size;
    u8 sha[32];
    u32 head[HEADER3 / 4];
    u32 why;                                            // take_reason at its check
};
static Waiting waiting[WAITING_MOST];
static u32 waiting_count;

static int same_name(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static int waiting_at(const char* name) {
    for (u32 i = 0; i < waiting_count; ++i)
        if (same_name(waiting[i].name, name)) return (int)i;
    return -1;
}

static void waiting_drop(const char* name) {
    int at = waiting_at(name);
    if (at < 0) return;
    for (u32 i = (u32)at; i + 1 < waiting_count; ++i) waiting[i] = waiting[i + 1];
    --waiting_count;
}

static volatile u32 tstate;                             // T_NONE, T_RECEIVING, T_CHECKING, T_ENDING
static volatile u32 busy;                               // a message is being handled (main_task): the frame keeps its hands off
static volatile u32 folder_state = F_EMPTYING;          // the start's emptying of user/pkin/: F_EMPTYING, F_READY, F_FAILED

static struct {
    char name[NAME_MOST + 1];
    char path[PATH_BYTES];
    u32 size, received, done;
    u8 expected[32];
    int handle;                                         // open while T_RECEIVING only
    u32 outcome;                                        // the last transfer's (X_...), kept until the next begin
    u32 free_kb;                                        // read at the last begin
    u32 began_ms, pieces, write_us, slowest_us, slowest_at, close_us, slices, read_us, hash_us;
    Check check;                                        // build 4: the SHA-256, the header and the CRC-32 of the check
} xfer;

static struct {
    u32 begun, kept, removed_at_check, ended_before, refused, bytes, slowest_us;
} files_total;
static u32 refusals[R_KINDS];

// A change of the transfer's state from `from` to `to`, made only when it is still `from`: 1 when made.
static int tchange(u32 from, u32 to) {
    u32 mask;
    int made = 0;
    MASK_OFF(mask);
    if (tstate == from) {
        tstate = to;
        made = 1;
    }
    MASK_ON(mask);
    return made;
}

// A file of user/pkin/ taken off the disk. The removal's answer is never trusted: the size before and after says what
// the disk holds (as R1's op1f_remove). 1 when the file is not there afterwards; `left` the size afterwards.
static DOOR_APART int pkin_remove(const char* name, int* answered, int* left) {
    char path[PATH_BYTES];
    *answered = 0;
    *left = -1;
    if (!pkin_path(path, name)) return 0;
    if (STOCK_FILE_SIZE(path) < 0) return 1;            // not there (or a folder): nothing to take off
    if (STOCK_IS_FOLDER(path)) return 0;
    *answered = STOCK_FILE_REMOVE(path);
    *left = STOCK_FILE_SIZE(path);
    return *left < 0;
}

// ---------------------------------------------------------------- list

static char raw[RAW_MOST][RAW_NAME];
static u32 raw_long[RAW_MOST];                          // the name was longer than RAW_NAME - 1 bytes
static u32 order[RAW_MOST];

static int before(const char* a, const char* b) {      // the order of the names: byte by byte, as the loader's
    while (*a && *a == *b) { ++a; ++b; }
    return (u8)*a < (u8)*b;
}

// ---------------------------------------------------------------- build 4: the removals noted, and the apply

// The removals noted by `remove` (main_task's; read by the frame only in the apply's states, when no message touches
// them), forgotten at every start, by `remove 02`, and when an apply has made its change.
static char noted[REMOVALS_MOST][NAME_MOST + 1];
static u32 noted_count;

static void copy_name(char* to, const char* from) {
    u32 n = 0;
    for (; from[n] && n < (u32)NAME_MOST; ++n) to[n] = from[n];
    to[n] = 0;
}

static int noted_at(const char* name) {
    for (u32 i = 0; i < noted_count; ++i)
        if (same_name(noted[i], name)) return (int)i;
    return -1;
}

static int noted_drop(const char* name) {
    int at = noted_at(name);
    if (at < 0) return 0;
    for (u32 i = (u32)at; i + 1 < noted_count; ++i) copy_name(noted[i], noted[i + 1]);
    --noted_count;
    return 1;
}

// base.pack in any capitals: the base, which is never removed, only replaced, and replaced last.
static int base_file_any_case(const char* name) {
    for (u32 i = 0; i < 10; ++i) {
        u8 c = (u8)name[i];
        if (c >= 'A' && c <= 'Z') c = (u8)(c + 32);
        if (c != (u8)BASE_FILE[i]) return 0;
    }
    return 1;
}

// The apply: what commit 01 settled (or the start's note gave), worked through by the frame one step a frame.
struct Move {
    char name[NAME_MOST + 1];
    u32 size;
    u8 sha[32];
};
static struct {
    u32 astate, outcome, found;                         // A_..., P_..., N_... (status 03 and hello)
    u32 test_k;                                         // commit 02's k: a stop after k changes; 0 for a real apply
    u32 finishing;                                      // 1: the start's finishing of a note
    u32 moves_n, removals_n, new_n, replaced_n, base;
    u32 checked_files, removals_made, moves_made, changes;
    u32 to_check, checked;                              // bytes
    Move moves[WAITING_MOST];                           // in the order of their names, base.pack last
    char removals[REMOVALS_MOST][NAME_MOST + 1];        // in the order of their names
    u32 step, at, done;                                 // the step, the file it is at, the bytes of that file checked
    Check check;
    char name[NAME_MOST + 1];                           // the file the apply is at or stopped at, "" for none (copied under the mask)
    u32 began_ms, changed_ms, tape_since, slices, read_us, hash_us, note_bytes, note_us;
    u32 files_after, room_after;
    u32 base_sum, new_base_sum;                         // the review's N-3: the header sums of the base.pack that runs and of the one it moves
} apply;
static u32 restart_from_apply;                          // the restart to come follows an apply (5 s with no message: restart all the same)
static u32 note_looked;                                 // the start has looked for a note
static volatile u32 start_settled;                      // the display thread's: 1 from START_SETTLE_MS after the start's first frame was over, never 0 again
static u32 door_frames, first_frame_over_ms;            // the display thread's: the frame calls counted to 2, and the ms of the second
static volatile u32 window_refused;                     // main_task's: the messages answered 13 by the start's window
static const char* aside_name;                          // a note set aside at this start: the emptying leaves it
static u8 note_bytes[NOTE_MOST];                        // the note, made in place (the frame's)
static volatile u32 tunnel_count, tunnel_said;          // the review's S-1: messages of the firmware's 0x66 service since the start
static volatile u32 tunnel_other;                       // build 5's T-2: of them, those not in the USB poll's buffer (Bluetooth MIDI, the second parser)
static u8 tunnel_first[6];
static volatile u32 tunnel_length;
static u32 picture_tries, picture_ms;                   // the review's N-5: the picture asked again at a start that finishes

static void apply_name(const char* name) {              // the name status 03 says, written whole with interrupts masked
    u32 mask;
    MASK_OFF(mask);
    copy_name(apply.name, name);
    MASK_ON(mask);
}

// user/pkin/<name> and user/pk/<name> for a name that passes the loader's rule: the one way to a move's two paths.
static DOOR_APART int move_paths(const char* name, char* from, char* to) {
    from[0] = to[0] = 0;
    if (!pack_name(name)) return 0;
    if (!pkin_path(from, name)) return 0;
    path_in(to, PREFIX, name);
    return 1;
}

// A pack of user/pk taken off the disk (an apply's removal): never base.pack, in any capitals, and only a name that
// passes the loader's rule. As pkin_remove, the sizes before and after say what the disk holds: 1 when the file is not
// there afterwards (or was not there before).
static DOOR_APART int pk_remove(const char* name, int* answered, int* size_before, int* left) {
    char path[PATH_BYTES];
    *answered = 0;
    *size_before = -1;
    *left = -1;
    if (!pack_name(name) || base_file_any_case(name)) return 0;
    path_in(path, PREFIX, name);
    *size_before = STOCK_FILE_SIZE(path);
    if (*size_before < 0) return 1;                     // not there (or a folder): nothing to take off
    if (STOCK_IS_FOLDER(path)) return 0;
    *answered = STOCK_FILE_REMOVE(path);
    *left = STOCK_FILE_SIZE(path);
    return *left < 0;
}

// A waiting file moved into user/pk by the firmware's rename, which replaces a file of that name in one step (yaffs's
// shadowed rename). The answer is never trusted either way: the move is made when the
// source is gone and the target has the size the apply holds for it. A source already gone (a start that finishes)
// is not renamed: its move is judged by the target alone.
static DOOR_APART int apply_move(const Move* m, int* answered, int* target_before, int* source_after, int* target_after) {
    char from[PATH_BYTES], to[PATH_BYTES];
    *answered = 0;
    *target_before = *source_after = *target_after = -1;
    if (!move_paths(m->name, from, to)) return 0;
    *target_before = STOCK_FILE_SIZE(to);
    if (STOCK_FILE_SIZE(from) >= 0) *answered = STOCK_RENAME(from, to);
    *source_after = STOCK_FILE_SIZE(from);
    *target_after = STOCK_FILE_SIZE(to);
    return *source_after < 0 && *target_after >= 0 && (u32)*target_after == m->size;
}

// The note set aside within the waiting folder, under one of its two fixed names: renamed, and removed when the rename
// did not take it away. 1 when it stands under the new name.
static DOOR_APART int note_aside(const char* to) {
    int answered = STOCK_RENAME(NOTE_PATH, to);
    (void)answered;
    if (STOCK_FILE_SIZE(NOTE_PATH) < 0 && STOCK_FILE_SIZE(to) >= 0) return 1;
    int removal_answered = 0, left = -1;
    pkin_remove(NOTE_NAME, &removal_answered, &left);
    return 0;
}

// The note (format 2 since the review's N-3): OFAP, the format, this base's version, the
// firmware's id, R, M, the base among the moves, commit 02's k, the header sum of the base.pack that runs, the header
// sum of the base.pack the apply moves (0 when none); R names of 40 bytes; M moves of a name of 40, the size, the
// SHA-256; the CRC-32 of all before.
static u32 note_make() {
    u8* p = note_bytes;
    u32 at = 0;
    p[0] = 'O'; p[1] = 'F'; p[2] = 'A'; p[3] = 'P';
    p[4] = (u8)NOTE_FORMAT; p[5] = 0;
    p[6] = (u8)op1f_door_base_version; p[7] = (u8)(op1f_door_base_version >> 8);
    store32(p + 8, DOOR_OS_ID);
    p[12] = (u8)apply.removals_n;
    p[13] = (u8)apply.moves_n;
    p[14] = (u8)apply.base;
    p[15] = (u8)apply.test_k;
    store32(p + 16, apply.base_sum);
    store32(p + 20, apply.base ? apply.new_base_sum : 0u);
    at = NOTE_HEAD;
    for (u32 i = 0; i < apply.removals_n; ++i, at += 40) {
        for (u32 c = 0; c < 40; ++c) p[at + c] = 0;
        for (u32 c = 0; c < (u32)NAME_MOST && apply.removals[i][c]; ++c) p[at + c] = (u8)apply.removals[i][c];
    }
    for (u32 i = 0; i < apply.moves_n; ++i, at += 76) {
        for (u32 c = 0; c < 40; ++c) p[at + c] = 0;
        for (u32 c = 0; c < (u32)NAME_MOST && apply.moves[i].name[c]; ++c) p[at + c] = (u8)apply.moves[i].name[c];
        store32(p + at + 40, apply.moves[i].size);
        for (u32 c = 0; c < 32; ++c) p[at + 44 + c] = apply.moves[i].sha[c];
    }
    if (!crc_made) crc_make();
    store32(p + at, crc_add(0xffffffffu, p, at) ^ 0xffffffffu);
    return at + 4;
}

// A note read: 1 when it holds (its magic, its sum, its format, this firmware's id, its counts, each name a pack's,
// base.pack last among the moves when it says so). The names then stand in the note's own bytes, 40 apart.
struct NoteView {
    u32 version, removals_n, moves_n, base, test_k, base_sum, new_base_sum;
    const u8* removals;
    const u8* moves;
};

static int note_name_holds(const u8* p) {
    char name[NAME_MOST + 1];
    u32 n = 0;
    for (; n < 40 && p[n]; ++n) {
        if (n == (u32)NAME_MOST) return 0;
        name[n] = (char)p[n];
    }
    name[n] = 0;
    for (u32 c = n; c < 40; ++c)
        if (p[c]) return 0;
    return pack_name(name);
}

static int note_parse(const u8* p, u32 n, NoteView* v) {
    if (n < (u32)NOTE_LEAST || n > (u32)NOTE_MOST) return 0;
    if (p[0] != 'O' || p[1] != 'F' || p[2] != 'A' || p[3] != 'P') return 0;
    if (!crc_made) crc_make();
    if ((crc_add(0xffffffffu, p, n - 4) ^ 0xffffffffu) != load32(p + n - 4)) return 0;
    if ((p[4] | (u32)p[5] << 8) != (u32)NOTE_FORMAT || load32(p + 8) != DOOR_OS_ID) return 0;
    v->version = p[6] | (u32)p[7] << 8;
    v->removals_n = p[12];
    v->moves_n = p[13];
    v->base = p[14];
    v->test_k = p[15];
    v->base_sum = load32(p + 16);
    v->new_base_sum = load32(p + 20);
    if (v->removals_n > (u32)REMOVALS_MOST || v->moves_n > (u32)WAITING_MOST || v->base > 1) return 0;
    if (n != NOTE_HEAD + 40 * v->removals_n + 76 * v->moves_n + 4) return 0;
    v->removals = p + NOTE_HEAD;
    v->moves = p + NOTE_HEAD + 40 * v->removals_n;
    for (u32 i = 0; i < v->removals_n; ++i)
        if (!note_name_holds(v->removals + 40 * i)) return 0;
    for (u32 i = 0; i < v->moves_n; ++i)
        if (!note_name_holds(v->moves + 76 * i)) return 0;
    if (v->base && (v->moves_n == 0 || !same_name((const char*)(v->moves + 76 * (v->moves_n - 1)), BASE_FILE))) return 0;
    return 1;
}

// `list`: one byte, user/pk (protocol 1); two bytes, the index and the folder: 0 user/pk, 1 user/pkin (protocol 2,
// with bit 3, checked and waiting, and bit 4, the transfer under way), 2 the removals noted (protocol 3, the sizes the
// files of user/pk have); three bytes, the same and `01` (protocol 3): an entry of user/pkin has bit 5, the loader
// would not take it, and bit 6, it replaces a file of user/pk.
// Build 5 (section 14): a listed file's header, behind its entry: the pack's version, its firmware id and its pack
// name, from the 96 bytes of the header alone, read only for a name the loader takes as a pack file's and a size of at
// least a header; where the header is not one the loader reads as a pack of format 3 (its magic, its format, its
// header's size), or could not be read, the version 0xffffffff, the firmware id 0 and no name. Answers the place
// behind it in `out`; the cycles the read took are added to *spent.
static DOOR_APART u32 list_header(u32 at, const char* path, u32 flags, int size, u32* spent) {
    u32 head[HEADER3 / 4];
    int got = -1;
    if ((flags & 1) != 0 && size >= (int)HEADER3) {
        u32 started = cycles();
        int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
        if (descriptor >= 0) {
            got = STOCK_FILE_READ(head, 1, HEADER3, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
        *spent += cycles() - started;
    }
    int read = got == (int)HEADER3 && head[0] == 0x4631504fu && head[1] == 0x4b434150u && head[H_FORMAT] == 3 && head[H_HEADER] == (u32)HEADER3;
    store32(out + at, read ? head[H_VERSION] : 0xffffffffu);
    store32(out + at + 4, read ? head[H_OS] : 0u);
    u32 n = 0;
    if (read) {
        char pack[17];
        pack_name_of(head, pack);
        while (n < 16 && pack[n]) {
            out[at + 9 + n] = (u8)pack[n];
            ++n;
        }
    }
    out[at + 8] = (u8)n;
    return at + 9 + n;
}

static DOOR_APART u32 list(const u8* args, int count, u32* bytes) {
    *bytes = 0;
    if ((count != 1 && count != 2 && count != 3) || args[0] > 63) return BAD;
    u32 folder_byte = count >= 2 ? args[1] : 0;
    if (folder_byte > 2 || (count == 3 && (args[2] == 0 || args[2] > 3))) return BAD;    // build 5: bit 1, the header (build 4: 01 only)
    u32 pkin = folder_byte == 1, removals = folder_byte == 2, more_bits = count == 3 && (args[2] & 1) != 0;
    u32 with_header = count == 3 && (args[2] & 2) != 0, header_cycles = 0;
    const char* folder = pkin ? PKIN_FOLDER : FOLDER;
    const char* prefix = pkin ? PKIN_PREFIX : PREFIX;
    u32 first = args[0], kept = 0, beyond = 0;
    void* dir = removals ? 0 : STOCK_DIR_OPEN(folder);
    if (removals) {                                     // the removals noted, in place of a folder's names
        for (; kept < noted_count; ++kept) {
            copy_name(raw[kept], noted[kept]);
            raw_long[kept] = 0;
        }
    }
    if (dir != 0) {
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(dir);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (name[0] == 0 || name[0] == '.') continue;
            if (kept == RAW_MOST) {
                ++beyond;
                continue;
            }
            u32 n = 0;
            for (; name[n] && n < RAW_NAME - 1; ++n) raw[kept][n] = name[n];
            raw[kept][n] = 0;
            raw_long[kept] = name[n] != 0;
            ++kept;
        }
        STOCK_DIR_CLOSE(dir);
    }
    u32 files = 0;                                      // the folders taken out, after the folder is closed
    for (u32 k = 0; k < kept; ++k) {
        char path[PATH_BYTES];
        path_in(path, prefix, raw[k]);
        if (!removals && !raw_long[k] && STOCK_IS_FOLDER(path)) continue;
        u32 at = files++;
        while (at > 0 && before(raw[k], raw[order[at - 1]])) {
            order[at] = order[at - 1];
            --at;
        }
        order[at] = k;
    }
    u32 total_listed = files < LIST_MOST ? files : LIST_MOST;
    beyond += files - total_listed;
    out[0] = (u8)total_listed;
    out[1] = (u8)(beyond > 255 ? 255 : beyond);
    out[2] = (u8)first;
    u32 at = 4, shown = 0, t = tstate;
    for (u32 i = first; i < total_listed && shown < LIST_PAGE; ++i, ++shown) {
        // Build 5: with the header, an answer ends once its header reading has taken HEADER_CYCLES_MOST (the page asks
        // again from first + count), and never runs past its room; one entry always goes.
        if (with_header && shown > 0 && (header_cycles >= (u32)HEADER_CYCLES_MOST || at + (u32)LIST_HEADER_MOST > (u32)OUT_BYTES)) break;
        const char* name = raw[order[i]];
        u32 n = 0;
        while (name[n]) ++n;
        u32 flags = 0;
        if (!raw_long[order[i]] && pack_name(name)) flags |= 1;
        if (pkin && !raw_long[order[i]]) {
            int w = waiting_at(name);
            if (w >= 0) flags |= 8;
            if ((t == T_RECEIVING || t == T_CHECKING) && same_name(xfer.name, name)) flags |= 16;
            if (more_bits && w >= 0) {
                char there[PATH_BYTES];
                path_in(there, PREFIX, name);
                if (waiting[w].why != 0) flags |= 32;
                if (STOCK_FILE_SIZE(there) >= 0) flags |= 64;
            }
        }
        if (n > NAME_MOST || raw_long[order[i]]) {
            flags |= 4;
            n = n > NAME_MOST ? NAME_MOST : n;
        }
        char path[PATH_BYTES];
        path_in(path, prefix, name);
        int size = raw_long[order[i]] ? -1 : STOCK_FILE_SIZE(path);
        if (size < 0) flags |= 2;
        for (u32 c = 0; c < n; ++c) {
            u8 ch = (u8)name[c];
            if (ch < 0x20 || ch > 0x7e) {
                ch = '?';
                flags &= ~1u;
            }
            out[at + 6 + c] = ch;
        }
        out[at] = (u8)flags;
        store32(out + at + 1, size < 0 ? 0xffffffffu : (u32)size);
        out[at + 5] = (u8)n;
        at += 6 + n;
        if (with_header) at = list_header(at, path, raw_long[order[i]] ? 0u : flags, size, &header_cycles);
    }
    out[3] = (u8)shown;
    *bytes = at;
    return DONE;
}

// ---------------------------------------------------------------- sink: read in place, nothing kept

static DOOR_APART u32 sink(u32 buffer, int length, u32* bytes) {
    u32 started = cycles();
    if (!crc_made) crc_make();
    const volatile u8* in = BYTES(buffer);
    u32 status = DONE, at = 6, end = (u32)length - 1, got = 0, crc = 0xffffffffu;
    if (in[end] != 0xf7) {
        status = BAD;
        end = at;                                       // nothing unpacked
    }
    while (at < end) {
        u32 tops = in[at];
        u32 group = end - at - 1 < 7 ? end - at - 1 : 7;
        if (group == 0 || (tops & 0x80) || (tops >> group) != 0) {
            status = BAD;
            break;
        }
        u32 i = 0;
        for (; i < group; ++i) {
            u32 byte = in[at + 1 + i];
            if (byte & 0x80) break;
            byte |= ((tops >> i) & 1) << 7;
            crc = crc_table[(crc ^ byte) & 0xff] ^ (crc >> 8);
            ++got;
        }
        if (i < group) {
            status = BAD;
            break;
        }
        at += 1 + group;
    }
    store32(out, got);
    store32(out + 4, crc ^ 0xffffffffu);
    store32(out + 8, (u32)length);
    store32(out + 12, cycles() - started);
    *bytes = 16;
    ++visit.sink;
    visit.sink_bytes += got;
    return status;
}

// ---------------------------------------------------------------- hello and status

static u32 status_now() {
    u32 s = state;
    if (s == RESTARTS) return RESTARTING;
    if (s == ASKS) return ASKING;
    if (s == APPLYING || s == UNFINISHED) return BUSY_APPLYING;
    if (outcome_unsaid) {
        outcome_unsaid = 0;
        return CANCELLED;
    }
    return DONE;
}

// The state as the answers say it: build 2's and build 4's (6, 7), or the transfer's 4 or 5 while one is under way
// (never the transfer's 6).
static u32 said_state() {
    u32 s = state, t = tstate;
    if (s != IDLE) return s;
    return t == T_RECEIVING || t == T_CHECKING ? t : (u32)IDLE;
}

// hello's 44 bytes for protocol 1 (exactly build 2's, the base's version as this base has it); for `level` 2, protocol
// 2's 60 with build 4's values; for 3, protocol 3's 68.
static u32 hello_payload(u32 level) {
    u32 version = op1f_door_base_version;
    out[0] = (u8)(level >= 3 ? PROTOCOL_3 : level == 2 ? PROTOCOL_2 : PROTOCOL);
    out[1] = MARK;
    out[2] = (u8)version;
    out[3] = (u8)(version >> 8);
    for (u32 i = 0; i < 16; ++i) out[4 + i] = (u8)op1f_release[i];
    store32(out + 20, DOOR_OS_ID);
    store32(out + 24, LARGEST);
    store32(out + 28, SINK_MOST);
    store32(out + 32, level >= 2 ? COPY_MOST : OTHER_MOST);
    store32(out + 36, SLICE);
    out[40] = LIST_PAGE;
    out[41] = (u8)said_state();
    out[42] = 0;
    out[43] = 0;
    if (level < 2) return 44;
    store32(out + 44, PIECE);
    store32(out + 48, LARGEST_FILE);
    store32(out + 52, MARGIN);
    out[56] = WAITING_MOST;
    out[57] = (u8)waiting_count;
    out[58] = DOOR_BUILD;
    out[59] = 0;
    if (level < 3) return 60;
    out[60] = REMOVALS_MOST;
    out[61] = (u8)noted_count;
    out[62] = (u8)RESTART_DELAY_MS;
    out[63] = (u8)(RESTART_DELAY_MS >> 8);
    out[64] = (u8)apply.found;
    out[65] = (u8)apply.outcome;
    out[66] = 0;
    out[67] = 0;
    return 68;
}

// status's 36 bytes (protocol 1); for `level` 2, the transfer's block and its name (12.7); for 3, then the apply's
// block and its name (13.7). Every field a whole word or byte read as it stands; the apply's name copied masked.
static u32 status_payload(u32 level) {
    u32 s = state, shown = said_state();
    out[0] = (u8)shown;
    out[1] = (u8)outcome;
    u32 left = s == ASKS ? seconds_left : 0;
    out[2] = (u8)left;
    out[3] = (u8)(left >> 8);
    store32(out + 4, s == HASHING ? hashing.size : 0);
    store32(out + 8, s == HASHING ? hashing.done : 0);
    store32(out + 12, total.messages);
    store32(out + 16, total.answers);
    store32(out + 20, total.unsent);
    store32(out + 24, total.dropped);
    store32(out + 28, op1f_door_keys[KEYS_LOW]);       // build 2: the door's own copy, as it saw the controller's last report
    store32(out + 32, op1f_door_keys[KEYS_HIGH]);
    if (level < 2) return 36;
    u32 t = tstate, n = 0;
    while (xfer.name[n]) ++n;
    out[36] = (u8)(t == T_RECEIVING || t == T_CHECKING ? t : 0);
    out[37] = (u8)xfer.outcome;
    out[38] = (u8)waiting_count;
    out[39] = (u8)n;
    store32(out + 40, n ? xfer.size : 0);
    store32(out + 44, n ? xfer.received : 0);
    store32(out + 48, n ? xfer.done : 0);
    store32(out + 52, xfer.free_kb);
    for (u32 i = 0; i < n; ++i) out[56 + i] = (u8)xfer.name[i];
    if (level < 3) return 56 + n;
    u8* b = out + 56 + n;
    b[0] = (u8)apply.astate;
    b[1] = (u8)apply.outcome;
    b[2] = (u8)apply.moves_n;
    b[3] = (u8)apply.checked_files;
    b[4] = (u8)apply.removals_n;
    b[5] = (u8)apply.removals_made;
    b[6] = (u8)apply.moves_made;
    b[7] = (u8)apply.found;
    store32(b + 8, apply.to_check);
    store32(b + 12, apply.checked);
    b[16] = (u8)noted_count;
    b[18] = 0;
    b[19] = 0;
    u32 mask, n2 = 0;
    MASK_OFF(mask);
    for (; apply.name[n2] && n2 < (u32)NAME_MOST; ++n2) b[20 + n2] = (u8)apply.name[n2];
    MASK_ON(mask);
    b[17] = (u8)n2;
    return 56 + n + 20 + n2;
}

// ---------------------------------------------------------------- the transfer: begin, data, end (protocol 2, on main_task)

static const char* const REFUSAL_TEXT[R_KINDS] = {
    "the name is not a pack file's", "the size is 0 or above the largest file", "as many files wait as the most",
    "too little free space", "the free space could not be read", "the file could not be opened for writing",
    "the tape moves", "the disk mode is open", "the waiting folder is not ready", "out of order",
    "the unit is being switched off"
};

static const char* const OUTCOME_TEXT[] = {
    "", "kept", "the fingerprint differs", "short", "unreadable", "a write failed", "abort from the page", "a new begin",
    "commit", "the visit ended", "the disk mode opened", "the power popup came up"
};

static u32 tape_word() {
    return RD32(TRANSPORT);
}

static u32 recording_byte() {
    return RD8(RECORDING);
}

// Review S-1: the power switch moved. Its handler (0x8fca3384, the log's `PSOC requested power mode: grace`) sets the
// popup's byte at once, about a second before the shutdown (`power mode: off`): the firmware's own first sign the door
// can read.
static int power_popup() {
    return RD8(POWER_POPUP_SHOWN) != 0;
}

static int tape_moves() {
    return RD32(TRANSPORT) != 0 || RD8(RECORDING) != 0;
}

static void refuse(u32 kind, u32 status, const char* command, const char* name, u32 size) {
    ++files_total.refused;
    if (++refusals[kind] <= (u32)FAULTS_SAID)
        STOCK_LOG(3, LOG_FILE, 28, "door: %s refused: %s (status %u, user/pkin/%s, %u bytes, the transport %u, recording %u); %u such refusals since the start",
                  command, REFUSAL_TEXT[kind], status, name, size, tape_word(), recording_byte(), refusals[kind]);
}

static u32 ended_lines = FAULTS_SAID;

// The transfer ends before its check: the file closed when it is open, removed, the outcome kept. On main_task from a
// message (busy), or on the display thread after the frame has moved the state to T_ENDING: either way the caller owns
// the handle. `was` is the state the transfer was in.
static void end_early(u32 was, u32 why) {
    if (was == T_RECEIVING) STOCK_FS_CLOSE(xfer.handle);
    xfer.handle = -1;
    int answered = 0, left = -1;
    int gone = pkin_remove(xfer.name, &answered, &left);
    xfer.outcome = why;
    ++files_total.ended_before;
    if (!gone || ended_lines) {
        if (gone) --ended_lines;
        STOCK_LOG(gone ? 3 : 1, LOG_FILE, 26, "door: a file ended before its check: user/pkin/%s, %u of %u bytes received, %u checked: %s; the file %s (the size afterwards %d)",
                  xfer.name, xfer.received, xfer.size, xfer.done, OUTCOME_TEXT[why], gone ? "is removed" : "is STILL there", left);
    }
}

// On main_task, with `busy` set: a transfer under way (receiving or checking) ends; nothing when there is none.
static int end_transfer(u32 why) {
    u32 t = tstate;
    if (t != T_RECEIVING && t != T_CHECKING) return 0;
    end_early(t, why);
    tstate = T_NONE;                                    // `busy`: the frame does not touch it meanwhile
    return 1;
}

static DOOR_APART u32 begin(const u8* args, int count, u32* bytes) {
    *bytes = 0;
    if (count < 4 + 32 + NAME_LEAST || count > 4 + 32 + NAME_MOST) return BAD;
    u32 size = load32(args);
    u32 n = (u32)count - 36;
    char name[NAME_MOST + 1];
    for (u32 i = 0; i < n; ++i) name[i] = (char)args[36 + i];
    name[n] = 0;
    if (tstate == T_ENDING) {                           // the frame is ending the last one: its handle is the frame's
        refuse(R_READY, NOT_READY, "begin", name, size);
        return NOT_READY;
    }
    if (power_popup()) {                                // review S-1: the unit is being switched off
        end_transfer(X_POWER);
        refuse(R_POWER, NOT_READY, "begin", name, size);
        return NOT_READY;
    }
    if (state == HASHING) change(HASHING, IDLE);        // before the checks: a hash in progress ends
    end_transfer(X_BEGIN);                              // and a transfer under way
    if (folder_state != F_READY) {
        refuse(R_READY, NOT_READY, "begin", name, size);
        return NOT_READY;
    }
    if (RD32(DISK_OPEN) != 0) {
        refuse(R_DISK, ORDER, "begin", name, size);
        return ORDER;
    }
    if (tape_moves()) {
        refuse(R_TAPE, TAPE, "begin", name, size);
        return TAPE;
    }
    int zero_inside = 0;
    for (u32 i = 0; i < n; ++i) zero_inside |= name[i] == 0;
    if (zero_inside || !pack_name(name)) {
        refuse(R_NAME, REFUSED, "begin", "?", size);
        return REFUSED;
    }
    if (size == 0 || size > (u32)LARGEST_FILE) {
        refuse(R_SIZE, BAD, "begin", name, size);
        return BAD;
    }
    int replaces = waiting_at(name) >= 0;
    if (waiting_count - (replaces ? 1u : 0u) >= (u32)WAITING_MOST) {
        refuse(R_COUNT, NO_ROOM, "begin", name, size);
        out[0] = 2;
        *bytes = 1;
        return NO_ROOM;
    }
    long long free = STOCK_FREE_SPACE(FREE_PATH);
    if (free < 0) {
        refuse(R_FREE, NO_ROOM, "begin", name, size);
        out[0] = 1;
        store32(out + 1, 0xffffffffu);
        *bytes = 5;
        return NO_ROOM;
    }
    unsigned long long free_bytes = (unsigned long long)free;
    u32 free_kb = (free_bytes >> 10) > 0xffffffffull ? 0xffffffffu : (u32)(free_bytes >> 10);
    xfer.free_kb = free_kb;
    if (free_bytes < (unsigned long long)size + (unsigned long long)MARGIN) {
        refuse(R_ROOM, NO_ROOM, "begin", name, size);
        out[0] = 1;
        store32(out + 1, free_kb);
        *bytes = 5;
        return NO_ROOM;
    }
    char path[PATH_BYTES];
    if (!pkin_path(path, name)) {                       // cannot be after pack_name; held all the same
        refuse(R_NAME, REFUSED, "begin", "?", size);
        return REFUSED;
    }
    int handle = STOCK_FS_OPEN(path, WRITE_MODE);
    if (handle < 0) {
        refuse(R_OPEN, NO_ROOM, "begin", name, size);
        out[0] = 3;
        *bytes = 1;
        return NO_ROOM;
    }
    if (replaces) waiting_drop(name);                   // emptied by the open: it no longer waits
    noted_drop(name);                                   // build 4: the file that comes replaces the one noted for removal (13.3)
    for (u32 i = 0; i <= n; ++i) xfer.name[i] = name[i];
    for (u32 i = 0; i < PATH_BYTES; ++i) xfer.path[i] = path[i];
    for (u32 i = 0; i < 32; ++i) xfer.expected[i] = args[4 + i];
    xfer.size = size;
    xfer.received = 0;
    xfer.done = 0;
    xfer.handle = handle;
    xfer.outcome = X_NONE;
    xfer.began_ms = op1f_ms();
    xfer.pieces = xfer.write_us = xfer.slowest_us = xfer.slowest_at = xfer.close_us = xfer.slices = xfer.read_us = xfer.hash_us = 0;
    tstate = T_RECEIVING;
    ++files_total.begun;
    STOCK_LOG(3, LOG_FILE, 24, "door: a file begins: user/pkin/%s, %u bytes, %u kB free, %u waiting%s, the transport %u, recording %u",
              name, size, free_kb, waiting_count, replaces ? ", replacing one of that name" : "", tape_word(), recording_byte());
    store32(out, free_kb);
    out[4] = (u8)waiting_count;
    out[5] = (u8)replaces;
    out[6] = 0;
    out[7] = 0;
    *bytes = 8;
    return DONE;
}

static u8 piece[4 + PIECE];

static void data_counts(u32 write_cycles, u32 unpack_cycles) {
    store32(out, xfer.received);
    store32(out + 4, write_cycles);
    store32(out + 8, unpack_cycles);
}

// `data`, read in place from the receive buffer (it is longer than the copy kept for the other commands).
static DOOR_APART u32 data(u32 buffer, int length, u32* bytes) {
    *bytes = 0;
    u32 started = cycles();
    const volatile u8* in = BYTES(buffer);
    if ((u32)length > (u32)DATA_MOST || length < 8 || in[length - 1] != 0xf7) return BAD;
    int count = unpack(in + 6, (u32)length - 7, piece, sizeof piece);
    u32 unpack_cycles = cycles() - started;
    if (count < 5) return BAD;
    u32 offset = load32(piece), n = (u32)count - 4;
    if (tstate != T_RECEIVING) {
        refuse(R_ORDER, ORDER, "data", xfer.name, xfer.size);
        return ORDER;
    }
    if (power_popup()) {                                // review S-1: nothing written while the unit is being switched off
        refuse(R_POWER, NOT_READY, "data", xfer.name, xfer.size);
        end_transfer(X_POWER);
        return NOT_READY;
    }
    if (RD32(DISK_OPEN) != 0) {
        refuse(R_DISK, ORDER, "data", xfer.name, xfer.size);
        end_transfer(X_DISK);
        return ORDER;
    }
    if (offset != xfer.received) {
        refuse(R_ORDER, ORDER, "data", xfer.name, xfer.size);
        data_counts(0, 0);
        *bytes = 12;
        return ORDER;
    }
    if (n > xfer.size - xfer.received) return BAD;     // never a byte past the size `begin` gave
    if (tape_moves()) {
        refuse(R_TAPE, TAPE, "data", xfer.name, xfer.size);
        data_counts(0, 0);
        *bytes = 12;
        return TAPE;
    }
    u32 written_at = cycles();
    int got = STOCK_FS_WRITE(piece + 4, 1, n, xfer.handle);
    u32 write_cycles = cycles() - written_at;
    u32 us = microseconds(write_cycles);
    if (got < 0 || (u32)got != n) {
        u32 took = got < 0 ? 0u : (u32)got;
        STOCK_LOG(1, LOG_FILE, 27, "door: a write into user/pkin/%s took %u of %u bytes at %u: the transfer ended", xfer.name, took, n, xfer.received);
        end_transfer(X_WRITE);
        out[0] = 4;
        store32(out + 1, took);
        *bytes = 5;
        return NO_ROOM;
    }
    xfer.received += n;
    ++xfer.pieces;
    xfer.write_us += us;
    if (us >= xfer.slowest_us) {
        xfer.slowest_us = us;
        xfer.slowest_at = offset;
    }
    files_total.bytes += n;
    if (us > files_total.slowest_us) files_total.slowest_us = us;
    data_counts(write_cycles, unpack_cycles);
    *bytes = 12;
    return DONE;
}

// The check's end: kept, or removed. `digest` the SHA-256 the unit computed.
static u32 check_done(const u8* digest, u32* bytes) {
    int equal = 1;
    for (u32 i = 0; i < 32; ++i) equal &= digest[i] == xfer.expected[i];
    u32 ms = op1f_ms() - xfer.began_ms;
    if (equal) {
        xfer.outcome = X_KEPT;
        if (waiting_count < (u32)WAITING_MOST && waiting_at(xfer.name) < 0) {
            u32 k = waiting_count;
            for (u32 i = 0; i <= NAME_MOST; ++i) waiting[k].name[i] = xfer.name[i];
            waiting[k].size = xfer.size;
            for (u32 i = 0; i < 32; ++i) waiting[k].sha[i] = digest[i];          // build 4: what the apply holds it to
            for (u32 w = 0; w < (u32)HEADER3 / 4; ++w) waiting[k].head[w] = xfer.check.head[w];
            if (!xfer.check.have_head)
                for (u32 w = 0; w < (u32)HEADER3 / 4; ++w) waiting[k].head[w] = 0;
            waiting[k].why = take_reason(xfer.name, xfer.size, &xfer.check, xfer.check.crc ^ 0xffffffffu);
            waiting_count = k + 1;
        }
        ++files_total.kept;
        tstate = T_NONE;
        STOCK_LOG(3, LOG_FILE, 25, "door: a file ended: user/pkin/%s, %u bytes in %u pieces over %u ms: kept, %u waiting",
                  xfer.name, xfer.size, xfer.pieces, ms, waiting_count);
        STOCK_LOG(3, LOG_FILE, 35, "door: its times: the writes %u us, the slowest %u us at %u, the close %u us; checked in %u slices, %u us reading, %u us hashing",
                  xfer.write_us, xfer.slowest_us, xfer.slowest_at, xfer.close_us, xfer.slices, xfer.read_us, xfer.hash_us);
        store32(out, xfer.size);
        store32(out + 4, xfer.done);
        for (u32 i = 0; i < 32; ++i) out[16 + i] = digest[i];
        *bytes = 48;
        return DONE;
    }
    int answered = 0, left = -1;
    int gone = pkin_remove(xfer.name, &answered, &left);
    xfer.outcome = X_DIFFERS;
    ++files_total.removed_at_check;
    tstate = T_NONE;
    STOCK_LOG(gone ? 3 : 1, LOG_FILE, 31, "door: a file ended: user/pkin/%s, %u bytes in %u pieces over %u ms: REMOVED, the fingerprint differs (the page's %08x, the unit's %08x); the file %s",
              xfer.name, xfer.size, xfer.pieces, ms, load32_be(xfer.expected), load32_be(digest), gone ? "is gone" : "is STILL there");
    out[0] = 1;
    store32(out + 1, xfer.size);
    store32(out + 5, xfer.done);
    for (u32 i = 0; i < 32; ++i) out[9 + i] = digest[i];
    *bytes = 41;
    return DIFFERS;
}

static u32 check_unreadable(u32 now, u32* bytes) {
    int answered = 0, left = -1;
    int gone = pkin_remove(xfer.name, &answered, &left);
    xfer.outcome = X_UNREADABLE;
    ++files_total.removed_at_check;
    tstate = T_NONE;
    STOCK_LOG(1, LOG_FILE, 32, "door: a file ended: user/pkin/%s, %u bytes: REMOVED, unreadable at its check at %u (its size now %d); the file %s",
              xfer.name, xfer.size, xfer.done, (int)now, gone ? "is gone" : "is STILL there");
    out[0] = 3;
    store32(out + 1, (int)now < 0 ? 0xffffffffu : xfer.size);
    store32(out + 5, xfer.done);
    *bytes = 9;
    return DIFFERS;
}

// One slice of the check, as `hash` reads one: open, seek, read, close, then hashed.
static DOOR_APART u32 check_slice(u32* bytes) {
    int size = STOCK_FILE_SIZE(xfer.path);
    if (size < 0 || (u32)size != xfer.size) return check_unreadable((u32)size, bytes);
    u32 want = xfer.size - xfer.done < SLICE ? xfer.size - xfer.done : SLICE;
    u32 read_cycles = 0, hash_cycles = 0;
    if (want > 0) {
        u32 started = cycles();
        int descriptor = STOCK_FILE_OPEN(xfer.path, STOCK_READ_MODE);
        int got = -1;
        if (descriptor >= 0) {
            int at = STOCK_FILE_SEEK(descriptor, xfer.done, 0, 0);
            if ((u32)at == xfer.done) got = STOCK_FILE_READ(slice, 1, (int)want, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
        read_cycles = cycles() - started;
        if (got < 0 || (u32)got != want) return check_unreadable((u32)size, bytes);
        started = cycles();
        check_add(&xfer.check, slice, xfer.done, want);  // build 4: the header and the CRC-32 beside the SHA-256
        hash_cycles = cycles() - started;
        xfer.done += want;
        ++xfer.slices;
        xfer.read_us += microseconds(read_cycles);
        xfer.hash_us += microseconds(hash_cycles);
    }
    if (xfer.done < xfer.size) {
        store32(out, xfer.size);
        store32(out + 4, xfer.done);
        store32(out + 8, read_cycles);
        store32(out + 12, hash_cycles);
        *bytes = 16;
        return MORE;
    }
    u8 digest[32];
    sha_end(&xfer.check.sha, digest);
    u32 status = check_done(digest, bytes);
    if (status == DONE) {
        store32(out + 8, read_cycles);
        store32(out + 12, hash_cycles);
    }
    return status;
}

static DOOR_APART u32 end(const u8* args, int count, u32* bytes) {
    *bytes = 0;
    if (count == 1 && args[0] == 0) {                   // close
        if (tstate != T_RECEIVING) {
            refuse(R_ORDER, ORDER, "end", xfer.name, xfer.size);
            return ORDER;
        }
        u32 started = cycles();
        STOCK_FS_CLOSE(xfer.handle);
        xfer.handle = -1;
        u32 close_cycles = cycles() - started;
        xfer.close_us = microseconds(close_cycles);
        if (xfer.received < xfer.size) {
            tstate = T_CHECKING;                        // the file is closed: end_transfer's close is not made twice
            end_transfer(X_SHORT);
            out[0] = 2;
            store32(out + 1, xfer.size);
            store32(out + 5, xfer.received);
            *bytes = 9;
            return DIFFERS;
        }
        check_start(&xfer.check);
        xfer.done = 0;
        tstate = T_CHECKING;
        store32(out, xfer.size);
        store32(out + 4, 0);
        store32(out + 8, close_cycles);
        store32(out + 12, 0);
        *bytes = 16;
        return MORE;
    }
    if (count == 5 && args[0] == 1) {                   // next
        if (tstate != T_CHECKING) {
            refuse(R_ORDER, ORDER, "end", xfer.name, xfer.size);
            return ORDER;
        }
        if (load32(args + 1) != xfer.done) {
            store32(out, xfer.size);
            store32(out + 4, xfer.done);
            store32(out + 8, 0);
            store32(out + 12, 0);
            *bytes = 16;
            return ORDER;
        }
        return check_slice(bytes);
    }
    return BAD;
}

// `abort 01`: at most PAGE_REMOVALS files of user/pkin/ removed. Bits 3 (files removed) and 4 (more remain) of `flags`.
static u32 empty_from_page(u32* removed_out) {
    u32 names = 0, removed = 0, failed = 0, others = 0;
    void* dir = STOCK_DIR_OPEN(PKIN_FOLDER);
    if (dir != 0) {
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(dir);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (name[0] == 0) continue;
            if (names == RAW_MOST) {
                ++others;
                continue;
            }
            u32 n = 0;
            for (; name[n] && n < RAW_NAME - 1; ++n) raw[names][n] = name[n];
            raw[names][n] = 0;
            raw_long[names] = name[n] != 0;
            ++names;
        }
        STOCK_DIR_CLOSE(dir);
    }
    u32 tried = 0;
    for (u32 k = 0; k < names; ++k) {
        char path[PATH_BYTES];
        if (raw[k][0] == '.' || raw_long[k] || !pkin_path(path, raw[k]) || STOCK_IS_FOLDER(path)) continue;   // the review's N-3: a note of the door's is never the page's to remove
        if (tstate == T_ENDING && same_name(raw[k], xfer.name)) continue;   // the frame's to remove
        if (tried == PAGE_REMOVALS) {
            ++others;
            continue;
        }
        ++tried;
        int answered = 0, left = -1;
        if (pkin_remove(raw[k], &answered, &left)) {
            ++removed;
            waiting_drop(raw[k]);
        } else {
            ++failed;
        }
    }
    STOCK_LOG(failed ? 1 : 3, LOG_FILE, 29, "door: the waiting folder emptied from the page: %u removed, %u could NOT be, %s",
              removed, failed, others ? "more left for the next message" : "none left");
    *removed_out = removed;
    return (removed ? 8u : 0u) | (others ? 16u : 0u);
}

// ---------------------------------------------------------------- a message of the door's

static u8 copy[COPY_MOST];
static u8 args_of[COPY_MOST];
static u32 restart_asked;
static volatile u32 restart_ms;                         // the cards' clock when the restart was asked (mend 1)
static u32 ask_slow, restart_slow;                      // slow_clock() when the asking began and when the restart was asked (mend 3)
static u32 asking_apply;                                // build 4: the page asks for an apply (commit 01 or 02), not a plain restart

static void visit_begins(u32 command, u32 number, u32 length) {
    for (u32* w = (u32*)&visit; w < (u32*)(&visit + 1); ++w) *w = 0;
    visit_on = 1;
    long_said = 0;
    ++total.visits;
    STOCK_LOG(3, LOG_FILE, 2, "door: a visit begins: command 0x%02x, number %u, %u bytes, the first in 10 s; before it %u messages, %u answers sent, %u not, %u SysEx dropped (%u not F0), %u visits",
              command, number, length, total.messages, total.answers, total.unsent, total.dropped, total.not_f0, total.visits - 1);
}

static void count_command(u32 command) {
    if (command == CMD_HELLO) ++visit.hello;
    else if (command == CMD_LIST) ++visit.list;
    else if (command == CMD_HASH) ++visit.hash;
    else if (command == CMD_COMMIT) ++visit.commit;
    else if (command == CMD_STATUS) ++visit.status;
    else if (command == CMD_ABORT) ++visit.abort;
}

static int known(u32 command) {
    return command == CMD_HELLO || command == CMD_LIST || command == CMD_HASH || command == CMD_ABORT || command == CMD_COMMIT ||
           command == CMD_STATUS || command == CMD_SINK || command == CMD_BEGIN || command == CMD_DATA || command == CMD_END ||
           command == CMD_REMOVE;
}

// An apply's page that closes before T1 (T2, 60 s, abort, no page): nothing changed; the files still wait and the
// removals stay noted.
static void apply_not_begun(u32 why) {
    if (!asking_apply) return;
    apply.outcome = why;
    apply.astate = A_STOPPED;
}

// Mend 3 (the review's finding 4): the limits of the asking and of the restart held against a clock that does not
// lean on the display thread's frame call. The page asks `status` every 250 ms while the unit asks, so each limit is met
// within one message of it; the frame call's own limits (60 s, 15 s) come first whenever frames are made.
static void guard() {
    u32 now = slow_clock(), s = state;
    if (s == ASKS && now - ask_slow >= (u32)GUARD_ASK_UNITS) {
        if (change(ASKS, IDLE)) {
            outcome = ENDED_TIME;
            outcome_unsaid = 1;
            apply_not_begun(P_TIME);
            op1f_top_close();                           // the keys back at once; the page's words go at the next frame
            STOCK_LOG(1, LOG_FILE, 14, "door: the confirm page has asked for more than 75 s by the core's clock and no frame closed it: ended here, outcome 3");
        }
    } else if (s == RESTARTS && restart_asked && now - restart_slow >= (u32)GUARD_RESTART_UNITS) {
        if (change(RESTARTS, IDLE)) {
            outcome = ENDED_NO_RESTART;
            outcome_unsaid = 0;                         // said in the status answer as status 0, outcome 7
            restart_asked = 0;
            op1f_top_close();
            STOCK_LOG(1, LOG_FILE, 15, "door: the restart was asked more than 20 s ago by the core's clock and did not come: the confirm page taken down here, outcome 7");
        }
    }
}

// The single byte of a one-byte payload read in place (the message as it came): -1 for any other payload.
static int one_byte(const volatile u8* in, int length) {
    if (length != 9 || in[8] != 0xf7 || in[6] > 1 || (in[7] & 0x80)) return -1;
    return (int)(in[7] | (u32)(in[6] & 1) << 7);
}

// ---------------------------------------------------------------- build 4: remove, on main_task

static DOOR_APART u32 remove_cmd(const u8* args, int count, u32* bytes) {
    *bytes = 0;
    if (count < 1 || args[0] > 2) return BAD;
    u32 form = args[0];
    if (form == 2) {                                    // take every one back
        if (count != 1) return BAD;
        u32 were = noted_count;
        noted_count = 0;
        out[0] = 0;
        out[1] = (u8)waiting_count;
        out[2] = (u8)were;
        out[3] = 0;
        *bytes = 4;
        return DONE;
    }
    if (count < 2) return BAD;
    if (form == 0) {
        if (power_popup()) return NOT_READY;
        if (RD32(DISK_OPEN) != 0) return ORDER;
    }
    u32 n = (u32)count - 1;
    char name[NAME_MOST + 1];
    int fits = n <= (u32)NAME_MOST;
    if (fits) {
        for (u32 i = 0; i < n; ++i) {
            name[i] = (char)args[1 + i];
            if (name[i] == 0) fits = 0;                 // a zero inside: not a name the loader takes
        }
        name[n] = 0;
    }
    if (form == 1) {                                    // take back: a name never noted answers 0
        int was = fits ? noted_drop(name) : 0;
        out[0] = (u8)noted_count;
        out[1] = (u8)waiting_count;
        out[2] = (u8)was;
        out[3] = 0;
        *bytes = 4;
        return DONE;
    }
    if (!fits || !pack_name(name)) return REFUSED;
    if (base_file_any_case(name)) {                     // the base is never removed: a new one comes by sending it
        out[0] = 1;
        *bytes = 1;
        return REFUSED;
    }
    u32 t = tstate;
    if (waiting_at(name) >= 0 || ((t == T_RECEIVING || t == T_CHECKING) && same_name(xfer.name, name))) {
        out[0] = 2;                                     // it will replace the one in user/pk: nothing to remove
        *bytes = 1;
        return ORDER;
    }
    char path[PATH_BYTES];
    path_in(path, PREFIX, name);
    if (STOCK_FILE_SIZE(path) < 0) return UNREADABLE;
    int already = noted_at(name) >= 0;
    if (!already && noted_count >= (u32)REMOVALS_MOST) {
        out[0] = 5;
        *bytes = 1;
        return NO_ROOM;
    }
    if (!already) copy_name(noted[noted_count++], name);
    out[0] = (u8)noted_count;
    out[1] = (u8)waiting_count;
    out[2] = (u8)already;
    out[3] = 0;
    *bytes = 4;
    return DONE;
}

// ---------------------------------------------------------------- build 4: commit 01 and 02 (13.5), on main_task

// user/pk as it is when an apply is asked: each pack file's name, and what the loader would take of the room for it
// (its header read, 96 bytes). `gone_at` the change after which it is no longer there (removed, or replaced by a move),
// 0 when it stays.
struct Kept {
    char name[NAME_MOST + 1];
    char pack[17];
    u32 takes, link, foot, version, gone_at, removed;
    u32 folder, sum, meets;                             // a folder of that name; its header's sum; it meets the base (bit 0 the kept, bit 1 the waiting)
};
static Kept kept[KEPT_MOST];
static u32 kept_count, kept_names, kept_more;           // kept, every pack name of user/pk (folders too, as the loader counts), more entries than listed
static u32 move_meets[WAITING_MOST];
static u32 move_w[WAITING_MOST];                        // each move's place in `waiting`
static u32 move_op[WAITING_MOST];                       // the change that makes it
static char move_pack[WAITING_MOST][17];
static u32 set_order[KEPT_MOST + WAITING_MOST];         // kept and moves together, in the order of their names
static u8 set_placed[KEPT_MOST + WAITING_MOST];

static const char* e_name(u32 e) { return e < kept_count ? kept[e].name : apply.moves[e - kept_count].name; }
static int e_there(u32 e, u32 s) {                     // in user/pk after the first s changes
    if (e < kept_count) return kept[e].gone_at == 0 || s < kept[e].gone_at;
    return s >= move_op[e - kept_count];
}
static int e_takes(u32 e) { return e < kept_count ? (int)kept[e].takes : 1; }
static const char* e_pack(u32 e) { return e < kept_count ? kept[e].pack : move_pack[e - kept_count]; }
static u32 e_head(u32 e, u32 field) {
    if (e < kept_count) return field == H_LINK ? kept[e].link : field == H_VERSION ? kept[e].version : kept[e].foot;
    const u32* h = waiting[move_w[e - kept_count]].head;
    return field == H_LINK ? h[H_LINK] : field == H_VERSION ? h[H_VERSION] : foot_of(h);
}

// Where the loader would place the packs of user/pk after the first s changes (build_m4_loader.cpp run_new): in the
// order of the files' names, of the files holding one pack the highest version (the first of equal ones), each at the
// next address that is a multiple of 32 from where it was linked for, the next behind its foot rounded to a word. The
// bytes from the room's first to the end of the last, the room taken as large as it needs to be: what the set needs.
static u32 room_needed(u32 s) {
    u32 n = kept_count + apply.moves_n;
    for (u32 i = 0; i < n; ++i) set_placed[i] = 0;
    u32 next = PACK_BASE, end = PACK_BASE;
    for (u32 oi = 0; oi < n; ++oi) {
        u32 i = set_order[oi];
        if (set_placed[i] || !e_there(i, s) || !e_takes(i)) continue;
        u32 best = i;
        for (u32 ok = oi + 1; ok < n; ++ok) {
            u32 k = set_order[ok];
            if (!set_placed[k] && e_there(k, s) && e_takes(k) && same_name(e_pack(k), e_pack(i)) && e_head(k, H_VERSION) > e_head(best, H_VERSION)) best = k;
        }
        u32 place = next + ((e_head(best, H_LINK) - next) & (u32)(PACK_ALIGN - 1));
        u32 foot = e_head(best, 0);
        u32 last = place + foot, after = place + ((foot + 3) & ~3u);
        if (best < kept_count) {
            // The review's N-3: a kept file's sum is not read here, and where it fails the loader takes the next version
            // of that pack; each of them counted, the largest end kept.
            for (u32 ok = oi; ok < n; ++ok) {
                u32 k = set_order[ok];
                if (k == best || !e_there(k, s) || !e_takes(k) || !same_name(e_pack(k), e_pack(i))) continue;
                u32 at = next + ((e_head(k, H_LINK) - next) & (u32)(PACK_ALIGN - 1)), f2 = e_head(k, 0);
                if (at + f2 > last) last = at + f2;
                if (at + ((f2 + 3) & ~3u) > after) after = at + ((f2 + 3) & ~3u);
            }
        }
        if (last > end) end = last;
        next = after;
        for (u32 ok = 0; ok < n; ++ok) {
            u32 k = set_order[ok];
            if (e_there(k, s) && e_takes(k) && same_name(e_pack(k), e_pack(best))) set_placed[k] = 1;
        }
    }
    return end - PACK_BASE;
}

// The pack names of user/pk after the first s changes.
static u32 count_after(u32 s) {
    u32 c = kept_names;
    for (u32 i = 0; i < kept_count; ++i)
        if (kept[i].removed && kept[i].gone_at && s >= kept[i].gone_at) --c;
    for (u32 j = 0; j < apply.moves_n; ++j) {
        int replaces = 0;
        for (u32 i = 0; i < kept_count; ++i) replaces |= same_name(kept[i].name, apply.moves[j].name);
        if (!replaces && s >= move_op[j]) ++c;
    }
    return c;
}

// user/pk read for the apply: its pack files (the folders and the other names left out), each file's header.
static void kept_list() {
    kept_count = kept_names = kept_more = 0;
    u32 names = 0;
    void* dir = STOCK_DIR_OPEN(FOLDER);
    if (dir != 0) {
        for (;;) {
            const char* entry = (const char*)STOCK_DIR_NEXT(dir);
            if (!entry) break;
            const char* name = entry + DIR_NAME_AT;
            if (name[0] == 0 || name[0] == '.') continue;
            if (names == RAW_MOST) {
                kept_more = 1;                          // the review's N-3: entries the door cannot count
                continue;
            }
            u32 n = 0;
            for (; name[n] && n < RAW_NAME - 1; ++n) raw[names][n] = name[n];
            raw[names][n] = 0;
            raw_long[names] = name[n] != 0;
            ++names;
        }
        STOCK_DIR_CLOSE(dir);
    }
    for (u32 k = 0; k < names; ++k) {
        if (raw_long[k] || !pack_name(raw[k])) continue;
        char path[PATH_BYTES];
        path_in(path, PREFIX, raw[k]);
        int folder = STOCK_IS_FOLDER(path);
        ++kept_names;                                   // a folder of a pack's name counts in the loader's 32 (its look)
        if (kept_count == (u32)KEPT_MOST) continue;
        Kept* f = &kept[kept_count++];
        copy_name(f->name, raw[k]);
        f->takes = f->link = f->foot = f->version = f->gone_at = f->removed = f->sum = f->meets = 0;
        f->folder = (u32)folder;
        f->pack[0] = 0;
        if (folder) continue;
        int size = STOCK_FILE_SIZE(path);
        if (size < (int)HEADER3) continue;
        u32 head[HEADER3 / 4];
        int got = -1;
        int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
        if (descriptor >= 0) {
            got = STOCK_FILE_READ(head, 1, HEADER3, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
        if (got != (int)HEADER3) continue;              // the loader leaves it out too: no room
        f->takes = header_takes(head, (u32)size) && header_holds(head);
        f->link = head[H_LINK];
        f->foot = foot_of(head);
        f->version = head[H_VERSION];
        f->sum = head[H_CRC];
        pack_name_of(head, f->pack);
    }
}

// ---- the review's S-5: a pack that sorts before base.pack and offers a name the base offers, or changes a place of
// stock the base changes, leaves the base out at the next start (build_m4_loader.cpp judge: WHY_NAME, WHY_CLAIM).

static u32 base_names[2][BASE_NAMES_MOST], base_sites[2][BASE_SITES_MOST][2], base_names_n[2], base_sites_n[2], base_read[2];
static u32 other_names[MEETS_NAMES_MOST], other_sites[2 * MEETS_NAMES_MOST][2];
static u8 offers_buf[640];

static int read_at(int descriptor, u32 at, void* to, u32 n) {
    if (n == 0) return 1;
    int now = STOCK_FILE_SEEK(descriptor, at, 0, 0);
    return (u32)now == at && STOCK_FILE_READ(to, 1, (int)n, descriptor) == (int)n;
}

// The bytes a table's entry stands on (build_m4_loader.cpp shape_of: a word 4, a hook 8, a longer trampoline its bytes).
static u32 span_of_kind(u32 kind) {
    u32 low = kind, trampoline = 0;
    if (kind > 5) {
        low = kind & 0xfu;
        trampoline = kind >> 16;
    }
    return low == 0 ? 4u : trampoline > 8 ? trampoline : 8u;
}

// The CRC-32 of each name a pack offers and each site of its table with its span, read from its file (its lists'
// counts, its exports, its strings into `slice`, its table): 1 when read whole within the room given.
static DOOR_APART int offers_of(const char* path, const u32* head, u32* names, u32 names_most, u32* names_n, u32 (*sites)[2], u32 sites_most, u32* sites_n) {
    *names_n = *sites_n = 0;
    if (!crc_made) crc_make();
    u32 body = head[H_BODY];
    int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
    if (descriptor < 0) return 0;
    u8 counts[24];
    int ok = read_at(descriptor, HEADER3 + body, counts, 24);
    u32 works = load32(counts), exports = load32(counts + 4), imports = load32(counts + 8), places = load32(counts + 12);
    u32 marks = load32(counts + 16), strings = load32(counts + 20);
    u32 entries = 0;
    for (u32 at = head[H_TABLE]; at < head[H_TABLE_END]; at += ENTRY_BYTES) ++entries;
    if (ok && (exports > names_most || strings > (u32)SLICE || entries > sites_most)) ok = 0;
    u32 exports_at = HEADER3 + body + 24 + 12 * works;
    u32 strings_at = exports_at + 8 * exports + 8 * imports + 12 * places + 8 * marks;
    if (ok) ok = read_at(descriptor, strings_at, slice, strings);
    for (u32 e = 0; ok && e < exports; e += 80) {      // the exports, eighty a read
        u32 take = exports - e < 80 ? exports - e : 80;
        ok = read_at(descriptor, exports_at + 8 * e, offers_buf, 8 * take);
        for (u32 i = 0; ok && i < take; ++i) {
            u32 name = load32(offers_buf + 8 * i), crc = 0xffffffffu;
            if (name >= strings) {
                ok = 0;
                break;
            }
            for (u32 c = name; c < strings && slice[c]; ++c) crc = crc_table[(crc ^ slice[c]) & 0xff] ^ (crc >> 8);
            names[(*names_n)++] = crc ^ 0xffffffffu;
        }
    }
    u32 table_at = HEADER3 + (head[H_TABLE] - head[H_LINK]);
    for (u32 e = 0; ok && e < entries; e += 32) {      // the table, thirty-two entries a read
        u32 take = entries - e < 32 ? entries - e : 32;
        ok = read_at(descriptor, table_at + ENTRY_BYTES * e, offers_buf, ENTRY_BYTES * take);
        for (u32 i = 0; ok && i < take; ++i) {
            sites[*sites_n][0] = load32(offers_buf + ENTRY_BYTES * i + 4);
            sites[*sites_n][1] = span_of_kind(load32(offers_buf + ENTRY_BYTES * i));
            ++*sites_n;
        }
    }
    STOCK_FILE_CLOSE(descriptor);
    return ok;
}

// Does a pack (read into other_*) meet the base of slot b: a name both offer, or two sites whose bytes meet? A base or
// a pack that could not be read is held to meet (the cautious way).
static int meets_base(u32 b, int read, u32 names_n, u32 sites_n) {
    if (!base_read[b] || !read) return 1;
    for (u32 i = 0; i < names_n; ++i)
        for (u32 k = 0; k < base_names_n[b]; ++k)
            if (other_names[i] == base_names[b][k]) return 1;
    for (u32 i = 0; i < sites_n; ++i)
        for (u32 k = 0; k < base_sites_n[b]; ++k)
            if (other_sites[i][0] < base_sites[b][k][0] + base_sites[b][k][1] && base_sites[b][k][0] < other_sites[i][0] + other_sites[i][1]) return 1;
    return 0;
}

static const char* const TAKE_TEXT[] = {
    "", "not a pack file of format 3", "another firmware's file", "a size that is not its header's",
    "its header's sum does not hold", "the pack name base on another file", "base.pack is not named base",
    "its card or its lists would not load", "it would leave the base out at the next start", "a folder of that name is in user/pk"
};
static const char* const APPLY_REFUSAL_TEXT[Q_KINDS] = {
    "the unit is being switched off", "the waiting folder is not ready", "the disk mode is open", "the tape moves",
    "nothing to apply", "a file the loader would not take", "more than 32 pack files after it", "the room the packs share",
    "too little free space", "the firmware's 0x66 service had a message since the start", "more than 96 entries in user/pk",
    "a pack ahead of the base", "a folder in the way"
};
static u32 apply_refusals[Q_KINDS];
static struct {
    u32 asked, refused, stopped, made, removed, moved;
} applies_total;

static void apply_refuse(u32 kind, u32 status, u32 reason, const char* name) {
    ++applies_total.refused;
    if (++apply_refusals[kind] <= (u32)FAULTS_SAID)
        STOCK_LOG(3, LOG_FILE, 37, "door: apply refused: %s%s%s (status %u, reason %u, %s); %u such refusal(s) since the start",
                  APPLY_REFUSAL_TEXT[kind], reason && kind == Q_NOT_TAKEN ? ", " : "", kind == Q_NOT_TAKEN && reason <= 9 ? TAKE_TEXT[reason] : "",
                  status, reason, name[0] ? name : "-", apply_refusals[kind]);
}

static u32 not_taken(u32 reason, const char* name, u32* bytes) {
    apply_refuse(Q_NOT_TAKEN, NOT_TAKEN, reason, name);
    u32 n = 0;
    out[0] = (u8)reason;
    for (; name[n] && n < (u32)NAME_MOST; ++n) out[1 + n] = (u8)name[n];
    *bytes = 1 + n;
    return NOT_TAKEN;
}

static DOOR_APART u32 commit_apply(u32 k, u32* bytes) {
    *bytes = 0;
    if (tunnel_count != 0) {                            // the review's S-1: until the unit is switched off and on
        apply_refuse(Q_TUNNEL, NOT_READY, 3, "");
        out[0] = 3;
        *bytes = 1;
        return NOT_READY;
    }
    if (power_popup()) {
        apply_refuse(Q_POWER, NOT_READY, 0, "");
        return NOT_READY;
    }
    if (folder_state != F_READY) {
        apply_refuse(Q_READY, NOT_READY, 0, "");
        return NOT_READY;
    }
    if (RD32(DISK_OPEN) != 0) {
        apply_refuse(Q_DISK, ORDER, 0, "");
        return ORDER;
    }
    if (tape_moves()) {
        apply_refuse(Q_TAPE, TAPE, 0, "");
        return TAPE;
    }
    change(HASHING, IDLE);                              // a hash in progress ends
    end_transfer(X_COMMIT);                             // and a transfer under way (outcome 8, its file removed)
    if (waiting_count == 0 && noted_count == 0) {
        apply_refuse(Q_NOTHING, ORDER, 3, "");
        out[0] = 3;
        *bytes = 1;
        return ORDER;
    }
    // The moves: the files that wait, in the order of their names, base.pack last; each one the loader would take.
    u32 m = 0, base_at = WAITING_MOST;
    for (u32 w = 0; w < waiting_count; ++w) {
        if (same_name(waiting[w].name, BASE_FILE)) {
            base_at = w;
            continue;
        }
        u32 at = m++;
        while (at > 0 && before(waiting[w].name, waiting[move_w[at - 1]].name)) {
            move_w[at] = move_w[at - 1];
            --at;
        }
        move_w[at] = w;
    }
    if (base_at != (u32)WAITING_MOST) move_w[m++] = base_at;
    for (u32 j = 0; j < m; ++j) {
        const Waiting* f = &waiting[move_w[j]];
        if (f->why != 0) return not_taken(f->why, f->name, bytes);
    }
    // The removals, in the order of their names; then user/pk as it is.
    u32 r = 0;
    for (u32 i = 0; i < noted_count; ++i) {
        u32 at = r++;
        while (at > 0 && before(noted[i], apply.removals[at - 1])) {
            copy_name(apply.removals[at], apply.removals[at - 1]);
            --at;
        }
        copy_name(apply.removals[at], noted[i]);
    }
    apply.removals_n = r;
    apply.moves_n = m;
    kept_list();
    if (kept_more) {                                    // the review's N-3: entries the door cannot count the loader's way
        apply_refuse(Q_ENTRIES, NO_ROOM, 8, "");
        out[0] = 8;
        *bytes = 1;
        return NO_ROOM;
    }
    for (u32 j = 0; j < m; ++j)                         // the review's N-3: a folder of a move's name is in the way
        for (u32 e = 0; e < kept_count; ++e)
            if (kept[e].folder && same_name(kept[e].name, waiting[move_w[j]].name)) return not_taken(9, kept[e].name, bytes);
    u32 new_n = 0, replaced_n = 0, to_check = 0;
    for (u32 i = 0; i < r; ++i)
        for (u32 f = 0; f < kept_count; ++f)
            if (same_name(kept[f].name, apply.removals[i])) {
                kept[f].gone_at = i + 1;
                kept[f].removed = 1;
            }
    for (u32 j = 0; j < m; ++j) {
        const Waiting* f = &waiting[move_w[j]];
        Move* mv = &apply.moves[j];
        copy_name(mv->name, f->name);
        mv->size = f->size;
        for (u32 c = 0; c < 32; ++c) mv->sha[c] = f->sha[c];
        pack_name_of(f->head, move_pack[j]);
        move_op[j] = r + j + 1;
        to_check += f->size;
        int replaces = 0;
        for (u32 e = 0; e < kept_count; ++e)
            if (same_name(kept[e].name, f->name)) {
                kept[e].gone_at = r + j + 1;
                replaces = 1;
            }
        if (replaces) ++replaced_n;
        else ++new_n;
    }
    u32 last = r + m, files = count_after(last);
    if (files > (u32)PACKS_MOST) {
        apply_refuse(Q_COUNT, NO_ROOM, 6, "");
        out[0] = 6;
        out[1] = (u8)(files > 255 ? 255 : files);
        *bytes = 2;
        return NO_ROOM;
    }
    // The review's S-5: the base that runs at each point the apply can be cut at (the kept base.pack until its move, the
    // waiting one after it), held against every pack of that point whose file sorts before base.pack: a name both
    // offer, or a site both change, and the loader would leave the base out.
    u32 base_kept = KEPT_MOST, base_move = WAITING_MOST;
    for (u32 e = 0; e < kept_count; ++e)
        if (!kept[e].folder && same_name(kept[e].name, BASE_FILE)) base_kept = e;
    if (base_at != (u32)WAITING_MOST) base_move = m - 1;
    apply.base_sum = base_kept != (u32)KEPT_MOST ? kept[base_kept].sum : 0u;
    apply.new_base_sum = base_move != (u32)WAITING_MOST ? waiting[move_w[base_move]].head[H_CRC] : 0u;
    for (u32 b = 0; b < 2; ++b) {
        base_read[b] = 0;
        base_names_n[b] = base_sites_n[b] = 0;
        char path[PATH_BYTES];
        if (b == 0 && base_kept != (u32)KEPT_MOST && kept[base_kept].takes) {
            path_in(path, PREFIX, BASE_FILE);
            u32 head[HEADER3 / 4];
            int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
            int got = descriptor >= 0 && STOCK_FILE_READ(head, 1, HEADER3, descriptor) == (int)HEADER3;
            if (descriptor >= 0) STOCK_FILE_CLOSE(descriptor);
            if (got) base_read[0] = offers_of(path, head, base_names[0], BASE_NAMES_MOST, &base_names_n[0], base_sites[0], BASE_SITES_MOST, &base_sites_n[0]);
        }
        if (b == 1 && base_move != (u32)WAITING_MOST && pkin_path(path, BASE_FILE))
            base_read[1] = offers_of(path, waiting[move_w[base_move]].head, base_names[1], BASE_NAMES_MOST, &base_names_n[1], base_sites[1], BASE_SITES_MOST, &base_sites_n[1]);
    }
    for (u32 e = 0; e < kept_count; ++e) {
        kept[e].meets = 0;
        if (kept[e].folder || !kept[e].takes || !before(kept[e].name, BASE_FILE)) continue;
        char path[PATH_BYTES];
        path_in(path, PREFIX, kept[e].name);
        u32 head[HEADER3 / 4], nn = 0, ns = 0;
        int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
        int read = descriptor >= 0 && STOCK_FILE_READ(head, 1, HEADER3, descriptor) == (int)HEADER3;
        if (descriptor >= 0) STOCK_FILE_CLOSE(descriptor);
        if (read) read = offers_of(path, head, other_names, MEETS_NAMES_MOST, &nn, other_sites, 2 * MEETS_NAMES_MOST, &ns);
        for (u32 b = 0; b < 2; ++b)
            if (meets_base(b, read, nn, ns)) kept[e].meets |= 1u << b;
    }
    for (u32 j = 0; j < m; ++j) {
        move_meets[j] = 0;
        const Waiting* f = &waiting[move_w[j]];
        if (!before(f->name, BASE_FILE)) continue;
        char path[PATH_BYTES];
        u32 nn = 0, ns = 0;
        int read = pkin_path(path, f->name) && offers_of(path, f->head, other_names, MEETS_NAMES_MOST, &nn, other_sites, 2 * MEETS_NAMES_MOST, &ns);
        for (u32 b = 0; b < 2; ++b)
            if (meets_base(b, read, nn, ns)) move_meets[j] |= 1u << b;
    }
    for (u32 s = 1; s <= r + m; ++s) {
        u32 b = base_move != (u32)WAITING_MOST && s >= move_op[base_move] ? 1u : 0u;
        if (b == 0 && base_kept == (u32)KEPT_MOST) continue;   // no base.pack at that point: nothing to leave out
        for (u32 e = 0; e < kept_count; ++e)
            if ((kept[e].meets >> b & 1) && e_there(e, s)) return not_taken(8, kept[e].name, bytes);
        for (u32 j = 0; j < m; ++j)
            if ((move_meets[j] >> b & 1) && s >= move_op[j]) return not_taken(8, apply.moves[j].name, bytes);
    }
    // The room at every point the apply can be cut at (the plan's 5.3): each state after one change more, as the
    // loader places it. A state that holds more than 32 pack names (only while removals come first) is the unit's own.
    u32 total = kept_count + m, sorted = 0;
    for (u32 e = 0; e < total; ++e) {
        u32 at = sorted++;
        while (at > 0 && before(e_name(e), e_name(set_order[at - 1]))) {
            set_order[at] = set_order[at - 1];
            --at;
        }
        set_order[at] = e;
    }
    u32 needed = 0, after = 0;
    for (u32 s = 1; s <= last; ++s) {
        if (count_after(s) > (u32)PACKS_MOST) continue;
        u32 bytes_s = room_needed(s);
        if (bytes_s > needed) needed = bytes_s;
        if (s == last) after = bytes_s;
    }
    if (needed > (u32)PACK_ROOM) {
        apply_refuse(Q_ROOM, NO_ROOM, 7, "");
        out[0] = 7;
        store32(out + 1, needed);
        store32(out + 5, PACK_ROOM);
        *bytes = 9;
        return NO_ROOM;
    }
    long long free = STOCK_FREE_SPACE(FREE_PATH);
    unsigned long long free_bytes = free < 0 ? 0ull : (unsigned long long)free;
    if (free < 0 || free_bytes < (unsigned long long)MARGIN) {
        u32 free_kb = free < 0 ? 0xffffffffu : (free_bytes >> 10) > 0xffffffffull ? 0xffffffffu : (u32)(free_bytes >> 10);
        apply_refuse(Q_FREE, NO_ROOM, 1, "");
        out[0] = 1;
        store32(out + 1, free_kb);
        *bytes = 5;
        return NO_ROOM;
    }
    apply.new_n = new_n;
    apply.replaced_n = replaced_n;
    apply.base = base_at != (u32)WAITING_MOST;
    apply.to_check = to_check;
    apply.checked = apply.checked_files = apply.removals_made = apply.moves_made = apply.changes = 0;
    apply.test_k = k;
    apply.finishing = 0;
    apply.astate = A_ASKING;
    apply.outcome = P_NONE;
    apply.files_after = files;
    apply.room_after = after;
    apply.step = STEP_NONE;
    apply_name("");
    outcome = 0;
    outcome_unsaid = 0;
    seconds_left = ASK_MS / 1000;
    ask_slow = slow_clock();
    asking_apply = 1;
    if (!change(IDLE, ASKS)) return ASKING;
    ++applies_total.asked;
    STOCK_LOG(3, LOG_FILE, 36, "door: an apply is asked%s: %u new, %u updated, %u removed, the base %s among them; %u bytes to check; after it %u pack files, %u of %u bytes of the room",
              k ? " as a test (commit 02)" : "", new_n, replaced_n, r, apply.base ? "too" : "not", to_check, files, after, (u32)PACK_ROOM);
    out[0] = (u8)new_n;
    out[1] = (u8)replaced_n;
    out[2] = (u8)r;
    out[3] = (u8)apply.base;
    store32(out + 4, to_check);
    *bytes = 8;
    return DONE;
}

static DOOR_APART void message(u32 buffer, int length) {
    u32 started = cycles();
    const volatile u8* in = BYTES(buffer);
    u32 command = in[4], number = in[5];
    u32 ms = op1f_ms();
    if (!visit_on || ms - last_message_ms >= (u32)VISIT_MS) {
        if (visit_on || tstate == T_RECEIVING || tstate == T_CHECKING) end_transfer(X_VISIT);   // 10 s without a message ended it
        visit_started_ms = ms;
        visit_begins(command, number, (u32)length);
    }
    total.last_command = command;
    total.last_number = number;
    total.last_length = (u32)length;
    last_message_ms = ms;
    ++total.messages;
    ++visit.messages;
    if ((u32)length > visit.longest) visit.longest = (u32)length;
    count_command(command);
    guard();
    u32 status = BAD, bytes = 0;
    u32 s = state;
    if (s == RESTARTS || s == APPLYING || s == UNFINISHED) {
        // T1 was pressed: every message answers 10, then the restart. Build 4: while the door applies or could not
        // finish, 14; `hello` and `status` read words and make no file call, every other command does nothing at all.
        status = s == RESTARTS ? RESTARTING : BUSY_APPLYING;
        int b = one_byte(in, length);
        if (command == CMD_HELLO) bytes = hello_payload(b >= PROTOCOL_3 ? 3u : b == PROTOCOL_2 ? 2u : 1u);
        else if (command == CMD_STATUS) bytes = status_payload(b == PROTOCOL_3 ? 3u : b == PROTOCOL_2 ? 2u : 1u);
    } else if (command >= 0x40 || number >= 0x40) {
        status = BAD;
    } else if (!known(command)) {
        status = NO_SUCH;
    } else if (!note_looked && command != CMD_HELLO && command != CMD_STATUS) {
        status = NOT_READY;                             // the review's S-2: the start has not looked for a note yet
    } else if (!start_settled && (command == CMD_LIST || command == CMD_HASH || command == CMD_BEGIN || command == CMD_DATA ||
                                  command == CMD_END || command == CMD_REMOVE || command == CMD_COMMIT)) {
        status = NOT_READY;                             // the start's window: no disk message yet, nothing done
        ++window_refused;
    } else if (s == ASKS && command != CMD_HELLO && command != CMD_STATUS && command != CMD_ABORT) {
        status = ASKING;
    } else if (command == CMD_SINK) {
        status = sink(buffer, length, &bytes);
    } else if (command == CMD_DATA) {
        status = data(buffer, length, &bytes);
    } else if ((u32)length > (command == CMD_BEGIN ? (u32)COPY_MOST : (u32)OTHER_MOST) || in[length - 1] != 0xf7) {
        status = BAD;                                   // protocol 1's 64 for every command of its own, as build 2
    } else {
        for (int i = 0; i < length; ++i) copy[i] = in[i];   // a copy: the USB side may start the next message in its buffer
        int count = unpack(copy + 6, (u32)length - 7, args_of, COPY_MOST);
        if (count < 0) {
            status = BAD;
        } else if (command == CMD_HELLO) {
            if (count == 1) {
                status = status_now();
                bytes = hello_payload(args_of[0] >= PROTOCOL_3 ? 3u : args_of[0] == PROTOCOL_2 ? 2u : 1u);
            }
        } else if (command == CMD_STATUS) {
            if (count == 0 || (count == 1 && (args_of[0] == PROTOCOL_2 || args_of[0] == PROTOCOL_3))) {
                status = status_now();
                bytes = status_payload(count == 0 ? 1u : args_of[0]);
            }
        } else if (command == CMD_LIST) {
            status = list(args_of, count, &bytes);
        } else if (command == CMD_HASH) {
            if (count >= 1 && args_of[0] == 0 && tstate != T_NONE) status = ORDER;   // a transfer under way goes on
            else status = hash(args_of, count, &bytes);
        } else if (command == CMD_BEGIN) {
            status = begin(args_of, count, &bytes);
        } else if (command == CMD_END) {
            status = end(args_of, count, &bytes);
        } else if (command == CMD_REMOVE) {
            status = remove_cmd(args_of, count, &bytes);
        } else if (command == CMD_ABORT) {
            if (count == 1 && args_of[0] == 1 && (folder_state == F_EMPTYING || !start_settled)) {
                status = NOT_READY;                     // review S-3: the start's emptying runs, or the start's window; nothing done
                if (!start_settled) ++window_refused;
            } else if (count == 0 || (count == 1 && args_of[0] == 1)) {
                u32 ended = 0, removed = 0, mask, now;
                int closed = change(ASKS, IDLE);
                MASK_OFF(mask);
                now = state;
                MASK_ON(mask);
                if (!closed && (now == APPLYING || now == UNFINISHED)) {
                    status = BUSY_APPLYING;             // the review's S-3: T1 came first and the door applies; nothing is done
                } else if (!closed && now == RESTARTS) {
                    status = RESTARTING;                // T1 came first on a plain commit's page; nothing is done
                } else {
                    if (change(HASHING, IDLE)) ended |= 1;
                    if (closed) {
                        outcome = ENDED_ABORT;
                        outcome_unsaid = 1;
                        apply_not_begun(P_ABORT);
                        ended |= 2;
                    }
                    if (end_transfer(X_ABORT)) ended |= 4;
                    if (count == 1) ended |= empty_from_page(&removed);
                    if (state == RESTARTS) {
                        status = RESTARTING;
                    } else {
                        status = DONE;
                        out[0] = (u8)ended;
                        out[1] = (u8)removed;
                        bytes = count == 1 ? 2 : 1;
                    }
                }
            }
        } else if (command == CMD_COMMIT) {
            if (count == 0) {
                if (state == ASKS) {
                    status = ASKING;
                } else if (RD32(DISK_OPEN) != 0) {
                    status = ORDER;
                } else {
                    change(HASHING, IDLE);              // a hash in progress ends
                    end_transfer(X_COMMIT);             // and a transfer under way: never one open beside the confirm page
                    outcome = 0;
                    outcome_unsaid = 0;
                    seconds_left = ASK_MS / 1000;
                    ask_slow = slow_clock();
                    asking_apply = 0;                   // a plain commit: T1 restarts, nothing is applied (13.5)
                    if (change(IDLE, ASKS)) {
                        status = DONE;
                        STOCK_LOG(3, LOG_FILE, 3, "door: commit: the confirm page is asked for (mode %u, screen %u shown)", RD32(CURRENT_MODE), RD32(CURRENT_SCREEN));
                    } else {
                        status = ASKING;
                    }
                }
            } else if (count == 1 && args_of[0] == 1) {
                status = commit_apply(0, &bytes);
#if OP1F_DOOR_TEST_STOP
            } else if (count == 2 && args_of[0] == 2 && args_of[1] >= 1 && args_of[1] <= 64) {
                status = commit_apply(args_of[1], &bytes);
#endif
            }
        }
    }
    if (status == BAD || status == REFUSED || status == NO_SUCH || status == ORDER) {
        ++visit.refused;
        if (status == BAD && faults_unread) {
            --faults_unread;
            STOCK_LOG(1, LOG_FILE, 8, "door: a message of %d bytes could not be read (command 0x%02x, number %u)", length, command, number);
        }
    }
    int sent = answer((command & 0x3f), number & 0x3f, status, bytes);
    u32 took = microseconds(cycles() - started);
    if (took > visit.slowest_us) {
        visit.slowest_us = took;
        visit.slowest_command = command;
    }
    int ask = 0;                                        // the review's race (C): decided on the state, masked, at the moment of asking
    if (status == RESTARTING) {
        u32 mask;
        MASK_OFF(mask);
        if (state == RESTARTS && !restart_asked) {
            restart_asked = 1;
            ask = 1;
        }
        MASK_ON(mask);
    }
    if (ask) {
        restart_ms = op1f_ms();
        restart_slow = slow_clock();
        STOCK_LOG(3, LOG_FILE, 6, "door: the firmware's restart is asked (code 20, in %u ms), after the answer to command 0x%02x number %u (%s)",
                  (u32)RESTART_DELAY_MS, command, number, sent ? "handed to the output" : "not sent");
        STOCK_RESTART(RESTART_CODE, RESTART_DELAY_MS);
    }
}

// Every complete SysEx message, before the firmware's handler (the door's stubs of base 18): 1 when the door took it
// (answered or dropped: the firmware never sees it), 0 for the firmware's.
extern "C" int op1f_door_sysex(u32 buffer, int length) OP1F_NAMED("op1f_door_sysex");
// The review's S-1, and build 5's T-2: a message the firmware's own SysEx service takes by its byte 5 (0x8fc8daae's
// test), which can reach the keyboard controller. Counted from every caller of the handler (the USB
// poll's buffer, the second parser's, and Bluetooth MIDI's, from command_handler_ble.cpp's 0x8fca1ed4 on the BLE
// link's loop): the first one's head and where it came from kept; an apply is refused for the rest of the start. The
// callers run on two threads (main_task's USB poll, the BLE link's loop), so the count and the head are taken masked.
static void tunnel_seen(u32 buffer, int length) {
    const volatile u8* in = BYTES(buffer);
    if (length < 6 || in[5] != 0x66 || (in[3] == 6 && (in[4] == 1 || in[4] == 2))) return;
    u32 mask;
    MASK_OFF(mask);
    if (tunnel_count == 0) {
        for (u32 i = 0; i < 6; ++i) tunnel_first[i] = in[i];
        tunnel_length = (u32)length;
    }
    ++tunnel_count;
    if (buffer != USB_SYSEX_BUFFER) ++tunnel_other;
    MASK_ON(mask);
}

extern "C" int op1f_door_sysex(u32 buffer, int length) {
    u32 s = state;
    int asking = s == ASKS || s == RESTARTS || s == APPLYING || s == UNFINISHED;   // the page is up: every other SysEx is dropped
    tunnel_seen(buffer, length);
    if (buffer != USB_SYSEX_BUFFER) {
        if (!asking) return 0;
        ++total.dropped;
        ++visit.dropped;
        return 1;
    }
    const volatile u8* in = BYTES(buffer);
    if (length < 1 || in[0] != 0xf0) {
        ++total.dropped;
        ++total.not_f0;
        ++visit.dropped;
        ++visit.not_f0;
        return 1;
    }
    if (length >= 7 && in[1] == 0x7d && in[2] == 0x4f && in[3] == 0x46) {
        busy = 1;                                       // the frame keeps its hands off the transfer meanwhile (4.3)
        message(buffer, length);
        busy = 0;
        return 1;
    }
    if (asking) {
        ++total.dropped;
        ++visit.dropped;
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------- the waiting folder at the start, and a transfer ended by the frame

static char start_names[START_NAMES][RAW_NAME];
static u32 start_long[START_NAMES];
static u32 start_count, start_at, start_more, start_listed, start_progress;
static u32 start_removed, start_failed, start_folders, start_lines = FAULTS_SAID;

static void start_list() {
    start_count = start_at = start_more = start_progress = 0;
    start_folders = 0;
    void* dir = STOCK_DIR_OPEN(PKIN_FOLDER);
    if (dir == 0) return;
    for (;;) {
        const char* entry = (const char*)STOCK_DIR_NEXT(dir);
        if (!entry) break;
        const char* name = entry + DIR_NAME_AT;
        if (name[0] == 0) continue;
        if (aside_name && same_name(name, aside_name)) continue;    // build 4: a note set aside at this start stays for the read-out
        if (start_count == START_NAMES) {
            start_more = 1;
            continue;
        }
        u32 n = 0;
        for (; name[n] && n < RAW_NAME - 1; ++n) start_names[start_count][n] = name[n];
        start_names[start_count][n] = 0;
        start_long[start_count] = name[n] != 0;
        ++start_count;
    }
    STOCK_DIR_CLOSE(dir);
}

// On the display thread, from the first frame on: user/pkin/ made, then emptied, START_REMOVALS removals a frame;
// `begin` answers 13 until it is done. Every file goes; a folder inside stays (counted).
static DOOR_APART void start_empty() {
    if (!start_listed) {
        start_listed = 1;
        if (!STOCK_MAKE_FOLDERS(PKIN_MAKE)) {
            folder_state = F_FAILED;
            STOCK_LOG(1, LOG_FILE, 33, "door: the waiting folder user/pkin could NOT be made at the start: no file is taken until the next start");
            return;
        }
        start_list();
    }
    for (u32 done = 0; done < (u32)START_REMOVALS && start_at < start_count; ++start_at) {
        const char* name = start_names[start_at];
        char path[PATH_BYTES];
        if (start_long[start_at] || !pkin_path(path, name)) {
            ++start_failed;
            continue;
        }
        if (STOCK_IS_FOLDER(path)) {
            ++start_folders;
            continue;
        }
        ++done;
        int answered = 0, left = -1;
        if (pkin_remove(name, &answered, &left)) {
            ++start_removed;
            start_progress = 1;
        } else {
            ++start_failed;
            if (start_lines) {
                --start_lines;
                STOCK_LOG(1, LOG_FILE, 34, "door: user/pkin/%s could NOT be removed at the start (the removal answered %d, the size is %d)", name, answered, left);
            }
        }
    }
    if (start_at < start_count) return;
    if (start_more && start_progress) {                 // more names than one listing holds: list again
        start_list();
        return;
    }
    folder_state = F_READY;
    STOCK_LOG(start_failed ? 1 : 3, LOG_FILE, 23, "door: the waiting folder user/pkin at the start: %u file(s) removed, %u could NOT be removed, %u folder(s) left; ready",
              start_removed, start_failed, start_folders);
}

// The frame takes a transfer left open over only while no message runs (4.3), then closes and removes.
static void frame_end_transfer(u32 why) {
    u32 mask, was = 0;
    MASK_OFF(mask);
    if (!busy && (tstate == T_RECEIVING || tstate == T_CHECKING)) {
        was = tstate;
        tstate = T_ENDING;
    }
    MASK_ON(mask);
    if (!was) return;                                   // a message runs: the next frame
    end_early(was, why);
    tchange(T_ENDING, T_NONE);
}

// ---------------------------------------------------------------- build 4: the apply, on the display thread

enum { WHY_APPLY_STOPPED = 20 };                        // the confirm page's line 7 for an apply that stopped before any change

static void page_stopped(u32 ms);
static void ask_restart_here(u32 ms, const char* why);

static void apply_stop(u32 ms, u32 why, const char* text, const char* name) {
    apply.outcome = why;
    apply.astate = A_STOPPED;
    ++applies_total.stopped;
    STOCK_LOG(1, LOG_FILE, 40, "door: the apply STOPPED before any change: %s%s%s; nothing changed, the files still wait",
              name[0] ? name : "", name[0] ? " " : "", text);
    if (change(APPLYING, IDLE)) page_stopped(ms);
}

// A removal or a move that could not be made: the note kept (or, at a start that finishes, set aside), no restart. The
// two sizes: for a removal the file's before and after it; for a move the source's and the target's after it.
static void apply_unfinished(const char* what, const char* folder, const char* name, int answered, int size, int target) {
    apply.outcome = P_FAILED;
    apply.astate = A_UNFINISHED;
    if (apply.finishing) {
        note_aside(NOTE_FAILED_PATH);
        apply.found = N_FAILED;
        STOCK_LOG(1, LOG_FILE, 47, "door: the start could NOT finish the last apply: the %s of user/%s/%s answered %d (the sizes %d, %d); the note set aside as user/pkin/.apply-failed; no restart",
                  what, folder, name, answered, size, target);
    } else {
        STOCK_LOG(1, LOG_FILE, 44, "door: the apply NOT finished: the %s of user/%s/%s answered %d (the sizes %d, %d); the note kept for the next start",
                  what, folder, name, answered, size, target);
    }
    change(APPLYING, UNFINISHED);
}

// A file that does not hold at its check: before any change, the apply stops; at a start that finishes, it is not finished.
static void check_failed(u32 ms, const char* name, const char* text) {
    if (!apply.finishing) {
        apply_stop(ms, P_DIFFERS, text, name);
        return;
    }
    apply.outcome = P_FAILED;
    apply.astate = A_UNFINISHED;
    note_aside(NOTE_FAILED_PATH);
    apply.found = N_FAILED;
    STOCK_LOG(1, LOG_FILE, 47, "door: the start could NOT finish the last apply: user/pkin/%s %s; the note set aside as user/pkin/.apply-failed; no restart", name, text);
    change(APPLYING, UNFINISHED);
}

// One slice of the second check (or of a start's check before it finishes): open, seek, read, close, then hashed and
// summed; at a file's end its size, SHA-256 and the loader's tests held to what the apply holds. The power popup stops
// the apply before any change; the tape moving holds the check, and 10 s of it stops it.
static DOOR_APART void check_step(u32 ms) {
    if (apply.at >= apply.moves_n) {
        apply.at = 0;
        apply.done = 0;
        apply.astate = A_CHANGING;
        apply.step = apply.finishing ? (apply.removals_n ? STEP_REMOVE : STEP_MOVE) : STEP_NOTE;
        return;
    }
    if (!apply.finishing) {
        if (power_popup()) {
            apply_stop(ms, P_POWER, "the unit is being switched off", "");
            return;
        }
        if (tape_moves()) {
            if (!apply.tape_since) apply.tape_since = ms;
            if (!apply.tape_since) apply.tape_since = 1;
            if (ms - apply.tape_since >= (u32)TAPE_HELD_MS) apply_stop(ms, P_TAPE, "the tape moved for 10 s", "");
            return;
        }
        apply.tape_since = 0;
    }
    const Move* m = &apply.moves[apply.at];
    char path[PATH_BYTES];
    int size = pkin_path(path, m->name) ? STOCK_FILE_SIZE(path) : -1;
    if (apply.done == 0) {
        if (apply.finishing && size < 0) {              // gone: moved before the stop; its move is judged by its target
            ++apply.at;
            return;
        }
        check_start(&apply.check);
        apply_name(m->name);
    }
    if (size < 0 || (u32)size != m->size) {
        check_failed(ms, m->name, size < 0 ? "is gone at its second check" : "has another size at its second check");
        return;
    }
    u32 want = m->size - apply.done < (u32)SLICE ? m->size - apply.done : (u32)SLICE;
    u32 started = cycles();
    int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
    int got = -1;
    if (descriptor >= 0) {
        int at = STOCK_FILE_SEEK(descriptor, apply.done, 0, 0);
        if ((u32)at == apply.done) got = STOCK_FILE_READ(slice, 1, (int)want, descriptor);
        STOCK_FILE_CLOSE(descriptor);
    }
    apply.read_us += microseconds(cycles() - started);
    if (got < 0 || (u32)got != want) {
        check_failed(ms, m->name, "could not be read whole at its second check");
        return;
    }
    started = cycles();
    check_add(&apply.check, slice, apply.done, want);
    apply.hash_us += microseconds(cycles() - started);
    apply.done += want;
    apply.checked += want;
    ++apply.slices;
    if (apply.done < m->size) return;
    u8 digest[32];
    sha_end(&apply.check.sha, digest);
    int equal = 1;
    for (u32 i = 0; i < 32; ++i) equal &= digest[i] == m->sha[i];
    u32 reason = take_reason(m->name, m->size, &apply.check, apply.check.crc ^ 0xffffffffu);
    if (!equal || reason) {
        check_failed(ms, m->name, !equal ? "differs at its second check" : TAKE_TEXT[reason]);
        return;
    }
    ++apply.checked_files;
    ++apply.at;
    apply.done = 0;
}

// The note written whole (the file class's open "w+b", one write, the close), then read back with te_fopen and held
// byte for byte and by its own sum: only a note read back whole counts. One that cannot be: removed, nothing changed.
static DOOR_APART void note_write(u32 ms) {
    u32 n = note_make();
    char path[PATH_BYTES];
    pkin_path(path, NOTE_NAME);
    u32 started = cycles();
    int handle = STOCK_FS_OPEN(path, WRITE_MODE);
    int got = -1;
    if (handle >= 0) {
        got = STOCK_FS_WRITE(note_bytes, 1, n, handle);
        STOCK_FS_CLOSE(handle);
    }
    u32 write_us = microseconds(cycles() - started);
    int back = -1;
    if (got == (int)n && STOCK_FILE_SIZE(path) == (int)n) {
        int descriptor = STOCK_FILE_OPEN(path, STOCK_READ_MODE);
        if (descriptor >= 0) {
            back = STOCK_FILE_READ(slice, 1, (int)n, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
    }
    int held = back == (int)n;
    for (u32 i = 0; held && i < n; ++i) held = slice[i] == note_bytes[i];
    NoteView v;
    if (held) held = note_parse(slice, n, &v);
    if (!held) {
        int answered = 0, left = -1;
        pkin_remove(NOTE_NAME, &answered, &left);
        apply_stop(ms, P_NOTE, handle < 0 ? "the note could not be opened for writing" : "the note could not be written and read back whole", "");
        return;
    }
    apply.note_bytes = n;
    apply.note_us = write_us;
    STOCK_LOG(3, LOG_FILE, 39, "door: the apply checked %u file(s) in %u slices over %u ms (reading %u us, hashing %u us); the note user/pkin/.apply written, %u bytes in %u us",
              apply.checked_files, apply.slices, ms - apply.began_ms, apply.read_us, apply.hash_us, n, write_us);
    apply.astate = A_CHANGING;
    apply.at = 0;
    apply.step = apply.removals_n ? STEP_REMOVE : STEP_MOVE;
}

// A change made: counted, and commit 02's test stop after k of them: the note kept and the change half made, as a cut
// leaves the disk; then state 3 as after a finished apply (the coordinator's ruling of 2026-11-20): the next message is
// answered status 10 and the restart comes 1,500 ms after it, or 5 s after the stop with no message.
static void apply_changed(u32 ms) {
    ++apply.changes;
    if (apply.finishing || apply.test_k == 0 || apply.changes != apply.test_k) return;
    apply.outcome = P_TEST;
    restart_from_apply = 1;
    apply.changed_ms = ms;
    STOCK_LOG(3, LOG_FILE, 48, "door: a test stop after %u change(s) of %u (commit 02): the note kept and the change half made, as a cut leaves them; the restart %u ms after the next answer, or in 5 s",
              apply.changes, apply.removals_n + apply.moves_n, (u32)RESTART_DELAY_MS);
    change(APPLYING, RESTARTS);
}

static DOOR_APART void remove_step(u32 ms) {
    if (apply.at >= apply.removals_n) {
        apply.at = 0;
        apply.step = STEP_MOVE;
        return;
    }
    const char* name = apply.removals[apply.at];
    apply_name(name);
    u32 started = cycles();
    int answered = 0, size_before = -1, left = -1;
    int gone = pk_remove(name, &answered, &size_before, &left);
    u32 us = microseconds(cycles() - started);
    if (!gone) {
        apply_unfinished("removal", "pk", name, answered, size_before, left);
        return;
    }
    if (size_before >= 0) {
        ++applies_total.removed;
        STOCK_LOG(3, LOG_FILE, 41, "door: removed user/pk/%s (%d bytes) in %u us", name, size_before, us);
    }
    ++apply.removals_made;
    ++apply.at;
    apply_changed(ms);
}

static DOOR_APART void move_step(u32 ms) {
    if (apply.at >= apply.moves_n) {
        apply.step = STEP_END;
        return;
    }
    const Move* m = &apply.moves[apply.at];
    apply_name(m->name);
    u32 started = cycles();
    int answered = 0, target_before = -1, source_after = -1, target_after = -1;
    int moved = apply_move(m, &answered, &target_before, &source_after, &target_after);
    u32 us = microseconds(cycles() - started);
    if (!moved) {
        apply_unfinished("move", "pkin", m->name, answered, source_after, target_after);
        return;
    }
    ++applies_total.moved;
    STOCK_LOG(3, LOG_FILE, 42, "door: moved user/pkin/%s into user/pk (%u bytes, replacing %d, -1 for none; the rename answered %d) in %u us",
              m->name, m->size, target_before, answered, us);
    ++apply.moves_made;
    ++apply.at;
    apply_changed(ms);
}

// The change made: the note removed, the removals noted forgotten, the files no longer waiting; the restart to come.
static DOOR_APART void end_step(u32 ms) {
    int answered = 0, left = -1;
    int gone = pkin_remove(NOTE_NAME, &answered, &left);
    noted_count = 0;
    waiting_count = 0;
    apply.outcome = P_DONE;
    apply.astate = A_DONE;
    apply_name("");
    ++applies_total.made;
    if (apply.finishing) {
        // The coordinator's ruling of 2026-11-20: as after an apply's change, the next message answered status 10 and
        // the restart 1,500 ms after it, or 5 s with no message (the plan said 1,500 ms with no message waited for).
        STOCK_LOG(gone ? 3 : 1, LOG_FILE, 46, "door: the start finished the last apply: %u removed, %u moved in %u ms; the note %s; the restart %u ms after the next answer, or in 5 s",
                  apply.removals_made, apply.moves_made, ms - apply.began_ms, gone ? "removed" : "NOT removed (the next start removes it)", (u32)RESTART_DELAY_MS);
        restart_from_apply = 1;
        apply.changed_ms = ms;
        change(APPLYING, RESTARTS);
        return;
    }
    STOCK_LOG(gone ? 3 : 1, LOG_FILE, 43, "door: the apply made its change: %u removed, %u moved, the base %s among them, in %u ms; the note %s; the restart %u ms after the next answer, or in 5 s",
              apply.removals_made, apply.moves_made, apply.base ? "too" : "not", ms - apply.began_ms, gone ? "removed" : "NOT removed (the next start removes it)",
              (u32)RESTART_DELAY_MS);
    restart_from_apply = 1;
    apply.changed_ms = ms;
    change(APPLYING, RESTARTS);
}

static DOOR_APART void apply_frame(u32 ms) {
    if (apply.finishing && RD32(DISK_OPEN) != 0) {
        // The review's N-5: the disk mode opened while a start finishes (its page could not be shown, or the keys were
        // free): stopped, the note kept for the next start, no restart.
        apply.outcome = P_FAILED;
        apply.astate = A_UNFINISHED;
        STOCK_LOG(1, LOG_FILE, 47, "door: the start could NOT finish the last apply: the disk mode opened; the note kept for the next start; no restart");
        change(APPLYING, UNFINISHED);
        return;
    }
    u32 step = apply.step;
    if (step == STEP_CHECK) check_step(ms);
    else if (step == STEP_NOTE) note_write(ms);
    else if (step == STEP_REMOVE) remove_step(ms);
    else if (step == STEP_MOVE) move_step(ms);
    else if (step == STEP_END) end_step(ms);
}

// The start: the note of an apply cut short looked for before the waiting folder is emptied.
// No note: the emptying as build 3. A note that does not hold: set aside. One whose every change
// was made: removed. One of another base: removed. Else this base's own: the apply finished from where it stopped.
// Build 5: note_looked is set by start_note once this has returned, at the end of each of its paths (the review of
// base 23, S-2's left-over): a message that comes while it reads the note and the running base.pack answers 13.
static DOOR_APART void look_for_note(u32 ms) {
    if (!crc_made) crc_make();
    int size = STOCK_FILE_SIZE(NOTE_PATH);
    if (size < 0) return;
    NoteView v;
    int got = -1;
    if (size >= (int)NOTE_LEAST && size <= (int)NOTE_MOST) {         // read into the note's own bytes: `slice` is main_task's outside the apply
        int descriptor = STOCK_FILE_OPEN(NOTE_PATH, STOCK_READ_MODE);
        if (descriptor >= 0) {
            got = STOCK_FILE_READ(note_bytes, 1, size, descriptor);
            STOCK_FILE_CLOSE(descriptor);
        }
    }
    if (got != size || !note_parse(note_bytes, (u32)size, &v)) {
        if (note_aside(NOTE_BAD_PATH)) aside_name = NOTE_BAD_PATH + 10;
        apply.found = N_UNREADABLE;
        STOCK_LOG(1, LOG_FILE, 45, "door: at the start, a note that could NOT be read (%d bytes, %d read, its sum or its fields): set aside as user/pkin/.apply-bad, nothing changed",
                  size, got);
        return;
    }
    u32 moves_left = 0, removals_left = 0, base_waits = 0;
    char path[PATH_BYTES];
    for (u32 i = 0; i < v.moves_n; ++i) {
        const char* name = (const char*)(v.moves + 76 * i);
        if (pkin_path(path, name) && STOCK_FILE_SIZE(path) >= 0) {
            ++moves_left;
            if (same_name(name, BASE_FILE)) base_waits = 1;
        }
    }
    for (u32 i = 0; i < v.removals_n; ++i) {
        path_in(path, PREFIX, (const char*)(v.removals + 40 * i));
        if (STOCK_FILE_SIZE(path) >= 0) ++removals_left;
    }
    int all_made = v.base ? !base_waits : moves_left == 0 && removals_left == 0;
    int answered = 0, left = -1;
    if (all_made) {
        pkin_remove(NOTE_NAME, &answered, &left);
        apply.found = N_ALL_MADE;
        STOCK_LOG(3, LOG_FILE, 45, "door: at the start, the note of an apply by base %u: every change was made before the stop; the note removed, nothing changed", v.version);
        return;
    }
    if (v.version != op1f_door_base_version) {
        pkin_remove(NOTE_NAME, &answered, &left);
        apply.found = N_OTHER_BASE;
        STOCK_LOG(3, LOG_FILE, 45, "door: at the start, the note of an apply by base %u, not this base: removed, nothing changed", v.version);
        return;
    }
    // The review's N-3: the base.pack that runs must be the file that wrote the note or the one it moves; a note left
    // behind under an older base and met again when this base is copied back by hand is not acted on.
    u32 running = 0;
    {
        char base_path[PATH_BYTES];
        path_in(base_path, PREFIX, BASE_FILE);
        u32 head[HEADER3 / 4];
        int descriptor = STOCK_FILE_OPEN(base_path, STOCK_READ_MODE);
        if (descriptor >= 0) {
            if (STOCK_FILE_READ(head, 1, HEADER3, descriptor) == (int)HEADER3) running = head[H_CRC];
            STOCK_FILE_CLOSE(descriptor);
        }
    }
    if (running == 0 || (running != v.base_sum && !(v.base && running == v.new_base_sum))) {
        pkin_remove(NOTE_NAME, &answered, &left);
        apply.found = N_OTHER_BASE;
        STOCK_LOG(3, LOG_FILE, 45, "door: at the start, the note of an apply by base %u, written under another base.pack (its sum %08x, this one's %08x): removed, nothing changed",
                  v.version, v.base_sum, running);
        return;
    }
    apply.removals_n = v.removals_n;
    apply.moves_n = v.moves_n;
    apply.to_check = 0;
    for (u32 i = 0; i < v.removals_n; ++i) copy_name(apply.removals[i], (const char*)(v.removals + 40 * i));
    for (u32 i = 0; i < v.moves_n; ++i) {
        const u8* at = v.moves + 76 * i;
        Move* mv = &apply.moves[i];
        copy_name(mv->name, (const char*)at);
        mv->size = load32(at + 40);
        for (u32 c = 0; c < 32; ++c) mv->sha[c] = at[44 + c];
        if (pkin_path(path, mv->name) && STOCK_FILE_SIZE(path) >= 0) apply.to_check += mv->size;
    }
    apply.base = v.base;
    apply.new_n = apply.replaced_n = 0;
    apply.test_k = 0;
    apply.finishing = 1;
    apply.checked = apply.checked_files = apply.removals_made = apply.moves_made = apply.changes = 0;
    apply.slices = apply.read_us = apply.hash_us = 0;
    apply.step = STEP_CHECK;
    apply.at = apply.done = 0;
    apply.astate = A_CHECKING;
    apply.outcome = P_NONE;
    apply.found = N_FINISHING;
    apply.began_ms = ms;
    asking_apply = 1;                                   // the page is an apply's
    apply_name("");
    STOCK_LOG(3, LOG_FILE, 45, "door: at the start, the note of an apply by base %u: %u removal(s) and %u of %u move(s) left; finishing",
              v.version, removals_left, moves_left, v.moves_n);
    change(IDLE, APPLYING);
}

static DOOR_APART void start_note(u32 ms) {
    look_for_note(ms);
    note_looked = 1;                                    // every path's end: no note, set aside, removed, or finishing (state 6 from here)
}

// ---------------------------------------------------------------- the confirm page, on the display thread

static struct {
    u32 said_ready;
    void* document;                                     // made once, kept
    u32 picture_asked;
    u32 up;                                             // the top page was asked for and not yet closed
    u32 since;                                          // the clock when it was asked for
    u32 clear_t1, clear_t2;                             // the key's bit was seen clear after the first half second
    u32 closing, closing_since, closing_shown;          // `cancelled` (or `nothing changed`) shown, then the page closed
    u32 next_second, left;
    u32 why;                                            // how it ended (ENDED_...)
    u32 keys_at_up, turns_at_up, t1_at_up;
    u32 reports_at_up;                                  // build 2: the keyboard's reports when the page went up
} page;

// The top page's key function: everything is taken while the page is up; a T1 event with the key not held in the
// keyboard's map is counted (a T1 posted by MIDI). The input threads: it counts and calls nothing.
static int door_keys(const Op1fInput* e) {
    if (e->what == OP1F_TURN) {
        ++turns_taken;
        return 1;
    }
    ++keys_taken;
    u32 copy = op1f_door_keys[KEYS_LOW];
    if ((e->what == OP1F_DOWN || e->what == OP1F_DOWN_SHIFT) && e->key == (u32)KEY_T1 && (copy & BIT_T1) == 0) ++t1_not_held;
    if ((e->key == (u32)KEY_T1 || e->key == (u32)KEY_T1 + 1) && e->what != OP1F_TURN) {
        u32 n = ev_count;
        if (n < (u32)KEY_EVENTS) {                      // the first eight a start: a press of the owner's is rare
            ev_key[n] = e->key;
            ev_what[n] = e->what;
            ev_word[n] = copy;
            ev_reports[n] = op1f_door_keys[KEYS_COUNT];
            ev_count = n + 1;
        }
    }
    return 1;
}

static const char* why_text(u32 why) {
    if (why == ENDED_T2) return "T2 held in the door's copy of the keyboard's map";
    if (why == ENDED_TIME) return "60 s passed";
    if (why == ENDED_ABORT) return "abort from the page";
    if (why == ENDED_NO_MESSAGE) return "no message came within 10 s after T1";
    if (why == ENDED_NO_PAGE) return "the page could not be shown";
    if (why == ENDED_NO_RESTART) return "the restart was asked and did not come";
    if (why == WHY_APPLY_STOPPED) return "the apply stopped before any change";
    return "T1, the unit restarts";
}

// ---------------------------------------------------------------- build 5: the card
//
// The picture door.svg (made on the unit from instructions, as every picture of the base's own) is the
// unit's own question card widened: power_popup.svg's card four times and its red band twice, each moved, so that
// their union is a card from x 56 to 424 and y 6 to 174 and a band from x 61 to 419 and y 11 to 41; a grey band of
// ours over it (`door_band_grey`); the red key of tapebrowser.svg's erase card with power_popup.svg's "1" in it and its
// grey key 2, at half their size (`door_keys`); and the row (`door_row`): power_popup.svg's dark line from x 80 to the
// bolt at 396, y 96, a red bar of ours on it (`door_lit`), twenty-four dot slots each holding power_popup.svg's dark dot
// and its red dot (`door_dotN`, `door_dotN_off`, `door_dotN_on`), all drawn at x 80 and moved by the door, tape.svg's
// red ring (`door_now`), the bolt's dark disc with a red one over it (`door_bolt_on`, power_popup.svg's red dot made
// 1.9 times as large) and the bolt itself, and tape.svg's warning triangle (`door_warning`), drawn over x 80. The words
// are six placeholders, each a 15-pixel stem in its letters' colour: `door_question` in the band (light grey, centred
// on 240), `door_counts` under it (the ground's dark, centred), `door_t1` and `door_t2` right of each key (dark, from
// x 128), `door_note` under the card (light grey, centred) and `door_under` under the row (dark, centred).
//
// The firmware's parser takes no `display`, so every part is shown when the picture is made; the door looks its parts
// up once (op1f_node), and from then on sets each part shown or hidden, and moves it, only when that changes. A part
// the picture lacks is said once and left alone (op1f_show and op1f_move do nothing with none). Display thread only.

enum {
    ROW_X0 = 80, ROW_LAST = 356, ROW_BOLT = 396,        // the first dot's place, the last's at its furthest, the bolt's (pixels)
    ROW_LENGTH = ROW_BOLT - ROW_X0,                     // the red bar as drawn: from the first dot to the bolt, 316
    DOTS_MOST = 24, GAP_TENTHS = 6,                     // at most 24 dots; a gap of 0.6 of a step between the checks and the changes
    NO_STEP = 0xffffffffu,
    W_BAND = 356, W_CARD = 356, W_KEY = 288, W_NOTE = 440,   // the widths a text is laid out in, pixels: the band's, the card's inside, from x 128 to the card's edge
    TX_QUESTION = 0, TX_COUNTS = 1, TX_T1 = 2, TX_T2 = 3, TX_NOTE = 4, TX_UNDER = 5, TEXTS = 6,
    CP_BAND = 0, CP_GREY = 1, CP_KEYS = 2, CP_ROW = 3, CP_LIT = 4, CP_NOW = 5, CP_BOLT = 6, CP_WARNING = 7, PARTS = 8,
    // The counts in full up to 27 characters, as build 4, and (build 5) only while the full words fit the card's line,
    // measured by the firmware's own layout at the page's first frame (counts_measure); else the short form
    LINE_BYTES = 40, COUNTS_LONGEST = 27, BASE_TOO_LONGEST = 24, W_MEASURE = 480,
    SHOWN_UNKNOWN = 2,
    ALIGN_LEFT = 0, ALIGN_CENTRE = 1                    // the label maker's third argument (0 left, 1 centred, 2 right; base 19's text unit)
};

struct CardText {
    const char* part;
    u32 align;                                          // ALIGN_LEFT or ALIGN_CENTRE, for good (the label maker keeps it)
    u32 width;
    void* label;
    u32 laid, failed;
    char text[LINE_BYTES];                              // as last laid out, in small letters
};

static CardText card_texts[TEXTS] = {
    { "door_question", ALIGN_CENTRE, W_BAND, 0, 0, 0, { 0 } },
    { "door_counts", ALIGN_CENTRE, W_CARD, 0, 0, 0, { 0 } },
    { "door_t1", ALIGN_LEFT, W_KEY, 0, 0, 0, { 0 } },
    { "door_t2", ALIGN_LEFT, W_KEY, 0, 0, 0, { 0 } },
    { "door_note", ALIGN_CENTRE, W_NOTE, 0, 0, 0, { 0 } },
    { "door_under", ALIGN_CENTRE, W_CARD, 0, 0, 0, { 0 } },
};

static const char* const CARD_PART[PARTS] = { "door_band", "door_band_grey", "door_keys", "door_row", "door_lit", "door_now", "door_bolt_on", "door_warning" };

static struct {
    void* document;                                     // the document the parts were looked up in
    void* part[PARTS];
    void* dot[DOTS_MOST];
    void* dot_off[DOTS_MOST];
    void* dot_on[DOTS_MOST];
    u8 shown[PARTS];
    u8 dot_shown[DOTS_MOST], off_shown[DOTS_MOST], on_shown[DOTS_MOST];
    u32 found, missing;
    char first_missing[24];
    u32 laid_d, laid_g, laid;                           // the dots' count and the gap's place the slots were moved for
    u32 x_q[DOTS_MOST];                                 // each dot's place, 1024ths of a pixel
    u32 now_at, warning_at, lit_q, lit_set;             // where the ring and the triangle were moved (a dot, or NO_STEP), the bar's end
    u32 draws, moves, shows;                            // counts for the line said when the page closes
} the_card;

// A text's view of the card at one moment: its six words, the parts shown, and the row.
struct CardView {
    const char* text[TEXTS];
    u32 grey, keys, row, bolt;
    u32 steps, checks, done;                            // the row: its steps (each check, then each change), the steps done
    u32 current, warning;                               // the step under way and the step it stopped at, or NO_STEP
};

static void view_start(CardView* v) {
    for (u32 i = 0; i < (u32)TEXTS; ++i) v->text[i] = "";
    v->grey = v->keys = v->row = v->bolt = 0;
    v->steps = v->checks = v->done = 0;
    v->current = v->warning = NO_STEP;
}

// a / b by shift and subtraction (the module links no runtime): 0 for b 0.
static u32 quotient(u32 a, u32 b) {
    if (b == 0) return 0;
    u32 q = 0, r = 0;
    for (int bit = 31; bit >= 0; --bit) {
        r = (r << 1) | ((a >> bit) & 1u);
        if (r >= b) {
            r -= b;
            q |= 1u << bit;
        }
    }
    return q;
}

// The row's rule: one dot a step up to 24 steps; past 24 the row keeps 24 dots and
// dot i stands for the steps from i x n / 24 to (i + 1) x n / 24, an equal share. A dot is done when its share is done
// (done x d >= (i + 1) x n), and the dot of the step under way is the one whose share holds it. The gap between the
// checks and the changes stands before the first dot whose share begins with a change (i x n >= checks x d), when the
// row holds both. Nothing is divided: each test is a product.
static u32 dots_of(u32 n) { return n < (u32)DOTS_MOST ? n : (u32)DOTS_MOST; }

static u32 dot_of_step(u32 n, u32 d, u32 step) {
    u32 i = 0;
    while (i + 1 < d && (i + 1) * n <= step * d) ++i;
    return i;
}

static int dot_done(u32 n, u32 d, u32 done, u32 i) {
    return done * d >= (i + 1) * n;
}

static u32 gap_of(u32 n, u32 d, u32 checks) {
    if (checks == 0 || checks >= n || d < 2) return d;  // no gap: one kind of step only
    u32 i = 1;
    while (i < d && i * n < checks * d) ++i;
    return i;
}

static void set_shown(void* part, u8* was, u32 on) {
    if (part == 0 || *was == (u8)on) return;
    op1f_show(part, (int)on);
    *was = (u8)on;
    ++the_card.shows;
}

static void move_to(void* part, int e) {               // a move in x of e 1024ths of a pixel
    if (part == 0) return;
    int matrix[9] = { 1024, 0, 0, 0, 1024, 0, e, 0, 1024 };
    op1f_move(part, matrix);
    ++the_card.moves;
}

// The card's parts, looked up once a document. Their names built with no runtime: "door_dot" and the slot's number.
static void* card_part(const char* name) {
    void* node = op1f_node(page.document, name);
    if (node == 0 && the_card.missing++ == 0) {
        u32 n = 0;
        for (; name[n] && n + 1 < sizeof the_card.first_missing; ++n) the_card.first_missing[n] = name[n];
        the_card.first_missing[n] = 0;
    }
    return node;
}

static u32 put_text(char* to, u32 at, const char* text);
static u32 put_number(char* to, u32 at, u32 v);

static void card_find() {
    if (the_card.found && the_card.document == page.document) return;
    the_card.found = 1;
    the_card.document = page.document;
    the_card.missing = 0;
    for (u32 i = 0; i < (u32)PARTS; ++i) {
        the_card.part[i] = card_part(CARD_PART[i]);
        the_card.shown[i] = SHOWN_UNKNOWN;
    }
    for (u32 i = 0; i < (u32)DOTS_MOST; ++i) {
        char name[24];
        u32 at = put_text(name, 0, "door_dot");
        at = put_number(name, at, i);
        the_card.dot[i] = card_part(name);
        put_text(name, at, "_off");
        the_card.dot_off[i] = card_part(name);
        put_text(name, at, "_on");
        the_card.dot_on[i] = card_part(name);
        the_card.dot_shown[i] = the_card.off_shown[i] = the_card.on_shown[i] = SHOWN_UNKNOWN;
    }
    the_card.laid = 0;
    the_card.now_at = the_card.warning_at = NO_STEP - 1;        // neither moved yet
    the_card.lit_set = 0;
    if (the_card.missing)
        STOCK_LOG(1, LOG_FILE, 53, "door: the card's picture %s lacks %u of its %u parts, the first %s: those are not shown, hidden or moved",
                  PICTURE, the_card.missing, (u32)PARTS + 3u * (u32)DOTS_MOST, the_card.first_missing);
    else
        STOCK_LOG(3, LOG_FILE, 53, "door: the card's picture %s: its %u parts found", PICTURE, (u32)PARTS + 3u * (u32)DOTS_MOST);
}

// A word's label, made once with its alignment: the part looked up, its paint asked first (a paint of 0 or past the
// document's paints stops the unit in the label maker), then the firmware's label maker. 0 when it cannot be.
static void* card_label(CardText* t) {
    if (t->label || t->failed) return t->label;
    void* node = op1f_node(page.document, t->part);
    u32 paint = node ? NODE_PAINT_OF(node) : 0u;
    if (node == 0 || paint == 0 || (paint - 1u) * 20u >= DOCUMENT_PAINT_BYTES(page.document)) {
        t->failed = 1;
        STOCK_LOG(1, LOG_FILE, 52, "door: the card's part %s cannot take a text (%s): its words are not shown", t->part, node ? "it is no group with a stroke" : "no part of that name");
        return 0;
    }
    u32 held[4] = { 0, (u32)page.document, 0, 0 };     // no screen holds it: the label maker reads the word at +4 and nothing else
    t->label = STOCK_LABEL_FROM_NODE(held, node, (int)t->align);
    if (t->label == 0) t->failed = 1;
    return t->label;
}

// A word laid out in small letters when it changed (the layout allocates); an empty one hides the label. `width` 0:
// the label's own width (a word wider is shortened by the layout with a mark).
static void card_text(u32 i, const char* text, u32 width = 0) {
    CardText* t = &card_texts[i];
    void* label = card_label(t);
    if (label == 0) return;
    char small[LINE_BYTES];
    u32 n = 0;
    for (; n + 1 < (u32)LINE_BYTES && text[n]; ++n) {
        u8 c = (u8)text[n];
        if (c < 0x20 || c > 0x7e) c = '?';              // the font's lookup takes a byte from 0x80 up past its table
        small[n] = c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : (char)c;
    }
    small[n] = 0;
    if (t->laid) {
        u32 k = 0;
        while (small[k] && small[k] == t->text[k]) ++k;
        if (small[k] == t->text[k]) return;
    }
    STOCK_LABEL_TEXT(label, small, (int)(width ? width : t->width));
    for (u32 k = 0; k <= n; ++k) t->text[k] = small[k];
    t->laid = 1;
}

// The laid-out width of a word in pixels, by the firmware's own box of its glyphs (as op1f_text_width, level 4): 0 for
// none or an empty one.
static int card_measure(u32 i) {
    CardText* t = &card_texts[i];
    if (t->label == 0 || !t->laid || t->text[0] == 0) return 0;
    LabelBox box = STOCK_LABEL_BOUNDS(t->label);                                  // by value
    int width = (box.x_max - box.x_min) >> 10;
    return width > 0 ? width : 0;
}

// The slots moved to the places of d dots with the gap before slot g, when those change: dot i at
// x0 + (10 i + 6 [i >= g]) x 276 / (10 (d - 1) + 6 [a gap]), in 1024ths of a pixel.
static void card_lay(u32 d, u32 g) {
    if (the_card.laid && the_card.laid_d == d && the_card.laid_g == g) return;
    the_card.laid = 1;
    the_card.laid_d = d;
    the_card.laid_g = g;
    u32 gap = g < d ? (u32)GAP_TENTHS : 0u;
    u32 units = d > 1 ? 10u * (d - 1u) + gap : 0u;
    for (u32 i = 0; i < d; ++i) {
        u32 tenths = 10u * i + (i >= g ? gap : 0u);
        u32 x_q = (u32)ROW_X0 * 1024u + quotient(tenths * (u32)(ROW_LAST - ROW_X0) * 1024u, units);
        the_card.x_q[i] = x_q;
        move_to(the_card.dot[i], (int)(x_q - (u32)ROW_X0 * 1024u));
    }
    the_card.now_at = the_card.warning_at = NO_STEP - 1;        // moved again at their next showing
}

// The card at one moment.
static void card_draw(const CardView* v) {
    if (!page.document) return;
    card_find();
    ++the_card.draws;
    for (u32 i = 0; i < (u32)TEXTS; ++i) card_text(i, v->text[i]);
    set_shown(the_card.part[CP_BAND], &the_card.shown[CP_BAND], v->grey ? 0u : 1u);
    set_shown(the_card.part[CP_GREY], &the_card.shown[CP_GREY], v->grey ? 1u : 0u);
    set_shown(the_card.part[CP_KEYS], &the_card.shown[CP_KEYS], v->keys ? 1u : 0u);
    set_shown(the_card.part[CP_ROW], &the_card.shown[CP_ROW], v->row ? 1u : 0u);
    if (!v->row) return;                                // what is inside the row stays as it was, hidden with it
    u32 n = v->steps, d = dots_of(n), g = gap_of(n, d, v->checks);
    card_lay(d, g);
    u32 now = v->current != NO_STEP && v->current < n ? dot_of_step(n, d, v->current) : NO_STEP;
    u32 reach = NO_STEP;                                // the last dot lit: the bar runs to it
    for (u32 i = 0; i < (u32)DOTS_MOST; ++i) {
        u32 used = i < d, lit = used && (dot_done(n, d, v->done, i) || i == now);
        if (lit) reach = i;
        set_shown(the_card.dot[i], &the_card.dot_shown[i], used);
        if (!used) continue;
        set_shown(the_card.dot_on[i], &the_card.on_shown[i], lit);
        set_shown(the_card.dot_off[i], &the_card.off_shown[i], lit ? 0u : 1u);
    }
    if (now != NO_STEP && the_card.now_at != now) {
        move_to(the_card.part[CP_NOW], (int)(the_card.x_q[now] - (u32)ROW_X0 * 1024u));
        the_card.now_at = now;
    }
    set_shown(the_card.part[CP_NOW], &the_card.shown[CP_NOW], now != NO_STEP);
    u32 w = v->warning != NO_STEP && n ? dot_of_step(n, d, v->warning < n ? v->warning : n - 1u) : NO_STEP;
    if (w != NO_STEP && the_card.warning_at != w) {
        move_to(the_card.part[CP_WARNING], (int)(the_card.x_q[w] - (u32)ROW_X0 * 1024u));
        the_card.warning_at = w;
    }
    set_shown(the_card.part[CP_WARNING], &the_card.shown[CP_WARNING], w != NO_STEP);
    set_shown(the_card.part[CP_BOLT], &the_card.shown[CP_BOLT], v->bolt ? 1u : 0u);
    // The red bar from the first dot to the last one lit (to the bolt at the restart): x scaled about x0, so that x0
    // stays and the bolt's end lands on the reach: a = (reach - x0) / 316, e = x0 x (1 - a), in 1024ths.
    u32 lit_q = v->bolt ? (u32)ROW_BOLT * 1024u : reach != NO_STEP ? the_card.x_q[reach] : (u32)ROW_X0 * 1024u;
    if (lit_q > (u32)ROW_X0 * 1024u && (!the_card.lit_set || the_card.lit_q != lit_q) && the_card.part[CP_LIT]) {
        u32 a = quotient(lit_q - (u32)ROW_X0 * 1024u, (u32)ROW_LENGTH);
        int matrix[9] = { (int)a, 0, 0, 0, 1024, 0, (int)((u32)ROW_X0 * 1024u - (u32)ROW_X0 * a), 0, 1024 };
        op1f_move(the_card.part[CP_LIT], matrix);
        ++the_card.moves;
        the_card.lit_q = lit_q;
        the_card.lit_set = 1;
    }
    set_shown(the_card.part[CP_LIT], &the_card.shown[CP_LIT], lit_q > (u32)ROW_X0 * 1024u);
}

// A text of our own put together with no runtime: a word, a number (by subtraction), each at most `room` bytes in all.
static u32 put_text(char* to, u32 at, const char* text) {
    while (*text && at + 1 < (u32)LINE_BYTES) to[at++] = *text++;
    to[at] = 0;
    return at;
}

static u32 put_number(char* to, u32 at, u32 v) {
    static const u32 TENS[10] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };
    int started = 0;
    for (u32 i = 0; i < 10; ++i) {
        u32 d = 0;
        while (v >= TENS[i]) {
            v -= TENS[i];
            ++d;
        }
        if ((d || started || i == 9) && at + 1 < (u32)LINE_BYTES) {
            to[at++] = (char)('0' + d);
            started = 1;
        }
    }
    to[at] = 0;
    return at;
}

// What an apply's page says: the question, the counts (`1 new, 1 updated,
// 1 removed`; where that runs past 26 characters (build 4: 27), `12 files, 32 removed`, the files that come and the removals; and
// `the base too` when base.pack comes: where the counts and those words would run past 24 characters, the question
// says `change packs and base?` instead), and the note (`closes in 60 s`; `, a test` for commit 02). Build 5: the
// words are build 4's; where they stand is the card's.
static u32 counts_long(char* counts) {
    u32 at = 0;
    counts[0] = 0;
    if (apply.new_n) {
        at = put_number(counts, at, apply.new_n);
        at = put_text(counts, at, " new");
    }
    if (apply.replaced_n) {
        if (at) at = put_text(counts, at, ", ");
        at = put_number(counts, at, apply.replaced_n);
        at = put_text(counts, at, " updated");
    }
    if (apply.removals_n) {
        if (at) at = put_text(counts, at, ", ");
        at = put_number(counts, at, apply.removals_n);
        at = put_text(counts, at, " removed");
    }
    return at;
}

// Build 5: whether the full counts fit the card's line, measured once a page (counts_measure): 0 not yet, 1 they fit, 2
// they do not and the short form stands.
static u32 counts_judged;

static const char* apply_question(char* counts) {
    u32 at = counts_long(counts);
    if (at > (u32)COUNTS_LONGEST || counts_judged == 2) {
        u32 files = apply.new_n + apply.replaced_n;
        at = put_number(counts, 0, files);
        at = put_text(counts, at, files == 1 ? " file" : " files");
        if (apply.removals_n) {
            at = put_text(counts, at, ", ");
            at = put_number(counts, at, apply.removals_n);
            at = put_text(counts, at, " removed");
        }
    }
    if (!apply.base) return "change the packs?";
    if (at + 14 <= (u32)BASE_TOO_LONGEST) {
        put_text(counts, at, at ? ", the base too" : "the base too");
        return "change the packs?";
    }
    return "change packs and base?";
}

static void with_test(char* note, u32 at) {
    if (apply.test_k) put_text(note, at, ", a test");
}

// The row of an apply: each check, then each change (a start that finishes: its changes only), and how many are done.
static void apply_row(CardView* v, u32 changes_done) {
    u32 changes = apply.removals_n + apply.moves_n;
    v->row = 1;
    v->checks = apply.finishing ? 0u : apply.moves_n;
    v->steps = v->checks + changes;
    v->done = v->checks + changes_done;
}

// The plain page of protocols 1 and 2 (a restart, nothing applied): the question, the keys, the note under the card.
static void plain_page(const char* note) {
    CardView v;
    view_start(&v);
    v.text[TX_QUESTION] = "restart the op-1?";
    v.text[TX_T1] = "t1 yes";
    v.text[TX_T2] = "t2 no";
    v.text[TX_NOTE] = note;
    v.keys = 1;
    card_draw(&v);
}

// Build 5: at the apply's page's first frame, the full counts laid out on their label with no bound and measured by the
// firmware's own layout; wider than the card's line (W_CARD), the short form stands for the page. One line says it.
static void counts_measure() {
    if (counts_judged || !page.document) return;
    char counts[LINE_BYTES];
    u32 at = counts_long(counts);
    counts_judged = 1;
    if (at == 0 || at > (u32)COUNTS_LONGEST) return;                              // nothing to measure, or the short form by its length
    card_find();
    card_text(TX_COUNTS, counts, W_MEASURE);
    int width = card_measure(TX_COUNTS);
    if (width > (int)W_CARD) counts_judged = 2;
    STOCK_LOG(3, LOG_FILE, 54, "door: the counts '%s' are %d pixels wide on the card's line of %u: %s", counts, width, (u32)W_CARD,
              counts_judged == 2 ? "the short form" : "in full");
}

static void countdown(u32 left) {
    char note[LINE_BYTES];
    u32 at = put_text(note, 0, "closes in ");
    at = put_number(note, at, left > 99 ? 99 : left);
    at = put_text(note, at, " s");
    if (!asking_apply) {
        plain_page(note);
        return;
    }
    with_test(note, at);
    counts_measure();
    char counts[LINE_BYTES];
    CardView v;
    view_start(&v);
    v.text[TX_QUESTION] = apply_question(counts);
    v.text[TX_COUNTS] = counts;
    v.text[TX_T1] = "t1 apply and restart";
    v.text[TX_T2] = "t2 cancel";
    v.text[TX_NOTE] = note;
    v.keys = 1;
    card_draw(&v);
}

// The card while the door applies, once it has made its change, stopped, or could not finish: the words in the band
// (`checking 1 of 2`, `changing 2 of 3`, `restarting`, `not finished:`, `finishing the last change`), the row of dots
// with the step under way, a start's `2 of 3` and `switch off and on` under the row.
static void apply_words(u32 s) {
    char title[LINE_BYTES], under[LINE_BYTES];
    CardView v;
    view_start(&v);
    title[0] = under[0] = 0;
    u32 at = 0, changes = apply.removals_n + apply.moves_n;
    v.text[TX_QUESTION] = title;
    v.text[TX_UNDER] = under;
    if (s == UNFINISHED) {
        apply_row(&v, apply.changes);
        v.warning = v.done;                             // the step that could not be made
        v.text[TX_QUESTION] = "not finished:";
        v.text[TX_UNDER] = "switch off and on";
        card_draw(&v);
        return;
    }
    if (s == RESTARTS) {                                // after the change, a test stop (`restarting, a test`) or a finishing
        apply_row(&v, apply.outcome == P_TEST ? apply.changes : changes);
        v.bolt = 1;
        at = put_text(title, 0, "restarting");
        if (!apply.finishing) with_test(title, at);
        card_draw(&v);
        return;
    }
    if (apply.step == STEP_CHECK || apply.step == STEP_NOTE) {
        if (apply.finishing) {
            at = put_number(under, 0, 1);
            at = put_text(under, at, " of ");
            put_number(under, at, changes);
            apply_row(&v, 0);
            v.current = 0;
        } else {
            // The files checked so far: apply.at while it checks; every one while the note is written (check_step sets
            // apply.at back to 0 for the changes when it hands over to the note: build 4's words said `checking 1 of 2`
            // for that one frame, build 5's say `checking 2 of 2`).
            u32 checked = apply.step == STEP_NOTE ? apply.moves_n : apply.at < apply.moves_n ? apply.at : apply.moves_n;
            at = put_text(title, 0, "checking ");
            u32 i = checked + 1 > apply.moves_n ? apply.moves_n : checked + 1;
            at = put_number(title, at, i);
            at = put_text(title, at, " of ");
            at = put_number(title, at, apply.moves_n);
            with_test(title, at);
            apply_row(&v, 0);
            v.done = checked;
            v.current = v.done;
        }
    } else {
        u32 j = apply.changes + 1 > changes ? changes : apply.changes + 1;
        char* to = apply.finishing ? under : title;
        at = apply.finishing ? 0u : put_text(title, 0, "changing ");
        at = put_number(to, at, j);
        at = put_text(to, at, " of ");
        at = put_number(to, at, changes);
        if (!apply.finishing) with_test(title, at);
        apply_row(&v, apply.changes);
        v.current = v.done;
    }
    if (apply.finishing) v.text[TX_QUESTION] = "finishing the last change";
    card_draw(&v);
}

static void close_page(u32 ms, u32 why) {
    page.closing = 1;
    page.closing_since = ms;
    page.closing_shown = CANCELLED_SHOWN_MS;
    page.why = why;
    if (!page.document) return;
    if (asking_apply) {                                 // an apply's page: nothing changed (or the restart did not come), three seconds
        page.closing_shown = STOPPED_SHOWN_MS;
        CardView v;
        view_start(&v);
        if (why == ENDED_NO_RESTART) {                  // the change was made, the restart did not come
            apply_row(&v, apply.removals_n + apply.moves_n);
            v.text[TX_QUESTION] = "switch off and on";
        } else {                                        // nothing changed: the band grey, no dot lit
            apply_row(&v, 0);
            v.done = 0;
            v.grey = 1;
            v.text[TX_QUESTION] = "nothing changed";
        }
        card_draw(&v);
        return;
    }
    plain_page("cancelled");
}

// An apply stopped before any change, after T1: `nothing changed` for three seconds, then the page closes.
static void page_stopped(u32 ms) {
    if (page.up) close_page(ms, WHY_APPLY_STOPPED);
}

static void said_keyboard();

static void said_closed(u32 ms) {
    STOCK_LOG(3, LOG_FILE, 7, "door: the confirm page closed: %s; %u ms up, %u key events and %u turns taken, %u T1 events with T1 not held",
              why_text(page.why), ms - page.since, keys_taken - page.keys_at_up, turns_taken - page.turns_at_up, t1_not_held - page.t1_at_up);
    said_keyboard();
}

// Build 2: what the door saw of the keyboard while the page asked, beside what the firmware's words say.
static void said_keyboard() {
    u32 object = op1f_door_keys[KEYS_OBJECT];
    STOCK_LOG(3, LOG_FILE, 17, "door: the keyboard while the page asked: %u reports, the door's copy %08x %08x, the firmware stores the map at 0x%08x, its words at 0x%08x hold %08x %08x",
              op1f_door_keys[KEYS_COUNT] - page.reports_at_up, op1f_door_keys[KEYS_LOW], op1f_door_keys[KEYS_HIGH], object ? object + (u32)MAP_AT_OBJECT : 0u,
              (u32)KEY_MAP, RD32(KEY_MAP), RD32(KEY_MAP_HIGH));
}

// Build 2: the totals since the start, at the power popup and every five minutes while they changed (the quiet gate).
static QuietGate totals_gate;
static u32 totals_next_ms;

static void say_totals(u32 ms, int why) {
    STOCK_LOG(3, LOG_FILE, 19, "door: totals (%s): %u messages, %u answers sent, %u not, %u other SysEx dropped (%u not beginning F0), %u visits, a visit on %u, %u keyboard reports",
              quiet_why(why), total.messages, total.answers, total.unsent, total.dropped, total.not_f0, total.visits, visit_on, op1f_door_keys[KEYS_COUNT]);
    if (total.messages)
        STOCK_LOG(3, LOG_FILE, 21, "door: totals: the last message %u ms ago, command 0x%02x, number %u, %u bytes",
                  ms - last_message_ms, total.last_command, total.last_number, total.last_length);
    if (files_total.begun || files_total.refused)
        STOCK_LOG(3, LOG_FILE, 30, "door: files since the start: %u begun, %u kept, %u removed at their check, %u ended before it, %u refused; %u bytes written, the slowest write %u us",
                  files_total.begun, files_total.kept, files_total.removed_at_check, files_total.ended_before, files_total.refused, files_total.bytes, files_total.slowest_us);
    if (applies_total.asked || applies_total.refused)
        STOCK_LOG(3, LOG_FILE, 49, "door: applies since the start: %u asked, %u refused, %u stopped before a change, %u made; %u removed, %u moved",
                  applies_total.asked, applies_total.refused, applies_total.stopped, applies_total.made, applies_total.removed, applies_total.moved);
}

static u32 totals_signature() {
    u32 sig = quiet_mix(quiet_start(), total.messages);
    sig = quiet_mix(sig, total.dropped);
    sig = quiet_mix(sig, files_total.begun);
    sig = quiet_mix(sig, files_total.refused);
    sig = quiet_mix(sig, applies_total.asked);
    sig = quiet_mix(sig, applies_total.refused);
    return quiet_mix(sig, total.unsent);
}

static void totals(u32 ms) {
    int why = quiet_popup(&totals_gate, RD8(POWER_POPUP_SHOWN) != 0);
    int period = (int)(ms - totals_next_ms) >= 0;
    if (period) {
        totals_next_ms = ms + (u32)TOTALS_MS;
        why |= quiet_period(&totals_gate, totals_signature(), 0) & QUIET_WHY_CHANGE;   // only while they changed: no line at the start, after it or at the floor
    }
    if (why) say_totals(ms, why);
}

// Build 2: a visit that has gone on for a minute says so once, with its counts by command.
static void long_visit(u32 ms) {
    if (!visit_on) {
        long_said = 0;
        return;
    }
    if (long_said || ms - visit_started_ms < (u32)LONG_VISIT_MS) return;
    long_said = 1;
    u32 known_ones = visit.hello + visit.list + visit.hash + visit.sink + visit.commit + visit.status + visit.abort;
    STOCK_LOG(3, LOG_FILE, 20, "door: a visit has gone on for 60 s: %u messages (hello %u, list %u, hash %u, sink %u, commit %u, status %u, abort %u, other %u), the last 0x%02x number %u",
              visit.messages, visit.hello, visit.list, visit.hash, visit.sink, visit.commit, visit.status, visit.abort, visit.messages - known_ones,
              total.last_command, total.last_number);
}

// Build 2: each T1 or T2 event the page took, with the door's copy at that moment.
static u32 ev_said;

static void key_events() {
    u32 n = ev_count;
    while (ev_said < n) {
        u32 i = ev_said++;
        STOCK_LOG(3, LOG_FILE, 18, "door: T%u %s taken while the page asked: the door's copy %08x at that moment, %u keyboard reports since the start",
                  ev_key[i] == (u32)KEY_T1 ? 1u : 2u, ev_what[i] == OP1F_UP ? "up" : "down", ev_word[i], ev_reports[i]);
    }
}

static void say_visit_end() {
    STOCK_LOG(3, LOG_FILE, 4, "door: the visit ended after 10 s without a message: %u messages (hello %u, list %u, hash %u in %u slices of %u files, sink %u with %u bytes, commit %u, status %u, abort %u), %u answers sent, %u not",
              visit.messages, visit.hello, visit.list, visit.hash, visit.slices, visit.files, visit.sink, visit.sink_bytes, visit.commit, visit.status, visit.abort, visit.sent, visit.unsent);
    STOCK_LOG(3, LOG_FILE, 5, "door: in that visit the longest message %u bytes, the slowest %u us (command 0x%02x), %u other SysEx dropped (%u not beginning F0), %u answered 1, 2, 4 or 5",
              visit.longest, visit.slowest_us, visit.slowest_command, visit.dropped, visit.not_f0, visit.refused);
}

// Build 4: the restart asked from the frame: no message within 5 s of an apply's change, its test stop or a start's finishing.
static void ask_restart_here(u32 ms, const char* why) {
    u32 mask;
    int ask = 0;
    MASK_OFF(mask);
    if (state == RESTARTS && !restart_asked) {
        restart_asked = 1;
        ask = 1;
    }
    MASK_ON(mask);
    if (!ask) return;
    restart_ms = ms;
    restart_slow = slow_clock();
    STOCK_LOG(3, LOG_FILE, 50, "door: the firmware's restart is asked (code 20, in %u ms) by the door's frame work: %s", (u32)RESTART_DELAY_MS, why);
    STOCK_RESTART(RESTART_CODE, RESTART_DELAY_MS);
}

// The top page put up (the confirm page, or an apply's page at a start that finishes): 1 when it stands.
static int page_up(u32 ms) {
    if (page.up) return 1;
    if (!page.document && !page.picture_asked) {
        page.picture_asked = 1;
        picture_tries = 1;
        picture_ms = ms;
        page.document = op1f_base_picture(PICTURE);
    }
    if (!page.document && apply.finishing && state == APPLYING && picture_tries < (u32)PICTURE_TRIES && ms - picture_ms >= (u32)PICTURE_RETRY_MS) {
        ++picture_tries;                                // the review's N-5: asked again at a start that finishes
        picture_ms = ms;
        page.document = op1f_base_picture(PICTURE);
    }
    if (!page.document) return 0;
    page.up = 1;
    page.closing = 0;
    counts_judged = 0;                                  // build 5: the counts measured again for this page
    page.since = ms;
    page.clear_t1 = page.clear_t2 = 0;
    page.left = ASK_MS / 1000;
    page.next_second = ms + 1000;
    page.keys_at_up = keys_taken;
    page.turns_at_up = turns_taken;
    page.t1_at_up = t1_not_held;
    page.reports_at_up = op1f_door_keys[KEYS_COUNT];
    op1f_top_open(page.document, door_keys);
    STOCK_LOG(3, LOG_FILE, 12, "door: the confirm page is asked to stand above every page, its document at 0x%08x, made from %s", (u32)page.document, PICTURE);
    return 1;
}

// The start's window: the door's call comes first in the cards unit's frame, so its second call means the start's
// first frame, with every pack's first frame call, is over; START_SETTLE_MS from then on the disk messages are taken.
static DOOR_APART void start_window(u32 ms) {
    if (door_frames < 2) {
        door_frames = door_frames + 1;
        if (door_frames == 2) first_frame_over_ms = ms;
        return;
    }
    u32 since = ms - first_frame_over_ms;
    if (since < (u32)START_SETTLE_MS) return;
    start_settled = 1;
    STOCK_LOG(3, LOG_FILE, 55, "door: the start's first frame was over %u ms ago: messages that read or write the disk are taken from now on, %u answered 13 before",
              since, window_refused);
}

// Once a frame, from the cards unit's frame call (base 18's cards unit), before the top page is placed.
extern "C" void op1f_door_frame(u32 ms) OP1F_NAMED("op1f_door_frame");
extern "C" void op1f_door_frame(u32 ms) {
    if (!start_settled) start_window(ms);
    if (!busy) {                                        // build 4: no file work while a message runs (4.3)
        if (!note_looked) start_note(ms);
        u32 s0 = state;
        if (s0 == APPLYING) apply_frame(ms);
        else if (folder_state == F_EMPTYING && s0 != UNFINISHED) start_empty();
    }
    if (!page.said_ready) {
        page.said_ready = 1;
        STOCK_LOG(3, LOG_FILE, 1, "door: ready: build %u, protocol %u, base %u from %s, release %s; messages F0 7D 4F 46 are the door's at the SysEx handler, at most %u bytes",
                  (u32)DOOR_BUILD, (u32)PROTOCOL_3, op1f_door_base_version, op1f_door_base_from, op1f_release, (u32)LARGEST);
        totals_next_ms = ms + (u32)TOTALS_MS;
        quiet_period(&totals_gate, totals_signature(), 0);   // the start's totals: the first five minutes are held to them
    }
    key_events();
    u32 tunnel = tunnel_count;
    if (tunnel != tunnel_said && tunnel_said < (u32)TUNNEL_SAID) {   // the review's S-1: the first eight times the count moves
        tunnel_said = tunnel_said + 1;
        STOCK_LOG(1, LOG_FILE, 51, "door: %u SysEx message(s) of the firmware's 0x66 service since the start (%u not from the USB poll), the first %02x %02x %02x %02x %02x %02x of %u bytes: an apply is refused until the unit is switched off and on",
                  tunnel, tunnel_other, tunnel_first[0], tunnel_first[1], tunnel_first[2], tunnel_first[3], tunnel_first[4], tunnel_first[5], tunnel_length);
    }
    if (visit_on && ms - last_message_ms >= (u32)VISIT_MS) {
        visit_on = 0;
        say_visit_end();
    }
    u32 t = tstate;
    if (t == T_RECEIVING || t == T_CHECKING) {          // a transfer left open: the power popup, the visit's end, the disk mode
        if (power_popup()) frame_end_transfer(X_POWER);  // review S-1: closed and removed before the firmware's shutdown
        else if (RD32(DISK_OPEN) != 0) frame_end_transfer(X_DISK);
        else if (!visit_on) frame_end_transfer(X_VISIT);
    }
    totals(ms);
    long_visit(ms);
    if (page.up && page.closing) {                     // `cancelled` (or `nothing changed`) shown: the page goes after it
        if (ms - page.closing_since < page.closing_shown) return;
        op1f_top_close();
        page.up = 0;
        page.closing = 0;
        said_closed(ms);
        return;
    }
    u32 s = state;
    if (s == ASKS) {
        if (!page.up) {
            if (!page_up(ms)) {
                if (change(ASKS, IDLE)) {
                    outcome = ENDED_NO_PAGE;
                    outcome_unsaid = 1;
                    apply_not_begun(P_NO_PAGE);
                    STOCK_LOG(1, LOG_FILE, 11, "door: the confirm page cannot be shown: %s could not be made (see the artwork line before)", PICTURE);
                }
                return;
            }
            seconds_left = page.left;
            countdown(page.left);
            return;
        }
        u32 up_for = ms - page.since;
        if (up_for >= (u32)ASK_MS) {
            if (change(ASKS, IDLE)) {
                outcome = ENDED_TIME;
                outcome_unsaid = 1;
                apply_not_begun(P_TIME);
                close_page(ms, ENDED_TIME);
            }
            return;
        }
        while (page.left > 0 && (int)(ms - page.next_second) >= 0) {
            --page.left;
            page.next_second += 1000;
        }
        seconds_left = page.left;
        if (up_for >= (u32)ARMED_MS) {
            // The door's own copy. Build 3 (result 9.3): before the controller's first report it is taken as clear, so
            // the first press after a start counts; a confirm still needs a report with the bit set after the first
            // half second (only the handler's stores write the copy). The known edge: T1 held without a break since
            // before the start, then another key pressed while the page asks, reports T1's bit after an assumed clear
            // and confirms; it needs the owner's finger on T1 the whole time.
            u32 map = op1f_door_keys[KEYS_COUNT] != 0 ? op1f_door_keys[KEYS_LOW] : 0u;
            if ((map & BIT_T2) == 0) page.clear_t2 = 1;
            else if (page.clear_t2) {
                if (change(ASKS, IDLE)) {
                    outcome = ENDED_T2;
                    outcome_unsaid = 1;
                    apply_not_begun(P_T2);
                    close_page(ms, ENDED_T2);
                }
                return;
            }
            if ((map & BIT_T1) == 0) page.clear_t1 = 1;
            else if (page.clear_t1) {
                if (asking_apply) {
                    // Build 4: T1 on an apply's page. The apply runs by itself from the next frame on (the plan's 4.1);
                    // its fields are set once the state is taken (the review's N-4), and it is never a start's finishing
                    // (the review's S-2).
                    if (change(ASKS, APPLYING)) {
                        apply.finishing = 0;
                        apply.step = STEP_CHECK;
                        apply.at = apply.done = 0;
                        apply.astate = A_CHECKING;
                        apply.began_ms = ms;
                        apply.tape_since = 0;
                        apply.slices = apply.read_us = apply.hash_us = 0;
                        restart_from_apply = 0;
                        outcome = ENDED_T1;
                        t1_ms = ms;
                        page.why = ENDED_T1;
                        apply_words(APPLYING);
                        STOCK_LOG(3, LOG_FILE, 38, "door: the apply begins (T1 %u ms after the page came up): %u file(s) to check again, %u bytes, %u removal(s)",
                                  up_for, apply.moves_n, apply.to_check, apply.removals_n);
                    }
                    return;
                }
                if (change(ASKS, RESTARTS)) {
                    outcome = ENDED_T1;
                    t1_ms = ms;
                    page.why = ENDED_T1;
                    restart_from_apply = 0;
                    plain_page("restarting");
                    STOCK_LOG(3, LOG_FILE, 13, "door: T1 held in the door's copy of the keyboard's map %u ms after the page came up: the unit restarts at the page's next message", up_for);
                }
                return;
            }
        }
        countdown(page.left);
        return;
    }
    if (s == APPLYING || s == UNFINISHED) {             // build 4: the page says what the apply does; the keys stay taken
        if (page_up(ms)) apply_words(s);
        return;
    }
    if (s == RESTARTS) {
        if (restart_from_apply) {                       // build 4: after an apply's change, its test stop, or a start that finished
            if (page.up && page.document) apply_words(RESTARTS);
            if (!restart_asked && ms - apply.changed_ms >= (u32)AFTER_CHANGE_MS) {
                ask_restart_here(ms, apply.finishing ? "no message came within 5 s of the start's finishing" : "no message came within 5 s of the change");
            } else if (restart_asked && ms - restart_ms >= (u32)RESTART_WAIT_MS) {
                if (change(RESTARTS, IDLE)) {
                    outcome = ENDED_NO_RESTART;
                    outcome_unsaid = 0;
                    restart_asked = 0;
                    close_page(ms, ENDED_NO_RESTART);   // the change is made: `switch off and on`, then the keys back
                    STOCK_LOG(1, LOG_FILE, 16, "door: the restart was asked %u ms ago and did not come: the confirm page is taken down and the keys given back, outcome 7", ms - restart_ms);
                }
            }
            return;
        }
        if (!restart_asked && ms - t1_ms >= (u32)AFTER_T1_MS) {
            if (change(RESTARTS, IDLE)) {
                outcome = ENDED_NO_MESSAGE;
                outcome_unsaid = 1;
                close_page(ms, ENDED_NO_MESSAGE);
            }
        } else if (restart_asked && ms - restart_ms >= (u32)RESTART_WAIT_MS) {   // mend 1: the unit runs on 15 s after the restart was asked
            if (change(RESTARTS, IDLE)) {
                outcome = ENDED_NO_RESTART;
                outcome_unsaid = 0;                     // said in the status answer as status 0, outcome 7
                restart_asked = 0;
                close_page(ms, ENDED_NO_RESTART);
                STOCK_LOG(1, LOG_FILE, 16, "door: the restart was asked %u ms ago and did not come: the confirm page is taken down and the keys given back, outcome 7", ms - restart_ms);
            }
        }
        return;
    }
    if (page.up) close_page(ms, outcome ? outcome : (u32)ENDED_ABORT);   // ended from main_task (abort, the guard)
}

#ifdef OP1F_HOST
// The PC's model asks the door's state, to hold that no message makes a file call in the apply's states.
extern "C" u32 host_door_state() { return state; }
#endif

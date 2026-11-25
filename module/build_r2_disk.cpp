// Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's.
// Step R10, the base pack (base.pack 34): the firmware's own log files shown as a folder `logs` at the top
// (module/build_r10_logs.cpp). The root's child maker asks the logs unit first (op1f_logs_child), then the folders
// unit, then the firmware; the population has the logs unit make its folder (op1f_logs_populate) before the folders
// unit makes the packs' folders. The rest holds as it is described below.
//
// Step R2, the base pack (base.pack 21): the disk mode's folders of a base that knows no pack by name. A sibling of
// M3's disk unit, which stays as it was for the bases before. Only the loader's two hand-overs remain, under
// the names it looks them up by: the root's child maker asks the folders unit (base 19's folders unit and its
// siblings) for a folder a card asked to be shown, and hands every other name to the firmware; the population has the
// folders unit make those folders. The base's own folders fx and lfo, with their sixty-four slots, the delete that
// wrote an empty slot's file, the old folder packs and the counters are gone: the folder packs is the loader's own.
//
// What follows is the elder's description of the two hand-overs, which holds but where it names fx, lfo and the old
// packs folder, and what was read of the firmware's tree, which the two meet.
//
// Step M3, the base pack: the folders of the disk mode that the base answers the loader for. This is
// Build H's disk unit, which is kept as it is for every base before
// M3, with two calls added and nothing else changed: the root's child maker asks the folders unit
// (M3's folders unit, op1f_folders_child) for a name that is not fx or lfo before it hands the name to
// stock, and the population, after fx, lfo and the old packs folder, has the folders unit make the folders the cards
// asked to be shown (op1f_folders_populate). The test of the two texts holds it (for tests this tree does not hold).
//
// What this unit uses of the firmware's disk mode (docs/methods.md says how such things are found):
//   * The disk mode shows a tree of folders and files that the firmware builds when the mode starts; it is no copy
//     of the disk's folders. 0x8fcaeb10(parent, name) makes a folder in the tree and 0x8fcaed68(folder, name) adds a
//     file to one. What a folder does is an object beside it, its behaviour, with six functions in this order: two
//     that take it away (and free nothing when the object is static), one with nothing to do, one that answers 0,
//     one that answers a folder inside it by its name (a behaviour, or 0 for none), and one that answers a file by
//     its name (a file object, or 0 to refuse the name). The root folder's function for a folder's name is
//     0x8fcacea0, and the word that holds its address is 0x8490fc4c.

typedef unsigned int u32;

typedef void* (*RootChildFn)(void* behaviour, const char* name);             // 0x8fcacea0: stock's root child maker

#define STOCK_ROOT_CHILD ((RootChildFn)0x8fcacea0u)

// ---------------------------------------------------------------- the hooks

// M3: the folders the cards asked to be shown (M3's folders unit): a folder's behaviour by its name, or 0;
// and the folders made in the tree with their files.
extern "C" void* op1f_folders_child(const char* name) asm("op1f_folders_child");
extern "C" void op1f_folders_populate(void* root) asm("op1f_folders_populate");

// R10: the firmware's log files (module/build_r10_logs.cpp): the folder `logs`'s behaviour by its name, or 0; and the
// folder made in the tree with its files.
extern "C" void* op1f_logs_child(const char* name) asm("op1f_logs_child");
extern "C" void op1f_logs_populate(void* root) asm("op1f_logs_populate");

// The root behaviour's +0x10, which the loader hands every name that is not its own folder "packs": a folder a card
// asked to be shown, else the firmware's own.
extern "C" void* op1f_mtp_root_child(void* behaviour, const char* name) asm("op1f_mtp_root_child");
extern "C" void* op1f_mtp_root_child(void* behaviour, const char* name) {
    void* logs_folder = op1f_logs_child(name);                             // R10: the firmware's log files, first
    if (logs_folder) return logs_folder;
    void* card_folder = op1f_folders_child(name);                          // M3: a folder a card asked to be shown
    if (card_folder) return card_folder;
    return STOCK_ROOT_CHILD(behaviour, name);
}

// Called by the loader with the root folder once it has made the folder "packs": the folders the cards asked to be
// shown.
extern "C" void op1f_mtp_populate_fx(void* root) asm("op1f_mtp_populate_fx");
extern "C" void op1f_mtp_populate_fx(void* root) {
    op1f_logs_populate(root);                                              // R10: the folder logs, before the packs'
    op1f_folders_populate(root);                                           // M3: the cards' folders
}

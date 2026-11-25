# The firmware's functions our code uses

Made by `tools/interfaces.py` from the sources and from `profiles/op1f-1.7.3-interfaces.json`; change those, not this page.

This page lists the functions of the firmware that open-field's own code calls, for OS 1.7.3. For each it says where the function stands, what the caller hands over, what comes back and which thread our code calls it on. It says nothing of how a function works inside: that is not needed to call it, and it is not written down in this repository ([CONTRIBUTING.md](../CONTRIBUTING.md), "Writing about the firmware").

**Every address belongs to one firmware file**, OS 1.7.3. On another version each one has to be found again ([methods](methods.md), "Moving the work to another firmware version").

**How a source binds a function.** Two lines: the function's type, and a name for its address. The type is ours; nothing of the firmware is copied.

```cpp
typedef int (*FileSizeFn)(const char* path);
#define STOCK_FILE_SIZE ((FileSizeFn)0x8c051f9cu)
```

**How arguments travel.** The first three go in the registers R0, R1 and R2, the rest on the stack. A result comes back in R0. A function that gives back a structure gets the address for it in P0. The compiler does all of that when the type is right, so a wrong type is a crash: before you bind a function that is not on this page, read how the firmware itself calls it ([methods](methods.md), "Reading a caller before you bind a function").

**The threads.** *The main thread* runs the start-up. *The display thread* draws a frame some sixty times a second; the shared part calls our frame functions from it. *The key thread* hands key presses and knob turns to the mode that is shown. *The audio thread* makes the sound, a block of 64 samples at a time, and must never wait: no log line, no file, no memory asked for there. Where the last column says "ours", it names the threads our code calls the function on today; that it is safe on another thread has not been shown.

There are 112 functions bound in the sources. 112 are described below, by subject. The other 0 are listed at the end with their type and the files that bind them: their words are still to be written.

## The log

The unit writes a log into the folder `logs` of its disk. [Methods](methods.md) says how to copy a log off the unit and how to read one.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c053a20` | `void log(int level, const char* file, int line, const char* format, ...)` | Writes one line into the unit's log. `level`: 3 for a line of information, 2 for a warning, 1 for an error. `file`: a text in the form of a source path; the log shows what stands between its last slash and its dot as the line's tag, so `op1f/mypack.cpp` gives the tag `mypack`. A text without a slash and a dot gives a line with no tag. `line`: a number shown in brackets behind the tag. `format` and the values behind it: as `printf` takes them. A line is cut at about 260 characters, the time and the tag counted. | Any thread but the audio thread: keep a count there and print it from a frame function. Ours: the main thread at the start, the display thread, the key thread, the tape's background thread. |
| `0x8c053a50` | `void assert(const char* message)` | Writes `message` into the log, then stops the unit for good: it never comes back. Our code puts an endless loop after it all the same. | Ours: the main thread at the start. |

## Files on the unit's disk

A path is a C text, counted from the top of the unit's disk, with no slash in front: `user/pk/kit.pack`. There are two families of calls. Use one family for a file from its open to its close.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c051f9c` | `int file_size(const char* path)` | The size of the file at a path, in bytes; -1 when there is no such file. It writes nothing into the log, so it is the way to ask whether a file is there. | Not the audio thread. Ours: the main thread at the start, the display thread, the disk mode's thread. |
| `0x8fc8a9d8` | `int file_open(const char* path, const char* mode)` | Opens a file. `mode` is a text as C's `fopen` takes it; our code hands over the firmware's own text for reading, `rb`, which stands at `0x848daec0`. Gives back a descriptor, a small number that may be 0. A negative number means the open failed, and the function has then written a line into the log itself. | Not the audio thread. Ours: the main thread at the start and when a screen is made, and the display thread. |
| `0x8fc8a9a8` | `int file_read(void* buffer, int size, int count, int descriptor)` | Reads `count` items of `size` bytes into the buffer. Gives back the number of items read. The descriptor is the fourth argument, so it travels on the stack. | As `file_open`. |
| `0x8fc8a994` | `int file_close(int descriptor)` | Closes a file opened with `file_open`. | As `file_open`. |
| `0x8c0a3630` | `int file_seek(int descriptor, u32 offset_low, u32 offset_high, int from)` | Goes to a place in a file opened with `file_open`. The offset is 64 bits wide and is handed over as two words, the low one first. `from`: 0 counts from the file's start, 1 from the present place, 2 from the file's end. Gives back the low word of the new place. | As `file_open`. |
| `0x8c05206c` | `int make_folders(const char* path)` | Makes the folders of a path that are missing. Only a part that a slash ends is made: `user/fx/` makes both folders, `user/fx` makes `user` alone. | Not the audio thread. |
| `0x8fcbefb4` | `int file_remove(const char* path)` | Removes the file at a path. Gives back 0 when the file is gone. | Ours: the disk mode's thread. |
| `0x8c051e28` | `void* dir_open(const char* path)` | Opens a folder to list it. The path has no slash at its end. Gives back a handle, or 0 when the folder cannot be opened. | Not the audio thread. Ours: the main thread at the start, the disk mode's thread. |
| `0x8c0a4bd0` | `void* dir_next(void* handle)` | The next entry of a folder opened with `dir_open`, or 0 at the end. The entry's name is the text that starts 11 bytes into what comes back. Do not count on the order of the entries: our code sorts the names itself. | As `dir_open`. |
| `0x8c0a57d4` | `int dir_close(void* handle)` | Closes a folder opened with `dir_open`. | As `dir_open`. |
| `0x8c051f88` | `int rename(const char* from, const char* to)` | Renames a file: `from` and `to` are paths as `file_size` takes them. Both must be counted from the top of the disk or both begin with a slash: one of each stops the unit. Not 0 when the rename was made. The file system behind it replaces a target that is there in one step. | Not the audio thread. Ours: the display thread, the main task when it handles a SysEx message. |
| `0x8c052054` | `int is_folder(const char* path)` | Not 0 when the path opens as a folder, written without a slash at its end. It opens the folder and closes it again. | As `rename`. |
| `0x8c0a4730` | `long long free_space(const char* path)` | The free bytes of the file system the path names, as one number of 64 bits (R1:R0); -1 on a failure. Our code hands over `/yaffs2`, the text the firmware's own callers hand over. | Ours: the main task when it handles a SysEx message. |
| `0x8fcba988` | `int open(const char* path, const char* mode)` | The second family: the calls of the firmware's file class. Opens a file and gives back a handle, or -1 with a line in the log. `mode` is one of four texts, compared as texts: `rb`, `ab`, `r+b` and `w+b`, which makes the file anew. Any other mode is refused with a line. A path that begins with a slash is taken from the device's root; any other is counted from the top of the unit's disk. | Not the audio thread. Ours: the display thread, the disk mode's thread, the main task when it handles a SysEx message. |
| `0x8fcba954` | `int read(void* buffer, u32 size, u32 count, int handle)` | Reads `count` items of `size` bytes from a file opened with `open`. Gives back the number of items read. The handle is the fourth argument, so it travels on the stack. | As `open`. |
| `0x8fcba8c0` | `int write(const void* data, u32 size, u32 count, int handle)` | Writes `count` items of `size` bytes into a file opened with `open`. Gives back the number of items written, or 0 with the line `write error` in the log. The handle is the fourth argument. | As `open`. |
| `0x8fcba8ac` | `void close(int handle)` | Closes a file opened with `open`. | As `open`. |

## JSON values and JSON files

The firmware keeps its settings as JSON, and so does our code. A JSON value is a small object of 12 bytes in memory the caller provides (our code keeps 16). It owns what it holds: make it with `json_ctor`, give it back with `json_dtor`. What `json_at`, `json_get` and `json_lookup` give back is a reference into another value, not a value of the caller's to destroy. A value has a kind: 0 nothing (null), 1 an integer, 2 an unsigned number, 3 a number with a fraction, 4 a text, 5 true or false, 6 an array, 7 an object. A number read from a file has the kind 2 or 3, never 1: do not test for 1, ask `json_int`.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c085ba8` | `void json_ctor(void* value, int kind)` | Makes a value of a kind in the caller's 12 bytes. Kind 0 makes a null, which becomes an array at its first `json_push`. | Not the audio thread. Ours: the main thread at the start, the display thread, the key thread. |
| `0x8c0844e0` | `void json_dtor(void* value)` | Gives back what a value holds. Call it for every value made with `json_ctor`, and for none that is a reference. | As `json_ctor`. |
| `0x8c0534d0` | `int json_read_file(const char* path, void* document)` | Reads a JSON file and parses it into a value the caller has made. Not 0 when the file was read and parsed. | As `json_ctor`. |
| `0x8c053168` | `int json_write_file(void* path, void* document)` | Writes a value as a JSON file. The path is a text object of the firmware (`string_ctor` below), not a C text. The file is written under a spare name in the folder `tmp` and then renamed over the target. 0 comes back when that failed: look at the answer, and make the target's folder first. | As `json_ctor`. |
| `0x8c084840` | `int json_int(const void* value)` | A number as an `int`. It takes all three kinds of number. For any other kind it gives back 0 and writes a line into the log. | As `json_ctor`. |
| `0x8c084c22` | `u32 json_size(const void* value)` | How many elements an array has. Ask it of an array only. | As `json_ctor`. |
| `0x8c084ee0` | `const void* json_at(const void* value, u32 index)` | An array's element by its number, from 0: a reference into the array. | As `json_ctor`. |
| `0x8c085068` | `int json_has(const void* value, const char* key)` | Not 0 when an object has a member of that name. | As `json_ctor`. |
| `0x8c084f74` | `const void* json_get(const void* value, const char* key)` | An object's member by its name: a reference into the object. Our code asks `json_has` first and never asks for a member that is not there. | As `json_ctor`. |
| `0x8c085e84` | `void* json_lookup(void* value, const char* key)` | An object's member by its name, as `json_get` gives it; a member that is not there is made, as a null. The way to add a member: look it up, then assign to it. | As `json_ctor`. |
| `0x8c0845b8` | `void* json_assign(void* to, const void* from)` | Gives a value the content of another. The target may be a reference, such as a member got with `json_lookup`. | As `json_ctor`. |
| `0x8c085d54` | `void* json_push(void* array, const void* value)` | Puts a copy of a value at an array's end. A null becomes an array at its first push. | As `json_ctor`. |
| `0x8c083b1c` | `void json_from_int(void* value, int number)` | Makes a value an integer (the kind 1). | As `json_ctor`. |
| `0x8c083aec` | `void json_from_bool(void* value, int number)` | Makes a value true or false (the kind 5). | As `json_ctor`. |
| `0x8c084430` | `void json_from_cstr(void* value, const char* text)` | Makes a value a text (the kind 4), copied from a C text. | As `json_ctor`. |
| `0x8c084748` | `const char* json_cstring(const void* value)` | The characters of a text value (the kind 4): the value's own, not a copy, good while the value lives and is not changed. A value of any other kind stops the unit: ask the kind first. | Not the audio thread. Ours: the display thread, the key thread, the main task, the firmware's background worker. |
| `0x8c0828e0` | `int json_read_raw(void* a, void* text, void* value, int flag)` | Reads a JSON text that is already in memory into a value. `a` is the firmware's JSON reader, an object on the caller's stack; `text` is a text object of the firmware that holds the whole text to parse; `value` is the value to fill; the firmware's own callers hand 1 for `flag`. Not 0 when the text was parsed. Our code calls it only in place of the firmware's own call, with the arguments that call came with. | Ours: in place of the firmware's own call in the synth's read of a preset, on the firmware's background worker. |

## Texts and memory

Our code has no library of its own: no `malloc`, no `memcpy`, no division by a variable. What it needs of memory it keeps in its own data, or asks of the firmware with the calls below.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c03e434` | `void string_ctor(void* out, const char* text)` | Makes a text object of the firmware from a C text, in memory the caller provides (our code keeps 48 zeroed bytes for it). A text of up to fifteen characters lives inside the object, and nothing has to be freed afterwards. Our code keeps its paths that short for this reason: `user/op1f.json` fits, `user/lfo/01.json` does not. | Not the audio thread. Ours: the main thread at the start, the display thread, the key thread, the disk mode's thread. |
| `0x8fcc7560` | `void* alloc(u32 bytes)` | A block of the firmware's own heap, the one its screens and pictures come from. It never gives back 0: when the heap has no room, the firmware writes an error into the log and stops. So bound what you ask for (our picture maker asks for a megabyte at most) and give every block back with `free` at `0x8fcc6c30`. | Not the audio thread. Ours: the main thread when a screen is made, and the display thread. |
| `0x8fcc6c30` | `void free(void* block)` | Gives back a block got with `alloc` at `0x8fcc7560`. | As `alloc`. |
| `0x8fcc51fc` | `u32 heap_free(int heap_index)` | How many bytes of such a heap are not in use, by the heap's number. | Ours: the display thread. |
| `0x8d4e63bc` | `void* new(u32 bytes)` | A block of `bytes` from the firmware's heap: the C++ `new` of the firmware. As `alloc`, it never gives back 0: with no room the firmware writes an error into the log and stops. Give the block back with `free` at `0x8d4e63cc`. | Not the audio thread. Ours: the main thread at the start, the disk mode's thread. |
| `0x8d4e63cc` | `void free(void* memory)` | Gives back a block of the firmware's heap: one from `new`, or the text of a firmware text object that holds sixteen characters or more. | Ours: the disk mode's thread. |

## Screens, pictures and texts

A screen of the firmware is an object with a picture: an SVG text, parsed into a document whose parts can be found by their id, shown, hidden, moved and given a text. Our own screens are made the same way.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c040298` | `void* parse_svg(char* text)` | Parses a picture, an SVG text that a zero byte ends, into a document. Gives back the document, or 0. Hand it only what the firmware's own pictures use: the path commands M, L, H, V, C, S and z in capitals or small letters, and no attribute its own files lack. A path command it does not know makes it hang, and the unit with it. | Ours: the main thread when a screen is made, and the display thread. |
| `0x8fc26398` | `void screen_ctor(void* object, int number, const char* name)` | Makes a screen object in memory the caller provides (our code keeps 64 bytes) and registers it under a number. Register a screen once: a number that is taken stops the firmware. In OS 1.7.3 our code uses the numbers 37, 38 and 77, which are free there. Keep a number below 82: the firmware draws the screens in the order of their numbers, and a screen above 82 is drawn over the shutdown countdown. The object's second word is its document: put the parsed picture there. `name` may be 0. | Ours: the display thread, once. |
| `0x8fc25cc0` | `void* find_node(void* screen, const char* id, int optional)` | Finds a part of a screen's picture by its id. With `optional` not 0 it gives back 0 for a part that is not there, and our code always asks that way. The screen must have its document: look at the object's second word before you ask. | Ours: the display thread. |
| `0x8c03dbbe` | `void apply_matrix(void* node, const int* matrix)` | Gives a part of a picture a transform: nine words, each a number with ten bits behind the point, `[a, b, 0, c, d, 0, e, f, 1024]`, for x' = a·x + c·y + e and y' = b·x + d·y + f. Keep the third, sixth and ninth word at 0, 0 and 1024. | Ours: the display thread. |
| `0x8fc25abc` | `void* label_from_node(void* screen, void* node, int align)` | Turns a part of a picture into a label, an object whose text can be set while the unit runs. The part is a placeholder: it gives the label its place and its colour. The label takes the part's place in the document and comes back; keep it, since the id finds it no more. Our code passes 0 for `kind`: the text then starts at the placeholder's left edge. | Ours: the display thread, once for each label. |
| `0x8fc3aa3c` | `void label_text(void* label, const char* text, int width)` | Sets a label's text from a C text. A text wider than `max_width` pixels is shortened and gets a mark at its end. An empty text hides the label. Send small letters and digits: the firmware's font has the letters at the small letters' codes and its own signs at the capitals'. Each call asks the heap for memory, so set a text only when it changes; setting the text a label already has changes nothing. | Ours: the display thread. |
| `0x8fc3820c` | `LabelBox label_bounds(void* label)` | The box of a label's text as it was last laid out: left, right, top and bottom, each a number with ten bits behind the point. It is the box of the shapes without their stroke, so an upright stroke is 0 wide. The box comes back as a structure, by value: bind the function as one that returns the structure, and the compiler hands over the address for it in P0. | Ours: the display thread. |
| `0x8fc64c68` | `void settings_screen_make(void* screen, const char* name)` | Makes a screen of the kind the settings rows KEYBOARD, SYSTEM and MIDI show on the right: up to seven lines, a name and a value each, and one set of seven lines that all such screens share. `screen` is a block of 44 bytes from the allocator; `name` is a text in small letters, which the list on the left shows in capitals. The function writes the screen's table into its first word: our code writes a table of its own over it, its name function first and its draw second. Each call empties the seven shared lines, so make a screen once, at the start. | Ours: the main thread at the start. |
| `0x8fc64de0` | `void settings_screen_draw(void* screen, void* context)` | Draws the right side of such a screen: the first lines of the shared seven, as many as the word at `screen + 0xc` says, in the unit's own letters and colours, with the line the word at `screen + 8` names drawn as the chosen one. `context` is what the firmware hands a screen's draw: pass it on as it came. | Ours: the display thread, from our own screen's draw. |
| `0x8fc64c2c` | `void settings_line_name(void* screen, u32 line, const char* text)` | Sets the name on line `line` (0 to 6) of the seven shared lines: a text in small letters, at most 31 characters, shown in capitals. `screen` is not read: the lines are the same for every such screen, so write them right before the draw of the screen they are meant for. | Ours: the display thread and the key thread. |
| `0x8fc64c08` | `void settings_line_value(void* screen, u32 line, const char* text)` | Sets the value on line `line` (0 to 6) of the seven shared lines, as `settings_line_name` sets the name. | Ours: the display thread and the key thread. |
| `0x8fc261fc` | `u32* screens(void)` | The table of the firmware's screens by their numbers: a word a number, the screen object or 0 for a number no screen has. A screen's document is the word at its +4, 0 until the firmware has loaded it. Our code reads only the numbers 1 to 85. | Ours: the display thread. |
| `0x8fc273f0` | `u32 screen_of(u32 type)` | The number of the screen of an effect's type, 0 to 10. For any other type it writes a line into the log and gives back 86. | Ours: the display thread. |
| `0x8fc274ac` | `u32 engine_screen(u32 type)` | The number of the screen of a synth engine's type. For a type that has no screen it writes a line into the log and gives back 86. | Ours: the display thread. |
| `0x8c0dd2cc` | `void draw_char(u16* framebuffer, int character)` | Draws one character of the firmware's small bitmap font into a frame buffer, at the cursor: the two words at `0x840148b0` (x) and `0x840148b4` (y). Every pixel of the character's cell is written, the character's in white and the rest in black. The cursor's x moves on by the character's width and one. A byte 0x80 to 0xbf draws nothing; any other character outside 0x20 to 0x7e is drawn as `?`. A character that does not fit before x = 480 is not drawn, and the cursor stays. Our code sets the cursor before a text and puts it back after. | Ours: the display thread. |
| `0x8fca6940` | `u32 tick(u32, u32, u32)` | The firmware's work of a frame before the draw, called by the display thread's frame loop. It takes one argument, the object the loop hands over, and gives nothing back. Our code calls it in place of the loop's own call, with the registers as they came. | Ours: the display thread. |
| `0x8fc27678` | `u32 draw(u32, u32, u32)` | The whole draw of a frame: every screen that is shown, into the frame buffer the loop hands over, its one argument. It gives nothing back. Our code calls it in place of the loop's own call, with the registers as they came. | Ours: the display thread. |

## Keys and the screen that is shown

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c0b2554` | `void request_screen(void)` | Asks the firmware to show the screen the present mode wants now. The firmware does this itself after every key; our code calls it after a change that no key press follows, such as a timer running out. It takes no argument. From the display thread, call it with interrupts masked for the length of the call: the firmware stops when two threads are inside it at once. | Ours: the key thread, and the display thread with interrupts masked. |
| `0x8fc24ca4` | `int shift(void)` | Not 0 while the shift key is held. | Ours: the key thread, the display thread. |
| `0x8c0b68b0` | `void knob_item_top(void* knob, u32 top)` | Sets the highest value of one of the unit's knob items, the 32-byte objects the settings' items and line pickers are made of, to `top` whole steps, and holds the item's value to it. KEYBOARD, SYSTEM and MIDI keep their line picker in the row at `row + 4`, with the number of lines less one as its top. The firmware stops when the top is below the item's lowest value. | Ours: the key thread. |
| `0x8c0d44b0` | `void keyboard_chosen(void* row)` | The settings screen lists its rows on the left; a row is an object whose first word is the address of its table of functions, which the firmware calls on the key thread. This is KEYBOARD's function for its row being chosen (`row`: the row). It writes the row's own three lines into the shared seven, sets the number of lines in the row (the word at `row + 0x2c`) and in its screen, and sets the top of the row's line picker. The firmware calls it through the row's table; our code calls it first from its own function in that word of the table. | Ours: the key thread. |
| `0x8c0d4d0c` | `void system_chosen(void* row)` | The same for SYSTEM, with its six lines. | Ours: the key thread. |
| `0x8c0d484c` | `void midi_chosen(void* row)` | The same for MIDI, with its five lines. | Ours: the key thread. |
| `0x8c0d45ce` | `void keyboard_red(void* row, u32 line, u32 amount)` | The settings row KEYBOARD's function for a turn of the red knob on line `line` of its row (`row`). `amount` is the turn as the firmware hands it, a float: pass it on as it came. Hand it only the row's own lines, 0 to 2. | Ours: the key thread. |
| `0x8c0d5018` | `void system_red(void* row, u32 line, u32 amount)` | The same for SYSTEM: only its own lines, 0 to 5. | Ours: the key thread. |
| `0x8c0d49d0` | `void midi_red(void* row, u32 line, u32 amount)` | The same for MIDI: only its own lines, 0 to 4. | Ours: the key thread. |

## Sounds: their engines, effects and LFOs

The unit has eight synth sounds and eight drum sounds, in slots 0 to 7 each. Each slot has a record, an object of the firmware's at a fixed address, which holds the sound's document: what the unit saves with the sound. A sound's LFO has a kind (a row of the list under shift and T4), eight words of settings and a switch. Our code changes them through the calls below, the way the unit's own keys do, so that what the unit shows, plays and saves agrees.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8d4f3c84` | `void lfo_words_doc(void* record, const u32* words)` | Writes eight words of LFO settings into the document of the sound whose record is `record`. Follow it with the record's LFO apply (`lfo_apply_synth` or `lfo_apply_drum`), so that the sound plays them. | Ours: the key thread. |
| `0x8d4f3d0c` | `void lfo_active_doc(void* record, int on)` | Writes the LFO's switch, `on` 1 or 0, into the document of the sound whose record is `record`. Our code calls it right after `lfo_on_synth` or `lfo_on_drum`, so that the switch the sound plays with is the one it is saved with. | Ours: the display thread. |
| `0x8d4f64c0` | `void lfo_apply_synth(void* record)` | Makes the synth sound whose record is `record` play the LFO its document holds now, as after a change on the unit's own LFO page. | Ours: the key thread. |
| `0x8d4f4f90` | `void lfo_apply_drum(void* record)` | The same for a drum sound's record. | Ours: the key thread. |
| `0x8d504308` | `void lfo_on_synth(u32 slot, int on)` | Switches the LFO of synth slot `slot` (0 to 7) on (`on` 1) or off (0), as the unit's own key does. It changes the switch the sound plays with; `lfo_active_doc` writes it into the document. | Ours: the key thread, the display thread. |
| `0x8d5042d8` | `void lfo_on_drum(u32 slot, int on)` | The same for drum slot `slot` (0 to 7). | As `lfo_on_synth`. |
| `0x8d504290` | `u32 lfo_on_synth_read(u32 slot)` | Whether the LFO of synth slot `slot` (0 to 7) is on: not 0 when it is. | Ours: the key thread, the display thread. |
| `0x8d504260` | `u32 lfo_on_drum_read(u32 slot)` | The same for drum slot `slot` (0 to 7). | Ours: the key thread, the display thread. |
| `0x8fc275e4` | `u32 lfo_screen_of(u32 kind)` | The number of the screen the unit shows for an LFO of kind `kind`, in the firmware's own numbering of its six kinds, 0 to 5. Our code asks it once for each kind, at its first frame, and knows the LFO pages by those numbers from then on. | Ours: the display thread. |
| `0x8d4f221c` | `u32 lookup(u32 database, const char* key, const void* name, void* out)` | Finds a row of the unit's catalog of sounds and effects. `database`: which part of the catalog, as the firmware hands it; `key`: the group's key; `name`: the row's name, as the firmware hands it; the row is put into the JSON value `out`. Gives back 0 when there is no such row. Our code calls it from its own function in its place where the master effect's list opens, and then, while the master rides an effect of a pack, puts that effect's id into the row. | Ours: the key thread. |
| `0x8d4f6c3c` | `void synth_put(void* record, int slot, void* json, int flag)` | Puts a sound on a synth slot: `record` is the slot's record, `slot` its number, 0 to 7, and `json` the sound's document, which is assigned to the record's. With `flag` 0 the firmware then keeps of the record's document only the members it knows. Last it sets the name the screen shows for the slot; a `slot` outside 0 to 7 sets no name. Nothing comes back. | Ours: the firmware's background worker, in place of its own call when a sound is loaded from the unit's list. |
| `0x8d4f6748` | `void drum_put(void* record, int slot, void* json, int flag)` | The same for a drum slot. With `flag` 0, a document the firmware's check refuses is written as an error into the log. | As `synth_put`. |
| `0x8d4f3b9c` | `void set_params(u32 record, const u32* words)` | Writes an effect's eight knob words into a slot's document, as the member `fx_params`: each word is a value shifted left by sixteen, and the value is what goes in. Then the record is marked to be written. `record` is the slot's record. | Ours: the display thread. |
| `0x8d4f3c24` | `void set_active(u32 record, u32 on)` | Writes whether a slot's effect is on into the slot's document, as the member `fx_active`, and marks the record to be written. `record` is the slot's record. | Ours: the key thread, the display thread. |
| `0x8d4f689c` | `void knob_set(u32 record, const u32* words)` | Writes a synth slot's eight knob words into its document, as the member `knobs`, each a value shifted left by sixteen, and marks the record to be written. The words of the sampler's sounds are taken another way: some of them are written as other members. | Ours: the display thread. |
| `0x8d504320` | `void on_synth(u32 slot, u32 on)` | Sets the byte that says whether the effect of synth slot `slot` plays. Only that byte: nothing is written into the slot's document. | Ours: the display thread, and in place of the firmware's own call when a slot's sound is applied. |
| `0x8d5042f0` | `void on_drum(u32 slot, u32 on)` | The same for a drum slot. | As `on_synth`. |
| `0x8d5042c0` | `void on_master(u32 on)` | The same for the master's effect. | Ours: the display thread. |
| `0x8d50424c` | `u32 master_on(void)` | Whether the master's effect plays: the byte `on_master` sets. | Ours: in place of the firmware's own call when the master is saved. |
| `0x8c0bb684` | `void master_pick(void* controller, u32 id)` | The mixer's pick of the master's effect: the function in the word +0x28 of the mixer's controller's table (the word at `0x8400f2cc`). `controller` is the mixer's controller, `id` the id of the effect's row in the catalog's list `fx_types`. Our code calls it with the controller as it came. | Ours: the key thread, in place of the firmware's own call. |
| `0x8f011068` | `void* factory(u32 type, u32 index)` | Makes an effect object of a type, 0 to 10, for an index, 0 or 1, and gives it back; 0, with a line in the log, for any other type or index. Each index has one room: an object made for an index is made in the place of the one made before for it. The object's first word is its table of functions. | Ours: in place of the firmware's own call, on whatever thread that call runs. |
| `0x8d505390` | `void build(u32 index, u32 type)` | Makes the effect object of `index` anew for a type and puts it in the index's word. A type `factory` refuses stops the unit. | Ours: the display thread. |
| `0x8d4fee88` | `int user_sounds(void)` | The number of user sounds the unit holds, synth and drum together. It stops the unit when the synth's or the drum's total is 0, as it is before the firmware's first count: our code reads the two totals first and calls it only when both are not 0. | Ours: the display thread, with interrupts masked. |
| `0x8fca2220` | `u32 classic(void)` | Not 0 when the unit holds the file `content/op1.classic`. The first call looks on the disk; every call after gives back what it found. | Not the audio thread. Ours: the display thread. |

## Notes and sequencers

These play on the instrument the unit is on: the synth or the drum, by the firmware's play mode. A note is a MIDI note number, 0 to 127.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8f007d68` | `NoteRecord note_record(int key, int velocity)` | The note a key plays: `key`, the key's number, and `velocity` become a record of four words, the MIDI note first and the velocity second. The note is 53 and the key, and on the synth twelve times its keyboard octave more. The record comes back as a structure, by value: bind the function as one that returns the structure, and the compiler hands over the address for it in P0. | Ours: the key thread, the audio thread or a MIDI handler's thread, when a key reaches a pack's sequencer. |
| `0x8f008388` | `void note_on(int note, int velocity, int third, int fourth, int octave, int target, int no_midi)` | Begins `note` at `velocity` on the instrument the unit is on, as a key does, and sends it out over MIDI. A `velocity` of -1 is taken as 127. A note above 127 stops the unit. `octave` counts on the drum only. `target` is where the MIDI goes, 0 for the unit's own settings; with `no_midi` not 0 nothing is sent. Our code hands 0 for `third`, `fourth`, `octave` and `target`. | Ours: from a pack's sequencer calls: the sequencer thread, the key thread, the audio thread, a MIDI handler's thread. |
| `0x8f008518` | `void note_off(int note, int velocity, int third, int fourth, int target, int no_midi)` | Ends `note` on the instrument the unit is on and sends the end out over MIDI, unless `no_midi` is not 0; `target` as for `note_on`. A note outside 0 to 127 is passed over. While the firmware holds its notes, as with a sustain pedal, the end waits until they are let go. Our code hands 64 for `velocity` and 0 for `third`, `fourth` and `target`. | As `note_on`. |
| `0x8f008ba8` | `void bend(int note, int pitch)` | Sets the pitch of the synth's voices that play `note`. `pitch` is a pitch word as the firmware's table of pitches holds them. It works on the synth's voices: our code calls it only while the unit is on the synth. | As `note_on`. |
| `0x8fc97844` | `u32* player_word(void)` | The address of the word that holds the sequencer player of the instrument the unit is on: the drum's while the unit is on the drum, else the synth's. A player is an object whose first word is its table of functions. | Ours: the key thread, or the main thread at the start. |
| `0x8fc978fc` | `void* current_player(void)` | The player in that word: the object, or 0. | As `player_word`. |
| `0x8fc9e818` | `int restart_allowed(void)` | Not 0 when the unit's clock settings let the sequencers' count be set back now, as the firmware's own sequencers ask before they set it back at a first key. | As `note_on`. |
| `0x8fc9e868` | `void restart(void)` | Sets the sequencers' count back to its start. It takes nothing and gives nothing back. Ask `restart_allowed` first. | As `note_on`. |
| `0x8fc681a0` | `int overlay_match(void* overlay, u32 screen)` | The matcher of the notice "sequencer off": the function in the word at `0x848d8668`, its table's. `overlay` is the notice's object and `screen` a screen's number. Our code answers in its place for the packs' page and hands it every other call with the arguments as they came. | Ours: in place of the firmware's own call. |

## The tape's document and its save

The firmware keeps the tape's settings as one JSON document, read from the tape's file at a load and written into it at a save. The save runs on the firmware's background worker.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8d4f2b80` | `void* tape_root(void)` | The tape's document: a JSON value at a fixed place, the same every call. The unit stops when the document is null. | Not the audio thread. Ours: the firmware's background worker, the key thread, the main task. |
| `0x8d4f24dc` | `int tape_set(int db, const char* key, const void* value)` | Sets a member of one of the firmware's documents to a copy of `value`, under the documents' lock, and marks that document changed. `db` picks the document: 2 is the tape's, the one `tape_root` gives. `key` is the member's name. It waits for the lock as long as it takes, so it waits while a tape's file is read or written. Gives back 1. | Not the audio thread. Ours: the firmware's background worker, in the tape's save. |
| `0x8c0c1dd0` | `void* tape_saver(void)` | The tape's saver: an object of the firmware's, made at the first call and the same at every call after. Hand it to `notify` to have the tape saved. | Ours: the display thread. |
| `0x8fca1ed2` | `void notify(void* saver)` | Marks a saver to save and wakes the background worker, which then saves. It does not wait. | Ours: the display thread. |

## The disk mode

When the unit is a disk on a computer, the firmware builds a tree of folders and files to show. A folder has a behaviour: an object whose first word is a table of functions. The word at +0x10 makes the behaviour of a folder inside it by its name, and the word at +0x14 gives a file object for a file of it by its name; each answers 0 for a name it does not take.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8fcacea0` | `void* root_child(void* behaviour, const char* name)` | The root folder's behaviour's maker of a folder inside it: the function in the word at `0x8490fc4c`, the root behaviour's +0x10. `behaviour` is the root behaviour, `name` the folder's name; it gives back the behaviour of that folder, or 0. Our code calls it for every name that is not its own, with the arguments as they came. | Ours: the disk mode's thread, in place of the firmware's own call. |
| `0x8fcaeb10` | `void* make_folder(void* parent, const char* name)` | Makes a folder of the disk mode's tree inside `parent` and gives it back. The folder's behaviour comes from the parent's behaviour, its word at +0x10. It gives back 0 with a line in the log for a name the parent already has, or when the behaviour gives none. | Ours: the disk mode's thread. |
| `0x8fcaed68` | `void* add_file(void* folder, const char* name)` | Adds a file to a folder of the disk mode's tree and gives back its node, or 0 when the folder's behaviour gives no file object for the name (its word at +0x14). An empty name stops the unit. | Ours: the disk mode's thread. |
| `0x8fcbabb8` | `void fs_file_ctor(void* obj, const char* path)` | Makes the firmware's file object for a file on the disk in memory the caller provides, 64 bytes from `new`: the object the disk mode reads and writes a file through. The path is copied into a text object of the firmware at the object's +4, and the file is not open yet. The object's first word is its table of twelve functions. | Ours: the disk mode's thread. |

## The keyboard controller, MIDI out and the restart

Calls that reach beyond the firmware's own memory: the keyboard controller, the USB MIDI output, and the restart of the unit.

| Address | The call | What the caller hands over and gets back | Called from |
| --- | --- | --- | --- |
| `0x8c0567f0` | `u32 kb_request(void* link, u32 type, const void* payload, u32 bytes, void* answer, u32 timeout_ms)` | Sends a request to the keyboard controller and waits for its answer. `link` is the controller's link, at `0x8497d844`; `type` the request; `payload` and `bytes` what goes with it, 0 and 0 for none. The answer is copied whole into `answer`, the type at its byte 2, the length at byte 3 and the payload from byte 4; it may be copied there even after the wait is over, so keep the buffer out of the stack. `timeout_ms` bounds the wait. Not 0 when the answer came in time; 0 when it did not, or when four requests are already waiting. | Ours: the main thread at the start. |
| `0x8d4e5cb8` | `void midi_out(int index, const void* packets, u32 bytes)` | Hands USB MIDI packets, four bytes each, to the MIDI output: `index` is the port, `packets` the packets, `bytes` their length in bytes. In some states of the USB connection nothing is sent and nothing is said, and it gives nothing back: our code looks at the output's room and at the USB mode first, and sends only in the mode 1. | Ours: the main task when it handles a SysEx message. |
| `0x8c0df868` | `void restart(int code, int delay_ms)` | Asks for a restart and comes back at once: `delay_ms` later the unit shuts down and starts again; `delay_ms` is taken in sixteen bits. It writes a line into the log. `code` picks what starts next: our code hands 20, and the unit comes back into the OS. | Ours: the main task when it handles a SysEx message, the display thread. |

## Bound in the sources, not described yet

Each of these is called by the files named. The type is what our code hands over and takes back; the name is the sources' own. Read the file's comments at the binding before you use one, and add its words to `profiles/op1f-1.7.3-interfaces.json` when you have.

| Address | The call | Bound in |
| --- | --- | --- |

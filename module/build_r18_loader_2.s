/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R18, base 40: the word through which the base reads the new loader N2's record. op1f_loader_2 is an optional
 * name: every object that names it declares it weak, so the pack
 * tools take it as an optional import. Under N1 no loader offers it and the word reads 0; under N2 it holds the
 * record's address. The record's u32 words: [0] its bytes (24 for N2), [1] the loader's generation (2), [2] the room's
 * first address, [3] the room's bytes, [4] the most pack files the loader looks at, [5] the most cards.
 *
 * C++ reads the word as `extern "C" const u32* const r18_loader_2_record` (module/build_r4c_door.cpp, the door, and
 * module/build_r6d_cards.cpp, the cards); no line of C++ names op1f_loader_2 itself, since a strong name there would
 * be one every loader must offer. The word's own name is the base's, not an op1f_ export.
 */
.section .rodata
.balign 4

.weak op1f_loader_2                         /* N2's record: 0 while no loader offers it */
.global r18_loader_2_record
.type r18_loader_2_record, STT_OBJECT
r18_loader_2_record:
    .long op1f_loader_2
.size r18_loader_2_record, 4

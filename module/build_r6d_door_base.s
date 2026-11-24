/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Base 37 (R13): the base the door's build 6 is in, base.pack 37, made from the base's recipe (the door's
 * test stop switched off: module/build_r4c_door.cpp). The door says these two in its answer to hello and in its ready
 * line: the base's version and the name its recipe had where base 37 was built (packs/base.json in this tree).
 */
.section .rodata
.balign 4
.global op1f_door_base_version
.type op1f_door_base_version, STT_OBJECT
op1f_door_base_version:
    .long 37
.size op1f_door_base_version, .-op1f_door_base_version
.global op1f_door_base_from
.type op1f_door_base_from, STT_OBJECT
op1f_door_base_from:
    .asciz "packs/r13/base.json"
.size op1f_door_base_from, .-op1f_door_base_from

/* Step R21, base 43: the entry at one function of the firmware for a pack's drum engine (OS 1.7.3+0). The logic is in
 * the drum unit (module/build_r18_drum.cpp). The base's table puts a JUMP.a to the entry over the first eight bytes of
 * the function, with a trampoline the loader makes in the room named beside it: those eight bytes copied from the
 * running firmware (whole instructions, none relative to its own place), and a jump back behind them. A jump to the
 * room is the firmware's function as it always was; a call of the room is the same function returning to its caller.
 *
 *   op1f_drum_pick_entry   D1, the drum's engine pick, 0x8d4eb160; R0 the catalog id picked, R1 the drum slot. With no
 *                          drum engine registered (op1f_drum_awake 0), the room at once; else op1f_drum_pick_do(id,
 *                          slot), which calls the room itself when the firmware's pick is to be made, and back to the
 *                          pick's caller.
 *
 * The registers, as at the synth's pick (module/build_r6_engines.s, E2), whose first eight bytes are the same: the entry
 * is a function's first instruction, reached by a call of compiled code (the drum's engine list and MIDI): R0 to R2 are
 * the function's arguments, and R3, P0 to P2 and ASTAT hold nothing the caller keeps. So the entry decides with R3, P0
 * and CC, and calls nothing before it has decided; on the way at rest it leaves every other register, RETS, SP and the
 * caller's stack as they came. On the way through the base's function it pushes what it gives back and gives the
 * compiled code its twelve bytes of argument space. The stack it takes at its deepest: 0 bytes at rest, 28 on the way
 * through (RETS, R0, R1, R2 and the twelve bytes).
 */
.section .text
.align 2

.global op1f_drum_pick_entry
.type op1f_drum_pick_entry, STT_FUNC
op1f_drum_pick_entry:
    P0.H = op1f_drum_awake;
    P0.L = op1f_drum_awake;
    R3 = [P0];
    CC = R3 == 0;
    IF !CC JUMP .Lpick_awake;
    JUMP.L op1f_drum_pick_room;             /* at rest: the firmware's own way at once */
.Lpick_awake:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    [--SP] = R2;
    SP += -12;
    CALL op1f_drum_pick_do;                 /* (id, slot), as they came: it calls the room itself */
    SP += 12;
    R2 = [SP++];
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    RTS;
.size op1f_drum_pick_entry, .-op1f_drum_pick_entry

/* The trampoline's room, in the pack's zeroed data: the eight bytes it copies, and eight for the jump back. */
.section .bss
.balign 4
.global op1f_drum_pick_room
.type op1f_drum_pick_room, STT_OBJECT
op1f_drum_pick_room:
    .space 16
.size op1f_drum_pick_room, .-op1f_drum_pick_room

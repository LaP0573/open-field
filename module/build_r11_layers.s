/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R11, base 35: the entries at the four places of the firmware for layers (in the public profile, places of the
 * group base; step R13). The logic is in the effects unit (module/build_r6d_effects.cpp) and the LFO
 * unit (module/build_r11_lfos.cpp). This file names no address of the firmware: it is the same for OS 1.7.0 and 1.7.3.
 *
 * One place is an eight-byte CALL.a of the firmware, retargeted to an entry here (the profile's `call: true`, as W1 and
 * W8 are). The entry hands on to the base's C function with the arguments as they came; the C function makes the
 * firmware's call itself and answers in R0, and the entry gives that answer back to the call's site:
 *   op1f_layer_c3_stub  C3, the call of the lookup for the master's list (the function at 0x8d4f221c): its first
 *                       argument 1, R1 the
 *                       key, R2 the name, the fourth argument (the out word) on the caller's stack ->
 *                       op1f_fx_master_start(1, key, name, out)
 *
 * Three are W2's form: the base's table puts a JUMP.a to the entry over the first eight bytes of a function of the
 * firmware, with a trampoline the loader makes in the room named beside it: those eight bytes copied from the running
 * firmware, and a jump back behind them. A call of the room is the firmware's function as it always was.
 *   op1f_layer_eh_stub  EH, the head of the effect slot's start row (1.7.0 0x8d4f7a34, 1.7.3 0x8d4f7a34), which the
 *                       synth's list and the drum's call alike: R0 the record -> op1f_fx_start_do(record), which answers
 *                       itself or calls the room op1f_layer_eh_room with (record); its answer, the list's start id, is
 *                       given back to the caller in R0
 *   op1f_layer_lh_stub  LH, the head of the LFO slot's start row (0x8d4f7ad0, 0x8d4f7ad0), the same for the lists of LFO
 *                       kinds: R0 the record -> op1f_lfo_start_do(record), the room op1f_layer_lh_room; its answer in R0
 *   op1f_layer_lg_stub  LG, the head of the LFO pick (0x8d4f3a54, 0x8d4f3a54): R0 the record, R1 the id ->
 *                       op1f_lfo_pick_do(record, id), which answers itself or calls the room op1f_layer_lg_room with
 *                       (record, id); then back to the pick's caller with R0 to R2 as they came, as W2's entry does
 *                       (the firmware's pick answers nothing)
 *
 * The registers. Each entry is reached by a call of compiled code (a head's by the JUMP.a its function's first bytes
 * became, with the call's RETS): R0 to R2 and the caller's stack words are the function's arguments, and R3, P0 to P2
 * and ASTAT hold nothing the caller keeps. Each entry pushes RETS (LG also R0 to R2), gives the compiled code its twelve
 * bytes of argument space (SP += -12 after the pushes) and takes them back before its pops (SP += 12), so that nothing
 * of the caller's is written. C3 copies the fourth argument below that space, where the C function reads it ([SP + 12]
 * at its entry), as the firmware's lookup read it from its caller's. The stack an entry takes at its deepest: 16 bytes
 * (EH, LH), 20 (C3), 28 (LG), below which the compiled code takes its own.
 */
.section .text
.align 2

.global op1f_layer_c3_stub
.type op1f_layer_c3_stub, STT_FUNC
op1f_layer_c3_stub:
    [--SP] = RETS;
    R3 = [SP + 16];                         /* the fourth argument: [SP + 12] at the entry, four bytes up by the push */
    [--SP] = R3;                            /* above the twelve bytes, where the C function reads it */
    SP += -12;
    CALL op1f_fx_master_start;              /* (1, key, name, out), as they came: answers the lookup's answer in R0 */
    SP += 12;
    SP += 4;                                /* the copy of the fourth argument */
    RETS = [SP++];
    RTS;
.size op1f_layer_c3_stub, .-op1f_layer_c3_stub

.global op1f_layer_eh_stub
.type op1f_layer_eh_stub, STT_FUNC
op1f_layer_eh_stub:
    [--SP] = RETS;
    SP += -12;
    CALL op1f_fx_start_do;                  /* (record), as it came: answers the start in R0 */
    SP += 12;
    RETS = [SP++];
    RTS;
.size op1f_layer_eh_stub, .-op1f_layer_eh_stub

.global op1f_layer_lh_stub
.type op1f_layer_lh_stub, STT_FUNC
op1f_layer_lh_stub:
    [--SP] = RETS;
    SP += -12;
    CALL op1f_lfo_start_do;                 /* (record), as it came: answers the start in R0 */
    SP += 12;
    RETS = [SP++];
    RTS;
.size op1f_layer_lh_stub, .-op1f_layer_lh_stub

.global op1f_layer_lg_stub
.type op1f_layer_lg_stub, STT_FUNC
op1f_layer_lg_stub:
    [--SP] = RETS;
    [--SP] = R0;
    [--SP] = R1;
    [--SP] = R2;
    SP += -12;
    CALL op1f_lfo_pick_do;                  /* (record, id), as they came: it answers itself or calls the room */
    SP += 12;
    R2 = [SP++];
    R1 = [SP++];
    R0 = [SP++];
    RETS = [SP++];
    RTS;
.size op1f_layer_lg_stub, .-op1f_layer_lg_stub

/* The trampolines' rooms, in the pack's zeroed data: each the eight bytes it copies, and eight for the jump back. */
.section .bss
.balign 4
.global op1f_layer_eh_room
.type op1f_layer_eh_room, STT_OBJECT
op1f_layer_eh_room:
    .space 16
.size op1f_layer_eh_room, .-op1f_layer_eh_room

.global op1f_layer_lh_room
.type op1f_layer_lh_room, STT_OBJECT
op1f_layer_lh_room:
    .space 16
.size op1f_layer_lh_room, .-op1f_layer_lh_room

.global op1f_layer_lg_room
.type op1f_layer_lg_room, STT_OBJECT
op1f_layer_lg_room:
    .space 16
.size op1f_layer_lg_room, .-op1f_layer_lg_room

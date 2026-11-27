/* Made for OS 1.7.3 from this source's OS 1.7.0 form; every address the code uses is 1.7.3's; an address in a comment that the code does not use may still be 1.7.0's. */
/* Step R17, base 39: SD, the head of the worker's synth drop job (0x8f00b7b0; in the public profile, a place of the
 * group base). The logic is in the sample unit (module/build_r17_sample.cpp). Its code names no address of the
 * firmware: it is the same for OS 1.7.0 and 1.7.3.
 *
 * W2's form, as EH's (module/build_r11_layers.s): the base's table puts a JUMP.a to op1f_sample_sd_stub over the
 * job's first eight bytes (whole instructions, none of them relative to where it stands), with a trampoline the
 * loader makes in op1f_sample_sd_room: those eight bytes copied from the running firmware, and a jump back behind them.
 * A jump or a call to the room is the firmware's job as it always was.
 *
 * The job is called by the worker's dispatch with R0 the slot and R1 the clipboard; RETS goes back into the dispatch.
 *   - no ticket for this slot (op1f_sample_sd_armed is not the slot + 1): the unit's own drop. It is counted
 *     (op1f_sample_sd_passes, the worker's word alone) and the stub jumps on to the room with every register as it
 *     came but R3, P2 and CC, which hold nothing at a function's entry, and RETS
 *     untouched: the firmware's RTS goes back into the dispatch
 *   - a ticket for this slot: op1f_sample_job(slot, clipboard), compiled code, which takes the ticket in a mask and
 *     answers itself, or calls the room when the ticket is no longer there. The stub pushes RETS, gives the compiled
 *     code its twelve bytes of argument space (SP += -12 after the push) and takes them back before its pop (SP += 12),
 *     so that nothing of the dispatch's is written. The job answers nothing the dispatch reads.
 * The stack the stub takes: 16 bytes on the ticket's way, none on the unit's own.
 */
.section .text
.align 2

.global op1f_sample_sd_stub
.type op1f_sample_sd_stub, STT_FUNC
op1f_sample_sd_stub:
    P2.H = op1f_sample_sd_armed;
    P2.L = op1f_sample_sd_armed;
    R3 = [P2];                              /* the slot + 1 a ticket is queued for, or 0 */
    R3 += -1;                               /* the slot, or -1: never a slot */
    CC = R0 == R3;
    IF CC JUMP .Lsd_ticket;
    P2.H = op1f_sample_sd_passes;
    P2.L = op1f_sample_sd_passes;
    R3 = [P2];
    R3 += 1;
    [P2] = R3;                              /* the unit's own drop, counted */
    P2.H = op1f_sample_sd_room;             /* the eight bytes the hook displaced, then the job's ninth byte on */
    P2.L = op1f_sample_sd_room;
    JUMP (P2);
.Lsd_ticket:
    [--SP] = RETS;
    SP += -12;
    CALL op1f_sample_job;                   /* (slot, clipboard), as they came */
    SP += 12;
    RETS = [SP++];
    RTS;
.size op1f_sample_sd_stub, .-op1f_sample_sd_stub

/* R21 (base 43): DD, the head of the worker's drum drop job (0x8f00ba38, request 10; its first eight bytes are the same
 * three instructions as SD's), in SD's form: no ticket for this drum slot (op1f_sample_dd_armed is not the slot + 1),
 * the unit's own drop, counted (op1f_sample_dd_passes) and on to the room; a ticket, op1f_sample_drum_job(slot, clip).
 */
.global op1f_sample_dd_stub
.type op1f_sample_dd_stub, STT_FUNC
op1f_sample_dd_stub:
    P2.H = op1f_sample_dd_armed;
    P2.L = op1f_sample_dd_armed;
    R3 = [P2];
    R3 += -1;
    CC = R0 == R3;
    IF CC JUMP .Ldd_ticket;
    P2.H = op1f_sample_dd_passes;
    P2.L = op1f_sample_dd_passes;
    R3 = [P2];
    R3 += 1;
    [P2] = R3;
    P2.H = op1f_sample_dd_room;
    P2.L = op1f_sample_dd_room;
    JUMP (P2);
.Ldd_ticket:
    [--SP] = RETS;
    SP += -12;
    CALL op1f_sample_drum_job;
    SP += 12;
    RETS = [SP++];
    RTS;
.size op1f_sample_dd_stub, .-op1f_sample_dd_stub

/* The trampoline's room, in the pack's zeroed data: the eight bytes it copies, and eight for the jump back. */
.section .bss
.balign 4
.global op1f_sample_sd_room
.type op1f_sample_sd_room, STT_OBJECT
op1f_sample_sd_room:
    .space 16
.size op1f_sample_sd_room, .-op1f_sample_sd_room
.balign 4
.global op1f_sample_dd_room
.type op1f_sample_dd_room, STT_OBJECT
op1f_sample_dd_room:
    .space 16
.size op1f_sample_dd_room, .-op1f_sample_dd_room

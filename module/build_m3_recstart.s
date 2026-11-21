/* The base pack: the place the input start (module/build_m3_recstart.cpp) runs from (OS 1.7.3+0).
 *
 *   op1f_recstart_entry  a call in place of the call at 0x8c0b2e9a, in the key thread's main loop, which calls
 *                        0x8c0b2794 for every key: the call stands on every pass. Ours makes the firmware's call
 *                        (0x8f005ef8, R0 as the firmware set it) and then op1f_recstart_poll. The code behind the
 *                        place loads R0 again, so the callee's answer is not handed back. R0 to R3, P0 to P2 and RETS
 *                        are free across the call, as across the firmware's own; each call gets its twelve bytes of
 *                        argument space.
 */
.text
.align 2
.global op1f_recstart_entry
.type op1f_recstart_entry, STT_FUNC
op1f_recstart_entry:
    [--SP] = RETS;
    SP += -12;
    P1.H = 0x8f00;
    P1.L = 0x5ef8;
    CALL (P1);
    CALL op1f_recstart_poll;
    SP += 12;
    RETS = [SP++];
    RTS;
.size op1f_recstart_entry, .-op1f_recstart_entry

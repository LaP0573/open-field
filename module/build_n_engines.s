/* Build N: where the firmware enters the pack engines (module/build_n_engines.cpp), and the rooms its trampolines run from. */
.text
.align 2

/* set-patch's call of the name -> type lookup (0x8d4f7354): the patch is in R5 there. */
.global _eng_set_type_entry
.type _eng_set_type_entry, STT_FUNC
_eng_set_type_entry:
    R1 = R5;
    JUMP.L _eng_set_type;
.size _eng_set_type_entry, .-_eng_set_type_entry

/* The voice render, in L1 (a jump at 0x11a0476e): P0 points at the voice, R1 is its output, P1 leads to its table.
 * A voice that is to render through its slot's engine: the engine renders, and the firmware goes on behind the render call
 * (P1 = 0x60 there, as the firmware sets it). Any other voice: the firmware's own render, through the room. */
.global _eng_voice_entry
.type _eng_voice_entry, STT_FUNC
_eng_voice_entry:
    [--SP] = P1;
    [--SP] = R1;
    [--SP] = R2;
    [--SP] = P0;
    [--SP] = RETS;
    R0 = [P0];                          /* the voice */
    SP += -12;
    CALL _eng_voice_is;
    SP += 12;
    RETS = [SP++];
    P0 = [SP++];
    R2 = [SP++];
    R1 = [SP++];
    P1 = [SP++];
    CC = R0 == 0;
    IF CC JUMP 1f;
    [--SP] = RETS;
    R0 = R1;                            /* the output buffer */
    R1 = [P0];                          /* the voice: polyphonic engines keep one state each */
    SP += -12;
    CALL _eng_voice_render;
    SP += 12;
    RETS = [SP++];
    P1 = 0x60;
    P0.H = 0x11a0;
    P0.L = 0x477c;                      /* behind the firmware's render call */
    JUMP (P0);
1:  JUMP.L _eng_orig_voice;                 /* the firmware's own: the displaced loads, then its render call */
.size _eng_voice_entry, .-_eng_voice_entry

/* The synth bus, in L1 (a jump at 0x11a052a2): the voices are summed into the bus, the firmware is about to call its effect
 * stage (the T3 effect, and fx train's train behind it). Everything is kept as it was; the room does the firmware's load. */
.global _eng_bus_entry
.type _eng_bus_entry, STT_FUNC
_eng_bus_entry:
    [--SP] = RETS;
    [--SP] = ASTAT;
    [--SP] = (R7:0, P5:0);
    [--SP] = A0.W;
    [--SP] = A0.X;
    [--SP] = A1.W;
    [--SP] = A1.X;
    [--SP] = LC0; [--SP] = LT0; [--SP] = LB0;
    [--SP] = LC1; [--SP] = LT1; [--SP] = LB1;
    SP += -12;
    CALL _eng_bus;
    SP += 12;
    LB1 = [SP++]; LT1 = [SP++]; LC1 = [SP++];
    LB0 = [SP++]; LT0 = [SP++]; LC0 = [SP++];
    A1.X = [SP++];
    A1.W = [SP++];
    A0.X = [SP++];
    A0.W = [SP++];
    (R7:0, P5:0) = [SP++];
    ASTAT = [SP++];
    RETS = [SP++];
    JUMP.L _eng_orig_bus;                   /* the firmware's load of the effect stage's object, then on */
.size _eng_bus_entry, .-_eng_bus_entry

/* The display commit (a call at 0x8c0b193e, only while a pack marked against these places is away): the faces, then the displaced load. */
.global _eng_draw_entry
.type _eng_draw_entry, STT_FUNC
_eng_draw_entry:
    [--SP] = RETS;
    SP += -12;
    P0.H = 0x0801;
    P0.L = 0xb804;
    R0 = [P0];                          /* the frame buffer */
    CALL op1f_engines_face;
    SP += 12;
    RETS = [SP++];
    P0.H = 0x0801;
    P0.L = 0xb804;
    R0 = [P0];
    RTS;
.size _eng_draw_entry, .-_eng_draw_entry

.section .bss
.balign 4
.global _eng_orig_name_to_type
_eng_orig_name_to_type: .space 16
.global _eng_orig_is_valid
_eng_orig_is_valid: .space 16
.global _eng_orig_voice
_eng_orig_voice: .space 16
.global _eng_orig_on
_eng_orig_on: .space 16
.global _eng_orig_off
_eng_orig_off: .space 16
.global _eng_orig_bus
_eng_orig_bus: .space 16

/* The layout of the loader v2 for stock OP-1 Field OS 1.7.3+0: its own block, the last 128 KiB of the firmware file's
 * record of zeros, [0x8d4c4234, 0x8d4e4234) (module_base and reserved_bytes in profiles/op1f-1.7.3-n2.json). The
 * builder's change of the heap's size there ends the heap at the block's first byte. The loader v2 puts the packs in a
 * room of their own, [0x85000000, 0x86000000) (pack_room); a pack is still linked at 0x8d2f4234
 * (linker/op1f-1.7.3-pack.ld, the profile's pack_link). Otherwise as linker/op1f-1.7.3-base.ld. */
OUTPUT_ARCH(bfin)
ENTRY(op1f_boot_stub)
MEMORY { MODULE (rwx) : ORIGIN = 0x8d4c4234, LENGTH = 0x20000 }
SECTIONS {
  __module_reserved_start = ORIGIN(MODULE);
  __module_reserved_end = ORIGIN(MODULE) + LENGTH(MODULE);
  .text : { *(.text .text.*) } > MODULE
  .rodata : ALIGN(4) { *(.rodata .rodata.* .data.rel.ro .data.rel.ro.*) } > MODULE
  .data : ALIGN(4) { *(.data .data.* .sdata .sdata.*) } > MODULE
  __module_initialized_end = .;
  .bss (NOLOAD) : ALIGN(4) {
    __module_bss_start = .;
    *(.bss .bss.* .sbss .sbss.* COMMON)
    __module_bss_end = .;
  } > MODULE
  __module_end = .;
  .init_array : { *(.init_array .init_array.* .ctors .ctors.*) }
  .fini_array : { *(.fini_array .fini_array.* .dtors .dtors.*) }
  /DISCARD/ : { *(.comment .note* .eh_frame .eh_frame_hdr .gcc_except_table) }
  ASSERT(SIZEOF(.init_array) == 0, "Static constructors are not run by the stock loader")
  ASSERT(SIZEOF(.fini_array) == 0, "Static destructors are not supported")
  ASSERT(__module_end <= __module_reserved_end, "The loader exceeds its 128 KiB")
}

OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
  /* Kern 0 bekommt die erste Hälfte: 64K ab 0x80000000 */
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 64K
}

SECTIONS
{
  .text   : { *(.text.init) *(.text) *(.text.*) } > ram
  .rodata : { *(.rodata) *(.rodata.*) } > ram
  .data   : { *(.data) *(.data.*) *(.sdata) } > ram
    /* ... davor .text, .rodata und .data ... */
  /* 4. Uninitialisierte globale Variablen */
  .bss    : {
    . = ALIGN(4);
    PROVIDE(_bss_start = .);     /* NEU: PROVIDE hinzugefügt */
    *(.bss) 
    *(.bss.*) 
    *(.sbss) 
    *(COMMON)
    . = ALIGN(4);
    PROVIDE(_bss_end = .);       /* NEU: PROVIDE hinzugefügt */
  } > ram

  /* Stack-Pointer ans Ende setzen */
  PROVIDE(_sp = ORIGIN(ram) + LENGTH(ram)); /* NEU: PROVIDE hinzugefügt */
}
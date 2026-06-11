OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY
{
  /* Kern 1 bekommt die zweite Hälfte: 64K ab 0x80010000 */
  ram (rwx) : ORIGIN = 0x80010000, LENGTH = 64K
}

/* Der SECTIONS-Block bleibt EXAKT gleich wie bei Core 0! */
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
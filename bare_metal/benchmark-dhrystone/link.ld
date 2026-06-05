OUTPUT_ARCH("riscv")
ENTRY(_start)

/* Hier definieren wir den Speicher exakt so, wie in deiner VCML system.h */
MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 128K
}

SECTIONS
{
  /* 1. Ausführbarer Code */
  .text : {
    *(.text.init) /* Unser Startup-Code muss ganz nach vorne! */
    *(.text)
    *(.text.*)
  } > ram

  /* 2. Konstanten und Strings (z.B. "Execution starts...") */
  .rodata : {
    *(.rodata)
    *(.rodata.*)
  } > ram

  /* 3. Initialisierte globale Variablen */
  .data : {
    *(.data)
    *(.data.*)
    *(.sdata)
  } > ram

  /* 4. Uninitialisierte globale Variablen (werden vom Startup mit 0 gefüllt) */
  .bss : {
    . = ALIGN(4);
    _bss_start = .;
    *(.bss)
    *(.bss.*)
    *(.sbss)
    *(COMMON) /* Hier landen Variablen wegen -fno-common */
    . = ALIGN(4);
    _bss_end = .;
  } > ram

  /* Wir setzen den Stack-Pointer (Stapel) ganz ans Ende des RAMs (wächst nach unten) */
  _sp = ORIGIN(ram) + LENGTH(ram);
}
.section .text.init
.global _start

# NEU: Dem Assembler sagen, dass diese Variablen vom Linker kommen!
.extern _sp
.extern _bss_start
.extern _bss_end

_start:
    # 1. Hardware-Thread-ID lesen
    csrr a0, mhartid

    # 2. Stack-Pointer initialisieren
    la sp, _sp

    # 3. SEINE EIGENE .bss Sektion mit Nullen füllen
    la t0, _bss_start
    la t1, _bss_end
    bgeu t0, t1, end_bss
clear_bss:
    sw zero, 0(t0)
    addi t0, t0, 4
    bltu t0, t1, clear_bss
end_bss:

    # 4. In unsere C-Funktion springen
    call main

    # 5. Fallback
infinite_loop:
    wfi
    j infinite_loop
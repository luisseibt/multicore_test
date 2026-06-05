.section .text.init
.global _start

_start:
    # 1. Hardware-Thread-ID (hartid) lesen
    csrr a0, mhartid

    # NEU: Wenn hartid NICHT 0 ist, schick den Kern sofort in die Endlosschleife!
    # bnez a0, infinite_loop 

    # 2. Stack-Pointer initialisieren (wird nur noch von Kern 0 ausgeführt!)
    la sp, _sp
    li t1, 0x1000           # 4KB stack per core
    mul t1, t1, a0          # Offset = id * 4KB
    sub sp, sp, t1          # Final SP = Top - Offset

    # 3. .bss Sektion mit Nullen füllen
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

    # 5. Fallback und Parkplatz für ungenutzte Kerne
infinite_loop:
    wfi                  # Wait For Interrupt (schickt die CPU schlafen)
    j infinite_loop
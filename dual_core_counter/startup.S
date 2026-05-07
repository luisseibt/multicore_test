.section .text

    .globl _start
    .type _start, @function

    .globl trap_entry
    .type trap_entry, @function

_start:
    # 1. Read Core ID (mhartid)
    csrr t0, mhartid

    # 2. SET UP INTERRUPTS: Set trap vector to trap_entry
    la t1, trap_entry
    csrw mtvec, t1

    # 3. SET UP MULTICORE STACKS
    la sp, _stack_top       # Load Top of RAM (from link.ld)
    li t1, 0x1000           # 4KB stack per core
    mul t1, t1, t0          # Offset = id * 4KB
    sub sp, sp, t1          # Final SP = Top - Offset

    # 4. Jump to C function 'main', pass hartid in a0
    mv a0, t0
    call main

hang:
    wfi
    j hang
    .size _start, . - _start

trap_entry:
    addi sp, sp, -128
    sd ra, 0(sp)
    sd t0, 8(sp)
    sd t1, 16(sp)
    sd t2, 24(sp)
    sd s0, 32(sp)
    sd s1, 40(sp)
    sd s2, 48(sp)
    sd s3, 56(sp)
    sd s4, 64(sp)
    sd s5, 72(sp)
    sd s6, 80(sp)
    sd s7, 88(sp)
    sd s8, 96(sp)
    sd s9, 104(sp)
    sd s10, 112(sp)
    sd s11, 120(sp)

    call trap_dispatch

    ld ra, 0(sp)
    ld t0, 8(sp)
    ld t1, 16(sp)
    ld t2, 24(sp)
    ld s0, 32(sp)
    ld s1, 40(sp)
    ld s2, 48(sp)
    ld s3, 56(sp)
    ld s4, 64(sp)
    ld s5, 72(sp)
    ld s6, 80(sp)
    ld s7, 88(sp)
    ld s8, 96(sp)
    ld s9, 104(sp)
    ld s10, 112(sp)
    ld s11, 120(sp)
    addi sp, sp, 128
    
    mret
    .size trap_entry, . - trap_entry
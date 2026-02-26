/* startup.S */
.section .text
.global _start

_start:
    # 1. Read Core ID (mhartid)
    csrr t0, mhartid

    # 2. Calculate separate stack pointers
    # Stack Top = 0x80100000 (End of 1MB RAM)
    # Core 0 Stack = 0x80100000
    # Core 1 Stack = 0x800FF000 (4KB lower)
    
    li sp, 0x80100000   # Load Top of RAM
    li t1, 0x1000       # 4KB per core
    mul t1, t1, t0      # Offset = id * 4KB
    sub sp, sp, t1      # Final SP = Top - Offset

    # 3. Jump to C function 'main'
    # We pass the Core ID as the first argument (a0)
    mv a0, t0
    call main

    # 4. If main returns, hang
hang:
    wfi
    j hang
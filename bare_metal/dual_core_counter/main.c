// compiled with: riscv64-unknown-elf-gcc -mcmodel=medany -T link.ld startup.S main.c -o multicore.elf -nostdlib -g
#include <stdint.h>

#define SIMDEV_CORE_DONE_ADDR 0x10000000
#define SIMDEV_CORE_DONE (*(volatile uint32_t *)(SIMDEV_CORE_DONE_ADDR))

#define SIMDEV_SOUT_ADDR 0x10000008
#define SIMDEV_SOUT (*(volatile uint32_t *)(SIMDEV_SOUT_ADDR))

// A simple custom print string function
void my_print(const char* str) {
    while (*str) {
        SIMDEV_SOUT = *str++;
    }
}


// THIS IS THE FUNCTION THE LINKER WAS LOOKING FOR!
void trap_dispatch(void)
{
    uintptr_t cause;
    asm volatile("csrr %0, mcause" : "=r"(cause));
    int is_interrupt = cause >> (sizeof(uintptr_t) * 8 - 1);
    int code = cause & 0xfff;
}

void main(int hartid)
{
    long TARGET = 1000000;

    if (hartid == 0)
    {
        for (long i = 0; i < TARGET; i++)
        {
            asm volatile(""); // Forces compiler to keep the loop, but does 0 memory access
        }
        my_print("Core 0 done!\n");
        SIMDEV_CORE_DONE = hartid; 
    }
    else
    {
        for (long i = 0; i < TARGET; i++)
        {
            asm volatile("");
        }

        my_print("Core 1 done!\n");
        SIMDEV_CORE_DONE = hartid; 
        
    }
}
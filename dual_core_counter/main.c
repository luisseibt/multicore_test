// compiled with: riscv64-unknown-elf-gcc -mcmodel=medany -T link.ld startup.S main.c -o multicore.elf -nostdlib -g

#include <stdint.h>

// 1. Define the "Exit Button" address (HTIF ToHost)
// Based on your previous logs, this is at 0x80001000
volatile uint32_t* tohost = (uint32_t*)0x80001000;

// 2. Global variables for counting and synchronization
volatile uint32_t counter_core0 = 0xAAAA0000;
volatile uint32_t counter_core1 = 0xBBBB0000;
volatile int core1_finished = 0;

void main(int hartid) {
    int i;

    if (hartid == 0) {
        // --- CORE 0 (Master) ---
        
        // Count 10 times
        for (i = 0; i < 10; i++) {
            counter_core0++;
        }

        // Wait for Core 1 to finish its job
        // (Spinlock until the flag changes)
        while (core1_finished == 0) {
            asm volatile ("nop");
        }

        // HIT THE KILL SWITCH
        // Writing 1 to tohost tells the simulator to exit
        *tohost = 1; 

        // Stop here just in case simulation takes a moment to close
        while(1); 

    } else {
        // --- CORE 1 (Worker) ---
        
        // Count 10 times
        for (i = 0; i < 10; i++) {
            counter_core1++;
        }

        // Signal to Core 0 that we are done
        core1_finished = 1;

        // Go to sleep forever (Wait For Interrupt)
        // This stops the instruction trace from spamming logs
        while(1) {
            asm volatile ("wfi");
        }
    }
}
// compiled with: riscv64-unknown-elf-gcc -mcmodel=medany -T link.ld startup.S main.c -o multicore.elf -nostdlib -g
#include <stdint.h>

#define UART_BASE 0x10009000
#define PLIC_BASE 0x1000a000
#define NIRQ 1024
#define NIRQ_LINES 32
#define NUM_EN_REG (NIRQ/NIRQ_LINES)
#define UART_IRQ       5
#define PLIC_CTX_M     0

#define UART_START_RX  (*(volatile uint8_t*)(UART_BASE + 0x0))
#define UART_START_TX  (*(volatile uint8_t*)(UART_BASE + 0x8))
#define UART_IRQ_EN    (*(volatile uint32_t*)(UART_BASE + 0x300))
#define UART_ENABLE    (*(volatile uint32_t*)(UART_BASE + 0x500))
#define UART_RX        (*(volatile uint8_t*)(UART_BASE + 0x518))
#define UART_TX        (*(volatile uint8_t*)(UART_BASE + 0x51c))
#define UART_BAUDRATE  (*(volatile uint32_t*)(UART_BASE + 0x524))

#define PLIC_PRIORITY(id) (*(volatile uint32_t*)(PLIC_BASE + (id) * 4))
#define PLIC_ENABLE(ctx, id) (*(volatile uint32_t*)(PLIC_BASE + 0x2000 + (ctx)*NUM_EN_REG*4 + ((id)/32)*4))
#define PLIC_THRESHOLD(ctx) (*(volatile uint32_t*)(PLIC_BASE + 0x200000 + (ctx)*0x1000))
#define PLIC_CLAIM(ctx) (*(volatile uint32_t*)(PLIC_BASE + 0x200004 + (ctx)*0x1000))

volatile int irq_flag = 0;
volatile uint32_t* tohost = (uint32_t*)0x80001000;

// Synchronization flags
volatile int uart_ready = 0;
volatile int core1_finished = 0;

// Variables to view in debugger
volatile uint32_t counter_core0 = 0xAAAA0000;
volatile uint32_t counter_core1 = 0xBBBB0000;


static inline void csr_set_mie(uintptr_t mask) {
    asm volatile("csrs mie, %0" :: "r"(mask));
}

static inline void csr_set_mstatus(uintptr_t mask) {
    asm volatile("csrs mstatus, %0" :: "r"(mask));
}

void uart_putc(char c) {
    UART_START_TX = 1;
    UART_TX = c;
    UART_START_TX = 0;
}

void uart_print(char* s) {
    while (*s) uart_putc(*s++);
}

void m_irq_handler(void) {
    uint32_t irq = PLIC_CLAIM(PLIC_CTX_M);
    if (irq == UART_IRQ) {
        char data = UART_RX;
        uart_putc(data);
        irq_flag = 1;
    }
    PLIC_CLAIM(PLIC_CTX_M) = irq;
}

void plic_init(void) {
    PLIC_PRIORITY(UART_IRQ) = 1;
    PLIC_ENABLE(PLIC_CTX_M, UART_IRQ) |= (1 << (UART_IRQ % 32));
    PLIC_THRESHOLD(PLIC_CTX_M) = 0;
}

void uart_init(void) {
    UART_ENABLE = 0x4;
    UART_BAUDRATE = 0x00275000;
    UART_IRQ_EN = 4;
    UART_START_RX = 1;
}

void trap_dispatch(void) {
    uintptr_t cause;
    asm volatile("csrr %0, mcause" : "=r"(cause));
    int is_interrupt = cause >> (sizeof(uintptr_t)*8 - 1);
    int code = cause & 0xfff;
    if (is_interrupt && code == 11) m_irq_handler();
}

void main(int hartid) {
    int i;

    if (hartid == 0) {
        // --- CORE 0 (Master) ---
        uart_init();
        
        // Signal Core 1 that the UART config is stable and ready to read
        uart_ready = 1; 

        plic_init();

        uart_print("Booting Core 0...\n");

        // Enable external interrupts
        csr_set_mie(1 << 11);     // MEIE
        csr_set_mstatus(1 << 3);  // global MIE

        // 1. Wait for the UART interrupt
        while (!irq_flag) {
            asm volatile("wfi");
        }
        uart_print("\nInterrupt received! Doing math...\n");

        // 2. Count 10 times
        for (i = 0; i < 10; i++) {
            counter_core0++;
        }

        // 3. Wait for Core 1 to finish its job
        while (core1_finished == 0) {
            asm volatile ("nop");
        }
        uart_print("Core 1 finished! Exiting...\n");

        // 4. Hit the kill switch
        *tohost = 1; 
        while(1); 

    } else {
        // --- CORE 1 (Worker) ---
        
        // 1. Wait until Core 0 has written the baud rate
        while (uart_ready == 0) {
            asm volatile("nop");
        }

        // 2. Safely read from the memory-mapped UART peripheral!
        // We will store it in our global variable so you can see it in GDB
        counter_core1 = UART_BAUDRATE; 

        // 3. Signal to Core 0 that we successfully read the bus
        core1_finished = 1;

        // Sleep forever
        while(1) {
            asm volatile ("wfi");
        }
    }
}
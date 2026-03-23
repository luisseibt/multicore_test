#include <stdint.h>
#include <stdio.h>

/* Use 'used' and ensure they are global (no static) */
__attribute__((used, section(".data"), aligned(16)))
volatile uint64_t tohost = 0;

__attribute__((used, section(".data"), aligned(16)))
volatile uint64_t fromhost = 0;

int main(void)
{
    /* FAKE USE: This prevents the linker from garbage collecting the symbols */
    if (tohost > 0) {
        printf("This will never happen, but the linker doesn't know that: %llu", tohost);
    }

    printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

    return 0;
}
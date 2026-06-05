#include <stdint.h>
#include "helpers.h"
// Deine ganzen Hardware-Adressen (SIMDEV_SOUT, UART, etc.) ...
// Deine my_print Funktion ...

// 1. Der Dummy-Timer für Dhrystone

// 2. Deklariere die umbenannte Dhrystone-Funktion
extern int dhrystone_main(); 

void main(int hartid) {
    if (hartid == 0) {
        // --- CORE 0 ---
        my_print("Core 0 startet Dhrystone...\n");
        
        dhrystone_main(); // Führe den Benchmark aus!
        
        my_print("Core 0 ist fertig!\n");
        SIMDEV_CORE_DONE = hartid; 
    }
    else {
        // --- CORE 1 ---
        // (Ggf. UART und PLIC Initialisierung hier)
        my_print("Core 1 startet Dhrystone...\n");
        
        dhrystone_main(); // Führe den Benchmark parallel aus!
        
        my_print("Core 1 ist fertig!\n");
        SIMDEV_CORE_DONE = hartid; 
    }
}
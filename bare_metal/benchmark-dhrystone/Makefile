# 1. Den richtigen RISC-V Cross-Compiler definieren
CC = riscv64-unknown-elf-gcc

# 2. Die originalen Dhrystone-Optimierungen beibehalten
DHRY-CFLAGS := -O3 -DTIME -DNOENUM -Wno-implicit
DHRY-CFLAGS += -fno-builtin-printf -fno-common -falign-functions=4

# 3. Unsere speziellen Bare-Metal-Flags hinzufügen
BARE_METAL_CFLAGS = -mcmodel=medany -nostdlib -g

# Alle CFLAGS zusammenbauen
override CFLAGS += $(DHRY-CFLAGS) $(BARE_METAL_CFLAGS)

# 4. ALLE Quelldateien inkl. startup.S angeben!
SRC = startup.S main.c dhry_1.c dhry_2.c strcmp.S
HDR = dhry.h

# ---------------------------------------------------------
# NEU: Das Standard-Target baut jetzt BEIDE Executables
# ---------------------------------------------------------
all: dhrystone_core0.elf dhrystone_core1.elf

# 5. Kompiliere Binary für Kern 0 (Nutzt linker_core0.ld)
dhrystone_core0.elf: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -T linker_core0.ld -o $@

# 6. Kompiliere Binary für Kern 1 (Nutzt linker_core1.ld)
dhrystone_core1.elf: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -T linker_core1.ld -o $@

# 7. Ein "clean" Befehl zum Aufräumen beider Dateien
clean:
	rm -f *.i *.s *.o dhrystone_core0.elf dhrystone_core1.elf
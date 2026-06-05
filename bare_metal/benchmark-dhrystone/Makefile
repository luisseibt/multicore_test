# 1. Den richtigen RISC-V Cross-Compiler definieren
CC = riscv64-unknown-elf-gcc

# 2. Die originalen Dhrystone-Optimierungen beibehalten
DHRY-CFLAGS := -O3 -DTIME -DNOENUM -Wno-implicit
DHRY-CFLAGS += -fno-builtin-printf -fno-common -falign-functions=4

# 3. Unsere speziellen Bare-Metal-Flags hinzufügen
BARE_METAL_CFLAGS = -mcmodel=medany -nostdlib -g

# 4. Das Linker-Skript definieren
LDFLAGS = -T link.ld

# 5. ALLE Quelldateien inkl. startup.S angeben!
SRC = startup.S main.c dhry_1.c dhry_2.c strcmp.S
HDR = dhry.h

# Alle CFLAGS zusammenbauen
override CFLAGS += $(DHRY-CFLAGS) $(BARE_METAL_CFLAGS)

# Das Ziel (Target) - Wir nennen es singlecore.elf, damit es zu deinem Simulator passt
singlecore.elf: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $@

# Ein "clean" Befehl zum Aufräumen
clean:
	rm -f *.i *.s *.o singlecore.elf
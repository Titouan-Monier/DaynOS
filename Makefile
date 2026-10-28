# Outils de la chaîne de compilation
ASM = nasm
CC  = x86_64-elf-gcc
EMU = qemu-system-x86_64

# Drapeaux de compilation et d'édition de liens
ASMFLAGS = -f elf32
CFLAGS   = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  = -m32 -Wl,-m,elf_i386 -T linker.ld -ffreestanding -O2 -nostdlib

# Fichiers cibles
OBJS   = boot.o kernel.o
TARGET = DaynOS.bin

.PHONY: all run clean

# Cible par défaut (exécutée en tapant simplement "make")
all: $(TARGET)

# Édition de liens : assemblage du binaire final
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Compilation du bootloader assembleur
boot.o: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.o

# Compilation du noyau C
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Lancement automatique dans l'émulateur
run: $(TARGET)
	$(EMU) -kernel $(TARGET)

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(TARGET)
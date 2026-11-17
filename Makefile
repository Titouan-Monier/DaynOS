# Outils de la chaîne de compilation
ASM  = nasm
CC   = x86_64-elf-gcc
EMU  = qemu-system-x86_64
GRUB = grub-mkrescue

# Drapeaux de compilation et d'édition de liens
ASMFLAGS = -f elf32
CFLAGS   = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  = -m32 -Wl,-m,elf_i386 -T linker.ld -ffreestanding -O2 -nostdlib

# Fichiers et dossiers cibles
OBJS   = boot.o kernel.o
TARGET = DaynOS.bin
ISO    = DaynOS.iso
ISODIR = isodir

.PHONY: all iso run clean

# Cible par défaut : génère l'image ISO complète
all: $(ISO)

# Création de l'image disque amorçable (.iso) avec GRUB
$(ISO): $(TARGET) grub.cfg
	mkdir -p $(ISODIR)/boot/grub
	cp $(TARGET) $(ISODIR)/boot/$(TARGET)
	cp grub.cfg $(ISODIR)/boot/grub/grub.cfg
	$(GRUB) -o $(ISO) $(ISODIR)

# Édition de liens : assemblage du binaire final
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Compilation du bootloader assembleur
boot.o: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.o

# Compilation du noyau C
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Lancement de l'image ISO dans l'émulateur
run: $(ISO)
	$(EMU) -cdrom $(ISO)

# Nettoyage des fichiers et dossiers générés
clean:
	rm -rf $(OBJS) $(TARGET) $(ISO) $(ISODIR)
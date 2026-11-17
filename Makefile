# Outils de la chaîne de compilation
ASM  = nasm
CC   = x86_64-elf-gcc
EMU  = qemu-system-x86_64
GRUB = grub-mkrescue

# Drapeaux de compilation et d'édition de liens
ASMFLAGS = -f elf32
CFLAGS = -m32 -march=i686 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS  = -m32 -Wl,-m,elf_i386 -T linker.ld -ffreestanding -O2 -nostdlib

# Fichiers et dossiers cibles
OBJS   = boot.o kernel.o gdt.o idt.o
TARGET = DaynOS.bin
ISO    = DaynOS.iso
ISODIR = isodir

.PHONY: all iso run clean

all: $(ISO)

$(ISO): $(TARGET) grub.cfg
	mkdir -p $(ISODIR)/boot/grub
	cp $(TARGET) $(ISODIR)/boot/$(TARGET)
	cp grub.cfg $(ISODIR)/boot/grub/grub.cfg
	$(GRUB) -o $(ISO) $(ISODIR)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

boot.o: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.o

# Règle générique pour compiler n'importe quel fichier .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(ISO)
	$(EMU) -cdrom $(ISO)

clean:
	rm -rf $(OBJS) $(TARGET) $(ISO) $(ISODIR)
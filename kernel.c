#include "gdt.h"
#include "idt.h"
#include "keyboard.h"

void kernel_main(void)
{
    init_gdt();
    init_idt();

    const char *logo[] = {
        "  ___                   ___  ___ ",
        " |   \\ __ _ _  _ _ _   / _ \\/ __|",
        " | |) / _` | || | ' \\ | (_) \\__ \\",
        " |___/\\__,_|\\_, |_||_| \\___/|___/",
        "            |__/                 "};
    const char *gdt_status = "[OK] GDT initialisee (Flat Memory Model - Ring 0/3)";
    const char *idt_status = "[OK] IDT & PIC 8259 initialises (ISR 0-31 / IRQ 32-47)";
    const char *kbd_status = "[OK] Pilote Clavier PS/2 actif (IRQ 1 - Port 0x60)";

    unsigned short *video_memory = (unsigned short *)0xB8000;
    int line_width = 80;

    for (int i = 0; i < 80 * 25; i++)
        video_memory[i] = (0x00 << 8) | ' ';

    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; logo[row][col] != '\0'; col++)
        {
            video_memory[row * line_width + col] = (unsigned short)logo[row][col] | (0x0B << 8);
        }
    }

    for (int col = 0; gdt_status[col] != '\0'; col++)
    {
        video_memory[6 * line_width + col] = (unsigned short)gdt_status[col] | (0x0A << 8);
    }
    for (int col = 0; idt_status[col] != '\0'; col++)
    {
        video_memory[7 * line_width + col] = (unsigned short)idt_status[col] | (0x0A << 8);
    }
    for (int col = 0; kbd_status[col] != '\0'; col++)
    {
        video_memory[8 * line_width + col] = (unsigned short)kbd_status[col] | (0x0A << 8);
    }

    // Auto-test d'interruption logicielle (Ligne 9)
    __asm__ volatile("int $0x03");

    // Initialisation du prompt clavier (Ligne 11) et activation des IRQ matérielles
    init_keyboard();
    __asm__ volatile("sti");

    // Boucle principale : vide tout octet résiduel puis met le CPU en veille jusqu'à la prochaine IRQ
    for (;;)
    {
        keyboard_isr();
        __asm__ volatile("hlt");
    }
}
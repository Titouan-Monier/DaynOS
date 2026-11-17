#include "gdt.h"
#include "idt.h"

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
    const char *idt_status = "[OK] IDT initialisee (256 portes - Exceptions ISR 0-31)";

    unsigned short *video_memory = (unsigned short *)0xB8000;
    int line_width = 80;

    // Effacer l'écran en noir
    for (int i = 0; i < 80 * 25; i++)
        video_memory[i] = (0x00 << 8) | ' ';

    // Dessiner le logo en cyan clair (0x0B)
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; logo[row][col] != '\0'; col++)
        {
            video_memory[row * line_width + col] = (unsigned short)logo[row][col] | (0x0B << 8);
        }
    }

    // Afficher la validation GDT (Ligne 7) et IDT (Ligne 8) en vert clair (0x0A)
    for (int col = 0; gdt_status[col] != '\0'; col++)
    {
        video_memory[7 * line_width + col] = (unsigned short)gdt_status[col] | (0x0A << 8);
    }
    for (int col = 0; idt_status[col] != '\0'; col++)
    {
        video_memory[8 * line_width + col] = (unsigned short)idt_status[col] | (0x0A << 8);
    }

    // Déclencher une interruption de test (Vecteur 3 : Breakpoint)
    __asm__ volatile("int $0x03");
}
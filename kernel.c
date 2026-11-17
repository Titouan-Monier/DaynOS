#include "gdt.h"

void kernel_main(void)
{
    // Initialisation de la segmentation mémoire (GDT)
    init_gdt();

    const char *logo[] = {
        "  ___                   ___  ___ ",
        " |   \\ __ _ _  _ _ _   / _ \\/ __|",
        " | |) / _` | || | ' \\ | (_) \\__ \\",
        " |___/\\__,_|\\_, |_||_| \\___/|___/",
        "            |__/                 "};
    const char *status = "[OK] GDT initialisee (Flat Memory Model - Ring 0/3)";

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

    // Afficher la validation GDT en vert clair (0x0A) sur la ligne 7
    for (int col = 0; status[col] != '\0'; col++)
    {
        video_memory[7 * line_width + col] = (unsigned short)status[col] | (0x0A << 8);
    }
}
void kernel_main(void) {
    const char* logo[] = {
        "  ___                   ___  ___ ",
        " |   \\ __ _ _  _ _ _   / _ \\/ __|",
        " | |) / _` | || | ' \\ | (_) \\__ \\",
        " |___/\\__,_|\\_, |_||_| \\___/|___/",
        "            |__/                 "
    };
    unsigned short* video_memory = (unsigned short*)0xB8000;
    int line_width = 80;
    
    // Effacer l'écran en noir
    for(int i = 0; i < 80 * 25; i++) video_memory[i] = (0x00 << 8) | ' ';

    // Dessiner le logo en cyan (0x0B)
    for(int row = 0; row < 5; row++) {
        for(int col = 0; logo[row][col] != '\0'; col++) {
            video_memory[row * line_width + col] = (unsigned short)logo[row][col] | (0x0B << 8);
        }
    }
}
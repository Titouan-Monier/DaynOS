#include "keyboard.h"
#include "idt.h"

static int cursor_row = 11;
static int cursor_col = 0;
static uint8_t irq_pulse = 0;

static const char scancode_azerty[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ')', '=', '\b',
    '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
    0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '%', '*', 0,
    '<', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, '*', 0, ' '};

static void kbd_wait_write(void)
{
    uint32_t timeout = 100000;
    while ((inb(0x64) & 0x02) && timeout--)
    {
    }
}

static void kbd_wait_read(void)
{
    uint32_t timeout = 100000;
    while (!(inb(0x64) & 0x01) && timeout--)
    {
    }
}

static void update_vga_cursor(int row, int col)
{
    uint16_t pos = (uint16_t)(row * 80 + col);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void print_prompt(void)
{
    unsigned short *vga = (unsigned short *)0xB8000;
    const char *prompt = "Dayn.OS # ";
    cursor_col = 0;
    for (int i = 0; prompt[i] != '\0'; i++)
    {
        vga[cursor_row * 80 + cursor_col] = (unsigned short)prompt[i] | (0x0B << 8);
        cursor_col++;
    }
    update_vga_cursor(cursor_row, cursor_col);
}

void init_keyboard(void)
{
    // 1. Désactiver temporairement le port PS/2 pendant la configuration
    kbd_wait_write();
    outb(0x64, 0xAD);

    // 2. Vider complètement le tampon de sortie 0x60
    while (inb(0x64) & 0x01)
    {
        inb(0x60);
    }

    // 3. Lire, modifier et réécrire l'octet de configuration du contrôleur 8042
    kbd_wait_write();
    outb(0x64, 0x20);
    kbd_wait_read();
    uint8_t config = inb(0x60);

    config |= (1 << 0) | (1 << 6); // Active IRQ 1 (bit 0) et traduction Set 1 (bit 6)
    config &= ~(1 << 4);           // Active l'horloge du port PS/2 (bit 4 à 0)

    kbd_wait_write();
    outb(0x64, 0x60);
    kbd_wait_write();
    outb(0x60, config);

    // 4. Réactiver le port PS/2 (0xAE) et le balayage clavier (0xF4)
    kbd_wait_write();
    outb(0x64, 0xAE);

    kbd_wait_write();
    outb(0x60, 0xF4);
    kbd_wait_read();
    inb(0x60); // Consomme l'acquittement 0xFA

    // 5. Acquitter toute interruption en attente sur le PIC
    outb(0x20, 0x20);

    print_prompt();
}

void keyboard_isr(void)
{
    unsigned short *vga = (unsigned short *)0xB8000;

    while (inb(0x64) & 0x01)
    {
        uint8_t scancode = inb(0x60);

        // Témoin visuel en haut à droite de l'écran à chaque signal reçu du port 0x60
        irq_pulse++;
        vga[79] = (unsigned short)('0' + (irq_pulse % 10)) | (0x2F << 8);

        if (scancode & 0x80)
        {
            continue;
        }

        char c = scancode_azerty[scancode];

        if (c == '\b')
        {
            if (cursor_col > 10)
            {
                cursor_col--;
                vga[cursor_row * 80 + cursor_col] = (0x0F << 8) | ' ';
                update_vga_cursor(cursor_row, cursor_col);
            }
        }
        else if (c == '\n')
        {
            cursor_row++;
            if (cursor_row >= 25)
            {
                cursor_row = 11;
                for (int r = 11; r < 25; r++)
                {
                    for (int col = 0; col < 80; col++)
                    {
                        vga[r * 80 + col] = (0x00 << 8) | ' ';
                    }
                }
            }
            print_prompt();
        }
        else if (c != 0 && cursor_col < 79)
        {
            vga[cursor_row * 80 + cursor_col] = (unsigned short)c | (0x0F << 8);
            cursor_col++;
            update_vga_cursor(cursor_row, cursor_col);
        }
    }
}
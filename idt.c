#include "idt.h"
#include "keyboard.h"

static daynos_idt_gate_t idt_table[256];
static daynos_idt_reg_t idt_reg;

extern uint32_t isr_stub_table[48];
extern void idt_load(uint32_t);

static const char *exception_labels[32] = {
    "Division par zero (#DE)",
    "Debug (#DB)",
    "Interruption non masquable (NMI)",
    "Point d'arret / Breakpoint (#BP)",
    "Debordement / Overflow (#OF)",
    "Depassement de bornes (#BR)",
    "Opcode invalide (#UD)",
    "Coprocesseur indisponible (#NM)",
    "Double faute (#DF)",
    "Depassement segment coprocesseur",
    "TSS invalide (#TS)",
    "Segment absent (#NP)",
    "Faute de segment de pile (#SS)",
    "Faute de protection generale (#GP)",
    "Faute de pagination (#PF)",
    "Reserve Intel (15)",
    "Erreur virgule flottante x87 (#MF)",
    "Controle d'alignement (#AC)",
    "Verification machine (#MC)",
    "Exception SIMD flottante (#XM)",
    "Exception de virtualisation (#VE)",
    "Exception de protection de controle (#CP)",
    "Reserve Intel (22)", "Reserve Intel (23)", "Reserve Intel (24)",
    "Reserve Intel (25)", "Reserve Intel (26)", "Reserve Intel (27)",
    "Reserve Intel (28)", "Reserve Intel (29)", "Reserve Intel (30)",
    "Reserve Intel (31)"};

// Reprogrammation du contrôleur PIC 8259 (IRQ 0-15 -> Vecteurs 32-47)
static void pic_remap(void)
{
    outb(0x20, 0x11); // ICW1 : Initialisation du PIC Maître
    outb(0xA0, 0x11); // ICW1 : Initialisation du PIC Esclave
    outb(0x21, 0x20); // ICW2 : Offset PIC Maître sur le vecteur 32 (0x20)
    outb(0xA1, 0x28); // ICW2 : Offset PIC Esclave sur le vecteur 40 (0x28)
    outb(0x21, 0x04); // ICW3 : Indique au Maître que l'Esclave est sur l'IRQ 2
    outb(0xA1, 0x02); // ICW3 : Identité cascade du PIC Esclave
    outb(0x21, 0x01); // ICW4 : Mode 8086
    outb(0xA1, 0x01); // ICW4 : Mode 8086

    // Masquage des lignes IRQ sauf l'IRQ 1 (clavier PS/2 = bit 1 mis à 0 -> 0xFD)
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}

static void idt_set_descriptor(uint8_t index, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_table[index].offset_low = (uint16_t)(base & 0xFFFF);
    idt_table[index].offset_high = (uint16_t)((base >> 16) & 0xFFFF);
    idt_table[index].selector = sel;
    idt_table[index].reserved = 0;
    idt_table[index].attributes = flags;
}

void init_idt(void)
{
    idt_reg.limit = (sizeof(daynos_idt_gate_t) * 256) - 1;
    idt_reg.base = (uint32_t)&idt_table;

    for (int i = 0; i < 256; i++)
    {
        idt_set_descriptor((uint8_t)i, 0, 0, 0);
    }

    // Reconfiguration du PIC 8259 avant l'enregistrement des 48 portes (ISR 0-31 + IRQ 32-47)
    pic_remap();

    for (uint8_t v = 0; v < 48; v++)
    {
        idt_set_descriptor(v, isr_stub_table[v], 0x08, 0x8E);
    }

    idt_load((uint32_t)&idt_reg);
}

void exception_handler(daynos_isr_frame_t *frame)
{
    // Traitement des interruptions matérielles PIC (Vecteurs 32 à 47)
    if (frame->vector >= 32 && frame->vector <= 47)
    {
        if (frame->vector == 33)
        {
            keyboard_isr(); // IRQ 1 : Clavier PS/2
        }
        // Envoi du signal EOI (End of Interrupt) au PIC pour autoriser les frappes suivantes
        if (frame->vector >= 40)
        {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
        return;
    }

    unsigned short *vga = (unsigned short *)0xB8000;
    int width = 80;

    if (frame->vector == 3)
    {
        const char *msg = "[OK] Test ISR (int 0x03 - Breakpoint) intercepte et repris !";
        for (int c = 0; msg[c] != '\0'; c++)
        {
            vga[9 * width + c] = (unsigned short)msg[c] | (0x0E << 8);
        }
        return;
    }

    const char *panic_prefix = "[PANIC] Exception CPU : ";
    const char *label = (frame->vector < 32) ? exception_labels[frame->vector] : "Inconnue";

    int col = 0;
    for (int i = 0; panic_prefix[i] != '\0'; i++, col++)
    {
        vga[11 * width + col] = (unsigned short)panic_prefix[i] | (0x4F << 8);
    }
    for (int i = 0; label[i] != '\0'; i++, col++)
    {
        vga[11 * width + col] = (unsigned short)label[i] | (0x4F << 8);
    }

    for (;;)
    {
        __asm__ volatile("cli; hlt");
    }
}
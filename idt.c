#include "idt.h"

static daynos_idt_gate_t idt_table[256];
static daynos_idt_reg_t idt_reg;

// Table des adresses des 32 points d'entrée générée dans boot.asm
extern uint32_t isr_stub_table[32];
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

    // Mise à zéro complète des 256 entrées
    for (int i = 0; i < 256; i++)
    {
        idt_set_descriptor((uint8_t)i, 0, 0, 0);
    }

    // Enregistrement des 32 exceptions matérielles x86 (Sélecteur 0x08, Attribut 0x8E)
    for (uint8_t v = 0; v < 32; v++)
    {
        idt_set_descriptor(v, isr_stub_table[v], 0x08, 0x8E);
    }

    idt_load((uint32_t)&idt_reg);
}

void exception_handler(daynos_isr_frame_t *frame)
{
    unsigned short *vga = (unsigned short *)0xB8000;
    int width = 80;

    // Si c'est le vecteur 3 (Breakpoint int 0x03), on affiche le succès et on reprend l'exécution
    if (frame->vector == 3)
    {
        const char *msg = "[OK] Test ISR (int 0x03 - Breakpoint) intercepte et repris !";
        for (int c = 0; msg[c] != '\0'; c++)
        {
            vga[9 * width + c] = (unsigned short)msg[c] | (0x0E << 8); // Jaune clair
        }
        return;
    }

    // Pour toute autre exception fatale : affichage d'une alerte rouge (Kernel Panic)
    const char *panic_prefix = "[PANIC] Exception CPU : ";
    const char *label = (frame->vector < 32) ? exception_labels[frame->vector] : "Inconnue";

    int col = 0;
    for (int i = 0; panic_prefix[i] != '\0'; i++, col++)
    {
        vga[11 * width + col] = (unsigned short)panic_prefix[i] | (0x4F << 8); // Blanc sur fond rouge
    }
    for (int i = 0; label[i] != '\0'; i++, col++)
    {
        vga[11 * width + col] = (unsigned short)label[i] | (0x4F << 8);
    }

    // Arrêt définitif du processeur
    for (;;)
    {
        __asm__ volatile("cli; hlt");
    }
}
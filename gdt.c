#include "gdt.h"

struct gdt_entry_struct gdt_entries[5];
struct gdt_ptr_struct gdt_ptr;

// Routine assembleur définie dans boot.asm
extern void gdt_flush(uint32_t);

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

void init_gdt(void)
{
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Index | Base | Limite     | Accès | Granularité | Rôle (Offset)
    gdt_set_gate(0, 0, 0, 0x00, 0x00);          // 0x00 : Descripteur nul (obligatoire)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 0x08 : Code Noyau (Ring 0, RX)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 0x10 : Données Noyau (Ring 0, RW)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // 0x18 : Code Utilisateur (Ring 3, RX)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // 0x20 : Données Utilisateur (Ring 3, RW)

    gdt_flush((uint32_t)&gdt_ptr);
}
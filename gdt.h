#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Structure d'un descripteur de segment GDT (8 octets / 64 bits)
struct gdt_entry_struct
{
    uint16_t limit_low;  // Bits 0-15 de la taille maximale du segment
    uint16_t base_low;   // Bits 0-15 de l'adresse de départ
    uint8_t base_middle; // Bits 16-23 de l'adresse de départ
    uint8_t access;      // Octet de droits d'accès (Ring 0/3, Exécutable, Lecture/Écriture)
    uint8_t granularity; // Granularité (blocs de 4 Ko, 32 bits) et bits 16-19 de la limite
    uint8_t base_high;   // Bits 24-31 de l'adresse de départ
} __attribute__((packed));

// Registre pointeur transmis à l'instruction assembleur lgdt (6 octets)
struct gdt_ptr_struct
{
    uint16_t limit; // Taille totale de la table en octets moins 1
    uint32_t base;  // Adresse mémoire de la première entrée gdt_entries[0]
} __attribute__((packed));

void init_gdt(void);

#endif
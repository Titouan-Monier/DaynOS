#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Descripteur d'une porte d'interruption (64 bits / 8 octets)
typedef struct
{
    uint16_t offset_low;  // Bits 0-15 de l'adresse de la routine assembleur
    uint16_t selector;    // Sélecteur de segment de code GDT (0x08)
    uint8_t reserved;     // Toujours à 0
    uint8_t attributes;   // Type de porte et privilèges (0x8E = Ring 0, Gate 32 bits)
    uint16_t offset_high; // Bits 16-31 de l'adresse de la routine assembleur
} __attribute__((packed)) daynos_idt_gate_t;

// Pointeur transmis à l'instruction matérielle lidt (48 bits / 6 octets)
typedef struct
{
    uint16_t limit; // Taille totale de la table en octets moins 1
    uint32_t base;  // Adresse linéaire du tableau idt_table
} __attribute__((packed)) daynos_idt_reg_t;

// État complet du processeur empilé par isr_common_router
typedef struct
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t vector;
    uint32_t error_code;
    uint32_t eip, cs, eflags;
} __attribute__((packed)) daynos_isr_frame_t;

void init_idt(void);
void exception_handler(daynos_isr_frame_t *frame);

#endif
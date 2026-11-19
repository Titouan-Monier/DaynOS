#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Écriture d'un octet sur un port matériel x86
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Lecture d'un octet depuis un port matériel x86
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Descripteur d'une porte d'interruption (64 bits / 8 octets)
typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t offset_high;
} __attribute__((packed)) daynos_idt_gate_t;

// Pointeur transmis à l'instruction matérielle lidt (48 bits / 6 octets)
typedef struct
{
    uint16_t limit;
    uint32_t base;
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
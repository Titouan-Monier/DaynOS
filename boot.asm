MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KB
stack_top:

section .text
global _start
global gdt_flush
extern kernel_main

_start:
    mov esp, stack_top
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang

; Charge la nouvelle GDT et actualise les registres de segments du CPU
gdt_flush:
    mov eax, [esp + 4]  ; Récupère l'adresse de gdt_ptr passée en argument
    lgdt [eax]          ; Charge le registre matériel GDTR

    mov ax, 0x10        ; 0x10 = Offset du segment Données Noyau
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush     ; Saut long vers l'offset 0x08 (Code Noyau) pour recharger CS
.flush:
    ret
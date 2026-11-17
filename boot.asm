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
global idt_load
global isr_stub_table

extern kernel_main
extern exception_handler

_start:
    cli
    mov esp, stack_top
    call kernel_main
.hang:
    hlt
    jmp .hang

; Chargement de la GDT
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

; Chargement de l'IDT
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; Macros de génération des points d'entrée d'exceptions (ISR 0 à 31)
%macro ISR_NOERR 1
isr_stub_%1:
    cli
    push dword 0          ; Code d'erreur fictif pour aligner la trame
    push dword %1         ; Numéro du vecteur
    jmp isr_common_router
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    cli
    push dword %1         ; Le CPU a déjà empilé le code d'erreur
    jmp isr_common_router
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; Routeur commun qui sauvegarde le contexte CPU et appelle le gestionnaire C
isr_common_router:
    pusha                 ; Empile eax, ecx, edx, ebx, esp, ebp, esi, edi
    mov ax, ds
    movzx eax, ax
    push eax              ; Sauvegarde le segment de données actif

    mov ax, 0x10          ; Bascule sur le segment de données noyau (Ring 0)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp              ; Passe le pointeur daynos_isr_frame_t* au code C
    call exception_handler
    add esp, 4            ; Nettoie l'argument pointeur

    pop eax               ; Restaure le segment de données initial
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                  ; Restaure les registres généraux
    add esp, 8            ; Nettoie le vecteur et le code d'erreur
    iret                  ; Retour d'interruption (restaure EIP, CS, EFLAGS)

section .rodata
align 4
isr_stub_table:
%assign i 0
%rep 32
    dd isr_stub_%+i
%assign i i+1
%endrep
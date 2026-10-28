# Dayn.OS

![Architecture](https://img.shields.io/badge/Architecture-x86__32_(i386)-00b4d8?style=flat-square&labelColor=0d1b2a)
![Standard](https://img.shields.io/badge/Boot-Multiboot_1-00b4d8?style=flat-square&labelColor=0d1b2a)
![Langages](https://img.shields.io/badge/Langages-NASM_%7C_C99-00b4d8?style=flat-square&labelColor=0d1b2a)
![Environnement](https://img.shields.io/badge/Build-WSL2_Ubuntu-00b4d8?style=flat-square&labelColor=0d1b2a)

```text
  ___                   ___  ___ 
 |   \ __ _ _  _ _ _   / _ \/ __|
 | |) / _` | || | ' \ | (_) \__ \
 |___/\__,_|\_, |_||_| \___/|___/
            |__/                 
```

Système d'exploitation minimaliste autonome (*freestanding*) développé en C et Assembleur x86.

---

## Architecture et Flux d'Amorçage

```text
[ BIOS / QEMU ] 
       │
       ▼
[ En-tête Multiboot ] ──► Validation du Magic Number (0x1BADB002)
       │
       ▼
[ boot.asm ]          ──► Allocation de la pile (16 Ko) & saut vers le C
       │
       ▼
[ kernel.c ]          ──► Écriture directe dans le buffer VGA (0xB8000)
```

### Structure du dépôt

```text
DaynOS/
├── boot.asm       # Amorçage Multiboot et initialisation du pointeur de pile (ESP)
├── kernel.c       # Noyau C et pilote d'affichage VGA mode texte
├── linker.ld      # Script d'édition de liens (alignement 4K, offset mémoire 1 Mo)
├── .gitignore     # Exclusion des artefacts de compilation (*.o, *.bin)
└── README.md      # Documentation technique
```

---

## Environnement et Chaîne de Compilation

Le projet compile via une chaîne d'outils croisée (*cross-compiler*) isolée du système hôte sous WSL 2 (Ubuntu).

| Composant | Outil / Cible | Rôle |
| :--- | :--- | :--- |
| **Assembleur** | `NASM` (Format `elf32`) | Compilation du point d'entrée matériel |
| **Compilateur** | `x86_64-elf-gcc` (Flags `-m32 -ffreestanding`) | Compilation du noyau sans bibliothèque standard |
| **Éditeur de liens** | `GNU ld` (`elf_i386`) | Assemblage des segments mémoire selon `linker.ld` |
| **Émulateur** | `QEMU` (`qemu-system-x86_64`) | Exécution et test direct du binaire noyau |

---

## Compilation et Exécution

Exécuter les commandes suivantes à la racine du dépôt :

```bash
# 1. Assemblage du bootloader
nasm -f elf32 boot.asm -o boot.o

# 2. Compilation du noyau C
x86_64-elf-gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# 3. Édition de liens (Génération du binaire final)
x86_64-elf-gcc -m32 -Wl,-m,elf_i386 -T linker.ld -o DaynOS.bin -ffreestanding -O2 -nostdlib boot.o kernel.o

# 4. Lancement dans l'émulateur
qemu-system-x86_64 -kernel DaynOS.bin
```

---

## Feuille de Route (Roadmap)

### Phase 1 : Environnement et Amorçage (Terminé)
- [x] Configuration du sous-système Linux (WSL 2) et de l'émulateur QEMU
- [x] Compilation des sources GNU Binutils (2.41) et du Cross-Compiler GCC (13.2.0)
- [x] Écriture de l'en-tête Multiboot et allocation de la pile en assembleur
- [x] Script de linkage mémoire en architecture 32 bits (`elf32-i386`)
- [x] Affichage d'un logo ASCII dans le buffer vidéo matériel (`0xB8000`)

### Phase 2 : Automatisation et Architecture Noyau (À faire)
- [ ] Création d'un `Makefile` pour automatiser le cycle de build et de nettoyage
- [ ] Création d'une image disque amorçable (`.iso`) avec GRUB (`grub-mkrescue`)
- [ ] Implémentation de la GDT (*Global Descriptor Table*) pour segmenter la mémoire
- [ ] Mise en place de l'IDT (*Interrupt Descriptor Table*) pour gérer les interruptions matérielles
- [ ] Développement d'un pilote clavier PS/2 (lecture des ports I/O `0x60` et `0x64`)
- [ ] Passage du mode texte VGA au mode graphique VESA (*Framebuffer*) pour le rendu par pixels
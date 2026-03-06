; src/loader.s
BITS 32
GLOBAL _start
EXTERN kmain

SECTION .multiboot
align 4
MB_MAGIC     equ 0x1BADB002
MB_FLAGS     equ 0x00000003
MB_CHECKSUM  equ -(MB_MAGIC + MB_FLAGS)

dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM

SECTION .bss
align 16
stack_bottom:
resb 16384
stack_top:

SECTION .text
_start:
    cli
    mov esp, stack_top

    ; GRUB gives:
    ; eax = 0x2BADB002 (magic)
    ; ebx = multiboot_info pointer
    push ebx            ; 2nd arg
    push eax            ; 1st arg
    call kmain

.hang:
    hlt
    jmp .hang
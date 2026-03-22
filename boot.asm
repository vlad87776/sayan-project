; Multiboot header + entry point for a 32-bit kernel

BITS 32

SECTION .multiboot
ALIGN 4
    dd 0x1BADB002                 ; magic
    dd 0x00000000                 ; flags
    dd -(0x1BADB002 + 0x00000000) ; checksum

SECTION .text
GLOBAL _start
EXTERN kernel_main

_start:
    cli
    mov esp, stack_top
    call kernel_main

.hang:
    hlt
    jmp .hang

SECTION .bss
ALIGN 16
stack_bottom:
    resb 16384
stack_top:

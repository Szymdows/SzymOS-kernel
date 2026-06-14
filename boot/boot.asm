; SzymOS bootloader stub
; This sets up the environment and jumps to the kernel

MULTIBOOT_MAGIC        equ 0x1BADB002
MULTIBOOT_PAGE_ALIGN   equ 1 << 0
MULTIBOOT_MEMORY_INFO  equ 1 << 1
MULTIBOOT_FLAGS        equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM     equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:

section .text
global start
extern kernel_main

start:
    ; Set up the stack
    mov esp, stack_top
    mov ebp, esp

    ; Reset EFLAGS
    push 0
    popf

    ; Push multiboot info (we might use it later)
    push ebx
    push eax

    ; Call the kernel
    call kernel_main

    ; If kernel returns, just hang
.hang:
    cli
    hlt
    jmp .hang

; Mark stack as non-executable
section .note.GNU-stack noalloc noexec nowrite progbits
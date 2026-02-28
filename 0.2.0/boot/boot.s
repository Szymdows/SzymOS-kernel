/* boot/boot.s */
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

/* Multiboot Header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Stack Reservation */
.section .bss
.align 16
stack_bottom:
.skip 16384 /* 16 KiB */
stack_top:

/* Kernel Entry */
.section .text
.global _start
.type _start, @function
_start:
	mov $stack_top, %esp

	/* Call the C kernel */
	call kmain

	/* Infinite loop if kmain returns */
	cli
1:	hlt
	jmp 1b

/* GDT Flush Function (Added for v0.2.0) */
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax   /* Get the pointer to the GDT */
    lgdt (%eax)         /* Load the new GDT pointer */

    mov $0x10, %ax      /* 0x10 is the offset to our Data Segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Far jump to Code Segment (0x08) to reload CS */
    jmp $0x08, $.flush
.flush:
    ret

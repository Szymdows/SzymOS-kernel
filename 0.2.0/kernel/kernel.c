/* kernel/kernel.c */
#include "../include/vga.h"
#include "../include/version.h"
#include "../cpu/gdt.h"

void kmain(void) {
    term_init();
    
    /* Professional Boot Banner */
    term_print("##################################################\n");
    term_print("#  ");
    term_print(KERNEL_NAME);
    term_print(" ");
    term_print(KERNEL_VERSION);
    term_print(" (");
    term_print(KERNEL_ARCH);
    term_print(")                 #\n");
    term_print("#  Built: ");
    term_print(KERNEL_BUILD_DATE);
    term_print(" at ");
    term_print(KERNEL_BUILD_TIME);
    term_print("               #\n");
    term_print("##################################################\n\n");

    term_print("[ OK ] Video Driver Initialized\n");
    
    /* Initialize GDT */
    term_print("[....] Initializing GDT (Memory Segmentation)...\n");
    init_gdt();
    term_print("[ OK ] GDT Loaded Successfully\n");

    term_print("\n");
    term_print(KERNEL_NAME);
    term_print(" is ready. Awaiting input...\n");
}
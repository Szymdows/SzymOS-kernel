/*
 * SzymOS Kernel Main
 * Copyright (c) SzymOS contributors
 */

#include <kernel/kernel.h>
#include <drivers/vga.h>
#include <libc/string.h>

void kernel_init(void) {
    vga_init();
}

void kernel_main(void) {
    kernel_init();
    
    vga_clear();
    
    // Display boot messages
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("SzymOS Kernel v0.1.0\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write("Developed by SzymOS contributors\n\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("[OK] Kernel initialized\n");
    vga_write("[OK] VGA text mode driver loaded\n");
    
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_write("\nWelcome to SzymOS!\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("\nSystem is running...\n");
    
    // TODO: add more subsystems here when we implement them
    
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_write("\n---\n");
    vga_write("Nothing else to do. System halted.\n");
    
    // Kernel main loop (for now, just halt)
    while(1) {
        __asm__ volatile("hlt");
    }
}
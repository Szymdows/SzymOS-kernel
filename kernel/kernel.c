#include "../include/kernel.h"
#include "terminal.h"
#include "keyboard.h"
#include "shell.h"
#include "memory.h"
#include "io.h"
#include "filesystem.h"
#include "disk_selector.h"
#include "ata.h"
#include "disk_selector.h"

// Utility functions
void* memset(void* dest, int val, size_t len) {
    uint8_t* ptr = (uint8_t*)dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (len--)
        *d++ = *s++;
    return dest;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void kernel_panic(const char* message) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_writestring("\n\n*** KERNEL PANIC ***\n");
    terminal_writestring(message);
    terminal_writestring("\n\nSystem halted.");
    
    __asm__ volatile ("cli");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// SzymOS unique feature: Boot art
//void print_boot_art(void) {
//    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
//    terminal_writestring("   ____                       ___  ____\n");
//    terminal_writestring("  / ___| _____ _ _ __ ___    / _ \\/ ___|\n");
//    terminal_writestring("  \\___ \\|_  / | | '_ ` _ \\  | | | \\___ \\\n");
//    terminal_writestring("   ___) |/ /| |_| | | | | | | |_| |___) |\n");
//    terminal_writestring("  |____//___|\\__, |_| |_| |_|\\___/|____/\n");
//    terminal_writestring("             |___/\n\n");
//}

void kernel_main(uint32_t magic, uint32_t addr) {
    // Setting up the terminal
    terminal_initialize();
    
    print_boot_art();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_printf("SzymOS Kernel v%s (%s)\n", SZYMOS_VERSION, SZYMOS_CODENAME);
    terminal_printf("Copyright (c) %s\n\n", SZYMOS_YEAR);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Verify multiboot
    if (magic != 0x2BADB002) {
        kernel_panic("Invalid multiboot magic number!");
    }
    
    terminal_printf("Multiboot magic: 0x%x\n", magic);
    terminal_printf("Multiboot info at: 0x%x\n\n", addr);
    
    // Initialize subsystems
    memory_init(addr);
    keyboard_init();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("[OK] Terminal initialized\n");
    terminal_writestring("[OK] Memory subsystem initialized\n");
    terminal_writestring("[OK] Keyboard driver loaded\n");
    terminal_writestring("[OK] I/O subsystem ready\n");
    
    // Initialize and detect ATA drives
    ata_init();
    ata_detect_drives();
    terminal_writestring("[OK] ATA driver initialized\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nSzymOS kernel loaded successfully!\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Let user select which drive to use
    disk_selector_run();
    
    // Initialize filesystem IMMEDIATELY after selection, don't do anything else first
    fs_init();
    
    // Initialize and run shell
    shell_init();
    shell_run();
    
    kernel_panic("Shell exited unexpectedly!");
}

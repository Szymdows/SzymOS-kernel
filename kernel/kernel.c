/*
 * SzymOS Kernel Main
 * Copyright (c) SzymOS contributors
 */

#include <kernel/kernel.h>
#include <kernel/multiboot.h>
#include <kernel/timer.h>
#include <drivers/vga.h>
#include <arch/x86/cpu.h>
#include <libc/string.h>

static void display_boot_art(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_write("   ____                       ___  ____\n");
    vga_write("  / ___| _____ _ _ __ ___    / _ \\/ ___|\n");
    vga_write("  \\___ \\|_  / | | '_ ` _ \\  | | | \\___ \\\n");
    vga_write("   ___) |/ /| |_| | | | | | | |_| |___) |\n");
    vga_write("  |____//___|\\__, |_| |_| |_|\\___/|____/\n");
    vga_write("             |___/\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("\n  Version 0.1.0\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write("  Made by SzymOS contributors\n\n");
}

static void print_ok(const char* msg) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("  OK  ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write(msg);
    vga_write("\n");
}

static void print_info(const char* msg) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[ ");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("INFO");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write(" ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write(msg);
    vga_write("\n");
}

static int check_multiboot(void) {
    if (multiboot_check()) {
        print_ok("Multiboot signature verified");
        return 1;
    }
    return 0;
}

static void check_memory(void) {
    if (mb_info && (mb_info->flags & 0x1)) {
        print_ok("Memory information available");
        
        uint32_t total_kb = mb_info->mem_lower + mb_info->mem_upper;
        uint32_t total_mb = total_kb / 1024;
        
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_write("[ ");
        vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
        vga_write("INFO");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_write(" ] ");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_write("Detected ");
        
        // Simple number to string conversion
        char num[16];
        int i = 0;
        uint32_t temp = total_mb;
        if (temp == 0) {
            num[i++] = '0';
        } else {
            int start = i;
            while (temp > 0) {
                num[i++] = '0' + (temp % 10);
                temp /= 10;
            }
            // Reverse
            for (int j = start; j < (start + i) / 2; j++) {
                char t = num[j];
                num[j] = num[i - 1 - (j - start)];
                num[i - 1 - (j - start)] = t;
            }
        }
        num[i] = '\0';
        
        vga_write(num);
        vga_write(" MB RAM\n");
    } else {
        print_info("Memory information not available");
    }
}

static void check_cpu(void) {
    cpu_info_t cpu;
    cpu_detect(&cpu);
    
    print_ok("CPU detected");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("[ ");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("INFO");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write(" ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_write("CPU Vendor: ");
    vga_write(cpu.vendor);
    vga_write("\n");
    
    // Check for some features
    if (cpu_has_feature(CPU_FEATURE_FPU)) {
        print_ok("FPU detected");
    }
}

void kernel_init(void) {
    vga_init();
    timer_init();
}

void kernel_main(uint32_t magic, void* mb_info_ptr) {
    kernel_init();
    vga_clear();
    
    // Initialize multiboot info
    multiboot_init(magic, (multiboot_info_t*)mb_info_ptr);
    
    // Display boot art
    display_boot_art();
    
    // Perform system checks with delays like Linux
    print_info("Starting SzymOS kernel...");
    
    if (!check_multiboot()) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write("[FAILED] Not loaded by multiboot bootloader!\n");
        while(1) { __asm__ volatile("hlt"); }
    }
    
    print_ok("VGA text mode initialized");
    
    check_cpu();
    
    check_memory();
    
    print_ok("Timer subsystem initialized");
    
    print_ok("Kernel core loaded");
    
    vga_write("\n");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_write("Welcome to SzymOS!\n\n");
    
    // Enable cursor and show prompt
    vga_enable_cursor(0, 15);
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write("szym");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write("@");
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_write("kernel");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_write(":$ ");
    
    vga_update_cursor();
    
    // Kernel main loop (for now, just halt)
    while(1) {
        __asm__ volatile("hlt");
    }
}
/*
 * VGA Text Mode Driver
 * Simple 80x25 text mode driver for SzymOS
 */

#include <drivers/vga.h>
#include <arch/x86/io.h>
#include <libc/string.h>
#include <libc/stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

static uint16_t* vga_buffer;
static uint8_t vga_row;
static uint8_t vga_col;
static uint8_t vga_current_color;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static inline uint8_t vga_color_code(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

void vga_init(void) {
    vga_buffer = (uint16_t*)VGA_MEMORY;
    vga_row = 0;
    vga_col = 0;
    vga_current_color = vga_color_code(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_current_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_current_color = vga_color_code(fg, bg);
}

uint8_t vga_get_row(void) {
    return vga_row;
}

uint8_t vga_get_col(void) {
    return vga_col;
}

void vga_enable_cursor(uint8_t start, uint8_t end) {
    outb(VGA_CTRL_PORT, 0x0A);
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xC0) | start); // What is wrong with this!? I've done this before!
    
    outb(VGA_CTRL_PORT, 0x0B);
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xE0) | end);
}

void vga_disable_cursor(void) {
    outb(VGA_CTRL_PORT, 0x0A);
    outb(VGA_DATA_PORT, 0x20);
}

void vga_update_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_col;
    
    outb(VGA_CTRL_PORT, 0x0F);
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_PORT, 0x0E);
    outb(VGA_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_current_color);
    }
    
    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll();
        }
        vga_update_cursor();
        return;
    }
    
    if (c == '\r') {
        vga_col = 0;
        vga_update_cursor();
        return;
    }
    
    if (c == '\t') {
        // Simple tab implementation - just add 4 spaces
        for (int i = 0; i < 4; i++) {
            vga_putchar(' ');
        }
        return;
    }
    
    const size_t index = vga_row * VGA_WIDTH + vga_col;
    vga_buffer[index] = vga_entry(c, vga_current_color);
    
    vga_col++;
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll();
        }
    }
    
    vga_update_cursor();
}

void vga_write(const char* str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
}

void vga_write_color(const char* str, uint8_t fg, uint8_t bg) {
    uint8_t old_color = vga_current_color;
    vga_set_color(fg, bg);
    vga_write(str);
    vga_current_color = old_color;
}
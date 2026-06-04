#include "terminal.h"
#include "../include/kernel.h"
#include "io.h"
#include <stdarg.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

static uint16_t* terminal_buffer;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

// Cursor functions
static void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

static void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_update_cursor(void) {
    update_cursor(terminal_column, terminal_row);
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    // Enable blinking cursor
    enable_cursor(0, 15);
    update_cursor(0, 0);
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// Printing the Os logo
void print_boot_art(void) {
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
  char l1[] = "   ____                       ___  ____\n";
  char l2[] = "  / ___| _____ _ _ __ ___    / _ \\/ ___|\n";
  char l3[] = "  \\___ \\|_  / | | '_ ` _ \\  | | | \\___ \\\n"; 
  char l4[] = "   ___) |/ /| |_| | | | | | | |_| |___) |\n";
  char l5[] = "  |____//___|\\__, |_| |_| |_|\\___/|____/\n"; 
  char l6[] = "             |___/\n\n";

  char *str[] = {&l1, &l2, &l3, &l4, &l5, &l6};
  
  for (int i = 0; i < 5; i++) {
    terminal_writestring(str[i]);
  }
  return;
}

void terminal_scroll(void) {
    // Move all rows up
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear last row
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

// Default terminal function to display chars
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
            update_cursor(terminal_column, terminal_row);
        }
        return;
    }
    
    if (c == '\t') {
        terminal_column = (terminal_column + 4) & ~(4 - 1);
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
        update_cursor(terminal_column, terminal_row);
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry(c, terminal_color);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
    
    update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    update_cursor(0, 0);
}

// Simple printf implementation
void terminal_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 's': {
                    char* str = va_arg(args, char*);
                    terminal_writestring(str);
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char buffer[12];
                    int i = 0;
                    int is_negative = 0;
                    
                    if (num < 0) {
                        is_negative = 1;
                        num = -num;
                    }
                    
                    if (num == 0) {
                        buffer[i++] = '0';
                    } else {
                        while (num > 0) {
                            buffer[i++] = '0' + (num % 10);
                            num /= 10;
                        }
                    }
                    
                    if (is_negative) {
                        buffer[i++] = '-';
                    }
                    
                    while (i > 0) {
                        terminal_putchar(buffer[--i]);
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[9];
                    int i = 0;
                    
                    if (num == 0) {
                        terminal_writestring("0x0");
                    } else {
                        while (num > 0) {
                            int digit = num % 16;
                            buffer[i++] = digit < 10 ? '0' + digit : 'A' + digit - 10;
                            num /= 16;
                        }
                        
                        terminal_writestring("0x");
                        while (i > 0) {
                            terminal_putchar(buffer[--i]);
                        }
                    }
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);
                    terminal_putchar(c);
                    break;
                }
                case '%':
                    terminal_putchar('%');
                    break;
            }
        } else {
            terminal_putchar(*format);
        }
        format++;
    }
    
    va_end(args);
}

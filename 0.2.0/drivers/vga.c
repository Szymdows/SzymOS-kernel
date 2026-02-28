/* drivers/vga.c */
#include <stdint.h>
#include <stddef.h>
#include "../include/vga.h"

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;
int term_col = 0;
int term_row = 0;

static uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void term_init() {
    term_col = 0;
    term_row = 0;
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {
            const size_t index = (VGA_COLS * row) + col;
            vga_buffer[index] = vga_entry(' ', vga_entry_color(7, 0));
        }
    }
}

void term_putc(char c) {
    switch (c) {
        case '\n':
            term_col = 0;
            term_row++;
            break;
        default: {
            const size_t index = (VGA_COLS * term_row) + term_col;
            vga_buffer[index] = vga_entry(c, vga_entry_color(15, 0));
            term_col++;
            break;
        }
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
    }
}

void term_print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++)
        term_putc(str[i]);
}
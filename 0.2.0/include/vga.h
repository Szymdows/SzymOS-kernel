/* include/vga.h */
#ifndef VGA_H
#define VGA_H

void term_init();
void term_print(const char* str);
void term_putc(char c);

#endif
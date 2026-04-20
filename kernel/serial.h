#ifndef SERIAL_H
#define SERIAL_H
#include "io.h"
#include "terminal.h"
#define PORT 0x3F8

static inline void serial_init() {
    outb(PORT + 1, 0x00);
    outb(PORT + 3, 0x80);
    outb(PORT + 0, 0x03);
    outb(PORT + 1, 0x00);
    outb(PORT + 3, 0x03);
    outb(PORT + 2, 0xC7);
    outb(PORT + 4, 0x0B);
    
    outb(PORT + 4, 0x1E);
    outb(PORT + 0, 0xAE);

    if(inb(PORT + 0) != 0xAE) {
        terminal_writestring("Connection Failed");
    }
    outb(PORT +4, 0x0F);
}

static inline int transmit_via_empty() {
    return inb(PORT + 5) & 0x20;
}
static inline int receive_via_empty() {
    return inb(PORT +5) & 0x01;
}
static inline void write_serial(char a) {
    while(transmit_via_empty() == 0);
    outb(PORT,a);
}

static inline char read_serial() {
    while(receive_via_empty() == 0);
    return inb(PORT);
}

static inline void serial_print(const char* str) {
    for(int i=0; str[i] != '\0'; i++) {
       write_serial(str[i]);
    }
}

static inline void serial_read(char* buffer, int limit) {
    int i = 0;
    while(i < limit - 1) {
        char c = read_serial();
	if (c == '\0') {
	    break;
	}
	buffer[i] = c;
	i++;
    }
    buffer[i]='\0';
}
#endif

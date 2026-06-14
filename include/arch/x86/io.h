#ifndef ARCH_X86_IO_H
#define ARCH_X86_IO_H

#include <libc/stdint.h>

// Port I/O functions
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

// Delay function
void io_wait(void);

#endif /* ARCH_X86_IO_H */
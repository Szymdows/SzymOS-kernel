#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <libc/stdint.h>

void timer_init(void);
void delay_ms(uint32_t ms);
void delay(uint32_t loops);

#endif /* KERNEL_TIMER_H */
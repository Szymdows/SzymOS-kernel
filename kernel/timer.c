/*
 * Simple timer and delay functions
 */

#include <kernel/timer.h>

void timer_init(void) {
    // For now, use busy-wait delays
    // TODO: Set up PIT interrupt handler
}

// Simple busy-wait delay
void delay(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops * 10000; i++) {
        __asm__ volatile("nop");
    }
}

void delay_ms(uint32_t ms) {
    // Rough approximation - not accurate without interrupts
    delay(ms * 100);
}
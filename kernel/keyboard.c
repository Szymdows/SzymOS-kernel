#include "keyboard.h"
#include "io.h"
#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// US QWERTY keyboard layout
static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* F1 key */
    0, 0, 0, 0, 0, 0, 0, 0, 0,  /* F2-F10 */
    0,  /* Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0, 0, 0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

// Shifted characters
static const char keyboard_map_shifted[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
};

static int shift_pressed = 0;
static uint8_t last_scancode = 0;

void keyboard_init(void) {
    shift_pressed = 0;
    last_scancode = 0;
    
    // Clear the keyboard buffer
    while (inb(KEYBOARD_STATUS_PORT) & 1) {
        inb(KEYBOARD_DATA_PORT);
    }
}

int keyboard_haskey(void) {
    return (inb(KEYBOARD_STATUS_PORT) & 1);
}

char keyboard_getchar(void) {
    uint8_t scancode;
    char c;
    
    // Wait for a key with active polling
    while (1) {
        if (keyboard_haskey()) {
            scancode = inb(KEYBOARD_DATA_PORT);
            
            // Avoid processing the same scancode twice
            if (scancode == last_scancode) {
                continue;
            }
            last_scancode = scancode;
            
            // Handle shift keys (press)
            if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = 1;
                continue;
            }
            
            // Handle shift release
            if (scancode == 0xAA || scancode == 0xB6) {
                shift_pressed = 0;
                continue;
            }
            
            // Ignore all key releases (bit 7 set)
            if (scancode & 0x80) {
                continue;
            }
            
            // Get the character
            if (shift_pressed && scancode < 128) {
                c = keyboard_map_shifted[scancode];
            } else if (scancode < 128) {
                c = keyboard_map[scancode];
            } else {
                c = 0;
            }
            
            if (c != 0) {
                return c;
            }
        }
    }
}

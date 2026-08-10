#include "memory.h"
#include "terminal.h"

static memory_info_t mem_info;

// Multiboot info structure (simplified)
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    // ... other fields we don't need yet
} __attribute__((packed));

void memory_init(uint32_t multiboot_info) {
    if (multiboot_info == 0) {
        mem_info.lower = 0;
        mem_info.upper = 0;
        mem_info.total = 0;
        return;
    }
    
    struct multiboot_info* mbi = (struct multiboot_info*)multiboot_info;
    
    if (mbi->flags & 0x01) {
        mem_info.lower = mbi->mem_lower;
        mem_info.upper = mbi->mem_upper;
        mem_info.total = mem_info.lower + mem_info.upper;
    }
}

void memory_print_info(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=== Memory Information ===\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    terminal_printf("Lower Memory: %d KB\n", mem_info.lower);
    terminal_printf("Upper Memory: %d KB\n", mem_info.upper);
    terminal_printf("Total Memory: %d KB (%d MB)\n",
                    mem_info.total, mem_info.total / 1024);
}

uint32_t get_total_memory(void) {
    return mem_info.total;
}
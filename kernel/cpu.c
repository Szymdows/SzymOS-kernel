#include "cpu.h"
#include "terminal.h"
#include <stdint.h>

static int check_cpuid(void) {
    uint32_t result;
    asm volatile (
        "pushfl\n"
        "pop %%eax\n"
        "mov %%eax, %%ecx\n"
        "xor $0x200000, %%eax\n"
        "push %%eax\n"
        "popfl\n"
        "pushfl\n"
        "pop %%eax\n"
        "xor %%ecx, %%eax\n"
        "and $0x200000, %%eax\n"
        : "=a"(result)
        :
        : "ecx"
    );
    return result != 0;
}

static void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    asm volatile (
        "cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(code)
    );
}

void cpu_detect(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;
    
    info->has_cpuid = check_cpuid();
    
    if (!info->has_cpuid) {
        info->vendor[0] = '\0';
        info->signature = 0;
        info->features = 0;
        return;
    }
    
    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    
    *(uint32_t*)(info->vendor + 0) = ebx;
    *(uint32_t*)(info->vendor + 4) = edx;
    *(uint32_t*)(info->vendor + 8) = ecx;
    info->vendor[12] = '\0';
    
    // Get CPU signature and features
    cpuid(1, &eax, &ebx, &ecx, &edx);
    info->signature = eax;
    info->features = edx;
}

void cpu_print_info(void) {
    cpu_info_t cpu;
    cpu_detect(&cpu);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n=== CPU Information ===\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    if (!cpu.has_cpuid) {
        terminal_writestring("CPUID not supported (pre-Pentium CPU)\n\n");
        return;
    }
    
    terminal_printf("Vendor: %s\n", cpu.vendor);
    terminal_printf("Signature: 0x%x\n", cpu.signature);
    
    terminal_writestring("Features: ");
    if (cpu.features & (1 << 0)) terminal_writestring("FPU ");
    if (cpu.features & (1 << 4)) terminal_writestring("TSC ");
    if (cpu.features & (1 << 5)) terminal_writestring("MSR ");
    if (cpu.features & (1 << 23)) terminal_writestring("MMX ");
    if (cpu.features & (1 << 25)) terminal_writestring("SSE ");
    if (cpu.features & (1 << 26)) terminal_writestring("SSE2 ");
    terminal_writestring("\n\n");
}

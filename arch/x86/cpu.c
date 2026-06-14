/*
 * CPU detection and feature checking
 */

#include <arch/x86/cpu.h>
#include <libc/string.h>

static cpu_info_t current_cpu;

static void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}

void cpu_detect(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;
    
    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    
    *(uint32_t*)(info->vendor + 0) = ebx;
    *(uint32_t*)(info->vendor + 4) = edx;
    *(uint32_t*)(info->vendor + 8) = ecx;
    info->vendor[12] = '\0';
    
    // Get feature flags
    cpuid(1, &eax, &ebx, &ecx, &edx);
    info->features = edx;
    
    // Save globally
    memcpy(&current_cpu, info, sizeof(cpu_info_t));
}

int cpu_has_feature(uint32_t feature) {
    return (current_cpu.features & feature) != 0;
}
#ifndef ARCH_X86_CPU_H
#define ARCH_X86_CPU_H

#include <libc/stdint.h>

typedef struct {
    char vendor[13];
    uint32_t features;
} cpu_info_t;

void cpu_detect(cpu_info_t* info);
int cpu_has_feature(uint32_t feature);

// CPU feature flags
#define CPU_FEATURE_FPU     (1 << 0)
#define CPU_FEATURE_PSE     (1 << 3)
#define CPU_FEATURE_TSC     (1 << 4)
#define CPU_FEATURE_MSR     (1 << 5)
#define CPU_FEATURE_PAE     (1 << 6)
#define CPU_FEATURE_APIC    (1 << 9)

#endif /* ARCH_X86_CPU_H */
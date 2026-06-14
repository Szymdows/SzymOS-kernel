#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include <libc/stdint.h>

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
} multiboot_info_t;

extern multiboot_info_t* mb_info;

void multiboot_init(uint32_t magic, multiboot_info_t* info);
int multiboot_check(void);

#endif /* KERNEL_MULTIBOOT_H */
/*
 * Multiboot information handling
 */

#include <kernel/multiboot.h>

#define MULTIBOOT_MAGIC 0x2BADB002

multiboot_info_t* mb_info = 0;
static uint32_t mb_magic = 0;

void multiboot_init(uint32_t magic, multiboot_info_t* info) {
    mb_magic = magic;
    mb_info = info;
}

int multiboot_check(void) {
    return mb_magic == MULTIBOOT_MAGIC;
}
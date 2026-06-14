#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include <libc/stdint.h>

void kernel_main(uint32_t magic, void* mb_info_ptr);
void kernel_init(void);

#endif /* KERNEL_KERNEL_H */
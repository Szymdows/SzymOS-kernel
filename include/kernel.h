#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define SZYMOS_VERSION "0.1.2"
#define SZYMOS_CODENAME "Initium"
#define SZYMOS_YEAR "2026"

// Kernel panic
void kernel_panic(const char* message);

// Utility functions
void* memset(void* dest, int val, size_t len);
void* memcpy(void* dest, const void* src, size_t len);
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);

#endif
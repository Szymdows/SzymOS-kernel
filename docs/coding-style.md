# Coding Style Guide

This document describes coding conventions for **SzymOS Kernel**.

Following these rules ensures **readability, maintainability, and easy collaboration**.

---

## File Organization

```
boot/        ; Assembly bootloader
kernel/      ; Core kernel code
cpu/         ; CPU setup (GDT)
drivers/     ; Hardware drivers
include/     ; Header files
iso/         ; GRUB config
docs/        ; Documentation
```

---

## C Coding Style

- Function & variable names: **lower_snake_case**

```
void term_init();
int term_row;
```

- Constants & macros: **UPPERCASE**

```
#define VGA_COLS 80
#define KERNEL_VERSION "0.2.0"
```

- Header guards in `.h` files:

```
#ifndef VGA_H
#define VGA_H
...
#endif
```

- Comments:
  - `//` single-line
  - `/* ... */` multi-line for complex blocks

- Indentation: **4 spaces**
- Max line length: 120 characters
- Always document low-level operations

---

## Assembly Style

- Labels: lowercase with underscores (`_start`, `stack_top`)
- Constants: uppercase (`MAGIC`, `FLAGS`)
- Sections:

```
.section .multiboot
.section .text
.section .bss
```

- Keep code aligned for readability
- Comment every non-obvious instruction

---

## General Rules

- Do not commit compiled binaries (`*.o`, `*.bin`, `*.iso`) — use `.gitignore`
- Maintain consistent indentation and spacing
- Comment all new modules or driver code

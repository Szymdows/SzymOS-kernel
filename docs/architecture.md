# Kernel Architecture

SzymOS Kernel is a **monolithic 32-bit operating system kernel** written from scratch in C and x86 assembly.

All core subsystems run in kernel space and are initialized during early boot.

---

## Project Layout

```
boot/        ; Assembly bootloader code
kernel/      ; Core kernel code
cpu/         ; CPU setup (GDT)
drivers/     ; Hardware drivers (VGA, future drivers)
include/     ; Header files
iso/         ; GRUB configuration
docs/        ; Documentation
```

---

## Core Components

### Boot Layer
- Located in `boot/boot.s`
- Handles Multiboot header, stack setup, and jumping to kernel entry (`kmain`)
- Prepares CPU for protected mode

### Kernel Core
- Located in `kernel/kernel.c`
- Responsibilities:
  - Initialize terminal output (VGA)
  - Print boot information
  - Initialize CPU structures (GDT)
  - Transition kernel to ready state

### CPU Subsystem
- Located in `cpu/`
- Implements **Global Descriptor Table (GDT)**
- Handles memory segmentation and protected mode setup

### Drivers
- Currently only VGA text-mode terminal
- Responsible for:
  - Clearing screen
  - Printing characters and strings
  - Handling cursor positioning

---

## Design Principles

- Simplicity: small, understandable codebase
- Modularity: separate files for core, drivers, CPU
- Educational: easy for contributors to learn OSDev
- Expandability: designed for future modules and drivers

# SzymOS Kernel

A simple, educational operating system kernel created by SzymOS contributors.

## Features

- Boots via GRUB (Multiboot)
- VGA text mode output
- Basic kernel initialization
- Modular subsystem design

## Project Structure

- `arch/` - Architecture-specific code (x86)
- `boot/` - Boot loader code and configuration
- `drivers/` - Device drivers (VGA, etc.)
- `fs/` - File system code (TODO)
- `include/` - Header files organized by subsystem
- `kernel/` - Core kernel code
- `libc/` - Basic C library functions
- `memory/` - Memory management (TODO)

## License

Created by SzymOS contributors.

## Current Status

- [x] Boot with GRUB
- [x] VGA text mode driver
- [x] Basic kernel initialization
- [ ] Keyboard driver
- [ ] Memory management
- [ ] Multitasking
- [ ] File system support

## Docs

- Inside of the ```docs``` folder there will be a markdown file with instructions and how to get started. See ```BUILDING.md``` to see how to build SzymOS-kernel and what requirements you need to install
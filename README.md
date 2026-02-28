# SzymOS Kernel 🚀

**SzymOS Kernel** is a hobby operating system project: a **monolithic 32-bit x86 kernel** written from scratch in **C** and **x86 assembly**, focused on exploring low-level programming, hardware interaction, and operating system design.

The project aims to provide a clean and understandable learning environment for kernel development while gradually evolving into a fully featured experimental operating system.

---

## ✨ Features (v0.2.0)

* ✅ Multiboot-compliant kernel (GRUB bootable)
* ✅ Custom x86 assembly boot entry
* ✅ Stack initialization in protected mode
* ✅ Global Descriptor Table (GDT) setup
* ✅ VGA text-mode terminal driver
* ✅ Kernel version/build information banner
* ✅ Modular source layout (CPU, drivers, kernel core)
* ✅ ISO image generation and QEMU boot support

---

## 🧠 Architecture Overview

SzymOS Kernel follows a **monolithic kernel design**, where core subsystems run in kernel space.

Boot flow:

```
BIOS → GRUB → Kernel Entry (_start) → kmain()
        ↓
   Initialize VGA
        ↓
   Load GDT (memory segmentation)
        ↓
   Kernel Ready State
```

### Components

| Component      | Description                                |
| -------------- | ------------------------------------------ |
| `boot/`        | Multiboot header and kernel entry assembly |
| `kernel/`      | Core kernel logic and initialization       |
| `cpu/`         | CPU architecture setup (GDT)               |
| `drivers/`     | Hardware drivers (VGA text output)         |
| `include/`     | Shared kernel headers                      |
| `linker.ld`    | Kernel memory layout                       |
| `build script` | Automated compile + ISO generation         |

---

## 🖥️ Current Boot Output

```
##################################################
#  SzymOS kernel 0.2.0 (i386-pc)
#  Built: <date> at <time>
##################################################

[ OK ] Video Driver Initialized
[....] Initializing GDT (Memory Segmentation)...
[ OK ] GDT Loaded Successfully

SzymOS kernel is ready. Awaiting input...
```

---

## ⚙️ Build Requirements

### Toolchain

* `x86_64-elf-gcc` (cross compiler) - which can be downloaded from https://github.com/lordmilko/i686-elf-tools/releases/tag/15.2.0 - you need to download 'x86_64-elf-tools-windows.zip' - these are the gnu binutils you need.
* NASM / GAS (assembly support)
* GRUB 2
* QEMU
* WSL (used for ISO creation on Windows)

---

## 🔨 Building the Kernel

On Windows, run:

```bash
build.bat
```

The build process:

1. Compiles kernel core
2. Compiles drivers
3. Compiles CPU architecture code
4. Assembles boot entry
5. Links kernel using `linker.ld`
6. Creates a bootable GRUB ISO
7. Launches QEMU automatically

---

## ▶️ Running Manually

```bash
qemu-system-x86_64 -cdrom SzymOS-kernel-0.2.0.iso
```

---

## 📁 Project Structure

```
SzymOS-Kernel/
├── boot/        # Kernel entry & multiboot header
├── kernel/      # Core kernel code
├── cpu/         # Architecture initialization (GDT)
├── drivers/     # Hardware drivers
├── include/     # Header files
├── iso/         # GRUB configuration
├── linker.ld    # Memory layout
├── build.bat    # Build automation
└── docs/        # SzymOS Kernel docs
```

---

## 🎯 Project Goals

* Learn operating system internals
* Implement low-level hardware interaction
* Develop clean kernel architecture
* Experiment with system design concepts
* Build a foundation for future OS features

Planned future work includes:

* Interrupt handling (IDT)
* Keyboard input driver
* Memory management
* Basic shell
* Filesystem experimentation

---

## 🤝 Contributing

Contributions are welcome!

You can help by:

* Opening issues
* Suggesting improvements
* Adding drivers or kernel subsystems
* Improving documentation

Please keep code readable and well-commented.

---

## 📜 License

This project is licensed under the **MIT License**.

You are free to use, modify, distribute, and learn from this project with proper attribution.

See the `LICENSE` file for details.

---

## ⭐ Acknowledgements

Inspired by the OS development community and educational resources focused on understanding how operating systems work from the ground up.

---

## 🚧 Project Status

**Early Development** — core kernel infrastructure is being actively developed.

Expect breaking changes as the architecture evolves.

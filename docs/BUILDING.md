# Getting Started

This guide will help you build and run SzymOS on your machine.

## Prerequisites

Before building SzymOS, make sure the following tools are installed:

* GCC with 32-bit support (`gcc-multilib`) or an `i686-elf` cross-compiler
* NASM assembler
* GNU Make
* GRUB utilities (`grub-mkrescue`)
* xorriso
* QEMU (recommended)

### Ubuntu / Debian

Install all required dependencies:

```
sudo apt install build-essential nasm gcc-multilib grub-pc-bin xorriso qemu-system-x86
```

---

## Cloning the Repository

The current development version of SzymOS is located on the `rewrite` branch.

Clone it using:

```
git clone -b rewrite https://github.com/Szymdows/SzymOS-kernel.git
```

Enter the project directory:

```
cd SzymOS-kernel
```

---

## Building SzymOS

Compile the kernel:

```
make
```

If the build succeeds, the kernel binary will be generated in:

```
build/szymos-kernel.bin
```

---

## Creating a Bootable ISO

Generate a bootable ISO image:

```
make iso
```

This will create:

```
szymos.iso
```

---

## Running SzymOS

### Quick Method

Launch SzymOS directly in QEMU:

```
make run
```

### Manual Method

```
qemu-system-i386 -cdrom szymos.iso
```

If everything is configured correctly, a QEMU window should appear and boot SzymOS.

---

## Cleaning Build Files

To remove generated build files:

```
make clean
```

---

## Troubleshooting

### GCC Does Not Support `-m32`

If you encounter errors related to 32-bit compilation:

```
sudo apt install gcc-multilib
```

Alternatively, use an `i686-elf` cross-compiler.

### `grub-mkrescue` Not Found

Install the required GRUB packages:

```
sudo apt install grub-pc-bin xorriso
```

### QEMU Command Not Found

Install QEMU:

```
sudo apt install qemu-system-x86
```

---

## Next Steps

Once SzymOS is running, you can begin exploring the source code, modifying kernel functionality, adding hardware support, and experimenting with operating system development.

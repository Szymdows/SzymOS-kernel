# Development Environment Setup

This document explains **exactly** how to set up a working development environment for building and running the **SzymOS Kernel** on Windows.

Follow the steps in order.

---

# Overview

The kernel is developed on **Windows** using:

* Windows for compiling
* WSL (Linux environment) for GRUB tools
* QEMU for emulation

---

# 1. Install Required Software

## 1.1 Install Git

Download:

https://git-scm.com/download/win

After installation, verify:

```bash
git --version
```

---

## 1.2 Install WSL (Windows Subsystem for Linux)

Open **PowerShell as Administrator** and run:

```powershell
wsl --install
```

Restart your computer when prompted.

After reboot, open WSL and create a Linux username/password.

Verify:

```bash
wsl --status
```

---

## 1.3 Install Ubuntu Packages (Inside WSL)

Open **WSL terminal** and run:

```bash
sudo apt update
sudo apt upgrade -y
```

Install required tools:

```bash
sudo apt install grub-pc-bin grub-common xorriso mtools build-essential -y
```

### What these packages do

| Package         | Purpose                  |
| --------------- | ------------------------ |
| grub-pc-bin     | GRUB bootloader tools    |
| grub-common     | provides `grub-mkrescue` |
| xorriso         | creates ISO images       |
| mtools          | filesystem utilities     |
| build-essential | basic compilation tools  |

Verify GRUB installation:

```bash
grub-mkrescue --version
```

You should see a GRUB version printed.

---

## 1.4 Install QEMU

Download:

https://www.qemu.org/download/#windows

Install using default options.

Verify:

```bash
qemu-system-x86_64 --version
```

---

## 1.5 Install Cross Compiler (GCC + GNU Binutils)

Download:

https://github.com/lordmilko/i686-elf-tools/releases/tag/15.2.0

Download file:

```
x86_64-elf-tools-windows.zip
```

This package already includes:

* GCC cross compiler
* GNU binutils
* linker (`ld`)
* assembler
* objcopy / objdump

---

### Extract Toolchain

Extract to:

```
C:\cross\
```

You should now have:

```
C:\cross\bin\x86_64-elf-gcc.exe
```

---

### Add Toolchain to PATH

1. Open Windows Search → type **Environment Variables**
2. Open **Edit the system environment variables**
3. Click **Environment Variables**
4. Select **Path**
5. Click **New**
6. Add:

```
C:\cross\bin
```

Restart terminal.

Verify:

```bash
x86_64-elf-gcc --version
x86_64-elf-ld --version
```

---

# 2. Clone the Repository

```bash
git clone https://github.com/<your-username>/SzymOS-Kernel.git
cd SzymOS-Kernel
```

---

# 3. Project Directory Layout

```
boot/        Kernel entry assembly
kernel/      Core kernel code
cpu/         CPU setup (GDT)
drivers/     Hardware drivers
include/     Header files
iso/         GRUB configuration
docs/        Documentation
```

---

# 4. Building the Kernel

From Windows Command Prompt:

```bash
build.bat
```

The build script performs:

1. Compile kernel sources
2. Compile drivers
3. Compile CPU subsystem
4. Assemble boot code
5. Link kernel binary
6. Create GRUB bootable ISO (via WSL)
7. Start QEMU

---

# 5. ISO Creation (What Happens Internally)

WSL commands executed automatically:

```bash
wsl rm -rf isodir
wsl mkdir -p isodir/boot/grub
wsl cp kernel.bin isodir/boot/szymoskernel.bin
wsl cp iso/boot/grub/grub.cfg isodir/boot/grub/grub.cfg
wsl grub-mkrescue -o SzymOS-kernel-0.2.0.iso isodir
```

---

# 6. Running the Kernel

Automatic launch:

```bash
qemu-system-x86_64 -cdrom SzymOS-kernel-0.2.0.iso
```

Manual run:

```bash
qemu-system-x86_64 -cdrom SzymOS-kernel-0.2.0.iso
```

---

# 7. Expected Output

You should see:

```
SzymOS kernel 0.2.0 boot banner
Video driver initialized
GDT loaded successfully
Kernel ready
```

inside the QEMU window.

---

# 8. Troubleshooting

## Command not found (x86_64-elf-gcc)

PATH not configured correctly.

Restart terminal after editing environment variables.

---

## grub-mkrescue not found

Run inside WSL:

```bash
sudo apt install grub-pc-bin grub-common -y
```

---

## ISO not created

Ensure WSL is installed and working:

```bash
wsl --status
```

---

## QEMU does not start

Verify installation:

```bash
qemu-system-x86_64 --version
```

---

# 9. Development Workflow

Typical workflow:

```bash
edit code → run build.bat → QEMU launches → test kernel
```

---

# 10. Supported Platform

Current target:

```
Architecture: i386 (32-bit x86)
Bootloader: GRUB (Multiboot)
Emulator: QEMU
Host OS: Windows + WSL
```

---

# ✅ Setup Complete

You now have a fully working operating system development environment.

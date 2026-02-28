@echo off
title Building SzymOS kernel 0.2.0

echo [1/6] Compiling Kernel Core...
x86_64-elf-gcc -c kernel/kernel.c -o kernel/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32 -Iinclude

echo [2/6] Compiling Drivers...
x86_64-elf-gcc -c drivers/vga.c -o drivers/vga.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32 -Iinclude

echo [3/6] Compiling GDT (CPU Arch)...
x86_64-elf-gcc -c cpu/gdt.c -o cpu/gdt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32 -Iinclude

echo [4/6] Assembling Bootloader...
x86_64-elf-gcc -c boot/boot.s -o boot/boot.o -m32

echo [5/6] Linking Kernel...
x86_64-elf-ld -T linker.ld -o kernel.bin -m elf_i386 boot/boot.o kernel/kernel.o drivers/vga.o cpu/gdt.o

if not exist kernel.bin (
    echo Error: Linking failed!
    pause
    exit /b
)

echo [6/6] Creating ISO (WSL)...
wsl rm -rf isodir
wsl mkdir -p isodir/boot/grub

REM --- THIS IS THE CHANGE ---
REM We copy our 'kernel.bin' to 'szymoskernel.bin' inside the ISO folder
wsl cp kernel.bin isodir/boot/szymoskernel.bin
REM --------------------------

wsl cp iso/boot/grub/grub.cfg isodir/boot/grub/grub.cfg
wsl grub-mkrescue -o SzymOS-kernel-0.2.0.iso isodir

echo.
echo Build Complete!
echo Running QEMU...
qemu-system-x86_64 -cdrom SzymOS-kernel-0.2.0.iso
AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -nostdlib -lgcc
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL = build/szym-kern.bin
ISO = build/szymos.iso

ASM_SRC = boot/boot.asm
C_SRC = kernel/kernel.c kernel/terminal.c kernel/keyboard.c kernel/shell.c kernel/cpu.c kernel/memory.c kernel/filesystem.c kernel/ata.c kernel/disk_selector.c kernel/io.c kernel/serial.c

ASM_OBJ = $(ASM_SRC:.asm=.o)
C_OBJ = $(C_SRC:.c=.o)

all: $(ISO)

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(KERNEL): $(ASM_OBJ) $(C_OBJ)
	mkdir -p build
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(KERNEL)
	mkdir -p build/iso/boot/grub
	cp $(KERNEL) build/iso/boot/szym-kern.bin
	cp grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) build/iso

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -rf build boot/*.o kernel/*.o

.PHONY: all run clean

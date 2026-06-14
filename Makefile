# SzymOS Kernel Makefile
# (c) SzymOS contributors

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -f elf32

# Directories
BUILD_DIR = build
ISO_DIR = isodir

# Source files
ASM_SOURCES = $(wildcard boot/*.asm) $(wildcard arch/x86/*.asm)
C_SOURCES = $(wildcard kernel/*.c) $(wildcard drivers/*.c) $(wildcard libc/*.c) \
            $(wildcard memory/*.c) $(wildcard arch/x86/*.c)

# Object files
ASM_OBJECTS = $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
C_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

KERNEL_BIN = $(BUILD_DIR)/szymos-kernel.bin

all: $(KERNEL_BIN)

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL_BIN): $(ASM_OBJECTS) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/szymos-kernel.bin
	@cp boot/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o szymos.iso $(ISO_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) *.iso

run: iso
	qemu-system-i386 -cdrom szymos.iso

.PHONY: all clean iso run
#include "filesystem.h"
#include "ata.h"
#include "terminal.h"
#include "../include/kernel.h"
#include <stdint.h>
#include "io.h"

// In-memory filesystem (will be loaded from/saved to disk)
static filesystem_header_t fs_header;
static uint8_t fs_data[MAX_FILES * MAX_FILE_SIZE];
static int fs_initialized = 0;
static int fs_persistent = 0;
static uint16_t fs_drive_io_base = 0;
static uint8_t fs_drive_num = 0;
static int fs_drive_selected = 0;

// Helper to calculate how many sectors we need
#define HEADER_SECTORS ((sizeof(filesystem_header_t) + SECTOR_SIZE - 1) / SECTOR_SIZE)
#define DATA_SECTORS ((sizeof(fs_data) + SECTOR_SIZE - 1) / SECTOR_SIZE)

static int fs_read_sector_direct(uint32_t lba, uint8_t* buffer) {
    if (!fs_drive_selected) return -1;
    
    uint16_t io = fs_drive_io_base;
    
    // Wait for busy
    int timeout = 100000;
    while ((inb(io + 7) & 0x80) && timeout--);
    if (timeout <= 0) return -1;
    
    // Select drive and send command
    outb(io + 6, 0xE0 | (fs_drive_num << 4) | ((lba >> 24) & 0x0F));
    outb(io + 2, 1);
    outb(io + 3, (uint8_t)lba);
    outb(io + 4, (uint8_t)(lba >> 8));
    outb(io + 5, (uint8_t)(lba >> 16));
    outb(io + 7, 0x20);  // READ command
    
    // Wait for DRQ
    timeout = 100000;
    while (!(inb(io + 7) & 0x08) && timeout--);
    if (timeout <= 0) return -1;
    
    // Read data
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(io);
    }
    
    return 0;
}

static int fs_write_sector_direct(uint32_t lba, const uint8_t* buffer) {
    if (!fs_drive_selected) return -1;
    
    uint16_t io = fs_drive_io_base;
    
    // Wait for busy
    int timeout = 100000;
    while ((inb(io + 7) & 0x80) && timeout--);
    if (timeout <= 0) {
        terminal_writestring("[FS ERROR] Timeout waiting for ready\n");
        return -1;
    }
    
    // Select drive and send command
    outb(io + 6, 0xE0 | (fs_drive_num << 4) | ((lba >> 24) & 0x0F));
    outb(io + 2, 1);
    outb(io + 3, (uint8_t)lba);
    outb(io + 4, (uint8_t)(lba >> 8));
    outb(io + 5, (uint8_t)(lba >> 16));
    outb(io + 7, 0x30);  // WRITE command
    
    // Wait for DRQ
    timeout = 100000;
    while (!(inb(io + 7) & 0x08) && timeout--);
    if (timeout <= 0) {
        terminal_writestring("[FS ERROR] Timeout waiting for DRQ\n");
        return -1;
    }
    
    // Write data
    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(io, buf16[i]);
    }
    
    // Wait for completion
    timeout = 100000;
    while ((inb(io + 7) & 0x80) && timeout--);
    
    return 0;
}

static int disk_available(void) {
    return ata_is_selected();  // Changed from ata_get_drive_count() > 0
}

void fs_load_from_disk(void) {
    if (!disk_available()) {
        return;
    }
    
    uint8_t sector_buffer[SECTOR_SIZE];
    
    // Read header sectors
    uint8_t* header_ptr = (uint8_t*)&fs_header;
    for (uint32_t i = 0; i < HEADER_SECTORS; i++) {
        if (fs_read_sector_direct(FS_START_SECTOR + i, sector_buffer) == 0) {
            
            memcpy(header_ptr + (i * SECTOR_SIZE), sector_buffer, SECTOR_SIZE);
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[ERROR] Failed to read filesystem header\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
    }
    
    // Check magic number
    if (fs_header.magic != SZYMFS_MAGIC) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("[INFO] No existing filesystem found, creating new one\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        // Initialize new filesystem
        fs_header.magic = SZYMFS_MAGIC;
        fs_header.file_count = 0;
        for (int i = 0; i < MAX_FILES; i++) {
            fs_header.files[i].used = 0;
        }
        memset(fs_data, 0, sizeof(fs_data));
        
        // Try to save it to disk
        fs_persistent = 1;  // Set this BEFORE saving
        fs_save_to_disk();
        
        // Check if save actually worked
        if (!fs_persistent) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[WARN] Failed to initialize filesystem on disk\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        return;
    }
    
    // Read data sectors
    terminal_printf("[DEBUG] Reading %d data sectors...\n", DATA_SECTORS);

    for (uint32_t i = 0; i < DATA_SECTORS; i++) {
        if (fs_read_sector_direct(FS_START_SECTOR + HEADER_SECTORS + i, sector_buffer) == 0) {
            memcpy(fs_data + (i * SECTOR_SIZE), sector_buffer, SECTOR_SIZE);
            
        } else {
            terminal_printf("[DEBUG] Failed to read data sector %d\n", i);
        }
    }

    terminal_printf("[DEBUG] Data load complete\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_printf("[OK] Loaded %d files from disk\n", fs_header.file_count);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    fs_persistent = 1;
}

void fs_save_to_disk(void) {
    if (!disk_available()) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Warning: No disk selected, changes are RAM-only\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        fs_persistent = 0;
        return;
    }
    
    uint8_t sector_buffer[SECTOR_SIZE];
    int write_failed = 0;
    
    // Write header sectors
    uint8_t* header_ptr = (uint8_t*)&fs_header;
    for (uint32_t i = 0; i < HEADER_SECTORS; i++) {
        memset(sector_buffer, 0, SECTOR_SIZE);
        memcpy(sector_buffer, header_ptr + (i * SECTOR_SIZE), SECTOR_SIZE);
        
        if (fs_write_sector_direct(FS_START_SECTOR + i, sector_buffer) != 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[ERROR] Failed to write filesystem header\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            write_failed = 1;
            fs_persistent = 0;
            return;
        }
    }
    
    for (uint32_t i = 0; i < DATA_SECTORS; i++) {
        memset(sector_buffer, 0, SECTOR_SIZE);
        uint32_t copy_size = SECTOR_SIZE;
        if ((i + 1) * SECTOR_SIZE > sizeof(fs_data)) {
            copy_size = sizeof(fs_data) - (i * SECTOR_SIZE);
        }
        memcpy(sector_buffer, fs_data + (i * SECTOR_SIZE), copy_size);
        
        if (fs_write_sector_direct(FS_START_SECTOR + HEADER_SECTORS + i, sector_buffer) != 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[ERROR] Failed to write filesystem data\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            write_failed = 1;
            fs_persistent = 0;
            return;
        }
    }
    
    // Only show success if we got here
    if (!write_failed && fs_persistent) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Filesystem saved to disk!\n\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

void fs_init(void) {
    // Initialize empty filesystem first
    fs_header.magic = SZYMFS_MAGIC;
    fs_header.file_count = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        fs_header.files[i].used = 0;
        fs_header.files[i].name[0] = '\0';
        fs_header.files[i].size = 0;
        fs_header.files[i].offset = 0;
    }
    
    memset(fs_data, 0, sizeof(fs_data));
    fs_initialized = 1;
    
    if (disk_available()) {
        // Save which drive was selected
        fs_load_from_disk();
    }
    
    if (!disk_available()) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("[OK] SzymFS initialized (RAM-only mode)\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("[OK] SzymFS initialized (persistent mode)\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

int fs_find_free_slot(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fs_header.files[i].used) {
            return i;
        }
    }
    return -1;
}

int fs_find_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_header.files[i].used && strcmp(fs_header.files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int fs_create_file(const char* name, const char* content) {
    if (!fs_initialized) return -1;
    
    // Check if file already exists
    if (fs_find_file(name) >= 0) {
        return -2; // File exists
    }
    
    int slot = fs_find_free_slot();
    if (slot < 0) {
        return -3; // No free slots
    }
    
    uint32_t content_len = strlen(content);
    if (content_len > MAX_FILE_SIZE) {
        return -4; // File too large
    }
    
    // Copy filename
    size_t name_len = strlen(name);
    if (name_len >= MAX_FILENAME) {
        name_len = MAX_FILENAME - 1;
    }
    memcpy(fs_header.files[slot].name, name, name_len);
    fs_header.files[slot].name[name_len] = '\0';
    
    // Set file properties
    fs_header.files[slot].size = content_len;
    fs_header.files[slot].offset = slot * MAX_FILE_SIZE;
    fs_header.files[slot].used = 1;
    
    // Copy content
    memcpy(fs_data + fs_header.files[slot].offset, content, content_len);
    
    fs_header.file_count++;
    
    // Auto-save to disk
    fs_save_to_disk();
    
    return 0;
}

int fs_read_file(const char* name, char* buffer, uint32_t max_size) {
    if (!fs_initialized) return -1;
    
    int slot = fs_find_file(name);
    if (slot < 0) {
        return -1; // File not found
    }
    
    uint32_t size = fs_header.files[slot].size;
    if (size > max_size - 1) {
        size = max_size - 1;
    }
    
    memcpy(buffer, fs_data + fs_header.files[slot].offset, size);
    buffer[size] = '\0';
    
    return size;
}

int fs_delete_file(const char* name) {
    if (!fs_initialized) return -1;
    
    int slot = fs_find_file(name);
    if (slot < 0) {
        return -1; // File not found
    }
    
    fs_header.files[slot].used = 0;
    fs_header.files[slot].name[0] = '\0';
    fs_header.file_count--;
    
    // Auto-save to disk
    fs_save_to_disk();
    
    return 0;
}

void fs_list_files(void) {
    if (!fs_initialized) {
        terminal_writestring("Filesystem not initialized!\n");
        return;
    }
    
    if (fs_header.file_count == 0) {
        terminal_writestring("\nNo files found.\n\n");
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n=== SzymFS File List ===\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_printf("Total files: %d / %d\n\n", fs_header.file_count, MAX_FILES);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("NAME                              SIZE\n");
    terminal_writestring("------------------------------------\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_header.files[i].used) {
            // Print filename
            terminal_writestring(fs_header.files[i].name);
            
            // Pad with spaces
            int name_len = strlen(fs_header.files[i].name);
            for (int j = name_len; j < 32; j++) {
                terminal_putchar(' ');
            }
            
            // Print size
            terminal_printf("  %d bytes\n", fs_header.files[i].size);
        }
    }
    terminal_writestring("\n");
}

int fs_file_exists(const char* name) {
    return fs_find_file(name) >= 0;
}

void fs_set_drive_info(uint16_t io_base, uint8_t drive_num) {
    fs_drive_io_base = io_base;
    fs_drive_num = drive_num;
    fs_drive_selected = 1;
    
    terminal_printf("[DEBUG] fs_set_drive_info: io_base=0x%x, drive=%d\n", 
                    io_base, drive_num);
}

#include "ata.h"
#include "io.h"
#include "terminal.h"
#include "../include/kernel.h"

static ata_device_t drives[MAX_DRIVES];
static int drive_count = -999;     // Non-zero initialization
static int selected_drive = -999;  // Non-zero initialization

static int ata_wait_bsy(uint16_t io_base) {
    int timeout = 100000;
    while ((inb(io_base + ATA_REG_STATUS) & ATA_SR_BSY) && timeout > 0) {
        timeout--;
    }
    return timeout > 0 ? 0 : -1;  // Return -1 on timeout
}

static int ata_wait_drq(uint16_t io_base) {
    int timeout = 100000;
    while (!(inb(io_base + ATA_REG_STATUS) & ATA_SR_DRQ) && timeout > 0) {
        timeout--;
    }
    return timeout > 0 ? 0 : -1;  // Return -1 on timeout
}

static void ata_400ns_delay(uint16_t io_base) {
    for (int i = 0; i < 4; i++) {
        inb(io_base + ATA_REG_STATUS);
    }
}

static int ata_identify(uint16_t io_base, uint8_t drive, ata_device_t* device) {
    device->io_base = io_base;
    device->drive = drive;
    device->present = 0;
    
    // Select drive
    outb(io_base + ATA_REG_DRIVE, 0xA0 | (drive << 4));
    ata_400ns_delay(io_base);
    
    // Send IDENTIFY command
    outb(io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_400ns_delay(io_base);
    
    uint8_t status = inb(io_base + ATA_REG_STATUS);
    if (status == 0) {
        return 0; // No drive
    }
    
    // Wait for BSY to clear
    int timeout = 10000;
    while (timeout--) {
        status = inb(io_base + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            return 0; // Error
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            break;
        }
    }
    
    if (timeout <= 0) {
        return 0;
    }
    
    // Read identification data
    uint16_t identify[256];
    for (int i = 0; i < 256; i++) {
        identify[i] = inw(io_base + ATA_REG_DATA);
    }
    
    // Extract model string (words 27-46)
    for (int i = 0; i < 40; i += 2) {
        device->model[i] = identify[27 + i/2] >> 8;
        device->model[i + 1] = identify[27 + i/2] & 0xFF;
    }
    device->model[40] = '\0';
    
    // Trim trailing spaces
    for (int i = 39; i >= 0; i--) {
        if (device->model[i] == ' ') {
            device->model[i] = '\0';
        } else {
            break;
        }
    }
    
    // Get sector count: prefer LBA48 (words 100-103) if supported,
    // fall back to LBA28 (words 60-61).
    // Words must be cast to uint32_t/uint64_t before shifting to avoid
    // undefined behaviour on 16-bit values shifted by >= 16 bits.
    if (identify[83] & (1 << 10)) {
        // LBA48 supported — use 48-bit sector count (words 100-103)
        // We cap at 32-bit (2 TB) since size_mb is uint32_t
        uint32_t sectors_lo = ((uint32_t)identify[101] << 16) | identify[100];
        uint32_t sectors_hi = ((uint32_t)identify[103] << 16) | identify[102];
        device->sectors = sectors_hi ? 0xFFFFFFFFUL : sectors_lo;
    } else {
        // LBA28 — 28-bit sector count in words 60-61
        device->sectors = ((uint32_t)identify[61] << 16) | identify[60];
    }
    device->size_mb = device->sectors / 2048; // 512-byte sectors -> MB

    // Check removable media bit: word 0, bit 7 (per ATA spec).
    // Do NOT use size as a removability heuristic — VirtualBox virtual
    // disks can be any size and are fixed, not removable.
    device->is_removable = (identify[0] & 0x80) ? 1 : 0;
    
    device->present = 1;
    return 1;
}

void ata_init(void) {
    // Initialize to known values
    for (int i = 0; i < MAX_DRIVES; i++) {
        drives[i].present = 0;
        drives[i].io_base = 0;
    }
    
    drive_count = 0;
    selected_drive = -1;
}

int ata_detect_drives(void) {
    drive_count = 0;
    
    // Check Primary Master
    if (ata_identify(ATA_PRIMARY_IO, 0, &drives[drive_count])) {
        drive_count++;
    }
    
    // Check Primary Slave
    if (ata_identify(ATA_PRIMARY_IO, 1, &drives[drive_count])) {
        drive_count++;
    }
    
    // Check Secondary Master
    if (ata_identify(ATA_SECONDARY_IO, 0, &drives[drive_count])) {
        drive_count++;
    }
    
    // Check Secondary Slave
    if (ata_identify(ATA_SECONDARY_IO, 1, &drives[drive_count])) {
        drive_count++;
    }
    
    return drive_count;
}

void ata_print_drives(void) {
    if (drive_count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("\nNo ATA drives detected.\n");
        terminal_writestring("Filesystem will run in RAM-only mode.\n\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n=== Detected Storage Devices ===\n\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    for (int i = 0; i < drive_count; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_printf("[%d] ", i);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        // Print drive location
        if (drives[i].io_base == ATA_PRIMARY_IO) {
            terminal_writestring("Primary ");
        } else {
            terminal_writestring("Secondary ");
        }
        
        if (drives[i].drive == 0) {
            terminal_writestring("Master");
        } else {
            terminal_writestring("Slave");
        }
        
        terminal_writestring("\n    Model: ");
        terminal_writestring(drives[i].model);
        
        terminal_printf("\n    Size: %d MB", drives[i].size_mb);
        
        if (drives[i].is_removable) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring(" [REMOVABLE]");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring(" [FIXED DISK - CAUTION!]");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
        terminal_writestring("\n\n");
    }
}

static int ata_select_device(int drive_index) {
    if (drive_index < 0 || drive_index >= drive_count) {
        return -1;
    }
    
    ata_device_t* dev = &drives[drive_index];
    uint16_t io_base = dev->io_base;
    
    // Select the drive
    outb(io_base + ATA_REG_DRIVE, 0xA0 | (dev->drive << 4));
    ata_400ns_delay(io_base);
    
    // Wait for it to be ready
    if (ata_wait_bsy(io_base) < 0) {
        return -1;
    }
    
    uint8_t status = inb(io_base + ATA_REG_STATUS);
    if (!(status & ATA_SR_DRDY)) {
        return -1;  // Drive not ready
    }
    
    return 0;
}

int ata_select_drive(int drive_num) {
    if (drive_num < 0 || drive_num >= drive_count) {
        return -1;
    }
    
    if (!drives[drive_num].present) {
        return -1;
    }
    
    // Try to select the device
    if (ata_select_device(drive_num) < 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("ERROR: Failed to select drive!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        return -1;
    }
    
    selected_drive = drive_num;

    terminal_printf("[DEBUG] selected_drive set to %d (drive_count=%d)\n", 
                    selected_drive, drive_count);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_printf("Selected drive %d: %s\n", drive_num, drives[drive_num].model);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    return 0;
}

int ata_is_selected(void) {
    return selected_drive >= 0;
}

int ata_get_drive_count(void) {
    return drive_count;
}

ata_device_t* ata_get_drive(int index) {
    if (index < 0 || index >= drive_count) {
        return 0;
    }
    return &drives[index];
}

int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    if (selected_drive < 0 || selected_drive >= drive_count) {
        terminal_writestring("[DEBUG] Invalid selected_drive index\n");
        return -1;
    }
    
    ata_device_t* dev = &drives[selected_drive];
    
    // Validate the drive data
    if (!dev->present || dev->io_base == 0) {
        terminal_printf("[DEBUG] Drive %d corrupted: present=%d, io_base=0x%x\n",
                       selected_drive, dev->present, dev->io_base);
        return -1;
    }
    
    uint16_t io_base = dev->io_base;
    
    terminal_printf("[DEBUG] Writing to drive %d, LBA %d, IO base 0x%x\n", 
                    selected_drive, lba, io_base);
    
    if (ata_wait_bsy(io_base) < 0) {
        terminal_writestring("[ATA ERROR] Timeout waiting for BSY\n");
        return -1;
    }
    
    outb(io_base + ATA_REG_DRIVE, 0xE0 | (dev->drive << 4) | ((lba >> 24) & 0x0F));
    ata_400ns_delay(io_base);
    
    outb(io_base + ATA_REG_SECCOUNT, 1);
    outb(io_base + ATA_REG_LBA_LO, (uint8_t)lba);
    outb(io_base + ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(io_base + ATA_REG_LBA_HI, (uint8_t)(lba >> 16));
    
    outb(io_base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    
    if (ata_wait_drq(io_base) < 0) {
        terminal_writestring("[ATA ERROR] Timeout waiting for DRQ\n");
        return -1;
    }
    
    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(io_base + ATA_REG_DATA, buf16[i]);
    }
    
    if (ata_wait_bsy(io_base) < 0) {
        terminal_writestring("[ATA ERROR] Timeout after write\n");
        return -1;
    }
    
    return 0;
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (selected_drive < 0 || selected_drive >= drive_count) {
        terminal_writestring("[ATA ERROR] Read - invalid drive index\n");
        return -1;
    }
    
    ata_device_t* dev = &drives[selected_drive];
    
    // Validate the drive data
    if (!dev->present || dev->io_base == 0) {
        terminal_writestring("[ATA ERROR] Read - drive data corrupted\n");
        return -1;
    }
    
    uint16_t io_base = dev->io_base;
    
    if (ata_wait_bsy(io_base) < 0) {
        terminal_writestring("[ATA ERROR] Read timeout waiting for BSY\n");
        return -1;
    }
    
    outb(io_base + ATA_REG_DRIVE, 0xE0 | (dev->drive << 4) | ((lba >> 24) & 0x0F));
    ata_400ns_delay(io_base);
    
    outb(io_base + ATA_REG_SECCOUNT, 1);
    outb(io_base + ATA_REG_LBA_LO, (uint8_t)lba);
    outb(io_base + ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(io_base + ATA_REG_LBA_HI, (uint8_t)(lba >> 16));
    
    outb(io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    
    if (ata_wait_drq(io_base) < 0) {
        terminal_writestring("[ATA ERROR] Read timeout waiting for DRQ\n");
        return -1;
    }
    
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(io_base + ATA_REG_DATA);
    }
    
    return 0;
}

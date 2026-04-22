#include "disk_selector.h"
#include "ata.h"
#include "keyboard.h"
#include "terminal.h"
#include "../include/kernel.h"
#include <stdint.h>
#include "filesystem.h"
#include "io.h"

void show_disks_run(void) {
    int drive_count = ata_get_drive_count();
    
    if (drive_count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("\nNo storage devices detected.\n");
        terminal_writestring("Running in RAM-only mode.\n");
        terminal_writestring("Files will NOT persist after reboot!\n\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("Press any key to continue...\n");
        keyboard_getchar();
        return;
    }
    
    // Show detected drives
    ata_print_drives();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=== Disk Selection ===\n\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    terminal_writestring("Select a drive for persistent storage:\n");
    terminal_printf("Enter drive number (0-%d), or 'n' for RAM-only: ", drive_count - 1);
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == 'n' || c == 'N') {
            terminal_writestring("n\n\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("RAM-only mode selected.\n");
            terminal_writestring("Files will NOT persist after reboot!\n\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        if (c >= '0' && c <= '9') {
            int selection = c - '0';
            
            if (selection >= 0 && selection < drive_count) {
                terminal_putchar(c);
                terminal_writestring("\n\n");
                
                ata_device_t* drive = ata_get_drive(selection);
                
                // Extra warning for fixed disks
                if (!drive->is_removable) {
                    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                    terminal_writestring("WARNING: This is a FIXED DISK!\n");
                    terminal_writestring("Writing to this disk could damage your system!\n");
                    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                    terminal_writestring("\nAre you SURE? (y/n): ");
                    
                    char confirm = keyboard_getchar();
                    terminal_putchar(confirm);
                    terminal_writestring("\n\n");
                    
                    if (confirm != 'y' && confirm != 'Y') {
                        terminal_writestring("Selection cancelled.\n");
                        disk_selector_run(); // Ask again
                        return;
                    }
                }
                
                // Select the drive
                if (ata_select_drive(selection) == 0) {
                    ata_device_t* drive = ata_get_drive(selection);
    
                    // Save drive info in filesystem module
                    fs_set_drive_info(drive->io_base, drive->drive);

                    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                    terminal_writestring("Drive selected successfully!\n");
                    terminal_writestring("Filesystem will be persistent.\n\n");
                    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                    terminal_writestring("Testing disk access...\n");

                    

                    terminal_writestring("\n");
                } else {
                    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                    terminal_writestring("Error selecting drive!\n\n");
                    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                }
                return;
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_writestring("\nInvalid selection!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                terminal_printf("Enter drive number (0-%d), or 'n' for RAM-only: ", drive_count - 1);
            }
        }
    }
}
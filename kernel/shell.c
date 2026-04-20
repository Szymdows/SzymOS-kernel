#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "../include/kernel.h"
#include "filesystem.h"
#include "ata.h"
#include "serial.h"
#include "io.h"
#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_index = 0;

// String starts with prefix
static int starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str++ != *prefix++) return 0;
    }
    return 1;
}

void shell_print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("szym");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("@");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("kernel");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring(":$ ");
}

void cmd_banner(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("   ____                       ___  ____\n");
    terminal_writestring("  / ___| _____ _ _ __ ___    / _ \\/ ___|\n");
    terminal_writestring("  \\___ \\|_  / | | '_ ` _ \\  | | | \\___ \\\n");
    terminal_writestring("   ___) |/ /| |_| | | | | | | |_| |___) |\n");
    terminal_writestring("  |____//___|\\__, |_| |_| |_|\\___/|____/\n");
    terminal_writestring("             |___/\n\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("\n         The Operating System Built From Scratch!\n\n");
}

void cmd_colors(void) {
    terminal_writestring("\n=== VGA Color Palette ===\n\n");
    
    const char* color_names[] = {
        "Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Brown", "Light Gray",
        "Dark Gray", "Light Blue", "Light Green", "Light Cyan", 
        "Light Red", "Light Magenta", "Yellow", "White"
    };
    
    for (int i = 0; i < 16; i++) {
        terminal_setcolor(vga_entry_color(i, VGA_COLOR_BLACK));
        terminal_printf("  ██  ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_printf(" %d: %s\n", i, color_names[i]);
    }
    terminal_writestring("\n");
}
void cmd_serial_print(const char* msg) {
    serial_init();
    serial_print(msg);
}
void cmd_serial_read() {
    char buffer[128];
    serial_init();
    serial_read(buffer,128);
    terminal_writestring(buffer);
}
void cmd_calc(const char* expr) {
    // Simple calculator: "calc 5 + 3"
    int num1 = 0, num2 = 0;
    char op = 0;
    int result = 0;
    
    // Parse the expression
    const char* p = expr;
    
    // Get first number
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') {
        num1 = num1 * 10 + (*p - '0');
        p++;
    }
    
    // Get operator
    while (*p == ' ') p++;
    op = *p;
    p++;
    
    // Get second number
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') {
        num2 = num2 * 10 + (*p - '0');
        p++;
    }
    
    // Calculate
    switch (op) {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '*': result = num1 * num2; break;
        case '/': 
            if (num2 == 0) {
                terminal_writestring("\nError: Division by zero!\n\n");
                return;
            }
            result = num1 / num2; 
            break;
        default:
            terminal_writestring("\nUsage: calc <num1> <op> <num2>\n");
            terminal_writestring("Operators: + - * /\n");
            terminal_writestring("Example: calc 15 + 27\n\n");
            return;
    }
    
    terminal_printf("\n%d %c %d = %d\n\n", num1, op, num2, result);
}

void cmd_ls(void) {
    fs_list_files();
}

void cmd_cat(const char* filename) {
    char buffer[MAX_FILE_SIZE + 1];
    
    int result = fs_read_file(filename, buffer, MAX_FILE_SIZE + 1);
    
    if (result < 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("\nFile not found: %s\n\n", filename);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else {
        terminal_writestring("\n");
        terminal_writestring(buffer);
        terminal_writestring("\n\n");
    }
}

void cmd_write(const char* args) {
    // Parse: write filename content here
    char filename[MAX_FILENAME];
    const char* content_start = args;
    int i = 0;
    
    // Skip leading spaces
    while (*content_start == ' ') content_start++;
    
    // Extract filename (until space)
    while (*content_start && *content_start != ' ' && i < MAX_FILENAME - 1) {
        filename[i++] = *content_start++;
    }
    filename[i] = '\0';
    
    if (filename[0] == '\0') {
        terminal_writestring("\nUsage: write <filename> <content>\n");
        terminal_writestring("Example: write hello.txt Hello World!\n\n");
        return;
    }
    
    // Skip spaces before content
    while (*content_start == ' ') content_start++;
    
    if (*content_start == '\0') {
        terminal_writestring("\nError: No content provided\n\n");
        return;
    }
    
    int result = fs_create_file(filename, content_start);
    
    if (result == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_printf("\nFile created: %s\n\n", filename);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (result == -2) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("\nError: File already exists: %s\n\n", filename);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else if (result == -3) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("\nError: Filesystem full\n\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("\nError: Could not create file\n\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

void cmd_rm(const char* filename) {
    int result = fs_delete_file(filename);
    
    if (result == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_printf("\nFile deleted: %s\n\n", filename);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("\nFile not found: %s\n\n", filename);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

void shell_execute_command(const char* cmd) {
    if (strcmp(cmd, "") == 0) {
        return;
    }
    else if (strcmp(cmd, "help") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("\nSzymOS Available Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("\nSystem Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("  help     - Show this help message\n");
        terminal_writestring("  clear    - Clear the screen\n");
        terminal_writestring("  info     - Display system information\n");
        terminal_writestring("  version  - Show kernel version\n");
        terminal_writestring("  cpuid    - Display CPU information\n");
        terminal_writestring("  mem      - Show memory information\n");
        terminal_writestring("  reboot   - Reboot the system\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("\nFile System Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("  ls       - List all files\n");
        terminal_writestring("  cat      - Display file contents (cat filename)\n");
        terminal_writestring("  write    - Create a file (write name content)\n");
        terminal_writestring("  rm       - Delete a file (rm filename)\n");
	terminal_writestring("  disks    - List ATA drives\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("\nFun Commands:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("  banner   - Display SzymOS banner\n");
        terminal_writestring("  colors   - Show all VGA colors\n");
        terminal_writestring("  calc     - Simple calculator (e.g., calc 5 + 3)\n");
        terminal_writestring("  echo     - Echo text back\n");
        terminal_writestring("  panic    - Trigger a kernel panic (test)\n");
        terminal_writestring("\n");
    }
    else if (strcmp(cmd, "ls") == 0) {
        cmd_ls();
    }
    else if (starts_with(cmd, "cat ")) {
        cmd_cat(cmd + 4);
    }
    else if (starts_with(cmd, "write ")) {
        cmd_write(cmd + 6);
    }
    else if (starts_with(cmd, "rm ")) {
        cmd_rm(cmd + 3);
    }
    else if (starts_with(cmd, "serial_print ")) {
        cmd_serial_print(cmd + 13);
    }
    else if (strcmp(cmd, "serial_read") == 0) {
        cmd_serial_read();
    } 
    else if (strcmp(cmd, "clear") == 0) {
        terminal_clear();
    }
    else if (strcmp(cmd, "info") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("\n=== SzymOS System Information ===\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_printf("Kernel Version: %s\n", SZYMOS_VERSION);
        terminal_printf("Codename: %s\n", SZYMOS_CODENAME);
        terminal_writestring("Architecture: x86 (32-bit)\n");
        terminal_writestring("Bootloader: GRUB Multiboot\n");
        terminal_writestring("Terminal: VGA Text Mode (80x25)\n");
        terminal_printf("Total Memory: %d KB\n", get_total_memory());
        terminal_writestring("\n");
    }
    else if (strcmp(cmd, "version") == 0) {
        terminal_printf("\nSzymOS Kernel v%s (%s)\n", SZYMOS_VERSION, SZYMOS_CODENAME);
        terminal_writestring("A custom operating system kernel\n");
        terminal_printf("Copyright (c) %s\n\n", SZYMOS_YEAR);
    }
    else if (strcmp(cmd, "save") == 0) {
        fs_save_to_disk();
    }
    else if (strcmp(cmd, "cpuid") == 0) {
        cpu_print_info();
    }
    else if (strcmp(cmd, "mem") == 0) {
        memory_print_info();
    }
    else if (strcmp(cmd, "banner") == 0) {
        cmd_banner();
    }
    else if (strcmp(cmd, "colors") == 0) {
        cmd_colors();
    }
    else if (strcmp(cmd, "disks") == 0) {
        ata_print_drives();
    }
    else if (starts_with(cmd, "calc ")) {
        cmd_calc(cmd + 5);
    }
    else if (strcmp(cmd, "reboot") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("\nRebooting system...\n");
        
        // Use keyboard controller to reboot
        uint8_t temp;
        asm volatile("cli");
        do {
            temp = inb(0x64);
            if (temp & 0x01)
                inb(0x60);
        } while (temp & 0x02);
        
        outb(0x64, 0xFE);
        
        // If that didn't work, halt
        asm volatile("hlt");
    }
    else if (starts_with(cmd, "echo ")) {
        terminal_writestring("\n");
        terminal_writestring(cmd + 5);
        terminal_writestring("\n\n");
    }
    else if (strcmp(cmd, "panic") == 0) {
        kernel_panic("User triggered panic from shell");
    }
    else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("\nCommand not found: %s\n", cmd);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_writestring("Type 'help' for available commands.\n\n");
    }
}

void shell_init(void) {
    command_index = 0;
    command_buffer[0] = '\0';
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nWelcome to SzymOS Shell!\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Type 'help' for available commands.\n\n");
}

void shell_run(void) {
    shell_print_prompt();
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == 0) {
            continue;
        }
        
        if (c == '\n') {
            terminal_putchar('\n');
            command_buffer[command_index] = '\0';
            shell_execute_command(command_buffer);
            command_index = 0;
            shell_print_prompt();
        }
        else if (c == '\b') {
            if (command_index > 0) {
                command_index--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        }
        else if (c == '\t') {
            continue;
        }
        else if (command_index < MAX_COMMAND_LENGTH - 1) {
            command_buffer[command_index++] = c;
            terminal_putchar(c);
        }
    }
}

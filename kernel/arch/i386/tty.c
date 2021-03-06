#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/cursor.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_buffer = VGA_MEMORY;

    terminal_clear();
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;

            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }

    // Reset cursor
    terminal_setcursor(0);
}

// Scrolls everything up one line
void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;

            if (index + 80 <= VGA_WIDTH * VGA_HEIGHT) {
                terminal_buffer[index] = terminal_buffer[index + 80];
            } else {
                terminal_buffer[index] = vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        }
    }

    terminal_row--;
}

void terminal_setcolor(uint8_t color) {
    terminal_color = vga_entry_color(color, VGA_COLOR_BLACK);
}

void terminal_setcursor(uint16_t pos) {
    // Save old color
    const uint8_t old_color = terminal_color;

    // Reset color
    terminal_setcolor(VGA_COLOR_WHITE);

    fb_move_cursor(pos);

    // Restore color
    terminal_setcolor(old_color);
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    unsigned char uc = c;

    if (uc == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    if (terminal_row == VGA_HEIGHT) {
        terminal_scroll();
    }

    // Set cursor to character after
    terminal_setcursor(terminal_row * VGA_WIDTH + terminal_column);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

#ifndef TTY_H
#define TTY_H

#include "klib.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#define VGA_CONTROLER_SELECT 0x3D4
#define VGA_CONTROLER_SET 0x3D5
#define VGA_CURSOR_LOW 0x0F
#define VGA_CURSOR_HIGH 0x0E

/* Hardware text mode color constants. */
enum vga_color
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

typedef struct color_map_s
{
	const char *name;
	uint8_t vga;
} color_map_t;
typedef struct terminal_window
{
	uint32_t terminal_row;
	uint32_t terminal_column;
	uint8_t terminal_color;
	uint16_t terminal_buffer[VGA_WIDTH * VGA_HEIGHT];
} terminal_window_t;

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, uint32_t x, uint32_t y);
void indent_terminal_rows();
void terminal_putnewline();
void terminal_putchar(char c);
int terminal_write(const char *data, uint32_t size);
int terminal_writestring(const char *data);
void terminal_move_cursor_right();
void terminal_move_cursor_left();
void terminal_move_cursor_down();
void terminal_move_cursor_up();
void switch_terminal_window();

#endif
#include <stdarg.h>
#include "stdlib.h"
#include "tty.h"
#include "scancodes.h"

uint8 text_attr;
uint16 tty_io_port;

void init_tty() {
    key_buffer_head = 0;
    key_buffer_tail = 0; 
	tty_buffer = (void*)0xB8000;
	tty_width = *((uint16*)0x44A);
	tty_height = 25;
	tty_io_port = *((uint16*)0x463);
	cursor = (*((uint8*)0x451)) * tty_width + (*((uint8*)0x450));
	text_attr = 7;
}

void out_char(char chr) {
	switch (chr) {
		case '\n':
            cur_start = (cursor / tty_width + 1) * tty_width;
			move_cursor(cur_start);
			break;
		default:
			tty_buffer[cursor].chr = chr;
			tty_buffer[cursor].attr = text_attr;
			move_cursor(cursor + 1);
	}
}

void out_string(char *str) {
	while (*str) {
		out_char(*str);
		str++;
	}
    cur_start = cursor;
}

void clear_screen() {
	memset_word(tty_buffer, (text_attr << 8) + ' ', tty_width * tty_height);
	move_cursor(0);
}

void set_text_attr(char attr) {
	text_attr = attr;
}

void move_cursor(unsigned int pos) {
	cursor = pos;
	if (cursor >= tty_width * tty_height) {
		cursor = (tty_height - 1) * tty_width;
		memcpy(tty_buffer, tty_buffer + tty_width, tty_width * tty_height * sizeof(TtyChar));
		memset_word(tty_buffer + tty_width * (tty_height - 1), (text_attr << 8) + ' ', tty_width);
	}
	outportb(tty_io_port, 0x0E);
	outportb(tty_io_port + 1, cursor >> 8);
	outportb(tty_io_port, 0x0F);
	outportb(tty_io_port + 1, cursor & 0xFF);
}

const char digits[] = "0123456789ABCDEF";
char num_buffer[65];

char *int_to_str(size_t value, unsigned char base) {
	size_t i = sizeof(num_buffer) - 1;
	num_buffer[i--] = '\0';
	do {
		num_buffer[i--] = digits[value % base];
		value = value / base;
	} while (value);
	return &num_buffer[i + 1];
}

void printf(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			size_t arg = va_arg(args, size_t);
			switch (*fmt) {
				case '%':
					out_char('%');
					break;
				case 'c':
					out_char(arg);
					break;
				case 's':
					out_string((char*)arg);
					break;
				case 'b':
					out_string(int_to_str(arg, 2));
					break;
				case 'o':
					out_string(int_to_str(arg, 8));
					break;
				case 'd':
					out_string(int_to_str(arg, 10));
					break;
				case 'x':
					out_string(int_to_str(arg, 16));
					break;
			}
		} else {
			out_char(*fmt);
		}
		fmt++;
	}
	va_end(args);
}

uint8 in_scancode() {
	uint8 result;
	if (key_buffer_head != key_buffer_tail) {
		if (key_buffer_head >= KEY_BUFFER_SIZE) {
			key_buffer_head = 0;
		}
		result = key_buffer[key_buffer_head];
		key_buffer_head++;
	} else {
		result = 0;
	}
	return result;
}

char in_char(bool wait) {
	static bool shift = false;
	uint8 chr;
	do {
		chr = in_scancode();
		switch (chr) {
			case 0x2A:
			case 0x36:
				shift = true;
				break;
			case 0x2A + 0x80:
			case 0x36 + 0x80:
				shift = false;
				break;
		}
		if (chr & 0x80) {
			chr = 0;
		}
		if (shift) {
			chr = scancodes_shifted[chr];
		} else {
			chr = scancodes[chr];
		}
	} while (wait && (!chr));
	return chr;
}

void in_string(char *buffer, size_t buffer_size) {
	char chr;
	size_t position = 0;
	do {
		chr = in_char(true);
		switch (chr) {
			case 0:
				break;
			case 8:
				if (position > 0) {
					position--;
					out_char(8);
				}
				break;
			case '\n':
				out_char('\n');
				break;
			default:
				if (position < buffer_size - 1) {
					buffer[position] = chr;
					position++;
					out_char(chr);
				}
		}
	} while (chr != '\n');
	buffer[position] = 0;
}
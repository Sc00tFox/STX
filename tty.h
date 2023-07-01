#ifndef TTY_H
#define TTY_H

#pragma once

#include "stdlib.h"

#define KEY_BUFFER_SIZE 16
char key_buffer[KEY_BUFFER_SIZE];
unsigned int key_buffer_head;
unsigned int key_buffer_tail; 

typedef struct {
	uint8 chr;
	uint8 attr;
} TtyChar;

unsigned int tty_width;
unsigned int tty_height;

unsigned int cur_start;
unsigned int cursor;

TtyChar *tty_buffer;

void init_tty();
void out_char(char chr);
void out_string(char *str);
void clear_screen();
void set_text_attr(char attr);
void move_cursor(unsigned int pos);
void printf(char *fmt, ...);
char in_char(bool wait);
void in_string(char *buffer, size_t buffer_size);

#endif
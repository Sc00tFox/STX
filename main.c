// #include "keymap.h"
#include "tty.h"
#include "stdlib.h"

// #define _COLUMNS 80
// #define _LINES 25
// #define _ITEM_BYTE 2
// #define _SCREENSIZE _COLUMNS * _LINES * _ITEM_BYTE

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E

// extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

// unsigned int cursor_max = 0;
// unsigned int cursor_loc = 0;

// char *videobuffer = (char*)0xb8000;

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);

	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	write_port(0x21 , 0xFD);
}

// void print(const char *str)
// {
// 	unsigned int i = 0;
// 	while (str[i] != '\0') {
// 		videobuffer[cursor_loc++] = str[i++];
// 		videobuffer[cursor_loc++] = 0x07;
// 	}
// }

// void print_newline(void)
// {
// 	unsigned int line_size = _ITEM_BYTE * _COLUMNS;
// 	cursor_loc = cursor_loc + (line_size - cursor_loc % (line_size));
//     cursor_max = cursor_loc;
// }

// void clear_screen(void)
// {
// 	unsigned int i = 0;
// 	while (i < _SCREENSIZE) {
// 		videobuffer[i++] = ' ';
// 		videobuffer[i++] = 0x07;
// 	}
// }

void keyboard_handler_main(void)
{
	unsigned char status;
	char key_code;

	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	if (status & 0x01) {
		key_code = read_port(KEYBOARD_DATA_PORT);

        if (key_code < 0) {
            return;
        }

        if (key_buffer_tail >= KEY_BUFFER_SIZE) {
		    key_buffer_tail = 0;
	    }
	    key_buffer_tail++;
	    key_buffer[key_buffer_tail - 1] = key_code;

        if (key_code == 0x0E) // Backspace
        {
            if (cursor > cur_start)
            {
                tty_buffer[cursor-1].attr = 0;
                tty_buffer[cursor-1].chr = ' ';
                move_cursor(cursor-1);
                tty_buffer[cursor].attr = 15;
                return;
            }
            else
            {
                return;
            }
            
        }
        
        if (key_code == 0x0F) // Tab
        {
            for (int i = 0; i < 4; i++)
            {
                tty_buffer[cursor+1].attr = 0;
                tty_buffer[cursor+1].chr = ' ';
                move_cursor(cursor+1);
            }
            return;
        }

        tty_buffer[cursor].chr = key_buffer[key_code];
        move_cursor(cursor);

		// if (keycode < 0)
		// 	return;

		// if (keycode == ENTER_KEY_CODE) {
		// 	printf("\n");
		// 	return;
		// }

        // if (keycode == BACKSPACE_KEY_CODE) {
        //     if (cursor_loc > cursor_max) {
        //         videobuffer[cursor_loc--] = keyboard_map[(unsigned char) keycode];
        //         videobuffer[cursor_loc--] = 0x00;
        //         cursor_loc -= 1;
        //         return;
        //     }
        // }

		// videobuffer[cursor_loc++] = keyboard_map[(unsigned char) keycode];
		// videobuffer[cursor_loc++] = 0x07;
	}
}

void kernelmain(void) {
    init_tty();
    clear_screen();
    set_text_attr(15);
	printf("STX initialized\n\n");

	idt_init();
	kb_init();

	while (true) {
		char buffer[256];
		out_string("> ");
		in_string(buffer, sizeof(buffer));
		// printf("You typed: %s\n", buffer);
        if (!cstrncmp(buffer, "test")) {
            printf("Out: test\n");
        }
        else if (!cstrncmp(buffer, "clear")) {
            clear_screen();
        }
	}
}
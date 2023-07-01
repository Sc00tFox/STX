bits 32

segment .text
    ALIGN 4
    DD 0x1BADB002
    DD 0x00
    DD - (0x1BADB002 + 0x00)

global _start
global read_port
global write_port
global load_idt
global keyboard_handler

extern kernelmain
extern keyboard_handler_main

read_port:
    MOV EDX, [ESP + 4]
    IN AL, DX
    RET

write_port:
    MOV EDX, [ESP + 4]
    MOV AL, [ESP + 4 + 4]
    OUT DX, AL
    RET

load_idt:
    MOV EDX, [ESP + 4]
    LIDT [EDX]
    STI
    RET

keyboard_handler:
    CALL keyboard_handler_main
    IRETD

_start:
    CLI
    MOV ESP, stack_space
    CALL kernelmain
    HLT

segment .bss

RESB 8192
stack_space:
all: startup.o stdlib.o tty.o main.o
	i386-elf-ld -m elf_i386 -T link.ld -o kernel start.o main.o stdlib.o tty.o
startup.o:
	nasm -f elf32 start.asm -o start.o
stdlib.o:	
	i386-elf-gcc -m32 -c stdlib.c -o stdlib.o
main.o:	
	i386-elf-gcc -m32 -c main.c -o main.o
tty.o:
	i386-elf-gcc -m32 -c tty.c -o tty.o
clean:
	rm start.o main.o stdlib.o tty.o
	qemu-system-i386 -kernel kernel

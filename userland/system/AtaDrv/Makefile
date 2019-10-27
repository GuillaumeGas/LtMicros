OBJ=AtaDrv.sys
INC_SYSDIR=../Common
INC_STDDIR=../../StdLib/src
CC=g++ -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie -I$(INC_SYSDIR) -I$(INC_STDDIR)
LD=ld -Ttext=40000000 -m elf_i386 --entry=main
ASM=nasm -f elf32

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

AtaDrv.sys: main.o ata.o stdio.o stdlib.o syscalls.o malloc.o
	$(LD) $^ -o bin/$@

main.o: src/main.cpp
	$(CC) -c $^

ata.o: src/ata.cpp
	$(CC) -c $^

stdio.o: ../../StdLib/src/stdio.cpp
	$(CC) -c $^

stdlib.o: ../../StdLib/src/stdlib.cpp
	$(CC) -c $^

syscalls.o: ../../StdLib/src/syscalls.asm
	$(ASM) -o $@ $^

malloc.o: ../../StdLib/src/malloc.cpp
	$(CC) -c $^
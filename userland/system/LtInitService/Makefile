OBJ=LtInitService.sys
INC_SYSDIR=../Common
INC_STDDIR=../../StdLib/src
INC_KERNELDIR=../../../
CC=g++ -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie -I$(INC_SYSDIR) -I$(INC_STDDIR) -I$(INC_KERNELDIR)
LD=ld -Ttext=40000000 -m elf_i386 --entry=main
ASM=nasm -f elf32

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

LtInitService.sys: main.o stdio.o stdlib.o logger.o syscalls.o malloc.o Ipc.o FileSystem.o LtFsCommon.o
	$(LD) $^ -o bin/$@

main.o: main.cpp
	$(CC) -c $^

stdio.o: ../../StdLib/src/stdio.cpp
	$(CC) -c $^

stdlib.o: ../../StdLib/src/stdlib.cpp
	$(CC) -c $^

logger.o: ../../StdLib/src/logger.cpp
	$(CC) -c $^

syscalls.o: ../../StdLib/src/syscalls.asm
	$(ASM) -o $@ $^

malloc.o: ../../StdLib/src/malloc.cpp
	$(CC) -c $^

Ipc.o: ../../StdLib/src/Ipc.cpp
	$(CC) -c $^

FileSystem.o: ../../StdLib/src/FileSystem.cpp
	$(CC) -c $^

LtFsCommon.o: ../Common/LtFsCommon.cpp
	$(CC) -c $^
OBJ=LtFsService.sys
INC_SYSDIR=../Common
INC_STDDIR=../../StdLib/src
INC_KERNELDIR=../../../
CC=g++ -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie -I$(INC_SYSDIR) -I$(INC_STDDIR) -I$(INC_KERNELDIR)
LD=ld -Ttext=40000000 -m elf_i386 --entry=main
ASM=nasm -f elf32

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

LtFsService.sys: main.o AtaDriver.o ServiceCommands.o LtFsCommon.o Ext2.o File.o FsManager.o stdio.o stdlib.o logger.o syscalls.o malloc.o Ipc.o status.o list.o
	$(LD) $^ -o bin/$@

main.o: src/main.cpp
	$(CC) -c $^

AtaDriver.o: src/AtaDriver.cpp
	$(CC) -c $^

ServiceCommands.o: src/ServiceCommands.cpp
	$(CC) -c $^

LtFsCommon.o: ../Common/LtFsCommon.cpp
	$(CC) -c $^

Ext2.o: src/Ext2.cpp
	$(CC) -c $^

File.o: src/File.cpp
	$(CC) -c $^

FsManager.o: src/FsManager.cpp
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

status.o: ../../StdLib/src/status.cpp
	$(CC) -c $^

list.o: ../../StdLib/src/list.cpp
	$(CC) -c $^
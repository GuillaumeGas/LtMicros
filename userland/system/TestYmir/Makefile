OBJ=YmirTest.sys
INC_SYSDIR=../Common
INC_STDDIR=../../StdLib/src
INC_KERNELDIR=../../../
CC=g++ -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie -I$(INC_STDDIR) -I$(INC_SYSDIR) -I$(INC_KERNELDIR)
YC=gyc -m32 -nostdinc -fno-stack-protector -fno-pie
LD=ld -Ttext=40000000 -m elf_i386 --entry=_yrt_run_main
ASM=nasm -f elf32

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

YmirTest.sys: main.o stdio.o syscalls.o status.o
	$(LD) $^ -o bin/$@

main.o: main.yr
	$(YC) -c $^

stdio.o: ../../StdLib/src/stdio.cpp
	$(CC) -c $^

syscalls.o: ../../StdLib/src/syscalls.asm
	$(ASM) -o $@ $^
	
status.o: ../../StdLib/src/status.cpp
	$(CC) -c $^
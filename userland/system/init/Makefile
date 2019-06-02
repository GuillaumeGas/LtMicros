OBJ=init.out
CC=g++ -m32 -ffreestanding -nostdlib -Wall -fno-stack-protector -fno-pie
LD=ld -Ttext=40000000 -m elf_i386 --oformat binary --entry=main

all: $(OBJ)

clean:
	rm -f $(OBJ) *.o

init.out: main.o
	$(LD) $^ -o $@

main.o: main.cpp
	$(CC) -c $^
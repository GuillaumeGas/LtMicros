OBJ=bootsector.o

all: $(OBJ) 

bootsector.o: bootsector.asm
	nasm -f elf32 -o $@ $^

clean:
	rm -f bootsector.o

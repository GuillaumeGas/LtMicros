OBJ=ltMicros

all: $(OBJ) 

ltMicros: bootsect kern
	rm -f userland/*.lock
	cp kernel/ltkernel iso/boot/ltkernel.img
	cp userland/system/LtFsService/bin/LtFsService.sys iso/boot/LtFsService.sys
	cp userland/system/LtInitService/bin/LtInitService.sys iso/boot/LtInitService.sys
	grub-mkrescue -o ltkernel.iso iso

bootsect: 
	make -C boot

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltkernel.img iso/boot/LtFsService.sys iso/boot/LtInitService.sys *.o ltkernel.iso
	make -C boot clean
	make -C kernel clean

doc:
	make -C kernel doc
OBJ=ltMicrosImg

all: $(OBJ) 

ltMicrosImg: bootsect kern
	cp kernel/kernel iso/boot/ltmicros.img
	cp userland/system/LtFsService/bin/LtFsService.sys iso/boot/LtFsService.sys
	cp userland/system/LtInitService/bin/LtInitService.sys iso/boot/LtInitService.sys
	grub-mkrescue -o ltmicros.iso iso

bootsect: 
	make -C boot

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltmicros.img iso/boot/LtFsService.sys iso/boot/LtInitService.sys *.o ltmicros.iso
	make -C boot clean
	make -C kernel clean

doc:
	make -C kernel doc
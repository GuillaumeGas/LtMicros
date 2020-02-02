OBJ=ltMicros

all: $(OBJ) 

ltMicros: bootsect kern user
	rm -f userland/*.lock
	cp kernel/ltkernel iso/boot/ltkernel.img
	cp userland/system/LtFsService/bin/LtFsService.sys iso/boot/LtFsService.sys
	cp userland/system/LtInitService/bin/LtInitService.sys iso/boot/LtInitService.sys
	grub-mkrescue -o ltkernel.iso iso

bootsect: 
	make -C boot

user:
	make -C userland/system/LtFsService
	make -C userland/system/LtInitService

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltkernel.img iso/boot/LtFsService.sys iso/boot/LtInitService.sys *.o ltkernel.iso
	make -C boot clean
	make -C userland/system/LtFsService clean
	make -C userland/system/LtInitService clean
	make -C kernel clean

doc:
	make -C kernel doc
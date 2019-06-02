OBJ=ltMicrosImg

all: $(OBJ) 

ltMicrosImg: bootsect kern
	cp kernel/kernel iso/boot/ltmicros.img
	cp userland/system/AtaDrv/bin/AtaDrv.sys iso/boot/AtaDrv.sys
	cp userland/system/AtaDrv/bin/AtaDrv.sys iso/boot/AtaDrv2.sys
	grub-mkrescue -o ltmicros.iso iso

bootsect: 
	make -C boot

kern: 
	make -C kernel

clean:
	rm -f $(OBJ) kernel.bin iso/boot/ltmicros.img iso/boot/AtaDrv.sys *.o ltmicros.iso
	make -C boot clean
	make -C kernel clean

doc:
	make -C kernel doc
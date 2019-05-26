#include <kernel/drivers/proc_io.hpp>
#include <kernel/drivers/Screen.hpp>
#include <kernel/arch/x86/Gdt.hpp>
#include <kernel/lib/StdIo.hpp>

#define __MULTIBOOT__
#include "multiboot.hpp"

#include "Kernel.hpp"

/*
    @brief Kernel entry point, called from grub bootsector code (boot/bootsector.asm)
           Initializes the GDT (Global Descriptor Table) and then the kernel itself.
    @param[in] mid A pointer on a multiboot partial info used to retrieve RAM information.
    @param[in] multibootMagicNumber An unsigned integer used to check if the kernel has been loaded correctly by GRUB
*/  
extern "C" void kmain(MultibootPartialInfo * mbi, u32 multibootMagicNumber);

void kmain(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
    DISABLE_IRQ();

    ScreenDriver::Clear();
    kprint("Booting LtMicros...\n");

    gGdt.Init();

    // Initialize the kernel stack
	asm("movw $0x10, %ax \n \
         movw %ax, %ss \n \
         movl $0xA0000, %esp");

	gKernel.Init(mbi, multibootMagicNumber);
    gKernel.Start();

    kprint("How did we get there ?\n");
    while (1);
}
# LtMicros

A little micro-kernel written in C++

![](https://github.com/GuillaumeGas/LtMicros/blob/master/img/screenshot.png)

# Kernel physical memory organisation

         0x0 - 0x1000   GDT/IDT
      0x1000 - 0x2000   Kernel page directory
      0x2000 - 0xA0000  Kernel stack
     0xA0000 - 0x100000 Hardware area
    0x100000 - 0x400000 Kernel code (loaded at this address by GRUB)
    0x400000 - 0x800000 Kernel page table

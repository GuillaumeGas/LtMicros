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

# Virtual address space organisation
## General virtual address space

           0x0 - 0x40000000  Kernel address space (1Go)
    0x40000000 - 0x100000000 User address space (3Go)

## Kernel virtual address space

           0x0 - 0x800000    Identity mapping
      0x800000 - 0x1000000   Page Heap
     0x1000000 - 0x40000000  Heap
    0x40000000 - 0x100000000 User space

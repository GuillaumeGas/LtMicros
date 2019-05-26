#define __GDT__
#include "Gdt.hpp"

#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/StdIo.hpp>

/// @addgroup ArchX86Group
/// @{

/// Gdt base address in RAM
#define GDT_ADDR 0x0
/// Gdt nb entries
#define GDT_SIZE 6

/// Gdt kernel code segment selector
#define K_CODE_SEG_SELECTOR 0x08
/// Gdt tss segment selector
#define TSS_SEG_SELECTOR    0x28

/// @brief Function written in assembly used to tells the processor where the GDT is.
extern "C" void _gdtLoad(const GdtInfo * gdtAddr);
/// @brief Function written in assembly used to tells the processor which gdt selector describes tss
extern "C" void _tssLoad(const u16 tssSelector);

Gdt::Gdt()
{
    info.limit = 0;
    info.base = GDT_ADDR;
}

void Gdt::Init()
{
    MemSet(GDT_ADDR, 0, (u32)(GDT_SIZE * sizeof(GdtDescriptor)));
    MemSet(&gTss, 0, (u32)(sizeof(Tss)));

    descriptors = GDT_ADDR;

    TssInit();

    InitDescriptor(0, 0, 0, 0, &descriptors[EMPTY_SEG_INDEX]);
    InitDescriptor(0, 0xFFFFF, 0x9B, 0x0D, &descriptors[KERNEL_CODE_SEG_INDEX]);
    InitDescriptor(0, 0xFFFFF, 0x93, 0x0D, &descriptors[KERNEL_DATA_SEG_INDEX]);
    InitDescriptor(0, 0xFFFFF, 0xFF, 0x0F, &descriptors[USER_CODE_SEG_INDEX]);
    InitDescriptor(0, 0xFFFFF, 0xF3, 0x0F, &descriptors[USER_DATA_SEG_INDEX]);
    InitDescriptor((u32)&gTss, (u32)(sizeof(Tss)), 0xE9, 0, &descriptors[TSS_SEG_INDEX]);

    info.limit = GDT_SIZE * sizeof(GdtDescriptor);
    info.base = GDT_ADDR;

    _gdtLoad(&info);

    _tssLoad(TSS_SEG_SELECTOR);
}

void Gdt::InitDescriptor(u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc)
{
    desc->limit0_15 = (limit & 0xFFFF);
    desc->base0_15 = (base & 0xFFFF);
    desc->base16_23 = (base >> 16) & 0xFF;
    desc->access = access;
    desc->limit16_19 = (limit >> 16) & 0xF;
    desc->flags = flags & 0xF;
    desc->base24_31 = (base >> 24) & 0xFF;
}

void Gdt::TssInit()
{
    gTss.debug_flag = 0x00;
    gTss.io_map = 0x00;
    gTss.ss0 = KERNEL_DATA_SELECTOR;
}

const GdtDescriptor * Gdt::GetDescriptor(const GdtSelector selector) const
{
    u32 offset = (u32)selector;
    if (offset <= ((GDT_SIZE - 1) * sizeof(GdtDescriptor)))
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        return (GdtDescriptor *)offset;
#pragma GCC diagnostic pop
    }
    return (GdtDescriptor *)0;
}

u32 Gdt::GetBaseAddrFromDescriptor(const GdtDescriptor * desc) const
{
    u32 addr = desc->base0_15;
    addr |= (desc->base16_23 << 16);
    addr |= (desc->base24_31 << 24);
    return addr;
}

void Gdt::Print() const
{
    int i = 1;
    GdtDescriptor * gdt_desc = (GdtDescriptor *)0;

    kprint(">> GDT (base : %x, limit : %x)\n\n", GDT_ADDR, GDT_ADDR + (GDT_SIZE * sizeof(GdtDescriptor)));

    for (; i < GDT_SIZE; i++)
        PrintDescriptor(&gdt_desc[i]);
}

void Gdt::PrintDescriptor(GdtDescriptor * gdtDesc) const
{
    u32 base = 0x0;
    u32 limit = 0x0;

    base |= gdtDesc->base24_31 << 24;
    base |= gdtDesc->base16_23 << 16;
    base |= gdtDesc->base0_15;

    limit |= gdtDesc->limit16_19 << 16;
    limit |= gdtDesc->limit0_15;

    kprint(" [%x] base : %x, limit : %x\n", gdtDesc, base, (base + limit));
}

/// @}
#define __IDT__
#include "Idt.hpp"

#include <kernel/arch/x86/Gdt.hpp>

#include <kernel/lib/StdIo.hpp>

/// @addgroup ArchX86Group
/// @{

/// Represents the number of entries in the IDT
#define IDT_SIZE 255
/// IDT location in RAM
#define IDT_ADDR 0x800

extern "C" void _asm_default_isr(void);

extern "C" void _asm_divided_by_zero_isr(void);
extern "C" void _asm_non_maskable_int_isr(void);
extern "C" void _asm_overflow_isr(void);
extern "C" void _asm_bound_range_exceeded_isr(void);
extern "C" void _asm_invalid_opcode_isr(void);
extern "C" void _asm_device_not_available_isr(void);
extern "C" void _asm_double_fault_isr(void);
extern "C" void _asm_invalid_tss_isr(void);
extern "C" void _asm_segment_not_present_isr(void);
extern "C" void _asm_stack_segment_fault_isr(void);
extern "C" void _asm_general_protection_fault_isr(void);
extern "C" void _asm_page_fault_isr(void);
extern "C" void _asm_x87_floating_point_isr(void);
extern "C" void _asm_alignment_check_isr(void);
extern "C" void _asm_machine_check_isr(void);
extern "C" void _asm_simd_floating_point_isr(void);
extern "C" void _asm_virtualization_isr(void);
extern "C" void _asm_security_isr(void);

extern "C" void _idtLoad(IdtInfo * idt_ptr);

extern "C" void TmpKeyboardIsr()
{

}

Idt::Idt()
{
    info.limit = 0;
    info.base = 0;
}

void Idt::Init()
{
    int i = 0;

    info.limit = IDT_SIZE * sizeof(IdtDescriptor);
    info.base = IDT_ADDR;

    descriptors = (IdtDescriptor *)info.base;

    for (; i < IDT_SIZE; i++)
        InitDescriptor((u32)_asm_default_isr, CPU_GATE, i);

    // We initialize processor exceptions ISRs
    InitDescriptor((u32)_asm_divided_by_zero_isr,          CPU_GATE, 0);
    InitDescriptor((u32)_asm_non_maskable_int_isr,         CPU_GATE, 2);
    InitDescriptor((u32)_asm_overflow_isr,                 CPU_GATE, 4);
    InitDescriptor((u32)_asm_bound_range_exceeded_isr,     CPU_GATE, 5);
    InitDescriptor((u32)_asm_invalid_opcode_isr,           CPU_GATE, 6);
    InitDescriptor((u32)_asm_device_not_available_isr,     CPU_GATE, 7);
    InitDescriptor((u32)_asm_double_fault_isr,             CPU_GATE, 8);
    InitDescriptor((u32)_asm_invalid_tss_isr,              CPU_GATE, 10);
    InitDescriptor((u32)_asm_segment_not_present_isr,      CPU_GATE, 11);
    InitDescriptor((u32)_asm_stack_segment_fault_isr,      CPU_GATE, 12);
    InitDescriptor((u32)_asm_general_protection_fault_isr, CPU_GATE, 13);
    InitDescriptor((u32)_asm_page_fault_isr,               CPU_GATE, 14);
    InitDescriptor((u32)_asm_x87_floating_point_isr,       CPU_GATE, 16);
    InitDescriptor((u32)_asm_alignment_check_isr,          CPU_GATE, 17);
    InitDescriptor((u32)_asm_machine_check_isr,            CPU_GATE, 18);
    InitDescriptor((u32)_asm_simd_floating_point_isr,      CPU_GATE, 19);
    InitDescriptor((u32)_asm_virtualization_isr,           CPU_GATE, 20);
    InitDescriptor((u32)_asm_security_isr,                 CPU_GATE, 30);

    // tmp
    InitDescriptor((u32)TmpKeyboardIsr,                    CPU_GATE, 33);

    Reload();
}

void Idt::InitDescriptor(u32 isrAddr, u16 type, unsigned int index)
{

    _InitDescriptor(isrAddr, KERNEL_CODE_SELECTOR, type, &descriptors[index]);
}

void Idt::_InitDescriptor(u32 isrAddr, u16 selector, GATE_ATTR type, IdtDescriptor * desc)
{
    desc->offset0_15 = (isrAddr & 0xFFFF);
    desc->selector = selector;
    desc->type = type << 8;
    desc->offset16_31 = (isrAddr & 0xFFFF0000) >> 16;
}

void Idt::Reload()
{
    _idtLoad(&info);
}

/// @}
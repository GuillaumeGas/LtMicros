#pragma once

#include <kernel/lib/Types.hpp>

/// @file

/// @addgroup ArchX86Group
/// @{

/// @brief Not used in this kernel
#define TASK_GATE 0x5
/// @brief Used to handle hardware interrupt, deactivates other interrupts
#define INT_GATE 0xE
/// @brief Works like INT_GATE but without deactivates other interrupts
#define TRAP_GATE 0xF

#define USER_DPL (3 << 5)
#define KERNEL_DPL 0
#define PRESENT (1 << 7)

#define CPU_GATE     INT_GATE  | PRESENT | KERNEL_DPL
#define SYSCALL_GATE TRAP_GATE | PRESENT | USER_DPL

/// @brief Represents an IDT entry, called an Idt descriptor
struct IdtDescriptor
{
    /// @brief Bits 0 to 15 of the interrupt function's offset
    u16 offset0_15;
    /// @brief A code segment selector in GDT or LDT
    u16 selector;
    /// @brief Example : P D P L . 0 1 1 1 . 0 0 0 x . x x x x
    ///        P   : The segment is present in memory (1) or not (0)
    ///        DPL : Specified which privilege level the calling descriptor minimum should have
    ///        x   : unsed bits
    u16 type;
    /// @brief Bits 16 to 31 of the interrupt function's offset
    u16 offset16_31;
} __attribute__((packed));

/// @brief Stores info about IDT
struct IdtInfo
{
    /// @brief Limit address
    u16 limit;
    /// @brief IDT location in memory
    u32 base;
} __attribute__((packed));

/// @brief Interrupt Descriptor Table (IDT)
///        The 32 first entries are used for processor exceptions.
///        The 16 next entries are reserved for hardware.
///        The entry 0x30 is used for syscalls.
class Idt
{
public:
    /// @brief Default Idt class constructor, do nothing
    Idt();

    /// @brief Initializes the IDT, stores it at IDT_ADDR, and transmits its location to the processor
    void Init();

    /// @brief Initializes an idt descriptor
    /// @param[in] isrAddr The interrupt service routine address (ISR)
    /// @param[in] type The interrupt gate type
    /// @param[in] index The interrupt descriptor index in IDT
    void InitDescriptor(u32 isrAddr, u16 type, unsigned int index);

    /// @brief Tells the processor to load the IDT (using the lidt instruction)
    void Reload();

    IdtInfo info;
    IdtDescriptor * descriptors;

private:
    typedef u16 GATE_ATTR;

    /// @brief Initializes an idt descriptor
    /// @param[in] isrAddr The interrupt service routine address (ISR)
    /// @param[in] selector A valid code gdt descriptor selector
    /// @param[in] type The interrupt gate type
    /// @param[in] desc A pointer to an idt descriptor
    void _InitDescriptor(u32 isrAddr, u16 selector, GATE_ATTR type, IdtDescriptor * desc);
};

#ifdef __IDT__
Idt gIdt;
#else
extern Idt gIdt;
#endif

/// @]
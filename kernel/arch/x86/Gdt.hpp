#pragma once

#include <kernel/lib/Types.hpp>

/// @file

/// @defgroup ArchX86Group Arch x86 group
/// @{

/// @brief Gdt segment selectors
enum GdtSelector
{
    /// @brief Kernel code segment selector
    KERNEL_CODE_SELECTOR        = 0x08,
    /// @brief Kernel data segment selector
    KERNEL_DATA_SELECTOR        = 0x10,
    /// @brief User code segment selector
    USER_CODE_SELECTOR          = 0x18,
    /// @brief User data segment selector
    USER_DATA_SELECTOR          = 0x20,
    /// @brief User code segment selector with a RPL bits set to '11'
    USER_CODE_SELECTOR_WITH_RPL = 0x1B,
    /// @brief User data segment selector with a RPL bits set to '11'
    USER_DATA_SELECTOR_WITH_RPL = 0x23,
    /// @brief Tss segment selector
    TSS_SELECTOR                = 0x28
};

/// @brief Gdt segment index
enum GdtDescriptorIndex
{
    EMPTY_SEG_INDEX = 0,
    /// @brief Kernel code segment index in gdt
    KERNEL_CODE_SEG_INDEX,
    /// @brief Kernel data segment index in gdt
    KERNEL_DATA_SEG_INDEX,
    /// @brief User code segment index in gdt
    USER_CODE_SEG_INDEX,
    /// @brief User data segment index in gdt
    USER_DATA_SEG_INDEX,
    /// @brief Tss segment index in gdt
    TSS_SEG_INDEX
};

/// @brief Represents a Gdt entry, called a gdt descriptor
struct GdtDescriptor
{
    /// @brief Bits 0 to 15 of the limit address
    u16 limit0_15;
    /// @brief Bits 0 to 15 of the base address
    u16 base0_15;
    /// @brief Bits 16 to 23 of the base address
    u8 base16_23;
    /// @brief Access rights/type
    u8 access;
    /// @brief Bits 16 to 19 of the limit address
    u8 limit16_19 : 4;
    /// @brief Flag field (depends on the descriptor type)
    u8 flags : 4;
    /// @brief Bits 24 to 31 of the base address
    u8 base24_31;
} __attribute__((packed));

/// @brief Stores info about GDT
struct GdtInfo
{
    /// @brief Limit address (the GDT beeing at 0x0, it's equal to the GDT size)
    u16 limit;
    /// @brief Gdt location in memory
    u32 base;
} __attribute__((packed));

/// @brief Tss structure, used for task switching by the processor
struct Tss
{
    u16    previous_task, __previous_task_unused;
    u32    esp0;
    u16    ss0, __ss0_unused;
    u32    esp1;
    u16    ss1, __ss1_unused;
    u32    esp2;
    u16    ss2, __ss2_unused;
    u32    cr3;
    u32    eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u16    es, __es_unused;
    u16    cs, __cs_unused;
    u16    ss, __ss_unused;
    u16    ds, __ds_unused;
    u16    fs, __fs_unused;
    u16    gs, __gs_unused;
    u16    ldt_selector, __ldt_sel_unused;
    u16    debug_flag, io_map;
} __attribute__((packed));

/// @brief The GDT (Global Descriptor Table) is store at 0x0.
///        It is composed of 6 entries :
///          - Kernel code segment
///          - Kernel data segment
///          - User code segment
///          - User data segment
///          - TSS segment
class Gdt
{
public:
    /// @brief Default Gdt class constructor, do nothing
    Gdt();

    /// @brief Initializes the GDT, stores it at GDT_ADDR and transmits its location to the processor
    void Init();

    /// @brief Retrieve a gdt descriptor from a selector
    /// @param[in] selector The selector of the wanted gdt descriptor
    /// @return A pointer to a gdt descriptor
    const GdtDescriptor * GetDescriptor(const GdtSelector selector) const;

    /// @brief Retrieve the base address from a gdt descriptor
    /// @param[in] desc A pointer to a gdt descriptor
    /// @return The base address contained in this descriptor
    u32 GetBaseAddrFromDescriptor(const GdtDescriptor * desc) const;

    /// @brief Display gdt info & content, for debug purpose
    void Print() const;

    /// @brief Display a gdt descriptor, for debug purpose
    void PrintDescriptor(GdtDescriptor * gdtDesc) const;

    GdtInfo info;
    GdtDescriptor * descriptors;

private:
    /// @brief Initializes the Tss segment
    void TssInit();

    /// @brief Initializes a gdt descriptor
    /// @param[in] base The segment base address
    /// @param[in] limit The segment limit address
    /// @param[in] access (seg type : code, stack, data, DPL, ...)
    /// @param[in] flags
    /// @param[in,out] A pointer to the gdt descriptor we want to initialize
    void InitDescriptor(u32 base, u32 limit, u8 access, u8 flags, GdtDescriptor * desc);
};

#ifdef __GDT__
Gdt gGdt;
Tss gTss;
#else
extern Gdt gGdt;
extern Tss gTss;
#endif

/// @}

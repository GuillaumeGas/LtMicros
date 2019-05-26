#pragma once

/// @file

#include <kernel/lib/Types.hpp>

/// @addgroup ArchX86Group
/// @{

/*

    Kernel physical memory organisation :

         0x0 - 0x1000   GDT/IDT
      0x1000 - 0x2000   Kernel page directory
      0x2000 - 0xA0000  Kernel stack
     0xA0000 - 0x100000 Hardware area
    0x100000 - 0x400000 Kernel code (loaded at this address by GRUB)
    0x400000 - 0x800000 Kernel page table

*/

/// @brief The maximum number of pages we could have
///        (our kernel can't handle more than 4Go of memory : (0x100000 * PAGE_SIZE = 4Go) with PAGE_SIZE = 0x1000)
#define RAM_MAXPAGE 0x100000

/// @brief Calculates the number of bytes needed to represents all pages of memory
///        (A byte representing 8 pages)
#define MEM_BITMAP_SIZE RAM_MAXPAGE / 8

#define PAGE(addr) (addr) >> 12

/// @brief This class doesn't implement the Physical Memory Manager.
///        It just aims to be the base class of a Pmm implementation.
///        Then, it will be easy to implement other Pmm algorithms from it.
class PmmInterface
{
public:
    /// @brief Initializes the Phyisical Memory Manager
    virtual void Init() {}

    /// @brief Looks for a free page and returns its address
    /// @return A valid physical page address or nullptr if nothing was found
    virtual void * GetFreePage() {}

    /// @brief Set free a used physical page thanks to its address
    /// @param[in] The physical page address to be freed
    virtual void ReleasePage(void * addr) {}
};

/// @brief This class implements a Physical Memory Manager based on a bitmap to 
///        determine whether a page is free or not.
class PmmBitmap : public PmmInterface
{
public:
    /// @brief Initializes the Phyisical Memory Manager
    void Init() override;

    /// @brief Looks for a free page, using a bitmap, and returns its address
    /// @return A valid physical page address or nullptr if nothing was found
    void * GetFreePage() override;

    /// @brief Set free a used physical page thanks to its address
    /// @param[in] The physical page address to be freed
    void ReleasePage(void * addr) override;

private:
    u8 memBitmap[MEM_BITMAP_SIZE];
};

#ifdef __PMM__
PmmBitmap gPmm;
#else
extern PmmBitmap gPmm;
#endif

/// @}
#define __PMM__
#include "Pmm.hpp"

/// @addgroup ArchX86Group
/// @{

#include "MemCommon.hpp"

#include <kernel/lib/StdLib.hpp>
#include <kernel/multiboot.hpp>
#include <kernel/Kernel.hpp>

/// @brief Macro used to set a specific bit corresponding on the given page and set it to indicates that the page is used
#define SET_PAGE_USED(page)   memBitmap[((u32)page) / 8] |= (1 << (((u32)page) % 8))

/// @brief Macro used to set a specific bit corresponding on the given page and set it to indicates that the page is unused
#define SET_PAGE_UNUSED(addr) memBitmap[((u32)addr / PAGE_SIZE) / 8] &= ~(1 << (((u32)addr / PAGE_SIZE) % 8))

void PmmBitmap::Init()
{
    int page = 0;

    MemSet(memBitmap, 0, MEM_BITMAP_SIZE);

    // We calculate the last page in RAM
    int lastPage = (gMbi.high_mem * 1024) / PAGE_SIZE;

    // We set the non-existing pages as used
    for (page = lastPage / 8; page < RAM_MAXPAGE / 8; page++)
        memBitmap[page] = 0xFF;

    // And we set the kernel pages as used too
    for (page = PAGE(0); page < PAGE(gKernel.info.pKernelLimit); page++)
        SET_PAGE_USED(page);
}

void * PmmBitmap::GetFreePage()
{
    int byte, bit;
    for (byte = 0; byte < MEM_BITMAP_SIZE; byte++)
    {
        if (memBitmap[byte] != 0xFF)
        {
            for (bit = 0; bit < 8; bit++)
            {
                u8 b = memBitmap[byte];
                if (!(b & (1 << bit)))
                {
                    u32 page = 8 * byte + bit;
                    SET_PAGE_USED(page);
                    return (void*)(page * PAGE_SIZE);
                }
            }
        }
    }
    return (void*)(-1);
}

void PmmBitmap::ReleasePage(void * addr)
{
    SET_PAGE_UNUSED(addr);
}

/// @}
#define __PAGE_POOL__
#include "PagePool.hpp"

#include <kernel/arch/x86/MemCommon.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Pmm.hpp>

#include <kernel/Kernel.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("MEM", LOG_LEVEL, format, ##__VA_ARGS__)

#include <kernel/cppsupport.hpp>

/// @addgroup Memory
/// @{

void PagePool::Init()
{
    _base = gKernel.info.vPagePoolBase;
    _limit = gKernel.info.vPagePoolLimit;

     // Creates and Initializes the list of pages
    _InitPagesList();
}

Page PagePool::Allocate()
{
    Page resPage = { 0 };

    if (_availPageList == nullptr)
    {
        KLOG(LOG_WARNING, "No more page available in page pool !");
        return resPage;
    }

    PageBlock * newPage = _availPageList;
    _availPageList = newPage->next;

    if (_availPageList != nullptr)
        _availPageList->prev = nullptr;
    
    PageBlock * headUsedPage = _usedPageList;
    _usedPageList = newPage;
    newPage->available = false;
    newPage->next = headUsedPage;

    if (headUsedPage != nullptr)
        headUsedPage->prev = newPage;

    u32 pAddr = (u32)gPmm.GetFreePage();
    gVmm.AddPageToKernelPageDirectory((u32)newPage->addr, pAddr, PAGE_PRESENT | PAGE_WRITEABLE);

    resPage.pAddr = pAddr;
    resPage.vAddr = (u32)newPage->addr;

    return resPage;
}

void PagePool::Free(const Page page)
{
    PageBlock * usedPage = _usedPageList;

    while (usedPage != nullptr)
    {
        if ((u32)usedPage->addr == page.vAddr)
        {
            if (usedPage == _usedPageList)
            {
                _usedPageList = usedPage->next;
            }
            else
            {
                usedPage->prev = usedPage->next;
                if (usedPage->next != nullptr)
                    usedPage->next->prev = usedPage->prev;
            }

            _availPageList->prev = usedPage;
            usedPage->next = _availPageList;
            _availPageList = usedPage;
            usedPage->available = true;

            gPmm.ReleasePage((void *)page.pAddr);

            break;
        }
        else
        {
            usedPage = usedPage->next;
        }
    }
}

void PagePool::_InitPagesList()
{
    const unsigned int areaSize = _limit - _base;
    const unsigned int nbPages = areaSize / PAGE_SIZE;

    PageBlock * blocks = (PageBlock *)HeapAlloc(nbPages * sizeof(PageBlock));
    if (blocks == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", nbPages * sizeof(PageBlock));
        gKernel.Panic();
    }

    PageBlock * prev = nullptr;

    for (unsigned int index = 0; index < nbPages; index++)
    {
        blocks[index].available = true;
        blocks[index].addr = (void *)(_base + (index * PAGE_SIZE));
        blocks[index].next = nullptr;
        blocks[index].prev = prev;

        if (prev != nullptr)
        {
            prev->next = &blocks[index];
        }

        prev = &blocks[index];
    }

    _availPageList = blocks;
}

/// @}
#define __HEAP__
#include "Heap.hpp"

#include <kernel/Kernel.hpp>
#include <kernel/arch/x86/MemCommon.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/lib/StdLib.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("MEM", LOG_LEVEL, format, ##__VA_ARGS__)

/// @addgroup Memory
/// @{

void Heap::Init()
{
    baseBlock = (MemBlock *)gKernel.info.vHeapBase;
    limitBlock = (MemBlock *)gKernel.info.vHeapLimit;
    lastBlock = baseBlock;

    Sbrk(1);
}

MemBlock * Heap::Sbrk(int n)
{
    if (n <= 0)
    {
        KLOG(LOG_ERROR, "n <= 0");
        return nullptr;
    }

    // On ne doit pas dépasser la limite de l'espace réservé au tas
    if ((u32)lastBlock + (n * PAGE_SIZE) > gKernel.info.vHeapLimit)
    {
        KLOG(LOG_ERROR, "Kernel heap limit reached (%x, %x, %x, %x)", baseBlock, n, n*PAGE_SIZE, gKernel.info.vHeapLimit);
        // TODO : kernel error handler
        while (1);
        return nullptr;
    }
    else
    {
        unsigned int i = 0;
        MemBlock * newBlock = baseBlock;
        u32 heap = (u32)lastBlock;

        for (; i < n; i++)
        {
            void * new_page = gPmm.GetFreePage();

            if (new_page == nullptr)
            {
                KLOG(LOG_ERROR, "Couldn't find a free page");
                // TODO : kernel error handler
                while (1);
            }

            gVmm.AddPageToKernelPageDirectory(heap, (u32)new_page, PAGE_PRESENT | PAGE_WRITEABLE);

            heap += (u32)PAGE_SIZE;
        }

        lastBlock = (MemBlock *)heap;

        newBlock->size = n * PAGE_SIZE;
        newBlock->state = BLOCK_FREE;

        MemSet((&(newBlock->data)), 0, newBlock->size - BLOCK_HEADER_SIZE);

        return newBlock;
    }
}

void * Heap::Allocate(int size)
{
    if (size <= 0)
    {
        KLOG(LOG_WARNING, "Kernel allocation with size <= 0");
        return nullptr;
    }

    void * res = nullptr;
    res = _Allocate(baseBlock, size + BLOCK_HEADER_SIZE);
    return res;
}

void Heap::Free(void * ptr)
{
    if (ptr == nullptr)
    {
        KLOG(LOG_ERROR, "Trying to free a NULL pointer");
        return;
    }

    MemBlock * block = (MemBlock*)((u32)ptr - BLOCK_HEADER_SIZE);

    if (block->state == BLOCK_FREE)
    {
        KLOG(LOG_ERROR, "Double free on %x !", ptr);
        //__debugbreak();
        return;
    }

    block->state = BLOCK_FREE;
    MemSet(&(block->data), 0, block->size - BLOCK_HEADER_SIZE);
    //_Defrag(); // TODO : fix it
}

/* TODO : find a way to implemet mod */
static unsigned int _local_mod(unsigned int a, unsigned int b)
{
    while (a > b)
        a -= b;
    return (a > 0) ? 1 : 0;
}

void * Heap::_Allocate(MemBlock * block, int size)
{
    void * res_ptr = nullptr;

    while (block < lastBlock && res_ptr == nullptr)
    {
        if (block->state == BLOCK_USED || size > block->size)
        {
            block = (MemBlock *)((unsigned int)block + block->size);
            continue;
        }

        if ((block->size - size) >= (int)(BLOCK_HEADER_SIZE + MINIMAL_BLOCK_SIZE))
            _SplitBlock(block, size);

        res_ptr = &(block->data);
    }

    if (res_ptr == nullptr)
    {
        if (size > DEFAULT_BLOCK_SIZE)
        {
            unsigned int usize = (unsigned int)size;
            const unsigned int ubsize = (unsigned int)DEFAULT_BLOCK_SIZE;
            unsigned int n = usize / ubsize;
            if (_local_mod(usize, ubsize) > 0)
            {
                n++;
            }
            block = Sbrk(n);
        }
        else
            block = Sbrk(1);

        res_ptr = _Allocate(block, size);
    }

    block->state = BLOCK_USED;

    return res_ptr;
}

void Heap::_SplitBlock(MemBlock * block, unsigned int size)
{
    MemBlock * second_block = (MemBlock *)((unsigned int)block + size);
    second_block->size = block->size - size;
    second_block->state = BLOCK_FREE;
    block->size = size;
}

void Heap::_Defrag()
{
    MemBlock * block = baseBlock;

    while (block < lastBlock)
    {
        MemBlock * next = (MemBlock *)((unsigned int)block + block->size);

        if (next < lastBlock)
        {
            if (block->state == BLOCK_FREE && next->state == BLOCK_FREE)
            {
                block->size += next->size;
                MemSet(next, 0, BLOCK_HEADER_SIZE);
            }
        }
        block = (MemBlock *)((unsigned int)(block)+block->size);
    }
}

/// @}
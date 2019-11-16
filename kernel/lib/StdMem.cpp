#include <kernel/mem/Heap.hpp>
#include <kernel/mem/PagePool.hpp>

void * HeapAlloc(int size)
{
    return gHeap.Allocate(size);
}

void HeapFree(void * ptr)
{
    gHeap.Free(ptr);
}

Page PageAlloc()
{
    return gPagePool.Allocate();
}

void PageFree(const Page page)
{
    gPagePool.Free(page);
}

void MemCopy(const void * const src, void * dst, unsigned int size)
{
    u8 * _dst = (u8 *)dst;
    u8 * _src = (u8 *)src;

    while ((size--) > 0)
        *(_dst++) = *(_src++);
}

void MemSet(void * src, u8 byte, unsigned int size)
{
    u8 * _src = (u8 *)src;

    while ((size--) > 0)
        *(_src++) = byte;
}
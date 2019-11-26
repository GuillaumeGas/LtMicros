#include "malloc.h"
#include "stdlib.h"
#include "stdio.h"

#include "syscalls.h"

/// @brief Size in bytes of a block header
#define BLOCK_HEADER_SIZE sizeof(int)
/// @brief Block size in bytes of a block, without its header
#define DEFAULT_BLOCK_SIZE PAGE_SIZE - BLOCK_HEADER_SIZE
/// @brief Block size in bytes
#define DEFAULT_BLOCK_SIZE_WITH_HEADER DEFAULT_BLOCK_SIZE + BLOCK_HEADER_SIZE

/// @brief Minimal block size, used to determine if the block is large enough to be splitted
#define MINIMAL_BLOCK_SIZE 4
/// @brief Indicates that a block if free
#define BLOCK_FREE 0
/// @brief Indicates that a block if used
#define BLOCK_USED 1

#define PAGE_SIZE 4096

/// @brief Describes a block
struct MemBlock
{
    /// @brief Block size in bytes, including the header, 31 bits used for the size, 1 bit for the state
    unsigned int size : 31;
    /// @brief Indicates if the block is free or not
    unsigned int state : 1;
    /// @brief Represents the block content, not directly used, but &data instead
    void * data;
};

static void * _Allocate(MemBlock * block, int size);
static void _SplitBlock(MemBlock * block, unsigned int size);
static MemBlock * _Sbrk(int n);

MemBlock * g_baseBlock = nullptr;
MemBlock * g_limitBlock = nullptr;
MemBlock * g_lastBlock = nullptr;

void DumpHeap()
{
    MemBlock * currentBlock = g_baseBlock;
    printf("limit is %x\n", g_limitBlock);
    printf("last is %x\n", g_lastBlock);
    while (currentBlock <= g_limitBlock)
    {
        printf("[%x, %d, %s]\n", currentBlock, currentBlock->size, currentBlock->state == BLOCK_FREE ? "free" : "used");
        currentBlock = (MemBlock*)((unsigned int)currentBlock + currentBlock->size);
        if (currentBlock->size == 0)
            break;
    }
}

void InitMalloc()
{
    g_baseBlock = nullptr;
    g_limitBlock = nullptr;
    g_lastBlock = nullptr;
}

void * HeapAlloc(int size)
{
    if (size <= 0)
    {
        printf("wtf %d\n", size);
        return nullptr;
    }

    if (g_baseBlock == nullptr)
    {
        g_baseBlock = (MemBlock*)_Sbrk(1);
        g_baseBlock->size = PAGE_SIZE;
        g_baseBlock->state = BLOCK_FREE;
        g_limitBlock = g_baseBlock + PAGE_SIZE;
        g_lastBlock = g_baseBlock;
    }

    void * res = nullptr;
    res = _Allocate(g_baseBlock, size + BLOCK_HEADER_SIZE);
    if (res == nullptr)
    {
        printf("Dumping heap\n");
        DumpHeap();
        while (1);
    }
    return res;
}

void HeapFree(void * ptr)
{
    if (ptr == nullptr)
    {
        return;
    }

    MemBlock * block = (MemBlock*)((u32)ptr - BLOCK_HEADER_SIZE);

    if (block->state == BLOCK_FREE)
    {
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

static void * _Allocate(MemBlock * block, int size)
{
    void * res_ptr = nullptr;

    while (block <= g_lastBlock && res_ptr == nullptr)
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
            block = (MemBlock*)_Sbrk(n);
        }
        else
        {
            block = (MemBlock*)_Sbrk(1);
        }

        res_ptr = _Allocate(block, size);
    }

    block->state = BLOCK_USED;

    return res_ptr;
}

static void _SplitBlock(MemBlock * block, unsigned int size)
{
    MemBlock * second_block = (MemBlock *)((unsigned int)block + size);
    second_block->size = block->size - size;
    second_block->state = BLOCK_FREE;
    block->size = size;
}

static MemBlock * _Sbrk(int n)
{
    if (n <= 0)
    {
        printf("n <= 0\n");
        return nullptr;
    }

    unsigned int i = 0;
    MemBlock * newBlock = (MemBlock*)_sysSbrk(n);    

    newBlock->size = n * PAGE_SIZE;
    newBlock->state = BLOCK_FREE;

    MemSet((&(newBlock->data)), 0, newBlock->size - BLOCK_HEADER_SIZE);

    g_lastBlock = newBlock;
    g_limitBlock = g_lastBlock + g_lastBlock->size;

    return newBlock;
}
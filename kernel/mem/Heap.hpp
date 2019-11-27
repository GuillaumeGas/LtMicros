#pragma once

#include <kernel/lib/Types.hpp>
#include <kernel/arch/x86/MemCommon.hpp>

/// @file

/// @defgroup Memory Memory group
/// @{

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

/// @brief Represents the kernel heap
///        This is a basic heap implem, and it should be improved because of the following problems :
///          - We don't avoid fragmentation
///          - The physical size reserved by sbrk is never released
class Heap
{
public:
    /// @brief Initializes the heap by looking for base and limit heap space virtual addr in kernel info structure
    void Init();

    /// @brief Increase the heap size by allocating physical memory
    /// @param[in] n Number of pages
    /// @return A pointer to a block
    MemBlock * Sbrk(int n);

    /// @brief Allocates memory
    /// @param[in] size The size is bytes
    /// @return A pointer to the allocated memory on success, nullptr otherwise
    void * Allocate(int size);

    /// @brief Frees memory
    /// @param[in] ptr A pointer to the memory to be freed
    void Free(void * ptr);

    /// @brief Heap space base address (also, address of the first block)
    MemBlock * baseBlock;
    /// @brief Heap space limit virtual address
    MemBlock * limitBlock;
    /// @brief Last block address
    MemBlock * lastBlock;

    /// @brief Used to record how many allocation did the kernel
    int allocCount;
    /// @brief Used to record how many free memory did the kernel
    int freeCount;

private:
    void * _Allocate(MemBlock * block, unsigned int size);
    void _SplitBlock(MemBlock * block, unsigned int size);
    void _Defrag();
};

#ifdef __HEAP__
Heap gHeap;
#else
extern Heap gHeap;
#endif

/// @}
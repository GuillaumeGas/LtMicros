#pragma once

/// @file

#include <kernel/lib/StdLib.hpp>

/// @addgroup Memory
/// @{

/// @brief The page pool is a memory pool used to allocate paged align blocks of memory
class PagePool
{
public:
    /// @brief Initializes the page pool
    void Init();

    Page Allocate();
    void Free(const Page page);

private:
    /// @brief Used to describe a page in the page pool (availability, a pointer to the next one..)
    struct PageBlock;
    struct PageBlock
    {
        bool available;
        void * addr;
        PageBlock * next;
        PageBlock * prev;
    };

    /// @brief Creates a list of PageBlock to describe the page pool state
    void _InitPagesList();

    PageBlock * _availPageList;
    PageBlock * _usedPageList;

    u32 _base;
    u32 _limit;
};

#ifdef __PAGE_POOL__
PagePool gPagePool;
#else
extern PagePool gPagePool;
#endif

/// @}
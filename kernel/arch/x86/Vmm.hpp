#pragma once

/// @file

#include "MemCommon.hpp"

#include <kernel/lib/Types.hpp>
#include <kernel/lib/List.hpp>

/// @addgroup ArchX86Group
/// @{

#define NB_PAGES_TABLE_PER_DIRECTORY 1024
#define NB_PAGES_PER_TABLE           1024

#define EMPTY_PAGE_TABLE             0

/// @brief Number of page tables per kernel directory (The first Go of virtual mem is used for the kernel : 1024 / 4 = 256, if we check : 256 * 1024 * 4096 = 1Go)
#define NB_PAGES_TABLE_PER_KERNEL_DIRECTORY 256

/// @brief The user space begins at this virtual address
#define V_USER_BASE_ADDR 0x40000000

typedef u32 PAGE_FLAG;

#define PAGE_EMPTY                 0
#define PAGE_PRESENT               1
#define PAGE_WRITEABLE             2
#define PAGE_NON_PRIVILEGED_ACCESS 4
#define PAGE_PWT                   8
#define PAGE_PCD                   16
#define PAGE_ACCESSED              32
#define PAGE_SIZE_4MO              128
#define PAGE_WRITTEN               64
#define PAGE_G                     256

/// @brief Describes a page directory entry
struct PageDirectoryEntry
{
    u32 present : 1;
    u32 writable : 1;
    u32 nonPrivilegedAccess : 1;
    u32 pwt : 1;
    u32 pcd : 1;
    u32 accessed : 1;
    u32 reserved : 1;
    u32 pageSize : 1; /// 0 : 4Ko, 1 : 4Mo
    u32 global : 1;
    u32 avail : 3;
    u32 pageTableAddr : 20;
};

/// @brief Describes a page table entry
struct PageTableEntry
{
    u32 present : 1;
    u32 writable : 1;
    u32 nonPrivilegedAccess : 1;
    u32 pwt : 1;
    u32 pcd : 1;
    u32 accessed : 1;
    u32 written : 1;
    u32 reserved : 1;
    u32 global : 1;
    u32 avail : 3;
    u32 pageAddr : 20;
};

/// @brief Describes a page directory
struct PageDirectory
{
    /// @brief Physical address of the page directory
    PageDirectoryEntry * pdEntry;
    /// @brief Used to save the list of allocated pages on the page pool (used for storing page tables or directories, ...)
    List * pagesList;
};

/// @brief Describes a page, couple of physical and virtual address
struct Page
{
    u32 pAddr;
    u32 vAddr;
};

/// @brief Virtual Memory Manager : Class used to manage the virtual memory
class Vmm
{
public:
    /// @brief Vmm default class constructor, do nothing special
    Vmm();

    /// @brief Initializes the Virtual Memory Manager
    void Init();

    /// @brief Cleans a page directory by calling SetPageDirectoryEntry with PAGE_EMPTY on each entry.
    /// @param[in] pageDirectoryEntry A pointer to a page directory entry structure
    void CleanPageDirectory(PageDirectoryEntry * pageDirectoryEntry);

    /// @brief Cleans a page table by calling SetPageTableEntry with PAGE_EMPTY on each entry
    /// @param[in] pageTableEntry A pointer to a page table entry structure
    void CleanPageTable(PageTableEntry * pageTableEntry);

    /// @brief Set a given page directory entry
    /// @param[in] pd A pointer to a page directory entry structure
    /// @param[in] ptAddr A 32 bits physical address pointing to a page table entry
    /// @param[in] flag One or more value of the PAGE_FLAG enum to describe the page state
    void SetPageDirectoryEntry(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags);

    /// @brief Set a given page directory entry, and allows to define the global and avail fields 
    /// @param[in] pd A pointer to a page directory entry structure
    /// @param[in] ptAddr A 32 bits physical address pointing to a page table entry
    /// @param[in] flag One or more value of the PAGE_FLAG enum to describe the page state
    /// @param[in] global A value for the global page directory entry field
    /// @param[in] avail A value for the avail page directory entry field
    void SetPageDirectoryEntryEx(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags, u8 global, u8 avail);

    /// @brief Set a given page table entry
    /// @param[in] pt A pointer to a page table entry structure
    /// @param[in] pageAddr A 32 bits physical address pointing to a page address
    /// @param[in] flag One or more values of the PAGE_FLAG enum to describe the page state 
    void SetPageTableEntry(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags);

    /// @brief Set a given page table entry plus the global and avail fields
    /// @param[in] pt A pointer to a page table entry structure
    /// @param[in] pageAddr A 32 bits physical address pointing to a page address
    /// @param[in] flag One or more values of the PAGE_FLAG enum to describe the page state
    void SetPageTableEntryEx(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags, u8 global, u8 avail);

    /// @brief Looks for the physical address that is mapped by a given virtual address using the current page directory (stored is cr3 register)
    /// @param[in] virtualAddress A 32bits virtual address that mappes the physical address we are looking for
    /// @return A 32bits physical address
    u32 GetPhysicalAddressOf(u32 virtualAddress);

    /// @brief Updates the kernel virtual space mem by setting the right page table entry
    ///        We begins by checking if the virtual address is in the kernel space
    ///        Then we check that the page is present, and the page table entry is set
    /// @param[in] vAddr A 32bits virtual address in kernel space
    /// @param[in] pAddr A physicial address of the page we want to map in kernel space
    /// @param[in] flags Page's flags
    void AddPageToKernelPageDirectory(u32 vAddr, u32 pAddr, PAGE_FLAG flags);

    /// @brief Updates a given page directory to map a given physical address
    /// @warning The right page directory physical address must be in cr3
    /// @param[in] vAddr A 32bits virtual address in kernel space
    /// @param[in] pAddr A physicial address of the page we want to map in kernel space
    /// @param[in] flags Page's flags
    /// @param[in] pd The page directory that must be modified
    void AddPageToPageDirectory(u32 vAddr, u32 pAddr, PAGE_FLAG flags, PageDirectory pd);

    /// @brief Retrieve a page table entry given a virtual address
    /// @param[in] vAddr A 32bits virtual address
    /// @return A copy of the page table entry
    PageTableEntry GetPageTableFromVirtualAddress(u32 vAddr) const;

    /// @brief Updates a given page table entry
    /// @param[in] vAddr A 32bits virtual address
    /// @param[in] pageTableEntry The new page table entry
    void SetPageTableFromVirtualAddress(u32 vAddr, const PageTableEntry & pageTableEntry);

    /// @brief Indicates if a given virtual address is available (the page directory AND the page table entry must have the bit PAGE_PRESENT)
    /// @param[in] vAddr A 32bits virtual address
    /// @return true if available, else false
    bool IsVirtualAddressAvailable(u32 vAddr);

    /// @brief Indicates if a given virtual address is valid in user space
    /// @param[in] vAddr A 32bits address
    /// @return true if valid else false
    bool CheckUserVirtualAddressValidity(u32 vAddr);

    /// @brief Saves the current page directory addr (stored in cr3) in a temporary variable.
    ///        This page directory may be restored using RestoreMemoryMapping()
    void SaveCurrentMemoryMapping();

    /// @brief Restores a saved memory mapping (saved by using SaveCurrentMemoryMapping())
    void RestoreMemoryMapping();

    /// @brief Set the current page directory used by the processor (so set the cr3 register)
    /// @param[in] pd A pointer to a physical 32bits address to a page directory entry
    void SetCurrentPageDirectory(PageDirectoryEntry * pd);

    /// brief Retreives the current page directory used by the processor (by reading the cr3 register)
    /// @return A pointer to a page directory entry (physical address)
    PageDirectoryEntry * GetCurrentPageDirectory();

private:
    PageDirectoryEntry * s_SavedPageDirectoryEntry;

    /// @brief Initializes the kernel page directory
    ///        The kernel page directory entries must be PAGE_PRESENT and WRITEABLE
    ///        Same as for each page table of these entries
    void InitKernelPageDirectoryAndPageTables();

    /// @brief Indentity mapping for the kernel (v_addr == p_addr from 0x0 to 0x800000)
    ///        This area includes :
    ///          - GDT/IDT, 
    ///          - The Kernel page directory,
    ///          - Kernel stack
    ///          - Hardware area
    ///          - Kernel code
    ///          - Kernel page tables
    void SetIdentityMapping();
};

#ifdef __VMM__
Vmm gVmm;
#else
extern Vmm gVmm;
#endif

/// @}
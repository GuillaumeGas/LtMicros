#define __VMM__
#include "Vmm.hpp"

#include <kernel/lib/StdLib.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/Kernel.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("VMM", LOG_LEVEL, format, ##__VA_ARGS__)

/// @file

/// @addgroup ArchX86Group
/// @{

#define PD_OFFSET(addr) ((addr) & 0xFFC00000) >> 20
#define PT_OFFSET(addr) (addr & 0x003FF000) >> 12

/// @brief Assembly routine that set the page directory (by setting the cr3 register) and enables pagging (by setting the cr0 register)
/// @param[in] pd0_addr A 32bits address of the first page directory entry
extern "C" void _init_vmm(PageDirectoryEntry * pd0_addr);

/// @brief Routine written in assembly that set the current page directory used by the processor (so set the cr3 register)
/// @param[in] pd A pointer to a physical 32bits address to a page directory entry
extern "C" void _setCurrentPageDirectory(PageDirectoryEntry * pd);

/// brief Routine written in assembly that retreives the current page directory used by the processor (by reading the cr3 register)
/// @return A pointer to a page directory entry (physical address)
extern "C" PageDirectoryEntry * _getCurrentPageDirectory();

Vmm::Vmm()
{
    s_SavedPageDirectoryEntry = nullptr;
}

void Vmm::Init()
{
    InitKernelPageDirectoryAndPageTables();
    SetIdentityMapping();

    _init_vmm(gKernel.info.pPageDirectory.pdEntry);
    KLOG(LOG_INFO, "Pagging enabled");
}

void Vmm::InitKernelPageDirectoryAndPageTables()
{
    PageDirectoryEntry * pageDirectory = gKernel.info.pPageDirectory.pdEntry;
    PageTableEntry * pageTable = gKernel.info.pPageTables;
    unsigned int pdIndex = 0;

    for (; pdIndex < NB_PAGES_TABLE_PER_KERNEL_DIRECTORY; pdIndex++)
    {
        SetPageDirectoryEntry(&(pageDirectory[pdIndex]), (u32)pageTable, PAGE_PRESENT | PAGE_WRITEABLE);

        unsigned int j = 0;
        for (; j < NB_PAGES_PER_TABLE; j++)
        {
            SetPageTableEntry(&(pageTable[j]), 0, PAGE_PRESENT | PAGE_WRITEABLE);
        }

        SetPageTableEntry(&(pageTable[1023]), (u32)pageTable, PAGE_PRESENT | PAGE_WRITEABLE);

        pageTable = (PageTableEntry *)((unsigned int)pageTable + PAGE_SIZE);
    }

    SetPageDirectoryEntry(&(pageDirectory[1023]), (u32)pageDirectory, PAGE_PRESENT | PAGE_WRITEABLE);
}

void Vmm::SetIdentityMapping()
{
    PageTableEntry * kernelFirstPt = gKernel.info.pPageTables;
    PageTableEntry * kernelSecondPt = (PageTableEntry *)((unsigned int)kernelFirstPt + PAGE_SIZE);

    SetPageDirectoryEntry(gKernel.info.pPageDirectory.pdEntry, (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
    SetPageDirectoryEntry(&(gKernel.info.pPageDirectory.pdEntry[1]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);

    unsigned int ptIndex = 0;
    for (unsigned int page = PAGE(0); page < PAGE(gKernel.info.pKernelLimit); page++)
    {
        SetPageTableEntry(&(kernelFirstPt[ptIndex]), ptIndex * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITEABLE);
        ptIndex++;
    }

    // We make the last entry point to the first one
    // This trick is used to be able to modify a page directory/table entry (see algorithm of GetPhysicalAddressOf())
    SetPageTableEntry(&(kernelFirstPt[1023]), (u32)kernelFirstPt, PAGE_PRESENT | PAGE_WRITEABLE);
    SetPageTableEntry(&(kernelSecondPt[1023]), (u32)kernelSecondPt, PAGE_PRESENT | PAGE_WRITEABLE);
}

void Vmm::CleanPageDirectory(PageDirectoryEntry * pageDirectoryEntry)
{
    MemSet(pageDirectoryEntry, 0, sizeof(PageDirectoryEntry) * NB_PAGES_TABLE_PER_DIRECTORY);
}

void Vmm::CleanPageTable(PageTableEntry * pageTableEntry)
{
    MemSet(pageTableEntry, 0, sizeof(PageTableEntry) * NB_PAGES_PER_TABLE);
}

void Vmm::SetPageDirectoryEntry(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags)
{
    if (flags == PAGE_EMPTY)
        MemSet(pd, 0, sizeof(PageDirectoryEntry));

    u32 * addr = (u32 *)pd;
    *addr = ptAddr;
    pd->present = FlagOn(flags, PAGE_PRESENT);
    pd->writable = FlagOn(flags, PAGE_WRITEABLE);
    pd->nonPrivilegedAccess = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
    pd->pwt = FlagOn(flags, PAGE_PWT);
    pd->pcd = FlagOn(flags, PAGE_PCD);
    pd->accessed = FlagOn(flags, PAGE_ACCESSED);
    pd->pageSize = FlagOn(flags, PAGE_SIZE_4MO);
}

void Vmm::SetPageDirectoryEntryEx(PageDirectoryEntry * pd, u32 ptAddr, PAGE_FLAG flags, u8 global, u8 avail)
{
    SetPageDirectoryEntry(pd, ptAddr, flags);
    pd->avail = avail;
    pd->global = global;
}

void Vmm::SetPageTableEntry(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags)
{
    if (flags == PAGE_EMPTY)
        MemSet(pt, 0, sizeof(PageTableEntry));

    u32 * addr = (u32 *)pt;
    *addr = pageAddr;
    pt->present = FlagOn(flags, PAGE_PRESENT);
    pt->writable = FlagOn(flags, PAGE_WRITEABLE);
    pt->nonPrivilegedAccess = FlagOn(flags, PAGE_NON_PRIVILEGED_ACCESS);
    pt->pwt = FlagOn(flags, PAGE_PWT);
    pt->pcd = FlagOn(flags, PAGE_PCD);
    pt->accessed = FlagOn(flags, PAGE_ACCESSED);
    pt->written = FlagOn(flags, PAGE_WRITTEN);
}

void Vmm::SetPageTableEntryEx(PageTableEntry * pt, u32 pageAddr, PAGE_FLAG flags, u8 global, u8 avail)
{
    SetPageTableEntry(pt, pageAddr, flags);
    pt->avail = avail;
    pt->global = global;
}

u32 Vmm::GetPhysicalAddressOf(u32 virtualAddress)
{
    u32 * pde = nullptr;
    u32 * pte = nullptr;

    /*
        Reminder : A virtual address v is [ dir offset (10bits) | table offset (10bits) | page offset (12bits) ]

        The last entry of a page directory points to the first entry.
        The last entry of a page table points to the first entry.

        Then, if the first 10 bits are equal to 1 :

        Page directory
        ------------------
        |   Last entry   | --
        |       ...      |   |
        |   First entry  | <-/
        ------------------

        With this way, we are able to check or set this page directory entry.
        The algorithm is the same for checking/setting a page table entry.
    */

    // We retrieve the page directory entry and check if the page is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET((u32)virtualAddress));

    if ((*pde & PAGE_PRESENT))
    {
        // Now, we retrieve the page table entry...
        pte = (u32 *)(0xFFC00000 | (((u32)virtualAddress & 0xFFFFF000) >> 10));
        if ((*pte & PAGE_PRESENT))
            // And finally, we do what does the CPU, retrieve the physical address
            return ((*pte & 0xFFFFF000) + ((((u32)virtualAddress)) & 0x00000FFF));
    }

    return 0;
}

void Vmm::AddPageToKernelPageDirectory(u32 vAddr, u32 pAddr, PAGE_FLAG flags)
{
    u32 * pde = nullptr; // physical address of the page directory entry
    u32 * pte = nullptr; // physical address of the page table entry

    if (vAddr > V_USER_BASE_ADDR)
    {
        KLOG(LOG_ERROR, "%p is not in kernel space !", vAddr);
        gKernel.Panic();
        return;
    }

    // We check if the page pointed by the page directory entry is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));

    if (!FlagOn(*pde, PAGE_PRESENT))
    {
        KLOG(LOG_ERROR, "Page not found (0x%x)", pde);
        gKernel.Panic();
        return;
    }

    // We retrieve the page table entry and set it with the given physical address
    pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));

    SetPageTableEntry((PageTableEntry *)pte, pAddr, PAGE_PRESENT | PAGE_WRITEABLE);
}

void Vmm::AddPageToPageDirectory(u32 vAddr, u32 pAddr, PAGE_FLAG flags, PageDirectory pd)
{
    u32 * pde = nullptr; // physical address of the page directory entry
    u32 * pte = nullptr; // physical address of the page table entry

    // We retrieve the page directory entry and check if the page pointed by it is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));

    if (!FlagOn(*pde, PAGE_PRESENT))
    {
        u32 * pt = nullptr;  // new virtual address 

        // The page was not present, so we take a new one and add it to the page directory
        
        Page new_page = PageAlloc();
        pt = (u32 *)new_page.vAddr;

        SetPageDirectoryEntry((PageDirectoryEntry *)pde, (u32)new_page.pAddr, PAGE_PRESENT | PAGE_WRITEABLE | flags);

        // TODO : Is it necessary ?
        CleanPageTable((PageTableEntry *)pt);

        // TODO
        //ListPush(pd.pageTableList, new_page.vAddr);
    }

    pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));

    SetPageTableEntry((PageTableEntry *)pte, pAddr, flags);
}

PageTableEntry Vmm::GetPageTableFromVirtualAddress(u32 vAddr) const
{
    u32 * pde = nullptr; // physical address of the page directory entry
    u32 * pte = nullptr; // physical address of the page table entry

    // We retrieve the page directory entry and check if the page pointed by it is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));
    if (!FlagOn(*pde, PAGE_PRESENT))
    {
        KLOG(LOG_ERROR, "Page dir entry not found (0x%x)", pde);
        gKernel.Panic();
    }

    // We do the same with the page table entry
    pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));
    return *((PageTableEntry *)pte);
}

void Vmm::SetPageTableFromVirtualAddress(u32 vAddr, const PageTableEntry & pageTableEntry)
{
    u32 * pde = nullptr; // physical address of the page directory entry
    u32 * pte = nullptr; // physical address of the page table entry

    // We retrieve the page directory entry and check if the page pointed by it is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));
    if (!FlagOn(*pde, PAGE_PRESENT))
    {
        KLOG(LOG_ERROR, "Page dir entry not found (0x%x)", pde);
        gKernel.Panic();
    }

    // We do the same with the page table entry
    pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));
    *((PageTableEntry *)pte) = pageTableEntry;
}

bool Vmm::IsVirtualAddressAvailable(u32 vAddr)
{
    u32 * pde = nullptr; // physical address of the page directory entry
    u32 * pte = nullptr; // physical address of the page table entry

    // We retrieve the page directory entry and check if the page pointed by it is present
    pde = (u32 *)(0xFFFFF000 | PD_OFFSET(vAddr));
    if (!FlagOn(*pde, PAGE_PRESENT))
    {
        return FALSE;
    }

    // We do the same with the page table entry
    pte = (u32 *)(0xFFC00000 | ((vAddr & 0xFFFFF000) >> 10));
    if (!FlagOn(*pte, PAGE_PRESENT))
    {
        return FALSE;
    }

    return TRUE;
}

bool Vmm::CheckUserVirtualAddressValidity(u32 vAddr)
{
    return (vAddr != 0 && vAddr >= V_USER_BASE_ADDR);
}

void Vmm::SaveCurrentMemoryMapping()
{
    if (s_SavedPageDirectoryEntry != nullptr)
    {
        KLOG(LOG_WARNING, "The s_SavedPageDirectoryEntry is already used");
        // __debugbreak();
    }

    s_SavedPageDirectoryEntry = _getCurrentPageDirectory();
}

void Vmm::RestoreMemoryMapping()
{
    if (s_SavedPageDirectoryEntry != nullptr)
    {
        _setCurrentPageDirectory(s_SavedPageDirectoryEntry);
        s_SavedPageDirectoryEntry = nullptr;
    }
}

void Vmm::SetCurrentPageDirectory(PageDirectoryEntry * pd)
{
    _setCurrentPageDirectory(pd);
}

/// brief Retreives the current page directory used by the processor (by reading the cr3 register)
/// @return A pointer to a page directory entry (physical address)
PageDirectoryEntry * Vmm::GetCurrentPageDirectory()
{
    return _getCurrentPageDirectory();
}

/// @}
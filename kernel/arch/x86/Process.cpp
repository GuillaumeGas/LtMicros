#include "Process.hpp"
#include "Vmm.hpp"
#include "Thread.hpp"

#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/Kernel.hpp>
#include <kernel/lib/StdLib.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("ARCH", LOG_LEVEL, format, ##__VA_ARGS__)

#define DEFAULT_HEAP_BASE_ADDRESS 0x80000000

/// @addgroup ArchX86Group
/// @{

static int s_ProcessId = 1;

void Process::AddThread(Thread * thread)
{
    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return;
    }

    if (mainThread == nullptr)
        mainThread = thread;
    else
        thread->AddNeighbor(mainThread);
}

KeStatus Process::IncreaseHeap(unsigned int nbPages, u32 * allocatedBlockAddr)
{
    KeStatus status = STATUS_FAILURE;
    u32 newBlockAddress = heapLimitAddress;

    if (nbPages == 0)
    {
        KLOG(LOG_ERROR, "Invalid nbPages parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (allocatedBlockAddr == 0)
    {
        KLOG(LOG_ERROR, "Invalid allocatedBlockAddr parameter");
        return STATUS_NULL_PARAMETER;
    }

    gVmm.SaveCurrentMemoryMapping();
    gVmm.SetCurrentPageDirectory(pageDirectory.pdEntry);

    for (unsigned int i = 0; i < nbPages; i++)
    {
        u32 newBlock = heapLimitAddress + (i * PAGE_SIZE);
        u32 pHeap = (u32)gPmm.GetFreePage();

        gVmm.AddPageToPageDirectory(newBlock, pHeap, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, pageDirectory);
    }

    gVmm.RestoreMemoryMapping();

    heapLimitAddress += (nbPages * PAGE_SIZE);

    *allocatedBlockAddr = newBlockAddress;

    return STATUS_SUCCESS;
}

KeStatus Process::Create(Process ** newProcess, Process * parent)
{
    return Create(newProcess, DEFAULT_HEAP_BASE_ADDRESS, parent);
}

KeStatus Process::Create(Process ** newProcess, u32 heapBaseAddress, Process * parent)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    List * childrenList = nullptr;
    PageDirectory processPd = { 0 };

    if (newProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid newProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (heapBaseAddress == 0)
    {
        KLOG(LOG_ERROR, "Invalid heapBaseAddress value");
        return STATUS_INVALID_PARAMETER;
    }

    process = (Process *)HeapAlloc(sizeof(Process));
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Process));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    // We create the process page directory for the new process
    processPd = Process::CreateProcessPageDirectory();

    childrenList = ListCreate();
    if (childrenList == nullptr)
    {
        KLOG(LOG_ERROR, "ListCreate() returned nullptr");
        status = STATUS_FAILURE;
        goto clean;
    }

    // create process heap, no memory allocated, user must call sbrk syscall to increase the heap
    process->heapBaseAddress = heapBaseAddress;
    process->heapLimitAddress = heapBaseAddress;

    process->pageDirectory = processPd;
    process->pid = s_ProcessId++;
    process->childrenList = childrenList;

    *newProcess = process;

    process = nullptr;
    childrenList = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (process != nullptr)
    {
        HeapFree(process);
        process = nullptr;
    }

    if (childrenList != nullptr)
    {
        ListDestroy(childrenList);
        childrenList = nullptr;
    }

    return status;
}

KeStatus Process::CreateSystem(Process ** newProcess)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    List * childrenList = nullptr;
    PageDirectory processPd = { 0 };

    if (newProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid newProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    process = (Process *)HeapAlloc(sizeof(Process));
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Process));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    childrenList = ListCreate();
    if (childrenList == nullptr)
    {
        KLOG(LOG_ERROR, "ListCreate() returned nullptr");
        status = STATUS_FAILURE;
        goto clean;
    }

    process->pageDirectory = gKernel.info.pPageDirectory;
    process->pid = 0;
    process->childrenList = childrenList;

    *newProcess = process;

    process = nullptr;
    childrenList = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (process != nullptr)
    {
        HeapFree(process);
        process = nullptr;
    }

    if (childrenList != nullptr)
    {
        ListDestroy(childrenList);
        childrenList = nullptr;
    }

    return status;
}

void Process::Delete(Process * process)
{
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return;
    }

    if (process->childrenList != nullptr)
    {
        ListDestroy(process->childrenList);
        process->childrenList = nullptr;
    }

    ReleaseProcessPageDirectoryEntry(process->pageDirectory);

    HeapFree(process);
    process = nullptr;
}

PageDirectory Process::CreateProcessPageDirectory()
{
    Page pd_page = PageAlloc();
    PageDirectory pd = { 0 };
    PageDirectoryEntry * kernelPdEntry = (PageDirectoryEntry *)gKernel.info.pPageDirectory.pdEntry;
    PageDirectoryEntry * pdEntry = (PageDirectoryEntry *)pd_page.vAddr;
    unsigned int i = 0;

    for (; i < NB_PAGES_TABLE_PER_KERNEL_DIRECTORY; i++)
        pdEntry[i] = kernelPdEntry[i];

    for (i = 256; i < NB_PAGES_TABLE_PER_DIRECTORY; i++)
        gVmm.SetPageDirectoryEntry(&(pdEntry[i]), 0, PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS);

    gVmm.SetPageDirectoryEntry(&(pdEntry[1023]), (u32)pd_page.pAddr, PAGE_PRESENT | PAGE_WRITEABLE);

    pd.pdEntry = (PageDirectoryEntry *)pd_page.pAddr;
    pd.pagesList = ListCreate();

    // A page was allocated on the page pool, we want to be able to release it
    // when the process is deleted, so we save it in a pages list

    // Allocating a list entry
    Page * savedPage = (Page *)HeapAlloc(sizeof(Page));
    if (savedPage == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Page));
        gKernel.Panic();
    }

    MemCopy(&pd_page, savedPage, sizeof(Page));

    ListPush(pd.pagesList, savedPage);

    return pd;
}

static void _CleanPageListEntry(void * listEntry)
{
    Page * page = (Page *)listEntry;

    if (page == nullptr)
    {
        KLOG(LOG_ERROR, "page is null");
        return;
    }

    PageFree(*page);
    page = nullptr;
}

void Process::ReleaseProcessPageDirectoryEntry(PageDirectory pd)
{
    if (pd.pagesList != nullptr)
    {
        ListDestroyEx(pd.pagesList, _CleanPageListEntry);
    }
}

/// @}
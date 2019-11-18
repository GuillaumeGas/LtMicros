#include "Process.hpp"
#include "Vmm.hpp"
#include "Thread.hpp"

#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/Kernel.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/mem/Vad.hpp>

#include <kernel/lib/StdIo.hpp>

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

KeStatus Process::IncreaseHeap(unsigned int nbPages, u8 ** allocatedBlockAddr)
{
    KeStatus status = STATUS_FAILURE;
    u8 * newBlockAddress = defaultHeap.limitAddress;

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

    if ((defaultHeap.limitAddress + (nbPages * PAGE_SIZE)) > defaultHeap.vad->limitAddress)
    {
        KLOG(LOG_ERROR, "Avoiding process heap overflow");
        return STATUS_PROCESS_HEAP_LIMIT_REACHED;
    }

    // TODO : revoir le fonctionnement de la heap

    //PageDirectoryEntry * pde = gVmm.GetCurrentPageDirectory();
    //gVmm.SetCurrentPageDirectory(pageDirectory.pdEntry);

    //for (unsigned int i = 0; i < nbPages; i++)
    //{
    //    u32 newBlock = heapLimitAddress + (i * PAGE_SIZE);
    //    u32 pHeap = (u32)gPmm.GetFreePage();

    //    gVmm.AddPageToPageDirectory(newBlock, pHeap, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, pageDirectory);
    //}

    //gVmm.SetCurrentPageDirectory(pde);

    defaultHeap.limitAddress += (nbPages * PAGE_SIZE);

    if ((defaultHeap.limitAddress - defaultHeap.baseAddress) > PAGE_SIZE)
    {
        KLOG(LOG_ERROR, "IncreaseHeap not implemented");
        gKernel.Panic();
    }

    *allocatedBlockAddr = newBlockAddress;

    return STATUS_SUCCESS;
}

KeStatus Process::Create(Process ** newProcess, Process * parent)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    List * childrenList = nullptr;
    PageDirectory processPd = { 0 };
    Vad * baseVad = nullptr;

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

    // We create the process page directory for the new process
    processPd = Process::CreateProcessPageDirectory();

    childrenList = ListCreate();
    if (childrenList == nullptr)
    {
        KLOG(LOG_ERROR, "ListCreate() returned nullptr");
        status = STATUS_FAILURE;
        goto clean;
    }

    status = Vad::Create((void*)V_PROCESS_BASE_ADDR, (V_PROCESS_LIMIT_ADDR - V_PROCESS_BASE_ADDR), true, &baseVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Vad::Create() failed with code %d", status);
        goto clean;
    }

    process->pageDirectory = processPd;
    process->pid = s_ProcessId++;
    process->childrenList = childrenList;
    process->mainThread = nullptr;
    process->baseVad = baseVad;

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

void Process::MemoryCopy(const u8 * const sourceAddress, u8 * const destAddress, const unsigned int size)
{
    PageDirectoryEntry * currentPd = gVmm.GetCurrentPageDirectory();

    if (sourceAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid sourceAddress parameter");
        return;
    }

    if (destAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid destAddress parameter");
        return;
    }

    gVmm.SetCurrentPageDirectory(this->pageDirectory.pdEntry);

    MemCopy(sourceAddress, destAddress, size);

    gVmm.SetCurrentPageDirectory(currentPd);
}

void Process::MemorySetAndCopy(const u8 * const sourceAddress, u8 * const destAddress, const unsigned int size, const u8 byte)
{
    PageDirectoryEntry * currentPd = gVmm.GetCurrentPageDirectory();

    if (sourceAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid sourceAddress parameter");
        return;
    }

    if (destAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid destAddress parameter");
        return;
    }

    gVmm.SetCurrentPageDirectory(this->pageDirectory.pdEntry);

    MemSet(destAddress, byte, size);
    MemCopy(sourceAddress, destAddress, size);

    gVmm.SetCurrentPageDirectory(currentPd);
}

KeStatus Process::AllocateMemory(const unsigned int size, void ** const outAddress)
{
    KeStatus status = STATUS_FAILURE;
    Vad * newVad = nullptr;

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (outAddress == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outAddress parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = baseVad->Allocate(size, &pageDirectory, &newVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Vad::Allocate() failed with cod %d", status);
        goto clean;
    }

    *outAddress = newVad->baseAddress;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Process::AllocateMemoryAtAddress(void * const address, const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;
    Vad * newVad = nullptr;

    if (address == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid address parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    status = baseVad->AllocateAtAddress(address, size, &pageDirectory, &newVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Vad::AllocateAtAddresss() failed with cod %d", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Process::CreateDefaultHeapAndStack()
{
    KeStatus status = STATUS_FAILURE;
    Vad * heapVad = nullptr;

    status = baseVad->Allocate(DEFAULT_HEAP_SIZE, &this->pageDirectory, &heapVad);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Vad::Allocate() failed with code %d", status);
        goto clean;
    }

    this->defaultHeap.vad = heapVad;
    this->defaultHeap.baseAddress = heapVad->baseAddress;
    this->defaultHeap.limitAddress = heapVad->limitAddress;

    this->mainThread->CreateDefaultStack();
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Thread::CreateDefaultStack() failed with code %d", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

void Process::PrintState()
{
    kprint("\nProcess %d at address %x\n", pid, this);
    kprint("Page directory entry : %x\n", pageDirectory.pdEntry);
    kprint("Default heap (%x - %x, %d bytes)\n", defaultHeap.baseAddress, defaultHeap.limitAddress, defaultHeap.vad->size);
    kprint("Base vad : [%x - %x, %d bytes]\n\n", baseVad->baseAddress, baseVad->limitAddress, baseVad->size);

    mainThread->PrintList();
}

/// @}
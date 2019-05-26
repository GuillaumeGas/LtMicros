#include "Thread.hpp"
#include "Process.hpp"
#include "Vmm.hpp"
#include "Gdt.hpp"
#include "Pmm.hpp"

#include <kernel/Kernel.hpp>
#include <kernel/lib/StdLib.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("ARCH", LOG_LEVEL, format, ##__VA_ARGS__)

/// @addgroup ArchX86Group
/// @{

static int s_ThreadId = 0;

void Thread::AddNeighbor(Thread * thread)
{
    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return;
    }

    if (neighbor != nullptr)
        thread->neighbor = neighbor;

    neighbor = thread;
}

void Thread::SaveState(InterruptContext * context)
{
    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        // TODO : handle critical error
        while (1);
    }

    regs.eflags = context->eflags;
    regs.cs = context->cs;
    regs.eip = context->eip;
    
    regs.eax = context->eax;
    regs.ebx = context->ebx;
    regs.ecx = context->ecx;
    regs.edx = context->edx;

    regs.ebp = context->ebp;
    regs.esi = context->esi;
    regs.edi = context->edi;

    regs.gs = context->gs;
    regs.fs = context->fs;
    regs.es = context->es;
    regs.ds = context->ds;

    // If we come from a user task
    if (regs.cs != KERNEL_CODE_SELECTOR)
    {
        // If an interrupt occures when a user task is running, 5 registers are pushed
        // on the stack (instead of 3 for kernel land)

        InterruptFromUserlandContext * uContext = (InterruptFromUserlandContext *)context;

        regs.ss = uContext->ss;
        regs.esp = uContext->esp;
    }
    else
    {
        regs.ss = gTss.ss0;
        // We need the value of esp before the interrupt
        // The esp value pushed on the stack (context->esp) corresponds to the esp value
        // after the interrupt, and before the pushad instruction.
        // The interrupt occured during an kernel task, so only 3 registers were pushed by the processor
        //  eflags, cs and eip : 3 * 4 bytes = 12 bytes
        regs.esp = context->esp + 12;
    }

    kstack.esp0 = gTss.esp0;
    kstack.ss0 = gTss.ss0;
}

void Thread::StartOrResume()
{
    PageDirectoryEntry * pd = nullptr;

    state = THREAD_STATE_RUNNING;

    if (privilegeLevel == PVL_USER)
    {
        pd = process->pageDirectory.pdEntry;

        gVmm.SaveCurrentMemoryMapping();
        gVmm.SetCurrentPageDirectory(pd);
        gVmm.AddPageToPageDirectory(stackPage.vAddr, stackPage.pAddr, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, process->pageDirectory);
        gVmm.RestoreMemoryMapping();
    }
    else
    {
        pd = gKernel.info.pPageDirectory.pdEntry;
    }

    gTss.esp0 = kstack.esp0;
    gTss.ss0 = kstack.ss0;

    _startOrResumeThread (
        pd,
        regs.ss,
        regs.esp,
        regs.eflags,
        regs.cs,
        regs.eip,
        regs.eax,
        regs.ecx,
        regs.edx,
        regs.ebx,
        regs.ebp,
        regs.esi,
        regs.edi,
        regs.ds,
        regs.es,
        regs.fs,
        regs.gs,
        regs.cs == KERNEL_CODE_SELECTOR ? PVL_KERNEL : PVL_USER
    );
}

KeStatus Thread::CreateThread(u32 entryAddr, Process * process, PrivilegeLevel privLevel, Thread ** thread)
{
    KeStatus status = STATUS_FAILURE;
    Thread * localThread = nullptr;

    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (privLevel != PVL_USER && privLevel != PVL_KERNEL)
    {
        KLOG(LOG_ERROR, "Invalid privLevel parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    localThread = (Thread *)HeapAlloc(sizeof(Thread));
    if (localThread == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Thread));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    if (privLevel == PVL_KERNEL)
    {
        status = _InitKernelThread(localThread, entryAddr);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "_InitKernelThread() failed with code %d", status);
            goto clean;
        }
    }
    else
    {
        status = _InitUserThread(localThread, entryAddr);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "_InitUserThread() failed with code %d", status);
            goto clean;
        }
    }

    localThread->tid = s_ThreadId++;
    localThread->state = THREAD_STATE_INIT;
    localThread->process = process;
    localThread->privilegeLevel = privLevel;

    *thread = localThread;
    localThread = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localThread != nullptr)
    {
        HeapFree(localThread);
        localThread = nullptr;
    }

    return status;
}

KeStatus Thread::_InitUserThread(Thread * thread, u32 entryAddr)
{
    KeStatus status = STATUS_FAILURE;
    Page kernelStackPage = { 0 };
    u32 pUserStack = 0;
    u32 vUserStack = 0;

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (entryAddr == 0)
    {
        KLOG(LOG_ERROR, "Invalid entryAddr parameter");
        return STATUS_INVALID_PARAMETER;
    }

    // Allocating a kernel stack to handle interrupts
    kernelStackPage = PageAlloc();

    // Allocating the user stack
    pUserStack = (u32)gPmm.GetFreePage();
    if (pUserStack == 0)
    {
        KLOG(LOG_ERROR, "Couldn't find a free physical page");
        status = STATUS_PHYSICIAL_MEMORY_FULL;
        goto clean;
    }

    // The previous physical stack address will be mapped to this virtual address
    // This means that all threads have their stack at the same virtual address
    vUserStack = USER_STACK_V_ADDR;

    // We initialize thread info
    thread->stackPage.vAddr = vUserStack;
    thread->stackPage.pAddr = pUserStack;

    thread->regs.ss = USER_DATA_SELECTOR_WITH_RPL;
    thread->regs.cs = USER_CODE_SELECTOR_WITH_RPL;
    thread->regs.ds = USER_DATA_SELECTOR_WITH_RPL;
    thread->regs.es = USER_DATA_SELECTOR_WITH_RPL;
    thread->regs.fs = USER_DATA_SELECTOR_WITH_RPL;
    thread->regs.gs = USER_DATA_SELECTOR_WITH_RPL;

    thread->regs.eax = 0;
    thread->regs.ecx = 0;
    thread->regs.edx = 0;
    thread->regs.edi = 0;
    thread->regs.esi = 0;

    thread->regs.esp = ((vUserStack + (u32)PAGE_SIZE) - (u32)(sizeof(void*)));
    thread->regs.ebp = thread->regs.esp;
    thread->regs.eip = entryAddr;

    thread->regs.eflags = 0x200 & 0xFFFFBFFF;
    thread->kstack.esp0 = (((u32)kernelStackPage.vAddr + PAGE_SIZE) - (u32)(sizeof(void*)));
    thread->kstack.ss0 = KERNEL_DATA_SELECTOR;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus Thread::_InitKernelThread(Thread * thread, u32 entryAddr)
{
    KeStatus status = STATUS_FAILURE;
    Page kernelStackPage = { 0 };

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    // Allocating a stack for the kernel thread
    kernelStackPage = PageAlloc();

    thread->stackPage.vAddr = kernelStackPage.vAddr;
    thread->stackPage.pAddr = kernelStackPage.pAddr;

    thread->regs.ss = KERNEL_DATA_SELECTOR;
    thread->regs.cs = KERNEL_CODE_SELECTOR;
    thread->regs.ds = KERNEL_DATA_SELECTOR;
    thread->regs.es = KERNEL_DATA_SELECTOR;
    thread->regs.fs = KERNEL_DATA_SELECTOR;
    thread->regs.gs = KERNEL_DATA_SELECTOR;

    thread->regs.eax = 0;
    thread->regs.ecx = 0;
    thread->regs.edx = 0;
    thread->regs.edi = 0;
    thread->regs.esi = 0;

    thread->regs.esp = (((u32)kernelStackPage.vAddr + (u32)PAGE_SIZE) - (u32)(sizeof(void*)));
    thread->regs.ebp = thread->regs.esp;
    thread->regs.eip = entryAddr;

    thread->regs.eflags = 0x200 & 0xFFFFBFFF;

    // These registers shouldn't be used the thread beeing running the kernel land
    thread->kstack.esp0 = 0;
    thread->kstack.ss0 = KERNEL_DATA_SELECTOR;

    status = STATUS_SUCCESS;

clean:
    return status;
}

/// @}
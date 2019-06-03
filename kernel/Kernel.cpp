#define __KERNEL__
#include "Kernel.hpp"

#include <kernel/arch/x86/Idt.hpp>
#include <kernel/arch/x86/Gdt.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Syscalls.hpp>

#include <kernel/drivers/Pic.hpp>
#include <kernel/drivers/proc_io.hpp>
#include <kernel/drivers/Clock.hpp>
#include <kernel/drivers/Serial.hpp>

#include <kernel/mem/PagePool.hpp>
#include <kernel/mem/Heap.hpp>

#include <kernel/task/ProcessManager.hpp>
#include <kernel/task/Scheduler.hpp>

#include <kernel/syscalls/SyscallsHandler.hpp>

#include <kernel/lib/StdIo.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/List.hpp>

#include <kernel/debug/LtDbg.hpp>

#include <kernel/tests/UserTests.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("KERNEL", LOG_LEVEL, format, ##__VA_ARGS__)

/// @brief Kernel page directory physical location
#define KERNEL_PAGE_DIR_P_ADDR   0x1000

/// @brief Kernel page tables physical location
#define KERNEL_PAGES_TABLE_P_ADDR 0x400000

//#define DEBUG_MODE

Kernel::Kernel()
{
    info.pPageDirectory.pdEntry = (PageDirectoryEntry *)KERNEL_PAGE_DIR_P_ADDR;
    info.pPageTables = (PageTableEntry *)KERNEL_PAGES_TABLE_P_ADDR;
    info.pKernelLimit = KERNEL_LIMIT_P_ADDR;
    info.pStackAddr = KERNEL_STACK_P_ADDR;
    info.vPagePoolBase = KERNEL_PAGE_POOL_V_BASE_ADDR;
    info.vPagePoolLimit = KERNEL_PAGE_POOL_V_LIMIT_ADDR;
    info.vHeapBase = KERNEL_HEAP_V_BASE_ADDR;
    info.vHeapLimit = KERNEL_HEAP_V_LIMIT_ADDR;

#ifdef DEBUG_MODE
    info.debug = true;
#else
    info.debug = false;
#endif
}

void Kernel::Init(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
    gLogger.SetMode(LOG_SCREEN);

    CheckMultibootPartialInfo(mbi, multibootMagicNumber);

    gIdt.Init();
    KLOG(LOG_INFO, "IDT initialized");

    gPicDrv.Init();
    KLOG(LOG_INFO, "PIC initialized");

    gClockDrv.Init();
    KLOG(LOG_INFO, "Clock driver initialized");

    gSerialDrv.Init();
    KLOG(LOG_INFO, "Serial driver initialized");

    gLogger.SetMode(LOG_SCREEN | LOG_SERIAL);

    gPmm.Init();
    KLOG(LOG_INFO, "Physical Memory Manager intialized");

    gVmm.Init();
    KLOG(LOG_INFO, "Virtual Memory Manager intialized");

    gHeap.Init();
    gPagePool.Init();

    KLOG(LOG_INFO, "Kernel heap initialized");

    gProcessManager.Init();
    KLOG(LOG_INFO, "Process manager initialized");

    gScheduler.Init();
    KLOG(LOG_INFO, "Scheduler initialized");

    gSyscallsX86.Init();
    KLOG(LOG_INFO, "X86 syscalls handler initialized");

    if (gKernel.info.debug)
    {
        gLtDbg.Init();
        KLOG(LOG_INFO, "Kernel debugger initialized");
        __debugbreak();
    }
}

void Kernel::Start()
{
    Process * systemProcess = nullptr;
    Thread * testThread = nullptr;

    KeStatus status = gProcessManager.CreateSystemProcess(&systemProcess);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "gProcessManager::Create() failed with code %d", status);
        Panic();
        goto clean;
    }

    gKernel.process = systemProcess;

    gScheduler.AddThread(systemProcess->mainThread);

    status = LoadModules();
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LoadAndStartModule() failed with code %d", status);
        Panic();
        goto clean;
    }

    gScheduler.Start();

    ENABLE_IRQ();

clean:
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LtMicros kernel failed to start !");
        Panic();
    }
    else
    {
        KLOG(LOG_DEBUG, "Kernel started !");
    }

    while (1);
}

void Kernel::CheckMultibootPartialInfo(MultibootPartialInfo * mbi, u32 multibootMagicNumber)
{
    gMbi = *mbi;

    if (multibootMagicNumber != MULTIBOOT_HEADER_MAGIC)
    {
        KLOG(LOG_ERROR, "Invalid magic number : %x", multibootMagicNumber);
        Panic();
    }

    if (FlagOn(mbi->flags, MEM_INFO))
    {
        KLOG(LOG_INFO, "RAM detected : %x (lower), %x (upper)", mbi->low_mem, mbi->high_mem);
    }
    else
    {
        KLOG(LOG_WARNING, "Missing MEM_INFO flag !");
        Panic();
    }

    if (FlagOn(mbi->flags, MODS_INFO))
    {
        KLOG(LOG_DEBUG, "Found %d module(s)", mbi->mods_count);
    }

    gKernel.info.multibootInfo = mbi;
}

KeStatus Kernel::LoadModules()
{
    MultibootPartialInfo * mbi = gKernel.info.multibootInfo;
    MultiBootModule * module = mbi->mods_addr;

    if (mbi->mods_count == 0 || mbi->mods_addr == nullptr)
        return STATUS_SUCCESS;

    for (int i = 0; i < mbi->mods_count; i++)
    {
        KLOG(LOG_INFO, "Loading module %s", module->name);
        UserSystemTest(module->mod_start);
        module += sizeof(MultiBootModule);
    }

    return STATUS_SUCCESS;
}

void Kernel::Cleanup()
{

}

void Kernel::Panic()
{
    DISABLE_IRQ();
    kprint("Kernel Panic !\n");
    Pause();
}
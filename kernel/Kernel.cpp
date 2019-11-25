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
#include <kernel/task/Ipc.hpp>

#include <kernel/syscalls/SyscallsHandler.hpp>

#include <kernel/lib/StdIo.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/List.hpp>

#include <kernel/module/Module.hpp>

#include <kernel/handle/HandleManager.h>

#include <kernel/debug/LtDbg.hpp>

#include <kernel/Logger.hpp>

//#define DEBUG_MODE
#define DEBUG_PRINT_MODE

#ifdef DEBUG_PRINT_MODE
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("KERNEL", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define KLOG(LOG_LEVEL, format, ...)
#endif

/// @brief Kernel page directory physical location
#define KERNEL_PAGE_DIR_P_ADDR   0x1000

/// @brief Kernel page tables physical location
#define KERNEL_PAGES_TABLE_P_ADDR 0x400000

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
    gPicDrv.Init();
    gClockDrv.Init();
    gSerialDrv.Init();

    gLogger.SetMode(LOG_SCREEN | LOG_SERIAL);

    gPmm.Init();
    gVmm.Init();
    gHeap.Init();
    gPagePool.Init();
    gProcessManager.Init();
    gHandleManager.Init();
    gScheduler.Init();
    gIpcHandler.Init();
    gSyscallsX86.Init();
    
    PrintHello();

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
        KLOG(LOG_ERROR, "gProcessManager::Create() failed with code %t", status);
        Panic();
        goto clean;
    }

#ifdef DEBUG_MODE
    __debugbreak();
#endif

    gKernel.process = systemProcess;

    gScheduler.AddThread(systemProcess->mainThread);

    LoadModules();

    KLOG(LOG_INFO, "Starting LtMicros...\n");

    gScheduler.Start();

    ENABLE_IRQ();

clean:
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "LtMicros kernel failed to start !");
        Panic();
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

    gKernel.info.multibootInfo = mbi;
}

void Kernel::LoadModules()
{
    MultibootPartialInfo * mbi = gKernel.info.multibootInfo;
    MultiBootModule * module = mbi->mods_addr;

    if (!FlagOn(mbi->flags, MODS_INFO))
        return;

    KLOG(LOG_INFO, "Found %d module(s)", mbi->mods_count);

    if (mbi->mods_count == 0 || mbi->mods_addr == nullptr)
        return;

    for (int i = 0; i < mbi->mods_count; i++)
    {
        KLOG(LOG_INFO, "Loading module %s (%d bytes)", module[i].name, ((int)module[i].mod_end - (int)module[i].mod_start));
        Module::Load(&module[i]);
    }
}

void Kernel::Panic()
{
    DISABLE_IRQ();
    kprint("Kernel Panic !\n");
    Pause();
}

void Kernel::PrintHello()
{
    kprint(" _    _   __  __ _                    \n");
    kprint("| |  | |_|  \\/  (_) ___ _ __ ___  ___ \n");
    kprint("| |  | __| |\\/| | |/ __| '__/ _ \\/ __|\n");
    kprint("| |__| |_| |  | | | (__| | | (_) \\__ \\\n");
    kprint("|_____\\__|_|  |_|_|\\___|_|  \\___/|___/\n\n");
}



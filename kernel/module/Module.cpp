#include "Module.hpp"
#include "Elf.hpp"

#include <kernel/Kernel.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/task/ProcessManager.hpp>
#include <kernel/task/Scheduler.hpp>
#include <kernel/lib/StdLib.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("MODULE", LOG_LEVEL, format, ##__VA_ARGS__)

void Module::Load(MultiBootModule * module)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    ElfFile elf;

    if (!Elf::ElfCheckIdent((ElfHeader *)module->mod_start))
    {
        KLOG(LOG_ERROR, "The module is not a elf file");
        gKernel.Panic();
    }

    if (Elf::ElfInit(module->mod_start, &elf) != STATUS_SUCCESS)
    {
        KLOG(LOG_ERROR, "ElfInit() failed");
        gKernel.Panic();
    }

    status = gProcessManager.CreateProcess(elf.header->entry, &process, SA_IO, nullptr);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ProcessManager::CreateProcess() failed with code %d", status);
        gKernel.Panic();
    }

    status = _MapElfInProcess(elf, process);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "MapElfInProcess() failed with code %d", status);
        gKernel.Panic();
    }

    status = process->CreateDefaultHeapAndStack();
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Process::CreateDefaultHeapAndStack() failed with code %d", status);
        gKernel.Panic();
    }

    gScheduler.AddThread(process->mainThread);
}

/* TODO : find a way to implemet mod */
static unsigned int _local_mod(unsigned int a, unsigned int b)
{
    while (a > b)
        a -= b;
    return (a > 0) ? 1 : 0;
}

KeStatus Module::_MapElfInProcess(ElfFile elf, Process * process)
{
    KeStatus status = STATUS_FAILURE;

    for (int i = 0; i < elf.header->phnum; i++)
    {
        u8 * vUserSectionPtr = (u8 *)elf.prgHeaderTable[i].vaddr;
        u32 sectionSize = elf.prgHeaderTable[i].memSize;
        u8 * pSectionPtr = (u8 *)((u32)elf.header + elf.prgHeaderTable[i].offset);

        if (vUserSectionPtr == nullptr)
            break;

        status = process->AllocateMemoryAtAddress(vUserSectionPtr, sectionSize);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Process::AllocateMemoryAtAddress() failed with status %d", status);
            goto clean;
        }

        process->MemorySetAndCopy(pSectionPtr, vUserSectionPtr, sectionSize, 0);
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}
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

    char * pAddr = (char *)gPmm.GetFreePage();

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

    gVmm.SaveCurrentMemoryMapping();
    gVmm.SetCurrentPageDirectory(process->pageDirectory.pdEntry);

    for (int i = 0; i < elf.header->phnum; i++)
    {
        u8 * vUserSectionPtr = (u8 *)elf.prgHeaderTable[i].vaddr;
        u32 sectionSize = elf.prgHeaderTable[i].memSize;
        u8 * pSectionPtr = (u8 *)((u32)elf.header + elf.prgHeaderTable[i].offset);
        u8 * vNewPage = nullptr;

        if (vUserSectionPtr == nullptr)
            break;

        if (vNewPage == nullptr || sectionSize > PAGE_SIZE)
        {
            unsigned int pagesToAllocate = sectionSize / PAGE_SIZE;
            u32 offset = 0;

            if (_local_mod(sectionSize, PAGE_SIZE) > 0)
            {
                pagesToAllocate++;
            }

            if (vNewPage == nullptr)
            {
                vNewPage = vUserSectionPtr;
            }
            else
            {
                vNewPage = (u8 *)((unsigned int)vNewPage + (unsigned int)PAGE_SIZE);
            }

            do
            {
                if (!gVmm.CheckUserVirtualAddressValidity((u32)vNewPage))
                {
                    KLOG(LOG_ERROR, "Invalid user virtual address (%x), can't map code in memory\n", vNewPage);
                    gKernel.Panic();
                }

                gVmm.AddPageToPageDirectory((u32)vNewPage, (u32)((u32)pSectionPtr + offset), PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, process->pageDirectory);

                pagesToAllocate--;

                vNewPage = (u8 *)((unsigned int)vNewPage + (unsigned int)PAGE_SIZE);
                offset += (u32)PAGE_SIZE;
            } while (pagesToAllocate > 0);
        }
    }

    gVmm.RestoreMemoryMapping();

    status = STATUS_SUCCESS;

    return status;
}
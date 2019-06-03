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

    gVmm.SaveCurrentMemoryMapping();
    gVmm.SetCurrentPageDirectory(process->pageDirectory.pdEntry);

    for (int i = 0; i < elf.header->phnum; i++)
    {
        u8 * vUserCodePtr = (u8 *)elf.prgHeaderTable[i].vaddr;
        u32 size = elf.prgHeaderTable[i].memSize;
        u32 count = 0;
        u8 * codeAddr = (u8 *)((u32)elf.header + elf.prgHeaderTable[i].offset);

        if (vUserCodePtr == nullptr)
            break;

        while (count < size)
        {
            // pas top, il faudrait prendre en compte la taille du code, et la pile utilisateur
            if (!gVmm.CheckUserVirtualAddressValidity((u32)vUserCodePtr))
            {
                KLOG(LOG_ERROR, "Invalid user virtual address (%x), can't map code in memory\n", vUserCodePtr);
                gKernel.Panic();
            }

            // On récupère une page physique libre dans laquelle on va y copier le code
            u8 * pNewCodePage = (u8 *)gPmm.GetFreePage();

            if (pNewCodePage == nullptr)
            {
                KLOG(LOG_ERROR, "Couldn't find a free page\n");
                gKernel.Panic();
            }

            // On ajoute la page physique dans l'espace d'adressage de la tâche utilisateur
            gVmm.AddPageToPageDirectory((u32)vUserCodePtr, (u32)pNewCodePage, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, process->pageDirectory);

            MemSet((u8*)vUserCodePtr, 0, PAGE_SIZE);

            // Si on a de quoi copier sur une page entière, on fait ça sinon on copie seulement le reste de code à copier
            if ((size - count) < PAGE_SIZE)
                MemCopy(codeAddr + count, vUserCodePtr, size - count);
            else
                MemCopy(codeAddr + count, vUserCodePtr, PAGE_SIZE);

            vUserCodePtr = (u8 *)((unsigned int)vUserCodePtr + (unsigned int)PAGE_SIZE);
            count += (u32)PAGE_SIZE;
        }
    }

    gVmm.RestoreMemoryMapping();

    gScheduler.AddThread(process->mainThread);
}
#include <kernel/task/Scheduler.hpp>
#include <kernel/task/ProcessManager.hpp>
#include <kernel/arch/x86/Pmm.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/lib/StdLib.hpp>

#define TEST() asm("mov $0x01, %%eax; int $0x30" ::);

static void _testUser()
{
    TEST();
    while (1);
}

void UserTest()
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;

    char * pAddr = (char *)gPmm.GetFreePage();
    char * vAddr = (char *)0x40000000;

    status = gProcessManager.CreateProcess((u32)vAddr, &process, nullptr);
    if (FAILED(status))
    {
        return;
    }

    gVmm.SaveCurrentMemoryMapping();
    gVmm.SetCurrentPageDirectory(process->pageDirectory.pdEntry);

    gVmm.AddPageToPageDirectory((u32)vAddr, (u32)pAddr, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_NON_PRIVILEGED_ACCESS, process->pageDirectory);

    MemCopy((void *)_testUser, vAddr, 512);

    gVmm.RestoreMemoryMapping();

    gScheduler.AddThread(process->mainThread);
}
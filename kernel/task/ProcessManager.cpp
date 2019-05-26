#define __PROCESS_MANAGER__
#include "ProcessManager.hpp"

#include "Common.hpp"

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("TASK", LOG_LEVEL, format, ##__VA_ARGS__)

void ProcessManager::Init()
{
    _processList = ListCreate();

    if (_processList == nullptr)
    {
        KLOG(LOG_ERROR, "CreateList() failed");

        // TODO : handle critical kernel error
        while (1);
    }
}

KeStatus ProcessManager::CreateProcess(u32 entryAddr, Process ** newProcess, Process * parent)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    Thread * mainThread = nullptr;

    if (entryAddr == 0)
    {
        KLOG(LOG_ERROR, "Invalid entryAddr parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (newProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid newProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = Process::Create(&process, parent);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Process::Create() failed with code %d", status);
        goto clean;
    }

    status = gThreadManager.CreateUserThread(entryAddr, process, &mainThread);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ThreadManager::CreateUserThread() failed with code %d", status);
        goto clean;
    }

    process->AddThread(mainThread);

    ListPush(_processList, process);

    KLOG(LOG_INFO, "Process %d created", process->pid);

    *newProcess = process;
    process = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus ProcessManager::CreateSystemProcess(Process ** newProcess)
{
    KeStatus status = STATUS_FAILURE;
    Process * process = nullptr;
    Thread * mainThread = nullptr;

    if (newProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid newProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = Process::CreateSystem(&process);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Process::Create() failed with code %d", status);
        goto clean;
    }

    // The created thread will represent the currently running kernel code, its eip register will be set automatically 
    // after the first task swithing operation by the scheduler
    status = gThreadManager.CreateKernelThread(0, process, &mainThread);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "ThreadManager::CreateKernelThread() failed with code %d", status);
        goto clean;
    }

    process->AddThread(mainThread);

    ListPush(_processList, process);

    KLOG(LOG_INFO, "System process %d created", process->pid);

    *newProcess = process;
    process = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

void ProcessManager::DeleteProcess(Process * process)
{
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return;
    }

    const int pid = process->pid;

    Process::Delete(process);

    KLOG(LOG_INFO, "Process %d deleted", pid);
}
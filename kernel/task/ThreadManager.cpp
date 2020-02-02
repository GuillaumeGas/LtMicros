#define __THREAD_MANAGER__
#include "ThreadManager.hpp"

#include "Common.hpp"
#include "Scheduler.hpp"
#include <kernel/lib/StdLib.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("TASK", LOG_LEVEL, format, ##__VA_ARGS__)

void ThreadManager::Init()
{
    // nothing to do
}

KeStatus ThreadManager::CreateUserThread(u32 entryAddr, Process * process, SecurityAttribute attribute, Thread ** thread)
{
    KeStatus status = STATUS_FAILURE;
    Thread * localThread = nullptr;

    if (entryAddr == 0)
    {
        KLOG(LOG_ERROR, "Invalid entryAddr parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = Thread::CreateThread(entryAddr, process, PVL_USER, attribute, &localThread);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Thread::CreateThread() failed with code %t", status);
        goto clean;
    }

    *thread = localThread;
    localThread = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus ThreadManager::CreateKernelThread(u32 entryAddr, Process * process, Thread ** thread)
{
    KeStatus status = STATUS_FAILURE;
    Thread * localThread = nullptr;

    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = Thread::CreateThread(entryAddr, process, PVL_KERNEL, SA_NONE, &localThread);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Thread::CreateThread() failed with code %t", status);
        goto clean;
    }

    *thread = localThread;
    localThread = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

void ThreadManager::DeleteThread(Thread * thread)
{
    // TODO
}

Thread * ThreadManager::GetCurrentThread()
{
    return gScheduler.GetCurrentThread();
}
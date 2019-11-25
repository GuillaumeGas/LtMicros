#include "SyscallsHandler.hpp"
#include "UKSyscallsCommon.h"

#include <kernel/task/ProcessManager.hpp>
#include <kernel/task/Ipc.hpp>
#include <kernel/lib/CriticalSection.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SYSCALLS", LOG_LEVEL, format, ##__VA_ARGS__)

#define SYSCALL(_, functionName) void functionName(InterruptFromUserlandContext *);
#define SYSCALL_ID(x, y) SYSCALL(x, y)
    SYSCALLS_LIST
#undef SYSCALL_ID
#undef SYSCALL

void SyscallsHandler::ExecuteSyscall(const SyscallId sysId, InterruptFromUserlandContext * context)
{
    if (sysId >= SYS_INVALID)
    {
        KLOG(LOG_ERROR, "Unknown syscall %d", sysId);
        return;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter\n");
        return;
    }

    switch (sysId) {
#define SYSCALL(id, functionName) \
        case id:                  \
            functionName(context);\
            break;  
#define SYSCALL_ID(x, y) SYSCALL(x, y)
        SYSCALLS_LIST
#undef SYSCALL_ID
#undef SYSCALL
    default:
        KLOG(LOG_ERROR, "Unknown syscall %d", sysId);
    }
}

    // Test to avoid mixing output on all processes 
    static CriticalSection s_CriticalSection;

/*  
    SYSCALLS Begin
*/
void SysPrintChar(InterruptFromUserlandContext * context)
{
    kprint("%c", context->ebx);
}

void SysPrintStr(InterruptFromUserlandContext * context)
{
    kprint("%s", context->ebx);
}

void SysSbrk(InterruptFromUserlandContext * context)
{
    KeStatus status = STATUS_FAILURE;
    Process * currentProcess = gProcessManager.GetCurrentProcess();
    u32 res = 0;

    if (currentProcess == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() returned null");
        goto clean;
    }

    status = currentProcess->IncreaseHeap(context->ebx, (u8**)&res);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Process::IncreaseHeap() failed with code %t", status);
        goto clean;
    }

clean:
    context->eax = res;
}

void SysIpcServerCreate(InterruptFromUserlandContext* context)
{
    KeStatus status = STATUS_FAILURE;
    const char * serverIdStr = (const char *)context->ebx;
    IpcHandle handle = INVALID_HANDLE_VALUE;
    Process* process = nullptr;

    process = gProcessManager.GetCurrentProcess();
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() failed !");
        goto clean;
    }

    status = gIpcHandler.AddNewServer(serverIdStr, process, &handle);
    if (FAILED(status))
    {
        KLOG(LOG_DEBUG, "IpcHandler::AddNewServerIpc() failed with code %d (Process %d)", status, process->pid);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    context->eax = status;
    context->ebx = handle;
}

void SysIpcServerConnect(InterruptFromUserlandContext* context)
{
    KeStatus status = STATUS_FAILURE;
    const char* serverIdStr = (const char*)context->ebx;
    IpcHandle handle = INVALID_HANDLE_VALUE;
    Process* process = nullptr;

    process = gProcessManager.GetCurrentProcess();
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() failed !");
        goto clean;
    }

    status = gIpcHandler.ConnectToServer(serverIdStr, process, &handle);
    if (FAILED(status))
    {
        KLOG(LOG_DEBUG, "IpcHandler::ConnectToServer() failed with code %d (Process %d)", status, process->pid);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    context->eax = status;
    context->ebx = handle;
}

void SysIpcSend(InterruptFromUserlandContext* context)
{
    KeStatus status = STATUS_FAILURE;
    IpcHandle handle = INVALID_HANDLE_VALUE;
    char* message = nullptr;
    unsigned int size = 0;
    Process* process = nullptr;

    process = gProcessManager.GetCurrentProcess();
    if (process == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() failed !");
        goto clean;
    }

    handle = (IpcHandle)context->ebx;
    message = (char*)context->ecx;
    size = (unsigned int)context->edx;

    status = gIpcHandler.Send(handle, process, message, size);
    if (FAILED(status))
    {
        KLOG(LOG_DEBUG, "IpcHandler::Send() failed with code %d (Process %d)", status, process->pid);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    context->eax = status;
}

void SysIpcReceive(InterruptFromUserlandContext* context)
{
    KeStatus status = STATUS_FAILURE;
    SysIpcReceiveParameter * parameters = nullptr;
    Process * serveurProcess = nullptr;

    serveurProcess = gProcessManager.GetCurrentProcess();
    if (serveurProcess == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() failed !");
        goto clean;
    }

    parameters = (SysIpcReceiveParameter*)context->ebx;

    status = gIpcHandler.Receive(parameters->ipcHandle, serveurProcess, parameters->message, parameters->sizePtr, parameters->clientHandlePtr);
    if (FAILED(status))
    {
        KLOG(LOG_DEBUG, "IpcHandler::Receive() failed with code %d (Process %d)", status, serveurProcess->pid);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    context->eax = status;
}

// Test to avoid mixing output on all processes 
void SysEnterScreenCriticalSection(InterruptFromUserlandContext * context)
{
    s_CriticalSection.Enter();
}

void SysLeaveScreenCriticalSection(InterruptFromUserlandContext * context)
{
    s_CriticalSection.Leave();
}

void SysInvalid(InterruptFromUserlandContext * context)
{
    KLOG(LOG_ERROR, "Invalid syscall called");
}

/*
    SYSCALLS End
*/
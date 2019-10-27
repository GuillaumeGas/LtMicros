#include "SyscallsHandler.hpp"

#include <kernel/task/ProcessManager.hpp>

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

    KLOG(LOG_DEBUG, "process : %x", currentProcess);

    if (currentProcess == nullptr)
    {
        KLOG(LOG_ERROR, "GetCurrentProcess() returned null");
        goto clean;
    }

    status = currentProcess->IncreaseHeap(context->ebx, &res);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "Process::IncreaseHeap() failed with code %d", status);
        goto clean;
    }

    KLOG(LOG_DEBUG, "res : %x", res);   

clean:
    context->eax = res;
}

void SysInvalid(InterruptFromUserlandContext * context)
{
    KLOG(LOG_ERROR, "Invalid syscall called");
}

/*
    SYSCALLS End
*/
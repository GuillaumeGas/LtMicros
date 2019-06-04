#include "SyscallsHandler.hpp"

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SYSCALLS", LOG_LEVEL, format, ##__VA_ARGS__)

#define SYSCALL(_, functionName) void functionName(const InterruptFromUserlandContext *);
#define SYSCALL_ID(x, y) SYSCALL(x, y)
    SYSCALLS_LIST
#undef SYSCALL_ID
#undef SYSCALL

void SyscallsHandler::ExecuteSyscall(const SyscallId sysId, const InterruptFromUserlandContext * context)
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
void SysPrintChar(const InterruptFromUserlandContext * context)
{
    kprint("%c", context->ebx);
}

void SysPrintStr(const InterruptFromUserlandContext * context)
{
    kprint("%s", context->ebx);
}

void SysInvalid(const InterruptFromUserlandContext * context)
{
    KLOG(LOG_ERROR, "Invalid syscall called");
}
/*
    SYSCALLS End
*/
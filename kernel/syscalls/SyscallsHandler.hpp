#pragma once

#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/lib/StdIo.hpp>

#include "SyscallsList.hpp"

enum SyscallId 
{
#define SYSCALL(name, _) name,
#define SYSCALL_ID(x, y) SYSCALL(x, y)
    SYSCALLS_LIST
#undef SYSCALL
#undef SYSCALL_ID
};

void SysPrint(const InterruptFromUserlandContext * context);
void SysInvalid(const InterruptFromUserlandContext * context);

class SyscallsHandler
{
public:
    static void ExecuteSyscall(const SyscallId sysId, const InterruptFromUserlandContext * context);
};

#define __SYSCALLS_X86__
#include "Syscalls.hpp"

#include <kernel/arch/x86/Idt.hpp>
#include <kernel/arch/x86/InterruptContext.hpp>

#include <kernel/syscalls/SyscallsHandler.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SYSCALLSX86", LOG_LEVEL, format, ##__VA_ARGS__)

extern "C" void _asm_syscall_isr(void);

extern "C" void syscall_isr(InterruptFromUserlandContext * context)
{
    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return;
    }

    SyscallsHandler::ExecuteSyscall((SyscallId)context->eax, context);
}

void SyscallsX86::Init()
{
    gIdt.InitDescriptor((u32)(_asm_syscall_isr), SYSCALL_GATE, 48);
    gIdt.Reload();
}

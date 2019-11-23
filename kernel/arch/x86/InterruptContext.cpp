#include "InterruptContext.hpp"

#include <kernel/lib/Types.hpp>
#include <kernel/lib/StdIo.hpp>

void PrintInterruptContext(InterruptContext * context)
{
    kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
    kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
    kprint("eip = %x\n", context->eip);
    kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds);
    kprint("cs : %x\n", context->cs);
    kprint("eflags : %x\n", context->eflags);
}

void PrintExceptionContext(ExceptionContext * context)
{
    u16 cs = 0;

    asm("mov %%cs, %%ax; mov %%ax, %0" : "=m" (cs) : );

    kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
    kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
    kprint("eip = %x\n", context->eip);
    kprint("gs = %x, fs = %x, es = %x, ds = %x\n", context->gs, context->fs, context->es, context->ds, cs);
    kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
    kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
    kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
    kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
}

void PrintExceptionContextWithCode(ExceptionContextWithCode * context)
{
    u16 cs = 0;

    asm("mov %%cs, %%ax; mov %%ax, %0" : "=m" (cs) : );

    kprint("eax = %x, ebx = %x, ecx = %x, edx = %x\n", context->eax, context->ebx, context->ecx, context->edx);
    kprint("esp = %x, ebp = %x, esi = %x, edi = %x\n", context->esp, context->ebp, context->esi, context->edi);
    kprint("eip = %x\n", context->eip);
    kprint("gs = %x, fs = %x, es = %x, ds = %x, cs = %x\n", (u16)context->gs, (u16)context->fs, (u16)context->es, (u16)context->ds, cs);
    kprint("eflags = %x (%b)\n", context->eflags, context->eflags);
    kprint("cr0 = %x (%b)\n", context->cr0, context->cr0);
    kprint("cr2 = %x (%b)\n", context->cr2, context->cr2);
    kprint("cr3 = %x (%b)\n", context->cr3, context->cr3);
    kprint("Error code : %x (%b)\n", context->code, context->code);
}

void PrintExceptionUserContextWithCode(ExceptionContextUserWithCode * context)
{
    ExceptionContextWithCode * baseContext = (ExceptionContextWithCode *)context;
    PrintExceptionContextWithCode(baseContext);

    kprint("espU : %x (%b*)\n", context->espU, context->espU, 32);
    kprint("csU : %x, ssU : %x\n", context->csU, context->ssU);
}
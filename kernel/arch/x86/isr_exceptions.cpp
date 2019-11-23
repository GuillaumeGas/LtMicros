#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/drivers/Screen.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/StdIo.hpp>
#include <kernel/task/ProcessManager.hpp>

#include "Exceptions/PageFault.h"

#define EXCEPTION_SCREEN               \
	ScreenDriver::Clear();             \
    ScreenDriver::SetBackground(BLUE); \

static void DefaultExceptionHandler(ExceptionContext * context, const char * str);

extern "C" void default_isr(ExceptionContext * context)
{
    //DefaultExceptionHandler(context, "[!] Unhandled interrupt !");
}

extern "C" void divided_by_zero_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Divided by zero");
}

extern "C" void debug_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault/Trap] Debug");
}

extern "C" void non_maskable_int_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Interrupt] Non-maskable Interrupt");
}

extern "C" void breakpoint_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Trap] Breakpoint");
}

extern "C" void overflow_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Trap] Overflow");
}

extern "C" void bound_range_exceeded_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Bound Range Exceeded");
}

extern "C" void invalid_opcode_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Invalid Opcode");
}

extern "C" void device_not_available_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Device not available");
}

extern "C" void double_fault_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Abort] Double fault");
}

extern "C" void invalid_tss_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Invalid TSS");
}

extern "C" void segment_not_present_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Segment not present");
}

extern "C" void stack_segment_fault_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Stack-Segment fault");
}

extern "C" void general_protection_fault_isr(ExceptionContextWithCode * context)
{
    u32 code = context->code;
    // https://wiki.osdev.org/Exceptions#Selector_Error_Code
    u8 E = (code & 1);
    u8 Tlb = ((code >> 1) & 3);
    u8 Index = (code >> 3);

    EXCEPTION_SCREEN

        ScreenDriver::SetColorEx(BLUE, RED, 0, 1);
    kprint(">> [Fault] General protection fault ! \n\n");

    ScreenDriver::SetColorEx(BLUE, WHITE, 0, 1);

    kprint("Selector Error Code : %x (%b)\n", code, code);

    kprint("E = %d, ", E);
    if (E == 1)
        kprint("The exception originated externally to the processor.\n");
    else
        kprint("\n");

    kprint("Tlb : %b, ", Tlb);
    if (Tlb == 0)
        kprint("The Selector Index references a descriptor in the GDT.\n");
    if (Tlb == 1)
        kprint("The Selector Index references a descriptor in the IDT.\n");
    if (Tlb == 2)
        kprint("The Selected Index references a descriptor in the LDT.\n");
    if (Tlb == 3)
        kprint("The Selected Index references a descriptor in the IDT.\n");

    kprint("Selector index : %x\n\n", Index);

    // coming from kernel
    if (context->cr3 == 0x1000)
    {
        PrintExceptionContextWithCode(context);
    }
    else
    {
        PrintExceptionUserContextWithCode((ExceptionContextUserWithCode *)context);
    }

    Pause();
}

extern "C" void page_fault_isr(ExceptionContextWithCode * context)
{
    PageFaultExceptionHandler(context);
}

extern "C" void x87_floating_point_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] x87 Floating-point exception");
}

extern "C" void alignment_check_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Alignment check");
}

extern "C" void machine_check_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Abort] Machine check");
}

extern "C" void simd_floating_point_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] SIMD Floating-point exception");
}

extern "C" void virtualization_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[Fault] Virtualization exception");
}

extern "C" void security_isr(ExceptionContext * context)
{
    DefaultExceptionHandler(context, "[!] Security exception");
}

static void DefaultExceptionHandler(ExceptionContext * context, const char * str)
{
    EXCEPTION_SCREEN

    ScreenDriver::SetColorEx(BLUE, RED, 0, 1);
    kprint(str);
    kprint("\n\n");
    ScreenDriver::SetColorEx(BLUE, WHITE, 0, 1);

    PrintExceptionContext(context);

    Pause();
}
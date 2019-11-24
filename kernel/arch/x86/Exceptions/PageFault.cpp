#include <kernel/task/ProcessManager.hpp>
#include <kernel/debug/LtDbg.hpp>
#include <kernel/arch/x86/Process.hpp>
#include <kernel/lib/StdIo.hpp>
#include <kernel/drivers/Screen.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("PAGE FAULT", LOG_LEVEL, format, ##__VA_ARGS__)

#define EXCEPTION_SCREEN               \
	ScreenDriver::Clear();             \
    ScreenDriver::SetBackground(BLUE); \

// p detail parameter
#define PROTECTION_VIOLATION_VALUE 1
#define NON_PRESENT_PAGE           0

// wr detail parameter
#define WRITE_ACCESS 1
#define READ_ACCESS  0

// us detail parameter
#define USER_MODE   1
#define KERNEL_MODE 0

struct PageFaultDetails
{
    u8 p;
    u8 wr;
    u8 us;
    u8 rsvd;
    u8 id;
};

static void PrintPageFaultException(ExceptionContextWithCode* context, PageFaultDetails* const details);

void PageFaultExceptionHandler(ExceptionContextWithCode* const context)
{
    u32 code = context->code;
    PageFaultDetails details = { (u8)(code & 1), (u8)(code & 2), (u8)(code & 4), (u8)(code & 8), (u8)(code & 16) };
    Process* process = gProcessManager.GetCurrentProcess();

    if (details.p == NON_PRESENT_PAGE)
    {
        KeStatus status = STATUS_FAILURE;

        status = process->ResolvePageFault((void*)context->cr2);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Process::ResolvePageFault() failed with code %d", status);

            if (gLtDbg.IsConnected())
            {
                // gLtDbg.BreakOnError(context);
            }
            else
            {
                PrintPageFaultException(context, &details);
                Pause();
            }
        }
    }
    else
    {
        PrintPageFaultException(context, &details);

        if (process != nullptr)
        {
            kprint("Process %d\n", process->pid);
        }

        Pause();
    }
}

static void PrintPageFaultException(ExceptionContextWithCode* context, PageFaultDetails * const details)
{

    EXCEPTION_SCREEN

        ScreenDriver::SetColorEx(BLUE, RED, 0, 1);
    kprint(">> [Fault] Page fault ! \n\n");

    ScreenDriver::SetColorEx(BLUE, WHITE, 0, 1);

    kprint("Error code : %x (%b)\n", context->code, context->code);
    kprint(" - P : %d (%s)\n", details->p ? PROTECTION_VIOLATION_VALUE : NON_PRESENT_PAGE, details->p ? "protection violation" : "non-present page");
    kprint(" - W/R : %d (%s)\n", details->wr ? WRITE_ACCESS : READ_ACCESS, details->wr ? "write access" : "read access");
    kprint(" - U/S : %d (%s)\n", details->us ? USER_MODE : KERNEL_MODE, details->us ? "user mode" : "supervisor mode");
    kprint(" - RSVD : %d (%s)\n", details->rsvd ? 1 : 0, details->rsvd ? "one or more page directory entries contain reserved bits which are set to 1" : "PSE or PAE flags in CR4 are set to 1");
    kprint(" - I/D : %d (%s)\n\n", details->id ? 1 : 0, details->id ? "instruction fetch (applies when the No-Execute bit is supported and enabled" : "-");
    kprint("Linear address : %x, %b*\n\n", context->cr2, context->cr2, 32);

    if (details->us == 1)
    {
        PrintExceptionContextWithCode(context);
    }
    else
    {
        PrintExceptionUserContextWithCode((ExceptionContextUserWithCode*)context);
    }
}
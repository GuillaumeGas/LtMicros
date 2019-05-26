#define __CLOCK_DRIVER__
#include "Clock.hpp"

#include <kernel/arch/x86/Idt.hpp>
#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/lib/StdIo.hpp>
#include <kernel/drivers/proc_io.hpp>
#include <kernel/task/Scheduler.hpp>

/// @addgroup DriversGroup
/// @{

extern "C" void _asm_clock_isr(void);

// #define CLOCK_DEBUG
#define TICKS_PER_SECOND 1000
#define BASE_FREQUENCY   1193182

#define PIT_PORT         0x43
#define TIMER0_CHANNEL   0x40
#define WRITE_WORD       0x30
#define MODE_SQUARE_WAVE 0x06

extern "C" void clock_isr(InterruptContext * context)
{
    gClockDrv.tics++;
    if (gClockDrv.tics >= TICKS_PER_SECOND)
    {
        gClockDrv.secs++;
        gClockDrv.tics = 0;

#ifdef CLOCK_DEBUG
        if (gClockDrv.secs % 2 == 0)
        {
            kprint(".");
        }
#endif
    }

    gScheduler.Schedules(context);
}

ClockDriver::ClockDriver()
{
    tics = 0;
    secs = 0;
}

void ClockDriver::Init()
{
    // https://wiki.osdev.org/Programmable_Interval_Timer
    //const u8 TIMER_RELOAD = (BASE_FREQUENCY / TICKS_PER_SECOND);

    //outb(PIT_PORT, WRITE_WORD | MODE_SQUARE_WAVE);
    //outb(TIMER0_CHANNEL, (TIMER_RELOAD >> 8) & 0xFF);
    //outb(TIMER0_CHANNEL, TIMER_RELOAD & 0xFF);

    gIdt.InitDescriptor((u32)_asm_clock_isr, CPU_GATE, 32);
    gIdt.Reload();

    isInitialized = true;
}

/// @}
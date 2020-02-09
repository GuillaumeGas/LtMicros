#include "SchedulerX86.hpp"
#include "InterruptContext.hpp"

#include <kernel/task/Scheduler.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("TEST", LOG_LEVEL, format, ##__VA_ARGS__)

extern "C" void ContextSwitchIsr(InterruptContext * context)
{
    gScheduler.Schedules(context);
}
#include "Event.hpp"

#include <kernel/task/Scheduler.hpp>
#include <kernel/task/ThreadManager.hpp>

#include <kernel/Logger.hpp>

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("EVENT", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("EVENT", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

Event EventCreate()
{
    return Event();
}

void EventWait(Event * evt)
{
    if (evt == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid evt parameter");
        return;
    }

    evt->criticalSection.Enter();

    if (evt->signaled == false)
    {
        evt->thread = gThreadManager.GetCurrentThread();
        evt->thread->state = THREAD_STATE_WAITING;
        evt->criticalSection.Leave();

        gScheduler.ContextSwitchInterrupt();
    }

    evt->signaled = false;
    evt->criticalSection.Leave();
}

void EventSignal(Event * evt)
{
    if (evt == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid evt parameter");
        return;
    }

    evt->criticalSection.Enter();
    evt->signaled = true;

    if (evt->thread != nullptr)
    {
        evt->thread->state = THREAD_STATE_RUNNING;
    }

    evt->criticalSection.Leave();
}
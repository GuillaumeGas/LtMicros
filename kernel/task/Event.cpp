#include "Event.hpp"

#include <kernel/task/Scheduler.hpp>
#include <kernel/task/ThreadManager.hpp>

Event EventCreate()
{
    return { nullptr };
}

void EventWait(Event evt)
{
    evt.thread = gThreadManager.GetCurrentThread();
    evt.thread->state = THREAD_STATE_PAUSED;
    gScheduler.SchedulesFromRunningThread();
}

void EventSignal(Event evt)
{
    if (evt.thread != nullptr)
        evt.thread->state = THREAD_STATE_RUNNING;
    // TODO : switch directly to the thread signaled thread ?
}
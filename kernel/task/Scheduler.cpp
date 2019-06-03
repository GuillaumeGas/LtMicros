#define __SCHEDULER__
#include "Scheduler.hpp"

#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/debug/LtDbg.hpp>
#include <kernel/Kernel.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SCHEDULER", LOG_LEVEL, format, ##__VA_ARGS__)

void Scheduler::Init()
{
    _threadsList = ListCreate();
    if (_threadsList == nullptr)
    {
        KLOG(LOG_ERROR, "ListCreate() failed");
        gKernel.Panic();
    }

    _running = false;
    _currentThread = nullptr;
    _nbThreads = 0;
}

void Scheduler::AddThread(Thread * thread)
{
    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return;
    }

    // The system process main thread must be the first to be added to the scheduler
    if (_currentThread == nullptr)
        _currentThread = thread;

    //__debugbreak();

    ListPush(_threadsList, thread);
    _nbThreads++;
}

void Scheduler::Start()
{
    _running = true;
}

void Scheduler::Stop()
{
    _running = false;
}

void Scheduler::Schedules(InterruptContext * context)
{
    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        gKernel.Panic();
    }

    if (_running && _nbThreads > 1)
    {
        Thread * nextThread = _PickNextThread();

        if (nextThread != nullptr && nextThread != _currentThread)
        {
            _SwitchToThread(context, nextThread);
            _currentThread = nextThread;
        }
    }
}

Thread * Scheduler::_PickNextThread()
{
    Thread * thread = nullptr;

    // We didn't start any thread yet
    if (_currentThread == nullptr)
    {
        thread = (Thread *)ListTop(_threadsList);
    }
    else
    {
        // TODO : we could be better, with a better list library
        ListElem * elem = (ListElem *)_threadsList;
        while (elem != nullptr)
        {
            // If we are on the current thread node, we retrieve the next one
            if ((Thread *)elem->data == _currentThread)
            {
                if (elem->next != nullptr)
                {
                    thread = (Thread *)elem->next->data;
                }
                else
                {
                    // If there is no next one, we retrieve the top of the list
                    thread = (Thread *)ListTop(_threadsList);
                }

                break;
            }

            elem = elem->next;
        }
    }

    return thread;
}

void Scheduler::_SwitchToThread(InterruptContext * context, Thread * thread)
{
    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        gKernel.Panic();
    }

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        gKernel.Panic();
    }

    _currentThread->SaveState(context);
    _currentThread->state = THREAD_STATE_PAUSED;

    _currentThread = thread;
    _currentThread->StartOrResume();
}
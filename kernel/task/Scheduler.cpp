#define __SCHEDULER__
#include "Scheduler.hpp"

#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/debug/LtDbg.hpp>
#include <kernel/Kernel.hpp>
#include <kernel/drivers/Clock.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("SCHEDULER", LOG_LEVEL, format, ##__VA_ARGS__)

#define DEFAULT_THREAD_LIMIT_WORKING_TIME 100

struct LOOK_FOR_THREAD_CONTEXT
{
    bool found;
    Thread* foundThread;
    Thread* thread;
};

static KeStatus LookForThreadWithHigherPriorityCallback(void* data, void* context);

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
        // If the current thread worked enough or if another thread has a higher priority
        if (_currentThread == nullptr
            || (gClockDrv.tics - _currentThread->ticsOnResume > DEFAULT_THREAD_LIMIT_WORKING_TIME) 
            || HasHigherThreadPriority(_currentThread))
        {
            bool found = false;
            Thread* foundThread = nullptr;
            do
            {
                Thread* nextThread = _PickNextThread(_currentThread);

                if (nextThread == nullptr && nextThread == _currentThread)
                {
                    found = true;
                }
                else
                {
                    foundThread = nextThread;

                    if (foundThread->threadPriority >= _currentThread->threadPriority)
                    {
                        found = true;
                    }
                }
            } while (!found);

            if (found && _currentThread != foundThread)
            {
                _SwitchToThread(context, foundThread);
            }
        }
    }
}

Process * Scheduler::GetCurrentProcess()
{
    if (_currentThread == nullptr)
    {
        KLOG(LOG_ERROR, "_currentThread is null");
        return nullptr;
    }

    return _currentThread->process;
}

Thread * Scheduler::GetCurrentThread()
{
    if (_currentThread == nullptr)
    {
        KLOG(LOG_ERROR, "_currentThread is null");
        return nullptr;
    }

    return _currentThread;
}

Thread * Scheduler::_PickNextThread(Thread * currentThread)
{
    Thread * thread = nullptr;

    // We didn't start any thread yet
    if (currentThread == nullptr)
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
            if ((Thread *)elem->data == currentThread)
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

bool Scheduler::HasHigherThreadPriority(Thread* thread)
{
    LOOK_FOR_THREAD_CONTEXT Context;

    if (thread == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid thread parameter");
        return false;
    }

    Context.thread = thread;
    Context.found = false;
    Context.foundThread = nullptr;

    ListEnumerate(_threadsList, LookForThreadWithHigherPriorityCallback, &Context);

    return Context.found;
}

static KeStatus LookForThreadWithHigherPriorityCallback(void* data, void* context)
{
    LOOK_FOR_THREAD_CONTEXT* foundContext = (LOOK_FOR_THREAD_CONTEXT*)context;
    Thread* thread = (Thread*)data;

    if (data == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid data parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (thread->threadPriority > foundContext->thread->threadPriority)
    {
        foundContext->found = true;
        foundContext->foundThread = thread;

        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}
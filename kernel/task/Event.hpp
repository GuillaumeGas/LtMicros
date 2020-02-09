#pragma once

#include <kernel/arch/x86/Thread.hpp>
#include <kernel/lib/CriticalSection.hpp>

struct Event
{
    Event() : signaled(false), thread(nullptr) {}

    bool signaled;
    Thread * thread;
    CriticalSection criticalSection;
};

Event EventCreate();
void EventWait(Event * evt);
void EventSignal(Event * evt);
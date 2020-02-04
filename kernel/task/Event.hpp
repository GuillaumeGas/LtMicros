#pragma once

#include <kernel/arch/x86/Thread.hpp>

struct Event
{
    Thread * thread;
};

Event EventCreate();
void EventWait(Event evt);
void EventSignal(Event evt);
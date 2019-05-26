#include "CriticalSection.hpp"
#include "AtomicOperation.hpp"
#include "StdIo.hpp"

CriticalSection::CriticalSection() : _value(0) {}

void CriticalSection::Enter()
{
    _acquire_lock(&_value);
}

void CriticalSection::Leave()
{
    _release_lock(&_value);
}
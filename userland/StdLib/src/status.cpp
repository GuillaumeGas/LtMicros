#include "status.h"

const char* StatusGetStringFromInt(Status status)
{
    switch (status)
    {
#define STATUS_ELEM(s) case s: return #s; break;
        STATUS_LIST
#undef STATUS_ELEM
    default:
        return UNKNOWN_STATUS;
    }
}
#pragma once

/// @file

#include "BaseDriver.hpp"

#include <kernel/lib/Types.hpp>

/// @defgroup DriversGroup Drivers group
/// @{

/// @brief A simply clock driver, mainly used by the scheduler
class ClockDriver : public BaseDriver
{
public:
    /// @brief Default Clock class constructor, do nothing
    ClockDriver();

    /// @brief Initializes the clock driver, by setting up the right IDT entry
    void Init();

    /// @brief Incremented at each cpu clock interrupt
    u32 tics;
    u32 secs;
};

#ifdef __CLOCK_DRIVER__
ClockDriver gClockDrv;
#else
extern ClockDriver gClockDrv;
#endif

/// @}
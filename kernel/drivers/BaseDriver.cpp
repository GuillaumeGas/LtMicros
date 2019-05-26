#include "BaseDriver.hpp"

/// @addgroup DriversGroup Drivers group
/// @{

BaseDriver::BaseDriver()
{
    isInitialized = false;
}

bool BaseDriver::IsInitialized() const
{
    return isInitialized;
}

/// @}
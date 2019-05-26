#pragma once

#include <kernel/lib/StdLib.hpp>

/// @defgroup DriversGroup Drivers group
/// @{

/// @brief Base class/interface of a driver. All drivers that inherits this class must set isInitialized in Init()
class BaseDriver
{
public:
    /// @brief Default DriverBase class constructor, do nothing
    BaseDriver();

    /// @brief This function must be implemented in drivers that inherits this class to initialize themself
    virtual void Init() {}

    /// @brief Indicates if the driver has been initialized
    /// @return true if the driver is initialized
    bool IsInitialized() const;

protected:
    bool isInitialized;
};

/// @}
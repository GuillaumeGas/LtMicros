#pragma once

/* @file */

#include <kernel/lib/stdarg.h>
#include <kernel/lib/CriticalSection.hpp>

/// Used to generate a message including the file name and line number
#define KLOGGER(kmodule, level, format, ...) gLogger.KernelLogger(__FILE__, __LINE__, kmodule, level, format, ##__VA_ARGS__)

/// @brief Used to determine if the log must be printed on the screen, send on serial port or both
typedef int LoggerMode;

/// @brief The message is printed on screen
#define LOG_SCREEN 1
/// @brief The message is sent to serial port COM1. The serial driver must be initialized before using this flag !
#define LOG_SERIAL 2

/// @brief A log may be printed with a different color depending on its level
enum LogLevel 
{ 
    /// @brief The message is printed in white, without any modification
    LOG_INFO, 
    /// @brief The message is printed in red, and includes the file name and line number
    LOG_ERROR, 
    /// @brief The message is printed in orange, and includes the file name and line number
    LOG_WARNING, 
    /// @brief The message is printed in blue, and includes the file name and line number
    LOG_DEBUG 
};

/// @brief The logger is used to display easily messages on screen or send them on serial port, mainly used for debug purpose
class Logger
{
public:
    /// @brief Default class constructor, set the default log type to LOG_SCREEN
    Logger();

    /// @brief Calls the _KernelLogger private function
    void KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...);

    /// @brief Sets the current logger mode
    void SetMode(const LoggerMode logMode);

    /// @brief Stores the logger mode : LOG_SCREEN and/or LOG_SERIAL
    LoggerMode mode;

private:
    /// @brief Generates the message depending on the parameters
    /// @param[in] fileName A pointer on a nullptr terminated string containing the source file name in which this function is called
    /// @param[in] lineNumber The line number of this function call
    /// @param[in] moduleName A pointer on a nullptr terminated string containing the module name
    /// @param[in] level The log level
    /// @param[in] format A pointer on a nullptr terminated string containing a valid kprint format
    void _KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, va_list args);

    CriticalSection _criticalSection;
};

#ifdef __LOGGER__
Logger gLogger;
#else
extern Logger gLogger;
#endif
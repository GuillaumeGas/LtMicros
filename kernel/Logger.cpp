#define __LOGGER__
#include "Logger.hpp"

#include <kernel/drivers/Screen.hpp>
#include <kernel/drivers/Clock.hpp>
#include <kernel/lib/StdIo.hpp>
#include <kernel/lib/StdLib.hpp>

Logger::Logger()
{
    // Default mode, because buring kernel startup the serial port may be not
    // initialized yet
    mode = LOG_SCREEN;
}

void Logger::KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    _KernelLogger(fileName, lineNumber, moduleName, level, format, args);
    va_end(args);
}

void Logger::SetMode(const LoggerMode mode)
{
    this->mode = mode;

    kprint("[LOGGER] Setting logger mode to");
    if (FlagOn(this->mode, LOG_SCREEN))
        kprint(" LOG_SCREEN");
    if (FlagOn(this->mode, LOG_SERIAL))
        kprint(" LOG_SERIAL");
    kprint("\n");
}

void Logger::_KernelLogger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, va_list args)
{
#ifdef DEBUG_PRINT
    u32 clockTics = gClockDrv.tics;

    _criticalSection.Enter();

    switch (level)
    {
    case LOG_INFO:
        break;
    case LOG_ERROR:
        ScreenDriver::SetColor(RED);
        break;
    case LOG_WARNING:
        ScreenDriver::SetColor(MAGENTA);
        break;
    case LOG_DEBUG:
        ScreenDriver::SetColor(CYAN);
        break;
    }

    if (level != LOG_INFO)
    {
        kprint("(%d) [%s] %s (%d) : ", clockTics, moduleName, fileName, lineNumber);
    }
    else
    {
        kprint("(%d) [%s] ", clockTics, moduleName);
    }

    kprintEx(format, args);
    kprint("\n");

    ScreenDriver::SetColor(WHITE);

    _criticalSection.Leave();

#endif
}
#include "logger.h"

#include "stdio.h"
#include "stdarg.h"

static void _logger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, va_list args);

void logger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    _logger(fileName, lineNumber, moduleName, level, format, args);
    va_end(args);
}

static void _logger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, va_list args)
{
    printf("[%s]", moduleName);

    switch (level)
    {
    case LOG_INFO:
        break;
    case LOG_ERROR:
        printf("[ERROR]");
        break;
    case LOG_WARNING:
        printf("[WARNING]");
        break;
    case LOG_DEBUG:
        printf("[DBG]");
        break;
    }

    if (level != LOG_INFO)
    {
        printf("%s:%d : ", fileName, lineNumber);
    }
    else
    {
        printf(" ");
    }

    printfEx(format, args);
    printf("\n");
}
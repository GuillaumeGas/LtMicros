#pragma once

#define LOGGER(umodule, level, format, ...) logger(__FILE__, __LINE__, umodule, level, format, ##__VA_ARGS__)

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

void logger(const char * fileName, const int lineNumber, const char * moduleName, const LogLevel level, const char * format, ...);
#pragma once

#include <logger.h>
#define LOG(LOG_LEVEL, format, ...) LOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)
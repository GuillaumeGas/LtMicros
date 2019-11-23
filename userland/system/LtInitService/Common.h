#pragma once

#include <logger.h>
#define LOG(LOG_LEVEL, format, ...) LOGGER("INIT", LOG_LEVEL, format, ##__VA_ARGS__)
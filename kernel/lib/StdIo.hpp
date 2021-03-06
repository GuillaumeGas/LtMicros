#pragma once

#include "stdarg.h"
#include "Types.hpp"

/// @defgroup KernelLibGroup Kernel lib group
/// @{

/// @brief Print function used to display text on screen using a format
///        Format : %d, %x, %p, %b, %s, %c, %b
///          example : kprint("value : %b", value); // display a 8bits value
///                    kprint("value : %b*", value, 32); // display a 32 value
/// @param[in] format A valid string/format
void kprint(const char * format, ...);

/// @brief Print function used to display text on screen using a format
///        Format : %d, %x, %p, %b, %s, %c
///        %b example : kprint("value : %b", value); // display a 8bits value
///                     kprint("value : %b*", value, 32); // display a 32 value
/// @param[in] format A valid string/format
/// @param[in] ap A va_list
void kprintEx(const char * format, va_list ap);

/// @}
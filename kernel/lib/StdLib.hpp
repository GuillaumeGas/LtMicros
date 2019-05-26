#pragma once

#include "stdarg.h"
#include "Types.hpp"
#include "StdMem.hpp"
#include "Status.hpp"

/// @defgroup KernelLibGroup Kernel lib group
/// @{

/// Used to determine if a one or more bits are set in a flag
#define FlagOn(a, b) ((a & b) != 0)

/// Used to avoid warning when parameters are not used
#define UNREFERENCED_PARAMETER(param) param=param

/// @brief Pauses the system (simple infinite loop...)
void Pause();

/// @brief Halt the system
void Halt();

/// @brief Copies a string into another
/// @param[in] src A pointer to the source string, must be nullptr terminated
/// @param[in] dst A pointer to the destination buffer
void StrCpy(const char * src, char * dst);

/// @brief Calculates a string length
/// @param[in] str A pointer to a nullptr terminated string
/// @return The string length in bytes
unsigned long StrLen(const char * str);

/// @brief Do a binary comparison between two strings
/// @param[in] str1 The first nullptr terminated string
/// @param[in] str2 The second nullptr terminated string
/// @return 0 if equal, -1 if str1 < str2, else 1
int StrCmp(const char * src, const char * dst);

/// @brief Looks for the first index of a given char in a string
/// @param[in] str A nullptr terminated string
/// @param[in] c The char we are looking for
/// @return A index >= 0 if c if found, -1 if not found, -2 if str is nullptr
int FirstIndexOf(const char * str, char const c);

/// @brief Modulo operation
/// @param[in] x Dividend value
/// @param[in] y Divisor value
/// @return x mod y
extern "C" int _mod(unsigned int x, unsigned int y);

/* @} */
#pragma once

#include "malloc.h"
#include "FileSystem.h"
#include "types.h"

#define __debugbreak() asm("int $3")

#define FlagOn(a, b) (((a) & (b)) != 0)

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

/// @brief Copies memory from a source to a destination
/// @param[in] src A pointer to the source
/// @param[in] dst A pointer to the destination
/// @param[in] size The length in bytes
void MemCopy(void * src, void * dst, unsigned int size);

/// @brief Sets a chunck of memory with a given byte
/// @param[in] src A pointer to the data
/// @param[in] byte A byte used to fill the data with
/// @param[in] size The length in bytes
void MemSet(void * src, u8 byte, unsigned int size);

// TODO : put that somewhere else
void RaiseThreadPriority();
void LowerThreadPriority();

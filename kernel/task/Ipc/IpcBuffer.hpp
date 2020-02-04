#pragma once

#include <kernel/lib/Status.hpp>

/// @file

/// @addgroup TaskGroup
/// @{

class IpcBuffer
{
public:
    IpcBuffer();

    KeStatus AddBytes(const char* message, const unsigned int size);
    KeStatus ReadBytes(char* const buffer, const unsigned int size, unsigned int* const bytesRead);

private:

};

/// @}
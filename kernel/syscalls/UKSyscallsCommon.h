#pragma once

typedef unsigned int Handle;

/// @file

/// @addgroup Syscalls
/// @{

/// @brief This file may be include by user AND kernel code

struct SysIpcReceiveParameter
{
    unsigned int ipcHandle;
    char * buffer;
    unsigned int size;
    unsigned int * readBytesPtr;
    Handle * clientProcessHandlePtr;
};

/// @}
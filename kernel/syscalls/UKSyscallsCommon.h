#pragma once

/// @file

/// @addgroup Syscalls
/// @{

/// @brief This file may be include by user AND kernel code

struct SysIpcReceiveParameter
{
    unsigned int ipcHandle;
    char** message;
    unsigned int* sizePtr;
    unsigned int * clientHandlePtr;
};

/// @}
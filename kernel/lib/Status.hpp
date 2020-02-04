#pragma once

/// @file
/// @addgroup KernelLibGroup Kernel lib group
/// @{

#define FAILED(status) (status != STATUS_SUCCESS)

#define UNKNOWN_STATUS "UNKOWN_STATUS"

#define STATUS_LIST                                   \
    STATUS_ELEM (STATUS_SUCCESS)                      \
    STATUS_ELEM (STATUS_FAILURE)                      \
    STATUS_ELEM (STATUS_NOT_FOUND)                    \
    STATUS_ELEM (STATUS_INVALID_PARAMETER)            \
    STATUS_ELEM (STATUS_NULL_PARAMETER)               \
    STATUS_ELEM (STATUS_INAVLID_HANDLE)               \
    STATUS_ELEM (STATUS_ALLOC_FAILED)                 \
    STATUS_ELEM (STATUS_INVALID_VIRTUAL_USER_ADDRESS) \
    STATUS_ELEM (STATUS_PHYSICAL_MEMORY_FULL)         \
    STATUS_ELEM (STATUS_PROCESS_HEAP_LIMIT_REACHED)   \
    STATUS_ELEM (IPC_STATUS_ID_STRING_ALREADY_USED)   \
    STATUS_ELEM (IPC_STATUS_SERVER_NOT_FOUND)         \
    STATUS_ELEM (IPC_STATUS_ACCESS_DENIED)            \
    STATUS_ELEM (STATUS_HANDLE_ALREADY_EXIST)         \
    STATUS_ELEM (STATUS_LIST_STOP_ITERATING)          \
    STATUS_ELEM (STATUS_UNEXPECTED)                   \

enum KeStatus
{
#define STATUS_ELEM(status) status,
    STATUS_LIST
#undef STATUS_ELEM
};

const char* StatusGetStringFromInt(KeStatus status);

/// @}
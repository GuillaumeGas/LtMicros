#pragma once

#define FAILED(status) (status != STATUS_SUCCESS)

#define UNKNOWN_STATUS "UNKOWN_STATUS"

#define STATUS_LIST                                   \
    STATUS_ELEM (STATUS_SUCCESS)                      \
    STATUS_ELEM (STATUS_FAILURE)                      \
    STATUS_ELEM (STATUS_NULL_PARAMETER)               \
    STATUS_ELEM (STATUS_INVALID_PARAMETER)            \
    STATUS_ELEM (STATUS_ALLOC_FAILED)                 \
    STATUS_ELEM (STATUS_FILE_NOT_FOUND)               \
    STATUS_ELEM (STATUS_NOT_A_DIRECTORY)              \
    STATUS_ELEM (STATUS_UNEXPECTED)                   \
    STATUS_ELEM (PATH_TOO_LONG)                       \

enum Status
{
#define STATUS_ELEM(status) status,
    STATUS_LIST
#undef STATUS_ELEM
};

const char* StatusGetStringFromInt(Status status);
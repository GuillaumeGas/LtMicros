#pragma once

/// @file

/// @defgroup TaskGroup Task group
/// @{

/// @brief Used to indicate the thread privilege level of a thread
enum PrivilegeLevel
{
    PVL_KERNEL = 0,
    PVL_USER
};

enum SecurityAttribute
{
    SA_NONE = 0,
    SA_IO,
};

/// @}
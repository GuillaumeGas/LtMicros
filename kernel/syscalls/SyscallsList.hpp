#pragma once

/// @file

/// @defgroup Syscalls Syscalls group
/// @{

/// @brief This file contains the syscalls list
///        To add a syscall, add it to the SYSCALLS_LIST list define, and the function signature with the others

#define SYSCALLS_LIST				       \
    SYSCALL (SYS_PRINT_CHAR, SysPrintChar) \
    SYSCALL (SYS_PRINT_STR, SysPrintStr)   \
    SYSCALL (SYS_INVALID, SysInvalid)


/*
    SYSCALLS Begin
*/
void SysPrintChar(const InterruptFromUserlandContext * context);
void SysPrintStr(const InterruptFromUserlandContext * context);
void SysInvalid(const InterruptFromUserlandContext * context);
/*
    SYSCALLS End
*/

/// @}
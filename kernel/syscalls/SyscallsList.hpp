#pragma once

/// @file

/// @defgroup Syscalls Syscalls group
/// @{

/// @brief This file contains the syscalls list
///        To add a syscall, add it to the SYSCALLS_LIST list define, and the function signature with the others

#define SYSCALLS_LIST				  \
    SYSCALL (SYS_PRINT, SysPrint)	  \
    SYSCALL (SYS_PRINT2, SysPrint2)   \
    SYSCALL (SYS_INVALID, SysInvalid)


/*
    SYSCALLS Begin
*/
void SysPrint(const InterruptFromUserlandContext * context);
void SysInvalid(const InterruptFromUserlandContext * context);
/*
    SYSCALLS End
*/

/// @}
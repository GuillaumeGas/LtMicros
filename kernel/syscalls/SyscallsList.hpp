#pragma once

/// @file

/// @defgroup Syscalls Syscalls group
/// @{

/// @brief This file contains the syscalls list
///        To add a syscall, add it to the SYSCALLS_LIST list define, and the function signature with the others

#define SYSCALLS_LIST                                     \
    SYSCALL (SYS_PRINT_CHAR,         SysPrintChar)        \
    SYSCALL (SYS_PRINT_STR,          SysPrintStr)         \
    SYSCALL (SYS_SBRK,               SysSbrk)             \
    SYSCALL (SYS_IPC_SERVER_CREATE,  SysIpcServerCreate)  \
    SYSCALL (SYS_IPC_SERVER_CONNECT, SysIpcServerConnect) \
    SYSCALL (SYS_IPC_SEND,           SysIpcSend)          \
    SYSCALL (SYS_IPC_RECV,           SysIpcReceive)       \
    SYSCALL (SYS_INVALID,            SysInvalid)


/*
    SYSCALLS Begin
*/
void SysPrintChar(InterruptFromUserlandContext * context);
void SysPrintStr(InterruptFromUserlandContext * context);

void SysSbrk(InterruptFromUserlandContext * context);

void SysIpcServerCreate(InterruptFromUserlandContext* context);
void SysIpcServerConnect(InterruptFromUserlandContext* context);
void SysIpcServerSend(InterruptFromUserlandContext* context);
void SysIpcServerReceive(InterruptFromUserlandContext* context);

void SysInvalid(InterruptFromUserlandContext * context);
/*
    SYSCALLS End
*/

/// @}
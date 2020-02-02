#pragma once

/*
    Syscalls functions defines, implem are in syscalls.asm
*/

#include <kernel/syscalls/UKSyscallsCommon.h>

extern "C" void _sysPrint(const char * str);
extern "C" void _sysPrintChar(const char c);
extern "C" void * _sysSbrk(const int nbBlock);
extern "C" int _sysIpcServerCreate(const char * serverId, int * handle);
extern "C" int _sysIpcServerConnect(const char* serverId, int* handle);
extern "C" int _sysIpcSend(const int ipcHandle, const char * message, const unsigned int size);
extern "C" int _sysIpcReceive(SysIpcReceiveParameter * const parameters);
extern "C" void _sysRaiseThreadPriority();
extern "C" void _sysLowerThreadPriority();
// TMP
extern "C" void _sysEnterScreenCriticalSection();
extern "C" void _sysLeaveScreenCriticalSection();
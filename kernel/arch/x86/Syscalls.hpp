#pragma once

class SyscallsX86
{
public:
    void Init();
};

#ifdef __SYSCALLS_X86__
SyscallsX86 gSyscallsX86;
#else
extern SyscallsX86 gSyscallsX86;
#endif
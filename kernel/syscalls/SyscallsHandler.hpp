#pragma once

#include <kernel/arch/x86/InterruptContext.hpp>
#include <kernel/lib/StdIo.hpp>

#include "SyscallsList.hpp"

/// @file

/// @addgroup Syscalls Syscalls group
/// @{

/// @brief Syscalls list built using the syscalls list defined in SyscallsList.hpp
enum SyscallId 
{
#define SYSCALL(name, _) name,
#define SYSCALL_ID(x, y) SYSCALL(x, y)
    SYSCALLS_LIST
#undef SYSCALL
#undef SYSCALL_ID
};

/// @brief Class used to execute syscalls
class SyscallsHandler
{
public:
    /// @brief Executes a syscall
    ///        Its body is build using the syscalls list defined in SyscallsList.hpp
    /// @param[in] sysId The syscall id we want to execute
    /// @param[in] context A pointer to the syscall trap context
    static void ExecuteSyscall(const SyscallId sysId, const InterruptFromUserlandContext * context);
};

/// @}
#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/Types.hpp>
#include <kernel/lib/List.hpp>

#include <kernel/arch/x86/Process.hpp>
#include <kernel/arch/x86/Thread.hpp>

/// @file

/// @defgroup TaskGroup Task group
/// @{

class ProcessManager
{
public:
    /// @brief Initializes the process manager
    void Init();

    /// @brief Creates a process
    /// @warning This does not add the process to the scheduler process list
    /// @param[in] entryAddr The virtual address of the first byte of code, the main thread will be created using this address
    /// @param[out] newProcess A pointer that will receive an pointer to the created process
    /// @param[in] attribute The process security attribute(s), may be one of the SecurityAttribute enum.
    /// @param[in,opt] parent A pointer to the parent process, or nullptr
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateProcess(u32 entryAddr, Process ** newProcess, SecurityAttribute attribute, Process * parent = nullptr);

    /// @brief Creates the system process (must be unique)
    /// @param[out] newProcess A pointer that will receive an pointer to the created process
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateSystemProcess(Process ** process);

    /// @brief Deletes a process
    /// @warning This does not stop its execution, or erase it from the scheduler process list, it just frees the structure describing the process
    /// @param[in] process A pointer to the process to delete
    void DeleteProcess(Process * process);

private:
    List * _processList;
};

#ifdef __PROCESS_MANAGER__
ProcessManager gProcessManager;
#else
extern ProcessManager gProcessManager;
#endif

/// @}
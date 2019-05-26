#pragma once

#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>

/// @addgroup ArchX86Group
/// @{

struct Thread;

/// @brief Describes a process
struct Process
{
    /// @brief Process unique identifier
    int pid;
    /// @brief Process page directory
    PageDirectory pageDirectory;
    /// @brief A pointer to the process main thread
    Thread * mainThread;
    /// @brief Process children list
    List * childrenList;

    /// @brief Adds a thread to the process. The mainThread is null, it is set with this thread
    void AddThread(Thread * thread);

    /// @brief Creates a x86 process
    /// @warning This does not add the process to the scheduler process list
    /// @param[out] newProcess A pointer that will receive an pointer to the created process
    /// @param[in,opt] parent A pointer to the parent process, or nullptr
    /// @return STATUS_SUCCESS on success, an error code otherwise
    static KeStatus Create(Process ** newProcess, Process * parent = nullptr);

    /// @brief Creates a x86 system process.
    /// @warning This does not add the process to the scheduler process list
    /// @param[out] newProcess A pointer that will receive an pointer to the created process
    /// @return STATUS_SUCCESS on success, an error code otherwise
    static KeStatus CreateSystem(Process ** newProcess);

    /// @brief Releases process ressources
    /// @param[in] process A pointer to the process to be released
    static void Delete(Process * process);

    /// @brief Creates a page directory for the process
    ///        The first part (1Go) is a copy of the kernel page directory
    /// @return A page directory structure
    static PageDirectory CreateProcessPageDirectory();

    /// @brief Releases resources allocated for the process page directory
    /// @param[in] pd A page directory structure
    static void ReleaseProcessPageDirectoryEntry(PageDirectory pd);
};

/// @}
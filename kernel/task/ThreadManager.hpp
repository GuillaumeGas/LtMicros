#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/Types.hpp>
#include <kernel/lib/List.hpp>

#include <kernel/arch/x86/Thread.hpp>
#include <kernel/arch/x86/Process.hpp>

/// @file

/// @defgroup TaskGroup Task group
/// @{

class ThreadManager
{
public:
    /// @brief Initializes the thread manager
    void Init();

    /// @brief Creates a user thread for a given process. If the main thread of the process is null, the created thread will me its main thread
    /// @warning This does not add the thread to the scheduler
    /// @param[in] entryAddr The virtual address of the first byte of code
    /// @param[in] process A pointer to the parent process.
    /// @param[in,opt] parent A pointer to the parent process, or nullptr
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateUserThread(u32 entryAddr, Process * process, Thread ** thread);

    /// @brief Creates a kernel thread for a given process. If the main thread of the process is null, the created thread will me its main thread
    /// @warning This does not add the thread to the scheduler
    /// @param[in] entryAddr The virtual address of the first byte of code
    /// @param[in] process A pointer to the parent process.
    /// @param[in,opt] parent A pointer to the parent process, or nullptr
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateKernelThread(u32 entryAddr, Process * process, Thread ** thread);

    /// @brief Deletes a thread
    /// @warning This does not stop its execution, or erase it from the scheduler, it just frees the structure describing the thread
    /// @param[in] thread A pointer to the thread to delete
    void DeleteThread(Thread * thread);
};

#ifdef __THREAD_MANAGER__
ThreadManager gThreadManager;
#else
extern ThreadManager gThreadManager;
#endif

/// @}
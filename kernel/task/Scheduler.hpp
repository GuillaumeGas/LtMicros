#pragma once

#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/List.hpp>

#include <kernel/arch/x86/Thread.hpp>
#include <kernel/arch/x86/InterruptContext.hpp>

/// @file

/// @defgroup TaskGroup Task group
/// @{

class Scheduler
{
public:
    /// @brief Initializes the scheduler
    void Init();

    /// @brief Add a thread to the scheduler list
    /// @warning The system process main thread must be the first to be added to the scheduler
    /// @param[in] thread A pointer to the thread to add to the scheduler
    void AddThread(Thread * thread);

    /// @brief Starts the scheduler
    void Start();

    /// @brief Stops the scheduler
    void Stop();

    /// @brief Switch to the next thread waiting to be executed
    /// @param[in] Interrupt context, used to save the current thread state (pushed on the stack during interrupt)
    void Schedules(InterruptContext * context);

    /// @brief Retrieves the current running process
    /// @return A pointer to the current process structure
    Process * GetCurrentProcess();

    /// @brief Retrieves the current running thread
    /// @return A pointer to the current thread structure
    Thread * GetCurrentThread();

private:
    bool _running;
    List * _threadsList;
    Thread * _currentThread;
    unsigned int _nbThreads;

    /// @brief Retrieves the next thread waiting to be executed
    /// @param[in] currentThread Pointer to the current thread
    /// @return A pointer to the thread to be executed
    Thread * _PickNextThread(Thread * currentThread);

    /// @brief Checks in the threads list if another thread has a higher priority than the given one
    /// @param[in] thread Pointer to the thread
    /// @return true if another process has a higher priority, else false
    bool HasHigherThreadPriority(Thread* thread);

    /// @brief Switch to another thread and executes it
    /// @param[in] thread A pointer to the thread to be executed
    /// @param[in] Interrupt context, used to save the current thread state (pushed on the stack during interrupt)
    void _SwitchToThread(InterruptContext * context, Thread * thread);
};

#ifdef __SCHEDULER__
Scheduler gScheduler;
#else
extern Scheduler gScheduler;
#endif

/// @}
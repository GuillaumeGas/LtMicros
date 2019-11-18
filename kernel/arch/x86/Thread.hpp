#pragma once

#include <kernel/lib/Types.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/InterruptContext.hpp>

#include <kernel/task/ThreadManager.hpp>
#include <kernel/task/Common.hpp>

/// @addgroup ArchX86Group
/// @{

#define USER_TASK_V_ADDR  0x40000000
#define USER_STACK_V_ADDR 0xE0000000

/// @brief Used to indicate the thread state
enum ThreadState
{
    /// @brief Indicated that the thread has been created but not launched yet
    THREAD_STATE_INIT = 0,
    /// @brief The thread is running
    THREAD_STATE_RUNNING,
    /// @brief The thread is paused
    THREAD_STATE_PAUSED,
    /// @brief The thread is dead and may be suppressed
    THREAD_STATE_DEAD
};

struct Process;

/// @brief Describes a thread
struct Thread
{
    /// @brief Thread unique identifier
    int tid;
    /// @brief Thread state, running, paused...
    ThreadState state;
    /// @brief A pointer to the process that own this thread
    Process * process;
    /// @brief A pointer to the next thread with the same parent process
    Thread * neighbor;
    /// @brief Indicated the thread privilege level
    PrivilegeLevel privilegeLevel;

    /// @brief Describes the kernel stack that will be used if an interrupt occured with this thread running
    struct
    {
        u32 esp0;
        u16 ss0;
    } kstack;

    /// @brief Describes all registers, used to same the current thread state by the scheduler before switching to another task
    struct
    {
        u32 eax, ebx, ecx, edx;
        u32 ebp, esp, esi, edi;
        u32 eip, eflags;
        u32 cs : 16, ss : 16, ds : 16, es : 16, fs : 16, gs : 16;
        u32 cr3;
    } regs;

    /// @brief Adds a thread in the current thread neighbors list
    /// @param[in] thread A pointer to the thread to add to the list
    void AddNeighbor(Thread * thread);

    /// @brief Saves the current thread state (all registers mainly)
    /// @param[in] context A pointer to the interrupt context
    void SaveState(InterruptContext * context);

    /// @brief Starts or resumes a thread execution by pushing all necessary registers and using iret instruction (in fact, calls a asm function to do that stuff)
    void StartOrResume();

    /// @brief Creates the default user thread stack
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateDefaultStack();

    void PrintList();
    void Print();

    /// @brief Creates an x86 thread
    /// @param[in] entryAddr A 32bits address of the entry code of the thread
    /// @param[in] process A pointer to the process that own this thread
    /// @param[in] privLevel Privilege level of the thread (user or kernel)
    /// @param[in] attribute The process security attribute(s), may be one of the SecurityAttribute enum.
    /// @param[out] thread Will a pointer to the created thread
    /// @return STATUS_SUCCESS on success, an error code otherwise
    static KeStatus CreateThread(u32 entryAddr, Process * process, PrivilegeLevel privLevel, SecurityAttribute attribute, Thread ** thread);

private:
    static KeStatus _InitUserThread(Thread * thread, Process * process, SecurityAttribute attribute, u32 entryAddr);
    static KeStatus _InitKernelThread(Thread * thread, u32 entryAddr);
};

extern "C" void _startOrResumeThread(PageDirectoryEntry * pd, u32 ss, u32 esp, u32 eflags, u32 cs, u32 eip,
    u32 eax, u32 ecx, u32 edx, u32 ebx, u32 ebp, u32 esi, u32 edi, u32 ds, u32 es, u32 fs, u32 gs,
    PrivilegeLevel privLevel);

/// @}
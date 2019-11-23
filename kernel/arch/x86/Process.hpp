#pragma once

#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>
#include <kernel/mem/Vad.hpp>

/// @addgroup ArchX86Group
/// @{

/// @brief Process virtual base address
#define V_PROCESS_BASE_ADDR  0x40000000
#define V_PROCESS_LIMIT_ADDR 0xFFFFFFFF

/// TEST : 4 pages
#define DEFAULT_HEAP_SIZE 0x1000 * 0x4

struct Thread;

struct ProcessHeap
{
    u8 * baseAddress;

    u8 * limitAddress;

    Vad * vad;
};

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
    /// @brief Structure describing the default process heap
    ProcessHeap defaultHeap;
    /// @brief Vads list
    Vad * baseVad;

    /// @brief Adds a thread to the process. The mainThread is null, it is set with this thread
    void AddThread(Thread * thread);

    /// @brief Increases the process heap of x pages
    /// @param[in]  nbPages The number of pages required
    /// @param[out] allocatedBlockAddr Pointer that will hold the allocated block virtual address
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus IncreaseHeap(unsigned int nbPages, u8 ** allocatedBlockAddr);

    /// @brief Creates the default process heap and main thread stack
    /// @note This function must be called after mapping the executable code in the process address space
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus CreateDefaultHeapAndStack();

    /// @brief Creates a x86 process
    /// @warning This does not add the process to the scheduler process list
    /// @warning This does not create the process heap and stack, for this call CreateDefaultHeapAndStack() after having mapped the executable code into process address space
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

    /// @brief Copies a source area to a destination
    /// @param[in] sourceAddress A pointer to the memory we want to copy
    /// @param[in] destAddress A pointer to the memory where to copy
    /// @param[in] size The memory size in bytes we want to copy
    void MemoryCopy(const u8 * const sourceAddress, u8 * const destAddress, const unsigned int size);

    /// @brief Sets at memory area with a given byte, and copies a source area to a destination
    /// @param[in] sourceAddress A pointer to the memory we want to copy
    /// @param[in] destAddress A pointer to the memory where to copy
    /// @param[in] size The memory size in bytes we want to copy
    /// @param[in] byte The byte used to set memory
    void MemorySetAndCopy(const u8 * const sourceAddress, u8 * const destAddress, const unsigned int size, const u8 byte);

    /// @brief Looks for a new vad and allocates the required size
    /// @param[in]  size The required size
    /// @param[in]  reservePhysicalPages Boolean telling if the physical pages must be reserved immediately
    /// @param[out] outAddress Pointer that will hold the new address
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus AllocateMemory(const unsigned int size, const bool reservePhysicalPages, void ** const outAddress);

    /// @brief Looks for a new vad at the asked address and allocates the required size
    /// @param[in] address The asked address
    /// @param[in] reservePhysicalPages Boolean telling if the physical pages must be reserved immediately
    /// @param[in] size The required size
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus AllocateMemoryAtAddress(void * const address, const bool reservePhysicalPages, const unsigned int size);

    /// @brief Try to resolve the page fault given an address by looking for the related VAD in process
    ///        If a VAD in use is found, a new physical page is reserved, and the PTE is updated to set the page as in memory.
    KeStatus ResolvePageFault(void* const address);

    void PrintState();
};

/// @}
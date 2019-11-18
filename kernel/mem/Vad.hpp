#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/arch/x86/Vmm.hpp>

/// @brief Virtual address descriptor, used to describe the virtual address space of a process
struct Vad
{
    /// The memory block base address
    u8 * baseAddress;
    /// The memoty block limit address
    u8 * limitAddress;

    /// The memory block size in bytes
    unsigned int size;

    /// Boolean telling if this block is available
    bool free;

    Vad * previous;
    Vad * next;

    /// @brief Creates a simple vad
    /// @param[in]  baseAddress The memory block base address
    /// @param[in]  size The memory block size in bytes
    /// @param[in]  free Boolean telling if the block is free or not
    /// @param[out] OutVad Pointer that will hold a pointer to the newly allocated vad
    /// @return A pointer to the allocated vad or nullptr if the allocation failed
    static KeStatus Create(void * const baseAddress, const unsigned int size, bool free, Vad ** const OutVad);

    /// @brief Looks for an available vad, split it if necessary, reserves it, and allocates memory
    /// @param[in]  size The required size
    /// @param[in]  pageDirectory The process page directory
    /// @param[out] outVad Pointer that will hold the new vad
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus Allocate(const unsigned int size, const PageDirectory * pageDirectory, Vad** const outVad);

    /// @brief Looks for a new vad at the asked address and allocate the required size
    /// @param[in] address The asked address
    /// @param[in] size The required size
    /// @param[in] pageDirectory The process page directory
    /// @param[out] OutVad Pointer that will hold a pointer to the newly allocated vad
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus AllocateAtAddress(void * const address, const unsigned int size, const PageDirectory * pageDirectory, Vad ** const OutVad);

    /// @brief Looks for an available vad with a minimum required size
    /// @param[in]  size The required size
    /// @param[out] outVad Pointer that will hold the free vad if found
    /// @return STATUS_SUCCESS on success, STATUS_NOT_FOUND if no vad is found, an error code otherwise
    KeStatus LookForFreeVadOfMinimumSize(const unsigned int size, Vad ** const outVad);

    /// @brief Looks for an available vad with a minimum required size
    /// @param[in]  address The asked address
    /// @param[in]  size The required size
    /// @param[out] outVad Pointer that will hold the free vad if found
    /// @return STATUS_SUCCESS on success, STATUS_NOT_FOUND if no vad is found, an error code otherwise
    KeStatus LookForFreeVadAtAddressOfMinimumSize(void * const address, const unsigned int size, Vad ** const outVad);

    /// @brief Splits the current vad in two vads
    /// @param[in] size The the vad minimum size
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus Split(const unsigned int size);

    /// @brief Splits the current vad in two vads
    /// @param[in] address The asked address
    /// @param[in] size The the vad minimum size
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus SplitAtAddress(void * const address, const unsigned int size);

    /// @brief Set the current vad as reserved, allocates physical memory and map it into the given address space
    /// @param[in]  pageDirectory The process page directory
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus ReserveAndAllocateMemory(const PageDirectory * pageDirectory);

    void PrintVad();
};
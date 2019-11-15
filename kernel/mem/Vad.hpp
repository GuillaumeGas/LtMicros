#pragma once

#include <kernel/lib/Status.hpp>

/// @brief Virtual address descriptor, used to describe the virtual address space of a process
struct Vad
{
    /// The memory block base address
    void* baseAddress;

    /// The memory block size in bytes
    unsigned int size;

    /// Boolean telling if this block is available
    bool free;

    /// @brief Creates a simple vad
    /// @param[in]  baseAddress The memory block base address
    /// @param[in]  size The memory block size in bytes
    /// @param[in]  free Boolean telling if the block is free or not
    /// @param[out] OutVad Pointer that will hold a pointer to the newly allocated vad
    /// @return A pointer to the allocated vad or nullptr if the allocation failed
    static KeStatus Create(const void* baseAddress, const unsigned int size, bool free, Vad ** const OutVad);

    KeStatus Allocate(const unsigned int size, Vad** const OutVad);
};
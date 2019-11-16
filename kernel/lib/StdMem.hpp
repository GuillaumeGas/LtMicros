#pragma once

#include <kernel/arch/x86/Vmm.hpp>

/// @file

/// @defgroup KernelLibGroup Kernel lib group
/// @{

/// @brief Because we like our traditional malloc()...

/// @brief Allocates memory
/// @param[in] size The size is bytes
/// @return A pointer to the allocated memory on success, nullptr otherwise
void * HeapAlloc(int size);

/// @brief Frees memory
/// @param[in] ptr A pointer to the memory to be freed
void HeapFree(void * ptr);

/// @brief Allocates a page
/// @return A pointer to the allocated page on success, nullptr otherwise
Page PageAlloc();

/// @brief Releases a given page, allocated from the page pool, using its address
/// @param[in] ptr The page to be freed
void PageFree(const Page page);

/// @brief Copies memory from a source to a destination
/// @param[in] src A pointer to the source
/// @param[in] dst A pointer to the destination
/// @param[in] size The length in bytes
void MemCopy(const void * const src, void * dst, unsigned int size);

/// @brief Sets a chunck of memory with a given byte
/// @param[in] src A pointer to the data
/// @param[in] byte A byte used to fill the data with
/// @param[in] size The length in bytes
void MemSet(void * src, u8 byte, unsigned int size);

/// @}
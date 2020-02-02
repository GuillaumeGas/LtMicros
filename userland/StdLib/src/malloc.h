#pragma once

/// @brief Allocates memory
/// @param[in] size The size is bytes
/// @return A pointer to the allocated memory on success, nullptr otherwise
void * HeapAlloc(int size);

/// @brief Frees memory
/// @param[in] ptr A pointer to the memory to be freed
void HeapFree(void * ptr);

/// @brief Initializes the malloc members
void InitMalloc();

void DumpHeap();
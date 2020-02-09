#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/task/Event.hpp>

// TODO : move event handling into IpcBuffer and make it thread safe

/// @file

/// @addgroup TaskGroup
/// @{

/// @brief The class provides a way to write and read bytes into a buffer. The buffer is composed of one or more pages
///        allocated on the kernel page pool.
/// @warning The IpcBuffer is not thread safe !
class IpcBuffer
{
public:
    /// @brief Empty constructor
    IpcBuffer();

    /// @brief Initializes the ipc buffer
    void Init();

    /// @brief Adds bytes to the buffer, may allocate a new page if necessary
    /// @param[in] message The message to be added in the buffer
    /// @param[in] size The message size in bytes
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus AddBytes(const char* message, const unsigned int size);
    
    /// @brief Reads bytes, may return less than the expected 'size' bytes, the real number of bytes is stored in the bytesRead out parameter
    /// @param[in]  buffer Pointer to a caller allocated buffer of minimum size 'size'
    /// @param[in]  size The buffer size in bytes, and the maximum number of bytes we will try to read
    /// @param[out] bytesRead Pointer that will receive the number of bytes read
    /// @return STATUS_SUCCESS on success, an error code otherwise
    KeStatus ReadBytes(char* const buffer, const unsigned int size, unsigned int* const bytesRead);

    /// @brief Tells if there are bytes available to be read
    /// @return true is there are some, else false
    bool BytesAvailable() const;

    /// @brief The event is set when bytes are available
    Event ReadyToReadEvent;

private:
    Page * AllocatePage() const;
    KeStatus AllocateWritePage();

    Page * currentPageWrite;
    Page * currentPageRead;

    char * currentPageWritePtr;
    char * currentPageReadPtr;
    
    char * currentWriteLimit;
    char * currentReadLimit;

    List * pagesList;
};

/// @}
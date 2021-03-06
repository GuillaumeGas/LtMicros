﻿#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>
#include <kernel/arch/x86/Process.hpp>
#include <kernel/handle/HandleManager.h>

/// @file

/// @addgroup TaskGroup
/// @{

typedef unsigned int IpcHandle;

struct IpcObject;

class IpcHandler
{
public:
    /// @brief Initializes the Ipc handler
    void Init();

    /// @brief Adds the given process as a new IPC server
    /// @param[in]  serverIdStr A string identifying the server
    /// @param[in]  serverProcess The process asking to become an IPC server
    /// @param[out] ipcHandle Pointer that will hold the new IpcHandle
    /// @return IPC_STATUS_SUCCESS on success, an error code otherwise
    KeStatus AddNewServer(const char* serverIdStr, Process* const serverProcess, IpcHandle* const ipcHandle);

    /// @brief Looks for a Ipc object given a server id string and retrieves its handle.
    /// @note This function could check if the client process is authorized to connect to this server in the futur.
    /// @param[in]  serverIdStr A string identifying the server
    /// @param[in]  clientProcess The client asking to connect to the server
    /// @param[out] ipcHandle Pointer that will hold the server IpcHandle
    KeStatus ConnectToServer(const char* serverIdStr, Process * const clientProcess, IpcHandle* const ipcHandle);

    /// @brief Adds a given message sent my a given process to the list of messages associated to a given handle
    /// @param[in] handle The handle on a Ipc object
    /// @param[in] clientProcess The client process sending the message
    /// @param[in] message A pointer to the message. The pointed memory is in the client process address space.
    /// @param[in] size The message size in bytes
    /// @return IPC_STATUS_SUCCESS on success, an error code otherwise
    KeStatus Send(const IpcHandle handle, Process* const clientProcess, const char* message, const unsigned int size);

    /// @brief Pops the next stacked message in the list associated to the Ipc object
    /// @param[in]  handle The handle on a Ipc object
    /// @param[in]  serverProcess The server process receiving the message
    /// @param[in]  message A pointer that will hold a pointer to the message. The memory is allocated by the caller (user process).
    /// @param[in]  size The message size in bytes
    /// @param[out] bytesRead A pointer that will hold the number of bytes copied in the buffer
    /// @return IPC_STATUS_SUCCESS on success, an error code otherwise
    KeStatus Receive(const IpcHandle handle, Process* const serverProcess, char * const buffer, const unsigned int size, unsigned int * const bytesRead);

    /// @brief Releases memory allocated for an IPC in a process address space
    /// @param[in] process The process in which the memory must be released
    /// @param[in] ptr Pointer to the memory to be released
    /// @return IPC_STATUS_SUCCESS on success, an error code otherwise
    KeStatus ReleaseMemory(Process* const process, void* ptr);

private:
    /// @brief Allocates memory in a given process address space
    /// @param[in]  process The process in which the memory must be allocated
    /// @param[in]  size The memory size in bytes that must be allocated
    /// @param[out] buffer Pointer that will hold a pointer to the allocated memory
    KeStatus _AllocateMemory(Process* processconst, unsigned int size, char** const buffer);

    /// @brief Retrieves an ipc object from its handle
    /// @param[in] handle The ipc object handle
    /// @return A pointer to the found ipc object, or nullptr if not found
    IpcObject* _FindIpcObjectByHandle(const IpcHandle handle) const ;

    /// @brief Retrieves an ipc object from its server id
    /// @param[in] serverId The ipc server id
    /// @return A pointer to the found ipc object, or nullptr if not found
    IpcObject* _FindIpcObjectByServerId(const char * serverId) const;

    /// @brief Checks if the given server id string is already used by the same or another process
    /// @param[in] serverIdStr The server id string
    /// @return true if the string is already used, else false
    bool _IsServerIdStrAlreadyUsed(const char * serverIdStr) const;

    List * _ipcObjects;
};

#ifdef __IPC_HANDLER__
IpcHandler gIpcHandler;
#else
extern IpcHandler gIpcHandler;
#endif

/// @}
#include "Ipc.hpp"
#include "syscalls.h"
#include "stdio.h"
#include "types.h"

#include <kernel/syscalls/UKSyscallsCommon.h>

IpcStatus IpcServer::Create(const char * const serverName, IpcServer * const server)
{
    IpcStatus status = STATUS_FAILURE;
    IpcHandle handle = INVALID_HANDLE_VALUE;

    if (serverName == nullptr)
    {
        printf("Invalid serverName parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (server == nullptr)
    {
        printf("Invalid server parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    printf("Creating server for %s\n", serverName);

    status = (IpcStatus)_sysIpcServerCreate(serverName, &handle);
    if (FAILED(status))
    {
        printf("_sysIpcServerCreate() failed with code %d\n", status);
        goto clean;
    }

    server->_serverHandle = handle;
    handle = INVALID_HANDLE_VALUE;

    status = STATUS_SUCCESS;

clean:
    return status;
}

IpcStatus IpcServer::Receive(char * const buffer, const unsigned int size, unsigned int * const readBytes, Handle * const clientProcessHandle)
{
    IpcStatus status = STATUS_FAILURE;
    SysIpcReceiveParameter parameters;

    if (buffer == nullptr)
    {
        printf("Invalid buffer parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        printf("Invalid size parameter\n");
        return STATUS_INVALID_PARAMETER;
    }

    if (readBytes == nullptr)
    {
        printf("Invalid readBytes parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (clientProcessHandle == nullptr)
    {
        printf("Invalid clientProcessHandle parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    parameters.ipcHandle = _serverHandle;
    parameters.buffer = buffer;
    parameters.size = size;
    parameters.readBytesPtr = readBytes;
    parameters.clientProcessHandlePtr = clientProcessHandle;

    status = (IpcStatus)_sysIpcReceive(&parameters);
    if (FAILED(status))
    {
        printf("_sysIpcReceive() failed with code %d\n", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

IpcStatus IpcClient::ConnectToServer(const char * serverName, IpcServerHandle * const handle)
{
    IpcStatus status = STATUS_FAILURE;

    if (serverName == nullptr)
    {
        printf("Invalid serverName parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (handle == nullptr)
    {
        printf("Invalid handle parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    status = (IpcStatus)_sysIpcServerConnect(serverName, handle);
    if (FAILED(status))
    {
        printf("_sysIpcServerConnect() failed with code %d\n", status);
    }

    return status;
}

IpcStatus IpcClient::Send(const IpcServerHandle serverHandle, const char * message, const unsigned int size)
{
    IpcStatus status = STATUS_FAILURE;

    if (serverHandle == INVALID_HANDLE_VALUE)
    {
        printf("Invalid serverHandle parameter\n");
        return STATUS_INVALID_PARAMETER;
    }

    if (message == nullptr)
    {
        printf("Invalid message parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        printf("Invaid size parameter\n");
        return STATUS_INVALID_PARAMETER;
    }

    status = (IpcStatus)_sysIpcSend(serverHandle, message, size);
    if (FAILED(status))
    {
        printf("_sysIpcSend() failed with code %d\n", status);
    }

    return status;
}
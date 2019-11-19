#include "Ipc.hpp"
#include "syscalls.h"
#include "stdio.h"

IpcStatus IpcServer::Create(const char * const serverName, IpcServer * const server)
{
    IpcStatus status = STATUS_FAILURE;
    IpcHandle handle = IPC_INVALID_HANDLE;

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

    status = (IpcStatus)_sysIpcServerCreate(serverName, &handle);
    if (FAILED(status))
    {
        printf("_sysIpcServerCreate() failed with code %d\n", status);
        goto clean;
    }

    server->_serverHandle = handle;
    handle = IPC_INVALID_HANDLE;

    status = STATUS_SUCCESS;

clean:
    return status;
}

IpcStatus IpcServer::Receive(IpcMessage * const message)
{
    IpcStatus status = STATUS_FAILURE;

    if (message == nullptr)
    {
        printf("Invalid message parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    status = (IpcStatus)_sysIpcReceive(_serverHandle, (char**)&message->data, &message->size);
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

    if (serverHandle == IPC_INVALID_HANDLE)
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
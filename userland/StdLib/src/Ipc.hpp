#pragma once

#include "status.h"

#define IPC_INVALID_HANDLE 0

typedef Status IpcStatus;
typedef int IpcHandle;
typedef IpcHandle IpcServerHandle;

struct IpcMessage
{
    void * data;
    unsigned int size;

    // void Release();
};

class IpcServer
{
public:
    static IpcStatus Create(const char * const serverName, IpcServer * const server);

    IpcStatus Receive(IpcMessage * const message);

private:
    IpcHandle _serverHandle;
};

class IpcClient
{
public:
    IpcStatus ConnectToServer(const char * serverName, IpcServerHandle * const handle);

    IpcStatus Send(const IpcServerHandle serverHandle, const char * message, const unsigned int size);
};
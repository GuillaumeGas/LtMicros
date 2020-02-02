#pragma once

#include "status.h"

#define INVALID_HANDLE_VALUE 0

typedef Status IpcStatus;
typedef int IpcHandle;
typedef IpcHandle IpcServerHandle;

// tmp
typedef int ProcessHandle;

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

    IpcStatus Receive(IpcMessage * const message, ProcessHandle * clientHandle);

private:
    IpcHandle _serverHandle;
};

class IpcClient
{
public:
    IpcStatus ConnectToServer(const char * serverName, IpcServerHandle * const handle);

    IpcStatus Send(const IpcServerHandle serverHandle, const char * message, const unsigned int size);
};
#pragma once

#include "status.h"
#include "types.h"

#define INVALID_HANDLE_VALUE 0

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

    IpcStatus Receive(char * const buffer, const unsigned int size, unsigned int * const readByteses, Handle * clientProcessHandle);

private:
    IpcHandle _serverHandle;
};

class IpcClient
{
public:
    IpcStatus ConnectToServer(const char * serverName, IpcServerHandle * const handle);

    IpcStatus Send(const IpcServerHandle serverHandle, const char * message, const unsigned int size);
};
#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>

#include <LtFsCommon.h>
#include <ServiceNames.h>

#include "Common.h"

static void LoadSystem();

void main()
{
    LOG(LOG_INFO, "Starting LtInit service");

    LoadSystem();

    LOG(LOG_INFO, "Terminating LtInit service");

    while (1);
}

static void LoadSystem()
{
    IpcClient client;
    IpcServerHandle serverHandle = IPC_INVALID_HANDLE;
    IpcStatus status = STATUS_FAILURE;

    status = client.ConnectToServer(LTFS_SERVICE_NAME, &serverHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::ConnectToServer() failed with code %d", status);
        return;
    }

    LOG(LOG_INFO, "Sending message '%s'", SERVICE_TEST_CMD);

    status = client.Send(serverHandle, SERVICE_TEST_CMD, StrLen(SERVICE_TEST_CMD));
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::Send() failed with code %d", status);
        return;
    }

    LOG(LOG_INFO, "Testing the heap :");
    InitMalloc();
    int* a = (int*)HeapAlloc(sizeof(int));
    *a = 42;
    LOG(LOG_INFO, "Value : %d", *a);
}
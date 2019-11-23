#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>

#include <LtFsCommon.h>
#include <ServiceNames.h>

static void LoadSystem();

void main()
{
    printf("[INIT] # Starting LtInit service\n");

    LoadSystem();

    printf("[INIT] # Terminating LtInit service\n");

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
        printf("[INIT] IpcClient::ConnectToServer() failed with code %d\n", status);
        return;
    }

    printf("[INIT] Sending message '%s'\n", SERVICE_TEST_CMD);

    status = client.Send(serverHandle, SERVICE_TEST_CMD, StrLen(SERVICE_TEST_CMD));
    if (FAILED(status))
    {
        printf("[INIT] IpcClient::Send() failed with code %d\n", status);
        return;
    }

    printf("[INIT] Testing the heap :\n");
    InitMalloc();
    int* a = (int*)HeapAlloc(sizeof(int));
    *a = 42;
    printf("[INIT] Value : %d\n", *a);
}
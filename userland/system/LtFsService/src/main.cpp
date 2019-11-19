#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>
#include <ServiceNames.h>

#include "AtaDriver.h"
#include "ServiceCommands.h"

static bool AtaDeviceCreate();
static void StartListening();

void main()
{
    printf("# Starting LtFs service...\n");

    if (AtaDeviceCreate())
    {
        StartListening();

        printf("# Terminating LtFs service\n");
    }

    while (1);
}

static bool AtaDeviceCreate()
{
    AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    if (!AtaInit(&device))
    {
        printf("Can't init ata device\n");
        return false;
    }
    else
    {
        printf("[FS] Ata device initialized !\n");
        return true;
    }
}

static void StartListening()
{
    IpcServer server;
    IpcStatus status = STATUS_FAILURE;
    bool serviceTerminate = false;

    status = IpcServer::Create(LTFS_SERVICE_NAME, &server);
    if (FAILED(status))
    {
        printf("IpcServer::Create() failed with code %d\n", status);
        return;
    }

    do
    {
        IpcMessage message;

        status = server.Receive(&message);
        if (FAILED(status))
        {
            printf("IpcServer::Receive() failed with code %d\n", status);
            break;
        }

        status = ServiceExecuteCommand((char*)message.data, message.size, &serviceTerminate);
        if (FAILED(status))
        {
            printf("ServiceExecuteCommand() failed with code %d\n", status);
            break;
        }

        // message.Release();
    } while (!serviceTerminate);
}
#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>
#include <ServiceNames.h>

#include "AtaDriver.h"
#include "ServiceCommands.h"
#include "Common.h"

static bool AtaDeviceCreate();
static void StartListening();

void main()
{
    LOG(LOG_INFO, "Starting LtFs service...");

    if (AtaDeviceCreate())
    {
        StartListening();

        LOG(LOG_INFO, "Terminating LtFs service");
    }

    while (1);
}

static bool AtaDeviceCreate()
{
    AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    if (!AtaInit(&device))
    {
        LOG(LOG_INFO, "Can't init ata device");
        return false;
    }
    else
    {
        LOG(LOG_INFO, "Ata device initialized !");
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
        LOG(LOG_ERROR, "IpcServer::Create() failed with code %d", status);
        return;
    }

    do
    {
        IpcMessage message;

        status = server.Receive(&message);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "IpcServer::Receive() failed with code %d", status);
            break;
        }

        status = ServiceExecuteCommand((char*)message.data, message.size, &serviceTerminate);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "ServiceExecuteCommand() failed with code %d", status);
            break;
        }

        // message.Release();
    } while (!serviceTerminate);
}
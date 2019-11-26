#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>
#include <ServiceNames.h>

#include "AtaDriver.h"
#include "FsManager.h"
#include "ServiceCommands.h"
#include "Common.h"

static bool AtaDeviceCreate();
static void StartListening();

void main()
{
    LOG(LOG_INFO, "Starting LtFs service...");

    // TMP !
    InitMalloc();

    if (AtaDeviceCreate())
    {
        StartListening();

        LOG(LOG_INFO, "Terminating LtFs service");
    }

    while (1);
}

static bool AtaDeviceCreate()
{
    bool result = false;

    RaiseThreadPriority();

    AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    if (!AtaInit(&device))
    {
        LOG(LOG_INFO, "Can't init ata device");
        result = false;
        goto clean;
    }
    else
    {
        LOG(LOG_INFO, "Ata device initialized !");

        Status status = FsInit(&device);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "FsInit() failed with code %t", status);
            result = false;
            goto clean;
        }

        File* file = nullptr;
        status = OpenFileFromName("test.txt", &file);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "OpenFileFromName() failed with code %t", status);
        }

        LOG(LOG_DEBUG, "file opened !");
    }

    result = true;

clean:
    LowerThreadPriority();

    return result;
}

static void StartListening()
{
    IpcServer server;
    IpcStatus status = STATUS_FAILURE;
    bool serviceTerminate = false;

    status = IpcServer::Create(LTFS_SERVICE_NAME, &server);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcServer::Create() failed with code %t", status);
        return;
    }

    do
    {
        IpcMessage message;
        ProcessHandle clientHandle = INVALID_HANDLE_VALUE;

        status = server.Receive(&message, &clientHandle);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "IpcServer::Receive() failed with code %t", status);
            break;
        }

        status = ServiceExecuteCommand((char*)message.data, message.size, &serviceTerminate);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "ServiceExecuteCommand() failed with code %t", status);
            break;
        }

        // message.Release();
    } while (!serviceTerminate);
}
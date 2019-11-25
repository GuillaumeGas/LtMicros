#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>

#include <LtFsCommon.h>
#include <ServiceNames.h>

#include "Common.h"
#define LOG(LOG_LEVEL, format, ...) LOGGER("INIT", LOG_LEVEL, format, ##__VA_ARGS__)

static void LoadSystem();

static void TestFile();

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
    IpcServerHandle serverHandle = INVALID_HANDLE_VALUE;
    IpcStatus status = STATUS_FAILURE;

    status = client.ConnectToServer(LTFS_SERVICE_NAME, &serverHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::ConnectToServer() failed with code %t", status);
        return;
    }

    TestFile();
}

static void TestFile()
{
    Status status = STATUS_FAILURE;
    Handle fileHandle = INVALID_HANDLE_VALUE;
    unsigned int fileSize = 0;

    status = FsOpenFile("test.txt", FILE_READ, &fileHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "FsOpenFile() failed with code %t", status);
        return;
    }

    //status = FsGetFileSize(fileHandle, &fileSize);
    //if (FAILED(status))
    //{
    //    LOG(LOG_ERROR, "FsGetFileSize() failed with code %d", status);
    //    return;
    //}

    //{
    //    char * fileContent = nullptr;
    //    unsigned int bytesRead = 0;
    //    status = FsReadFile(fileHandle, fileContent, fileSize, &bytesRead);
    //    if (FAILED(status))
    //    {
    //        LOG(LOG_ERROR, "FsReadFile() failed with code %d", status);
    //        return;
    //    }

    //    LOG(LOG_INFO, "File content : %s", fileContent);

    //    FsCloseFile(fileHandle);
    //}
}
#include <stdio.h>
#include <stdlib.h>
#include <Ipc.hpp>

#include <LtFsCommon.h>
#include <ServiceNames.h>

#include "Common.h"
#define LOG(LOG_LEVEL, format, ...) LOGGER("INIT", LOG_LEVEL, format, ##__VA_ARGS__)

static void LoadSystem();

static void TestFile();

static void Test()
{
    IpcClient client;
    IpcHandle serverHandle = INVALID_HANDLE_VALUE;
    Status status = client.ConnectToServer(LTFS_SERVICE_NAME, &serverHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpClient::ConnectToServer() failed with code %t", status);
        return;
    }

    char chaine[] = "Hello World !";
    status = client.Send(serverHandle, chaine, StrLen(chaine) + 1);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpClient::Send() failed with code %t", status);
        return;
    }
}

void main()
{
    LOG(LOG_INFO, "Starting LtInit service");

    InitMalloc();
    //LoadSystem();

    Test();

    LOG(LOG_INFO, "Terminating LtInit service");

    while (1);
}

static void LoadSystem()
{
    TestFile();
}

static void TestFile()
{
    Status status = STATUS_FAILURE;
    Handle fileHandle = INVALID_HANDLE_VALUE;
    unsigned int fileSize = 0;

    //status = FsOpenFile("test.txt", FILE_READ, FILE_SHARE_READ, &fileHandle);
    //if (FAILED(status))
    //{
    //    LOG(LOG_ERROR, "FsOpenFile() failed with code %t", status);
    //    return;
    //}

    //status = FsOpenFile("test.txt", FILE_READ, FILE_SHARE_NONE, &fileHandle);
    //if (FAILED(status))
    //{
    //    LOG(LOG_ERROR, "FsOpenFile() failed with code %t", status);
    //    return;
    //}

    //LOG(LOG_INFO, "Got handle %d", fileHandle);

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

    //    //LOG(LOG_INFO, "File content : %s", fileContent);

    //    FsCloseFile(fileHandle);
    //}
}
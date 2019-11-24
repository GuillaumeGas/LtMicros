#include "FileSystem.h"
#include "Ipc.hpp"
#include "stdlib.h"
#include "stdio.h"

#include <ServiceNames.h>
#include <LtFsCommon.h>

struct FsContext
{
    IpcClient ipcClient;
    IpcServerHandle ltFsServiceHandle;
    bool isInitilaized;
};

FsContext gFsContext;

static Status FsInit()
{
    Status status = STATUS_FAILURE;
    IpcHandle serverHandle = INVALID_HANDLE_VALUE;

    status = gFsContext.ipcClient.ConnectToServer(LTFS_SERVICE_NAME, &serverHandle);
    if (FAILED(status))
        return status;

    gFsContext.isInitilaized = true;
    gFsContext.ltFsServiceHandle = serverHandle;

    return status;
}

Status FsOpenFile(const char * filePath, const FileAccess access, Handle * const fileHandle)
{
    Status status = STATUS_FAILURE;
    LtFsRequest * request = nullptr;
    LtFsOpenFileParameters parameters;
    unsigned int requestSize = 0;

    if (filePath == nullptr)
    {
        return STATUS_NULL_PARAMETER;
    }

    if (access >= FILE_ACCESS_MAX)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (fileHandle == nullptr)
    {
        return STATUS_NULL_PARAMETER;
    }

    if (gFsContext.isInitilaized == false)
    {
        status = FsInit();
        if (FAILED(status))
        {
            goto clean;
        }
    }

    if ((StrLen(filePath) + 1) > MAX_FULL_PATH_SIZE)
    {
        status = PATH_TOO_LONG;
        goto clean;
    }

    parameters.access = access;
    parameters.fileHandlePtr = fileHandle;

    MemCopy((void*)filePath, &(parameters.filePath), StrLen(filePath) + 1);

    status = LtFsRequest::Create(LTFS_REQUEST_OPEN_FILE, &parameters, sizeof(LtFsOpenFileParameters), &request);
    if (FAILED(status))
    {
        goto clean;
    }

    requestSize = sizeof(LtFsRequest) + sizeof(LtFsOpenFileParameters);

    gFsContext.ipcClient.Send(gFsContext.ltFsServiceHandle, (char*)request, requestSize);
    if (FAILED(status))
    {
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    if (request != nullptr)
    {
        HeapFree(request);
        request = nullptr;
    }

    return status;
}

Status FsGetFileSize(const Handle fileHandle, unsigned int * const fileSize)
{
    return STATUS_SUCCESS;
}

Status FsReadFile(const Handle fileHandle, char * const buffer, const int bytes, unsigned int * const bytesRead)
{
    return STATUS_SUCCESS;
}

void FsCloseFile(const Handle fileHandle)
{

}
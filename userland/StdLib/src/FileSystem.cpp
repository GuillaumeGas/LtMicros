#include "FileSystem.h"
#include "Ipc.hpp"
#include "stdlib.h"
#include "stdio.h"

#include <ServiceNames.h>
#include <LtFsCommon.h>

struct FsContext
{
    IpcClient ipcClient;
    IpcServer ipcServer;
    IpcServerHandle ltFsServiceHandle;
    bool isInitilaized;
    bool connectedToLtFs;
};

FsContext gFsContext;

static Status FsInit()
{
    Status status = STATUS_FAILURE;
    IpcHandle serverHandle = INVALID_HANDLE_VALUE;
    IpcServer ipcServer;
    LtFsRequest * connectRequest = nullptr;
    LtFsConnectParameter parameter;
    unsigned int requestSize = 0;

    // TODO : find a way to generate a GUID
    const char uniqueIpcServerId[] = __TIME__;

    status = gFsContext.ipcClient.ConnectToServer(LTFS_SERVICE_NAME, &serverHandle);
    if (FAILED(status))
        return status;

    // We are also an ipc server to be able to receive responses
    status = IpcServer::Create(uniqueIpcServerId, &ipcServer);
    if (FAILED(status))
        return status;

    // We send a connect request to the LtFs service so it can connect to our ipc server
    MemCopy((void*)uniqueIpcServerId, &(parameter.ipcServerId), StrLen(uniqueIpcServerId) + 1);

    status = LtFsRequest::Create(LTFS_REQUEST_CONNECT, &parameter, sizeof(LtFsConnectParameter), &connectRequest);
    if (FAILED(status))
        return status;

    requestSize = sizeof(LtFsRequest) + sizeof(LtFsOpenFileParameters);

    gFsContext.ipcClient.Send(serverHandle, (char*)connectRequest, requestSize);
    if (FAILED(status))
    {
        if (connectRequest != nullptr)
        {
            HeapFree(connectRequest);
            connectRequest = nullptr;
        }
        return status;
    }

    gFsContext.isInitilaized = true;
    gFsContext.ltFsServiceHandle = serverHandle;
    gFsContext.ipcServer = ipcServer;

    return status;
}

Status FsOpenFile(const char * filePath, const FileAccess access, const FileShareMode shareMode, Handle * const fileHandle)
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

    if (shareMode >= FILE_SHARE_MAX)
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
        status = STATUS_PATH_TOO_LONG;
        goto clean;
    }

    parameters.shareMode = shareMode;
    parameters.access = access;

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

    {
        IpcMessage response;
        ProcessHandle clientHandle = INVALID_HANDLE_VALUE;
        LtFsResponse * ltFsResponse = nullptr;

        status = gFsContext.ipcServer.Receive(&response, &clientHandle);
        if (FAILED(status))
        {
            goto clean;
        }

        ltFsResponse = (LtFsResponse*)response.data;

        if (ltFsResponse->status != STATUS_ACCESS_DENIED)
            *fileHandle = (Handle)ltFsResponse->data;
        else
            status = ltFsResponse->status;

        // TODO : response.Release();
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
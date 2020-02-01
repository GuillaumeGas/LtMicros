#include "ServiceCommands.h"
#include "Common.h"

#include "File.h"
#include "FsManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <list.h>
#include <LtFsCommon.h>

typedef struct ProcessEntry
{
    IpcHandle serverHandle;
    IpcClient ipcClient;
    ProcessHandle processHandle;
};

static Status RequestConnect(const ProcessHandle processHandle, LtFsConnectParameter * const parameters);
static Status RequestOpenFile(const ProcessHandle processHandle, LtFsOpenFileParameters * const parameters);

static Status CreateProcessEntry(ProcessHandle processHandle, ProcessEntry ** process);
static Status LookForProcessFromHandle(ProcessHandle processHandle, ProcessEntry ** process);

List * gProcessesList;

void ServiceCommandInit()
{
    gProcessesList = ListCreate();
}

Status ServiceExecuteCommand(const ProcessHandle processHandle, char * const message, unsigned int size, bool * serviceTerminate)
{
    Status status = STATUS_FAILURE;
    LtFsRequest * request = (LtFsRequest*)message;

    if (request == nullptr)
    {
        LOG(LOG_ERROR, "Invalid message");
        return STATUS_NULL_PARAMETER;
    }

    switch (request->type)
    {
    case LTFS_REQUEST_TERMINATE:
        *serviceTerminate = true;
        status = STATUS_SUCCESS;
        break;
    case LTFS_REQUEST_CONNECT:
        status = RequestConnect(processHandle, (LtFsConnectParameter*)&(request->parameters));
        break;
    case LTFS_REQUEST_OPEN_FILE:
        status = RequestOpenFile(processHandle, (LtFsOpenFileParameters*)&(request->parameters));
        break;
    default:
        LOG(LOG_ERROR, "Invalid request type %d", request->type);
        status = STATUS_UNEXPECTED;
    }

    return status;
}

static Status RequestConnect(const ProcessHandle processHandle, LtFsConnectParameter * const parameters)
{
    Status status = STATUS_FAILURE;
    ProcessEntry * process = nullptr;

    if (parameters == nullptr)
    {
        LOG(LOG_ERROR, "Invalid parameters parameter");
        return STATUS_NULL_PARAMETER;
    }

    LOG(LOG_INFO, "Connecting to %s", parameters->ipcServerId);

    status = LookForProcessFromHandle(processHandle, &process);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "LookForProcessFromHandle() failed with code %t", status);
        goto clean;
    }

    if (process != nullptr)
    {
        LOG(LOG_DEBUG, "This process is already registered !");
        // not a fatal error
        status = STATUS_SUCCESS;
        goto clean;
    }

    status = CreateProcessEntry(processHandle, &process);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "CreateProcessEntry() failed with code %t", status);
        goto clean;
    }

    status = process->ipcClient.ConnectToServer(parameters->ipcServerId, &process->serverHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::ConnectToServer() failed with code %t (id : %s)", parameters->ipcServerId);
        goto clean;
    }

    ListPush(gProcessesList, process);
    process = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (process != nullptr)
    {
        HeapFree(process);
        process = nullptr;
    }

    return status;
}

static Status RequestOpenFile(const ProcessHandle processHandle, LtFsOpenFileParameters * const parameters)
{
    Status status = STATUS_FAILURE;
    File * file = nullptr;
    ProcessEntry * process = nullptr;

    if (parameters == nullptr)
    {
        LOG(LOG_ERROR, "Invalid parameters parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = LookForProcessFromHandle(processHandle, &process);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "LookForProcessFromHandle() failed with code %t", status);
        goto clean;
    }

    if (process == nullptr)
    {
        LOG(LOG_ERROR, "Process not found !");
        status = STATUS_UNEXPECTED;
        goto clean;
    }

    LOG(LOG_INFO, "Openning %s", parameters->filePath);

    RaiseThreadPriority();

    //__debugbreak();

    status = OpenFileFromName(parameters->filePath, &file);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "OpenFileFromName() failed with code %t", status);
        goto clean;
    }

    LOG(LOG_INFO, "File openned ! Send its content...");

    // send the response
    status = process->ipcClient.Send(process->serverHandle, (char*)file->content, StrLen((char*)file->content) + 1);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::Send() failed with code %t", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    LowerThreadPriority();

    return status;
}

static Status CreateProcessEntry(ProcessHandle processHandle, ProcessEntry ** process)
{
    Status status = STATUS_FAILURE;
    ProcessEntry * localProcess = nullptr;

    if (processHandle == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid processHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (process == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    localProcess = (ProcessEntry*)HeapAlloc(sizeof(ProcessEntry));
    if (localProcess == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(ProcessEntry));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    localProcess->processHandle = processHandle;

    *process = localProcess;
    localProcess = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

typedef struct LOOK_FOR_PROCESS_CONTEXT
{
    ProcessHandle processHandle;
    ProcessEntry * processEntry;
};

static Status LookForProcessFromHandleCallback(void * data, void * context)
{
    ProcessEntry * entry = (ProcessEntry*)data;
    LOOK_FOR_PROCESS_CONTEXT * ctx = (LOOK_FOR_PROCESS_CONTEXT*)context;

    if (data == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid data parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (entry->processHandle == ctx->processHandle)
    {
        ctx->processEntry = entry;
        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}

static Status LookForProcessFromHandle(ProcessHandle processHandle, ProcessEntry ** process)
{
    Status status = STATUS_FAILURE;
    ProcessEntry * foundProcess = nullptr;
    LOOK_FOR_PROCESS_CONTEXT context = { 0 };

    if (processHandle == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid processHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (process == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    context.processHandle = processHandle;
    context.processEntry = nullptr;

    status = ListEnumerate(gProcessesList, LookForProcessFromHandleCallback, &context);
    if (FAILED(status) && status != STATUS_LIST_STOP_ITERATING)
    {
        LOG(LOG_ERROR, "ListEnumerate() failed with code %t", status);
        goto clean;
    }

    *process = context.processEntry;

    status = STATUS_SUCCESS;

clean:
    return status;
}
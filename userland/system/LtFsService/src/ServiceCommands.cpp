#include "ServiceCommands.h"
#include "Common.h"

#include "File.h"
#include "FsManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <list.h>
#include <LtFsCommon.h>

struct ProcessEntry
{
    IpcHandle serverHandle;
    IpcClient ipcClient;
    ProcessHandle processHandle;

    List * fileEntries;
};

struct FileEntry
{
    Handle handle;
    unsigned int cursor;
    File * file;
    FileShareMode shareMode;
};

struct ServiceCommandContext
{
    List * processes;
    List * fileEntries;
};

static Status RequestConnect(const ProcessHandle processHandle, LtFsConnectParameter * const parameters);
static Status RequestOpenFile(const ProcessHandle processHandle, LtFsOpenFileParameters * const parameters);

static Status CreateProcessEntry(ProcessHandle processHandle, ProcessEntry ** process);
static Status LookForProcessFromHandle(ProcessHandle processHandle, ProcessEntry ** process);
static Status LookForFileEntryFromFilePtr(File * file, FileEntry ** fileEntry);
static bool IsFileAccessible(FileEntry * fileEntry, FileAccess access);
static Status NewFileEntry(File * file, FileAccess access, FileShareMode shareMode, FileEntry ** newFileEntry);

ServiceCommandContext gSvcContext = { 0 };
Handle gCurrentFileHandle = INVALID_HANDLE_VALUE;

void ServiceCommandInit()
{
    gSvcContext.processes = ListCreate();
    gSvcContext.fileEntries = ListCreate();
    gCurrentFileHandle = 0;
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

    ListPush(gSvcContext.processes, process);
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
    Status resStatus = STATUS_SUCCESS;
    File * file = nullptr;
    FileEntry * existingFileEntry = nullptr;
    ProcessEntry * process = nullptr;
    LtFsResponse * response = nullptr;

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

    status = OpenFileFromName(parameters->filePath, &file);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "OpenFileFromName() failed with code %t", status);
        goto clean;
    }

    status = LookForFileEntryFromFilePtr(file, &existingFileEntry);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "LookForFileEntryFromFilePtr() failed with code %t", status);
        goto clean;
    }

    // If a file has been found, we check if it may be shared
    if (existingFileEntry != nullptr)
    {
        if (IsFileAccessible(existingFileEntry, parameters->access) == false)
        {
            resStatus = STATUS_ACCESS_DENIED;
        }
    }

    if (resStatus == STATUS_ACCESS_DENIED)
    {
        status = LtFsResponse::Create(resStatus, nullptr, 0, &response);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "LtFsResponse::Create() failed with code %t", status);
            goto clean;
        }

        LOG(LOG_INFO, "Access denied !");
    }
    else
    {
        FileEntry * newFileEntry = nullptr;

        status = NewFileEntry(file, parameters->access, parameters->shareMode, &newFileEntry);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "NewFileEntry() failed with code %t", status);
            goto clean;
        }

        ListPush(gSvcContext.fileEntries, newFileEntry);
        ListPush(process->fileEntries, newFileEntry);

        status = LtFsResponse::Create(resStatus, &newFileEntry->handle, sizeof(Handle), &response);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "LtFsResponse::Create() failed with code %t", status);
            goto clean;
        }

        LOG(LOG_INFO, "File openned ! Sending response...");
    }


    LOG(LOG_DEBUG, "Sending message of size %d", response->size);

    // send the response
    status = process->ipcClient.Send(process->serverHandle, (char*)response, response->size);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::Send() failed with code %t", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    LowerThreadPriority();

    /*
        TODO : free memory
    */

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

struct LOOK_FOR_PROCESS_CONTEXT
{
    ProcessHandle processHandle;
    ProcessEntry * processEntry;
};

static Status LookForProcessFromHandleCallback(void * data, void * context)
{
    ProcessEntry * entry = (ProcessEntry*)data;
    LOOK_FOR_PROCESS_CONTEXT * ctx = (LOOK_FOR_PROCESS_CONTEXT*)context;

    if (data == nullptr)
    {
        LOG(LOG_ERROR, "Invalid data parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
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
    LOOK_FOR_PROCESS_CONTEXT context = { 0 };

    if (processHandle == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid processHandle parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (process == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid process parameter");
        return STATUS_NULL_PARAMETER;
    }

    context.processHandle = processHandle;
    context.processEntry = nullptr;

    status = ListEnumerate(gSvcContext.processes, LookForProcessFromHandleCallback, &context);
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

struct LOOK_FOR_FILE_ENTRY_CONTEXT
{
    File * file;
    FileEntry * fileEntry;
};

static Status LookForFileEntryFromFilePtrCallback(void * elem, void * context)
{
    LOOK_FOR_FILE_ENTRY_CONTEXT * ctx = (LOOK_FOR_FILE_ENTRY_CONTEXT*)context;
    FileEntry * entry = (FileEntry*)elem;

    if (elem == nullptr)
    {
        LOG(LOG_ERROR, "Invalid elem parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
    {
        LOG(LOG_ERROR, "Invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (entry->file == ctx->file)
    {
        ctx->fileEntry = entry;
        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}

static Status LookForFileEntryFromFilePtr(File * file, FileEntry ** fileEntry)
{
    Status status = STATUS_FAILURE;
    LOOK_FOR_FILE_ENTRY_CONTEXT Context = { 0 };

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (fileEntry == nullptr)
    {
        LOG(LOG_ERROR, "Invalid fileEntry parameter");
        return STATUS_NULL_PARAMETER;
    }

    Context.file = file;
    Context.fileEntry = nullptr;

    LOG(LOG_DEBUG, "Looking for %x in list %x", file, gSvcContext.fileEntries);

    status = ListEnumerate(gSvcContext.fileEntries, LookForFileEntryFromFilePtrCallback, &Context);
    if (FAILED(status) && status != STATUS_LIST_STOP_ITERATING)
    {
        LOG(LOG_ERROR, "ListEnumerate() failed with code %t", status);
        goto clean;
    }

    *fileEntry = Context.fileEntry;

    status = STATUS_SUCCESS;

clean:
    return status;
}

static bool IsFileAccessible(FileEntry * fileEntry, FileAccess access)
{
    if (access == FILE_READ)
    {
        if (FlagOn(FILE_SHARE_READ, fileEntry->shareMode) == false)
        {
            return false;
        }
    }

    return true;
}

static Status NewFileEntry(File * file, FileAccess access, FileShareMode shareMode, FileEntry ** newFileEntry)
{
    Status status = STATUS_FAILURE;
    FileEntry * fileEntry = nullptr;

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (access >= FILE_ACCESS_MAX)
    {
        LOG(LOG_ERROR, "Invalid access parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (shareMode >= FILE_SHARE_MAX)
    {
        LOG(LOG_ERROR, "Invalid shareMode parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (newFileEntry == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    fileEntry = (FileEntry*)HeapAlloc(sizeof(FileEntry));
    if (fileEntry == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(FileEntry));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    fileEntry->cursor = 0;
    fileEntry->file = file;
    fileEntry->handle = ++gCurrentFileHandle;
    fileEntry->shareMode = shareMode;

    *newFileEntry = fileEntry;
    fileEntry = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}
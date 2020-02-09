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
    Handle processHandle;

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

static Status RequestConnect(IpcServer * const server, const Handle clientProcess);
static Status RequestOpenFile(IpcServer * const server, const Handle clientProcess);

static Status CreateProcessEntry(Handle processHandle, ProcessEntry ** process);
static Status LookForProcessFromHandle(Handle processHandle, ProcessEntry ** process);
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

Status ServiceExecuteCommand(IpcServer * const server, const Handle clientProcess, const LtFsRequestType requestType, bool * serviceTerminate)
{
    Status status = STATUS_FAILURE;

    if (server == nullptr)
    {
        LOG(LOG_ERROR, "Invalid server parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (clientProcess == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid clientProcess parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (serviceTerminate == nullptr)
    {
        LOG(LOG_ERROR, "Invalid serviceTerminate parameter");
        return STATUS_NULL_PARAMETER;
    }

    switch (requestType)
    {
    case LTFS_REQUEST_TERMINATE:
        *serviceTerminate = true;
        status = STATUS_SUCCESS;
        break;
    case LTFS_REQUEST_CONNECT:
        status = RequestConnect(server, clientProcess);
        break;
    case LTFS_REQUEST_OPEN_FILE:
        status = RequestOpenFile(server, clientProcess);
        break;
    default:
        LOG(LOG_ERROR, "Invalid request type %d", requestType);
        status = STATUS_UNEXPECTED;
    }

    return status;
}

static Status RequestConnect(IpcServer * const server, const Handle clientProcess)
{
    Status status = STATUS_FAILURE;
    ProcessEntry * process = nullptr;
    LtFsConnectParameter parameters = { 0 };

    if (server == nullptr)
    {
        LOG(LOG_ERROR, "Invalid server parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (clientProcess == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid clientProcess parameter");
        return STATUS_INVALID_PARAMETER;
    }

    status = LookForProcessFromHandle(clientProcess, &process);
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

    status = CreateProcessEntry(clientProcess, &process);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "CreateProcessEntry() failed with code %t", status);
        goto clean;
    }

    {
        unsigned int readBytes = 0;
        Handle processHandle;

        status = server->Receive((char*)&parameters, sizeof(LtFsConnectParameter), &readBytes, &processHandle);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "IpcServer::Receive() failed with code %t", status);
            goto clean;
        }
    }

    status = process->ipcClient.ConnectToServer(parameters.ipcServerId, &process->serverHandle);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::ConnectToServer() failed with code %t (id : %s)", parameters.ipcServerId);
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

static Status RequestOpenFile(IpcServer * const server, const Handle clientProcess)
{
    Status status = STATUS_FAILURE;
    Status resStatus = STATUS_SUCCESS;
    File * file = nullptr;
    FileEntry * existingFileEntry = nullptr;
    ProcessEntry * process = nullptr;
    LtFsResponse * response = nullptr;
    LtFsOpenFileParameters parameters = { 0 };
    Handle fileHandle = INVALID_HANDLE_VALUE;

    if (server == nullptr)
    {
        LOG(LOG_ERROR, "Invalid server parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (clientProcess == INVALID_HANDLE_VALUE)
    {
        LOG(LOG_ERROR, "Invalid clientProcess parameter");
        return STATUS_INVALID_PARAMETER;
    }

    status = LookForProcessFromHandle(clientProcess, &process);
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

    {
        unsigned int readBytes;
        Handle processHandle;
        status = server->Receive((char*)&parameters, sizeof(LtFsOpenFileParameters), &readBytes, &processHandle);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "IpcServer::Receive() failed with code %t", status);
            goto clean;
        }
    }

    RaiseThreadPriority();

    status = OpenFileFromName(parameters.filePath, &file);
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
        if (IsFileAccessible(existingFileEntry, parameters.access) == false)
        {
            resStatus = STATUS_ACCESS_DENIED;
        }
    }

    if (resStatus == STATUS_ACCESS_DENIED)
    {
        LOG(LOG_INFO, "Access denied !");
    }
    else
    {
        FileEntry * newFileEntry = nullptr;

        status = NewFileEntry(file, parameters.access, parameters.shareMode, &newFileEntry);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "NewFileEntry() failed with code %t", status);
            goto clean;
        }

        ListPush(gSvcContext.fileEntries, newFileEntry);
        ListPush(process->fileEntries, newFileEntry);

        fileHandle = newFileEntry->handle;

        LOG(LOG_INFO, "File openned ! Sending response...");
    }

    // Sending return status
    status = process->ipcClient.Send(process->serverHandle, (char*)&resStatus, sizeof(Status));
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "IpcClient::Send() failed with code %t", status);
        goto clean;
    }

    if (FAILED(response->status) == false)
    {
        status = process->ipcClient.Send(process->serverHandle, (char*)&fileHandle, sizeof(Handle));
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "IpcClient::Send() failed with code %t", status);
            goto clean;
        }
    }

    status = STATUS_SUCCESS;

clean:
    LowerThreadPriority();

    /*
        TODO : free memory
    */

    return status;
}

static Status CreateProcessEntry(Handle processHandle, ProcessEntry ** process)
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
    Handle processHandle;
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

static Status LookForProcessFromHandle(Handle processHandle, ProcessEntry ** process)
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
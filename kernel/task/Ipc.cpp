#define __IPC_HANDLER__
#include "Ipc.hpp"

#include <kernel/Logger.hpp>
#include <kernel/lib/StdMem.hpp>
#include <kernel/lib/StdLib.hpp>

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("IPC", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("IPC", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

struct IpcObject
{
    /// String identifying the server process
    char * id;
    /// Handle identifying the Ipc object
    IpcHandle handle;
    /// Pointer to the server process
    Process * serverProcess;
    /// List of messages
    List * messages;

    static KeStatus Create(const char * serverIdStr, const Process * serverProcess, const IpcHandle handle, IpcObject** const ipcObject);
};

struct FIND_IPC_OBJECTS_CONTEXT
{
    char * serverId;
    IpcObject* ipcObject;
    IpcHandle ipcHandle;
    bool found;
};

static void FindIpcObjectByHandleCallback(void* ipcObjectPtr, void* context);
static void FindIpcObjectByServerIdCallback(void * ipcObjectPtr, void * context);

static IpcHandle s_IpcObjectHandleCount = IPC_INVALID_HANDLE;

void IpcHandler::Init()
{
    _ipcObjects = ListCreate();
}

KeStatus IpcHandler::AddNewServer(const char * serverIdStr, const Process* serverProcess, IpcHandle* const handle)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * ipcObject = nullptr;
    IpcHandle ipcObjectHandle = IPC_INVALID_HANDLE;

    if (serverIdStr == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverIdStr parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (serverProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (handle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid handle parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (_IsServerIdStrAlreadyUsed(serverIdStr))
    {
        KLOG(LOG_DEBUG, "The id string '%s' is already used", serverIdStr);
        return IPC_STATUS_ID_STRING_ALREADY_USED;
    }

    ipcObjectHandle = ++s_IpcObjectHandleCount;

    status = IpcObject::Create(serverIdStr, serverProcess, ipcObjectHandle, &ipcObject);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "IpcObject::Create() failed with code %d", status);
        goto clean;
    }

    ListPush(_ipcObjects, ipcObject);

    *handle = ipcObjectHandle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcHandler::ConnectToServer(const char* serverIdStr, const Process * clientProcess, IpcHandle* const ipcHandle)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject* ipcObject = nullptr;

    // TODO : check if the clientProcess is authorized to connect to this server

    if (serverIdStr == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverIdStr parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (clientProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid clientProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (ipcHandle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid ipcHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    ipcObject = _FindIpcObjectByServerId(serverIdStr);
    if (ipcObject == nullptr)
    {
        KLOG(LOG_DEBUG, "Server '%s' not found", serverIdStr);
        status = IPC_STATUS_SERVER_NOT_FOUND;
        goto clean;
    }

    *ipcHandle = ipcObject->handle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcHandler::Send(const IpcHandle handle, const Process* clientProcess, const char* message, const unsigned int size)
{
    /*
        - Aller chercher l'ipc object à partir du handle
        - Allouer la taille nécessaire dans le processus serveur
        - Y copier les données
        - Créer un object ipc message contenant le pointeur vers le buffer en question
    */

    return STATUS_SUCCESS;
}

KeStatus IpcHandler::Receive(const IpcHandle handle, const Process* serverProcess, const char** message, const unsigned int* size)
{
    return STATUS_SUCCESS;
}

KeStatus IpcHandler::ReleaseMemory(const Process* process, void* ptr) {
    return STATUS_SUCCESS;
}

KeStatus IpcHandler::_AllocateMemory(const Process* process, unsigned int size, char** const buffer)
{
    return STATUS_SUCCESS;
}

IpcObject* IpcHandler::_FindIpcObjectByHandle(const IpcHandle handle) const
{
    FIND_IPC_OBJECTS_CONTEXT context;

    if (handle == IPC_INVALID_HANDLE)
    {
        KLOG(LOG_ERROR, "Invalid handle");
        return nullptr;
    }

    context.serverId = nullptr;
    context.ipcObject = nullptr;
    context.ipcHandle = handle;
    context.found = false;

    ListEnumerate(_ipcObjects, FindIpcObjectByHandleCallback, &context);

    return context.ipcObject;
}

IpcObject* IpcHandler::_FindIpcObjectByServerId(const char * serverId) const
{
    FIND_IPC_OBJECTS_CONTEXT context;

    if (serverId == nullptr)
    {
        KLOG(LOG_ERROR, "serverId handle");
        return nullptr;
    }

    context.serverId = (char*)serverId;
    context.ipcObject = nullptr;
    context.ipcHandle = IPC_INVALID_HANDLE;
    context.found = false;

    ListEnumerate(_ipcObjects, FindIpcObjectByServerIdCallback, &context);

    return context.ipcObject;
}

bool IpcHandler::_IsServerIdStrAlreadyUsed(const char * serverIdStr) const
{
    FIND_IPC_OBJECTS_CONTEXT context;

    if (serverIdStr == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverIdStr");
        return true;
    }

    context.serverId = (char*)serverIdStr;
    context.found = false;

    ListEnumerate(_ipcObjects, FindIpcObjectByServerIdCallback, &context);

    return context.found;
}

static void FindIpcObjectByHandleCallback(void* ipcObjectPtr, void* context)
{
    IpcObject* ipcObject = (IpcObject*)ipcObjectPtr;
    FIND_IPC_OBJECTS_CONTEXT* ipcContext = (FIND_IPC_OBJECTS_CONTEXT*)context;

    if (ipcObjectPtr == nullptr)
    {
        KLOG(LOG_ERROR, "invalid ipcObjectPtr parameter");
        return;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "invalid context parameter");
        return;
    }

    if (ipcObject->handle == ipcContext->ipcHandle)
    {
        ipcContext->found = true;
        ipcContext->ipcObject = ipcObject;
    }
}

static void FindIpcObjectByServerIdCallback(void * ipcObjectPtr, void * context)
{
    IpcObject * ipcObject = (IpcObject*)ipcObjectPtr;
    FIND_IPC_OBJECTS_CONTEXT * ipcContext = (FIND_IPC_OBJECTS_CONTEXT*)context;

    if (ipcObjectPtr == nullptr)
    {
        KLOG(LOG_ERROR, "invalid ipcObjectPtr parameter");
        return;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "invalid context parameter");
        return;
    }

    if (StrCmp(ipcObject->id, ipcContext->serverId) == 0)
    {
        ipcContext->found = true;
        ipcContext->ipcObject = ipcObject;
    }
}

KeStatus IpcObject::Create(const char * serverIdStr, const Process * serverProcess, const IpcHandle handle, IpcObject** const ipcObject)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * object = nullptr;

    if (serverIdStr == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverIdStr parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (serverProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (ipcObject == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid ipcObject parameter");
        return STATUS_NULL_PARAMETER;
    }

    object = (IpcObject*)HeapAlloc(sizeof(IpcObject));
    if (object == nullptr)
    {
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    object->handle = handle;
    object->id = (char *)serverIdStr;
    object->messages = ListCreate();
    object->serverProcess = (Process *)serverProcess;

    *ipcObject = object;
    object = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}
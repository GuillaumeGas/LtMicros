#define __IPC_HANDLER__
#include "Ipc.hpp"

#include <kernel/Logger.hpp>
#include <kernel/lib/StdMem.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/CriticalSection.hpp>
#include <kernel/Kernel.hpp>
#include <kernel/task/ProcessManager.hpp>
#include <kernel/task/Event.hpp>
#include <kernel/drivers/Clock.hpp>
#include <kernel/handle/HandleManager.h>

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
    /// Event signaled when a message is ready to be read
    Event messageAvailableEvent;
    /// Critical section used to protect the ipcObject
    CriticalSection criticalSection;

    static KeStatus Create(const char * serverIdStr, Process * const serverProcess, const IpcHandle handle, IpcObject** const ipcObject);
};

struct IpcMessage
{
    /// The sender process
    Handle clientProcess;
    /// Pointer to the message content
    char * message;
    /// The message size
    unsigned int size;
    /// The first page containing the message
    Page firstPage;

    static KeStatus Create(const Handle clientProcess, char * const message, const unsigned int size, IpcMessage** const ipcMessage);
};

struct FIND_IPC_OBJECTS_CONTEXT
{
    char * serverId;
    IpcObject* ipcObject;
    IpcHandle ipcHandle;
    bool found;
};

static KeStatus FindIpcObjectByHandleCallback(void* ipcObjectPtr, void* context);
static KeStatus FindIpcObjectByServerIdCallback(void * ipcObjectPtr, void * context);

static IpcHandle s_IpcObjectHandleCount = INVALID_HANDLE_VALUE;

void IpcHandler::Init()
{
    _ipcObjects = ListCreate();
}

KeStatus IpcHandler::AddNewServer(const char * serverIdStr, Process* const serverProcess, IpcHandle* const handle)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * ipcObject = nullptr;
    IpcHandle ipcObjectHandle = INVALID_HANDLE_VALUE;

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
        KLOG(LOG_ERROR, "IpcObject::Create() failed with code %t", status);
        goto clean;
    }

    ListPush(_ipcObjects, ipcObject);

    *handle = ipcObjectHandle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcHandler::ConnectToServer(const char* serverIdStr, Process * const clientProcess, IpcHandle* const ipcHandle)
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

KeStatus IpcHandler::Send(const IpcHandle handle, Process* const clientProcess, const char* message, const unsigned int size)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * ipcObject = nullptr;
    IpcMessage * ipcMessage = nullptr;
    Process * serverProcess = nullptr;
    u8 * serverBuffer = nullptr;
    u8 * kernelBuffer = nullptr;
    Handle clientProcessHandle = INVALID_HANDLE_VALUE;

    if (handle == 0)
    {
        KLOG(LOG_ERROR, "Invalid handle parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (clientProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid clientProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (message == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid message parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_INVALID_PARAMETER;
    }

    DKLOG(LOG_DEBUG, "Handling message from %s, msg addr : %x, size : %d", clientProcess->name, message, size);

    ipcObject->criticalSection.Enter();

    ipcObject = _FindIpcObjectByHandle(handle);
    if (ipcObject == nullptr)
    {
        KLOG(LOG_DEBUG, "Didn't found ipc object for handle %d", handle);
        status = IPC_STATUS_SERVER_NOT_FOUND;
        goto clean;
    }

    // A ipc server can't send a message, but only receive one
    if (clientProcess == ipcObject->serverProcess)
    {
        KLOG(LOG_DEBUG, "A ipc server tried to send a message");
        status = IPC_STATUS_ACCESS_DENIED;
        goto clean;
    }

    status = gHandleManager.FindOrCreate(HandleType::PROCESS_HANDLE, clientProcess, &clientProcessHandle);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "HandleManager::FindOrCreate() failed with code %t", status);
        goto clean;
    }

    KLOG(LOG_DEBUG, "Writing handle %d", clientProcessHandle);

    // TODO : if the send message pages may be paged, we should make these pages unpageable so we can map them in the receive function

    status = IpcMessage::Create(clientProcessHandle, (char*)serverBuffer, size, &ipcMessage);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "IpcMessage::Create() failed with code %t", status);
        gKernel.Panic();
    }

    ipcObject->criticalSection.Enter();

    ListPush(ipcObject->messages, ipcMessage);
    EventSignal(&ipcObject->messageAvailableEvent);

    ipcObject->criticalSection.Leave();

    status = STATUS_SUCCESS;

clean:
    ipcObject->criticalSection.Leave();

    if (kernelBuffer != nullptr)
    {
        HeapFree(kernelBuffer);
        kernelBuffer = nullptr;
    }

    return status;
}

/* TODO : find a way to implemet mod */
static unsigned int _local_mod(unsigned int a, unsigned int b)
{
    while (a > b)
        a -= b;
    return (a < b) ? 1 : 0;
}

KeStatus IpcHandler::Receive(const IpcHandle handle, Process* const serverProcess, char * const buffer, const unsigned int size, unsigned int * const bytesRead, Handle * clientProcessHandle)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * ipcObject = nullptr;
    IpcMessage * ipcMessage = nullptr;
    unsigned int localBytesRead = 0;
    Handle localProcessHandle = INVALID_HANDLE_VALUE;

    if (handle == 0)
    {
        KLOG(LOG_ERROR, "Invalid handle parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (serverProcess == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid serverProcess parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (buffer == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid buffer parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        KLOG(LOG_ERROR, "Invalid size parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (bytesRead == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid bytesRead parameter");
        return STATUS_NULL_PARAMETER;
    }

    ipcObject = _FindIpcObjectByHandle(handle);
    if (ipcObject == nullptr)
    {
        KLOG(LOG_DEBUG, "Didn't found ipc object for handle %d", handle);
        return IPC_STATUS_SERVER_NOT_FOUND;
    }

    // An ipc client can't receive a message, but only send one
    if (serverProcess != ipcObject->serverProcess)
    {
        KLOG(LOG_DEBUG, "An ipc client tried to receive a message");
        status = IPC_STATUS_ACCESS_DENIED;
    }

    // We determine if a message is available
    {
        bool messageAvailable = false;

        ipcObject->criticalSection.Enter();
        messageAvailable = ListIsEmpty(ipcObject->messages) == false;
        ipcObject->criticalSection.Leave();

        if (!messageAvailable)
            EventWait(&ipcObject->messageAvailableEvent);
    }

    ipcObject->criticalSection.Enter();
        
    ipcMessage = (IpcMessage*)ListPop(&ipcObject->messages);
    if (ipcMessage == nullptr)
    {
        KLOG(LOG_ERROR, "Unexpected null ipc message");
        status = STATUS_UNEXPECTED;
        ipcObject->criticalSection.Leave();
        goto clean;
    }

    /*
        - Allouer x pages de mémoire virtuelle dans le noyau (ou dans le processus vu qu'on a pas ce qu'il faut en noyau je crois)
        - Y mapper les pages physiques
        - Effectuer la copie
        - Désallouer
    */

    {
        unsigned int calculatedSize = 0;
        unsigned int nbPages = 0;
        Vad * vad = nullptr;
        u32 vAddr = 0;
        u32 pAddr = 0;

        nbPages = ipcMessage->size / PAGE_SIZE;
        if (_local_mod(ipcMessage->size, PAGE_SIZE) > 0)
            nbPages++;

        calculatedSize = nbPages * PAGE_SIZE;

        status = serverProcess->baseVad->Allocate(calculatedSize, &serverProcess->pageDirectory, false, &vad);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "Vad::Allocate() failed with code %t (size : %d)", status, calculatedSize);
            ipcObject->criticalSection.Leave();
            goto clean;
        }

        vAddr = (u32)vad->baseAddress;
        pAddr = (u32)ipcMessage->firstPage.pAddr;

        for (unsigned int index = 0; index < nbPages; index++)
        {
            gVmm.AddPageToPageDirectory(vAddr, pAddr, PAGE_PRESENT | PAGE_WRITEABLE, serverProcess->pageDirectory);

            vAddr += PAGE_SIZE;
            pAddr += PAGE_SIZE;
        }

        MemCopy((char*)vAddr, buffer, ipcMessage->size);

        vad->Release();
    }

    ipcObject->criticalSection.Leave();

    *bytesRead = localBytesRead;
    *clientProcessHandle = localProcessHandle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus IpcHandler::ReleaseMemory(Process* const process, void* ptr) {
    return STATUS_SUCCESS;
}

KeStatus IpcHandler::_AllocateMemory(Process* const process, unsigned int size, char** const buffer)
{
    return STATUS_SUCCESS;
}

IpcObject* IpcHandler::_FindIpcObjectByHandle(const IpcHandle handle) const
{
    FIND_IPC_OBJECTS_CONTEXT context;

    if (handle == INVALID_HANDLE_VALUE)
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
    context.ipcHandle = INVALID_HANDLE_VALUE;
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

static KeStatus FindIpcObjectByHandleCallback(void* ipcObjectPtr, void* context)
{
    IpcObject* ipcObject = (IpcObject*)ipcObjectPtr;
    FIND_IPC_OBJECTS_CONTEXT* ipcContext = (FIND_IPC_OBJECTS_CONTEXT*)context;

    if (ipcObjectPtr == nullptr)
    {
        KLOG(LOG_ERROR, "invalid ipcObjectPtr parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    //KLOG(LOG_DEBUG, "[%d - %d]", ipcObject->handle, ipcContext->ipcHandle);

    if (ipcObject->handle == ipcContext->ipcHandle)
    {
        ipcContext->found = true;
        ipcContext->ipcObject = ipcObject;

        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}

static KeStatus FindIpcObjectByServerIdCallback(void * ipcObjectPtr, void * context)
{
    IpcObject * ipcObject = (IpcObject*)ipcObjectPtr;
    FIND_IPC_OBJECTS_CONTEXT * ipcContext = (FIND_IPC_OBJECTS_CONTEXT*)context;

    if (ipcObjectPtr == nullptr)
    {
        KLOG(LOG_ERROR, "invalid ipcObjectPtr parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (StrCmp(ipcObject->id, ipcContext->serverId) == 0)
    {
        //KLOG(LOG_DEBUG, "found %x - %x", ipcObject->id, ipcContext->serverId);

        ipcContext->found = true;
        ipcContext->ipcObject = ipcObject;

        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}

KeStatus IpcObject::Create(const char * serverIdStr, Process * const serverProcess, const IpcHandle handle, IpcObject** const ipcObject)
{
    KeStatus status = STATUS_FAILURE;
    IpcObject * object = nullptr;
    char * serverIdStrCopy = nullptr;

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

    serverIdStrCopy = (char*)HeapAlloc(StrLen(serverIdStr) + 1);
    if (object == nullptr)
    {
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    StrCpy(serverIdStr, serverIdStrCopy);

    object->handle = handle;
    object->id = serverIdStrCopy;
    object->serverProcess = (Process *)serverProcess;
    object->buffer.Init();

    *ipcObject = object;
    object = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}
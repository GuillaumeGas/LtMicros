#define __HANDLE_MANAGER__
#include "HandleManager.h"

#include <kernel/lib/StdMem.hpp>

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("HANDLE", LOG_LEVEL, format, ##__VA_ARGS__)

static Handle s_NextHandleValue = 1;

struct FIND_HANDLE_CONTEXT
{
    bool found;
    Handle foundHandle;
    void * object;
};

struct HandleObject
{
    Handle handle;
    void * object;
};

static KeStatus _FindHandleCallback(void * object, void * context);

void HandleManager::Init()
{
    _processHandleList = ListCreate();
}

KeStatus HandleManager::FindOrCreate(const HandleType type, void * object, Handle * const outHandle)
{
    KeStatus status = STATUS_FAILURE;
    Handle localHandle = INVALID_HANDLE_VALUE;

    if (type >= HANDLE_TYPE_MAX)
    {
        KLOG(LOG_ERROR, "Invalid type parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (object == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid object parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outHandle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = FindFromObjectPtr(type, object, &localHandle);
    if (FAILED(status) && status != STATUS_NOT_FOUND)
    {
        KLOG(LOG_ERROR, "FindFromObjectPtr() failed with code %t", status);
        goto clean;
    }

    if (status == STATUS_NOT_FOUND)
    {
        status = CreateHandle(type, object, &localHandle);
        if (status)
        {
            KLOG(LOG_ERROR, "CreateHandle() failed with code %t", status);
            goto clean;
        }
    }

    *outHandle = localHandle;
    localHandle = INVALID_HANDLE_VALUE;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus HandleManager::FindFromObjectPtr(const HandleType type, void * object, Handle * const outHandle)
{
    KeStatus status = STATUS_FAILURE;
    FIND_HANDLE_CONTEXT context;

    if (type >= HANDLE_TYPE_MAX)
    {
        KLOG(LOG_ERROR, "Invalid type parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (object == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid object parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outHandle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    context.object = object;
    context.found = false;
    context.foundHandle = INVALID_HANDLE_VALUE;

    switch (type)
    {
    case PROCESS_HANDLE:
        status = ListEnumerate(_processHandleList, _FindHandleCallback, &context);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "ListEnumerate() failed with code %t", status);
            goto clean;
        }
        break;
    default:
        KLOG(LOG_ERROR, "Invalid type %d !", type);
    }

    if (context.found == false)
    {
        status = STATUS_NOT_FOUND;
        goto clean;
    }

    *outHandle = context.foundHandle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus HandleManager::CreateHandle(const HandleType type, void * object, Handle * const outHandle)
{
    KeStatus status = STATUS_FAILURE;
    Handle localHandle = INVALID_HANDLE_VALUE;

    if (type >= HANDLE_TYPE_MAX)
    {
        KLOG(LOG_ERROR, "Invalid type parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (object == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid object parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outHandle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    status = FindFromObjectPtr(type, object, &localHandle);
    if (FAILED(status) && status != STATUS_NOT_FOUND)
    {
        KLOG(LOG_ERROR, "FindFromObjectPtr() failed with code %t", status);
        goto clean;
    }

    if (status != STATUS_NOT_FOUND)
    {
        status = STATUS_HANDLE_ALREADY_EXIST;
        goto clean;
    }

    status = _CreateHandle(type, object, &localHandle);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "_CreateHandle() failed with code %t", status);
        goto clean;
    }

    *outHandle = localHandle;
    localHandle = INVALID_HANDLE_VALUE;

    status = STATUS_SUCCESS;

clean:
    return status;
}

KeStatus HandleManager::_CreateHandle(const HandleType type, void * object, Handle * const outHandle)
{
    KeStatus status = STATUS_FAILURE;
    HandleObject * newObject = nullptr;

    if (type >= HANDLE_TYPE_MAX)
    {
        KLOG(LOG_ERROR, "Invalid type parameter");
        return STATUS_INVALID_PARAMETER;
    }

    if (object == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid object parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (outHandle == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid outHandle parameter");
        return STATUS_NULL_PARAMETER;
    }

    newObject = (HandleObject*)HeapAlloc(sizeof(HandleObject));
    if (newObject == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(HandleObject));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    newObject->object = object;
    newObject->handle = s_NextHandleValue++;

    switch (type)
    {
    case PROCESS_HANDLE:
        ListPush(_processHandleList, newObject);
        break;
    default:
        KLOG(LOG_ERROR, "Invalid type %d !", type);
    }

    *outHandle = newObject->handle;

    status = STATUS_SUCCESS;

clean:
    return status;
}

static KeStatus _FindHandleCallback(void * data, void * context)
{
    HandleObject * handleObject = (HandleObject*)data;
    FIND_HANDLE_CONTEXT * findContext = (FIND_HANDLE_CONTEXT*)context;

    if (data == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid data parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (handleObject->object == findContext->object)
    {
        findContext->found = true;
        findContext->foundHandle = handleObject->handle;

        return STATUS_LIST_STOP_ITERATING;
    }

    return STATUS_SUCCESS;
}
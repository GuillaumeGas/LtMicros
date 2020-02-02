#include "LtFsCommon.h"

#include <status.h>
#include <malloc.h>
#include <stdlib.h>

#include <stdio.h>

Status LtFsRequest::Create(const LtFsRequestType type, void * const parameters, const unsigned int size, LtFsRequest ** outRequest)
{
    Status status = STATUS_FAILURE;
    LtFsRequest * request = nullptr;

    if (type >= LTFS_REQUEST_MAX)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (parameters == nullptr)
    {
        return STATUS_NULL_PARAMETER;
    }

    if (size == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (outRequest == nullptr)
    {
        return STATUS_NULL_PARAMETER;
    }

    request = (LtFsRequest*)HeapAlloc(sizeof(LtFsRequest) + size);
    if (request == nullptr)
    {
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    request->type = type;
    
    MemCopy(parameters, &(request->parameters), size);

    *outRequest = request;
    request = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}

Status LtFsResponse::Create(const Status resStatus, void * const data, const unsigned int size, LtFsResponse ** outResponse)
{
    Status status = STATUS_FAILURE;
    LtFsResponse * res = nullptr;

    if (data == nullptr && size != 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (outResponse == nullptr)
    {
        return STATUS_INVALID_PARAMETER;
    }

    res = (LtFsResponse*)HeapAlloc(sizeof(LtFsResponse) + size);
    if (res == nullptr)
    {
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    res->size = sizeof(LtFsResponse) + size;
    res->status = resStatus;

    if (size > 0)
        MemCopy(data, &res->data, size);

    *outResponse = res;
    res = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}
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

    printf("request : %x\n", request);

    request->type = type;
    
    MemCopy(parameters, &(request->parameters), size);

    printf("(2) source : %x, dest : %x\n", parameters, &(request->parameters));

    *outRequest = request;
    request = nullptr;

    status = STATUS_SUCCESS;

clean:
    return status;
}
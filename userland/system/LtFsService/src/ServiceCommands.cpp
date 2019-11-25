#include "ServiceCommands.h"
#include "Common.h"

#include "File.h"
#include "FsManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <LtFsCommon.h>

static Status RequestOpenFile(LtFsOpenFileParameters * const parameters);

Status ServiceExecuteCommand(char * const message, unsigned int size, bool * serviceTerminate)
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
    case LTFS_REQUEST_OPEN_FILE:
        status = RequestOpenFile((LtFsOpenFileParameters*)&(request->parameters));
        break;
    default:
        LOG(LOG_ERROR, "Invalid request type %d", request->type);
        status = STATUS_UNEXPECTED;
    }

    return status;
}

static Status RequestOpenFile(LtFsOpenFileParameters * const parameters)
{
    Status status = STATUS_FAILURE;
    File * file = nullptr;

    if (parameters == nullptr)
    {
        LOG(LOG_ERROR, "Invalid parameters parameter");
        return STATUS_NULL_PARAMETER;
    }

    LOG(LOG_DEBUG, "Openning %s", parameters->filePath);

    status = OpenFileFromName(parameters->filePath, &file);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "OpenFileFromName() failed with code %t", status);
        goto clean;
    }

    LOG(LOG_INFO, "File openned !");

    status = STATUS_SUCCESS;

clean:
    return status;
}
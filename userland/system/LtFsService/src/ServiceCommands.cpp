#include "ServiceCommands.h"
#include "Common.h"

#include <stdio.h>
#include <stdlib.h>
#include <LtFsCommon.h>

Status ServiceExecuteCommand(char * const message, unsigned int size, bool * serviceTerminate)
{
    LOG(LOG_INFO, "Received message '%s' of length %d", message, size);

    if (StrCmp(message, SERVICE_TERMINATE_CMD) == 0)
    {
        *serviceTerminate = true;
    }

    return STATUS_SUCCESS;
}
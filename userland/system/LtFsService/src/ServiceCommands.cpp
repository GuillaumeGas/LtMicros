#include "ServiceCommands.h"

#include <stdio.h>
#include <stdlib.h>
#include <LtFsCommon.h>

Status ServiceExecuteCommand(char * const message, unsigned int size, bool * serviceTerminate)
{
    printf("[FS] Received message '%s' of length %d\n", message, size);

    if (StrCmp(message, SERVICE_TERMINATE_CMD) == 0)
    {
        *serviceTerminate = true;
    }

    return STATUS_SUCCESS;
}
#include "ata.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <proc_io.h>

#include <syscalls.h>

void main()
{
    printf("# Starting Ata driver...\n");

    //AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    //if (!AtaInit(&device))
    //{
    //    printf("Can't init ata device\n");
    //}
    //else
    //{
    //    printf("[AtaDrv] Ata device initialized !\n");
    //}

    {
        int handle = 0;
        int res = _ipcServerCreate("__LtFsServer__", &handle);
        if (res != 0)
        {
            printf("IpcServerCreate() failed with code %d\n", res);
        }
        else
        {
            printf("[AtaDrv] Ipc server created (handle %d)\n", handle);
        }

        {
            char * newMessage = nullptr;
            unsigned int newSize = 0;

            res = _ipcReceive(handle, &newMessage, &newSize);
            if (res != 0)
            {
                printf("_ipcReceive() failed with code %d\n", res);
            }
            else
            {
                printf("[AtaDrv] Received message %s of size %d\n", newMessage, newSize);
            }
        }
    }

    while (1);
}
#include <stdio.h>
#include <stdlib.h>
#include <proc_io.h>

#include <syscalls.h>

void main()
{
    printf("# Starting Init process\n");

    {
        int handle = 0;
        int res = 0;

        do
        {
            res = _ipcServerConnect("__LtFsServer__", &handle);
            if (res == 0)
            {
                printf("[Init] Connected to Ipc server (handle %d)\n", handle);

                const char * message = "Hello";
                const unsigned int size = 6;

                printf("[Init] Sending message...\n");
                res = _ipcSend(handle, message, size);
                if (res != 0)
                {
                    printf("_ipcSend() failed with code %d\n", res);
                }
                else
                {
                    printf("[Init] Message sent !\n");
                }
            }
        } while (res != 0);
    }

    while (1);
}
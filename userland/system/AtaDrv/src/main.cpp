#include "ata.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <proc_io.h>

#include <syscalls.h>

void main()
{
    printf("Starting Ata driver...\n");

    //AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    //if (!AtaInit(&device))
    //{
    //    printf("Can't init ata device\n");
    //}
    //else
    //{
    //    printf("Ata device initialized !\n");
    //}

    //printf("Test : \n");
    //int * res = (int*)HeapAlloc(512);
    //*res = 42;
    //printf("res : %d\n", *res);
    //int * test = (int*)HeapAlloc(1000);
    //*test = 43;
    //printf("test : %d\n", *test);
    //int * test2 = (int*)HeapAlloc(10);
    //*test2 = 45;
    //printf("test2 : %d\n", *test2);
    //HeapFree(test2);
    //test2 = (int*)HeapAlloc(10);
    //printf("test2 : %d\n", *test2);

    {
        int handle = 0;
        int res = _ipcServerCreate("__LtFsServer__", &handle);
        if (res != 0)
        {
            printf("IpcServerCreate() failed with code %d\n", res);
        }
        else
        {
            printf("Ipc server created (handle %d)\n", handle);
        }
    }

    {
        int handle = 0;
        int res = _ipcServerConnect("__LtFsServer__", &handle);
        if (res != 0)
        {
            printf("_ipcServerConnect() failed with code %d\n", res);
        }
        else
        {
            printf("Connected to Ipc server (handle %d)\n", handle);

            const char * message = "Hello";
            const unsigned int size = 6;

            printf("Sending message...");
            res = _ipcSend(handle, message, size);
            if (res != 0)
            {
                printf("_ipcSend() failed with code %d\n", res);
            }
            else
            {
                printf("Ok !\n");

                char * newMessage = nullptr;
                unsigned int newSize = 0;

                printf("Kernel should write at %x\n", &newMessage);

                res = _ipcReceive(handle, &newMessage, &newSize);
                if (res != 0)
                {
                    printf("_ipcReceive() failed with code %d", res);
                }
                else
                {
                    printf("Received message %s (%x) of size %d", newMessage, newMessage, newSize);
                }
            }
        }
    }

    while (1);
}


// #####################################
//#define IPC_INVALID_HANDLE 0
//
//typedef int IpcHandle;
//typedef int IpcStatus;
//
//typedef enum IpcStatus
//{
//    IPC_STATUS_SUCCESS = 0,
//    IPC_STATUS_ERROR
//};
//
//IpcStatus IpcCreateServer(const char * IpcIdString, IpcHandle * const IpcHandle);
//IpcStatus IpcReceiveMessage(const IpcHandle handle, void * const buffer, unsigned int * const bytesRead);
//// #####################################
//
//void TestServerIpc()
//{
//    IpcHandle handle = IPC_INVALID_HANDLE;
//    IpcStatus status = IPC_STATUS_ERROR;
//    unsigned int bytes = 0;
//    char * buffer = nullptr;
//
//    status = IpcCreateServer("__LtFsServer__", &handle);
//    if (status != IPC_STATUS_SUCCESS)
//    {
//        printf("IpcCreateServer() failed with code %d\n", status);
//        return;
//    }
//
//    status = IpcReceiveMessage(handle, &buffer, &bytes);
//    if (status != IPC_STATUS_SUCCESS)
//    {
//        printf("IpcReceiveMessage() failed with code %d\n", status);
//        return;
//    }
//}
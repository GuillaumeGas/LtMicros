#include "ata.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <proc_io.h>

void main()
{
    printf("Starting Ata driver...\n");

    AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    if (!AtaInit(&device))
    {
        printf("Can't init ata device\n");
    }
    else
    {
        printf("Ata device initialized !\n");
    }

    printf("Test : \n");
    int * res = (int*)HeapAlloc(512);
    *res = 42;
    printf("res : %d\n", *res);
    int * test = (int*)HeapAlloc(1000);
    *test = 43;
    printf("test : %d\n", *test);
    int * test2 = (int*)HeapAlloc(10);
    *test2 = 45;
    printf("test2 : %d\n", *test2);
    HeapFree(test2);
    test2 = (int*)HeapAlloc(10);
    printf("test2 : %d\n", *test2);

    while (1);
}
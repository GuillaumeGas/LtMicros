#include "ata.hpp"

#include <stdio.h>
#include <proc_io.h>

#include <syscalls.h>

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

    int * res = (int*)_sbrk(1);
    *res = 42;
    printf("test : %d\n", *res);
    *res = 43;
    printf("test : %d\n", *res);

    while (1);
}
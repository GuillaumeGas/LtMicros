#include "ata.hpp"

#include <stdio.h>
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

    while (1);
}
#include "ata.hpp"

#include <stdio.h>
#include <proc_io.h>

void main()
{
    Print("Starting Ata driver...\n");

    AtaDevice device = AtaCreate(ATA_SECONDARY, ATA_MASTER);
    if (!AtaInit(&device))
    {
        Print("Can't init ata device\n");
    }
    else
    {
        Print("Ata device initialized !\n");
    }

    while (1);
}
#include "AtaDriver.h"

#include <proc_io.h>
#include <stdio.h>

#define EIO 5
#define ENOSYS 35
#define ENOMEM 12

#define ATA_BLOCK_SIZE 512

static int AtaStatusWait(int io_base, int timeout)
{
    int status;

    if (timeout > 0)
    {
        int i = 0;
        while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
    }
    else
    {
        while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
    }
    return status;
}

static void AtaIoWait(int io_base)
{
    inb(io_base + ATA_REG_ALTSTATUS);
    inb(io_base + ATA_REG_ALTSTATUS);
    inb(io_base + ATA_REG_ALTSTATUS);
    inb(io_base + ATA_REG_ALTSTATUS);
}

static int AtaWait(int io, int adv)
{
    u8 status = 0;

    AtaIoWait(io);

    status = AtaStatusWait(io, -1);

    if (adv)
    {
        status = inb(io + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) return 1;
        if (status & ATA_SR_DF)  return 1;
        if (!(status & ATA_SR_DRQ)) return 1;
    }

    return 0;
}

static int AtaReadSectorPio(AtaDevice * device, char * buf, int lba)
{
    u16 io = device->dataPort;

    u8 cmd = 0xE0;
    int errors = 0;

try_label:
    outb(io + ATA_REG_CONTROL, 0x02);

    AtaWait(io, 0);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
    outb(io + ATA_REG_FEATURES, 0x00);
    outb(device->sectorCountPort, 1);
    outb(device->lbaLowPort, (u8)(lba));
    outb(device->lbaMidPort, (u8)(lba >> 8));
    outb(device->lbaHiPort, (u8)(lba >> 16));
    outb(device->commandPort, ATA_CMD_READ_PIO);

    if (AtaWait(io, 1))
    {
        errors++;
        if (errors > 4)
            return -EIO;

        goto try_label;
    }

    for (int i = 0; i < 256; i++)
    {
        u16 d = inw(device->dataPort);
        *(u16 *)(buf + i * 2) = d;
    }

    AtaWait(io, 0);
    return 0;
}

int AtaRead(AtaDevice * dev, void * buf, unsigned long offset, unsigned long size)
{
    int rc = 0, read = 0;

    int nbBlocks = 0;
    unsigned long blockBegin = 0, blockEnd = 0;
    char * buffer = nullptr, *bufferPtr = nullptr;

    blockBegin = offset / ATA_BLOCK_SIZE;
    blockEnd = (offset + size) / ATA_BLOCK_SIZE;
    nbBlocks = blockEnd - blockBegin + 1;

    //printf("nbBlocks : %d\n", nbBlocks);
    //printf("blockBegin : %d, end : %d\n", blockBegin, blockEnd);

    buffer = (char *)HeapAlloc(nbBlocks * ATA_BLOCK_SIZE);
    if (buffer == nullptr)
    {
        printf("Failed to allocate memory.\n");
        return 0;
    }
    bufferPtr = buffer;

    DISABLE_IRQ();

    for (int i = 0; i < nbBlocks; i++)
    {
        rc = AtaReadSectorPio(dev, bufferPtr, blockBegin + i);
        //printf("rc : %d\n", rc);
        if (rc == -EIO)
            return -EIO;

        bufferPtr += ATA_BLOCK_SIZE;
        read += ATA_BLOCK_SIZE;
    }

    MemCopy((u8 *)(buffer + offset % ATA_BLOCK_SIZE), (u8 *)buf, size);
    HeapFree(buffer);

    ENABLE_IRQ();
    return size;
}

static int AtaWriteSectorPio(AtaDevice * device, u16 * buf, int lba, unsigned long size)
{
    u16 io = device->dataPort;
    u8 cmd = 0xE0;

    outb(io + ATA_REG_CONTROL, 0x02);

    AtaWait(io, 0);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0F))));
    AtaWait(io, 0);
    outb(io + ATA_REG_FEATURES, 0x00);
    outb(device->sectorCountPort, 1);
    outb(device->lbaLowPort, (u8)(lba));
    outb(device->lbaMidPort, (u8)(lba >> 8));
    outb(device->lbaHiPort, (u8)(lba >> 16));
    outb(device->commandPort, ATA_CMD_WRITE_PIO);
    AtaWait(io, 0);

    int limit = (size >= ATA_BLOCK_SIZE ? 256 : size / 2);
    for (int i = 0; i < 256; i++)
    {
        if (i < limit)
        {
            outw(device->dataPort, buf[i]);
        }
        else
        {
            outw(device->dataPort, 0);
        }
        asm volatile("nop; nop; nop");
    }
    outb(device->commandPort, ATA_CMD_CACHE_FLUSH);

    AtaWait(io, 0);

    return 0;
}

int AtaWrite(AtaDevice * device, void * buf, unsigned long offset, unsigned long size)
{
    int count = (size > ATA_BLOCK_SIZE ? size / ATA_BLOCK_SIZE : 1);
    unsigned long block = offset / (unsigned long)ATA_BLOCK_SIZE;
    u16 * buffer = (u16 *)buf;

    if (offset % ATA_BLOCK_SIZE != 0)
    {
        printf("ata.c!AtaRead() : invalid offset parameter, should be a multiple of 512 !\n");
        return -1;
    }

    //printf("count : %d, block : %d, size : %d, offset : %d\n", count, block, size, offset);

    DISABLE_IRQ();
    for (int i = 0; i < count && size > 0; i++)
    {
        AtaWriteSectorPio(device, buffer, block + i, size);

        if (size >= ATA_BLOCK_SIZE)
        {
            size -= ATA_BLOCK_SIZE;
            buffer += ATA_BLOCK_SIZE;
        }
        else
        {
            buffer += size;
            size = 0;
        }

        for (int j = 0; j < 1000; j++);
    }
    ENABLE_IRQ();
    return count;
}

static int AtaIdentify(AtaDevice * device)
{
    outb(device->devicePort, device->type);
    outb(device->sectorCountPort, 0);
    outb(device->lbaLowPort, 0);
    outb(device->lbaMidPort, 0);
    outb(device->lbaHiPort, 0);
    outb(device->commandPort, ATA_CMD_IDENTIFY);

    u8 status = inb(device->dataPort + ATA_REG_STATUS);
    if (status)
    {
        for (int i = 0; i < 256; i++)
            inw(device->dataPort);

        return 1;
    }
    else
    {
        printf("IDENTIFY error on b0d0 -> no status\n");
        return 0;
    }
}

AtaDevice AtaCreate(AtaIoPort ioPort, AtaType type)
{
    AtaDevice device = { 0 };

    device.type = type;
    device.dataPort = ioPort;
    device.errorPort = ioPort + 0x1;
    device.sectorCountPort = ioPort + 0x2;
    device.lbaLowPort = ioPort + 0x3;
    device.lbaMidPort = ioPort + 0x4;
    device.lbaHiPort = ioPort + 0x5;
    device.devicePort = ioPort + 0x6;
    device.commandPort = ioPort + 0x7;
    device.controlPort = ioPort + 0x206;

    return device;
}

int AtaInit(AtaDevice * ataDevice)
{
    return AtaIdentify(ataDevice);
}

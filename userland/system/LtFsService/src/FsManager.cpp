#define __FS__
#include "FsManager.h"

#include <stdio.h>
#include <malloc.h>

#include <logger.h>
#define LOG(LOG_LEVEL, format, ...) LOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

//#include "elf.h"
#include "File.h"

static Status InitRoot();

Status FsInit(AtaDevice * device)
{
    Status status = Ext2ReadDiskOnDevice(device, &gExt2Disk);
    if (FAILED(status) || gExt2Disk == nullptr)
    {
        LOG(LOG_ERROR, "File system intialization failed with status %d", status);
        return status;
    }

    status = InitRoot();
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "InitRootFile() failed with status %d", status);
        return status;
    }

    //PrintDirectory(gRootFile);

    return STATUS_SUCCESS;
}

void FsCleanCallback()
{
    Ext2FreeDisk(gExt2Disk);
}

void FreeFile(File * file)
{
    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return;
    }

    // TODO : compléter nettoyage...

    HeapFree(file);
}

static Status InitRoot()
{
    Status status = STATUS_FAILURE;
    Ext2Inode * inode = nullptr;

    if (gExt2Disk == nullptr)
    {
        LOG(LOG_ERROR, "Ext2 file system is not initialized !");
        return STATUS_UNEXPECTED;
    }

    status = Ext2ReadInode(gExt2Disk, EXT2_ROOT_INODE_NUMBER, &inode);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
        goto clean;
    }

    status = CreateFile(gExt2Disk, inode, EXT2_ROOT_INODE_NUMBER, &gRootFile);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "CreateFile() failed with code %d", status);
        goto clean;
    }

    status = InitRootFile(gRootFile);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "InitRootFile() failed with code %d", status);
        goto clean;
    }

    status = BrowseAndCacheDirectory(gRootFile);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "BrowseAndCacheDirectory() failed with code %d", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}
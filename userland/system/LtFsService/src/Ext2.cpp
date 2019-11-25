#include "Ext2.h"
#include "File.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <logger.h>
#define LOG(LOG_LEVEL, format, ...) LOGGER("EXT2", LOG_LEVEL, format, ##__VA_ARGS__)

#define MBR_RESERVED_SIZE 1024
#define INODE_NB_DIRECT_PTR 12
#define INODE_SINGLY_INDIRECT_PTR_INDEX 12
#define INODE_DOUBLY_INDIRECT_PTR_INDEX 13
#define INODE_TRIPLY_INDIRECT_PTR_INDEX 14

// https://wiki.osdev.org/Ext2

typedef Ext2Disk Disk;
typedef AtaDevice Device;

static Status ReadSuperBlock(Device * device, Ext2SuperBlock ** superBlock);
static Status ReadGroupDesc(Ext2Disk * disk, Ext2GroupDesc ** groupDesc);

Status Ext2ReadDiskOnDevice(AtaDevice * device, Ext2Disk ** disk)
{
    Ext2Disk * localDisk = nullptr;
    Ext2SuperBlock * superBlock = nullptr;
    Ext2GroupDesc * groupDesc = nullptr;
    Status status = STATUS_FAILURE;
    u32 i = 0, j = 0;

    if (device == nullptr)
    {
        LOG(LOG_ERROR, "Invalid device parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (disk == nullptr)
    {
        LOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    *disk = nullptr;

    localDisk = (Disk *)HeapAlloc(sizeof(Disk));
    if (localDisk == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Disk));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    status = ReadSuperBlock(device, &superBlock);
    if (FAILED(status) || superBlock == nullptr)
    {
        LOG(LOG_ERROR, "ReadSuperBlock() failed with status %d", status);
        goto clean;
    }

    localDisk->device = device;
    localDisk->superBlock = superBlock;
    localDisk->blockSize = 1024 << superBlock->logBlockSize;

    i = (superBlock->blocksCount / superBlock->blocksPerGroup)
        + ((superBlock->blocksCount % superBlock->blocksPerGroup) ? 1 : 0);
    j = (superBlock->inodesCount / superBlock->inodesPerGroup)
        + ((superBlock->inodesCount % superBlock->inodesPerGroup) ? 1 : 0);

    localDisk->groups = (i > j ? i : j);

    status = ReadGroupDesc(localDisk, &groupDesc);
    if (groupDesc == nullptr)
    {
        LOG(LOG_ERROR, "ReadGroupDesc() failed with status %d", status);
        goto clean;
    }

    localDisk->groupDec = groupDesc;

    *disk = localDisk;
    localDisk = nullptr;
    superBlock = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localDisk != nullptr)
    {
        HeapFree(localDisk);
        localDisk = nullptr;
    }

    if (superBlock != nullptr)
    {
        HeapFree(superBlock);
        superBlock = nullptr;
    }

    return status;
}

Status Ext2ReadInode(Ext2Disk * disk, int num, Ext2Inode ** inode)
{
    Ext2Inode * localInode = nullptr;
    int ret = 0;
    int inodeGroupIndex = 0;
    int inodeIndex = 0;
    int offset = 0;
    Status status = STATUS_FAILURE;

    if (inode == nullptr)
    {
        LOG(LOG_ERROR, "Invalid inode parameter");
        return STATUS_NULL_PARAMETER;
    }

    *inode = nullptr;

    if (num < 1)
    {
        LOG(LOG_ERROR, "Inode number must be > 0 !");
        return STATUS_INVALID_PARAMETER;
    }

    localInode = (Ext2Inode *)HeapAlloc(sizeof(Ext2Inode));
    if (localInode == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Ext2Inode));
        status = STATUS_ALLOC_FAILED;
    }

    // https://wiki.osdev.org/Ext2#Inodes
    inodeGroupIndex = (num - 1) / disk->superBlock->inodesPerGroup;
    inodeIndex = (num - 1) % disk->superBlock->inodesPerGroup;

    offset = disk->groupDec[inodeGroupIndex].inodeTable * disk->blockSize + inodeIndex * disk->superBlock->inodeSize;

    LOG(LOG_DEBUG, "AtaRead() : localInode(%x), offset(%d), inodeSize(%d)", localInode, offset, disk->superBlock->inodeSize);
    ret = AtaRead(disk->device, localInode, offset, disk->superBlock->inodeSize);

    if (ret < 0)
    {
        LOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
        goto clean;
    }

    *inode = localInode;
    localInode = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localInode != nullptr)
    {
        HeapFree(localInode);
        localInode = nullptr;
    }

    return status;
}

static Status ReadSuperBlock(Device * device, Ext2SuperBlock ** superBlock)
{
    Ext2SuperBlock * localSuperBlock = nullptr;
    const unsigned int offset = MBR_RESERVED_SIZE;
    int ret = 0;
    Status status = STATUS_FAILURE;

    if (device == nullptr)
    {
        LOG(LOG_ERROR, "Invalid device parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (superBlock == nullptr)
    {
        LOG(LOG_ERROR, "Invalid superBlock parameter");
        return STATUS_NULL_PARAMETER;
    }

    *superBlock = nullptr;

    localSuperBlock = (Ext2SuperBlock *)HeapAlloc(sizeof(Ext2SuperBlock));
    if (localSuperBlock == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(Ext2SuperBlock));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    ret = AtaRead(device, localSuperBlock, sizeof(Ext2SuperBlock), offset);

    if (ret < 0)
    {
        LOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
        goto clean;
    }

    *superBlock = localSuperBlock;
    localSuperBlock = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localSuperBlock != nullptr)
    {
        HeapFree(localSuperBlock);
        localSuperBlock = nullptr;
    }

    return status;
}

static Status ReadGroupDesc(Ext2Disk * disk, Ext2GroupDesc ** groupDesc)
{
    Ext2GroupDesc * localGroupDesc = nullptr;
    int ret = 0;
    int size = disk->groups * sizeof(Ext2GroupDesc);
    unsigned long offset = (disk->blockSize == MBR_RESERVED_SIZE ? 2048 : disk->blockSize);
    Status status = STATUS_FAILURE;

    if (disk == nullptr)
    {
        LOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (groupDesc == nullptr)
    {
        LOG(LOG_ERROR, "Invalid groupDesc parameter");
        return STATUS_NULL_PARAMETER;
    }

    *groupDesc = nullptr;

    localGroupDesc = (Ext2GroupDesc *)HeapAlloc(size);
    if (localGroupDesc == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    ret = AtaRead(disk->device, localGroupDesc, offset, (unsigned long)size);

    if (ret < 0)
    {
        LOG(LOG_ERROR, "AtaRead() returned %d", ret);
        status = STATUS_FAILURE;
        goto clean;
    }

    *groupDesc = localGroupDesc;
    localGroupDesc = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localGroupDesc != nullptr)
    {
        HeapFree(localGroupDesc);
        localGroupDesc = nullptr;
    }

    return status;
}

Status Ext2ReadFile(Ext2Disk * disk, Ext2Inode * inode, int inum, char ** content)
{
    char * fileContent = nullptr;
    char * block = nullptr;
    u32 * singlyBlock = nullptr;
    u32 * doublyBlock = nullptr;
    u32 fileSize = 0;
    unsigned int fileOffset = 0, size = 0;
    Status status = STATUS_FAILURE;

    if (disk == nullptr)
    {
        LOG(LOG_ERROR, "Invalid disk parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (inode == nullptr)
    {
        LOG(LOG_ERROR, "Invalid inode parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (content == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    fileSize = inode->size;

    fileContent = (char *)HeapAlloc(fileSize);
    if (fileContent == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    block = (char *)HeapAlloc(disk->blockSize);
    if (block == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    // Direct blocks ptr
    for (int i = 0; i < INODE_NB_DIRECT_PTR && inode->block[i]; i++)
    {
        unsigned long offset = inode->block[i] * disk->blockSize;
        int ret = AtaRead(disk->device, block, offset, disk->blockSize);

        if (ret < 0)
        {
            LOG(LOG_ERROR, "AtaRead() failed for direct block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
        }

        size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
        MemCopy((u8 *)block, (u8 *)(fileContent + fileOffset), size);

        fileOffset += size;
        fileSize -= size;
    }

    // Singly Indirect block ptr
    if (inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX])
    {
        singlyBlock = (u32 *)HeapAlloc(disk->blockSize);
        if (singlyBlock == nullptr)
        {
            LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
            status = STATUS_ALLOC_FAILED;
            goto clean;
        }

        unsigned long offset = inode->block[INODE_SINGLY_INDIRECT_PTR_INDEX] * disk->blockSize;
        int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

        if (ret < 0)
        {
            LOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
        }

        for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
        {
            offset = singlyBlock[i] * disk->blockSize;
            int ret = AtaRead(disk->device, block, offset, disk->blockSize);

            if (ret < 0)
            {
                LOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
                status = STATUS_FAILURE;
                goto clean;
            }

            size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
            MemCopy((u8 *)block, (u8 *)(fileContent + fileOffset), size);
            fileOffset += size;
            fileSize -= size;
        }
    }

    // Double indirect block ptr
    if (inode->block[INODE_DOUBLY_INDIRECT_PTR_INDEX])
    {
        if (singlyBlock == nullptr)
        {
            singlyBlock = (u32 *)HeapAlloc(disk->blockSize);
            if (singlyBlock == nullptr)
            {
                LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
                status = STATUS_ALLOC_FAILED;
                goto clean;
            }
        }

        doublyBlock = (u32 *)HeapAlloc(disk->blockSize);
        if (doublyBlock == nullptr)
        {
            LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(size));
            status = STATUS_ALLOC_FAILED;
            goto clean;
        }

        unsigned long offset = inode->block[INODE_DOUBLY_INDIRECT_PTR_INDEX] * disk->blockSize;
        int ret = AtaRead(disk->device, singlyBlock, offset, disk->blockSize);

        if (ret < 0)
        {
            LOG(LOG_ERROR, "AtaRead() failed for singly indirect block ptr (returned %d)", ret);
            status = STATUS_FAILURE;
            goto clean;
        }

        for (int i = 0; i < disk->blockSize / sizeof(u32) && singlyBlock[i]; i++)
        {
            offset = singlyBlock[i] * disk->blockSize;
            ret = AtaRead(disk->device, doublyBlock, offset, disk->blockSize);

            if (ret < 0)
            {
                LOG(LOG_ERROR, "AtaRead() failed for doubly indirect block ptr (returned %d)", ret);
                status = STATUS_FAILURE;
                goto clean;
            }

            for (int j = 0; j < disk->blockSize / sizeof(u32) && doublyBlock[j]; j++)
            {
                offset = doublyBlock[i] * disk->blockSize;
                int ret = AtaRead(disk->device, block, offset, disk->blockSize);

                if (ret < 0)
                {
                    LOG(LOG_ERROR, "AtaRead() failed for doubly indirect block ptr (returned %d)", ret);
                    status = STATUS_FAILURE;
                    goto clean;
                }

                size = fileSize > disk->blockSize ? disk->blockSize : fileSize;
                MemCopy((u8 *)block, (u8 *)(fileContent + fileOffset), size);
                fileOffset += size;
                fileSize -= size;
            }
        }
    }

    // TODO : triply indirect, mais faut faire du ménage, trop l'bordel
    if (inode->block[INODE_TRIPLY_INDIRECT_PTR_INDEX])
    {
        LOG(LOG_WARNING, "TRIPLY INDIRECT PTR NOT SUPPORTED ");
    }

    *content = fileContent;
    fileContent = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (fileContent != nullptr)
    {
        HeapFree(fileContent);
        fileContent = nullptr;
    }

    if (block != nullptr)
    {
        HeapFree(block);
        block = nullptr;
    }

    if (singlyBlock != nullptr)
    {
        HeapFree(singlyBlock);
        singlyBlock = nullptr;
    }

    if (doublyBlock != nullptr)
    {
        HeapFree(doublyBlock);
        doublyBlock = nullptr;
    }

    return status;
}

void Ext2FreeDisk(Ext2Disk * disk)
{
    if (disk == nullptr)
    {
        LOG(LOG_ERROR, "Invalid disk parameter");
        return;
    }

    if (disk->groupDec != nullptr)
    {
        HeapFree(disk->groupDec);
        disk->groupDec = nullptr;
    }

    if (disk->superBlock != nullptr)
    {
        HeapFree(disk->superBlock);
        disk->superBlock = nullptr;
    }

    HeapFree(disk);
    disk = nullptr;
}
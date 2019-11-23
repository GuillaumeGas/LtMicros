#pragma once

#include <stdlib.h>
#include <status.h>

#include "AtaDriver.h"

#define EXT2_ROOT_INODE_NUMBER 2

/* super_block: errors */
#define EXT2_ERRORS_CONTINUE    1
#define EXT2_ERRORS_RO          2
#define EXT2_ERRORS_PANIC       3
#define EXT2_ERRORS_DEFAULT     1

/* inode: mode */
#define EXT2_S_IFMT     0xF000  /* format mask  */
#define EXT2_S_IFSOCK   0xC000  /* socket */
#define EXT2_S_IFLNK    0xA000  /* symbolic link */
#define EXT2_S_IFREG    0x8000  /* regular file */
#define EXT2_S_IFBLK    0x6000  /* block device */
#define EXT2_S_IFDIR    0x4000  /* directory */
#define EXT2_S_IFCHR    0x2000  /* character device */
#define EXT2_S_IFIFO    0x1000  /* fifo */

#define EXT2_S_ISUID    0x0800  /* SUID */
#define EXT2_S_ISGID    0x0400  /* SGID */
#define EXT2_S_ISVTX    0x0200  /* sticky bit */
#define EXT2_S_IRWXU    0x01C0  /* user access rights mask */
#define EXT2_S_IRUSR    0x0100  /* read */
#define EXT2_S_IWUSR    0x0080  /* write */
#define EXT2_S_IXUSR    0x0040  /* execute */
#define EXT2_S_IRWXG    0x0038  /* group access rights mask */
#define EXT2_S_IRGRP    0x0020  /* read */
#define EXT2_S_IWGRP    0x0010  /* write */
#define EXT2_S_IXGRP    0x0008  /* execute */
#define EXT2_S_IRWXO    0x0007  /* others access rights mask */
#define EXT2_S_IROTH    0x0004  /* read */
#define EXT2_S_IWOTH    0x0002  /* write */
#define EXT2_S_IXOTH    0x0001  /* execute */

struct Ext2SuperBlock
{
    u32 inodesCount;           /* Total number of inodes */
    u32 blocksCount;           /* Total number of blocks */
    u32 reservedBlocksCount;   /* Total number of blocks reserved for the super user */
    u32 freeBlocksCount;       /* Total number of free blocks */
    u32 freeInodesCount;       /* Total number of free inodes */
    u32 firstDataBlock;        /* Id of the block containing the superblock structure */
    u32 logBlockSize;          /* Used to compute block size = 1024 << log_block_size */
    u32 logFragSize;           /* Used to compute fragment size */
    u32 blocksPerGroup;        /* Total number of blocks per group */
    u32 fragsPerGroup;         /* Total number of fragments per group */
    u32 inodesPerGroup;        /* Total number of inodes per group */
    u32 mtime;                 /* Last time the file system was mounted */
    u32 wtime;                 /* Last write access to the file system */
    u16 mntCount;              /* How many `mount' since the last was full verification */
    u16 maxMntCount;           /* Max count between mount */
    u16 magic;                 /* = 0xEF53 */
    u16 state;                 /* File system state */
    u16 errors;                /* Behaviour when detecting errors */
    u16 minorRevLevel;         /* Minor revision level */
    u32 lastcheck;             /* Last check */
    u32 checkinterval;         /* Max. time between checks */
    u32 creatorOs;             /* = 5 */
    u32 revLevel;              /* = 1, Revision level */
    u16 defResuid;             /* Default uid for reserved blocks */
    u16 defResgid;             /* Default gid for reserved blocks */
    u32 firstIno;              /* First inode useable for standard files */
    u16 inodeSize;             /* Inode size */
    u16 blockGroupNr;          /* Block group hosting this superblock structure */
    u32 featureCompat;
    u32 featureIncompat;
    u32 featureRoCompat;
    u8 uuid[16];               /* Volume id */
    char volumeName[16];       /* Volume name */
    char lastMounted[64];      /* Path where the file system was last mounted */
    u32 algoBitmap;            /* For compression */
    u8 padding[820];
} __attribute__((packed));
typedef struct Ext2SuperBlock Ext2SuperBlock;

struct Ext2GroupDesc
{
    u32 blockBitmap;    /* Id of the first block of the "block bitmap" */
    u32 inodeBitmap;    /* Id of the first block of the "inode bitmap" */
    u32 inodeTable;     /* Id of the first block of the "inode table" */
    u16 freeBlocksCount;       /* Total number of free blocks */
    u16 freeInodesCount;       /* Total number of free inodes */
    u16 usedDirsCount; /* Number of inodes allocated to directories */
    u16 pad;             /* Padding the structure on a 32bit boundary */
    u32 reserved[3];     /* Future implementation */
} __attribute__((packed));
typedef struct Ext2GroupDesc Ext2GroupDesc;

struct Ext2Disk
{
    AtaDevice * device;  // TODO : remplacer par device générique
    Ext2SuperBlock * superBlock;
    u32 blockSize;
    u16 groups;             /* Total number of groups */
    Ext2GroupDesc * groupDec;
};
typedef struct Ext2Disk Ext2Disk;

struct Ext2Inode
{
    u16 mode;             /* File type + access rights */
    u16 uid;
    u32 size;
    u32 atime;
    u32 ctime;
    u32 mtime;
    u32 dtime;
    u16 gid;
    u16 links_count;
    u32 blocks;           /* 512 bytes blocks ! */
    u32 flags;
    u32 osd1;

    /*
    * [0] -> [11] : block number (32 bits per block)
    * [12]        : indirect block number
    * [13]        : bi-indirect block number
    * [14]        : tri-indirect block number
    */
    u32 block[15];

    u32 generation;
    u32 fileAcl;
    u32 dirAcl;
    u32 faddr;
    u8 osd2[12];
} __attribute__((packed));
typedef struct Ext2Inode Ext2Inode;

struct Ext2DirectoryEntry
{
    u32 inode;              /* inode number or 0 (unused) */
    u16 recLen;            /* offset to the next dir. entry */
    u8 nameLen;            /* name length */
    u8 fileType;
    char name;
} __attribute__((packed));
typedef struct Ext2DirectoryEntry Ext2DirectoryEntry;

Status Ext2ReadDiskOnDevice(AtaDevice * device, Ext2Disk ** disk);
Status Ext2ReadInode(Ext2Disk * disk, int num, Ext2Inode ** inode);
Status Ext2ReadFile(Ext2Disk * disk, Ext2Inode * inode, int inum, char ** content);
void Ext2FreeDisk(Ext2Disk * disk);
#pragma once

#include <status.h>

#include "Ext2.h"

struct File
{
    Ext2Disk * disk;
    int inum;
    Ext2Inode * inode;
    char * name;
    void * content;
    bool opened;
    File * leaf;   // premier répertoire fils
    File * parent;
    File * prev;
    File * next;
};

Status CreateFile(Ext2Disk * disk, Ext2Inode * inode, int inum, File ** file);
Status OpenFile(File * file);
Status OpenFileFromName(const char * filePath, File ** file);
Status ReadFileFromInode(int inodeNumber, File ** file);
Status BrowseAndCacheDirectory(File * directory);
Status InitRootFile(File * file);
bool IsDirectory(File * file);
bool IsCached(File * dir, const char * fileName, File ** file);

void PrintDirectory(File * dir);
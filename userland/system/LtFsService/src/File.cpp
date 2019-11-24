#include "File.h"
#include "FsManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include <logger.h>
#define LOG(LOG_LEVEL, format, ...) LOGGER("FS", LOG_LEVEL, format, ##__VA_ARGS__)

Status CreateFile(Ext2Disk * disk, Ext2Inode * inode, int inum, File ** file)
{
    Status status = STATUS_FAILURE;
    File * localFile = nullptr;

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

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    localFile = (File *)HeapAlloc(sizeof(File));
    if (localFile == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(File));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    localFile->content = nullptr;
    localFile->disk = disk;
    localFile->inode = inode;
    localFile->inum = inum;
    localFile->name = nullptr;
    localFile->opened = false;
    localFile->next = nullptr;
    localFile->parent = nullptr;
    localFile->prev = nullptr;

    *file = localFile;
    localFile = nullptr;

    status = STATUS_SUCCESS;

clean:
    if (localFile != nullptr)
    {
        HeapFree(localFile);
        localFile = nullptr;
    }

    return status;
}

Status OpenFile(File * file)
{
    Status status = STATUS_FAILURE;

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (file->inode == nullptr)
    {
        LOG(LOG_DEBUG, "inum : %d, inode : %d", file->inum, file->inode);
        status = Ext2ReadInode(gExt2Disk, file->inum, &file->inode);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
            goto clean;
        }
    }

    status = Ext2ReadFile(file->disk, file->inode, file->inum, (char **)&file->content);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "Ext2ReadFile() failed with code %d", status);
        goto clean;
    }

    file->opened = true;

    status = STATUS_SUCCESS;

clean:
    return status;
}

Status OpenFileFromName(const char * filePath, File ** file)
{
    Status status = STATUS_FAILURE;
    File * localFile = nullptr;
    File * directory = nullptr;
    char * path = (char *)filePath;

    if (filePath == nullptr)
    {
        LOG(LOG_ERROR, "Invalid filePath parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (path[0] == '/')
    {
        directory = gRootFile;
        path++; // on passe le '/'

        if (path[0] == '\0')
        {
            // On veut ouvrir le rep racine
            *file = gRootFile;
        }
    }
    else
    {
        // TODO : on a pas de process kernel, donc faut mettre l'info dans le thread
        // tmp
        //if (gCurrentThread->privilegeLevel == USER)
        //    directory = gCurrentProcess->currentDirectory;
        //else
            directory = gRootFile;
    }

    while (*path != '\0')
    {
        // Si on se trouve à la fin du chemin, ce soit être le nom du fichier
        int indexSlash = 0;
        if ((indexSlash = FirstIndexOf(path, '/')) < 0)
        {
            if (IsCached(directory, path, &localFile) == true)
            {
                if (localFile->opened == false)
                {
                    status = OpenFile(localFile);
                    if (FAILED(status))
                    {
                        LOG(LOG_ERROR, "OpenFile() failed with code %d", status);
                        goto clean;
                    }
                }

                *file = localFile;
                localFile = nullptr;

                status = STATUS_SUCCESS;
                break;
            }
            else
            {
                status = STATUS_FILE_NOT_FOUND;
                goto clean;
            }
        }
        else
        {
            // Sinon, on remplace la prochaine occurence de '/' par '\0' pour déterminer quoi faire
            // et on oublie pas de rétablir le '/' ensuite
            path[indexSlash] = '\0';

            if (StrCmp(path, "..") == 0)
            {
                directory = directory->parent;
            }
            else if (StrCmp(path, ".") == 0)
            {
                // on ne fait rien
            }
            else
            {
                if (IsCached(directory, path, &directory) == false)
                {
                    status = STATUS_FILE_NOT_FOUND;
                    goto clean;
                }

                if (directory->opened == false)
                {
                    status = OpenFile(directory);
                    if (FAILED(status))
                    {
                        // TODO : il faudra sans doute mieux gérer cette erreur... (accès interdit par exemple ?)
                        LOG(LOG_ERROR, "OpenFile() failed with code %d", status);
                        goto clean;
                    }
                }
                BrowseAndCacheDirectory(directory);
            }

            path[indexSlash] = '/';
            path = &path[indexSlash + 1];
        }
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

Status ReadFileFromInode(int inodeNumber, File ** file)
{
    Ext2Inode * inode = nullptr;
    Status status = STATUS_FAILURE;

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_INVALID_PARAMETER;
    }

    status = Ext2ReadInode(gExt2Disk, inodeNumber, &inode);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "Failed to retrieve inode %d !", inodeNumber);
        goto clean;
    }

    status = CreateFile(gExt2Disk, inode, inodeNumber, file);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "CreateFile() failed with code %d", status);
        goto clean;
    }

    status = OpenFile(*file);
    if (FAILED(status))
    {
        LOG(LOG_ERROR, "OpenFile() failed with code %d", status);
        goto clean;
    }

    status = STATUS_SUCCESS;

clean:
    if (inode != nullptr)
    {
        HeapFree(inode);
    }

    return status;
}

/*

*/
Status BrowseAndCacheDirectory(File * directory)
{
    Status status = STATUS_FAILURE;
    Ext2DirectoryEntry * currentEntry = nullptr;
    u32 dirContentSize = 0;

    if (directory == nullptr)
    {
        LOG(LOG_ERROR, "Invalid directory parameter");
        return STATUS_NULL_PARAMETER;
    }

    // Shouldn't happen
    if (directory->inode == nullptr)
    {
        LOG(LOG_WARNING, "directory->inode == nullptr");

        status = Ext2ReadInode(gExt2Disk, directory->inum, &directory->inode);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
            goto clean;
        }
    }

    if (directory->opened == false)
    {
        LOG(LOG_DEBUG, "Openning directory");
        status = OpenFile(directory);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "OpenFile() failed with code %d", status);
            goto clean;
        }
    }

    if (IsDirectory(directory) == false)
    {
        LOG(LOG_ERROR, "Not a directory !");
        status = STATUS_NOT_A_DIRECTORY;
        goto clean;
    }

    currentEntry = (Ext2DirectoryEntry *)directory->content;
    dirContentSize = directory->inode->size;
    
    while (dirContentSize > 0 && currentEntry->recLen > 0)
    {
        char * fileName = (char *)HeapAlloc(currentEntry->nameLen + 1);
        if (fileName == nullptr)
        {
            LOG(LOG_ERROR, "Couldn't allocate %d bytes", currentEntry->nameLen + 1);
            goto clean;
        }

        MemCopy(&currentEntry->name, fileName, currentEntry->nameLen + 1);
        fileName[currentEntry->nameLen] = '\0';

        if (StrCmp(fileName, ".") != 0 && StrCmp(fileName, "..") != 0)
        {
            if (IsCached(directory, fileName, nullptr) == false)
            {
                File * newCachedFile = (File *)HeapAlloc(sizeof(File));
                if (newCachedFile == nullptr)
                {
                    LOG(LOG_ERROR, "Couldn't allocate %d bytes", sizeof(File));
                    goto clean;
                }

                newCachedFile->content = nullptr;
                newCachedFile->disk = gExt2Disk;
                newCachedFile->inode = nullptr;
                newCachedFile->inum = currentEntry->inode;
                newCachedFile->leaf = nullptr;
                newCachedFile->name = fileName;
                fileName = nullptr;
                newCachedFile->next = nullptr;
                newCachedFile->parent = directory;
                newCachedFile->prev = nullptr;

                if (directory->leaf == nullptr)
                {
                    directory->leaf = newCachedFile;
                }
                else
                {
                    newCachedFile->next = directory->leaf;
                    directory->leaf->prev = newCachedFile;
                    directory->leaf = newCachedFile;
                }
            }
            else
            {
                HeapFree(fileName);
                fileName = nullptr;
            }
        }
        else
        {
            HeapFree(fileName);
            fileName = nullptr;
        }


        dirContentSize -= currentEntry->recLen;
        currentEntry = (Ext2DirectoryEntry *)((char *)currentEntry + (currentEntry->recLen));

        if (currentEntry == nullptr)
        {
            LOG(LOG_ERROR, "nullptr irectory entry");
            status = STATUS_UNEXPECTED;
            goto clean;
        }
    }

    status = STATUS_SUCCESS;

clean:
    return status;
}

Status InitRootFile(File * file)
{
    Status status = STATUS_FAILURE;
    const char name[] = "/";

    if (file == nullptr)
    {
        LOG(LOG_ERROR, "Invalid file parameter");
        return STATUS_NULL_PARAMETER;
    }

    file->name = (char *)HeapAlloc(StrLen(name) + 1);
    if (file->name == nullptr)
    {
        LOG(LOG_ERROR, "Couldn't allocate %d bytes", StrLen(name));
        status = STATUS_ALLOC_FAILED;
        goto clean;
    }

    StrCpy(name, file->name);

    file->parent = file;

    status = STATUS_SUCCESS;

clean:
    return status;
}

bool IsDirectory(File * file)
{
    if (file->inode == nullptr)
    {
        Status status = Ext2ReadInode(gExt2Disk, file->inum, &file->inode);
        if (FAILED(status))
        {
            LOG(LOG_ERROR, "Ext2ReadInode() failed with code %d", status);
            return false;
        }
    }

    return (file->inode->mode & EXT2_S_IFDIR) ? true : false;
}

bool IsCached(File * dir, const char * fileName, File ** file)
{
    if (dir == nullptr)
    {
        LOG(LOG_ERROR, "Invalid dir parameter");
        return false;
    }

    if (fileName == nullptr)
    {
        LOG(LOG_ERROR, "Invalid fileName parameter");
        return false;
    }

    File * leaf = dir->leaf;

    while (leaf != nullptr)
    {
        if (leaf->name != nullptr)
        {
            if (StrCmp(leaf->name, fileName) == 0)
            {
                if (file != nullptr)
                {
                    *file = leaf;
                }

                return true;
            }
        }
        leaf = leaf->next;
    }
    return false;
}

void PrintDirectory(File * dir)
{
    if (dir == nullptr)
    {
        LOG(LOG_ERROR, "Invalid dir parameter");
        return;
    }

    File * f = dir;
    int nb = 0;

    while (f != nullptr)
    {
        if (f->name != nullptr)
        {
            int i = 0;
            for (; i < nb; i++)
                printf(" ");

            if (f->leaf != nullptr)
                printf("[%s]\n", f->name);
            else
                printf("%s\n", f->name);
        }

        if (f->leaf != nullptr)
        {
            f = f->leaf;
            nb += 4;
        }
        else if (f->next != nullptr)
        {
            f = f->next;
        }
        else
        {
            f = f->parent->next;
            nb -= 4;

            if (f == dir)
                f = nullptr;
        }
    }
}
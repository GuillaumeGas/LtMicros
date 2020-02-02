#pragma once

#include <stdlib.h>
#include "AtaDriver.h"

#include "Ext2.h"
#include "File.h"

Status FsInit(AtaDevice * device);
void FsCleanCallback();

void FreeFile(File * file);

#ifdef __FS__
Ext2Disk * gExt2Disk = nullptr;
File * gRootFile = nullptr;
#else
extern Ext2Disk * gExt2Disk;
extern File * gRootFile;
#endif
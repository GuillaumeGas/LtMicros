#pragma once

#include "status.h"
#include "types.h"

#include <LtFsCommon.h>

Status FsOpenFile(const char * filePath, const FileAccess access, Handle * const fileHandle);
Status FsGetFileSize(const Handle fileHandle, unsigned int * const fileSize);
Status FsReadFile(const Handle fileHandle, char * const buffer, const int bytes, unsigned int * const bytesRead);
void FsCloseFile(const Handle fileHandle);
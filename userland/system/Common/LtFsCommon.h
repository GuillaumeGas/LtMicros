#pragma once

#include <status.h>
#include <types.h>

#define SERVICE_TERMINATE_CMD "terminate"
#define SERVICE_TEST_CMD "Hello world"

#define MAX_FULL_PATH_SIZE 255

enum FileAccess
{
    FILE_READ,
    FILE_ACCESS_MAX
};

enum LtFsRequestType
{
    LTFS_REQUEST_TERMINATE,
    LTFS_REQUEST_OPEN_FILE,
    LTFS_REQUEST_MAX
};

struct LtFsRequest
{
    LtFsRequestType type;
    void * parameters;

    static Status Create(const LtFsRequestType type, void * const parameters, const unsigned int size, LtFsRequest ** outRequest);
};

struct LtFsOpenFileParameters
{
    char filePath[MAX_FULL_PATH_SIZE];
    FileAccess access;
    Handle * fileHandlePtr;
};
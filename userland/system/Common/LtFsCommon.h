#pragma once

#include <status.h>
#include <types.h>

#define SERVICE_TERMINATE_CMD "terminate"
#define SERVICE_TEST_CMD "Hello world"

#define MAX_FULL_PATH_SIZE 255

typedef unsigned int FileHandle;

enum FileAccess
{
    FILE_READ,
    FILE_ACCESS_MAX
};

enum FileShareMode
{
    FILE_SHARE_NONE        = 0,
    FILE_SHARE_READ        = 1,
    FILE_SHARE_WRITE       = 2,
    FILE_SHARE_MAX
};

enum LtFsRequestType
{
    LTFS_REQUEST_TERMINATE,
    LTFS_REQUEST_CONNECT,
    LTFS_REQUEST_OPEN_FILE,
    LTFS_REQUEST_MAX
};

struct LtFsRequest
{
    LtFsRequestType type;
    void * parameters;

    static Status Create(const LtFsRequestType type, void * const parameters, const unsigned int size, LtFsRequest ** outRequest);
};

struct LtFsResponse
{
    Status status;
    unsigned int size;
    void * data;

    static Status Create(const Status resStatus, void * const data, const unsigned int size, LtFsResponse ** outResponse);
};

struct LtFsOpenFileParameters
{
    char filePath[MAX_FULL_PATH_SIZE];
    FileAccess access;
    FileShareMode shareMode;
};

struct LtFsConnectParameter
{
    char ipcServerId[MAX_FULL_PATH_SIZE];
};
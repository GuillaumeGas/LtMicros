#pragma once

#define FAILED(status) (status != STATUS_SUCCESS)

enum KeStatus
{
    /* General */
    STATUS_SUCCESS = 0,
    STATUS_FAILURE,

    /* Function parameters */
    STATUS_INVALID_PARAMETER,
    STATUS_NULL_PARAMETER,

    /* Memory */
    STATUS_ALLOC_FAILED,
    STATUS_PHYSICIAL_MEMORY_FULL,
    STATUS_INVALID_VIRTUAL_USER_ADDRESS,
    STATUS_PHYSICAL_MEMORY_FULL,

    /* IPC */
    IPC_STATUS_SUCCESS,

    STATUS_UNEXPECTED,
};
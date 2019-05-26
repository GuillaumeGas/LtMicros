#pragma once

#define FAILED(status) (status != STATUS_SUCCESS)

enum KeStatus
{
    /* General */
    STATUS_SUCCESS = 0,
    STATUS_FAILURE,
    STATUS_INVALID_PARAMETER,
    STATUS_NULL_PARAMETER,
    STATUS_ALLOC_FAILED,
    STATUS_PHYSICIAL_MEMORY_FULL,
};
#pragma once

#define FAILED(s) (s != STATUS_SUCCESS)

enum Status
{
    STATUS_SUCCESS = 0,
    STATUS_FAILURE,
    STATUS_NULL_PARAMETER,
    STATUS_INVALID_PARAMETER,
};
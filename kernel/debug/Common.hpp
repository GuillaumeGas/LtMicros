#pragma once

#include <kernel/lib/Types.hpp>

#include "LtDbgCommands.hpp"

/// @file

/// @addgroup Debug Debug group
/// @{

enum KeDebugStatus
{
    DBG_STATUS_SUCCESS,
    DBG_STATUS_FAILURE,
    DBG_STATUS_ALREADY_CONNECTED,
    DBG_STATUS_BREAKPOINT_REACHED,
    DBG_STATUS_WRONG_PARAMETER,
    DBG_STATUS_MEMORY_UNAVAILABLE
} typedef KeDebugStatus;

enum BpState
{
    BP_ENABLED,
    BP_DISABLED
} typedef BpState;

struct KeDebugContext
{
    u32 cr3, cr2, cr0;
    u32 gs, fs, es, ds, ss;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip;
    u16 cs;
    u32 eflags;
} typedef KeDebugContext;

struct KeBreakpoint
{
    u32 addr;
    BpState state;
    u8 savedInstByte;
    int id;
} typedef KeBreakpoint;

struct KeDebugPacket
{
    unsigned int size;
    u8 * content; // KeDebugRequest ou KeDebugResponse
} typedef KeDebugPacket;

struct KeDebugRequest
{
    CommandId command;
    unsigned int paramSize;
    char * param;
} typedef KeDebugRequest;

struct KeDebugResponseHeader
{
    CommandId command;
    KeDebugStatus status;
    KeDebugContext context;
    unsigned int dataSize;
} typedef KeDebugResponseHeader;

struct KeDebugResponse
{
    KeDebugResponseHeader header;
    char * data;
} typedef KeDebugResponse;

/// @}
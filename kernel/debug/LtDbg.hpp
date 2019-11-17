#pragma once

#include <kernel/lib/Types.hpp>

#include "Common.hpp"
#include "LtDbgCom.hpp"

/// @file

/// @defgroup Debug Debug group
/// @{

#define DEFAULT_ASM_BUFFER_SIZE 20
#define __debugbreak() asm("int $3")

/* DISASS CMD */
struct KeDebugDisassParamReq
{
    unsigned int nbInst;
} typedef KeDebugDisassParamReq;

struct KeDebugDisassParamRes
{
    unsigned int size;
    unsigned int startingAddress;
    char * data;
} typedef KeDebugDisassParamRes;
/* DISASS CMD */

/* MEMORY CMD*/
struct KeDebugMemoryParamReq
{
    unsigned int nbBytes;
    unsigned int startingAddress;
} typedef KeDebugMemoryParamReq;
/* MEMORY CMD*/

class LtDbg
{
public:
    LtDbg();

    void Init();

    bool IsInitialized() const;
    bool IsConnected() const;

    /* These functions must be called only from the LtDbg isr */
    void WaitForConnectCommand(KeDebugContext * context);
    void WaitForPacket(KeDebugContext * context);
    void BreakpointHit(KeDebugContext * context);

private:
    LtDbgCom _com;

    bool _isInitialized;
    bool _isConnected;

    void CleanupKeDebugResponse(KeDebugResponse * response);
    void CleanupKeDebugRequest(KeDebugRequest * request);

    bool ContinueCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool QuitCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool StepCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool RegistersCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool DisassCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool StackTraceCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool MemoryCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);
    bool IdtCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response);

};

#ifdef __LTDBG__
LtDbg gLtDbg;
#else
extern LtDbg gLtDbg;
#endif

/// @}
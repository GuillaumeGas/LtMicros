#define __LTDBG__
#include "LtDbg.hpp"
#include "LtDbgCom.hpp"
#include "LtDbgCommands.hpp"
#include "Common.hpp"

#include <kernel/arch/x86/Idt.hpp>
#include <kernel/arch/x86/Gdt.hpp>
#include <kernel/arch/x86/Vmm.hpp>
#include <kernel/arch/x86/Process.hpp>
#include <kernel/lib/Status.hpp>
#include <kernel/lib/StdLib.hpp>
#include <kernel/lib/StdIo.hpp>
#include <kernel/task/ProcessManager.hpp>
#include <kernel/Kernel.hpp>

#include <kernel/Logger.hpp>

//#define DEBUG_DEBUGGER

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

extern "C" void _asm_debug_isr(void);
extern "C" void _asm_breakpoint_isr(void);

extern "C" void DebugIsr(KeDebugContext * context)
{
    DKLOG(LOG_DEBUG, "Debug interrupt !");

    if (gLtDbg.IsInitialized() == false)
    {
        KLOG(LOG_ERROR, "Debugger uninitialized !");
        return;
    }

    if (gLtDbg.IsConnected() == false)
    {
        KLOG(LOG_ERROR, "Debugger not connected to client");
        return;
    }

    gLtDbg.BreakpointHit(context);
    gLtDbg.WaitForPacket(context);
}

extern "C" void BreakpointIsr(KeDebugContext * context)
{
    DKLOG(LOG_DEBUG, "Debug breakpoint !");

    if (gLtDbg.IsInitialized() == false)
    {
        KLOG(LOG_ERROR, "Debugger uninitialized !");
        return;
    }

    if (gLtDbg.IsConnected() == false)
    {
        gLtDbg.WaitForConnectCommand(context);
    }
    else
    {
        gLtDbg.BreakpointHit(context);
    }

    gLtDbg.WaitForPacket(context);
}

LtDbg::LtDbg() : _isInitialized(false), _isConnected(false) {}

void LtDbg::Init()
{
    gIdt.InitDescriptor((u32)_asm_debug_isr, DEBUG_GATE, ISR_INDEX_DEBUG);
    gIdt.InitDescriptor((u32)_asm_breakpoint_isr, DEBUG_GATE, ISR_INDEX_BREAKPOINT);
    gIdt.Reload();

    _isInitialized = true;
}

bool LtDbg::IsInitialized() const
{
    return _isInitialized;
}

bool LtDbg::IsConnected() const
{
    return _isConnected;
}

void LtDbg::BreakpointHit(KeDebugContext * context)
{
    KeDebugResponse response;
    response.header.command = CMD_UNKNOWN;
    response.header.context = *context;
    response.header.dataSize = 0;
    response.header.status = DBG_STATUS_BREAKPOINT_REACHED;
    response.data = nullptr;

    KeStatus status = UpdateKeDebugResponseWithProcessInfo(context, &response);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "UpdateResponseWithProcessInfo() failed with code %t", status);
    }

    _com.SendResponse(&response);
}

void LtDbg::WaitForConnectCommand(KeDebugContext * context)
{
    KeDebugRequest request;
    KeDebugResponse response;
    KeStatus status = STATUS_FAILURE;

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return;
    }

    u8 byte = _com.ReadByte();
    if (byte != 1)
        KLOG(LOG_ERROR, "Wrong startup byte !");
    else
        _isConnected = true;

    _com.WriteByte(1);

    KLOG(LOG_INFO, "LtDbg trying to connect... waiting for a connect request...");

    status = _com.RecvRequest(&request);
    if (status != STATUS_SUCCESS)
    {
        KLOG(LOG_ERROR, "RecvRequest() failed, status = %t", status);
        return;
    }

    if (request.command != CMD_CONNECT)
    {
        KLOG(LOG_ERROR, "Wrong command : %d (CMD_CONNECT expected)", request.command);
        return;
    }

    response.header.command = CMD_CONNECT;
    response.header.status = DBG_STATUS_SUCCESS;
    response.header.dataSize = 0;
    response.header.context = *context;
    response.data = nullptr;

    status = UpdateKeDebugResponseWithProcessInfo(context, &response);
    if (FAILED(status))
    {
        KLOG(LOG_ERROR, "UpdateResponseWithProcessInfo() failed with code %t", status);
    }

    _com.SendResponse(&response);

    _isConnected = true;

    KLOG(LOG_INFO, "LtDbg connected !");
}

void LtDbg::WaitForPacket(KeDebugContext * context)
{
    KeDebugRequest request;
    KeDebugResponse response;
    KeStatus status = STATUS_FAILURE;
    bool running = false;

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return;
    }

    while (running == false)
    {
        status = _com.RecvRequest(&request);
        if (status != STATUS_SUCCESS)
        {
            KLOG(LOG_ERROR, "RecvRequest() failed (status = %t)", status);
            return;
        }

        switch (request.command)
        {
        case CMD_STEP:
            DKLOG(LOG_DEBUG, "Step command");
            running = StepCommand(&request, context, &response);
            break;
        case CMD_CONTINUE:
            DKLOG(LOG_DEBUG, "Continue command");
            running = ContinueCommand(&request, context, &response);
            break;
        case CMD_QUIT:
            DKLOG(LOG_DEBUG, "Quit command");
            running = QuitCommand(&request, context, &response);
            break;
        case CMD_REGISTERS:
            DKLOG(LOG_DEBUG, "Registers command");
            running = RegistersCommand(&request, context, &response);
            break;
        case CMD_DISASS:
            DKLOG(LOG_DEBUG, "Disass command");
            running = DisassCommand(&request, context, &response);
            break;
        case CMD_STACK_TRACE:
            DKLOG(LOG_DEBUG, "Stack trace command");
            running = StackTraceCommand(&request, context, &response);
            break;
        case CMD_MEMORY:
            DKLOG(LOG_DEBUG, "Memory command");
            running = MemoryCommand(&request, context, &response);
            break;
        case CMD_BP:
            DKLOG(LOG_DEBUG, "Breakpoint command");
            break;
        case CMD_IDT:
            DKLOG(LOG_DEBUG, "Idt command");
            running = IdtCommand(&request, context, &response);
            break;
        default:
            DKLOG(LOG_DEBUG, "Undefined debug command");
            response.header.command = request.command;
            response.header.context = *context;
            response.header.dataSize = 0;
            response.header.status = DBG_STATUS_FAILURE;
            response.data = nullptr;
        }

        status = UpdateKeDebugResponseWithProcessInfo(context, &response);
        if (FAILED(status))
        {
            KLOG(LOG_ERROR, "UpdateResponseWithProcessInfo() failed with code %t", status);
        }

        if (running == false)
        {
            status = _com.SendResponse(&response);

            if (status != STATUS_SUCCESS)
            {
                KLOG(LOG_ERROR, "SendResponse() failed with code : %t", status);
            }
        }
        else
        {
            DKLOG(LOG_DEBUG, "Continuing...");
        }

        CleanupKeDebugRequest(&request);
        CleanupKeDebugResponse(&response);
    }
}

void LtDbg::CleanupKeDebugResponse(KeDebugResponse * response)
{
    if (response == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid response parameter");
        return;
    }

    if (response->data == nullptr)
    {
        return;
    }

    HeapFree(response->data);
}

void LtDbg::CleanupKeDebugRequest(KeDebugRequest * request)
{
    if (request == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid request parameter");
        return;
    }

    if (request->param == nullptr)
    {
        return;
    }

    HeapFree(request->param);
}

KeStatus LtDbg::UpdateKeDebugResponseWithProcessInfo(KeDebugContext * context, KeDebugResponse * response)
{
    KeStatus status = STATUS_FAILURE;
    Process * currentProcess = nullptr;

    if (context == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid context parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (response == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid response parameter");
        return STATUS_NULL_PARAMETER;
    }

    currentProcess = gProcessManager.GetCurrentProcess();
    if (currentProcess == nullptr)
    {
        KLOG(LOG_ERROR, "ProcessManager::GetCurrentProcess() returned null");
        goto clean;
    }

    MemCopy(currentProcess->name, &response->header.processName, 512);

    status = STATUS_SUCCESS;

clean:
    return status;
}

bool LtDbg::StepCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    const u32 TRAP_FLAG_MASK = 0x100;

    context->eflags |= TRAP_FLAG_MASK;

    response->header.command = CMD_STEP;
    response->header.context = *context;
    response->header.dataSize = 0;
    response->header.status = DBG_STATUS_SUCCESS;
    response->data = nullptr;

    return true;
}

bool LtDbg::ContinueCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    context->eflags &= 0x0FEFF;

    response->header.command = CMD_CONTINUE;
    response->header.context = *context;
    response->header.dataSize = 0;
    response->header.status = DBG_STATUS_SUCCESS;
    response->data = nullptr;

    return true;
}

bool LtDbg::QuitCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    context->eflags &= 0x0FEFF;

    response->header.command = CMD_QUIT;
    response->header.context = *context;
    response->header.dataSize = 0;
    response->header.status = DBG_STATUS_SUCCESS;
    response->data = nullptr;

    return true;
}

bool LtDbg::RegistersCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    response->header.command = CMD_REGISTERS;
    response->header.context = *context;
    response->header.dataSize = 0;
    response->header.status = DBG_STATUS_SUCCESS;
    response->data = nullptr;

    return false;
}

bool LtDbg::DisassCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    KeDebugDisassParamRes * paramRes = nullptr;
    KeDebugDisassParamReq * paramReq = nullptr;
    unsigned int paramSize = 0;

    response->header.command = CMD_DISASS;
    response->header.context = *context;

    if (request->paramSize == 0 || request->param == nullptr)
    {
        KLOG(LOG_ERROR, "DisassCommand() paramSize == 0 or/and param == nullptr");
        response->header.status = DBG_STATUS_FAILURE;
        return false;
    }

    paramReq = (KeDebugDisassParamReq *)request->param;
    if (paramReq->nbInst == 0)
    {
        response->header.status = DBG_STATUS_SUCCESS;
        return false;
    }

    paramSize = sizeof(KeDebugDisassParamRes) + (paramReq->nbInst * DEFAULT_ASM_BUFFER_SIZE);
    response->header.dataSize = paramSize;
    response->data = (char *)HeapAlloc(paramSize);
    if (response->data == nullptr)
    {
        KLOG(LOG_ERROR, "DisassCommand(), couldn't allocate memory for response->data");
        response->header.status = DBG_STATUS_FAILURE;
    }

    paramRes = (KeDebugDisassParamRes *)response->data;
    paramRes->size = paramReq->nbInst * DEFAULT_ASM_BUFFER_SIZE;
    paramRes->startingAddress = context->eip;
    MemCopy((const void *)context->eip, &paramRes->data, paramRes->size);

    response->header.status = DBG_STATUS_SUCCESS;

    return false;
}

bool LtDbg::StackTraceCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    u8 * buffer = nullptr;
    List * list = ListCreate();
    unsigned int nbPtr = 0;
    u32 * ebp = (u32*)context->ebp;
    unsigned int bufferSize = 0;
    unsigned int index = 0;
    unsigned int * addresses = nullptr;
    unsigned int addr = 0;

    ListPush(list, (void *)context->eip);
    nbPtr++;

    while (ebp != nullptr)
    {
        ListPush(list, (void *)ebp[1]);
        ebp = (u32 *)ebp[0];
        nbPtr++;
    }

    bufferSize = nbPtr * sizeof(u32);

    buffer = (u8 *)HeapAlloc(bufferSize);
    if (buffer == nullptr)
    {
        KLOG(LOG_ERROR, "Can't allocate %d bytes\n", bufferSize);
        goto clean;
    }

    addresses = (unsigned int *)buffer;
    while ((addr = (unsigned int)ListPop(&list)) != 0)
    {
        addresses[index++] = addr;
    }

    response->header.command = CMD_STACK_TRACE;
    response->header.context = *context;
    response->header.dataSize = bufferSize;
    response->header.status = DBG_STATUS_SUCCESS;
    response->data = (char *)addresses;

clean:
    if (list != nullptr)
    {
        ListDestroy(list);
        list = nullptr;
    }

    return false;
}

bool LtDbg::MemoryCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    unsigned int addr = 0;
    unsigned int size = 0;
    char * buffer = nullptr;
    PageDirectoryEntry * currentPd = nullptr;

    response->header.command = CMD_MEMORY;
    response->header.context = *context;

    KeDebugMemoryParamReq * reqParam = (KeDebugMemoryParamReq *)request->param;

    if (request->paramSize != sizeof(KeDebugMemoryParamReq))
    {
        KLOG(LOG_ERROR, "Wrong param size");

        response->header.dataSize = 0;
        response->header.status = DBG_STATUS_WRONG_PARAMETER;
        response->data = nullptr;
        return false;
    }

    addr = reqParam->startingAddress;
    size = reqParam->nbBytes;

    currentPd = gVmm.GetCurrentPageDirectory();
    gVmm.SetCurrentPageDirectory((PageDirectoryEntry *)context->cr3);

    if (!gVmm.IsVirtualAddressAvailable(addr))
    {
        KLOG(LOG_WARNING, "Memory unavailable (0x%x)", addr);

        response->header.dataSize = 0;
        response->header.status = DBG_STATUS_MEMORY_UNAVAILABLE;
        response->data = nullptr;

        goto clean;
    }

    if (!gVmm.IsVirtualAddressAvailable(addr + size - 1))
    {
        KLOG(LOG_WARNING, "Memory unavailable (0x%x)", addr + size - 1);

        response->header.dataSize = 0;
        response->header.status = DBG_STATUS_MEMORY_UNAVAILABLE;
        response->data = nullptr;

        goto clean;
    }

    buffer = (char *)HeapAlloc(size);
    if (buffer == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", size);

        response->header.dataSize = 0;
        response->header.status = DBG_STATUS_FAILURE;
        response->data = nullptr;

        goto clean;
    }

    MemCopy((void *)addr, buffer, size);

    response->header.status = DBG_STATUS_SUCCESS;
    response->header.dataSize = size;
    response->data = buffer;

clean:
    gVmm.SetCurrentPageDirectory(currentPd);

    return false;
}

bool LtDbg::IdtCommand(KeDebugRequest * request, KeDebugContext * context, KeDebugResponse * response)
{
    const unsigned int idtSize = gIdt.info.limit;
    IdtDescriptor * descriptors = (IdtDescriptor*)HeapAlloc(idtSize);

    if (descriptors == nullptr)
    {
        KLOG(LOG_ERROR, "HeapAlloc() failed to allocate %d bytes", idtSize);
        response->header.dataSize = 0;
        response->header.status = DBG_STATUS_FAILURE;
        response->data = nullptr;

        goto clean;
    }

    MemCopy((void*)gIdt.info.base, descriptors, idtSize);

    response->header.command = CMD_IDT;
    response->header.context = *context;
    response->header.status = DBG_STATUS_SUCCESS;
    response->header.dataSize = idtSize;
    response->data = (char*)descriptors;

clean:
    return false;
}
#include "LtDbgCom.hpp"

#include <kernel/drivers/Serial.hpp>

#include <kernel/Logger.hpp>

#define DEBUG_DEBUGGER

#define KLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#ifdef DEBUG_DEBUGGER
#define DKLOG(LOG_LEVEL, format, ...) KLOGGER("DBG", LOG_LEVEL, format, ##__VA_ARGS__)
#else
#define DKLOG(LOG_LEVEL, format, ...)
#endif

u8 LtDbgCom::ReadByte()
{
    return gSerialDrv.ReadByte(COM2_PORT);
}

void LtDbgCom::ReadBytes(u8 * buffer, unsigned int size)
{
    if (buffer == nullptr)
        return;

    for (int i = 0; i < size; i++)
    {
        buffer[i] = ReadByte();
    }
}

void LtDbgCom::WriteByte(u8 byte)
{
    gSerialDrv.WriteByte(COM2_PORT, byte);
}

void LtDbgCom::WriteBytes(u8 * buffer, unsigned int bufferSize)
{
    for (unsigned int i = 0; i < bufferSize; i++)
    {
        WriteByte(buffer[i]);
    }
}

KeStatus LtDbgCom::RecvPacket(KeDebugPacket * packet)
{
    if (packet == nullptr)
    {
        return STATUS_NULL_PARAMETER;
    }

    ReadBytes((u8 *)&(packet->size), sizeof(unsigned int));

    DKLOG(LOG_DEBUG, "Reading %d bytes...", packet->size);

    if (packet->size == 0)
        return STATUS_SUCCESS;

    packet->content = (u8 *)HeapAlloc(packet->size);
    if (packet->content == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", packet->size);
        packet->size = 0;
        return STATUS_ALLOC_FAILED;
    }

    ReadBytes(packet->content, packet->size);
    return STATUS_SUCCESS;
}

KeStatus LtDbgCom::SendPacket(KeDebugPacket * packet)
{
    if (packet == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid packet parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (packet->content == nullptr)
    {
        KLOG(LOG_ERROR, "packet->content is nullptr");
        return STATUS_INVALID_PARAMETER;
    }

    if (packet->size == 0)
    {
        KLOG(LOG_ERROR, "packet->size == 0");
        return STATUS_INVALID_PARAMETER;
    }

    WriteBytes((u8 *)&packet->size, sizeof(unsigned int));
    WriteBytes(packet->content, packet->size);

    return STATUS_SUCCESS;
}

void LtDbgCom::CleanupPacket(KeDebugPacket * packet)
{
    if (packet == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid packet parameter");
        return;
    }

    if (packet->content != nullptr)
    {
        HeapFree(packet->content);
        packet->content = nullptr;
    }
}

KeStatus LtDbgCom::RecvRequest(KeDebugRequest * request)
{
    KeDebugPacket packet = { 0 };
    KeDebugRequest * ptrRequest = nullptr;
    KeStatus status = STATUS_FAILURE;

    status = RecvPacket(&packet);
    if (status != STATUS_SUCCESS)
    {
        KLOG(LOG_ERROR, "RecvPacket() failed with status : %d", status);
        return status;
    }

    if (packet.size == 0 || packet.content == nullptr)
    {
        DKLOG(LOG_DEBUG, "packet.size == %d || packet.content == %x", packet.size, packet.content);
        return STATUS_SUCCESS;
    }

    ptrRequest = (KeDebugRequest *)packet.content;
    request->command = ptrRequest->command;
    request->paramSize = ptrRequest->paramSize;

    if (request->paramSize > 0)
    {
        request->param = (char *)HeapAlloc(request->paramSize);
        if (request->param == nullptr)
        {
            KLOG(LOG_ERROR, "Couldn't allocate %d bytes for request->param", request->paramSize);
            return STATUS_ALLOC_FAILED;
        }

        MemCopy(&(ptrRequest->param), request->param, request->paramSize);
    }

    CleanupPacket(&packet);

    return STATUS_SUCCESS;
}

KeStatus LtDbgCom::SendResponse(KeDebugResponse * response)
{
    KeDebugPacket packet;
    u8 * buffer = nullptr;

    if (response == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid response parameter");
        return STATUS_NULL_PARAMETER;
    }

    if (response->header.dataSize != 0 && response->data == nullptr)
    {
        KLOG(LOG_ERROR, "response->data shouldn't be nullptr (dataSize : %d)", response->header.dataSize);
        return STATUS_INVALID_PARAMETER;
    }

    packet.size = sizeof(KeDebugResponseHeader) + response->header.dataSize;

    buffer = (u8 *)HeapAlloc(packet.size);
    if (buffer == nullptr)
    {
        KLOG(LOG_ERROR, "Couldn't allocate %d bytes", packet.size);
        return STATUS_ALLOC_FAILED;
    }

    MemCopy(&response->header, buffer, sizeof(KeDebugResponseHeader));
    MemCopy(response->data, buffer + sizeof(KeDebugResponseHeader), response->header.dataSize);

    packet.content = buffer;

    return SendPacket(&packet);
}
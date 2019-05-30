#pragma once

/// @file

/// @addgroup Debug Debug group
/// @{

#include <kernel/lib/StdLib.hpp>
#include "Common.hpp"

class LtDbgCom
{
private:
    friend class LtDbg;

    u8 ReadByte();
    void ReadBytes(u8 * buffer, unsigned int size);
    void WriteByte(u8 byte);
    void WriteBytes(u8 * buffer, unsigned int bufferSize);

    KeStatus RecvPacket(KeDebugPacket * packet);
    KeStatus SendPacket(KeDebugPacket * packet);
    KeStatus RecvRequest(KeDebugRequest * request);
    KeStatus SendResponse(KeDebugResponse * response);
    void CleanupPacket(KeDebugPacket * packet);
};

/// @}
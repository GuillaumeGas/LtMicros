#pragma once

#include <status.h>
#include <Ipc.hpp>
#include <LtFsCommon.h>

Status ServiceExecuteCommand(IpcServer * const server, const Handle clientProcess, const LtFsRequestType requestType, bool * serviceTerminate);
void ServiceCommandInit();
#pragma once

#include <status.h>
#include <Ipc.hpp>

Status ServiceExecuteCommand(const ProcessHandle processHandle, char * const message, unsigned int size, bool * serviceTerminate);
void ServiceCommandInit();
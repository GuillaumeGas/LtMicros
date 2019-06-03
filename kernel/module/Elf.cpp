#include "Elf.hpp"

#include <kernel/Logger.hpp>
#define KLOG(LOG_LEVEL, format, ...) KLOGGER("MODULE", LOG_LEVEL, format, ##__VA_ARGS__)

static const char ElfIdent[] = { 0x7f, 'E', 'L', 'F' };

bool Elf::ElfCheckIdent(ElfHeader * header)
{
    for (int i = 0; i < 4; i++)
        if (header->ident[i] != ElfIdent[i])
            return FALSE;
    return TRUE;
}

KeStatus Elf::ElfInit(void * file, ElfFile * elf)
{
    KeStatus status = STATUS_FAILURE;

    if (file == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid extFile parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    if (elf == nullptr)
    {
        KLOG(LOG_ERROR, "Invalid file parameter\n");
        return STATUS_NULL_PARAMETER;
    }

    elf->header = (ElfHeader *)file;

    if (elf->header->phoff == 0)
    {
        KLOG(LOG_ERROR, "Elf program header table pointer is NULL\n");
        status = STATUS_UNEXPECTED;
    }

    elf->prgHeaderTable = (ElfProgramHeaderTable *)((u8 *)elf->header + elf->header->phoff);

    status = STATUS_SUCCESS;

    return status;
}
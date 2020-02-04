#pragma once

#include <kernel/lib/Status.hpp>
#include <kernel/lib/List.hpp>
#include <kernel/arch/x86/Vmm.hpp>

/// @file

/// @addgroup TaskGroup
/// @{

class IpcBuffer
{
public:
    IpcBuffer();

    void Init();

    KeStatus AddBytes(const char* message, const unsigned int size);
    KeStatus ReadBytes(char* const buffer, const unsigned int size, unsigned int* const bytesRead);

private:
    Page * AllocatePage() const;
    KeStatus AllocateWritePage();

    /*
        TODO : 
         - ajouter un boolean disant si le buffer est pret a etre lu
         - éventuellement ajouter une section critique/mutex...
    */

    Page * currentPageWrite;
    Page * currentPageRead;

    char * currentPageWritePtr;
    char * currentPageReadPtr;
    
    char * currentWriteLimit;
    char * currentReadLimit;

    List * pagesList;
};

/// @}
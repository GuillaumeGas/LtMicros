#pragma once

#include <kernel/multiboot.hpp>
#include <kernel/lib/Status.hpp>
#include <kernel/arch/x86/Process.hpp>

#include "Elf.hpp"

class Module
{
public:
    static void Load(MultiBootModule * module);

private:
    static KeStatus _MapElfInProcess(ElfFile elf, Process * process);
};
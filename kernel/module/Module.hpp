#pragma once

#include <kernel/multiboot.hpp>

class Module
{
public:
    static void Load(MultiBootModule * module);
};
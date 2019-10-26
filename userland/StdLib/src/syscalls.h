#pragma once

/*
    Syscalls functions defines, implem are in syscalls.asm
*/

extern "C" void _print(const char * str);
extern "C" void _printChar(const char c);
extern "C" void * _sbrk(const int nbBlock);
#include "stdio.h"
#include "stdlib.h"
#include "syscalls.h"

void Print(const char * str)
{
	if (str == nullptr)
		return;

	_print(str);
}
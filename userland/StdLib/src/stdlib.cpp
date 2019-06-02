#include "stdlib.h"
#include "stdio.h"

int StrCmp(const char * str1, const char * str2)
{
	if (str1 == nullptr)
		return -2;

	if (str2 == nullptr)
		return -2;

	unsigned int index = 0;

	while (str1[index] != '\0' && str2[index] != '\0')
	{
		if (str1[index] < str2[index])
			return -1;

		if (str1[index] > str2[index])
			return 1;

		if (str1[index + 1] == '\0' && str2[index + 1] != '\0')
			return -1;

		if (str2[index + 1] == '\0' && str1[index + 1] != '\0')
			return 1;

		index++;
	}

	return 0;
}

void MmSet(char * src, char byte, unsigned int size)
{
	if (src == nullptr || size == 0)
		return;

	while ((size--) > 0)
		*(src++) = byte;
}

unsigned long StrLen(const char * str)
{
	if (str == nullptr)
		return 0;

	unsigned long length = 0;
	while (*(str++) != '\0')
		length++;
	return length;
}
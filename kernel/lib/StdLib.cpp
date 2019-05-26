#include <kernel/lib/StdLib.hpp>

void Halt()
{
    asm("hlt");
}

void Pause()
{
    while (1) {}
}

void StrCpy(const char * src, char * dst)
{
    if (src == nullptr || dst == nullptr)
        return;

    while (*src != '\0')
        *(dst++) = *(src++);
    *dst = '\0';
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

int FirstIndexOf(const char * str, char const c)
{
    if (str == nullptr)
        return -2;

    int index = 0;

    while (str[index] != '\0')
    {
        if (str[index] == c)
            return index;
        index++;
    }

    return -1;
}
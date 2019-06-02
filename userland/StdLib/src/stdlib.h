#pragma once

typedef unsigned char u8;
typedef unsigned char uchar;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

int StrCmp(const char * str1, const char * str2);
void MmSet(char * src, char byte, unsigned int size);
unsigned long StrLen(const char * str);
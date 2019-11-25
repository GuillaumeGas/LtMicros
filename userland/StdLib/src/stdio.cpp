#include "stdio.h"
#include "stdlib.h"
#include "syscalls.h"
#include "stdarg.h"

/// @brief For now, we don't handle value > 32 bits
#define MAX_BITS 32

static int checkType(char type)
{
    return (type == 'd' || type == 'c' || type == 's' || type == 'x' || type == 'b' || type == 't');
}

static void printChar(char c)
{
    _sysPrintChar(c);
}

static void printStr(char * str)
{
    _sysPrint(str);
}

static void printInt(const int x, const unsigned short base)
{
    char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int i, cpt, reste;
    char chaine[34];
    int quotient = x;

    if (x == 0)
        printChar('0');

    /* vérification de la base. il faut que ça cadre avec "HEX" */
    if ((base < 2) || (16 < base))
    {
        //KLOG(LOG_ERROR, "Base non valide ! (%d)", base);
        return;
    }

    /* parce qu'on ne travaille qu'avec des entiers positifs */
    if (quotient < 0)
        quotient = -quotient;

    cpt = 0;

    while (quotient != 0)
    {
        reste = quotient % base;
        quotient = (int)quotient / base;
        chaine[cpt] = HEX[reste];
        cpt++;
    }

    if (x < 0)
    {
        chaine[cpt] = '-';
        cpt++;
    }

    for (i = cpt - 1; i >= 0; i--)
        printChar(chaine[i]);
}


static void printBin(const int value, int nbBits)
{
    int offset;
    int i = 0;

    if (nbBits > MAX_BITS)
        nbBits = MAX_BITS;

    offset = nbBits - 1;
    while (offset >= 0)
    {
        if (nbBits > 8 && i % 8 == 0 && i != 0)
            printChar('.');
        i++;
        if (((value >> offset--) & 1) == 1)
            printChar('1');
        else
            printChar('0');
    }

    printChar('b');
}

static void printStatus(const int x)
{
    printStr((char*)StatusGetStringFromInt((Status)x));
}

void printf(const char * format, ...)
{
    _sysEnterScreenCriticalSection();

    va_list ap;
    va_start(ap, format);
    printfEx(format, ap);
    va_end(ap);

    _sysLeaveScreenCriticalSection();
}

void printfEx(const char * format, va_list ap)
{
    int nbBits = 8;
    int value;

    while (*format != '\0')
    {
        char c = *format;

        if (c == '%')
        {
            char type = *(format + 1);
            if (checkType(type) == 0)
            {
                printChar(c);
            }
            else
            {
                switch (type)
                {
                    case 'd':
                        printInt(va_arg(ap, int), 10);
                        break;
                    case 'x':
                    case 'p':
                        printStr("0x");
                        printInt(va_arg(ap, int), 16);
                        break;
                    case 'b':
                        value = va_arg(ap, int);
                        if (*(format + 2) == '*')
                        {
                            nbBits = va_arg(ap, int);
                            format++;
                        }
                        printBin(value, nbBits);
                        break;
                    case 's':
                        printStr((char*)va_arg(ap, char *));
                        break;
                    case 't':
                        printStatus(va_arg(ap, int));
                        break;
                    case 'c':
                    default:
                        printChar((char)va_arg(ap, int));
                }
                format++;
            }
        }
        else
        {
            printChar(*format);
        }

        format++;
    }
}
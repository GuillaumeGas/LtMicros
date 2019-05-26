#pragma once

#include <kernel/lib/Types.hpp>

/// @addgroup ArchX86Group
/// @{

/// @brief Structures used to access data pushed on stack during an interrupt request.

struct InterruptContext
{
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip;
    u32 cs;
    u32 eflags;
};

struct InterruptFromUserlandContext
{
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp_i;
    u32 ss;
};

struct ExceptionContext
{
    u32 cr3, cr2, cr0;
    u32 gs, fs, es, ds;
    u32 eflags;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip;
};

struct ExceptionContextWithCode
{
    u32 cr3, cr2, cr0;
    u32 gs, fs, es, ds;
    u32 eflags;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 code;
    u32 eip;
};

struct ExceptionContextUserWithCode
{
    u32 cr3, cr2, cr0;
    u32 gs, fs, es, ds;
    u32 eflags;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 code;
    u32 eip;

    u32 csU;
    u32 eflagsU;
    u32 espU;
    u32 ssU;
};

/// @brief The following functions display the interrupt context content.
/// @param[in] content A pointer to an interrupt context
void PrintInterruptContext(InterruptContext * context);
/// @brief See PrintInterruptContext description
/// @param[in] content A pointer to an interrupt context
void PrintExceptionContext(ExceptionContext * context);
/// @brief See PrintInterruptContext description
/// @param[in] content A pointer to an interrupt context
void PrintExceptionContextWithCode(ExceptionContextWithCode * context);
/// @brief See PrintInterruptContext description
/// @param[in] content A pointer to an interrupt context
void PrintExceptionUserContextWithCode(ExceptionContextUserWithCode * context);

/// @}
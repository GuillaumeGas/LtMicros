#pragma once

/// @addgroup DriversGroup
/// @{

/// @brief Wrapper for some usefull asm instruction

/// @brief Deactivates interrupts
#define DISABLE_IRQ() asm("cli"::)

/// @brief Activates interrupts
#define ENABLE_IRQ() asm("sti"::)

/// Writes a byte (value) in a given port
#define outb(port,value) \
    asm volatile ("outb %%al, %%dx" :: "d" (port), "a" (value));

/// Writes a byte (value) in a given port and adds a time delay
#define outbp(port,value) \
    asm volatile ("outb %%al, %%dx; jmp 1f; 1:" :: "d" (port), "a" (value));

/// Reads a byte on a given port
#define inb(port) ({ \
	    unsigned char _v; \
	    asm volatile ("inb %%dx, %%al" : "=a" (_v) : "d" (port)); \
	    _v; \
	})

/// Write a word on a given port
#define outw(port,value) \
	asm volatile ("outw %%ax, %%dx" :: "d" (port), "a" (value));

/// Reads a word on a given port
#define inw(port) ({ \
	u16 _v; \
	asm volatile ("inw %%dx, %%ax" : "=a" (_v) : "d" (port)); \
        _v; })

/// @}
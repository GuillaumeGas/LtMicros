#pragma once

/// The magic field should contain this.
#define MULTIBOOT_HEADER_MAGIC 0x2BADB002

struct MultiBootModule
{
    void * mod_start;
    void * mod_end;
    char * name;
    u32 reserved;
};

struct MultibootPartialInfo 
{
	unsigned long flags;
	unsigned long low_mem;
	unsigned long high_mem;
	unsigned long boot_device;
	unsigned long cmdline;
    unsigned long mods_count;
    MultiBootModule * mods_addr;
};

enum MultibootFlag
{
	MEM_INFO = 1,
	BOOT_DEVICE = 2,
	CMD_LINE = 4,
	MODS_INFO = 8,
	SYMS = 48,
	MMAP_INFO = 64,
	DRIVES_INFO = 128,
	CONFIG_TABLE = 256,
	BOOT_LOADER_NAME = 512,
	APM_TABLE = 1024,
	VBE_INFO = 2048,
	FRAMEBUFFER_INFO = 4096
} typedef MultibootFlag;

#ifdef __MULTIBOOT__
MultibootPartialInfo gMbi = { 0 };
#else
extern MultibootPartialInfo gMbi;
#endif
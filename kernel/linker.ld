ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .text ALIGN(4K):
    {
        KEEP(*(.multiboot_header))
        KEEP(*(.text))
		*(.gnu.linkonce.t*)
    }
 
    /* Read-only data. */
    .rodata ALIGN(4K):
    {
	    start_ctors = .;
        KEEP(*(SORT(.init_array*)))  /* Note the "SORT" */
        end_ctors = .;

        KEEP(*(.rodata))
		*(.gnu.linkonce.t*)
    }
 
    /* Read-write data (initialized) */
    .data ALIGN(4K):
    {
        KEEP(*(.data))
		*(.gnu.linkonce.t*)
    }
 
    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K):
    {
        KEEP(*(COMMON))
		*(.gnu.linkonce.t*)
        KEEP(*(.bss))
    }
 
    /* The compiler may produce other sections, by default it will put them in
       a segment with the same name. Simply add stuff here as needed. */
}
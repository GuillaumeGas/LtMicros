MBALIGN  equ  1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; Provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; This is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum of above, to prove we are multiboot

section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
extern kmain
global _start

extern start_ctors               ; beginning and end
extern end_ctors                 ; of the respective

; reserve initial kernel stack space
STACKSIZE equ 0x4000             ; that's 16k.

_start:
        mov  esp, stack + STACKSIZE         ; set up the stack
        mov  [magic], eax                   ; Multiboot magic number
        mov  [mbd], ebx                     ; Multiboot info structure
 
		mov  ebx, start_ctors    ; call the constructors
		jmp  .ctors_until_end
	.call_constructor:
		call [ebx]
		add  ebx,4
	.ctors_until_end:
		cmp  ebx, end_ctors
		jb   .call_constructor

		mov eax, [magic]
		mov ebx, [mbd]
		push eax
		push ebx
        call kmain

        cli ; stop interrupts
        hlt ; halt the CPU

section .bss
 
align 4
magic: resd 1
mbd:   resd 1
stack: resb STACKSIZE                   ; reserve 16k stack on a doubleword boundary
[BITS 32]

extern DebugIsr
extern BreakpointIsr

global _asm_debug_isr
global _asm_breakpoint_isr

%macro  SAVE_REGS_DBG 0
    pushad
    push ds
    push es
    push fs
    push gs
	push ss

	mov ebx, cr0
	push ebx
	mov ebx, cr2
	push ebx
	mov ebx, cr3
	push ebx

	push esp

	mov bx, 0x10
	mov ds, bx
%endmacro

%macro  RESTORE_REGS_DBG 0
    ; pop crX et esp
	pop ebx
	pop ebx
	pop ebx
	pop ebx
	pop ss
    pop gs
    pop fs
    pop es
    pop ds
	; pop eflags
	; pop ebx
    popad
%endmacro

%macro  EOI 0 		; EOI (End Of Interrupt)
	mov al, 0x20
	out 0x20, al
%endmacro

%macro  INT_PROLOG_DBG 0
	SAVE_REGS_DBG
%endmacro

%macro  INT_EPILOG_DBG 0
	EOI
	RESTORE_REGS_DBG
	iret
%endmacro

_asm_debug_isr:
	INT_PROLOG_DBG
	call DebugIsr
	INT_EPILOG_DBG

_asm_breakpoint_isr:
	INT_PROLOG_DBG
	call BreakpointIsr
	INT_EPILOG_DBG
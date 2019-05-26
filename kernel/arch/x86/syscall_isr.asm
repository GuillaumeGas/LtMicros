[BITS 32]

%include "arch/x86/isr_utils.asm"

extern syscall_isr

global _asm_syscall_isr

_asm_syscall_isr:
	INT_PROLOG
	call syscall_isr
	INT_EPILOG
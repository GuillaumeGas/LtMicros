[BITS 32]

%include "arch/x86/isr_utils.asm"

extern clock_isr

global _asm_clock_isr

_asm_clock_isr:
	INT_PROLOG
	call clock_isr
	INT_EPILOG
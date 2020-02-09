[BITS 32]

%include "arch/x86/isr_utils.asm"

extern ContextSwitchIsr

global _asm_context_swtich_isr

_asm_context_swtich_isr:
	INT_PROLOG
	call ContextSwitchIsr
	INT_EPILOG
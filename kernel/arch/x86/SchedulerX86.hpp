#pragma once

#define ISR_INDEX_CONTEXT_SWITCH 49
#define __contextSwitchInt() asm("int $49")

extern "C" void _asm_context_swtich_isr(void);
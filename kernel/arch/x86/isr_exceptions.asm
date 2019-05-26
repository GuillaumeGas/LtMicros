[BITS 32]

%include "arch/x86/isr_utils.asm"

;;; These ISRs addresses, related to processor exceptions/trap and the default isr, are stored in the IDT (see Idt.cpp)
;;; They represent the entry point of the following extern interrupt request, all registers are pushed on the stack
;;; so that the real isr will be able to access to the interrupted task context.
;;;
;;; This context, pushed on the stack, is then usable by using the kernel esp also pushed on the stack before calling the real isr routine.

;;; Default isr
extern default_isr
global _asm_default_isr

;;; Processor exceptions & faults
extern divided_by_zero_isr
extern debug_isr
extern non_maskable_int_isr
extern breakpoint_isr	
extern overflow_isr
extern bound_range_exceeded_isr
extern invalid_opcode_isr
extern device_not_available_isr
extern double_fault_isr
extern invalid_tss_isr
extern segment_not_present_isr
extern stack_segment_fault_isr
extern general_protection_fault_isr
extern page_fault_isr
extern x87_floating_point_isr
extern alignment_check_isr
extern machine_check_isr
extern simd_floating_point_isr
extern virtualization_isr
extern security_isr

;;; Processor exceptions & faults	
global _asm_divided_by_zero_isr
global _asm_non_maskable_int_isr
global _asm_overflow_isr
global _asm_bound_range_exceeded_isr
global _asm_invalid_opcode_isr
global _asm_device_not_available_isr
global _asm_double_fault_isr
global _asm_invalid_tss_isr
global _asm_segment_not_present_isr
global _asm_stack_segment_fault_isr
global _asm_general_protection_fault_isr
global _asm_page_fault_isr
global _asm_x87_floating_point_isr
global _asm_alignment_check_isr
global _asm_machine_check_isr
global _asm_simd_floating_point_isr
global _asm_virtualization_isr
global _asm_security_isr

_asm_default_isr:
	INT_PROLOG_EXCEPTION
	call default_isr
	INT_EPILOG_EXCEPTION

_asm_divided_by_zero_isr:
	INT_PROLOG_EXCEPTION
	call divided_by_zero_isr
	INT_EPILOG_EXCEPTION

_asm_non_maskable_int_isr:
	INT_PROLOG_EXCEPTION
	call non_maskable_int_isr
	INT_EPILOG_EXCEPTION

_asm_overflow_isr:
	INT_PROLOG_EXCEPTION
	call overflow_isr
	INT_EPILOG_EXCEPTION

_asm_bound_range_exceeded_isr:
	INT_PROLOG_EXCEPTION
	call bound_range_exceeded_isr
	INT_EPILOG_EXCEPTION

_asm_invalid_opcode_isr:
	INT_PROLOG_EXCEPTION
	call invalid_opcode_isr
	INT_EPILOG_EXCEPTION

_asm_device_not_available_isr:
	INT_PROLOG_EXCEPTION
	call device_not_available_isr
	INT_EPILOG_EXCEPTION

_asm_double_fault_isr:
	INT_PROLOG_EXCEPTION
	call double_fault_isr
	INT_EPILOG_EXCEPTION

_asm_invalid_tss_isr:
	INT_PROLOG_EXCEPTION
	call invalid_tss_isr
	INT_EPILOG_EXCEPTION

_asm_segment_not_present_isr:
	INT_PROLOG_EXCEPTION
	call segment_not_present_isr
	INT_EPILOG_EXCEPTION

_asm_stack_segment_fault_isr:
	INT_PROLOG_EXCEPTION
	call stack_segment_fault_isr
	INT_EPILOG_EXCEPTION

_asm_general_protection_fault_isr:
	INT_PROLOG_EXCEPTION
	call general_protection_fault_isr
	INT_EPILOG_EXCEPTION

_asm_page_fault_isr:
	INT_PROLOG_EXCEPTION
	call page_fault_isr
	INT_EPILOG_EXCEPTION

_asm_x87_floating_point_isr:
	INT_PROLOG_EXCEPTION
	call x87_floating_point_isr
	INT_EPILOG_EXCEPTION

_asm_alignment_check_isr:
	INT_PROLOG_EXCEPTION
	call alignment_check_isr
	INT_EPILOG_EXCEPTION

_asm_machine_check_isr:
	INT_PROLOG_EXCEPTION
	call machine_check_isr
	INT_EPILOG_EXCEPTION

_asm_simd_floating_point_isr:
	INT_PROLOG_EXCEPTION
	call simd_floating_point_isr
	INT_EPILOG_EXCEPTION

_asm_virtualization_isr:
	INT_PROLOG_EXCEPTION
	call virtualization_isr
	INT_EPILOG_EXCEPTION

_asm_security_isr:
	INT_PROLOG_EXCEPTION
	call security_isr
	INT_EPILOG_EXCEPTION
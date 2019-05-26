[BITS 32]

global _gdtLoad
global _tssLoad

extern gTss
	
;;; Tansmits the gdt address to the processor
;;; Param : gdt address
_gdtLoad:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	lgdt [eax]

	;; We initializes data segments (0x10 because third entry in gdt)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;; a jump inst to update the code segment register
	jmp 0x08:next
next:
	leave
	ret

;;; Indicates what is the tss segment selector in the gdt
;;; Param : tss segment selector (u16)
_tssLoad:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov eax, [ebp+8]
	ltr ax

	leave
	ret

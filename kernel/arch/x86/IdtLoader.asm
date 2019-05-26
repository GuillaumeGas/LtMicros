[BITS 32]

global _idtLoad

;;; Asks the processor to load the IDT
;;; Param : Idt base address
_idtLoad:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	lidt [eax]

	leave
	ret
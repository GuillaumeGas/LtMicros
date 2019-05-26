[BITS 32]

global _init_vmm
global _setCurrentPageDirectory
global _getCurrentPageDirectory

;;; Put the page directory physical address in the cr3 register
;;; Set the pagging bit (31) in cr0 to enable pagging
_init_vmm:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	
	leave
	ret

_setCurrentPageDirectory:
	push ebp
	mov ebp, esp

	push eax
	mov eax, [ebp+8]
	mov cr3, eax
	pop eax

	leave
	ret

_getCurrentPageDirectory:
    push ebp
    mov ebp, esp

    mov eax, cr3

    leave
    ret
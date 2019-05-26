[BITS 32]

%define KERNEL_MODE 0
%define USER_MODE 1

extern gTss
global _startOrResumeThread

_startOrResumeThread:
	DISABLE_IRQ

	push ebp
	mov esi, esp

	mov eax, [esi+8]
	mov cr3, eax

	mov eax, [esi+76]
	cmp eax, KERNEL_MODE
	jne user_mode

kernel_mode:
	mov ebx, [esi+16]
	mov esp, ebx

	; eflags
	push dword [esi+20]
	; cs
	push dword [esi+24]
	; eip
	push dword [esi+28]

	jmp next

user_mode:
	; ss
	push dword [esi+12]
	; esp
	push dword [esi+16]
	; eflags
	push dword [esi+20]
	; cs
	push dword [esi+24]
	; eip
	push dword [esi+28]
	
next:
	push dword [esi+32]
	push dword [esi+36]
	push dword [esi+40]
	push dword [esi+44]
	push dword [esi+48]
	push dword [esi+52]
	push dword [esi+56]
	push dword [esi+60]
	push dword [esi+64]
	push dword [esi+68]
	push dword [esi+72]

	pop gs
	pop fs
	pop es
	pop ds
	pop edi
	pop esi
	pop ebp
	pop ebx
	pop edx
	pop ecx
	pop eax

	mov ax, 0x23
	mov ds, ax
	
	; PIC
	mov al, 0x20
	out 0x20, al

	iret
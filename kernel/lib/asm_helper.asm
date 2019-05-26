[BITS 32]

;global _mod
global _acquire_lock
global _release_lock

;;; Regroups functions that can't be written in C easily

; TODO : couldn't call this function without a weird error in MemoryPoolManager.cpp...
_mod:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]
	mov ebx, [ebp+12]

	;div ebx

	;; We retrieve the remainder in edx
	mov eax, 0

	leave
	ret

_acquire_lock:
	push ebp
	mov ebp, esp

	mov ebx, [ebp+8]
	mov eax, 0
	mov ecx, 1

.spin_wait:
	lock cmpxchg [ebx], ecx

	cmp dword eax, 1
	je .spin_wait

	leave
	ret

_release_lock:
	push ebp
	mov ebp, esp

	mov eax, [ebp+8]

	mov dword [eax], 0

	leave
	ret
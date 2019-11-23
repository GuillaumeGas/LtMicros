[BITS 32]

;;; Theses macro are usefull to handle interrupt requests
;;; The processor doesn't push all the current task context and that's what we do here
;;; 
;;; Usage :
;;;   In the isr_asm_file.asm :
;;;      INT_PROLOG
;;;      call my_isr
;;;      INT_EPILOG
;;;
;;;   In the isr_c_file.c :
;;;      void my_isr(InterruptContext * context)
;;;      {
;;;          // isr stuff
;;;      }
;;;
;;; Note : The 'EXCEPTION' includes an error code in the context structure (InterruptContextWithCode)

%macro  SAVE_REGS 0
    pushad
    push ds
    push es
    push fs
    push gs
	push ebx
	mov bx, 0x10
	mov ds, bx
	pop ebx

	push esp
%endmacro

%macro  SAVE_REGS_EXCEPTION 0
    pushad
	pushfd
    push ds
    push es
    push fs
    push gs

	mov ebx, cr0
	push ebx
	mov ebx, cr2
	push ebx
	mov ebx, cr3
	push ebx

	push esp

	mov bx, 0x10
	mov ds, bx
%endmacro

%macro  RESTORE_REGS 0
	pop ebx ; actualy, pop esp
    pop gs
    pop fs
    pop es
    pop ds
    popad
%endmacro

%macro  RESTORE_REGS_EXCEPTION 0
    ; pop crX et esp
	pop ebx
	pop ebx
	pop ebx
	pop ebx
    pop gs
    pop fs
    pop es
    pop ds
	; pop eflags
	pop ebx
    popad
	add esp, 4
%endmacro

%macro  EOI 0 		; EOI (End Of Interrupt)
	mov al, 0x20
	out 0x20, al
%endmacro

%macro  INT_PROLOG 0
	SAVE_REGS
%endmacro

%macro  INT_PROLOG_EXCEPTION 0
	SAVE_REGS_EXCEPTION
%endmacro

%macro  INT_EPILOG 0
	EOI
	RESTORE_REGS
	iret
%endmacro

%macro  INT_EPILOG_EXCEPTION 0
	EOI
	RESTORE_REGS_EXCEPTION
	iret
%endmacro
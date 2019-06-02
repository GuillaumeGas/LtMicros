[BITS 32]

%define SYSCALL_INTERRUPT 0x30

%define SYSCALL_PRINT         0x0

global _print

_print:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the pointer to the string on the stack
    mov eax, SYSCALL_PRINT

    int SYSCALL_INTERRUPT

    leave
    ret
[BITS 32]

%define SYSCALL_INTERRUPT 0x30

%define SYSCALL_PRINT_CHAR   0x0
%define SYSCALL_PRINT_STR    0x1
%define SYSCALL_SBRK         0x2

global _print
global _printChar
global _sbrk

_print:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the pointer to the string on the stack
    mov eax, SYSCALL_PRINT_STR

    int SYSCALL_INTERRUPT

    leave
    ret

_printChar:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the char on the stack
    mov eax, SYSCALL_PRINT_CHAR

    int SYSCALL_INTERRUPT

    leave
    ret

_sbrk:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the parameter on the stack
    mov eax, SYSCALL_SBRK

    int SYSCALL_INTERRUPT

    leave
    ret
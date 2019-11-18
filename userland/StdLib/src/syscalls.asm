[BITS 32]

%define SYSCALL_INTERRUPT 0x30

%define SYSCALL_PRINT_CHAR     0x0
%define SYSCALL_PRINT_STR      0x1
%define SYSCALL_SBRK           0x2
%define SYS_IPC_SERVER_CREATE  0x3
%define SYS_IPC_SERVER_CONNECT 0X4
%define SYS_IPC_SEND           0x5
%define SYS_IPC_RECEIVE        0x6
%define SYS_ENTER_SCREEN_CRITICAL_SECTION 0x7
%define SYS_LEAVE_SCREEN_CRITICAL_SECTION 0x8

global _print
global _printChar
global _sbrk
global _ipcServerCreate
global _ipcServerConnect
global _ipcSend
global _ipcReceive
global _sysEnterScreenCriticalSection
global _sysLeaveScreenCriticalSection

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

_ipcServerCreate:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (serverId) on the stack
	mov ecx, [ebp+12]  ; we retrieve the second parameter (handle) on the stack
    mov eax, SYS_IPC_SERVER_CREATE

    int SYSCALL_INTERRUPT

	mov [ecx], ebx

    leave
    ret

_ipcServerConnect:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (serverId) on the stack
	mov ecx, [ebp+12]  ; we retrieve the second parameter (handle) on the stack
    mov eax, SYS_IPC_SERVER_CONNECT

    int SYSCALL_INTERRUPT

	mov [ecx], ebx

    leave
    ret

_ipcSend:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (ipcHandle) on the stack
	mov ecx, [ebp+12] ; we retrieve the second parameter (message pointer) on the stack
	mov edx, [ebp+16] ; we retrieve the third parameter (size) on the stack
    mov eax, SYS_IPC_SEND

    int SYSCALL_INTERRUPT

    leave
    ret

_ipcReceive:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (ipcHandle) on the stack
	mov ecx, [ebp+12] ; we retrieve the second parameter (message pointer) on the stack
	mov edx, [ebp+16] ; we retrieve the third parameter (size pointer) on the stack
    mov eax, SYS_IPC_RECEIVE

    int SYSCALL_INTERRUPT

    leave
    ret

_sysEnterScreenCriticalSection:
    push ebp
    mov ebp, esp

    mov eax, SYS_ENTER_SCREEN_CRITICAL_SECTION

    int SYSCALL_INTERRUPT

    leave
    ret

_sysLeaveScreenCriticalSection:
    push ebp
    mov ebp, esp

    mov eax, SYS_LEAVE_SCREEN_CRITICAL_SECTION

    int SYSCALL_INTERRUPT

    leave
    ret
[BITS 32]

%define SYSCALL_INTERRUPT                 0x30

%define SYSCALL_PRINT_CHAR                0x0
%define SYSCALL_PRINT_STR                 0x1
%define SYSCALL_SBRK                      0x2
%define SYS_IPC_SERVER_CREATE             0x3
%define SYS_IPC_SERVER_CONNECT            0X4
%define SYS_IPC_SEND                      0x5
%define SYS_IPC_RECEIVE                   0x6
%define SYS_ENTER_SCREEN_CRITICAL_SECTION 0x7
%define SYS_LEAVE_SCREEN_CRITICAL_SECTION 0x8
%define SYS_RAISE_THREAD_PRIORITY         0x9
%define SYS_LOWER_THREAD_PRIORITY         0xA

global _sysPrint
global _sysPrintChar
global _sysSbrk
global _sysIpcServerCreate
global _sysIpcServerConnect
global _sysIpcSend
global _sysIpcReceive
global _sysEnterScreenCriticalSection
global _sysLeaveScreenCriticalSection
global _sysRaiseThreadPriority
global _sysLowerThreadPriority

_sysPrint:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the pointer to the string on the stack
    mov eax, SYSCALL_PRINT_STR

    int SYSCALL_INTERRUPT

    leave
    ret

_sysPrintChar:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the char on the stack
    mov eax, SYSCALL_PRINT_CHAR

    int SYSCALL_INTERRUPT

    leave
    ret

_sysSbrk:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; we retrieve the parameter on the stack
    mov eax, SYSCALL_SBRK

    int SYSCALL_INTERRUPT

    leave
    ret

_sysIpcServerCreate:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (serverId) on the stack
	mov ecx, [ebp+12]  ; we retrieve the second parameter (handle) on the stack
    mov eax, SYS_IPC_SERVER_CREATE

    int SYSCALL_INTERRUPT

	mov [ecx], ebx

    leave
    ret

_sysIpcServerConnect:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (serverId) on the stack
	mov ecx, [ebp+12]  ; we retrieve the second parameter (handle) on the stack
    mov eax, SYS_IPC_SERVER_CONNECT

    int SYSCALL_INTERRUPT

	mov [ecx], ebx

    leave
    ret

_sysIpcSend:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the first parameter (ipcHandle) on the stack
	mov ecx, [ebp+12] ; we retrieve the second parameter (message pointer) on the stack
	mov edx, [ebp+16] ; we retrieve the third parameter (size) on the stack
    mov eax, SYS_IPC_SEND

    int SYSCALL_INTERRUPT

    leave
    ret

_sysIpcReceive:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]  ; we retrieve the ipc receive syscall parameter pointer on the stack
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

_sysRaiseThreadPriority:
    push ebp
    mov ebp, esp

    mov eax, SYS_RAISE_THREAD_PRIORITY

    int SYSCALL_INTERRUPT

    leave
    ret

_sysLowerThreadPriority:
    push ebp
    mov ebp, esp

    mov eax, SYS_LOWER_THREAD_PRIORITY

    int SYSCALL_INTERRUPT

    leave
    ret
OBJ=ltkernel
CC=g++ -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-rtti -m32 -Wno-write-strings -I ../
ASM=nasm -f elf32
LD=ld -m elf_i386 -nostdlib -T linker.ld

BOOT=../boot/bootsector.o
ROOT=kmain.o Kernel.o Logger.o cppsupport.o
DRIVERS=Screen.o Pic.o Clock.o clock_isr.o Serial.o BaseDriver.o
LIB=StdLib.o StdIo.o asm_helper.o StdMem.o List.o CriticalSection.o Status.o
ARCHX86=gdtLoader.o Gdt.o Idt.o idtLoader.o isr_utils.o isr_exceptions_asm.o isr_exceptions.o InterruptContext.o Pmm.o Vmm.o vmm_utils.o Process.o Thread.o thread_utils.o Syscalls.o syscall_isr.o PageFault.o
MEM=PagePool.o Heap.o Vad.o
SYSCALLS=SyscallsHandler.o
TASK=ProcessManager.o ThreadManager.o Scheduler.o Ipc.o IpcBuffer.o
MODULE=Module.o Elf.o
HANDLE=HandleManager.o
DEBUG=LtDbg.o ltdbg_isr.o LtDbgCom.o

all: $(OBJ) 

.PHONY: doc clean

clean:
	rm -f $(OBJ) *.o
	rm -rf doc/html
	rm -rf doc/latex

doc:
	doxygen doc/doxyConf

ltkernel: $(BOOT) $(ROOT) $(DRIVERS) $(LIB) $(ARCHX86) $(MEM) $(SYSCALLS) $(TASK) $(MODULE) $(HANDLE) $(DEBUG)
	$(LD) $^ -o $@

# ROOT
kmain.o: kmain.cpp
	$(CC) -c $^

Kernel.o: Kernel.cpp
	$(CC) -c $^
	
Logger.o: Logger.cpp
	$(CC) -c $^

cppsupport.o: cppsupport.cpp
	$(CC) -c $^

# DRIVERS DIRECTORY
Screen.o: drivers/Screen.cpp
	$(CC) -c $^

Pic.o: drivers/Pic.cpp
	$(CC) -c $^

Clock.o: drivers/Clock.cpp
	$(CC) -c $^

clock_isr.o: drivers/clock_isr.asm
	$(ASM) -o $@ $^

Serial.o: drivers/Serial.cpp
	$(CC) -c $^

BaseDriver.o: drivers/BaseDriver.cpp
	$(CC) -c $^

# LIB DIRECTORY
StdLib.o: lib/StdLib.cpp
	$(CC) -c $^
	
StdIo.o: lib/StdIo.cpp
	$(CC) -c $^

asm_helper.o: lib/asm_helper.asm
	$(ASM) -o $@ $^

StdMem.o: lib/StdMem.cpp
	$(CC) -c $^

List.o: lib/List.cpp
	$(CC) -c $^

CriticalSection.o: lib/CriticalSection.cpp
	$(CC) -c $^

Status.o: lib/Status.cpp
	$(CC) -c $^

# X86 ARCH DIRECTORY
Gdt.o: arch/x86/Gdt.cpp
	$(CC) -c $^

gdtLoader.o: arch/x86/GdtLoader.asm
	$(ASM) -o $@ $^

Idt.o: arch/x86/Idt.cpp
	$(CC) -c $^

idtLoader.o: arch/x86/IdtLoader.asm
	$(ASM) -o $@ $^

isr_utils.o: arch/x86/isr_utils.asm
	$(ASM) -o $@ $^

isr_exceptions_asm.o: arch/x86/isr_exceptions.asm
	$(ASM) -o $@ $^

isr_exceptions.o: arch/x86/isr_exceptions.cpp
	$(CC) -c $^

InterruptContext.o: arch/x86/InterruptContext.cpp
	$(CC) -c $^

Pmm.o: arch/x86/Pmm.cpp
	$(CC) -c $^

Vmm.o: arch/x86/Vmm.cpp
	$(CC) -c $^

vmm_utils.o: arch/x86/vmm_utils.asm
	$(ASM) -o $@ $^

Process.o: arch/x86/Process.cpp
	$(CC) -c $^

Thread.o: arch/x86/Thread.cpp
	$(CC) -c $^

thread_utils.o: arch/x86/thread_utils.asm
	$(ASM) -o $@ $^

Syscalls.o: arch/x86/Syscalls.cpp
	$(CC) -c $^

syscall_isr.o: arch/x86/syscall_isr.asm
	$(ASM) -o $@ $^

PageFault.o: arch/x86/Exceptions/PageFault.cpp
	$(CC) -c $^

# MEM DIRECTORY
PagePool.o: mem/PagePool.cpp
	$(CC) -c $^

Heap.o: mem/Heap.cpp
	$(CC) -c $^

Vad.o: mem/Vad.cpp
	$(CC) -c $^

# SYSCALLS DIRECTORY
SyscallsHandler.o: syscalls/SyscallsHandler.cpp
	$(CC) -c $^

# TASK DIRECTORY
ProcessManager.o: task/ProcessManager.cpp
	$(CC) -c $^

ThreadManager.o: task/ThreadManager.cpp
	$(CC) -c $^

Scheduler.o: task/Scheduler.cpp
	$(CC) -c $^

Ipc.o: task/Ipc/Ipc.cpp
	$(CC) -c $^

IpcBuffer.o: task/Ipc/IpcBuffer.cpp
	$(CC) -c $^

# MODULE DIRECTORY
Module.o: module/Module.cpp
	$(CC) -c $^

Elf.o: module/Elf.cpp
	$(CC) -c $^

# HANDLE DIRECTORY
HandleManager.o: handle/HandleManager.cpp
	$(CC) -c $^

# DEBUG DIRECTORY
LtDbg.o: debug/LtDbg.cpp
	$(CC) -c $^

ltdbg_isr.o: debug/ltdbg_isr.asm
	$(ASM) -o $@ $^

LtDbgCom.o: debug/LtDbgCom.cpp
	$(CC) -c $^
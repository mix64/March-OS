.code64
.globl _entry
_entry:
/* 
    boot_info form UEFI in %rcx
    See: https://wiki.osdev.org/Calling_Conventions (Microsoft x64 calling convention)
*/
    pushq %rcx

    xorq %rax, %rax
    movq %rax, %rbx
    movq %rax, %rcx
    movq %rax, %rdx
    movq %rax, %rsi
    movq %rax, %rdi
    movq %rax, %r8
    movq %rax, %r9
    movq %rax, %r10
    movq %rax, %r11
    movq %rax, %r12
    movq %rax, %r13
    movq %rax, %r14
    movq %rax, %r15

/* 
    send boot_info to kernel_stub() use %rdi
    See: https://wiki.osdev.org/Calling_Conventions (System V X86_64 calling convention)
*/
    popq %rdi

    jmp kernel_stub

/* 
    Global Descriptor Table
    See: https://wiki.osdev.org/Global_Descriptor_Table

    OVMF uses the following GDT:
    0x00: Null descriptor
    0x08: DS (DPL=0, DC=1, RW=1, A=0)
    0x10: CS32 (DPL=0, DC=1, RW=1)
    0x18: DS (DPL=0, DC=1, RW=1, A=1)
    0x20: CS32 (DPL=0, DC=0, RW=1)
    0x28: CS16 (DPL=0, DC=0, RW=1)
    0x30: == 0x18
    0x38: CS64 (DPL=0, DC=0, RW=1)

    CS = 0x38
    ES/SS/DS/FS/GS = 0x30
    See: https://github.com/tianocore/edk2/blob/master/UefiCpuPkg/CpuDxe/CpuGdt.c

    TODO: Need to implement a User Segment?
*/

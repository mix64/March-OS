.code64
.globl _entry
_entry:
/* setup GDT */
    lea gdtdesc(%rip), %rax          /* Load gdtr */
    lgdt (%rax)

    mov $0x10, %ax                   /* Kernel DS */
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    pushq $0x8                       /* Kernel CS  */
    lea .ret_label(%rip), %rax       /* get return address */
    pushq %rax
    lretq                            /* change code segment */
.ret_label:

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

.section .data
.p2align 3
.globl gdt
gdt:
    .quad 0x0000000000000000 /* Null descriptor */
    .quad 0x00af9a000000ffff /* Kernel CS64: base=0, limit=0xfffff, Access=0x9A, Flags=0xA  */
    .quad 0x00cf92000000ffff /* Kernel DS: base=0, limit=0xfffff, Access=0x92, Flags=0xC  */
    .quad 0x0000000000000000 /* Null descriptor */
    .quad 0x00cff2000000ffff /* User DS: base=0, limit=0xfffff, Access=0xF2, Flags=0xC  */
    .quad 0x00affa000000ffff /* User CS64: base=0, limit=0xfffff, Access=0xFA, Flags=0xA  */
    .quad 0x0000000000000000 /* Task State Segment [127:64] */
    .quad 0x0000000000000000 /* Task State Segment [63:0] */

.section .data
gdtdesc:
    .word (gdtdesc - gdt - 1) /* Limit (Size) */
    .quad gdt

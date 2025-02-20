.globl _syscall
.globl _sysret

.code64
_syscall:
    movq %rcx, %r15 # ring3 rip
    movq %rsp, %r14 # ring3 stack
    movq %r11, %r13 # ring3 rflags

    xorq %rcx, %rcx
    xorq %rax, %rax
    movl $0x175, %ecx # IA32_SYSENTER_ESP
    rdmsr

    movq %rax, %rsp
    pushq %r15 # save ring3 rip
    pushq %r14 # save ring3 stack
    pushq %r13 # save ring3 rflags
    call syscall
    popq %r14
    popq %r15
    popq %r13

_sysret:
    movq %r13, %r11
    movq %r14, %rsp
    movq %r15, %rcx
    sysretq
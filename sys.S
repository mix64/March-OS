.globl _sysenter
.globl _sysret

.code64
_sysenter:
    pushq %rcx # ring3 stack
    pushq %rdx # ring3 rip
    call syscall
    popq %rdx
    popq %rcx
_sysret:
    sysret
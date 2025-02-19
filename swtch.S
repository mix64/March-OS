# Context switch
#
#   void context_switch(context_t **old, context_t *new);
# 
# Save the current callee-saved registers in old,
# and restore previously-saved registers.

.globl context_switch
context_switch:
    # rdi = old, rsi = new

    # Save for new process
    pushq %rdx # ring3 rip
    pushq %rcx # ring3 stack

    # Save old callee-saved registers
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Switch stacks
    movq %rsp, (%rdi)
    movq %rsi, %rsp

    # Load new callee-saved registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    # Restore for new process
    popq %rcx
    popq %rdx
    ret
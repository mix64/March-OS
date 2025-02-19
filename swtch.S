# Context switch
#
#   void swtch(context_t **old, context_t *new, uint64 *pgdir);
# 
# Save the current registers on the stack,
# creating a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
    # rdi = old, rsi = new, rdx = pgdir

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
    ret
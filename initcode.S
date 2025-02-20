#include <syscall.h>

.code64
.globl start
start:
    # spawn("/bin/shell", NULL)
    movabs $init, %rdi
    xorq %rsi, %rsi
    movq $SYS_spawn, %rax # (SYS_spawn)
    syscall

exit:
    # exit(1)
    movq $0x1, %rdi
    movq $SYS_exit, %rax
    syscall

init:
    .string "/bin/shell\0"
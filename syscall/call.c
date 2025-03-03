#include <kernel.h>
#include <syscall.h>

extern int sys_spawn(char *filename);

void syscall(uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5,
             uint64 arg6, uint64 syscall_num) {
    debugf("#SYSCALL: %d\n", syscall_num);
    debugf("    args: %x %x %x %x %x %x\n", arg1, arg2, arg3, arg4, arg5, arg6);

    switch (syscall_num) {
        case SYS_spawn:
            sys_spawn((char *)arg1);
            break;
        case SYS_exit:
            kprintf("SYS_exit\n");
            break;
        default:
            kprintf("unknown syscall %d\n", syscall_num);
            break;
    }
    while (1);
}
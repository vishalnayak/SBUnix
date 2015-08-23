#include<sys/sbunix.h>
#include<sys/sys-handlers.h>
#include<sys/process.h>
#include<sys/gdt.h>
#include<sys/memory.h>
#include<sys/syscall.h>

extern struct process_queue *curr_task;
extern int setup;
void syscall_handler(struct syscall_frame *p_sysf)
{
    uint64_t n = 0, ret_val = 0, rcx = 0, r11 = 0, rsp = 0;
    __asm__ __volatile__("movq %%rax, %[n];"
            "movq %%rsp, %[rsp];"
            "movq %%rcx, %[rcx];"
            "movq %%r11, %[r11];":[n] "=r"(n)
            ,[rsp] "=r"(rsp)
            ,[rcx] "=r"(rcx)
            ,[r11] "=r"(r11)
            ::"memory");

    //for 0x4000e8. Dirty hack.
    if (curr_task->proc.rip == 0x4000e8) {
        curr_task->proc.forkrip = *(uint64_t *) (17 * 8 + rsp);
        curr_task->proc.rbx = *(uint64_t *) (rsp + 19 * 8);
        curr_task->proc.stack = (uint64_t *) (21 * 8 + rsp);
        curr_task->proc.rbp = (rsp + 23 * 8 + 0x18);
    } else {
        //for 0x4000f0
        curr_task->proc.forkrip = *(uint64_t *) (11 * 8 + rsp);
        curr_task->proc.rbx = *(uint64_t *) (rsp + 13 * 8);
        curr_task->proc.stack = (uint64_t *) (15 * 8 + rsp);
        curr_task->proc.rbp = (rsp + 17 * 8 + 0x18);
    }
    __asm__ __volatile__("movq %0, %%rsp"::
            "r"(curr_task->proc.kern_rsp):"memory");
    switch (n) {
        case SYS_clear:
            ret_val = syscall_clear();
            break;
        case SYS_getpid:
            ret_val = syscall_getpid();
            break;
        case SYS_write:
            ret_val =
                syscall_write((int32_t) p_sysf->a1,
                        (const void *) p_sysf->a2,
                        (uint64_t) p_sysf->a3);
            break;
        case SYS_exit:
            ret_val = syscall_exit((int32_t) p_sysf->a1);
            break;
        case SYS_mmap:
            ret_val =
                syscall_mmap((void *) p_sysf->a1, (uint64_t) p_sysf->a2,
                        (int32_t) p_sysf->a3, (int32_t) p_sysf->a4,
                        (int32_t) p_sysf->a5, (uint64_t) p_sysf->a6);
            break;
        case SYS_munmap:
            ret_val =
                syscall_munmap((void *) p_sysf->a1, (uint64_t) p_sysf->a2);
            break;
        case SYS_brk:
            ret_val = syscall_brk((void *) p_sysf->a1);
            break;
        case SYS_fork:
            ret_val = syscall_fork();
            printf("fork returning:[%d]\n", ret_val);
            break;
        case SYS_getppid:
            ret_val = syscall_getppid();
            break;
        case SYS_execve:
            ret_val =
                syscall_execve((const char *) p_sysf->a1,
                        (char *const *) p_sysf->a2,
                        (char *const *) p_sysf->a3);
            break;
        case SYS_wait4:
            ret_val =
                syscall_wait4((uint32_t) p_sysf->a1, (int32_t *) p_sysf->a2,
                        (int32_t) p_sysf->a3);
            break;
        case SYS_nanosleep:
            ret_val =
                syscall_nanosleep((const struct timespec *) p_sysf->a1,
                        (struct timespec *) p_sysf->a2);
            break;
        case SYS_alarm:
            ret_val = syscall_alarm((uint32_t) p_sysf->a1);
            break;
        case SYS_getcwd:
            ret_val =
                syscall_getcwd((char *) p_sysf->a1, (uint64_t) p_sysf->a2);
            break;
        case SYS_chdir:
            ret_val = syscall_chdir((const char *) p_sysf->a1);
            break;
        case SYS_open:
            ret_val =
                syscall_open((const char *) p_sysf->a1, (int32_t) p_sysf->a2);
            break;
        case SYS_read:
            ret_val =
                syscall_read((int32_t) p_sysf->a1, (void *) p_sysf->a2,
                        (uint64_t) p_sysf->a3);
            break;
        case SYS_lseek:
            ret_val =
                syscall_lseek((int32_t) p_sysf->a1, (uint64_t) p_sysf->a2,
                        (int32_t) p_sysf->a3);
            break;
        case SYS_close:
            ret_val = syscall_close((int32_t) p_sysf->a1);
            break;
        case SYS_pipe:
            ret_val = syscall_pipe((int32_t *) p_sysf->a1);
            break;
        case SYS_dup:
            ret_val = syscall_dup((int32_t) p_sysf->a1);
            break;
        case SYS_dup2:
            ret_val = syscall_dup2((int32_t) p_sysf->a1, (int32_t) p_sysf->a2);
            break;
        case SYS_getdents:
            ret_val =
                syscall_getdents((uint32_t) p_sysf->a1,
                        (struct dirent *) p_sysf->a2,
                        (uint32_t) p_sysf->a3);
            break;
        default:
            printf("syscall unrecognized: num=%d\n", n);
    }
    //    tss.rsp0 = (uint64_t) curr_task->proc.kern_stack;
    //           printf("ret_val[%d] ",ret_val);
    __asm__ __volatile__("movq %%rsp,%0;":"=r"(curr_task->proc.kern_rsp):);
    __asm__ __volatile__("movq %[rsp], %%rsp;"::[rsp] "r"(rsp):"memory");
    __asm__ __volatile__("movq %[val], %%fs;"::[val] "a"((uint64_t) 0x23));
    __asm__ __volatile__("movq %[val], %%gs;"::[val] "a"((uint64_t) 0x23));
    __asm__ __volatile__("movq %[val], %%ds;"::[val] "a"((uint64_t) 0x23));
    __asm__ __volatile__("movq %[val], %%es;"::[val] "a"((uint64_t) 0x23));
    __asm__ __volatile__("movq %[r11], %%r11;"::[r11] "a"(r11));
    __asm__ __volatile__("movq %[rcx], %%rcx;"::[rcx] "a"(rcx));
    __asm__ __volatile__("movq %[ret_val], %%rax;"::[ret_val]
            "a"(ret_val));
}

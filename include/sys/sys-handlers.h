#ifndef __SYS_HANDLERS_H
#define __SYS_HANDLERS_H

#define NAME_MAX 255

typedef int64_t time_t;
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct dirent {
    long d_ino;
    uint64_t d_off;
    unsigned short d_reclen;
    char d_name[NAME_MAX + 1];
};
uint64_t syscall_exit(int32_t status);
uint64_t syscall_getpid();
uint64_t syscall_getppid();
uint64_t syscall_fork();
uint64_t syscall_execve(const char *filename, char *const argv[],
        char *const envp[]);
uint64_t syscall_wait4(uint32_t pid, int32_t * status, int32_t options);
uint64_t syscall_nanosleep(const struct timespec *req,
        struct timespec *rem);
uint64_t syscall_alarm(uint32_t seconds);
uint64_t syscall_getcwd(char *buf, uint64_t size);
uint64_t syscall_chdir(const char *path);
uint64_t syscall_open(const char *pathname, int32_t flags);
uint64_t syscall_read(int32_t fd, void *buf, uint64_t count);
uint64_t syscall_write(int32_t fd, const void *buf, uint64_t count);
uint64_t syscall_close(int32_t fd);
uint64_t syscall_lseek(int32_t fildes, uint64_t offset, int32_t whence);
uint64_t syscall_pipe(int32_t filedes[2]);
uint64_t syscall_dup(int32_t oldfd);
uint64_t syscall_dup2(int32_t oldfd, int32_t newfd);
uint64_t syscall_getdents(uint32_t fd, struct dirent *dirp,
        uint32_t count);
uint64_t syscall_mmap(void *start, uint64_t length, int prot, int flags,
        int fd, uint64_t offset);
uint64_t syscall_munmap(void *start, uint64_t length);
uint64_t syscall_brk(void *end_data_segment);
uint64_t syscall_clear();
uint64_t syscall_ps();
uint64_t syscall_kill(int32_t option, int32_t pid);
typedef enum { BG_ENABLE } sched_param;
uint64_t syscall_sched_setparam(sched_param param, uint64_t value);
#endif

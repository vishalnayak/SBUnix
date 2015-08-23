#include <stdlib.h>
#include <syscall.h>
#include <stdarg.h>
#include <sbulib.h>
#include <stdio.h>
int errno;
int xatoi(const char *str)
{
    int n = 0;
    char *p_str = (char *) str;
    while (*p_str) {
        int d = 0;
        if ((*p_str - '0') >= 0 && (*p_str - '0') <= 9) {
            d = *p_str - '0';
        } else {
            if (('A' == *p_str) || ('a' == *p_str))
                d = 10;
            if (('B' == *p_str) || ('b' == *p_str))
                d = 11;
            if (('C' == *p_str) || ('c' == *p_str))
                d = 12;
            if (('D' == *p_str) || ('d' == *p_str))
                d = 13;
            if (('E' == *p_str) || ('e' == *p_str))
                d = 14;
            if (('F' == *p_str) || ('f' == *p_str))
                d = 15;
        }
        n = n * 16 + d;
        p_str++;
    }
    return n;
}

int atoi(const char *str)
{
    int n = 0;
    char *p_str = (char *) str;
    while (*p_str) {
        n = n * 10 + (*p_str++ - '0');
    }
    return n;
}

char *itoa(int value, char *str, int base)
{
    char *p_str = str;

    //int index = 0;
    int n = value;
    if (0 == n) {
        *p_str++ = '0';
        *p_str = '\0';
    } else {
        while (n) {
            if (16 == base) {
                int bval = (n % base);
                if (10 == bval) {
                    *p_str = 'A';
                } else if (11 == bval) {
                    *p_str = 'B';
                } else if (12 == bval) {
                    *p_str = 'C';
                } else if (13 == bval) {
                    *p_str = 'D';
                } else if (14 == bval) {
                    *p_str = 'E';
                } else if (15 == bval) {
                    *p_str = 'F';
                } else {
                    *p_str = (n % base) + '0';
                }
            } else {
                *p_str = (n % base) + '0';
            }
            n /= base;
            p_str++;
        }
        *p_str = '\0';
        int str_len = p_str - str;
        int i = 0;
        int ch = 0;
        for (i = 0; i < (str_len / 2); i++) {
            ch = *(str + i);
            *(str + i) = *(str + str_len - 1 - i);
            *(str + str_len - 1 - i) = ch;
        }
    }
    return str;
}

char *strcpy(char *dest, const char *src)
{
    char *p_dest = dest;
    const char *p_src = src;
    while (p_dest && p_src && *p_src != '\0') {
        *p_dest = *p_src;
        p_dest++;
        p_src++;
    }
    *p_dest = '\0';
    return dest;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 == *str2) {
        if (*str1 == 0 || *str2 == 0) {
            return 0;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

size_t strlen(const char *str)
{
    size_t count = 0;
    while (*str != '\0') {
        str++;
        count++;
    }
    return count;
}

size_t strcspn(const char *str1, const char *str2)
{
    size_t count = 0;
    int str1_len = strlen(str1);
    while (*str1++ != *str2 && count <= str1_len) {
        count++;
        if (count == str1_len) {
            break;
        }
    }
    if (count == str1_len) {
        return count;
    }
    return count;
}

char *strncpy(char *d, const char *s, size_t n)
{
    char *dest = d;
    const char *src = s;
    size_t i = 0;
    int src_len = strlen(src);

    //int dest_len = strlen(dest_len);
    for (i = 0; i < n && i < src_len; i++) {
        *(dest + i) = *src++;
    }
    if (src_len < n) {
        *(dest + i) = '\0';
    }
    return dest;
}

void *memset(void *dest, int c, size_t count)
{
    size_t i = 0;
    char *mem = dest;
    for (i = 0; i < count; i++) {
        if (mem)
            *mem++ = c;
    }
    return dest;
}

void exit(int status)
{
    syscall_int_1((uint64_t) SYS_exit, (uint64_t) status);
    return;
}

void *malloc(size_t size)
{
    size_t alloc_size = sizeof(vm_struct_t) + size;
    size_t addr =
        (size_t) syscall_int_6((uint64_t) SYS_mmap, (uint64_t) NULL,
                (uint64_t) alloc_size,
                (uint64_t) (PROT_READ | PROT_WRITE),
                (uint64_t) (MAP_ANONYMOUS | MAP_PRIVATE),
                (uint64_t) - 1, (uint64_t) 0);
    vm_struct_t *mem_block = (vm_struct_t *) addr;
    mem_block->size = alloc_size;
    return (void *) (addr + sizeof(vm_struct_t));
}

void free(void *ptr)
{
    int64_t ret_val = 0;
    if (ptr) {
        size_t addr = (size_t) ptr - sizeof(vm_struct_t);
        size_t size = ((vm_struct_t *) addr)->size;

        //printf("calling free: addr=%x size=%d\n",addr,size);
        ret_val =
            (int64_t) syscall_int_2((uint64_t) SYS_munmap, (uint64_t) addr,
                    (uint64_t) size);
        if (ret_val < 0) {
            errno = -1 * ret_val;
        }
    }
    return;
}

int brk(void *end_data_segment)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_1((uint64_t) SYS_brk,
            (uint64_t) end_data_segment);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

pid_t fork(void)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_int_0((uint64_t) SYS_fork);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (pid_t) ret_val;
}

pid_t getpid(void)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_int_0((uint64_t) SYS_getpid);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (pid_t) ret_val;
}

pid_t getppid(void)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_int_0((uint64_t) SYS_getppid);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (pid_t) ret_val;
}

int execve(const char *filename, char *const argv[], char *const envp[])
{
    int64_t ret_val =
        (int64_t) syscall_int_3((uint64_t) SYS_execve, (uint64_t) filename,
                (uint64_t) argv, (uint64_t) envp);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

pid_t waitpid(pid_t pid, int *status, int options)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_int_3((uint64_t) SYS_wait4, (uint64_t) pid,
            (uint64_t) status,
            (uint64_t) options);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (pid_t) ret_val;
}

unsigned int sleep(unsigned int seconds)
{
    struct timespec req;
    struct timespec rem;
    req.tv_sec = (time_t) seconds;
    req.tv_nsec = 0;
    int64_t ret_val = 0;
    while (-1 ==
            (ret_val =
             syscall_int_2((uint64_t) SYS_nanosleep, (uint64_t) & req,
                 (uint64_t) & rem))) {
        if (EINTR != errno) {
            printf("nanosleep() failed.\n");
            exit(1);
        }
        req.tv_sec = rem.tv_sec;
        req.tv_nsec = rem.tv_nsec;
        rem.tv_sec = 0;
        rem.tv_nsec = 0;
    }
    return (unsigned int) ret_val;
}

unsigned int alarm(unsigned int seconds)
{
    int64_t ret_val = 0;
    ret_val =
        (int64_t) syscall_int_1((uint64_t) SYS_alarm, (uint64_t) seconds);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (unsigned int) ret_val;
}

char *getcwd(char *buf, size_t size)
{
    int64_t ret_val = 0;
    ret_val =
        (int64_t) syscall_int_2((uint64_t) SYS_getcwd, (uint64_t) buf,
                (uint64_t) size);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return NULL;
    }
    return (char *) ret_val;
}

int chdir(const char *path)
{
    int64_t ret_val = 0;
    ret_val = (int64_t) syscall_1((uint64_t) SYS_chdir, (uint64_t) path);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int open(const char *pathname, int flags)
{
    int64_t ret_val =
        (int64_t) syscall_2((uint64_t) SYS_open, (uint64_t) pathname,
                (uint64_t) flags);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

ssize_t read(int fd, void *buf, size_t count)
{
    int64_t ret_val =
        (int64_t) syscall_3((uint64_t) SYS_read, (uint64_t) fd,
                (uint64_t) buf, (uint64_t) count);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (ssize_t) ret_val;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    int64_t ret_val =
        (int64_t) syscall_3((uint64_t) SYS_write, (uint64_t) fd,
                (uint64_t) buf, (uint64_t) count);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (ssize_t) ret_val;
}

off_t lseek(int fildes, off_t offset, int whence)
{
    int64_t ret_val =
        (int64_t) syscall_3((uint64_t) SYS_lseek, (uint64_t) fildes,
                (uint64_t) offset, (uint64_t) whence);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (off_t) ret_val;
}

int close(int fd)
{
    int64_t ret_val =
        (int64_t) syscall_1((uint64_t) SYS_close, (uint64_t) fd);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int pipe(int filedes[2])
{
    int64_t ret_val =
        (int64_t) syscall_1((uint64_t) SYS_pipe, (uint64_t) filedes);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int dup(int oldfd)
{
    int64_t ret_val =
        (int64_t) syscall_int_1((uint64_t) SYS_dup, (uint64_t) oldfd);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int dup2(int oldfd, int newfd)
{
    int64_t ret_val =
        (int64_t) syscall_int_2((uint64_t) SYS_dup2, (uint64_t) oldfd,
                (uint64_t) newfd);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
    int64_t ret_val =
        (int64_t) syscall_3((uint64_t) SYS_getdents, (uint64_t) fd,
                (uint64_t) dirp, (uint64_t) count);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

DIR *opendir(const char *name)
{
    int dir_fd = -1;
    if (-1 == (dir_fd = open(name, O_DIRECTORY | O_RDONLY))) {
        printf("open() failed. name=%s flags=O_DIRECTORY|O_RDONLY", name);
        exit(1);
    }
    DIR *dir = (DIR *) malloc(sizeof(DIR));
    dir->dir_fd = dir_fd;
    dir->p_dir = (struct dirent *) malloc(sizeof(struct dirent));
    return dir;
}

struct dirent *readdir(DIR * dir)
{
    struct dirent *dir_ent = dir->p_dir;
    int bytes = 0;
    if (-1 ==
            (bytes = getdents(dir->dir_fd, dir_ent, sizeof(struct dirent)))) {
        printf("getdents() failed. dir_fd=%d &dir_ent=%x\n",
                dir->dir_fd, dir_ent);
        exit(1);
    }
    if (0 == bytes) {
        return NULL;
    }
    if (-1 == lseek(dir->dir_fd, dir_ent->d_off, SEEK_SET)) {
        printf("getdents() failed. dir_fd=%d &dir_ent=%x\n",
                dir->dir_fd, dir_ent);
        exit(1);
    }
    return dir->p_dir;
}

int closedir(DIR * dir)
{
    int64_t ret_val = 0;
    ret_val = close(dir->dir_fd);
    free(dir->p_dir);
    free(dir);
    return (int) ret_val;
}

int clear()
{
    int64_t ret_val = (int64_t) syscall_0((uint64_t) SYS_clear);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

char errstr[10] = { 0 };

char *error()
{
    switch (errno) {
        case (EPERM):
            strcpy(errstr, "EPERM");
            break;
        case (ENOENT):
            strcpy(errstr, "ENOENT");
            break;
        case (ESRCH):
            strcpy(errstr, "ESRCH");
            break;
        case (EINTR):
            strcpy(errstr, "EINTR");
            break;
        case (EIO):
            strcpy(errstr, "EIO");
            break;
        case (ENXIO):
            strcpy(errstr, "ENXIO");
            break;
        case (E2BIG):
            strcpy(errstr, "E2BIG");
            break;
        case (ENOEXEC):
            strcpy(errstr, "ENOEXEC");
            break;
        case (EBADF):
            strcpy(errstr, "EBADF");
            break;
        case (ECHILD):
            strcpy(errstr, "ECHILD");
            break;
        case (EAGAIN):
            strcpy(errstr, "EAGAIN");
            break;
        case (ENOMEM):
            strcpy(errstr, "ENOMEM");
            break;
        case (EACCES):
            strcpy(errstr, "EACCESS");
            break;
        case (EFAULT):
            strcpy(errstr, "EFAULT");
            break;
        case (ENOTBLK):
            strcpy(errstr, "ENOTBLK");
            break;
        case (EBUSY):
            strcpy(errstr, "EBUSY");
            break;
        case (EEXIST):
            strcpy(errstr, "EEXIST");
            break;
        case (EXDEV):
            strcpy(errstr, "EXDEV");
            break;
        case (ENODEV):
            strcpy(errstr, "ENODEV");
            break;
        case (ENOTDIR):
            strcpy(errstr, "ENOTDIR");
            break;
        case (EISDIR):
            strcpy(errstr, "EISDIR");
            break;
        case (EINVAL):
            strcpy(errstr, "EINVAL");
            break;
        case (ENFILE):
            strcpy(errstr, "ENFILE");
            break;
        case (EMFILE):
            strcpy(errstr, "EMFILE");
            break;
        case (ENOTTY):
            strcpy(errstr, "ENOTTY");
            break;
        case (ETXTBSY):
            strcpy(errstr, "ETXTBSY");
            break;
        case (EFBIG):
            strcpy(errstr, "EFBIG");
            break;
        case (ENOSPC):
            strcpy(errstr, "ENOSPC");
            break;
        case (ESPIPE):
            strcpy(errstr, "ESPIPE");
            break;
        case (EROFS):
            strcpy(errstr, "EROFS");
            break;
        case (EMLINK):
            strcpy(errstr, "EMLINK");
            break;
        case (EPIPE):
            strcpy(errstr, "EPIPE");
            break;
        case (EDOM):
            strcpy(errstr, "EDOM");
            break;
        case (ERANGE):
            strcpy(errstr, "ERANGE");
            break;
        default:
            break;
    }
    return errstr;
}

int scanf(const char *format, ...)
{
    va_list val;
    char buf[MAX_WORD_LENGTH];
    va_start(val, format);
    int index = 0;
    while (*format) {
        if ('%' == *format && 's' == *(format + 1)) {
            char *line = va_arg(val, char *);
            int ch = 0;
            printf("");
            read(0, &ch, 1);
            while (' ' == ch || '\t' == ch) {
                read(0, &ch, 1);
            }
            index = 0;
            while (' ' != ch && '\t' != ch && '\n' != ch) {
                *(line + index++) = ch;
                read(0, &ch, 1);
            }
            printf("line=[%s]\n", line);
            format += 2;
        } else if ('%' == *format && 'x' == *(format + 1)) {
            memset(buf, 0, MAX_WORD_LENGTH);
            int ch = 0;
            int swap = 1;
            read(0, &ch, 1);
            while (' ' == ch || '\t' == ch) {
                read(0, &ch, 1);
            }
            if ('-' == ch) {
                swap = -1;
                read(0, &ch, 1);
            }
            index = 0;
            while (' ' != ch && '\t' != ch && '\n' != ch) {
                *(buf + index++) = ch;
                read(0, &ch, 1);
            }
            int *p_var = va_arg(val, int *);
            *p_var = xatoi(buf) * swap;
            format += 2;
        } else if ('%' == *format && 'd' == *(format + 1)) {
            memset(buf, 0, MAX_WORD_LENGTH);
            int ch = 0;
            int swap = 1;
            read(0, &ch, 1);
            while (' ' == ch || '\t' == ch) {
                read(0, &ch, 1);
            }
            if ('-' == ch) {
                swap = -1;
                read(0, &ch, 1);
            }
            index = 0;
            while (' ' != ch && '\t' != ch && '\n' != ch) {
                *(buf + index++) = ch;
                read(0, &ch, 1);
            }
            int *p_var = va_arg(val, int *);
            *p_var = atoi(buf) * swap;
            format += 2;
        } else if ('%' == *format && 'c' == *(format + 1)) {
            char *p_var = va_arg(val, char *);
            char ch = 0;
            read(0, &ch, 1);
            while (' ' == ch || '\t' == ch) {
                read(0, &ch, 1);
            }
            *p_var = ch;
            read(0, &ch, 1);
            while (' ' != ch && '\t' != ch && '\n' != ch)
                read(0, &ch, 1);
            format += 2;
        } else if ('%' == *format && 'v' == *(format + 1)) {
            char *line = va_arg(val, char *);
            memset(line, 0, MAX_LINE_LENGTH);
            int i = 0;
            for (i = 0; i < MAX_LINE_LENGTH; i++) {
                read(0, line + i, 1);
                if (*(line + i) == '\n') {
                    break;
                }
            }
            format += 2;
        } else if ('%' == *format && 'z' == *(format + 1)) {
            char *ln = va_arg(val, char *);
            char *line = ln;
            memset(line, 0, MAX_LINE_LENGTH);
            while(read(0, line, MAX_LINE_LENGTH)<0);
            format += 2;
        } else {
            format++;
        }
    }
    va_end(val);
    return 0;
}

int ps()
{
    int64_t ret_val = (int64_t) syscall_int_0((uint64_t) SYS_ps);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int kill(int32_t option, int32_t pid)
{
    int64_t ret_val =
        (int64_t) syscall_int_2((uint64_t) SYS_kill, (uint64_t) option,
                (uint64_t) pid);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

int sched_setparam(sched_param param, int64_t value)
{
    int64_t ret_val =
        (int64_t) syscall_int_2((uint64_t) SYS_sched_setparam,
                (uint64_t) param, (uint64_t) value);
    if (ret_val < 0) {
        errno = -1 * ret_val;
        return -1;
    }
    return (int) ret_val;
}

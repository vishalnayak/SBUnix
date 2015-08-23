#include <sys/io.h>
#include <sys/tarfs.h>
#include <sys/sys-handlers.h>
#include <sys/klibc.h>
#include <sys/pagetables.h>
#include <sys/keyboard.h>
#include <sys/process.h>
#include <sys/gdt.h>

extern struct tarfs_file_descriptors fs_fd[MAX_OPEN_FILES];
uint64_t insert_fstruct(struct file_struct *fstruct);

uint64_t syscall_write(int32_t fd, const void *buf, uint64_t count)
{
    uint64_t ret_val = count;
    if (curr_task->proc.fstructs[1]) {
        struct file_struct *fs =
            (struct file_struct *) curr_task->proc.fstructs[1];
        const char *buff = (const char *) buf;
        uint64_t cnt = count;
        while (cnt) {
            while (fs->pipe_free_count && cnt) {
                *fs->p_pipe_write++ = *buff++;
                fs->pipe_free_count--;
                cnt--;
                if (fs->p_pipe_write == fs->vnode + fs->size) {
                    fs->p_pipe_write = fs->vnode;
                }
            }
            if (cnt < count) {
                resume_read_pipe_proc(fs->vnode);
            }
            if (cnt) {
                __asm__ __volatile__("pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;pushq %%rbp;pushq %%rdi;" "pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;pushq %%r12;" "pushq %%r13;pushq %%r14;pushq %%r15;":::);
                __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.
                    kern_rsp)::);
                int cs = curr_task->proc.cs;
                int ds = curr_task->proc.ds;
                curr_task->proc.cs = KERN_CS;
                curr_task->proc.ds = KERN_DS;
                curr_task->proc.wait_for_pipe_read = fs->vnode;
                wait(SCHED_WAIT, curr_task->proc.kern_rsp);
                curr_task->proc.wait_for_pipe_read = 0;
                curr_task->proc.cs = cs;
                curr_task->proc.ds = ds;
                __asm__ __volatile__("popq %%r15;popq %%r14;popq %%r13;popq %%r12;popq %%r11;popq %%r10;" "popq %%r9;popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;popq %%rdx;" "popq %%rcx;popq %%rbx;popq %%rax;":::);
            }
        }
    } else {
        const char *b = (const char *) buf;
        while (count--) {
            printf("%c", *(char *) b++);
        }
    }
    return ret_val;
}

uint64_t syscall_read(int32_t fd, void *buf, uint64_t count)
{
    if (curr_task->proc.fstructs[0]) {
        struct file_struct *fs =
            (struct file_struct *) curr_task->proc.fstructs[0];
        char *buff = (char *) buf;
        uint64_t cnt = count;
        while (cnt) {
            while (fs->pipe_free_count < fs->size && cnt) {
                if (*fs->p_pipe_read == '\0') {
                    memset_kernel(fs->vnode, 0, fs->size);
                    fs->pipe_free_count = fs->size;
                    fs->p_pipe_write = fs->vnode;
                    fs->p_pipe_read = fs->vnode;
                    return -1;
                }
                *buff++ = *fs->p_pipe_read++;
                fs->pipe_free_count++;
                cnt--;
                if (fs->p_pipe_read == fs->vnode + fs->size) {
                    fs->p_pipe_read = fs->vnode;
                }
            }
            if (cnt < count) {
                resume_write_pipe_proc(fs->vnode);
            }
            if (cnt) {
                __asm__ __volatile__("pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;pushq %%rbp;pushq %%rdi;" "pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;pushq %%r12;" "pushq %%r13;pushq %%r14;pushq %%r15;":::);
                __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.
                    kern_rsp)::);
                curr_task->proc.cs = KERN_CS;
                curr_task->proc.ds = KERN_DS;
                curr_task->proc.wait_for_pipe_write = fs->vnode;
                wait(SCHED_WAIT, curr_task->proc.kern_rsp);
                curr_task->proc.wait_for_pipe_write = 0;
                curr_task->proc.cs = USER_CS;
                curr_task->proc.ds = USER_DS;
                __asm__ __volatile__("popq %%r15;popq %%r14;popq %%r13;popq %%r12;popq %%r11;popq %%r10;" "popq %%r9;popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;popq %%rdx;" "popq %%rcx;popq %%rbx;popq %%rax;":::);
            }
        }
        return count;
    } else {
        if (fd == 0) {
            if (!curr_task->proc.fg) {
                if(curr_task->proc.pid==1){
                    syscall_exit(1);
                    return -1;
                }
                printf
                    ("-sbush: pid %d killed: terminal read from a non-foreground process not allowed\n",
                     curr_task->proc.pid);
                syscall_exit(0);
            }
            int k = 0;
            char kb_char = 0;
            int cnt = count;
            for (; cnt;) {
                __asm__ __volatile__("pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;pushq %%rbp;pushq %%rdi;" "pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;pushq %%r12;" "pushq %%r13;pushq %%r14;pushq %%r15;":::);
                __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.
                    kern_rsp)::);
                curr_task->proc.cs = KERN_CS;
                curr_task->proc.ds = KERN_DS;
                curr_task->proc.wait_for_read = 1;
                wait(SCHED_WAIT, curr_task->proc.kern_rsp);
                curr_task->proc.wait_for_read = 0;
                curr_task->proc.cs = USER_CS;
                curr_task->proc.ds = USER_DS;
                __asm__ __volatile__("popq %%r15;popq %%r14;popq %%r13;popq %%r12;popq %%r11;popq %%r10;" "popq %%r9;popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;popq %%rdx;" "popq %%rcx;popq %%rbx;popq %%rax;":::);
                kb_char = read_key();
                while (kb_char != -1 && cnt) {
                    *((char *) buf + k) = kb_char;
                    if (kb_char == '\n')
                        return count - (count - cnt);
                    if (kb_char == '\0')
                        return -1;
                    if (kb_char == 8 && k == 0) {
                        *((char *) buf + k--) = 0;
                        cnt++;
                        if (cnt != count) {
                            video_mem_backspace();
                        }
                    } else if (kb_char == 8 && k > 0) {
                        *((char *) buf + k--) = 0;
                        *((char *) buf + k--) = 0;
                        cnt++;
                        cnt++;
                        video_mem_backspace();
                    }
                    cnt--;
                    k++;
                    kb_char = read_key();
                }
            }
            return count;
        } else {
            struct file_struct *fstruct =
                (struct file_struct *) curr_task->proc.fstructs[fd];
            if (fstruct->offset == fstruct->size) {
                return -1;
            }
            if (fstruct->offset + count > fstruct->size) {
                count = fstruct->size - fstruct->offset;
            }
            strncpy_kernel(buf, fstruct->vnode + fstruct->offset, count);
            fstruct->offset += count;
        }
    }
    return count;
}

uint64_t syscall_pipe(int32_t filedes[2])
{
    int idx = -1;
    struct file_struct *fstruct =
        (struct file_struct *) kmalloc(PAGE_SIZE);
    fstruct->offset = 0;
    fstruct->vnode = (char *) kmalloc(PAGE_SIZE);
    fstruct->size = PAGE_SIZE;
    fstruct->type = FS_PIPE;
    fstruct->pipe_free_count = PAGE_SIZE;
    fstruct->p_pipe_write = fstruct->vnode;
    fstruct->p_pipe_read = fstruct->vnode;
    if ((idx = insert_fstruct(fstruct)) < 0) {
        printf("Max Open File limit reached\n");
        return -1;
    }
    filedes[0] = idx;
    if ((idx = insert_fstruct(fstruct)) < 0) {
        printf("Max Open File limit reached\n");
        return -1;
    }
    filedes[1] = idx;
    return 0;
}

uint64_t syscall_getdents(uint32_t fd, struct dirent * dirp,
        uint32_t count)
{
    uint64_t ret_val = 0;
    if (ROOT_FD == fd) {
        fd = 0;
    }
    char buf[256] = { 0 };
    syscall_getcwd(buf, 256);
    memset_kernel(dirp->d_name, 0, NAME_MAX + 1);
    if (get_entry(fd, dirp->d_name, buf + 1) < 0) {
        dirp = NULL;
        fs_fd[fd].offset = 0;
        fs_fd[fd].fd_index = 0;
        strcpy_kernel(fs_fd[fd].match_source, "");
        return ret_val;
    }
    dirp->d_ino = 0;
    dirp->d_off += 1;
    dirp->d_reclen = 0;
    ret_val = 1;
    return ret_val;
}

uint64_t syscall_lseek(int32_t fildes, uint64_t offset, int32_t whence)
{
    if (ROOT_FD == fildes) {
        fildes = 0;
    }
    fs_fd[fildes].offset = offset;
    return 0;
}

uint64_t syscall_getcwd(char *buf, uint64_t size)
{
    memset_kernel(buf, 0, size);
    strcpy_kernel(buf, curr_task->proc.cwd);
    return 0;
}

int process_path(char *path)
{
    char buf[256] = { 0 };
    char *p = NULL;
    int plen = 0, len = 0, lc = 0, fc = 0, update = -1, idx = -1;
    syscall_getcwd(buf, 256);
start:
    plen = strlen_kernel(path);
    lc = *(path + plen - 1);
    fc = *path;
    if (lc != '/') {
        *(path + plen) = '/';
    }
    if (plen == 1 && *path == '/') {
        //"/"
        *(path + 1) = 0;
        p = path;
        update = 0;
    } else if ((plen == 1 || plen == 2) && !strcmp_kernel("./", path)) {
        //"." or "./"
        strcpy_kernel(path, buf);
        p = path;
        update = 0;
    } else if ((plen == 2 || plen == 3) && !strcmp_kernel("../", path)) {
        if (!strcmp_kernel("/", buf)) {
            //".." or "../" on root directory
            strcpy_kernel(path, buf);
            p = path;
            update = 0;
        } else {
            //".." or "../" on sub-directory
            strncpy_kernel(path, buf, penultimate_index_of('/', buf) + 1);
            p = path;
            update = 0;
        }
    } else if (!strcmp_kernel((const char *) &fc, "/")) {
        //"<Absolute path>"
        p = path;
    } else {
        char *pth = path;
        idx = index_of('.', pth);
        //printf("pth=%s path=%s idx=%d\n",pth,path,idx);
        //remove all instances of './' in the path
        int fnd = 0;
        while (idx != -1) {
            if (!idx) {
                if (*(pth + idx + 1) == '/') {
                    strcpy_kernel(pth + idx, pth + idx + 2);
                    fnd = 1;
                }
            } else {
                if (('/' == *(pth + idx + 1))
                        && ('.' != *(pth + idx - 1))) {
                    strcpy_kernel(pth + idx, pth + idx + 2);
                    fnd = 1;
                }
            }
            if (!fnd) {
                pth = pth + idx + 1;
                if (*pth == '.')
                    pth++;
            }
            fnd = 0;
            idx = index_of('.', pth);
            //printf("idx=%d pth=%s path=%s\n",idx,pth,path);
        }
        //normal case where pwd is prepended
        len = strlen_kernel(buf);
        strcpy_kernel(buf + len, path);
        strcpy_kernel(path, buf);
        //All the "./" are removed. Now, process "../" patterns in the absolute path
        pth = path;
        idx = index_of('.', pth);
        fnd = 0;
        //printf("idx=%d pth=%s path=%s\n",idx,pth,path);
        while (idx != -1) {
            if (idx == 1) {
                //remove only if '/../' pattern is found
                if ((*(pth + idx - 1) == '/')
                        && (*(pth + idx + 1) == '.')
                        && (*(pth + idx + 2) == '/')) {
                    strcpy_kernel(pth, pth + 3);
                    fnd = 1;
                }
            } else {
                //remove only if '/X/../' pattern is found
                if ((*(pth + idx - 1) == '/')
                        && (*(pth + idx + 1) == '.')
                        && (*(pth + idx + 2) == '/')) {
                    char t_pth[256] = { 0 };
                    strncpy_kernel(t_pth, pth, idx);
                    int t_idx = penultimate_index_of('/', t_pth);
                    strcpy_kernel(pth + t_idx, pth + idx + 2);
                    fnd = 1;
                }
            }
            if (!fnd) {
                pth = pth + idx + 1;
            }
            fnd = 0;
            idx = index_of('.', pth);
            //printf("idx=%d pth=%s path=%s\n",idx,pth,path);
        }
        //printf("processed path=[%s]\n",path);
        p = path;
        goto start;
    }
    strcpy_kernel(path, p);
    return update;
}

uint64_t syscall_chdir(const char *pathname)
{
    char path[256] = { 0 };
    char buf[256] = { 0 };
    int fd = 0, update = -1;
    syscall_getcwd(buf, 256);
    strcpy_kernel(path, pathname);
    update = process_path(path);
    if (-1 != (fd = open(path + 1, O_RDONLY))) {
        close(fd);
        update = 0;
    }
    if (!update) {
        memset_kernel(curr_task->proc.cwd, 0, 256);
        strcpy_kernel(curr_task->proc.cwd, path);
    }
    return update;
}

uint64_t syscall_open(const char *pathname, int32_t flags)
{
    if (flags == O_WRONLY || flags == O_RDWR) {
        return -EACCES;
    }
    int idx = -1;
    char path[256] = { 0 };
    int len = 0;
    int fd = -1, fc = 0, lc = 0;
    char buf[256] = { 0 };
    syscall_getcwd(buf, 256);
    strcpy_kernel(path, pathname);
    process_path(path);
    if (curr_task->proc.fstructs[0]) {
        return 0;
    }
    if (strlen_kernel(path) == 1 && *path == '/') {
        return 0;
    }
    fd = open(path + 1, flags);
    if (-1 == fd) {
        len = strlen_kernel(path);
        lc = *(path + len - 1);
        if (lc != '/') {
            *(path + len) = '/';
        } else {
            *(path + len - 1) = 0;
        }
        fd = open(path + 1, flags);
    }
    if (fd != -1) {
        struct file_struct *fstruct =
            (struct file_struct *) kmalloc(PAGE_SIZE);
        fstruct->fd = fd;
        fstruct->offset = 0;
        fstruct->vnode = get_start_addr_from_descriptor(fstruct->fd);
        fstruct->size = get_size_from_descriptor(fstruct->fd);
        if ((idx = insert_fstruct(fstruct)) < 0) {
            printf("Max Open File limit reached\n");
        }
    } else if (fc == '/') {
        idx = ROOT_FD;
    }
    return idx;
}

uint64_t insert_fstruct(struct file_struct * fstruct)
{
    int found = 0;
    int32_t idx = 11;
    for (; curr_task->proc.fstructs[idx] && idx < MAX_OPEN_FILES; idx++);
    if (idx < MAX_OPEN_FILES) {
        curr_task->proc.fstructs[idx] = (uint64_t) fstruct;
        fstruct->refcount++;
        found = 1;
    }
    return found ? idx : -1;
}

uint64_t syscall_close(int32_t fd)
{
    struct file_struct *fstruct =
        (struct file_struct *) curr_task->proc.fstructs[fd];
    if (!fstruct->refcount--) {
        syscall_munmap(fstruct, PAGE_SIZE);
        deref_page(kern_to_page_desc((uint64_t *) fstruct->vnode));
    }
    curr_task->proc.fstructs[fd] = 0;
    return 0;
}

uint64_t syscall_dup(int32_t oldfd)
{
    int idx = -1;
    if (oldfd >= 0 && oldfd < MAX_OPEN_FILES) {
        struct file_struct *fstruct =
            (struct file_struct *) curr_task->proc.fstructs[oldfd];
        if (fstruct) {
            idx = insert_fstruct(fstruct);
        }
    }
    return idx;
}

uint64_t syscall_dup2(int32_t oldfd, int32_t newfd)
{
    int idx = -1;
    if ((oldfd >= 0 && oldfd < MAX_OPEN_FILES)
            && (newfd >= 0 && newfd < MAX_OPEN_FILES)) {
        struct file_struct *fstruct =
            (struct file_struct *) curr_task->proc.fstructs[oldfd];
        if (fstruct) {
            curr_task->proc.fstructs[newfd] = (uint64_t) fstruct;
            idx = newfd;
        }
    }
    return idx;
}

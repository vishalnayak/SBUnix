#ifndef __IO_H
#define __IO_H

#include<sys/sbunix.h>

#define ROOT_FD 2
#define FS_FILE 1
#define FS_DIRECTORY 2
#define FS_PIPE 3

struct file_struct {
    uint32_t fd;
    uint64_t offset;
    uint64_t size;
    uint64_t type;
    uint64_t refcount;
    uint64_t pipe_free_count;
    char *p_pipe_write;
    char *p_pipe_read;
    char *vnode;
};

#endif

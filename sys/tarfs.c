#include <sys/tarfs.h>
#include <sys/sbunix.h>
#include <sys/klibc.h>

void print_tarfs_entry(struct posix_header_ustar *ustar);
int get_index(const char *name);
void print_file(char *name);
void write_to_file(char *filename, char *data);

struct tarfs_file_descriptors fs_fd[MAX_OPEN_FILES];

int prefix_match(const char *str1, const char *str2, int *idx)
{
    int index = -1;
    int len = strlen_kernel(str1);
    if (!len) {
        *idx = 0;
        return 1;
    }
    while (*str1 == *str2 && index + 1 < len) {
        index++;
        if (*str1 == 0 || *str2 == 0) {
            return index;
        }
        str1++;
        str2++;
    }
    if (len == index + 1 && *str1 && *str2 && *str1 != *str2) {
        index = -1;
        *idx = len;
    } else {
        *idx = index + 1;
    }
    return index == -1 ? 0 : len == index + 1;
}

int get_entry(int fd, char *name, char *prefix)
{
    int ret_val = -1;
    if (fd > -1 && fd < MAX_OPEN_FILES) {
        int off = fs_fd[fd].offset;
        strcpy_kernel(fs_fd[fd].match_source, "");
        struct posix_header_ustar *iter =
            (struct posix_header_ustar *) &_binary_tarfs_start;
        uint64_t fd_index = 1;
        int count = 0;
        while (strlen_kernel(iter->name)) {
            int idx = -1;
            if (prefix_match(prefix, iter->name, &idx)) {
                if (idx < strlen_kernel(iter->name)) {
                    int nameidx = index_of('/', (iter->name + idx));
                    if (-1 == nameidx) {
                        if (count == off) {
                            if (fd_index >= fs_fd[fd].fd_index) {
                                strcpy_kernel(name, iter->name + idx);
                                strcpy_kernel(fs_fd[fd].match_source, "");
                                fs_fd[fd].fd_index = 0;
                                ret_val = 0;
                                break;
                            }
                        } else {
                            count++;
                        }
                    } else {
                        char subname[20];
                        strncpy_kernel(subname, iter->name, nameidx);
                        if (strcmp_kernel(fs_fd[fd].match_source, subname)) {
                            if (fd_index >= fs_fd[fd].fd_index) {
                                if (count == off) {
                                    strncpy_kernel
                                        (name, iter->name + idx, nameidx + 1);
                                    strcpy_kernel
                                        (fs_fd[fd].match_source, "");
                                    fs_fd[fd].fd_index = 0;
                                    ret_val = 0;
                                    break;
                                } else {
                                    count++;
                                    strcpy_kernel
                                        (fs_fd[fd].match_source, subname);
                                    fs_fd[fd].fd_index = fd_index;
                                }
                            }
                        }
                    }
                }
            }
            uint64_t size = otoi_kernel(atoi_kernel(iter->size));
            if (size) {
                size =
                    (uint64_t) align_up((void *) size,
                            sizeof(struct posix_header_ustar));
                iter = (struct posix_header_ustar *) ((uint64_t)
                        iter + size);
            }
            iter++;
            fd_index++;
        }
    }
    return ret_val;
}

void *get_start_addr_from_name(char *program)
{
    return get_start_addr_from_descriptor(open(program, O_RDONLY));
}

uint64_t get_size_from_descriptor(int fd)
{
    if (fd > -1 && fd < MAX_OPEN_FILES) {
        return fs_fd[fd].size;
    }
    return -1;
}

void *get_start_addr_from_descriptor(int fd)
{
    if (fd > -1 && fd < MAX_OPEN_FILES) {
        return fs_fd[fd].data;
    }
    return NULL;
}

void init_root_dir()
{
    *fs_fd[0].name = '/';
    fs_fd[0].size = 0;
    fs_fd[0].offset = 0;
    fs_fd[0].data = 0;
    fs_fd[0].type = 5;
}

void tarfs_init()
{
    struct posix_header_ustar *iter =
        (struct posix_header_ustar *) &_binary_tarfs_start;
    uint64_t fd_index = 1;
    init_root_dir();
    while (strlen_kernel(iter->name)) {
        uint64_t size = otoi_kernel(atoi_kernel(iter->size));
        //printf("name=%s size=%p\n", iter->name, size);
        strcpy_kernel(fs_fd[fd_index].name, iter->name);
        fs_fd[fd_index].size = size;
        fs_fd[fd_index].offset = 0;
        fs_fd[fd_index].data = 0;
        fs_fd[fd_index].type = 5;
        if (size) {
            fs_fd[fd_index].type = 0;
            fs_fd[fd_index].data =
                (void *) ((uint64_t) iter +
                        sizeof(struct posix_header_ustar));
            size =
                (uint64_t) align_up((void *) size,
                        sizeof(struct posix_header_ustar));
            iter = (struct posix_header_ustar *) ((uint64_t) iter + size);
        }
        iter++;
        fd_index++;
    }
}

int open(const char *pathname, int flags)
{
    return get_index(pathname);
}

uint64_t read(int fd, void *buf, uint64_t count)
{
    char *out = (char *) buf;
    char *start = (char *) fs_fd[fd].data;
    uint64_t size = fs_fd[fd].size;
    uint64_t offset = fs_fd[fd].offset;
    uint64_t bytes;
    if (size - offset < count) {
        count = size - offset;
    }
    bytes = count;
    while (count--) {
        *out++ = (*(start + offset))++;
    }
    *out = 0;
    fs_fd[fd].offset += bytes;
    return bytes;
}

void print_file(char *name)
{
    int fd = open(name, O_RDONLY);
    char ch[10];
    uint64_t bytes = 0;
    while (read(fd, ch, 1)) {
        bytes++;
        printf("%c", ch[0]);
        memset_kernel(ch, 0, 10);
    }
    printf("bytes=%d\n", bytes);
    close(fd);
}

int close(int fd)
{
    fs_fd[fd].offset = 0;
    return 0;
}

int get_index(const char *name)
{
    struct posix_header_ustar *iter =
        (struct posix_header_ustar *) &_binary_tarfs_start;
    int index = 0;
    while (strlen_kernel(iter->name)) {
        index++;
        if (!strcmp_kernel(name, iter->name)) {
            return index;
        }
        uint64_t size = otoi_kernel(atoi_kernel(iter->size));
        iter++;
        if (size) {
            size =
                (uint64_t) align_up((void *) size,
                        sizeof(struct posix_header_ustar));
            iter = (struct posix_header_ustar *) ((uint64_t) iter + size);
        }
    }
    return -1;
}

void print_tarfs_entry(struct posix_header_ustar *ustar)
{
    printf("name %s\n", ustar->name);
    printf("size %s\n", ustar->size);
    printf("%p \n", atoi_kernel(ustar->size));
}

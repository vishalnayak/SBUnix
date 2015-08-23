#ifndef _TARFS_H
#define _TARFS_H
#include <sys/sbunix.h>
extern char _binary_tarfs_start;
extern char _binary_tarfs_end;
extern char _binary_tarfs_size;

struct tarfs_file_descriptors {
    char name[100];
    char match_source[20];
    int type;
    uint64_t size;
    uint64_t fd_index;
    uint64_t offset;
    void *data;
};

struct posix_header_ustar {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

void tarfs_init();
void *get_start_addr_from_name(char *program);
void *get_start_addr_from_descriptor(int fd);
uint64_t get_size_from_descriptor(int fd);

enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT =
    0x40, O_DIRECTORY = 0x10000
};
int open(const char *pathname, int flags);
uint64_t read(int fd, void *buf, uint64_t count);
uint64_t write(int fd, const void *buf, uint64_t count);
int close(int fd);
int opendir(const char *name);
int readdir(int fd);
int closedir(int fd);
int get_entry(int fd, char *name, char *prefix);

#endif

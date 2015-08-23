#ifndef _STRING_H
#define _STRING_H

#include <stdlib.h>
#include <errno-base.h>
#include <mman-common.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORD_LENGTH 256
#define MAX_ARGS 20

typedef struct vm_struct {
    size_t size;
    struct vm_struct *next;
} vm_struct_t;
char *strcpy(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);
size_t strlen(const char *str);
size_t strcspn(const char *str1, const char *str2);
char *strrchr(const char *str1, int c);
char *strncpy(char *dest, const char *src, size_t n);
void *memset(void *dest, int c, size_t count);
char *itoa(int value, char *str, int base);
int atoi(const char *str);
int xatoi(const char *str);
char *error();
int clear();
int ps();
int kill(int32_t option, int32_t pid);
typedef enum { BG_ENABLE } sched_param;
int sched_setparam(sched_param param, int64_t value);
/*
   SYSCALLS
 */
int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);
typedef int64_t time_t;
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif				/*  */

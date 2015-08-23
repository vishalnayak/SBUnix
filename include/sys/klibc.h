#ifndef __KLIBC_H
#define __KLIBC_H

#include<stdarg.h>
#include<sys/sbunix.h>
#include<errno-base.h>

#define MAX_OPEN_FILES 512

__thread int errno;
unsigned char screen_color;

uint64_t *kmalloc(uint64_t n);
uint64_t video_mem_write(const void *buf, uint64_t count);
void video_mem_backspace();
uint64_t strlen_kernel(const char *str);
int strcmp_kernel(const char *str1, const char *str2);
int strncmp_kernel(const char *str1, const char *str2, int n);
char *strcpy_kernel(char *dest, const char *src);
char *strncpy_kernel(char *dest, const char *src, uint64_t n);
char *itoa_kernel(uint64_t value, char *str, int base);
uint64_t atoi_kernel(const char *str);
uint64_t otoi_kernel(uint64_t octal);
void *memset_kernel(void *dest, int c, uint64_t count);
void *memcpy_kernel(void *dest, const void *src, uint64_t n);
uint64_t malloc_kernel(uint64_t n);

void *align_up(void *p_val, uint64_t size);
void *align_down(void *p_val, uint64_t size);

unsigned char inb(int port);
void outb(int port, unsigned char data);
void load_cr3(uint64_t cr3);
void load_rsp(uint64_t rsp);
void save_rsp(uint64_t rsp);
void load_es(uint64_t val);
void load_ds(uint64_t val);
void load_gs(uint64_t val);
void load_fs(uint64_t val);
uint64_t read_cr3();
int penultimate_index_of(int ch, char *str);
int last_index_of(int ch, char *str);
int index_of(int ch, char *str);


#endif

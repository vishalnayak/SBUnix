#ifndef __MEMORY_H
#define __MEMORY_H

#include<sys/sbunix.h>

#define KERNEL_BASE   0xffffffff80000000
#define USER_STACK    0x00000000e0000000
#define ANON_MEM_BASE 0x00000000c0000000

struct mm_struct {
    uint64_t *pml4;
    struct vm_area_struct *mmap;
    struct process *owner;
    uint64_t anon_mem_brk;
};

struct vm_area_struct {
    struct mm_struct *vm_mm;
    uint64_t vm_start;
    uint64_t vm_end;
    uint64_t vm_flags;
    uint64_t *vm_inode;
    struct vm_area_struct *vm_next;
};

struct vm_area_struct *add_vma(struct mm_struct *mm, uint64_t start,
        uint64_t size);
int32_t remove_vma(struct mm_struct *mm, uint64_t start, uint64_t size);

#endif

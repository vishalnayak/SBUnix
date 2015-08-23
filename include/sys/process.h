#ifndef __PROCESS_H
#define __PROCESS_H

#include<sys/sbunix.h>
#include<sys/memory.h>
#include<sys/idt.h>

#define DPL_3 0x3
#define USER_CS (uint64_t)(0x18 | DPL_3)
#define USER_DS (uint64_t)(0x20 | DPL_3)
#define KERN_CS (uint64_t)(0x8)
#define KERN_DS (uint64_t)(0x10)
#define TIMESLICE 2

#define EFLAGS 0x200242

struct sched_param {
    int32_t bg_enable;
};

struct syscall_frame {
    uint64_t num;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t rip;
    uint64_t eflags;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t fs;
    uint64_t gs;
    uint64_t ds;
    uint64_t es;
};

struct process {
    char name[30];
    uint64_t pid;
    uint64_t ds;
    uint64_t cs;
    uint64_t ppid;
    uint64_t *pml4;
    uint64_t cr3;
    uint64_t fg;
    struct process *parent_fg;
    uint64_t state;
    uint64_t eflags;
    uint64_t *fstructs;
    uint64_t *stack;
    uint64_t *kern_stack;
    struct mm_struct *mm;
    uint64_t kern_rsp;
    uint64_t rip;
    uint64_t readrip;
    uint64_t forkrip;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t started;
    uint64_t timeslice;
    uint64_t wait_for_read;
    uint64_t wait_for_pid;
    uint64_t wait_for_sleep;
    char *wait_for_pipe_write;
    char *wait_for_pipe_read;
    char cwd[256];
};

struct process_queue {
    struct process proc;
    struct process_queue *next;
};

struct process_queue *curr_task;
void process_init();
typedef enum { SCHED_YIELD, SCHED_DELETE, SCHED_WAIT } sched_type;
void schedule(sched_type type, uint64_t rsp);
void wait(sched_type type, uint64_t rsp);
void resume_read_proc();
void resume_wait_proc(uint64_t pid);
void resume_write_pipe_proc(char *pipe_addr);
void resume_read_pipe_proc(char *pipe_addr);
extern struct process_queue *pqueue;
extern struct process_queue *wqueue;
extern struct sched_param scheduler_param;
extern struct process *fg_proc;
extern struct process *fg_parent_proc;

#endif

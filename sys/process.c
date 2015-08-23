#include<sys/process.h>
#include<sys/io.h>
#include<sys/tarfs.h>
#include<sys/elf.h>
#include<sys/klibc.h>
#include<sys/memory.h>
#include<mman-common.h>
#include<sys/sys-handlers.h>
#include<sys/pagetables.h>
#include<sys/gdt.h>
#include<sys/klibc.h>

extern uint64_t *p_ker_pml4e;
extern uint64_t phys_pml4e;
extern struct tss_t tss;
extern __volatile__ char *video_mem_start;
extern __volatile__ char *video_mem;

struct sched_param scheduler_param;
struct process_queue *pqueue;
struct process_queue *wqueue;
struct process_queue *zqueue;
struct process_queue *prev_task;
struct process_queue *idle_task;
struct process *fg_proc = NULL;
static uint64_t proc_id = -1;

void handle_switch(struct stack_frame *p_sf);
void kern_proc_init(struct process *proc, uint64_t func);
uint64_t user_process_init(struct process *proc, char *name);
void first_switch();
void cleanup_task(struct process_queue *task);
void subsequent_switch();
void idle_proc_func();
void kern_proc_func();
void print_queues();
void add_to_queue(struct process_queue **queue,
        struct process_queue *task);
void remove_from_queue(struct process_queue **queue,
        struct process_queue *task);
void update_foreground(struct process *new, struct process *parent);

uint64_t syscall_sched_setparam(sched_param param, uint64_t value)
{
    switch (param) {
        case BG_ENABLE:
            scheduler_param.bg_enable = value;
            break;
        default:
            break;
    }
    return 0;
}

uint64_t syscall_kill(int32_t option, int32_t pid)
{
    if (pid == 0 || pid == 1) {
        printf("-sbush: kill(%d): Not allowed\n", pid);
        return -1;
    }
    struct process_queue *iter = pqueue;
    struct process_queue *item = NULL;
    while (iter) {
        if (iter->proc.pid == pid) {
            item = iter;
            break;
        }
        iter = iter->next;
    }
    if (item) {
        remove_from_queue(&pqueue, item);
        resume_wait_proc(item->proc.pid);
        if (curr_task->proc.parent_fg == &item->proc) {
            curr_task->proc.parent_fg = item->proc.parent_fg;
        }
        cleanup_task(item);
        return 0;
    }
    iter = wqueue;
    while (iter) {
        if (iter->proc.pid == pid) {
            item = iter;
            break;
        }
        iter = iter->next;
    }
    if (item) {
        remove_from_queue(&wqueue, item);
        resume_wait_proc(item->proc.pid);
        if (curr_task->proc.parent_fg == &item->proc) {
            curr_task->proc.parent_fg = item->proc.parent_fg;
        }
        cleanup_task(item);
        return 0;
    }
    printf("-sbush: kill(%d): No such process\n", pid);
    return 0;
}
void ps_print(uint64_t pid,char*pname){
    char str[10]= {0};
    itoa_kernel(pid,str,10);
    syscall_write(1,"PID:[",strlen_kernel("PID:["));
    syscall_write(1,str,strlen_kernel(str));
    syscall_write(1,"] TTY:pts/1 Process:[",strlen_kernel("] TTY:pts/1 Process:["));
    syscall_write(1,pname,strlen_kernel(pname));
    syscall_write(1,"]\n",2);
}
uint64_t syscall_ps()
{
    ps_print(curr_task->proc.pid,curr_task->proc.name);
    //printf("PID:[%d] Process:[%s]\n", curr_task->proc.pid,
    //        curr_task->proc.name);
    struct process_queue *iter = pqueue;
    while (iter) {
        //printf("PID:[%d] Process:[%s]\n", iter->proc.pid,
        //        iter->proc.name);
        ps_print(iter->proc.pid,iter->proc.name);
        iter = iter->next;
    }
    iter = wqueue;
    while (iter) {
        //printf("PID:[%d] Process:[%s]\n", iter->proc.pid,
        //        iter->proc.name);
        ps_print(iter->proc.pid,iter->proc.name);
        iter = iter->next;
    }
    //printf("PID:[%d] Process:[%s]\n", idle_task->proc.pid,
    //        idle_task->proc.name);
    ps_print(idle_task->proc.pid,idle_task->proc.name);
    return 0;
}

uint64_t syscall_wait4(uint32_t pid, int32_t * status, int32_t options)
{
    struct process_queue *zombieq = zqueue;
    while (zombieq) {
        if (zombieq->proc.pid == pid) {
            return pid;
        }
        zombieq = zombieq->next;
    }
    if (scheduler_param.bg_enable) {
        return pid;
    }
    __asm__ __volatile__("pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;pushq %%rbp;pushq %%rdi;" "pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;pushq %%r12;pushq %%r13;" "pushq %%r14;" "pushq %%r15;":::);
    __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.kern_rsp)::);
    curr_task->proc.cs = KERN_CS;
    curr_task->proc.ds = KERN_DS;
    curr_task->proc.wait_for_pid = pid;
    wait(SCHED_WAIT, curr_task->proc.kern_rsp);
    curr_task->proc.wait_for_pid = 0;
    curr_task->proc.cs = USER_CS;
    curr_task->proc.ds = USER_DS;
    __asm__ __volatile__("popq %%r15;popq %%r14;popq %%r13;popq %%r12;popq %%r11;popq %%r10;popq %%r9;" "popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;popq %%rdx;popq %%rcx;popq %%rbx;popq %%rax;":::);
    return pid;
}

uint64_t syscall_exit(int32_t status)
{
    if (curr_task->proc.fstructs[1] && curr_task->proc.pid != -1) {
        syscall_write(1, "\0", 1);
    }
    if (curr_task->proc.pid == 1) {
        if(status == 1){
            update_foreground(&curr_task->proc,&curr_task->proc);
            return 0;
        }
        printf("SBUSH shutting down...\nConsole will be active!\n");
    }
    resume_wait_proc(curr_task->proc.pid);
    add_to_queue(&zqueue, curr_task);
    if (curr_task->proc.fg) {
        update_foreground(curr_task->proc.parent_fg,
                curr_task->proc.parent_fg->parent_fg);
    }
    schedule(SCHED_DELETE, 0);
    return 0;
}

uint64_t syscall_alarm(uint32_t seconds)
{
    printf("syscall_alarm received\n");
    return 0;
}

uint64_t syscall_nanosleep(const struct timespec * req,
        struct timespec * rem)
{
    __asm__ __volatile__("pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;pushq %%rbp;pushq %%rdi;" "pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;pushq %%r12;pushq %%r13;" "pushq %%r14;pushq %%r15;":::);
    __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.kern_rsp)::);
    curr_task->proc.cs = KERN_CS;
    curr_task->proc.ds = KERN_DS;
    curr_task->proc.wait_for_sleep = req->tv_sec * 100 / TIMESLICE;
    wait(SCHED_YIELD, curr_task->proc.kern_rsp);
    curr_task->proc.wait_for_sleep = 0;
    curr_task->proc.cs = USER_CS;
    curr_task->proc.ds = USER_DS;
    __asm__ __volatile__("popq %%r15;popq %%r14;popq %%r13;popq %%r12;popq %%r11;popq %%r10;" "popq %%r9;popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;popq %%rdx;popq %%rcx;popq %%rbx;" "popq %%rax;":::);
    rem->tv_sec = 0;
    return 0;
}

uint64_t syscall_getppid()
{
    uint64_t ret_val = -1;
    if (curr_task) {
        ret_val = curr_task->proc.ppid;
    }
    return ret_val;
}

uint64_t syscall_getpid()
{
    uint64_t ret_val = -1;
    if (curr_task) {
        ret_val = curr_task->proc.pid;
    }
    return ret_val;
}

void wait(sched_type type, uint64_t rsp)
{
    if (curr_task->proc.rip == 0x4000e8) {
        __asm__ __volatile__("movq %%rsp, %%r15;"
                "addq $24,%%r15;"
                "movq (%%r15),%[rip];":[rip]
                "=r"(curr_task->proc.readrip)
                ::"memory");
    } else {
        __asm__ __volatile__("movq %%rsp, %%r15;"
                "addq $8,%%r15;"
                "movq (%%r15),%[rip];":[rip]
                "=r"(curr_task->proc.readrip)
                ::"memory");
    }
    __asm__
        __volatile__
        ("pushq %[ds];pushq %[stack];pushq %[eflags];pushq %[cs];pushq %[rip];"::
         [ds] "r"((uint64_t) 0x10)
         ,[stack] "r"((uint64_t) (curr_task->proc.kern_rsp))
         ,[eflags] "r"((uint64_t) 0x200046)
         ,[cs] "r"((uint64_t) 0x8)
         ,[rip] "r"(curr_task->proc.readrip)
         :	 "memory");
    __asm__ __volatile__("pushq %%r15;pushq %%r15;pushq %%rax;pushq %%rbx;pushq %%rcx;pushq %%rdx;" "pushq %%rbp;pushq %%rdi;pushq %%rsi;pushq %%r8;pushq %%r9;pushq %%r10;pushq %%r11;" "pushq %%r12;pushq %%r13;pushq %%r14;pushq %%r15;movq %%es,%%r15;pushq %%r15;movq %%ds,%%r15;" "pushq %%r15;movq %%gs,%%r15;pushq %%r15;movq %%fs,%%r15;pushq %%r15;":::"memory");
    __asm__ __volatile__("movq %%rsp,%0":"=r"(curr_task->proc.kern_rsp)::);

    schedule(type, curr_task->proc.kern_rsp);
}

void remove_from_queue(struct process_queue **queue,
        struct process_queue *task)
{
    struct process_queue *q = *queue, *prev = NULL;
    while (q) {
        if (q == task) {
            if (!prev) {
                *queue = (*queue)->next;
            } else {
                prev->next = q->next;
                q->next = NULL;
            }
            break;
        }
        prev = q;
        q = q->next;
    }
}

void add_to_queue(struct process_queue **queue, struct process_queue *task)
{
    struct process_queue *q = *queue, *prev = NULL;
    int found = 0;
    task->next = NULL;
    if (!*queue) {
        *queue = task;
    } else {
        while (q) {
            if (q == task) {
                found = 1;
                break;
            }
            prev = q;
            q = q->next;
        }
        if (!found) {
            prev->next = task;
        }
    }
}

void resume_wait_proc(uint64_t pid)
{
    struct process_queue *pq = pqueue;
    struct process_queue *wq = wqueue;
    struct process_queue *wq_prev = NULL;
    struct process_queue *lpq = NULL;
    while (pq) {
        lpq = pq;
        pq = pq->next;
    }
    while (wq) {
        if (wq->proc.wait_for_pid == pid) {
            if (!wq_prev) {
                wqueue = wqueue->next;
            } else {
                wq_prev->next = wq->next;
            }
            wq->next = NULL;
            if (!lpq) {
                pqueue = wq;
            } else {
                lpq->next = wq;
            }
            break;
        }
        wq_prev = wq;
        wq = wq->next;
    }
}

void resume_write_pipe_proc(char *pipe_addr)
{
    struct process_queue *pq = pqueue;
    struct process_queue *wq = wqueue;
    struct process_queue *wq_prev = NULL;
    struct process_queue *lpq = NULL;
    while (pq) {
        lpq = pq;
        pq = pq->next;
    }
    while (wq) {
        if (wq->proc.wait_for_pipe_read == pipe_addr) {
            if (!wq_prev) {
                wqueue = wqueue->next;
            } else {
                wq_prev->next = wq->next;
            }
            wq->next = NULL;
            if (!lpq) {
                pqueue = wq;
            } else {
                lpq->next = wq;
            }
            break;
        }
        wq_prev = wq;
        wq = wq->next;
    }
}

void resume_read_pipe_proc(char *pipe_addr)
{
    struct process_queue *pq = pqueue;
    struct process_queue *wq = wqueue;
    struct process_queue *wq_prev = NULL;
    struct process_queue *lpq = NULL;
    while (pq) {
        lpq = pq;
        pq = pq->next;
    }
    while (wq) {
        if (wq->proc.wait_for_pipe_write == pipe_addr) {
            if (!wq_prev) {
                wqueue = wqueue->next;
            } else {
                wq_prev->next = wq->next;
            }
            wq->next = NULL;
            if (!lpq) {
                pqueue = wq;
            } else {
                lpq->next = wq;
            }
            break;
        }
        wq_prev = wq;
        wq = wq->next;
    }
}

void resume_read_proc()
{
    struct process_queue *pq = pqueue;
    struct process_queue *wq = wqueue;
    struct process_queue *wq_prev = NULL;
    struct process_queue *lpq = NULL;
    while (pq) {
        lpq = pq;
        pq = pq->next;
    }
    while (wq) {
        if (wq->proc.wait_for_read) {
            if (!wq_prev) {
                wqueue = wqueue->next;
            } else {
                wq_prev->next = wq->next;
            }
            wq->next = NULL;
            if (!lpq) {
                pqueue = wq;
            } else {
                lpq->next = wq;
            }
            break;
        }
        wq_prev = wq;
        wq = wq->next;
    }
}

void idle_proc_func()
{
    while (1);
}

void kern_proc_init(struct process *proc, uint64_t func)
{
    strcpy_kernel(proc->name, "idle");
    proc->pid = ++proc_id;
    proc->timeslice = TIMESLICE;
    proc->cs = KERN_CS;
    proc->ds = KERN_DS;
    strcpy_kernel(proc->cwd, "/");
    proc->pid = 0;
    proc->eflags = EFLAGS;

    proc->cr3 = phys_pml4e;

    proc->pml4 = p_ker_pml4e;

    proc->kern_stack = &((uint64_t *) kmalloc(PAGE_SIZE))[511];

    proc->stack = &((uint64_t *) kmalloc(PAGE_SIZE))[511];
    proc->stack -= 8;

    proc->mm = (struct mm_struct *) kmalloc(PAGE_SIZE);
    proc->mm->pml4 = proc->pml4;
    proc->mm->owner = proc;
    proc->mm->anon_mem_brk = ANON_MEM_BASE;

    *(proc->kern_stack - 0) = proc->ds;
    *(proc->kern_stack - 1) = (uint64_t) proc->stack;
    *(proc->kern_stack - 2) = proc->eflags;
    *(proc->kern_stack - 3) = proc->cs;
    *(proc->kern_stack - 4) = proc->rip;
    proc->kern_rsp = (uint64_t) (proc->kern_stack - 5);

    proc->rip = func;
}

void schedule(sched_type type, uint64_t rsp)
{
    //    printf("[s]");
    curr_task->proc.kern_rsp = rsp;
    if (curr_task == idle_task && !pqueue) {
        return;
    }
    prev_task = curr_task;
    prev_task->next = NULL;
    prev_task->proc.state = 0;

    if (prev_task != idle_task) {
        if (type != SCHED_DELETE) {
            if (type == SCHED_WAIT) {
                add_to_queue(&wqueue, prev_task);
            } else if (type == SCHED_YIELD) {
                add_to_queue(&pqueue, prev_task);
            }
        } else {
            //printf("Deleting process [%d]\n",prev_task->proc.pid);
        }
    }

    if (!pqueue) {
        curr_task = idle_task;
        curr_task->proc.state = 0;
        tss.rsp0 = (uint64_t) curr_task->proc.kern_stack;
        first_switch();
    } else {
        curr_task = NULL;
        struct process_queue *p = pqueue;
        struct process_queue *prev = NULL;
        while (p) {
            if (!p->proc.wait_for_sleep) {
                if (!prev) {
                    pqueue = pqueue->next;
                } else {
                    prev->next = p->next;
                }
                curr_task = p;
                break;
            } else {
                p->proc.wait_for_sleep--;
            }
            prev = p;
            p = p->next;
        }
        if (!curr_task) {
            curr_task = idle_task;
            curr_task->proc.state = 0;
            tss.rsp0 = (uint64_t) curr_task->proc.kern_stack;
            if (type == SCHED_DELETE) {
                cleanup_task(prev_task);
            }
            first_switch();
        }

        curr_task->proc.state = 0;
        tss.rsp0 = (uint64_t) curr_task->proc.kern_stack;
        if (prev_task->proc.pid != curr_task->proc.pid) {
            //printf("[%d,%d] \n", prev_task->proc.pid,curr_task->proc.pid);
            if (curr_task->proc.started == 0) {
                curr_task->proc.started = 1;
                if (type == SCHED_DELETE) {
                    cleanup_task(prev_task);
                }
                first_switch();
            } else {
                if (type == SCHED_DELETE) {
                    cleanup_task(prev_task);
                }
                subsequent_switch();
            }
        }
    }
}

void cleanup_task(struct process_queue *task)
{
    //if(task && task->proc.pid != -1 && !task->proc.fstructs[0] && !task->proc.fstructs[1]){
    if (task && task->proc.pid != -1) {
        struct vm_area_struct *vma = task->proc.mm->mmap;
        while (vma) {
            remove_vma(task->proc.mm, vma->vm_start,
                    vma->vm_end - vma->vm_start);
            vma = vma->vm_next;
        }
        deref_page(kern_to_page_desc((uint64_t *) task->proc.kern_stack));
        deref_page(kern_to_page_desc((uint64_t *) task->proc.mm));
        deref_page(kern_to_page_desc((uint64_t *) task->proc.fstructs));
        deref_page(kern_to_page_desc((uint64_t *) task->proc.stack));
        deref_page(kern_to_page_desc((uint64_t *) task->proc.pml4));
    }
}

void first_switch()
{
    curr_task->proc.state = 1;
    tss.rsp0 = (uint64_t) curr_task->proc.kern_stack;
    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var]
            "r"(curr_task->proc.cr3));
    load_fs(curr_task->proc.ds);
    load_gs(curr_task->proc.ds);
    load_ds(curr_task->proc.ds);
    load_es(curr_task->proc.ds);
    __asm__ __volatile__("movq %[rsp_var], %%rsp;"::[rsp_var]
            "r"(curr_task->proc.kern_rsp):"memory");
    __asm__ __volatile__("pushq %[ds];"
            "pushq %[stack];"
            "pushq %[eflags];"
            "pushq %[cs];"
            "pushq %[rip];"
            "iretq;"::[ds] "r"(curr_task->proc.ds)
            ,[stack] "r"((uint64_t) curr_task->proc.stack)
            ,[eflags] "r"(curr_task->proc.eflags)
            ,[cs] "r"(curr_task->proc.cs)
            ,[rip] "r"(curr_task->proc.rip)
            :"memory");
}

void subsequent_switch()
{
    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var]
            "r"(curr_task->proc.cr3));
    __asm__ __volatile__("movq %0,%%rsp;"::
            "r"(curr_task->proc.kern_rsp):"memory");
    __asm__ __volatile__("popq %%r15;movq %%r15,%%fs;popq %%r15;movq %%r15,%%gs;popq %%r15;" "movq %%r15,%%ds;popq %%r15;movq %%r15,%%es;popq %%r15;popq %%r14;popq %%r13;" "popq %%r12;popq %%r11;popq %%r10;popq %%r9;popq %%r8;popq %%rsi;popq %%rdi;popq %%rbp;" "popq %%rdx;popq %%rcx;popq %%rbx;popq %%rax;popq %%r15;popq %%r15;pushq %%rax;":::);
    __asm__
        __volatile__
        ("movq %%rsp,%%r15;addq $16,%%r15;movq %[cs],(%%r15);addq $24,%%r15;"
         "movq %[ds],(%%r15);popq %%rax;iretq;"::[cs] "r"(curr_task->
        proc.cs),
         [ds] "r"(curr_task->proc.ds)
         :	 "memory");
}

uint64_t user_process_init(struct process *proc, char *name)
{
    int li = last_index_of('/', name);
    if (-1 == li) {
        strcpy_kernel(proc->name, name);
    } else {
        strncpy_kernel(proc->name, name + li + 1,
                strlen_kernel(name) - li - 1);
    }
    proc->pid = ++proc_id;
    proc->timeslice = TIMESLICE;
    proc->cs = USER_CS;
    proc->ds = USER_DS;
    proc->eflags = EFLAGS;
    strcpy_kernel(proc->cwd, "/");
    proc->pml4 = kmalloc(PAGE_SIZE);
    proc->cr3 = (uint64_t) kern_to_phys_addr(proc->pml4);
    proc->fstructs = kmalloc(PAGE_SIZE);

    proc->pml4[pml4_offset((uint64_t *) KERNEL_BASE)] =
        p_ker_pml4e[pml4_offset((uint64_t *) KERNEL_BASE)];

    proc->kern_stack = &(kmalloc(PAGE_SIZE))[511];

    load_cr3(proc->cr3);

    proc->mm = (struct mm_struct *) kmalloc(PAGE_SIZE);
    proc->mm->pml4 = proc->pml4;
    proc->mm->owner = proc;
    proc->mm->anon_mem_brk = ANON_MEM_BASE;

    proc->stack = (uint64_t *) USER_STACK - 1;
    map_address(proc->pml4, ((uint64_t *) (USER_STACK - PAGE_SIZE)),
            PAGE_SIZE);
    add_vma(proc->mm, USER_STACK - 256 * PAGE_SIZE, 255 * PAGE_SIZE);
    proc->stack -= 8;		//to accommodate argc,argv,envp of user programs.

    load_cr3(phys_pml4e);

    struct elf64_header *p_program =
        (struct elf64_header *) get_start_addr_from_name(name);
    if (!p_program) {
        return -ENOENT;
    }
    struct elf64_program_header *p_header =
        (struct elf64_program_header *) ((char *) p_program +
                p_program->phoff);
    struct elf64_program_header *last_program_header =
        p_header + p_program->phnum;
    uint64_t count = 0;
    while (p_header < last_program_header) {
        if (1 == p_header->type) {
            count++;
            map_address(proc->pml4, (uint64_t *) p_header->vaddr,
                    p_header->memsz);
            load_cr3(proc->cr3);
            memcpy_kernel((void *) p_header->vaddr,
                    (char *) p_program + p_header->offset,
                    p_header->filesz);
            memset_kernel((void *) p_header->vaddr +
                    p_header->filesz, 0,
                    p_header->memsz - p_header->filesz);
            load_cr3(phys_pml4e);
            add_vma(proc->mm, p_header->vaddr, p_header->memsz);
        }
        p_header++;
    }
    proc->rip = p_program->entry;

    load_cr3(proc->cr3);

    *(proc->kern_stack - 0) = proc->ds;
    *(proc->kern_stack - 1) = (uint64_t) proc->stack;
    *(proc->kern_stack - 2) = proc->eflags;
    *(proc->kern_stack - 3) = proc->cs;
    *(proc->kern_stack - 4) = proc->rip;
    proc->kern_rsp = (uint64_t) (proc->kern_stack - 5);

    load_cr3(phys_pml4e);
    return 0;
}

uint64_t fork_process_init()
{
    struct process_queue *fork_task =
        (struct process_queue *) kmalloc(PAGE_SIZE);
    struct process *proc = &fork_task->proc;
    strcpy_kernel(proc->name, curr_task->proc.name);
    proc->pid = ++proc_id;
    update_foreground(proc, &curr_task->proc);

    proc->started = 1;

    proc->timeslice = curr_task->proc.timeslice / 2;
    curr_task->proc.timeslice /= 2;

    proc->ppid = curr_task->proc.pid;
    strcpy_kernel(proc->cwd, curr_task->proc.cwd);
    proc->rip = curr_task->proc.rip;
    proc->cs = curr_task->proc.cs;
    proc->ds = curr_task->proc.ds;
    proc->eflags = curr_task->proc.eflags;
    proc->pml4 = kmalloc(PAGE_SIZE);
    proc->pml4[511] = curr_task->proc.pml4[511];
    proc->cr3 = (uint64_t) kern_to_phys_addr(proc->pml4);
    proc->fstructs = kmalloc(PAGE_SIZE);
    proc->stack = curr_task->proc.stack;
    proc->kern_stack = &((uint64_t *) kmalloc(PAGE_SIZE))[511];

    uint64_t *parent_stack = kmalloc(PAGE_SIZE);
    uint64_t i = 0;
    for (i = 0; i < 512; i++) {
        parent_stack[i] =
            (uint64_t) * ((uint64_t *) (USER_STACK - PAGE_SIZE + i * 8));
    }

    proc->mm = (struct mm_struct *) kmalloc(PAGE_SIZE);
    proc->mm->pml4 = proc->pml4;
    proc->mm->owner = proc;
    proc->mm->anon_mem_brk = ANON_MEM_BASE;

    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var]
            "r"(proc->cr3));

    add_page(proc->pml4, kern_to_page_desc(parent_stack),
            (uint64_t *) (USER_STACK - PAGE_SIZE), PTE_P | PTE_W | PTE_U);

    for (i = 0; i < 512; i++) {
        *(proc->kern_stack - i) = *(curr_task->proc.kern_stack - i);
        *(proc->fstructs + i) = *(curr_task->proc.fstructs + i);
        if (*(curr_task->proc.fstructs + i)) {
            ((struct file_struct
              *) (*(curr_task->proc.fstructs + i)))->refcount++;
        }
    }

    *(proc->kern_stack - 1) = (uint64_t) proc->stack;
    *(proc->kern_stack - 9) = 0;	//rax
    proc->kern_rsp = (uint64_t) (proc->kern_stack - 27);

    struct vm_area_struct *parent_vma = curr_task->proc.mm->mmap;
    while (parent_vma) {
        add_vma(proc->mm, parent_vma->vm_start,
                (parent_vma->vm_end - parent_vma->vm_start));
        parent_vma = parent_vma->vm_next;
    }

    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var]
            "r"(curr_task->proc.cr3));

    parent_vma = curr_task->proc.mm->mmap;
    while (parent_vma) {
        mark_for_cow(curr_task->proc.pml4, proc->pml4,
                (uint64_t *) parent_vma->vm_start,
                (parent_vma->vm_end - parent_vma->vm_start));
        parent_vma = parent_vma->vm_next;
    }

    add_to_queue(&pqueue, fork_task);

    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var]
            "r"(curr_task->proc.cr3));
    return proc->pid;
}

uint64_t syscall_fork()
{
    return fork_process_init();
}

void update_foreground(struct process *new, struct process *parent)
{
    if (parent) {
        new->parent_fg = parent;
    }
    if (!scheduler_param.bg_enable) {
        /*
           printf("curr foreground %d ",fg_proc->pid);
           printf("new foreground %d ",new->pid);
           printf("parent %d \n",parent);
         */
        fg_proc->fg = 0;
        new->fg = 1;
        fg_proc = new;
    }
}

void process_init()
{
    idle_task = (struct process_queue *) kmalloc(PAGE_SIZE);
    kern_proc_init(&idle_task->proc, (uint64_t) idle_proc_func);
    struct process_queue *user_task1 =
        (struct process_queue *) kmalloc(PAGE_SIZE);
    user_process_init(&user_task1->proc, "bin/sbush");
    //user_process_init(&user_task1->proc, "bin/user1");
    fg_proc = &user_task1->proc;
    user_task1->proc.fg = 1;
    user_task1->proc.parent_fg = &user_task1->proc;
    struct process_queue *user_task2 =
        (struct process_queue *) kmalloc(PAGE_SIZE);
    user_process_init(&user_task2->proc, "bin/user2");

    user_task1->next = user_task2;
    pqueue = user_task1;

    curr_task = idle_task;
    prev_task = curr_task;
    first_switch();
}

//really, really, really messy. But, it works like a charm :)
uint64_t syscall_execve(const char *filename, char *const argv[],
        char *const envp[])
{
    int i = 0;
    //    printf("syscall_execve received %s\n",filename);
    struct process_queue *exec_task =
        (struct process_queue *) kmalloc(PAGE_SIZE);
    struct process *proc = &exec_task->proc;
    int argc = 0;
    int envc = 0;
    char **av = (char **) kmalloc(PAGE_SIZE);;
    char **ep = (char **) kmalloc(PAGE_SIZE);;
    for (argc = 0; argv[argc]; argc++) {
        av[argc] = (char *) kmalloc(strlen_kernel(argv[argc]));
        strcpy_kernel(av[argc], argv[argc]);
    }
    for (envc = 0; envp[envc]; envc++) {
        ep[envc] = (char *) kmalloc(strlen_kernel(envp[envc]));
        strcpy_kernel(ep[envc], envp[envc]);
    }
    char fname[100];
    strcpy_kernel(fname, &filename[1]);
    if (-ENOENT == user_process_init(proc, fname)) {
        load_cr3(curr_task->proc.cr3);
        return -ENOENT;
    }
    load_cr3(proc->cr3);
    for (i = 0; i < 512; i++) {
        *(proc->fstructs + i) = *(curr_task->proc.fstructs + i);
        if (*(curr_task->proc.fstructs + i)) {
            ((struct file_struct
              *) (*(curr_task->proc.fstructs + i)))->refcount++;
        }
    }
    load_cr3(curr_task->proc.cr3);
    update_foreground(proc, curr_task->proc.parent_fg);
    char buf[256] = { 0 };
    syscall_getcwd(buf, 256);
    strcpy_kernel(proc->cwd, buf);
    struct process_queue *curr = curr_task;
    curr_task = exec_task;
    load_cr3(proc->cr3);
    uint64_t *exec_stack = proc->stack + 8;
    *exec_stack-- = 0;
    for (i = envc - 1; i >= 0; i--) {
        char *s = (char *) syscall_mmap(NULL, strlen_kernel(ep[i]),
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1,
                0);
        map_address(curr_task->proc.pml4, (uint64_t *) s, PAGE_SIZE);
        strcpy_kernel(s, ep[i]);
        *exec_stack-- = (uint64_t) s;
    }
    *exec_stack-- = 0;
    for (i = argc - 1; i >= 0; i--) {
        char *s = (char *) syscall_mmap(NULL, strlen_kernel(av[i]),
                PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1,
                0);
        map_address(curr_task->proc.pml4, (uint64_t *) s, PAGE_SIZE);
        strcpy_kernel(s, av[i]);
        *exec_stack-- = (uint64_t) s;
    }
    *exec_stack = argc;
    proc->stack = exec_stack;
    load_cr3(curr_task->proc.cr3);
    curr_task = curr;
    proc->pid = curr_task->proc.pid;
    proc->ppid = curr_task->proc.ppid;
    curr_task->proc.pid = -1;
    add_to_queue(&pqueue, exec_task);
    syscall_exit(0);
    return -1;
}

#include <sys/handlers.h>
#include <sys/sbunix.h>
#include <sys/sys-handlers.h>
#include <sys/syscall.h>
#include <sys/klibc.h>
#include <sys/process.h>
#include <errno-base.h>
#include <sys/pagetables.h>

void isr_syscall_handler(struct stack_frame *p_sf)
{
    struct syscall_frame sysf;
    struct syscall_frame *p_sysf = &sysf;
    uint64_t ret_val = -1;
    sysf.num = p_sf->rax;
    sysf.a1 = p_sf->rdi;
    sysf.a2 = p_sf->rsi;
    sysf.a3 = p_sf->rdx;
    sysf.a4 = p_sf->r10;
    sysf.a5 = p_sf->r8;
    sysf.a6 = p_sf->r9;
    sysf.rbx = p_sf->rbx;
    sysf.rbp = p_sf->rbp;
    sysf.r12 = p_sf->r12;
    sysf.r13 = p_sf->r13;
    sysf.r14 = p_sf->r14;
    sysf.r15 = p_sf->r15;
    sysf.fs = p_sf->fs;
    sysf.gs = p_sf->gs;
    sysf.ds = p_sf->ds;
    sysf.es = p_sf->es;

    switch (p_sf->rax) {
        case SYS_sched_setparam:
            ret_val =
                syscall_sched_setparam((sched_param) p_sysf->a1,
                        (int64_t) p_sysf->a2);
            break;
        case SYS_kill:
            ret_val = syscall_kill((int32_t) p_sysf->a1, (int32_t) p_sysf->a2);
            break;
        case SYS_dup:
            ret_val = syscall_dup((int32_t) p_sysf->a1);
            break;
        case SYS_dup2:
            ret_val = syscall_dup2((int32_t) p_sysf->a1, (int32_t) p_sysf->a2);
            break;
        case SYS_ps:
            ret_val = syscall_ps();
            break;
        case SYS_getcwd:
            ret_val =
                syscall_getcwd((char *) p_sysf->a1, (uint64_t) p_sysf->a2);
            break;
        case SYS_alarm:
            ret_val = syscall_alarm((uint32_t) p_sysf->a1);
            break;
        case SYS_nanosleep:
            ret_val =
                syscall_nanosleep((const struct timespec *) p_sysf->a1,
                        (struct timespec *) p_sysf->a2);
            break;
        case SYS_wait4:
            ret_val =
                syscall_wait4((uint32_t) p_sysf->a1, (int32_t *) p_sysf->a2,
                        (int32_t) p_sysf->a3);
            break;
        case SYS_execve:
            ret_val =
                syscall_execve((const char *) p_sysf->a1,
                        (char *const *) p_sysf->a2,
                        (char *const *) p_sysf->a3);
            break;
        case SYS_getpid:
            ret_val = syscall_getpid();
            break;
        case SYS_getppid:
            ret_val = syscall_getppid();
            break;
        case SYS_munmap:
            ret_val =
                syscall_munmap((void *) p_sysf->a1, (uint64_t) p_sysf->a2);
            break;
        case SYS_exit:
            ret_val = syscall_exit((int32_t) p_sysf->a1);
            break;
        case SYS_fork:
            ret_val = syscall_fork();
            p_sf->rax = ret_val;
            break;
        case SYS_mmap:
            ret_val =
                syscall_mmap((void *) p_sysf->a1, (uint64_t) p_sysf->a2,
                        (int32_t) p_sysf->a3, (int32_t) p_sysf->a4,
                        (int32_t) p_sysf->a5, (uint64_t) p_sysf->a6);
            break;
        default:
            printf("int syscall unrecognized: num=%d\n", p_sf->rax);
    }
    p_sf->rax = ret_val;
}

void isr_err_page_fault_handler(struct stack_frame *p_sf)
{
    uint64_t faddr;
    __asm __volatile("movq %%cr2,%0":"=r"(faddr));
    struct vm_area_struct *vma = curr_task->proc.mm->mmap;
    int count = 0;
    int found = 0;
    while (vma) {
        count++;
        if (faddr >= vma->vm_start && faddr < vma->vm_end) {
            found = 1;
            break;
        }
        vma = vma->vm_next;
    }
    if (found) {
        uint64_t *p_pt_entry = NULL;
        struct page_descriptor *page =
            lookup_page(curr_task->proc.pml4, (uint64_t *) faddr,
                    &p_pt_entry);
        //check for copy on write
        if ((*p_pt_entry & PTE_P) && !(*p_pt_entry & PTE_W)) {
            if (page->page_ref > 1) {
                struct page_descriptor *newpage = alloc_page(1);
                newpage->page_ref++;
                memcpy_kernel(page_desc_to_kern_addr(newpage),
                        page_desc_to_kern_addr(page), PAGE_SIZE);
                add_page(curr_task->proc.pml4, newpage,
                        (uint64_t *) faddr, PTE_P | PTE_W | PTE_U);
            } else {
                *p_pt_entry |= PTE_W;
            }
        } else if (!(*p_pt_entry & PTE_P)) {
            map_address(curr_task->proc.pml4, (uint64_t *) faddr,
                    PAGE_SIZE);
        }
        load_cr3(curr_task->proc.cr3);
    } else {
        syscall_exit(0);
    }
}

void isr_err_general_protection_fault_handler(struct stack_frame *p_sf)
{
    syscall_exit(0);
    return;
}

void isr_divide_by_zero_handler(struct stack_frame *p_sf)
{
    printf("isr_divide_by_zero_handler\n");
}

void isr_debug_handler(struct stack_frame *p_sf)
{
    printf("isr_debug_handler\n");
}

void isr_nmi_handler(struct stack_frame *p_sf)
{
    printf("isr_nmi_handler\n");
}

void isr_breakpoint_handler(struct stack_frame *p_sf)
{
    printf("isr_breakpoint_handler\n");
}

void isr_overflow_handler(struct stack_frame *p_sf)
{
    printf("isr_overflow_handler\n");
}

void isr_out_of_bounds_handler(struct stack_frame *p_sf)
{
    printf("isr_out_of_bounds_handler\n");
}

void isr_invalid_opcode_handler(struct stack_frame *p_sf)
{
    printf("isr_invalid_opcode_handler\n");
    print_stack_frame(p_sf);
}

void isr_no_coprocessor_handler(struct stack_frame *p_sf)
{
    printf("isr_no_coprocessor_handler\n");
}

void isr_err_double_fault_handler(struct stack_frame *p_sf)
{
    printf("isr_err_double_fault_handler\n");
}

void idr_coprocessor_segment_overrun_handler(struct stack_frame *p_sf)
{
    printf("idr_coprocessor_segment_overrun_handler\n");
}

void isr_err_invalid_tss_handler(struct stack_frame *p_sf)
{
    printf("isr_err_invalid_tss_handler\n");
}

void isr_err_segment_not_present_handler(struct stack_frame *p_sf)
{
    printf("isr_err_segment_not_present_handler\n");
}

void isr_err_stack_fault_handler(struct stack_frame *p_sf)
{
    printf("isr_err_stack_fault_handler\n");
}

void isr_unknown_interrupt_handler(struct stack_frame *p_sf)
{
    printf("isr_unknown_interrupt_handler\n");
}

void isr_coprocessor_fault_handler(struct stack_frame *p_sf)
{
    printf("isr_coprocessor_fault_handler\n");
}

void isr_alignment_check_handler(struct stack_frame *p_sf)
{
    printf("isr_alignment_check_handler\n");
}

void isr_machine_check_handler(struct stack_frame *p_sf)
{
    printf("isr_machine_check_handler\n");
}

void isr_simd_floating_point_handler(struct stack_frame *p_sf)
{
    printf("isr_simd_floating_point_handler\n");
}

void isr_default_handler(struct stack_frame *p_sf)
{
    printf("isr_default_handler\n");
}

#include<sys/memory.h>
#include<mman-common.h>
#include<sys/pagetables.h>
#include<sys/klibc.h>
#include<sys/process.h>

int32_t remove_vma(struct mm_struct *mm, uint64_t start, uint64_t size)
{
    int32_t ret_val = -1;
    size = (uint64_t) align_up((uint64_t *) size, PAGE_SIZE);
    struct vm_area_struct *vma_node = mm->mmap, *vma_prev = NULL;
    while (vma_node) {
        if (start == vma_node->vm_start) {
            if (vma_prev) {
                vma_prev->vm_next = vma_node->vm_next;
            } else {
                mm->mmap = mm->mmap->vm_next;
            }
            if (start >= ANON_MEM_BASE && start < KERNEL_BASE) {
                //printf("Deleting: Start=[%p]--End[%p]\n",start,start+size);
                unmap_address(curr_task->proc.pml4,
                        (uint64_t *) start, size);
                deref_page(kern_to_page_desc((uint64_t *) vma_node));
            }
            ret_val = 0;
            break;
        }
        vma_prev = vma_node;
        vma_node = vma_node->vm_next;
    }
    return ret_val;
}

struct vm_area_struct *add_vma(struct mm_struct *mm, uint64_t start,
        uint64_t size)
{
    size = (uint64_t) align_up((uint64_t *) size, PAGE_SIZE);
    uint64_t num = size / PAGE_SIZE;
    uint64_t end = start + num * PAGE_SIZE;
    struct vm_area_struct *vma_node = mm->mmap, *vma_prev = NULL, *vma =
        NULL;
    while (vma_node) {
        if (start >= vma_node->vm_start
                && (start + size - 1) < vma_node->vm_end) {
            vma = vma_node;
            break;
        }
        vma_prev = vma_node;
        vma_node = vma_node->vm_next;
    }
    if (!vma) {
        vma = (struct vm_area_struct *) kmalloc(PAGE_SIZE);
        vma->vm_start = start;
        vma->vm_end = end;
    }
    if (!mm->mmap) {
        mm->mmap = vma;
    } else {
        vma_prev->vm_next = vma;
    }
    return vma;
}

uint64_t syscall_brk(void *end_data_segment)
{
    printf("syscall_brk received\n");
    return 0;
}

uint64_t syscall_munmap(void *start, uint64_t length)
{
    remove_vma(curr_task->proc.mm, (uint64_t) start, length);
    return 0;
}

uint64_t syscall_mmap(void *start, uint64_t length, int32_t prot,
        int32_t flags, int32_t fd, uint64_t offset)
{
    uint64_t anon_brk = curr_task->proc.mm->anon_mem_brk;
    if (!start && 0 == offset && MAP_ANONYMOUS | MAP_PRIVATE) {
        struct vm_area_struct *vma =
            add_vma(curr_task->proc.mm, anon_brk, length);
        curr_task->proc.mm->anon_mem_brk = vma->vm_end;
    }
    return anon_brk;
}

#include <sys/klibc.h>
#include <sys/pagetables.h>
#include <sys/sbunix.h>
#include <sys/klibc.h>
#include <sys/memory.h>

extern __volatile__ char *video_mem;
extern __volatile__ char *video_mem_start;
uint64_t *p_ker_pml4e;
uint64_t phys_pml4e;

static uint64_t *free_start;
struct page_descriptor *p_page_desc;
struct page_descriptor *p_free_page_list;
uint64_t num_pages;

void mark_for_cow(uint64_t * parent_pml4, uint64_t * child_pml4,
        uint64_t * virt_addr, uint64_t size)
{
    virt_addr = align_down(virt_addr, PAGE_SIZE);
    uint64_t num =
        ((uint64_t) align_up((uint64_t *) size, PAGE_SIZE)) / PAGE_SIZE;
    uint64_t i = 0;
    for (i = 0; i < num; i++) {
        remove_write_perm(parent_pml4, child_pml4,
                (uint64_t *) ((uint64_t) virt_addr +
                    i * PAGE_SIZE));
    }
}

void remove_write_perm(uint64_t * parent_pml4, uint64_t * child_pml4,
        uint64_t * virt_addr)
{
    uint64_t *p_pt_entry = NULL;
    struct page_descriptor *page =
        lookup_page(parent_pml4, virt_addr, &p_pt_entry);
    if (*p_pt_entry & PTE_P) {
        *p_pt_entry &= ~PTE_W;
        uint64_t *p_pt_entry_child =
            walk_pml4(child_pml4, virt_addr, PTE_P | PTE_W | PTE_U, 0);
        *p_pt_entry_child = *p_pt_entry;
        page->page_ref++;
    }
}

void unmap_address(uint64_t * pml4, uint64_t * virt_addr, uint64_t size)
{
    virt_addr = align_down(virt_addr, PAGE_SIZE);
    uint64_t num =
        ((uint64_t) align_up((uint64_t *) size, PAGE_SIZE)) / PAGE_SIZE;
    uint64_t i = 0;
    for (i = 0; i < num; i++) {
        remove_page(pml4,
                (uint64_t *) ((uint64_t) virt_addr + i * PAGE_SIZE));
    }
}

void map_address(uint64_t * pml4, uint64_t * virt_addr, uint64_t size)
{
    virt_addr = align_down(virt_addr, PAGE_SIZE);
    uint64_t num =
        ((uint64_t) align_up((uint64_t *) size, PAGE_SIZE)) / PAGE_SIZE;
    uint64_t i = 0;
    for (i = 0; i < num; i++) {
        struct page_descriptor *page = alloc_page(1);
        page->page_ref++;
        add_page(pml4, page,
                (uint64_t *) ((uint64_t) virt_addr + i * PAGE_SIZE),
                PTE_P | PTE_W | PTE_U);
    }
}

void add_page(uint64_t * pml4, struct page_descriptor *page,
        uint64_t * virt_addr, int perm)
{
    uint64_t *p_pt_entry = walk_pml4(pml4, virt_addr, perm, 0);
    if (*p_pt_entry & PTE_P) {
        remove_page(pml4, virt_addr);
    }
    *p_pt_entry = perm | (uint64_t)
        page_num_to_phys_addr(page_desc_to_page_num(page));
}

void remove_page(uint64_t * pml4, uint64_t * virt_addr)
{
    uint64_t *p_pt_entry = NULL;
    struct page_descriptor *page =
        lookup_page(pml4, virt_addr, &p_pt_entry);
    if (page) {
        deref_page(page);
        *p_pt_entry = 0;
        __asm __volatile("invlpg (%[virt_addr])"::[virt_addr]
                "r"(virt_addr):"memory");
    }
}

uint64_t pml4_offset(uint64_t * virt_addr)
{
    return ((uint64_t) virt_addr >> 39) & 0x1FF;
}

uint64_t pdp_offset(uint64_t * virt_addr)
{
    return ((uint64_t) virt_addr >> 30) & 0x1FF;
}

uint64_t pd_offset(uint64_t * virt_addr)
{
    return ((uint64_t) virt_addr >> 21) & 0x1FF;
}

uint64_t pt_offset(uint64_t * virt_addr)
{
    return ((uint64_t) virt_addr >> 12) & 0x1FF;
}

uint64_t p_offset(uint64_t * virt_addr)
{
    return (uint64_t) virt_addr & 0xFFF;
}

uint64_t *phys_addr_mask(uint64_t * phys_addr)
{
    return (uint64_t *) ((uint64_t) phys_addr & ~0xFFF);
}

void deref_page(struct page_descriptor *page)
{
    if (0 == --page->page_ref) {
        page->next_page = p_free_page_list;
        p_free_page_list = page;
    }
}

struct page_descriptor *alloc_page(int reset)
{
    if (!p_free_page_list) {
        return NULL;
    }
    struct page_descriptor *page = p_free_page_list;
    p_free_page_list = p_free_page_list->next_page;
    page->next_page = NULL;
    if (reset) {
        memset_kernel(page_desc_to_kern_addr(page), 0, PAGE_SIZE);
    }
    return page;
}

uint64_t *page_desc_to_phys_addr(struct page_descriptor * page)
{
    return page_num_to_phys_addr(page_desc_to_page_num(page));
}

uint64_t *page_desc_to_kern_addr(struct page_descriptor * page)
{
    return
        phys_to_kern_addr(page_num_to_phys_addr
                (page_desc_to_page_num(page)));
}

uint64_t phys_to_page_num(uint64_t * phys_addr)
{
    uint64_t addr = (uint64_t) phys_addr;
    return addr >> 12;
}

uint64_t page_desc_to_page_num(struct page_descriptor * p_desc)
{
    return p_desc - p_page_desc;
}

uint64_t *page_num_to_phys_addr(uint64_t page_num)
{
    return (uint64_t *) (page_num << 12);
}

struct page_descriptor *kern_to_page_desc(uint64_t * p_kern_addr)
{
    return
        page_num_to_page_desc(phys_to_page_num
                (kern_to_phys_addr(p_kern_addr)));
}

uint64_t *kern_to_phys_addr(uint64_t * p_kern_addr)
{
    uint64_t addr = (uint64_t) p_kern_addr;
    return (uint64_t *) (addr - KERNEL_BASE);
}

uint64_t *phys_to_kern_addr(uint64_t * p_phys_addr)
{
    uint64_t addr = (uint64_t) p_phys_addr;
    return (uint64_t *) (addr + KERNEL_BASE);
}

void *align_down(void *p_val, uint64_t size)
{
    char *p_char_val = (char *) p_val;
    p_char_val = p_char_val - ((uint64_t) p_char_val % size);
    return p_char_val;
}

void *align_up(void *p_val, uint64_t size)
{
    char *p_char_val = (char *) p_val;
    p_char_val =
        p_char_val - 1 - (((uint64_t) p_char_val - 1) % size) + size;
    return p_char_val;
}

void page_desc_init()
{
    p_page_desc = alloc_bytes(num_pages * sizeof(struct page_descriptor));
    memset_kernel((void *) p_page_desc, 0,
            num_pages * sizeof(struct page_descriptor));
    uint64_t i = 0;
    p_free_page_list = NULL;
    for (i = 0; i < num_pages; i++) {
        if (page_num_to_phys_addr(i) >= p_physbase
                && page_num_to_phys_addr(i) <=
                kern_to_phys_addr(alloc_bytes(0))) {
            p_page_desc[i].page_ref++;
        } else if ((uint64_t) page_num_to_phys_addr ==
                (uint64_t) video_mem_start) {
            p_page_desc[i].page_ref++;
        } else {
            p_page_desc[i].next_page = p_free_page_list;
            p_free_page_list = &p_page_desc[i];
        }
    }
}

void *alloc_bytes(uint32_t bytes)
{
    static char *p_free;
    char *ret_mem = NULL;
    if (!p_free) {
        p_free = align_up((void *) free_start, PAGE_SIZE);
    }
    if (bytes == 0) {
        ret_mem = p_free;
    } else if (bytes > 0) {
        ret_mem = p_free;
        p_free += bytes;
        p_free = align_up(p_free, PAGE_SIZE);
    }
    return ret_mem;
}

void paging_init()
{
    num_pages = max_physmem / PAGE_SIZE;
    free_start = phys_to_kern_addr(p_physfree);
    p_ker_pml4e = alloc_bytes(PAGE_SIZE);
    phys_pml4e = (uint64_t) kern_to_phys_addr(p_ker_pml4e);

    page_desc_init();
    //280KB
    //printf("mapping = %x\n", (uint64_t)p_physfree - (uint64_t)p_physbase);
    //map(p_ker_pml4e, (uint64_t)phys_to_kern_addr(p_physbase), (uint64_t)p_physfree-(uint64_t)p_physbase, (uint64_t)p_physbase, PTE_P | PTE_W);
    //map(p_ker_pml4e, KERNEL_BASE + 0xB8000, PAGE_SIZE, 0xB8000, PTE_P | PTE_W);

    map(p_ker_pml4e, KERNEL_BASE, PAGE_SIZE * num_pages, 0, PTE_P | PTE_U | PTE_W);	//remove PTE_U after testing

    video_mem += KERNEL_BASE;
    video_mem_start += KERNEL_BASE;
    load_cr3(phys_pml4e);
}

struct page_descriptor *page_num_to_page_desc(uint64_t page_num)
{
    return p_page_desc + page_num;
}

struct page_descriptor *lookup_page(uint64_t * p_table,
        uint64_t * virt_addr,
        uint64_t ** pp_pt_entry)
{
    uint64_t *p_pt_entry = walk_pml4(p_table, virt_addr, 0, 1);
    kern_to_phys_addr(p_pt_entry);
    struct page_descriptor *page =
        page_num_to_page_desc(phys_to_page_num
                (phys_addr_mask((uint64_t *) * p_pt_entry)));
    *pp_pt_entry = p_pt_entry;
    return page;
}

void map(uint64_t * p_pml4e_addr, uint64_t virt_addr, uint64_t n,
        uint64_t phys_addr, int perm)
{
    virt_addr = (uint64_t) align_up((void *) virt_addr, PAGE_SIZE);
    n = (uint64_t) align_up((void *) n, PAGE_SIZE);
    uint64_t i = 0;
    uint64_t count = 0;
    for (i = 0; i < n; i += PAGE_SIZE) {
        count++;
        *(walk_pml4
                (p_pml4e_addr, (uint64_t *) (virt_addr + i), perm, 0)) =
            (uint64_t) phys_addr_mask((uint64_t *) (phys_addr + i)) | perm;
    }
}

uint64_t *walk_pml4(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup)
{
    struct page_descriptor *page = NULL;
    uint64_t *p_pt_entry = NULL;
    uint64_t *p_table_entry = p_table + pml4_offset(p_virt_addr);
    if (((uint64_t) * p_table_entry & PTE_P) == 0 && !lookup) {
        page = alloc_page(1);
        if (!page) {
            return NULL;
        }
        page->page_ref++;
        *p_table_entry =
            (uint64_t) page_desc_to_phys_addr(page) | perm | PTE_U;
        p_pt_entry =
            walk_pdp(phys_to_kern_addr
                    (phys_addr_mask((uint64_t *) * p_table_entry)),
                    p_virt_addr, perm, lookup);
        if (!p_pt_entry) {
            deref_page(page);
            *p_table_entry = 0;
            return NULL;
        } else {
            return p_pt_entry;
        }
    } else {
        return
            walk_pdp(phys_to_kern_addr
                    (phys_addr_mask((uint64_t *) * p_table_entry)),
                    p_virt_addr, perm, lookup);
    }
    return NULL;
}

uint64_t *walk_pdp(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup)
{
    struct page_descriptor *page = NULL;
    uint64_t *p_pt_entry = NULL;
    uint64_t *p_table_entry = p_table + pdp_offset(p_virt_addr);
    if (((uint64_t) * p_table_entry & PTE_P) == 0 && !lookup) {
        page = alloc_page(1);
        if (!page) {
            return NULL;
        }
        page->page_ref++;
        *p_table_entry =
            (uint64_t) page_desc_to_phys_addr(page) | perm | PTE_U;
        p_pt_entry =
            walk_pdir(phys_to_kern_addr
                    (phys_addr_mask((uint64_t *) * p_table_entry)),
                    p_virt_addr, perm, lookup);
        if (!p_pt_entry) {
            deref_page(page);
            *p_table_entry = 0;
            return NULL;
        } else {
            return p_pt_entry;
        }
    } else {
        return
            walk_pdir(phys_to_kern_addr
                    (phys_addr_mask((uint64_t *) * p_table_entry)),
                    p_virt_addr, perm, lookup);
    }
    return NULL;
}

uint64_t *walk_pdir(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup)
{
    struct page_descriptor *page = NULL;
    uint64_t *p_pt_entry = NULL;
    uint64_t *p_table_entry = p_table + pd_offset(p_virt_addr);
    if (((uint64_t) * p_table_entry & PTE_P) != 0) {
        p_pt_entry =
            phys_to_kern_addr(phys_addr_mask
                    ((uint64_t *) * p_table_entry));
        return p_pt_entry + pt_offset(p_virt_addr);
    }
    if (lookup && !p_table_entry) {
        return NULL;
    }
    page = alloc_page(1);
    if (page) {
        page->page_ref++;
    } else {
        return NULL;
    }
    uint64_t *phys_addr =
        page_num_to_phys_addr(page_desc_to_page_num(page));
    p_pt_entry = phys_to_kern_addr(phys_addr);
    *p_table_entry = (uint64_t) phys_addr | perm | PTE_U;
    return p_pt_entry + pt_offset(p_virt_addr);
}

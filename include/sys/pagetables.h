#ifndef __PAGETABLES_H
#define __PAGETABLES_H

#define PAGE_SIZE 4096
#define PTE_P   0x1
#define PTE_W   0x2
#define PTE_U   0x4

extern uint64_t num_basemem_pages;
extern uint64_t num_extmem_pages;
extern uint64_t *p_physbase;
extern uint64_t *p_physfree;
extern uint64_t max_physmem;

struct page_descriptor {
    struct page_descriptor *next_page;
    uint16_t page_ref;
};

void mark_for_cow(uint64_t * parent_pml4, uint64_t * child_pml4,
        uint64_t * virt_addr, uint64_t size);
void remove_write_perm(uint64_t * parent_pml4, uint64_t * child_pml4,
        uint64_t * virt_addr);
void paging_init();
struct page_descriptor *alloc_page(int reset);
void map_address(uint64_t * pml4, uint64_t * virt_addr, uint64_t size);
void unmap_address(uint64_t * pml4, uint64_t * virt_addr, uint64_t size);
struct page_descriptor *lookup_page(uint64_t * p_table,
        uint64_t * virt_addr,
        uint64_t ** pp_pt_entry);
void *alloc_bytes(uint32_t bytes);
void page_desc_init();
uint64_t *kern_to_phys_addr(uint64_t * p_kern_addr);
struct page_descriptor *kern_to_page_desc(uint64_t * p_kern_addr);
uint64_t *phys_to_kern_addr(uint64_t * p_phys_addr);
uint64_t phys_to_page_num(uint64_t * phys_addr);
struct page_descriptor *page_num_to_page_desc(uint64_t page_num);
uint64_t page_desc_to_page_num(struct page_descriptor *p_desc);
uint64_t *page_num_to_phys_addr(uint64_t page_num);
uint64_t *page_desc_to_kern_addr(struct page_descriptor *page);
uint64_t *page_desc_to_phys_addr(struct page_descriptor *page);
uint64_t *phys_addr_mask(uint64_t * phys_addr);
uint64_t pml4_offset(uint64_t * virt_addr);
uint64_t pdp_offset(uint64_t * virt_addr);
uint64_t pd_offset(uint64_t * virt_addr);
uint64_t pt_offset(uint64_t * virt_addr);
uint64_t p_offset(uint64_t * virt_addr);
uint64_t *walk_pml4(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup);
uint64_t *walk_pdp(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup);
uint64_t *walk_pdir(uint64_t * p_table, uint64_t * p_virt_addr, int perm,
        int lookup);
void map(uint64_t * p_pml4e_addr, uint64_t virt_addr, uint64_t n,
        uint64_t phys_addr, int perm);
void deref_page(struct page_descriptor *page);
void remove_page(uint64_t * pml4, uint64_t * virt_addr);
void add_page(uint64_t * pml4, struct page_descriptor *page,
        uint64_t * virt_addr, int perm);

#endif

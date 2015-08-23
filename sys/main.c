#include <sys/sbunix.h>
#include <sys/process.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/klibc.h>
#include <sys/pagetables.h>

uint64_t num_basemem_pages;
uint64_t num_extmem_pages;
uint64_t *p_physbase;
uint64_t *p_physfree;
uint64_t max_physmem;
extern __volatile__ char *video_mem_start;

void setup_msrs();
void load_ltr();

extern void _syscall_handler(struct syscall_frame *p_sysf);
extern void syscall_handler(struct syscall_frame *p_sysf);
extern void handle_switch(struct stack_frame *p_sf);

void load_ltr()
{
    __asm__ __volatile__("mov $0x28,%ax");
    __asm__ __volatile__("ltr %ax");
}

void setup_msrs()
{
    uint32_t EFER = 0xc0000080;
    uint32_t STAR = 0xc0000081;
    uint32_t LSTAR = 0xc0000082;
    uint32_t SFMASK = 0xc0000084;
    uint64_t handler = (uint64_t) _syscall_handler;

    __asm__ __volatile__("wrmsr "::"c"(EFER), "a"(0x101));	//Enable syscall instruction and long mode

    uint64_t cstar = 0;
    cstar |= (uint64_t) 0x8 << 32;
    cstar |= (uint64_t) 0x23 << 48;

    __asm__ __volatile__("wrmsr"::"a"((uint32_t) cstar),
            "d"((uint32_t) (cstar >> 32)), "c"(STAR));

    __asm__ __volatile__("wrmsr"::"a"((uint32_t) handler),
            "d"((uint32_t) (handler >> 32)), "c"(LSTAR));

    uint64_t sfmask = 0x3700;	//Clear IF, TF, DF, IOPL

    __asm__ __volatile__("wrmsr"::"a"((uint32_t) sfmask),
            "d"((uint32_t) (sfmask >> 32)), "c"(SFMASK));

}

void start(uint32_t * modulep, void *physbase, void *physfree)
{
    p_physbase = (uint64_t *) physbase;
    p_physfree = (uint64_t *) physfree;
    volatile char *screen = (volatile char *) video_mem_start;
    //screen_color = 0x1F;
    screen_color = 0x00;
    memset_kernel((void *) screen, 0, 4000);	//clear the screen
    int i = 0;
    for (i = 1; i < 4000; i += 2) {
        *(screen + i) = screen_color;
    }
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    } __attribute__ ((packed)) * smap;
    while (modulep[0] != 0x9001)
        modulep += modulep[1] + 2;
    for (smap = (struct smap_t *) (modulep + 2);
            smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4);
            ++smap) {
        if (smap->type == 1 /* memory */  && smap->length != 0) {
            //printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
            if (0 == smap->base) {
                num_basemem_pages = smap->length / PAGE_SIZE;
            } else {
                num_extmem_pages =
                    (smap->base + smap->length -
                     (uint64_t) physfree) / PAGE_SIZE;
                max_physmem = smap->base + smap->length;
            }
        }
    }
    //printf("physfree = %p\n", physfree);
    //printf("physbase = %p\n", physbase);

    paging_init();
    setup_msrs();
    irq_init();
    load_idt();
    timer_init();
    tarfs_init();
    load_ltr();
    process_init();
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t *loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    __asm__("movq %%rsp, %0;" "movq %1, %%rsp;":"=g"(loader_stack)
            :	    "r"(&stack[INITIAL_STACK_SIZE])
           );
    reload_gdt();
    setup_tss();
    start((uint32_t *) ((char *) (uint64_t) loader_stack[3] +
                (uint64_t) & kernmem - (uint64_t) & physbase),
            &physbase, (void *) (uint64_t) loader_stack[4]
         );
    printf("\n\nStart Returned\n");
    //memcpy_kernel((void*)video_mem_start, "!1S1t1a1r1t1 1R1e1t1u1r1n1e1d1!1", 32);
    while (1);
}

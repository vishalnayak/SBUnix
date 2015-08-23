#include <sys/idt.h>
#include <sys/klibc.h>
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/handlers.h>
#include <sys/process.h>

extern __volatile__ char *video_mem_start;
extern struct tss_t tss;
struct sys_idt_descriptor idt[MAX_IDT] = { {0} };

static struct idtr_t idtr = {
    sizeof(idt),
    (uint64_t) idt,
};

int timer_tick = 100;
int timer_count = 0;

void irq_timer_handler(struct stack_frame *p_sf)
{
    uint64_t rsp = 0;
    __asm__ __volatile__("movq %%rdi,%0":"=r"(rsp)::);
    timer_tick--;
    if (!timer_tick) {
        timer_tick = 100;
        timer_count++;
        display_timer(timer_count);
    }

    curr_task->proc.timeslice--;
    if (!curr_task->proc.timeslice) {
        curr_task->proc.timeslice = TIMESLICE;
        schedule(SCHED_YIELD, rsp);
    }
    return;
}

void display_timer(int timer_count)
{
    int i = 0;
    char time[2];

    int seconds = timer_count % 60;
    int minutes = timer_count / 60;
    int hours = minutes / 60;

    minutes = minutes % 60;
    hours = hours % 60;

    __volatile__ char *clock_mem =
        (__volatile__ char *) ((uint64_t) video_mem_start + 0xF90);

    for (i = 0; i < 6; i += 2)
        *(clock_mem + i) = 0;
    memset_kernel(time, 0, 2);
    itoa_kernel((uint64_t) hours, time, 10);

    //print 'hours'
    for (i = 0; time[i]; i++) {
        *clock_mem++ = time[i];
        *clock_mem++ = screen_color;
    }
    //print ':'
    *clock_mem++ = ':';
    *clock_mem++ = screen_color;

    for (i = 0; i < 6; i += 2)
        *(clock_mem + i) = 0;
    memset_kernel(time, 0, 2);
    itoa_kernel((uint64_t) minutes, time, 10);

    //print 'minutes'
    for (i = 0; time[i]; i++) {
        *clock_mem++ = time[i];
        *clock_mem++ = screen_color;
    }

    //print ':'
    *clock_mem++ = ':';
    *clock_mem++ = screen_color;

    for (i = 0; i < 6; i += 2)
        *(clock_mem + i) = 0;
    memset_kernel(time, 0, 2);
    itoa_kernel((uint64_t) seconds, time, 10);

    //print 'minutes'
    for (i = 0; time[i]; i++) {
        *clock_mem++ = time[i];
        *clock_mem++ = screen_color;
    }

}

void irq_acknowledge()
{
    outb(0x20, 0x20);
}

void print_stack_frame(struct stack_frame *p_sf)
{
    uint64_t cr2;
    uint64_t cr0;
    __asm __volatile("movq %%cr0,%0":"=r"(cr0));
    __asm __volatile("movq %%cr2,%0":"=r"(cr2));
    printf("cr0: %x\n", cr0);
    printf("cr2: %p\n", cr2);
    //printf("rax: %p\n", p_sf->rax);
    //printf("rbx: %p\n", p_sf->rbx);
    //printf("rcx: %p\n", p_sf->rcx);
    //printf("rdx: %p\n", p_sf->rdx);
    //printf("rbp: %p\n", p_sf->rbp);
    //printf("rdi: %p\n", p_sf->rdi);
    //printf("rsi: %p\n", p_sf->rsi);
    //printf("r11: %p\n", p_sf->r11);
    //printf("r8: %p\n", p_sf->r8);
    //printf("r9: %p\n", p_sf->r9);
    //printf("r10: %p\n", p_sf->r10);
    //printf("r11: %p\n", p_sf->r11);
    //printf("r12: %p\n", p_sf->r12);
    //printf("r13: %p\n", p_sf->r13);
    //printf("r14: %p\n", p_sf->r14);
    //printf("r15: %p\n", p_sf->r15);
    //printf("es: %p\n", p_sf->es);
    //printf("ds: %p\n", p_sf->ds);
    //printf("gs: %p\n", p_sf->gs);
    //printf("fs: %p\n", p_sf->fs);
    //printf("isr_err: %p\n", p_sf->isr_err);
    //printf("rip: %p\n", p_sf->rip);
}

void interrupt_service_routine(struct stack_frame *p_sf)
{
    switch (p_sf->isr_num) {
        case IRQ_TIMER:
            irq_acknowledge();
            irq_timer_handler(p_sf);
            break;
        case IRQ_KEYBOARD:
            irq_acknowledge();
            irq_keyboard_handler(p_sf);
            break;
        case INTR_DIVIDE_BY_ZERO:
            isr_divide_by_zero_handler(p_sf);
            break;
        case INTR_DEBUG:
            isr_debug_handler(p_sf);
            break;
        case INTR_NMI:
            isr_nmi_handler(p_sf);
            break;
        case INTR_BREAKPOINT:
            isr_breakpoint_handler(p_sf);
            break;
        case OVERFLOW:
            isr_overflow_handler(p_sf);
            break;
        case OUT_OF_BOUNDS:
            isr_out_of_bounds_handler(p_sf);
            break;
        case INVALID_OPCODE:
            isr_invalid_opcode_handler(p_sf);
            break;
        case NO_COPROCESSOR:
            isr_no_coprocessor_handler(p_sf);
            break;
        case DOUBLE_FAULT:
            isr_err_double_fault_handler(p_sf);
            break;
        case COPROCESSOR_SEGMENT_OVERRUN:
            idr_coprocessor_segment_overrun_handler(p_sf);
            break;
        case INVALID_TSS:
            isr_err_invalid_tss_handler(p_sf);
            break;
        case SEGMENT_NOT_PRESENT:
            isr_err_segment_not_present_handler(p_sf);
            break;
        case STACK_FAULT:
            isr_err_stack_fault_handler(p_sf);
            break;
        case GENERAL_PROTECTION_FAULT:
            isr_err_general_protection_fault_handler(p_sf);
            break;
        case PAGE_FAULT:
            isr_err_page_fault_handler(p_sf);
            break;
        case UNKNOWN_INTERRUPT:
            isr_unknown_interrupt_handler(p_sf);
            break;
        case COPROCESSOR_FAULT:
            isr_coprocessor_fault_handler(p_sf);
            break;
        case ALIGNMENT_CHECK:
            isr_alignment_check_handler(p_sf);
            break;
        case MACHINE_CHECK:
            isr_machine_check_handler(p_sf);
            break;
        case SIMD_FLOATING_POINT:
            isr_simd_floating_point_handler(p_sf);
            break;
        case INTR_SYSCALL:
            isr_syscall_handler(p_sf);
            break;
        case INTR_DEFAULT:
            isr_default_handler(p_sf);
            break;
        default:
            printf("Interrupt Detected: isr_num=%d", p_sf->isr_num);
            break;
    }
}

void setup_idt_entry(struct sys_idt_descriptor *entry, void *isr,
        int privilege)
{
    memset_kernel((void *) entry, 0, sizeof(struct sys_idt_descriptor));
    entry->id_offset_first_16 = (uint64_t) (isr) & 0xffff;
    entry->id_offset_second_16 = ((uint64_t) (isr) >> 16) & 0xffff;
    entry->id_offset_third_32 = ((uint64_t) (isr) >> 32) & 0xffffffff;
    entry->id_segment_selector = 0x08;
    entry->id_type = 0xE;	//kernel data segment
    entry->id_dpl = privilege;
    entry->id_present = 1;
}

void setup_idt_entries()
{
    setup_idt_entry(&idt[0], isr_divide_by_zero, 0);
    setup_idt_entry(&idt[1], isr_debug, 0);
    setup_idt_entry(&idt[2], isr_nmi, 0);
    setup_idt_entry(&idt[3], isr_breakpoint, 3);
    setup_idt_entry(&idt[4], isr_overflow, 0);
    setup_idt_entry(&idt[5], isr_out_of_bounds, 0);
    setup_idt_entry(&idt[6], isr_invalid_opcode, 0);
    setup_idt_entry(&idt[7], isr_no_coprocessor, 0);
    setup_idt_entry(&idt[8], isr_err_double_fault, 0);
    setup_idt_entry(&idt[9], isr_coprocessor_segment_overrun, 0);
    setup_idt_entry(&idt[10], isr_err_invalid_tss, 0);
    setup_idt_entry(&idt[11], isr_err_segment_not_present, 0);
    setup_idt_entry(&idt[12], isr_err_stack_fault, 0);
    setup_idt_entry(&idt[13], isr_err_general_protection_fault, 0);
    setup_idt_entry(&idt[14], isr_err_page_fault, 0);
    setup_idt_entry(&idt[15], isr_unknown_interrupt, 0);
    setup_idt_entry(&idt[16], isr_coprocessor_fault, 0);
    setup_idt_entry(&idt[17], isr_alignment_check, 0);
    setup_idt_entry(&idt[18], isr_machine_check, 0);
    setup_idt_entry(&idt[19], isr_simd_floating_point, 0);
    setup_idt_entry(&idt[48], isr_syscall, 3);
    setup_idt_entry(&idt[500], isr_default, 0);
    setup_idt_entry(&idt[32], irq_timer, 0);
    setup_idt_entry(&idt[33], irq_keyboard, 0);
}

void load_idt()
{
    setup_idt_entries();
    _x86_64_asm_lidt(&idtr);
}

void timer_init()
{
    unsigned char lobyte = 11931 & 0xff;
    unsigned char hibyte = (11931 >> 8) & 0xff;
    outb(0x43, 0x36);
    outb(0x40, lobyte);
    outb(0x40, hibyte);
}

void irq_init()
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void enable_interrupts()
{
    __asm__ __volatile__("sti"::);
}

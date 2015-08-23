#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

#define INTR_DIVIDE_BY_ZERO 0
#define INTR_DEBUG 1
#define INTR_NMI 2
#define INTR_BREAKPOINT 3
#define OVERFLOW 4
#define OUT_OF_BOUNDS 5
#define INVALID_OPCODE 6
#define NO_COPROCESSOR 7
#define DOUBLE_FAULT 8
#define COPROCESSOR_SEGMENT_OVERRUN 9
#define INVALID_TSS 10
#define SEGMENT_NOT_PRESENT 11
#define STACK_FAULT 12
#define GENERAL_PROTECTION_FAULT 13
#define PAGE_FAULT 14
#define UNKNOWN_INTERRUPT 15
#define COPROCESSOR_FAULT 16
#define ALIGNMENT_CHECK 17
#define MACHINE_CHECK 18
#define SIMD_FLOATING_POINT 19

#define INTR_SYSCALL 48
#define INTR_DEFAULT 500

#define IRQ_TIMER 32
#define IRQ_KEYBOARD 33
#define IRQ_CASCADED 34
#define IRQ_SERIAL_CONTROLLER_PORT_2 35
#define IRQ_SERIAL_CONTROLLER_PORT_1 36
#define IRQ_PARALLEL_PORT_2_3_OR_SOUND 37
#define IRQ_FLOPPY 38
#define IRQ_PARALLEL_PORT_1 39
#define IRQ_REAL_TIME_CLOCK 40
#define IRQ_POWER_INTERFACE 41
#define IRQ_PERIPHERALS_1 42
#define IRQ_PERIPHERALS_2 43
#define IRQ_MOUSE_PS_2 44
#define IRQ_CPU_COPROCESSOR 45
#define IRQ_PRIMARY_ATA 46
#define IRQ_SECONDARY_ATA 47

#define MAX_IDT 256

struct sys_idt_descriptor {
    uint64_t id_offset_first_16:16;
    uint64_t id_segment_selector:16;
    uint64_t id_ist:3;
    uint64_t id_dont_know_1:5;
    uint64_t id_type:4;
    uint64_t id_dont_know_2:1;
    uint64_t id_dpl:2;
    uint64_t id_present:1;
    uint64_t id_offset_second_16:16;
    uint64_t id_offset_third_32:32;
    uint64_t id_reserved:32;
} __attribute__ ((packed));


struct idtr_t {
    uint16_t size;
    uint64_t addr;
} __attribute__ ((packed));

struct stack_frame {
    uint64_t fs;
    uint64_t gs;
    uint64_t ds;
    uint64_t es;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t isr_num;
    uint64_t isr_err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t stack;
    uint64_t ss;
};

void _x86_64_asm_lidt(struct idtr_t *idtr);
void setup_idt_entries();
void setup_idt_entry(struct sys_idt_descriptor *entry, void *isr,
        int privilege);
void load_idt();
void irq_init();
void timer_init();
void keyboard_init();
void enable_interrupts();
void display_timer();

void isr_divide_by_zero();
void isr_debug();
void isr_nmi();
void isr_breakpoint();
void isr_overflow();
void isr_out_of_bounds();
void isr_invalid_opcode();
void isr_no_coprocessor();
void isr_err_double_fault();
void isr_coprocessor_segment_overrun();
void isr_err_invalid_tss();
void isr_err_segment_not_present();
void isr_err_stack_fault();
void isr_err_general_protection_fault();
void isr_err_page_fault();
void isr_unknown_interrupt();
void isr_coprocessor_fault();
void isr_alignment_check();
void isr_machine_check();
void isr_simd_floating_point();

void isr_syscall();
void isr_default();

void irq_timer();
void irq_timer_handler();
void irq_keyboard();
void irq_keyboard_handler();


void irq_acknowledge();


#endif

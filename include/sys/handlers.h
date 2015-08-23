#ifndef __HANDLERS_H
#define __HANDLERS_H
#include <sys/idt.h>

void isr_divide_by_zero_handler(struct stack_frame *p_sf);
void isr_debug_handler(struct stack_frame *p_sf);
void isr_nmi_handler(struct stack_frame *p_sf);
void isr_breakpoint_handler(struct stack_frame *p_sf);
void isr_overflow_handler(struct stack_frame *p_sf);
void isr_out_of_bounds_handler(struct stack_frame *p_sf);
void isr_invalid_opcode_handler(struct stack_frame *p_sf);
void isr_no_coprocessor_handler(struct stack_frame *p_sf);
void isr_err_double_fault_handler(struct stack_frame *p_sf);
void idr_coprocessor_segment_overrun_handler(struct stack_frame *p_sf);
void isr_err_invalid_tss_handler(struct stack_frame *p_sf);
void isr_err_segment_not_present_handler(struct stack_frame *p_sf);
void isr_err_stack_fault_handler(struct stack_frame *p_sf);
void isr_err_general_protection_fault_handler(struct stack_frame *p_sf);
void isr_err_page_fault_handler(struct stack_frame *p_sf);
void isr_unknown_interrupt_handler(struct stack_frame *p_sf);
void isr_coprocessor_fault_handler(struct stack_frame *p_sf);
void isr_alignment_check_handler(struct stack_frame *p_sf);
void isr_machine_check_handler(struct stack_frame *p_sf);
void isr_simd_floating_point_handler(struct stack_frame *p_sf);
void isr_syscall_handler(struct stack_frame *p_sf);
void isr_default_handler(struct stack_frame *p_sf);
void irq_timer_handler(struct stack_frame *p_sf);
void irq_keyboard_handler(struct stack_frame *p_sf);

void print_stack_frame(struct stack_frame *p_sf);

#endif

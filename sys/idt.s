/
# idt.s
#
#  Created on: Feb 20, 2015
#      Author: vishal nayak (vishalnayak@gmail.com)
#

.text

######
# load IDT
.global _syscall_handler
.global _x86_64_asm_lidt
.global isr_divide_by_zero
.global isr_debug
.global isr_nmi
.global isr_breakpoint
.global isr_overflow
.global isr_out_of_bounds
.global isr_invalid_opcode
.global isr_no_coprocessor
.global isr_err_double_fault
.global isr_coprocessor_segment_overrun
.global isr_err_invalid_tss
.global isr_err_segment_not_present
.global isr_err_stack_fault
.global isr_err_general_protection_fault
.global isr_err_page_fault
.global isr_unknown_interrupt
.global isr_coprocessor_fault
.global isr_alignment_check
.global isr_machine_check
.global isr_simd_floating_point
.global isr_syscall
.global isr_default
.global irq_timer
.global irq_keyboard

_syscall_handler:

    pushq %rbp
    pushq %rbx
    pushq %r11
    pushq %rcx
    pushq %r9
    pushq %r8
    pushq %r10
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rax

    movq %rsp, %rdi

    callq syscall_handler
   
    popq %r15
    popq %rdi
    popq %rsi
    popq %rdx
    popq %r10
    popq %r8
    popq %r9
    popq %rcx
    popq %r11
    popq %rbx
    popq %rbp

    sysretq
    

_x86_64_asm_lidt:
    lidt (%rdi)
    retq

isr_divide_by_zero:
    pushq $0
    pushq $0
    jmp interrupt_handler

isr_debug:
    pushq $0
    pushq $1
    jmp interrupt_handler

isr_nmi:
    pushq $0
    pushq $2
    jmp interrupt_handler

isr_breakpoint:
    pushq $0
    pushq $3
    jmp interrupt_handler

isr_overflow:
    pushq $0
    pushq $4
    jmp interrupt_handler

isr_out_of_bounds:
    pushq $0
    pushq $5
    jmp interrupt_handler

isr_invalid_opcode:
    pushq $0
    pushq $6
    jmp interrupt_handler

isr_no_coprocessor:
    pushq $0
    pushq $7
    jmp interrupt_handler

isr_err_double_fault:
    pushq $8
    jmp interrupt_handler

isr_coprocessor_segment_overrun:
    pushq $0
    pushq $9
    jmp interrupt_handler

isr_err_invalid_tss:
    pushq $10
    jmp interrupt_handler

isr_err_segment_not_present:
    pushq $11
    jmp interrupt_handler

isr_err_stack_fault:
    pushq $12
    jmp interrupt_handler

isr_err_general_protection_fault:
    pushq $13
    jmp interrupt_handler

isr_err_page_fault:
    pushq $14
    jmp interrupt_handler

isr_unknown_interrupt:
    pushq $0
    pushq $15
    jmp interrupt_handler

isr_coprocessor_fault:
    pushq $0
    pushq $16
    jmp interrupt_handler

isr_alignment_check:
    pushq $0
    pushq $17
    jmp interrupt_handler

isr_machine_check:
    pushq $0
    pushq $18
    jmp interrupt_handler

isr_simd_floating_point:
    pushq $0
    pushq $19
    jmp interrupt_handler

isr_syscall:
    pushq $0
    pushq $48
    jmp interrupt_handler

isr_default:
    pushq $0
    pushq $500
    jmp interrupt_handler

irq_timer:
    pushq $0
    pushq $32
    jmp interrupt_handler

irq_keyboard:
    pushq $0
    pushq $33
    jmp interrupt_handler

interrupt_handler:

    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %r8 
    pushq %r9 
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %es,%r15
    pushq %r15

    movq %ds,%r15
    pushq %r15

    movq %gs,%r15
    pushq %r15

    movq %fs,%r15
    pushq %r15

    movq %rsp, %rdi

    movq $0x10,%r15
    movq %r15,%es
    movq %r15,%ds
    movq %r15,%gs
    movq %r15,%fs

    movq interrupt_service_routine, %rax
    callq interrupt_service_routine 
    movq %rsp, %r15
    addq $176, %r15
    movq $0x1b, (%r15)#kernel code segment
    addq $24, %r15
    movq $0x23, (%r15)#kernel data segment

    popq %r15
    movq %r15,%fs

    popq %r15
    movq %r15,%gs

    popq %r15
    movq %r15,%ds

    popq %r15
    movq %r15,%es

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rsi
    popq %rdi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16,%rsp
    iretq

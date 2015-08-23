#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>
#include <stdio.h>
static __inline uint64_t syscall_int_0(uint64_t n)
{
    uint64_t r = -1;
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_int_1(uint64_t n, uint64_t a1)
{
    uint64_t r = -1;
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1):"cc",
            "memory");
    return r;
}

static __inline uint64_t syscall_int_2(uint64_t n, uint64_t a1,
        uint64_t a2)
{
    uint64_t r = -1;
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1), "S"(a2):"cc",
            "memory");
    return r;
}

static __inline uint64_t syscall_int_3(uint64_t n, uint64_t a1,
        uint64_t a2, uint64_t a3)
{
    uint64_t r = -1;
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_int_4(uint64_t n, uint64_t a1,
        uint64_t a2, uint64_t a3,
        uint64_t a4)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10"::[a4] "r"(a4):"cc", "memory");
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_int_5(uint64_t n, uint64_t a1,
        uint64_t a2, uint64_t a3,
        uint64_t a4, uint64_t a5)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10\n\t"
            "movq %[a5], %%r8"::[a4] "r"(a4),
            [a5] "r"(a5):"cc", "memory");
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_0(uint64_t n)
{
    uint64_t r = -1;
    __asm__ __volatile__("syscall":"=a"(r):"a"(n):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_int_6(uint64_t n, uint64_t a1,
        uint64_t a2, uint64_t a3,
        uint64_t a4, uint64_t a5,
        uint64_t a6)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10\n\t"
            "movq %[a5], %%r8\n\n"
            "movq %[a6], %%r9"::[a4] "r"(a4),[a5] "r"(a5),
            [a6] "r"(a6):"cc", "memory");
    __asm__ __volatile__("int %[sys_call]":"=a"(r):[sys_call]
            "i"((uint64_t) 48), "a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1)
{
    uint64_t r = -1;
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2)
{
    uint64_t r = -1;
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1), "S"(a2):"cc",
            "memory");
    return r;
}

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2,
        uint64_t a3)
{
    uint64_t r = -1;
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2,
        uint64_t a3, uint64_t a4)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10"::[a4] "r"(a4):"cc", "memory");
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_5(uint64_t n, uint64_t a1, uint64_t a2,
        uint64_t a3, uint64_t a4, uint64_t a5)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10\n\t"
            "movq %[a5], %%r8"::[a4] "r"(a4),
            [a5] "r"(a5):"cc", "memory");
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}

static __inline uint64_t syscall_6(uint64_t n, uint64_t a1, uint64_t a2,
        uint64_t a3, uint64_t a4, uint64_t a5,
        uint64_t a6)
{
    uint64_t r = -1;
    __asm__ __volatile__("movq %[a4], %%r10\n\t"
            "movq %[a5], %%r8\n\n"
            "movq %[a6], %%r9"::[a4] "r"(a4),[a5] "r"(a5),
            [a6] "r"(a6):"cc", "memory");
    __asm__ __volatile__("syscall":"=a"(r):"a"(n), "D"(a1), "S"(a2),
            "d"(a3):"cc", "memory");
    return r;
}
#endif

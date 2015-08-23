#include<sys/klibc.h>
#include<sys/idt.h>
#include<sys/pagetables.h>

__volatile__ char *video_mem = (__volatile__ char *) (0xB8000);
__volatile__ char *video_mem_start = (__volatile__ char *) (0xB8000);
int charCount = 0;

void save_rsp(uint64_t rsp)
{
    __asm__ __volatile__("movq %%rsp, %[rsp_var];":[rsp_var] "=r"(rsp)::
            "memory");
}

void load_rsp(uint64_t rsp)
{
    __asm__ __volatile__("movq %[rsp_var], %%rsp;"::[rsp_var]
            "r"(rsp):"memory");
}

void load_cr3(uint64_t cr3)
{
    __asm__ __volatile__("movq %[cr3_var], %%cr3;"::[cr3_var] "r"(cr3));
}

uint64_t read_cr3()
{
    uint64_t val = 0;
    __asm__ __volatile__("movq %%cr3, %[val]":[val] "=r"(val):);
    return val;
}

void load_fs(uint64_t val)
{
    __asm__ __volatile__("movq %[val], %%fs;"::[val] "r"(val));
}

void load_gs(uint64_t val)
{
    __asm__ __volatile__("movq %[val], %%gs;"::[val] "r"(val));
}

void load_ds(uint64_t val)
{
    __asm__ __volatile__("movq %[val], %%ds;"::[val] "r"(val));
}

void load_es(uint64_t val)
{
    __asm__ __volatile__("movq %[val], %%es;"::[val] "r"(val));
}

int last_index_of(int ch, char *str)
{
    int len = strlen_kernel(str);
    int i = 0;
    for (i = len - 1; i >= 0; i--) {
        if (*(str + i) == ch) {
            return i;
        }
    }
    return -1;
}

int penultimate_index_of(int ch, char *str)
{
    int len = strlen_kernel(str);
    int i = 0;
    for (i = len - 2; i >= 0; i--) {
        if (*(str + i) == ch) {
            return i;
        }
    }
    return -1;
}

int index_of(int ch, char *str)
{
    int index = -1;
    int len = strlen_kernel(str);
    for (; *str && index + 1 < len; str++) {
        index++;
        if (ch == *str) {
            break;
        }
    }
    if (index + 1 == len && ch != *str) {
        index = -1;
    }
    return index;
}

char *strncpy_kernel(char *dest, const char *src, uint64_t n)
{
    uint64_t i = 0;
    int src_len = strlen_kernel(src);

    for (i = 0; i < n && i < src_len; i++) {
        *dest++ = *src++;
    }
    if (src_len < n) {
        *dest = '\0';
    }
    *dest = '\0';
    return dest;
}

uint64_t *kmalloc(uint64_t n)
{
    if (n > PAGE_SIZE) {
        printf
            ("Requested more than a page. Returning only one! Fix this.\n");
    }
    struct page_descriptor *page = alloc_page(1);
    page->page_ref++;
    return page_desc_to_kern_addr(page);
}

uint64_t malloc_kernel(uint64_t n)
{
    uint64_t num =
        ((uint64_t) align_up((void *) n, PAGE_SIZE)) / PAGE_SIZE;
    uint64_t i = 0;
    if (!num) {
        return 0;
    }
    struct page_descriptor *start = alloc_page(1);
    start->page_ref++;

    struct page_descriptor *p = NULL;
    for (i = 0; i < num - 1; i++) {
        p = alloc_page(1);
        p->page_ref++;
    }
    return (uint64_t) start;
}

int strncmp_kernel(const char *str1, const char *str2, int n)
{
    int count = 0;
    while (*str1 == *str2) {
        if (*str1 == 0 || *str2 == 0) {
            return 0;
        }
        str1++;
        str2++;
        count++;
    }
    if (count == n) {
        return 0;
    }
    return *str1 - *str2;
}

char *strcpy_kernel(char *dest, const char *src)
{
    char *p_dest = dest;
    const char *p_src = src;
    while (p_dest && p_src && *p_src != '\0') {
        *p_dest = *p_src;
        p_dest++;
        p_src++;
    }
    *p_dest = '\0';
    return dest;
}

unsigned char inb(int port)
{
    unsigned char val = 0;
    __asm__ __volatile__("inb %w[port],%[val]":[val] "=a"(val):[port]
            "d"(port));
    return val & 0xff;
}

void outb(int port, unsigned char data)
{
    __asm__ __volatile__("outb %[data],%w[port]"::[data] "a"(data),
            [port] "d"(port));
}

int strcmp_kernel(const char *str1, const char *str2)
{
    while (*str1 == *str2) {
        if (*str1 == 0 || *str2 == 0) {
            return 0;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

uint64_t strlen_kernel(const char *str)
{
    uint64_t count = 0;
    while (*str != '\0') {
        str++;
        count++;
    }
    return count;
}

uint64_t atoi_kernel(const char *str)
{
    uint64_t n = 0;
    char *p_str = (char *) str;
    while (*p_str) {
        n = n * 10 + (*p_str++ - '0');
    }
    return n;
}

uint64_t otoi_kernel(uint64_t octal)
{
    uint64_t decimal = 0, oct = octal;
    uint64_t mul = 1;
    while (oct) {
        decimal += mul * (oct % 10);
        mul *= 8;
        oct /= 10;
    }
    return decimal;
}

char *itoa_kernel(uint64_t value, char *str, int base)
{
    char *p_str = str;
    uint64_t n = value;
    if (0 == n) {
        *p_str++ = '0';
        *p_str = '\0';
    } else {
        while (n) {
            if (16 == base) {
                int bval = (n % base);
                if (10 == bval) {
                    *p_str = 'A';
                } else if (11 == bval) {
                    *p_str = 'B';
                } else if (12 == bval) {
                    *p_str = 'C';
                } else if (13 == bval) {
                    *p_str = 'D';
                } else if (14 == bval) {
                    *p_str = 'E';
                } else if (15 == bval) {
                    *p_str = 'F';
                } else {
                    *p_str = (n % base) + '0';
                }
            } else {
                *p_str = (n % base) + '0';
            }
            n /= base;
            p_str++;
        }
        *p_str = '\0';
        int str_len = p_str - str;
        int i = 0;
        int ch = 0;
        for (i = 0; i < (str_len / 2); i++) {
            ch = *(str + i);
            *(str + i) = *(str + str_len - 1 - i);
            *(str + str_len - 1 - i) = ch;
        }
    }
    return str;
}

void *memcpy_kernel(void *dest, const void *src, uint64_t n)
{
    uint64_t i = 0;
    char *p_dest = dest;
    const char *p_src = src;
    for (i = 0; i < n; i++) {
        *p_dest++ = *p_src++;
    }
    return dest;
}

void *memset_kernel(void *dest, int c, uint64_t count)
{
    uint64_t i = 0;
    char *mem = dest;
    for (i = 0; i < count; i++) {
        if (mem)
            *mem++ = c;
    }
    return dest;
}

void video_mem_backspace()
{
    if (video_mem != video_mem_start) {
        *video_mem = 0;
        *(video_mem + 1) = screen_color;
        video_mem -= 2;
        *video_mem = '_';
        *(video_mem + 1) = screen_color;
        charCount--;
    }
}

void scroll_screen()
{
    memcpy_kernel((void *) video_mem_start,
            (const void *) ((uint64_t) video_mem_start + 0xA0),
            0xF00 - 24);
    volatile char *last_line =
        (volatile char *) ((uint64_t) video_mem_start + 0xF00);
    int i = 0;
    for (i = 24; i >= 1; i -= 2) {
        *(last_line - i) = 0;
    }
    for (i = 0; i < 0xA0; i += 2) {
        *(last_line + i) = 0;
    }
    video_mem = (volatile char *) ((uint64_t) video_mem_start + 0xF00);
    charCount = 1920;
}

uint64_t syscall_clear()
{
    volatile char *screen = (volatile char *) video_mem_start;
//    screen_color = 0x1F;
    memset_kernel((void *) screen, 0, 4000);	//clear the screen
    int j = 0;
    for (j = 1; j < 4000; j += 2) {
        *(screen + j) = screen_color;
    }
    video_mem = video_mem_start;
    charCount = 0;
    return 0;
}

uint64_t video_mem_write(const void *buf, uint64_t count)
{
    int i = 0;
    const char *buffer = buf;
    for (i = 0; i < count; i++) {
        if (2000 - 12 == charCount) {
            scroll_screen();
        }
        if ('\n' == *buffer) {
            *video_mem = 0;
            *(video_mem + 1) = screen_color;
            if (video_mem >=
                    (char *) ((uint64_t) video_mem_start + 0xF00 - 24)) {
                scroll_screen();
            } else {
                int offset =
                    ((uint64_t) video_mem -
                     (uint64_t) video_mem_start) % 0xA0;
                video_mem -= offset;
                video_mem += 160;
                charCount += 80 - offset / 2;
            }
        } else {
            *video_mem++ = *buffer++;
            *video_mem++ = screen_color;
            charCount++;
        }
        *video_mem = '_';
        *(video_mem + 1) = screen_color;
    }
    return count;
}

void printf(const char *format, ...)
{
    va_list val;
    int printed = 0;
    char buf[64];
    va_start(val, format);
    while (*format) {
        if (*format == '%' && *(format + 1) == 's') {
            char *p_str = va_arg(val, char *);
            while (*p_str) {
                video_mem_write(p_str++, 1);
            }
            format += 2;
        } else if (*format == '%' && *(format + 1) == 'd') {
            memset_kernel(buf, 0, 64);
            int num = va_arg(val, int);
            if (num < 0) {
                buf[0] = '-';
                video_mem_write(buf, 1);
                buf[0] = 0;
                num *= -1;
            }
            itoa_kernel(num, buf, 10);
            video_mem_write(buf, strlen_kernel(buf));
            format += 2;
        } else if (*format == '%' && *(format + 1) == 'c') {
            int ch = va_arg(val, int);
            video_mem_write(&ch, 1);
            format += 2;
        } else if (*format == '%'
                && (*(format + 1) == 'x' || 'p' == *(format + 1))) {
            memset_kernel(buf, 0, 64);
            int num = va_arg(val, int);
            itoa_kernel(num, buf, 16);
            if ('p' == *(format + 1)) {
                video_mem_write("0x", 2);
            }
            video_mem_write(buf, strlen_kernel(buf));
            format += 2;
        } else {
            video_mem_write(format, 1);
            printed++;
            format++;
        }
    }
    va_end(val);
}

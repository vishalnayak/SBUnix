#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sbulib.h>
int printf(const char *format, ...)
{
    va_list val;
    int printed = 0;
    char buf[64];
    va_start(val, format);
    while (*format) {
        if (*format == '%' && *(format + 1) == 's') {
            char *p_str = va_arg(val, char *);
            while (p_str && *p_str) {
                write(1, p_str++, 1);
            }
            format += 2;
        } else if (*format == '%' && *(format + 1) == 'd') {
            memset(buf, 0, 64);
            int num = va_arg(val, int);
            if (num < 0) {
                buf[0] = '-';
                write(1, buf, 1);
                buf[0] = 0;
                num *= -1;
            }
            itoa(num, buf, 10);
            write(1, buf, strlen(buf));
            format += 2;
        } else if (*format == '%' && *(format + 1) == 'c') {
            int ch = va_arg(val, int);
            write(1, &ch, 1);
            format += 2;
        } else if (*format == '%'
                && (*(format + 1) == 'x' || *(format + 1) == 'p')) {
            memset(buf, 0, 64);
            int num = va_arg(val, int);
            if (num < 0) {
                buf[0] = '-';
                write(1, buf, 1);
                buf[0] = 0;
                num *= -1;
            }
            itoa(num, buf, 16);
            if (*(format + 1) == 'p') {
                write(1, "0x", 2);
            }
            write(1, buf, strlen(buf));
            format += 2;
        } else {
            write(1, format, 1);
            printed++;
            format++;
        }
    }
    va_end(val);
    return printed;
}

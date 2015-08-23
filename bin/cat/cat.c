#include <stdio.h>
#include <sbulib.h>
int fd = 0;
int main(int argc, char *argv[], char *envp[])
{
    int ch = 0;
    char buf[256] = { 0 };
    int k = 0;
    if (argc == 1) {
        while (1) {
            for (;; k++) {
                if (-1 == read(fd, buf + k, 1))
                    return 0;
                if (*(buf + k) == '\n')
                    break;
            }
            k = 0;
            printf("%s", buf);
            memset(buf, 0, 256);
        }
        return 0;
    }
    getcwd(buf, 256);
    if (-1 == (fd = open(argv[1], O_RDONLY))) {
        printf("-sbush: cat: file not found[%s]\n", argv[1]);
        exit(1);
    }
    ch = 0;
    while (0 < read(fd, &ch, 1)) {
        if (ch >= 10 && ch <= 127)
            printf("%c", ch);
    }
    return 0;
}

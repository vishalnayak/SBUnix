#include <stdio.h>
#include <stdlib.h>
#include <sbulib.h>

char cmd_str[MAX_LINE_LENGTH] = { 0 };

int index_of(int ch, char *str);
int match(const char *str1, const char *str2, int *idx);
int pipe1[2];
int main(int argc, char *argv[], char *envp[])
{
    char buf[256];
    scanf("%s", buf);
    printf("buf=[%s]\n", buf);
    scanf("%s", buf);
    printf("buf=[%s]\n", buf);
    while (1);
    return 0;
    int pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    if (!fork()) {
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[1], 1);
        close(pipe1[0]);
        if (-1 == execve("/bin/user3", argv, envp)) {
            printf("-sbush : %s: Failed. errno: %d\n", "/bin/user3",
                    errno);
            exit(1);
        }
    } else {
        if (!fork()) {
            close(pipe2[0]);
            close(pipe2[1]);
            dup2(pipe1[0], 0);
            close(pipe1[1]);
            if (-1 == execve("/bin/user4", argv, envp)) {
                printf("-sbush : %s: Failed. errno: %d\n", "/bin/user4",
                        errno);
                exit(1);
            }
        }
    }
    while (1);
    return 0;
#if 0
    printf("user1\n");
    printf("[/,b/ixyn/] [%d]\n\n", index_of('/', "b/ixyn/"));
    printf("[/,b//] [%d]\n\n", index_of('/', "b//"));
    printf("[/,/b//] [%d]\n\n", index_of('/', "/b//"));
    printf("[/,bin] [%d]\n\n", index_of('/', "bin"));
    printf("[/,bin/] [%d]\n\n", index_of('/', "bin/"));
    int idx = -1;
    printf("[bin/,bin/] match[%d] ", match("bin/", "bin/", &idx));
    printf("index[%d]\n", idx);
    printf("[bin/,bin/user1] match[%d] ",
            match("bin/", "bin/user1", &idx));
    printf("index[%d]\n", idx);
    printf("[bin/,bin] match[%d] ", match("bin/", "bin", &idx));
    printf("index[%d]\n", idx);
    printf("[bin/,b/user1] match[%d] ", match("bin/", "b/user1", &idx));
    printf("index[%d]\n", idx);
    printf("[bin/,b] match[%d] ", match("bin/", "b", &idx));
    printf("index[%d]\n", idx);
    printf("[,bin/a] match[%d] ", match("", "bin/a", &idx));
    printf("index[%d]\n", idx);
    printf("[,b] match[%d] ", match("", "b", &idx));
    printf("index[%d]\n", idx);
    return 0;
    //DIR* dir = opendir("bin/");
    //DIR* dir = opendir("/bin");
    //DIR* dir = opendir("/bin/");
    //DIR* dir = opendir("b");
    //DIR* dir = opendir("b/");
    //DIR* dir = opendir("/b");
    //DIR* dir = opendir("/b/");
    //DIR* dir = opendir(buf);
#endif
    //=========================================================

    int pid = -1;
    int fd;
    if ((pid = fork()) == 0) {
        printf("child proc\n");
        //open
        if ((fd = open("bin/vishal", O_RDONLY)) < 0) {
            printf("Open Failed.");
        }
        printf("Opened file fd[%d]\n", fd);

        //read
        char ch = 0;
        while (0 < read(fd, &ch, 1)) {
            printf("%c", ch);
        }
        printf("\n");
        while (1);
    } else {
        printf("parent proc\n");
        while (1) {
        }
    }
    printf("finishing pid [%d]\n", pid);

    //scanf
    printf("Enter line:");
    scanf("%z", cmd_str);
    if (cmd_str[strlen(cmd_str) - 1] == '\n') {
        cmd_str[strlen(cmd_str) - 1] = 0;
    }
    printf("Received line:[%s]\n", cmd_str);

    //--malloc()
    char *s = (char *) malloc(10);
    s[0] = 'f';
    s[1] = 'i';
    s[2] = 'n';
    s[3] = 'a';
    s[4] = 'l';
    s[5] = 'l';
    s[6] = 'y';
    s[7] = '!';
    s[8] = '!';
    s[9] = '!';
    s[10] = '!';
    s[11] = '!';
    s[12] = '\0';
    printf("malloc-ed = %s\n", s);

    free(s);

    pid_t gpid = getpid();
    printf("pid:%d\n", gpid);

    pid_t ppid = getppid();
    printf("ppid:%d\n", ppid);

    getcwd(NULL, 0);

    execve(NULL, NULL, NULL);

    waitpid(0, NULL, 0);

    open(NULL, 0);

    read(0, NULL, 0);

    write(0, NULL, 0);

    chdir(NULL);

    sleep(0);

    //getdents();

    lseek(0, 0, 0);

    close(0);

    //ps();

    dup(0);

    dup2(0, 0);
    //=========================================================

    //    munmap();
    pipe(NULL);

    printf("~~~~~\n");
    while (1);
    return 0;
}

int match(const char *str1, const char *str2, int *idx)
{
    int index = -1;
    int len = strlen(str1);
    printf("len[%d] ", len);
    if (!len) {
        *idx = 0;
        return 1;
    }
    while (*str1 == *str2 && index + 1 < len) {
        index++;
        if (*str1 == 0 || *str2 == 0) {
            return index;
        }
        str1++;
        str2++;
    }
    if (len == index + 1 && *str1 && *str2 && *str1 != *str2) {
        index = -1;
        *idx = len;
    } else {
        *idx = index + 1;
    }
    return index == -1 ? 0 : len == index + 1;
}

int index_of(int ch, char *str)
{
    int index = -1;
    int len = strlen(str);
    for (; *str && index + 1 < len; str++) {
        index++;
        printf("ch %d *str %d\n", ch, *str);
        if (ch == *str) {
            break;
        }
    }
    if (index + 1 == len && ch != *str) {
        index = -1;
    }
    return index;
}

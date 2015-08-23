#include <stdio.h>
#include <sbulib.h>
void testing()
{
    printf("User space printing\n");
    return;
}

int main(int argc, char *argv[], char *envp[])
{

    //test_printf_scanf();
    //test_pipe(argc,argv,envp);
    //test_malloc();
    //test_ls();
    //test_cat();
    //int r = chdir("..");
    //printf("%d\n", r);
    //sleep(5);
    testing();
    return 0;
}


void test_printf_scanf(void)
{

    //printf("Hello World!\n");
    //printf("String=%s done.\n","VishalNayak");
    //printf("+ve Integer=%d done.\n",111132);
    //printf("-ve Integer=%d done.\n",-541);
    //printf("+ve Hex=%x done.\n",48);
    //printf("-ve Hex=%x done.\n",-48);
    //printf("Character=%c done.\n",105);
    int i = 0;
    char ch = 0;
    char str[20];
    char line[MAX_LINE_LENGTH];

    //printf("+ve Integer: ");
    //scanf("%d", &i);
    //printf("%d\n", i);
    printf("-ve Integer: ");
    scanf("%d", &i);
    printf("%d\n", i);
    printf("+ve Hex to Integer: ");
    scanf("%x", &i);
    printf("%d\n", i);
    printf("-ve Hex to Integer: ");
    scanf("%x", &i);
    printf("%d\n", i);
    printf("String: ");
    scanf("%s", str);
    printf("%s\n", str);
    printf("Character: ");
    scanf("%c", &ch);
    printf("%c\n", ch);
    printf("Line: ");
    scanf("%v", line);
    printf("%s\n", line);
} void test_pipe(int argc, char *argv[], char *envp1[])
{
    char **args[2];
    char *args1[] = { "/bin/ls", NULL };
    char *args2[] = { "/bin/grep", "rootfs", NULL };
    char *envp[] = { "PATH=/bin:/usr/bin", "PS1=vishal-sbush $", NULL };
    args[0] = args1;
    args[1] = args2;
    pid_t pid;
    int filedes[2];
    pipe(filedes);
    int i = 0;
    while (envp[i]) {

        //printf("%s\n", envp[i]);
        i++;
    }
    printf("\n\n");
    if ((pid = fork()) < 0) {
        printf("fork() failed\n");
    } else if (0 == pid) {
        dup2(filedes[1], 1);
        close(filedes[0]);
        execve((const char *) *args[0], (char *const *) args[0], envp);
        printf("-sbush: %s: command not found\n", *args[0]);
        exit(0);
    } else {
        dup2(filedes[0], 0);
        close(filedes[1]);
        execve((const char *) *args[1], (char *const *) args[1], envp);
    }} void test_malloc()
{
    size_t *addr = (size_t *) malloc(10);
    free(addr);
    char *name = (char *) malloc(2 * sizeof(char));
    free(name);
} void test_cat()
{
    char *loc = "/home/stufs1/vnayak/cse506/w1/sbush.sh";
    int fd = 0;
    if (-1 == (fd = open(loc, O_RDONLY))) {
        printf("Open failed. pathname=%s O_RDONLY\n", loc);
        exit(1);
    }
    char ch = 0;
    while (0 < read(fd, &ch, 1)) {
        printf("%c", ch);
    }
}

void test_ls()
{
    char *loc = "/home/stufs1/vnayak/cse506/w1";
    struct dirent *dirent;
    DIR *dir = opendir(loc);
    while ((dirent = readdir(dir))) {
        printf("%s  ", dirent->d_name);
    }
    printf("\n");
    closedir(dir);
}

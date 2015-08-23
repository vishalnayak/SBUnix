#include<sbulib.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    if (argc != 3 || strcmp(argv[1], "-9")) {
        printf("-sbush: Usage: kill -9 <pid>\n");
        exit(0);
    }
    kill(-9, atoi(argv[2]));
    return 0;
}

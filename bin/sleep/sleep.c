#include<sbulib.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    if (argc != 2) {
        printf("-sbush: Usage: sleep <seconds>\n");
        exit(0);
    }
    sleep(atoi(argv[1]));
    return 0;
}

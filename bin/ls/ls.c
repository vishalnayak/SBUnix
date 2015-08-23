#include <stdio.h>
#include <sbulib.h>
int main(int argc, char *argv[], char *envp[])
{
    char buf[256];
    struct dirent *dirent;
    getcwd(buf, 256);
    DIR *dir = opendir(buf);
    while ((dirent = readdir(dir))) {
        printf("%s  ", dirent->d_name);
    }
    printf("\n");
    return 0;
}

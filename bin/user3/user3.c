#include <stdio.h>
#include <stdlib.h>
#include <sbulib.h>
#define SIZE 600 
int main(int argc, char *argv[], char *envp[])
{
    int i = 0;
    for(i=0;i<SIZE;i++){
        if(0 == fork()){
        printf("c=%d\n",i);
            return 0;
        }
    }
    printf("user3 printing\n");
    return 0;
}

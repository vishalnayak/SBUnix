#include<sbulib.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]){
    if(argc < 2){
        printf("-sbush: Usage: echo <string>\n");
        exit(0);
    }
    if(!strcmp(argv[1],"$PATH")){
        printf("%s\n",envp[0]);
        return 0;
    }
    if(!strcmp(argv[1],"$PS1")){
        printf("%s\n",envp[1]);
        return 0;
    }
    argv++;
    while(*argv){
        printf("%s ", *argv++);
    }
    printf("\n", *argv++);
    return 0;
}

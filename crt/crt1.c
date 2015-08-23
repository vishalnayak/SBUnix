#include <stdlib.h>
#include <stdio.h>
int main(int argc, char* argv[], char* envp[]);
void _start(void)
{
#if 0
    int argc = 0;
    char **argv = NULL;
    char **envp = NULL;
    uint64_t *p_rsp = 0;
    printf("",argc,argv,envp,p_rsp);
    exit(main(1,NULL,NULL));
#else 
    __volatile__ int argc;
    __volatile__ char **argv;
    __volatile__ char **envp;
    __volatile__ uint64_t *p_rsp = 0;
    __asm__ __volatile__("movq %%rsp, %[p_rsp]":[p_rsp] "=r"(p_rsp)::"memory");
    if((uint64_t)_start==0x4000f0){
        argc = *(p_rsp + 3);
        argv = (__volatile__ char **)(p_rsp + 4);
        envp = (__volatile__ char **)(p_rsp + 3 + argc + 2);
    }else{
        argc = *(p_rsp + 5);
        argv = (__volatile__ char **)(p_rsp + 6);
        envp = (__volatile__ char **)(p_rsp + 5 + argc + 2);
    }
//    printf("",argc,argv,envp,p_rsp);
    exit(main((int)argc, (char**)argv, (char**)envp));
    //exit(main(1,NULL,NULL));
#endif
}

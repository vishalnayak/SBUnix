#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <sbulib.h>
int process_cmd_ls();
int process_cmd();
int process_cmd_cd();
int process_cmd_export();
int process_cmd_exec_bin();
int process_cmd_cat();
int parse_cmd(char *p_cmd_str, int *argc, char *argv[]);
int show_prompt(void);
void print_args(char *argv[]);
void print_all();
void *memset1(void *str, int c, size_t n);
int contains(char *str, int ch);
int parse_envs(char *envp[]);
void free_all();
int execute(char *filename, char *argv[], char *envp[]);
char cmd_str[MAX_LINE_LENGTH] = { 0 };

int cmd_argc = 0;
char cmd2_str[MAX_LINE_LENGTH] = { 0 };

int cmd2_argc = 0;
char cmd3_str[MAX_LINE_LENGTH] = { 0 };

int cmd3_argc = 0;
char cmd4_str[MAX_LINE_LENGTH] = { 0 };

int cmd4_argc = 0;
char cmd5_str[MAX_LINE_LENGTH] = { 0 };

int cmd5_argc = 0;
char **argvs[5];
int *argcs[] =
{ &cmd_argc, &cmd2_argc, &cmd3_argc, &cmd4_argc, &cmd5_argc };
char env_path[MAX_LINE_LENGTH] = { 0 };
char env_ps1[MAX_LINE_LENGTH] = { 0 };
char *envps[] = { env_path, env_ps1, 0 };

int fd[8] = { 0 };

int parse_envs(char *envp[])
{
    char path_str[4] = { 0 };
    char ps1_str[3] = { 0 };
    int i = 0, count = 0;
    while (envp[i] != NULL) {
        strncpy(path_str, envp[i], 4);
        strncpy(ps1_str, envp[i], 3);
        if (!strcmp(path_str, "PATH")) {
            memset(env_path, 0, MAX_LINE_LENGTH);
            strncpy(env_path, envp[i], strlen(envp[i]));
            count++;
        }
        if (!strcmp(ps1_str, "PS1")) {
            memset(env_ps1, 0, MAX_LINE_LENGTH);
            strncpy(env_ps1, envp[i], strlen(envp[i]));
            count++;
        }
        if (2 == count) {
            break;
        }
        i++;
    }
    return 0;
}

int execute(char *filename, char *argv[], char *envp[])
{
    char file_name[MAX_LINE_LENGTH];
    char *ptr = env_path;
    int env_len = strlen(env_path);
    int index = 0;
    char path_name[MAX_LINE_LENGTH] = { 0 };
    strcpy(file_name, filename);
    ptr += contains(ptr, '=') + 1;
    while (ptr < env_path + env_len) {
        if (-1 == contains(ptr, ':')) {
            index = strlen(ptr);
            memset(path_name, 0, MAX_LINE_LENGTH);
            strncpy(path_name, ptr, strlen(ptr));
            path_name[index] = '/';
            strncpy(path_name + index + 1, file_name, strlen(file_name));
            if (*file_name != '/')
                strcpy(filename, path_name);

            if (-1 == execve(filename, argv, envp)) {
                printf("-sbush: %s: command not found: error:%s\n",
                        file_name, error());
                exit(1);
            }
            exit(0);
        } else if (0 <= (index = contains(ptr, ':'))) {
            memset(path_name, 0, MAX_LINE_LENGTH);
            strncpy(path_name, ptr, index);
            path_name[index] = '/';
            strncpy(path_name + index + 1, file_name, strlen(file_name));
            if (*file_name != '/')
                strcpy(filename, path_name);
            if (-1 == execve(filename, argv, envp)
                    && strcmp(error(), "ENOENT")) {
                printf("-sbush: %s: failed: error:%s\n", file_name,
                        error());
                exit(1);
            }
            ptr += index + 1;
        }
    }
    return 0;
}

int process_cmd_exec_bin()
{
    pid_t pid[5] = { 0 };
    int count = 0;
    while (count < 5) {
        if (*argcs[count] == 0)
            break;
        count++;
    }
    if (-1 == contains(env_path, '=')) {
        strcpy(env_path, "PATH=/bin");
    }
    if (-1 == contains(env_ps1, '=')) {
        strcpy(env_ps1, "PS1=[sbush#vishal]$ ");
    }
    int k = 0;
    int i = 0;
    char **ptr = argvs[count - 1];
    for (; ptr[k]; k++);
    if (!strcmp("&", argvs[count - 1][k - 1]) && k!=1) {
        argvs[count - 1][k - 1] = 0;
        sched_setparam(BG_ENABLE, 1);
    }
    if (1 == count) {
        if (0 == (pid[0] = fork())) {
            printf("");
            argvs[0][*argcs[0]] = 0;
            execute(argvs[0][0], argvs[0], envps);
        }
    } else {
        pipe(fd);
        pipe(fd + 2);
        pipe(fd + 4);
        pipe(fd + 6);
        if (0 == (pid[0] = fork())) {
            close(fd[2]);
            close(fd[3]);
            close(fd[4]);
            close(fd[5]);
            close(fd[6]);
            close(fd[7]);
            dup2(fd[1], 1);
            close(fd[0]);
            argvs[0][*argcs[0]] = 0;
            execute(argvs[0][0], argvs[0], envps);
        } else {
            if (2 == count) {
                if (0 == (pid[1] = fork())) {
                    close(fd[2]);
                    close(fd[3]);
                    close(fd[4]);
                    close(fd[5]);
                    close(fd[6]);
                    close(fd[7]);
                    dup2(fd[0], 0);
                    close(fd[1]);
                    argvs[1][*argcs[1]] = 0;
                    execute(argvs[1][0], argvs[1], envps);
                }
                goto end;
            }
            if (0 == (pid[1] = fork())) {
                close(fd[4]);
                close(fd[5]);
                close(fd[6]);
                close(fd[7]);
                dup2(fd[0], 0);
                close(fd[1]);
                close(fd[2]);
                dup2(fd[3], 1);
                argvs[1][*argcs[1]] = 0;
                execute(argvs[1][0], argvs[1], envps);
            } else {
                if (3 == count) {
                    if (0 == (pid[2] = fork())) {
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[4]);
                        close(fd[5]);
                        close(fd[6]);
                        close(fd[7]);
                        dup2(fd[2], 0);
                        close(fd[3]);
                        argvs[2][*argcs[2]] = 0;
                        execute(argvs[2][0], argvs[2], envps);
                    }
                    goto end;
                }
                if (0 == (pid[2] = fork())) {
                    close(fd[0]);
                    close(fd[1]);
                    close(fd[6]);
                    close(fd[7]);
                    dup2(fd[2], 0);
                    close(fd[3]);
                    close(fd[4]);
                    dup2(fd[5], 1);
                    argvs[2][*argcs[2]] = 0;
                    execute(argvs[2][0], argvs[2], envps);
                } else {
                    if (4 == count) {
                        if (0 == (pid[3] = fork())) {
                            close(fd[0]);
                            close(fd[1]);
                            close(fd[2]);
                            close(fd[3]);
                            close(fd[6]);
                            close(fd[7]);
                            dup2(fd[4], 0);
                            close(fd[5]);
                            argvs[3][*argcs[3]] = 0;
                            execute(argvs[3][0], argvs[3], envps);
                        }
                        goto end;
                    }
                    if (0 == (pid[3] = fork())) {
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[2]);
                        close(fd[3]);
                        dup2(fd[4], 0);
                        close(fd[5]);
                        close(fd[6]);
                        dup2(fd[7], 1);
                        argvs[3][*argcs[3]] = 0;
                        execute(argvs[3][0], argvs[3], envps);
                    } else {
                        if (0 == (pid[4] = fork())) {
                            close(fd[0]);
                            close(fd[1]);
                            close(fd[2]);
                            close(fd[3]);
                            close(fd[4]);
                            close(fd[5]);
                            dup2(fd[6], 0);
                            close(fd[7]);
                            argvs[4][*argcs[4]] = 0;
                            execute(argvs[4][0], argvs[4], envps);
                        }
                    }
                }
            }
        }
end:for (i = 0; i < 8; i++)
        close(fd[i]);
    }
    int status = 0;
    //printf("wait for: pid[%d]\n",pid[0]);
    for (i = 0; i < count; i++) {
        //printf("WF:(%d)\n",pid[i]);
        waitpid(pid[i], &status, 0);
        //printf("WFR:(%d)\n",rpid);
    }
    //    printf("returned\n");
    free_all();
    sched_setparam(BG_ENABLE, 0);
    return 0;

}

int process_cmd()
{
    if (NULL == cmd_str) {
        return -1;
    }
    int index = 0;
    if ((index = contains(cmd_str, '|')) >= 0) {
        char *p_str = cmd_str;
        int i = 0;
        for (i = 0; i < 5 && (index = contains(p_str, '|')) >= 0; i++) {
            p_str[index] = '\0';
            parse_cmd(p_str, argcs[i], (char **) argvs[i]);
            p_str += index + 1;
        }
        //strncpy(cmd2_str,cmd_str + index + 1, strlen(cmd_str) - index - 1);
        p_str[index] = '\0';
        p_str += index + 1;
        if (i < 5) {
            parse_cmd(p_str, argcs[i], argvs[i]);

            //print_args(argvs[i]);
        }
        process_cmd_exec_bin();
    } else {
        if (0 > parse_cmd(cmd_str, &cmd_argc, (char **) argvs[0])) {
            free_all();
            return 0;
        }
        if (!strlen(argvs[0][0])) {
            free_all();
            return 0;
        }
        if (!strcmp(argvs[0][0], "exit")) {
            printf("%s\n", argvs[0][0]);
            exit(0);
        }
        if (!strcmp(argvs[0][0], "cd")) {
            process_cmd_cd();
            free_all();
            return 0;
        }
        if (!strcmp(argvs[0][0], "export")) {
            process_cmd_export();
            free_all();
            return 0;
        }
        if (!strcmp(argvs[0][0], "clear")) {
            clear();
            return 0;
        }
        if (!strcmp(argvs[0][0], "pwd")) {
            char buf[256];
            getcwd(buf, 256);
            printf("%s\n", buf);
            return 0;
        }
        /*
           if (!strcmp(argvs[0][0], "ls")) {
           process_cmd_ls();
           free_all();
           return 0;
           }
           if (!strcmp(argvs[0][0], "ps")) {
           ps();
           return 0;
           }
           if (!strcmp(argvs[0][0], "cat")) {
           process_cmd_cat();
           free_all();
           return 0;
           }
         */
        process_cmd_exec_bin();
    }
    return 0;
}

void print_all()
{
    size_t i = 0;
    printf
        ("\n\n\n----------------------------------------------------\nenvp={ ");
    for (i = 0; envps[i]; i++) {
        printf(" %s,", envps[i]);
    }
    printf("}\n");
    if (cmd_argc != 0) {
        printf("argc=%d argv=", cmd_argc);
        i = 0;
        while (argvs[0][i])
            printf(" %s", argvs[0][i++]);
        printf("\n");
    }
    if (cmd2_argc != 0) {
        printf("argc2=%d argv2=", cmd2_argc);
        i = 0;
        while (argvs[1][i])
            printf(" %s", argvs[1][i++]);
        printf("\n");
    }
    if (cmd3_argc != 0) {
        printf("argc3=%d argv3=", cmd3_argc);
        i = 0;
        while (argvs[2][i])
            printf(" %s", argvs[2][i++]);
        printf("\n");
    }
    if (cmd4_argc != 0) {
        printf("argc4=%d argv4=", cmd4_argc);
        i = 0;
        while (argvs[3][i])
            printf(" %s", argvs[3][i++]);
        printf("\n");
    }
    if (cmd5_argc != 0) {
        printf("argc5=%d argv5=", cmd5_argc);
        i = 0;
        while (argvs[4][i])
            printf(" %s", argvs[4][i++]);
        printf("\n");
    }
    printf("----------------------------------------------------\n\n\n");
}

void print_args(char *argv[])
{
    size_t i = 0;
    printf("argv={");
    while (argv[i]) {
        printf(" %s,", argv[i++]);
    }
    printf(" }\n\n\n");
}

int process_cmd_cat()
{
    if (cmd_argc < 2) {
        printf("-sbush: Usage: cat <filename>\n");
        return -1;
    }
    int fd = 0;
    if (-1 == (fd = open(argvs[0][1], O_RDONLY))) {
        printf("Open failed. pathname=%s O_RDONLY\n", argvs[0][1]);
        return -1;
    }
    int ch = 0;
    while (0 < read(fd, &ch, 1)) {
        printf("%c", ch);
    }
    close(fd);
    return 0;
}

int process_cmd_ls()
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

int process_cmd_cd()
{
    if (cmd_argc < 2) {
        printf("-sbush: Usage: cd <directory>\n");
        return -1;
    }
    if (0 > chdir(argvs[0][1])) {
        printf("-sbush: cd: %s: No such file or directory\n", argvs[0][1]);
        return -1;
    }
    return 0;
}

int process_cmd_export()
{
    if (cmd_argc < 2) {
        printf
            ("-sbush: Usage: export <ENV_VAR_NAME>=\"<ENV_VAR_VALUE>\"\n");
        return -1;
    }
    char path_str[] = "PATH";
    char ps1_str[] = "PS1";
    strncpy(path_str, argvs[0][1], strlen(path_str));
    strncpy(ps1_str, argvs[0][1], strlen(ps1_str));
    if (!strcmp(path_str, "PATH")) {
        memset(env_path, 0, MAX_LINE_LENGTH);
        strncpy(env_path, argvs[0][1], strlen(argvs[0][1]));
        return 0;
    }
    if (!strcmp(ps1_str, "PS1")) {
        memset(env_ps1, 0, MAX_LINE_LENGTH);
        strncpy(env_ps1, argvs[0][1], strlen(argvs[0][1]));
        return 0;
    }
    printf("-sbush: export: unsupported env variable\n");
    return 0;
}

int parse_cmd(char *p_cmd_str, int *argc, char *argv[])
{
    int word_length = 0;
    char *p_str = p_cmd_str;
    int str_len = strlen(p_cmd_str);
    int ret_val = -1;
    *argc = 0;
    while (*p_str == ' ')
        p_str++;
    word_length = strcspn(p_str, " ");
    while (word_length > 0 && ((p_str - p_cmd_str) < str_len)) {
        argv[*argc] = (char *) malloc(sizeof(char) * word_length);
        memset(argv[*argc], 0, sizeof(char) * word_length);
        strncpy(argv[*argc], p_str, word_length);
        argv[*argc][word_length] = 0;
        p_str += word_length + 1;
        word_length = strcspn(p_str, " ");
        (*argc)++;
        ret_val = 0;
    }
    return ret_val;
}

int contains(char *str, int ch)
{
    int i = 0;
    char *s = str;
    for (i = 0; i < strlen(str) && (*s++ != ch); i++);
    return (i == strlen(str)) ? -1 : i;
}

int show_prompt(void)
{
    if (-1 == contains(env_ps1, '=')) {
        strcpy(env_ps1, "PS1=[sbush#vishal]");
    }
    int index = 0;
    index = contains(env_ps1, '=');
    //printf("%s", env_ps1 + index + 1);
    char path[256] = { 0 };
    char prompt[256] = { 0 };
    getcwd(path, 256);
    strcpy(prompt, env_ps1 + index + 1);
    strncpy(prompt + strlen(prompt), path, strlen(path));
    strcpy(prompt + strlen(prompt), "$");
    printf("%s ", prompt);
    return 0;
}

int main(int argc, char *argv[], char *envp[])
{
    sched_setparam(BG_ENABLE, 1);
    sched_setparam(BG_ENABLE, 0);
    if (!argc)
        argc = 1;
    int i = 0;
    for (i = 0; i < 5; i++) {
        argvs[i] = (char **) malloc(sizeof(char) * MAX_LINE_LENGTH);
    }
    int cmd_count = 0;
    show_prompt();
    while (1) {
        if (1 == argc || (argc > 1 && cmd_count != 0)) {
            char *p_line = cmd_str;
            memset(p_line, 0, MAX_LINE_LENGTH);
            scanf("%z", p_line);
            //printf("%s", p_line);
            while (*p_line == ' ')
                p_line++;
            if (0 == strlen(p_line)) {
                show_prompt();
                continue;
            }
            if (p_line[strlen(p_line) - 1] == '\n') {
                p_line[strlen(p_line) - 1] = 0;
            }
            if (process_cmd() < 0) {
                break;
            }
            show_prompt();
        } else {
            int fd = open(argv[1], O_RDONLY);
            char buf[MAX_LINE_LENGTH] = { 0 };
            int buf_idx = 0;
            while (read(fd, &buf[buf_idx], 1) > 0) {
                if (buf[buf_idx] == '\n') {
                    buf[buf_idx] = 0;
                    if (cmd_count == 0) {
                        cmd_count++;
                    } else {
                        memset(cmd_str, 0, MAX_LINE_LENGTH);
                        strcpy(cmd_str, buf);
                        process_cmd();
                    }
                    buf_idx = 0;
                    memset(buf, 0, MAX_LINE_LENGTH);
                } else {
                    buf_idx++;
                }
            }
            close(fd);
            break;
        }
    }
    return 0;
}

void free_all()
{
    memset(cmd_str, 0, MAX_LINE_LENGTH);
    memset(cmd2_str, 0, MAX_LINE_LENGTH);
    memset(cmd3_str, 0, MAX_LINE_LENGTH);
    memset(cmd4_str, 0, MAX_LINE_LENGTH);
    memset(cmd5_str, 0, MAX_LINE_LENGTH);
    cmd_argc = 0;
    cmd2_argc = 0;
    cmd3_argc = 0;
    cmd4_argc = 0;
    cmd5_argc = 0;
}

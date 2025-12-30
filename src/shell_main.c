/*
 * shell_main.c - 主Shell程序
 * 功能：实现基础Shell功能，包括命令提示符、命令解析、命令执行
 * 环境：Linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>

/* 添加gethostname所需的头文件 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64
#define HISTORY_FILE ".mini_shell_history"

/**
 * 显示动态提示符
 * 格式：用户名@主机名:当前目录$
 */
void show_prompt() {
    char hostname[256];
    char cwd[1024];
    struct passwd *pw = getpwuid(getuid());
    
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    
    // 如果在home目录，显示~
    if (pw && strncmp(cwd, pw->pw_dir, strlen(pw->pw_dir)) == 0) {
        printf("\033[1;32m%s@%s\033[0m:\033[1;34m~%s\033[0m$ ", 
               pw->pw_name, hostname, cwd + strlen(pw->pw_dir));
    } else {
        printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ", 
               pw->pw_name, hostname, cwd);
    }
    fflush(stdout);
}

/**
 * 解析命令行输入
 * 将输入字符串分割成参数数组
 */
int parse_command(char *cmd, char **args) {
    int argc = 0;
    char *token = strtok(cmd, " \t\n");
    
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    return argc;
}

/**
 * 检查文件是否可执行
 */
int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return (st.st_mode & S_IXUSR) != 0;
    }
    return 0;
}

/**
 * 检查是否为Shell脚本
 */
int is_shell_script(const char *path) {
    if (!is_executable(path)) {
        return 0;
    }
    
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    
    char first_line[256];
    if (fgets(first_line, sizeof(first_line), fp)) {
        fclose(fp);
        // 检查是否以#!/bin/bash或#!/bin/sh开头
        return (strncmp(first_line, "#!/bin/bash", 11) == 0 ||
                strncmp(first_line, "#!/bin/sh", 9) == 0);
    }
    fclose(fp);
    return 0;
}

/**
 * 将命令追加到历史文件
 */
void append_to_history(const char *cmd) {
    char history_path[1024];
    struct passwd *pw = getpwuid(getuid());
    if (!pw) return;
    
    snprintf(history_path, sizeof(history_path), "%s/%s", 
             pw->pw_dir, HISTORY_FILE);
    
    FILE *fp = fopen(history_path, "a");
    if (fp) {
        fprintf(fp, "%s\n", cmd);
        fclose(fp);
    }
}

/**
 * 执行外部C程序
 * 使用fork()创建子进程，execvp()执行命令，waitpid()等待结束
 */
int execute_c_program(char **args) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // 子进程：执行命令
        // 如果命令不包含路径分隔符，尝试在bin目录查找
        if (strchr(args[0], '/') == NULL) {
            char bin_path[1024];
            snprintf(bin_path, sizeof(bin_path), "./bin/%s", args[0]);
            if (access(bin_path, X_OK) == 0) {
                args[0] = bin_path;
            }
        }
        
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // 父进程：等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    return 0;
}

/**
 * 执行Shell脚本
 * 使用system()函数调用
 */
int execute_shell_script(char **args) {
    char cmd[MAX_CMD_LEN] = {0};
    int i = 0;
    
    // 重新组合命令字符串
    while (args[i]) {
        strcat(cmd, args[i]);
        if (args[i + 1]) {
            strcat(cmd, " ");
        }
        i++;
    }
    
    int ret = system(cmd);
    return WIFEXITED(ret) ? WEXITSTATUS(ret) : -1;
}

/**
 * 内置命令处理
 */
int handle_builtin(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        printf("退出Shell...\n");
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            // cd without args goes to home
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                chdir(pw->pw_dir);
            }
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        }
        return 1;
    } else if (strcmp(args[0], "help") == 0) {
        printf("Mini Linux Shell - 可用命令:\n");
        printf("  内置命令:\n");
        printf("    exit          - 退出Shell\n");
        printf("    cd [dir]      - 切换目录\n");
        printf("    pwd           - 显示当前目录\n");
        printf("    help          - 显示帮助信息\n");
        printf("\n  文件操作:\n");
        printf("    mytouch <file>       - 创建文件\n");
        printf("    mycat <file>         - 显示文件内容\n");
        printf("    mycp <src> <dst>     - 复制文件\n");
        printf("    myrm <file>          - 删除文件\n");
        printf("    mychmod [mode] <file> - 查看/修改文件权限\n");
        printf("\n  目录操作:\n");
        printf("    myls [-l] [dir]      - 列出目录内容\n");
        printf("\n  进程管理:\n");
        printf("    myps [-a] [-u]       - 显示进程信息\n");
        printf("    mykill <name>        - 终止进程\n");
        printf("\n  历史记录:\n");
        printf("    myhistory [-a|-n|-c] - 查看/管理命令历史\n");
        printf("\n  用户管理(Shell脚本):\n");
        printf("    ./create_user.sh     - 创建用户\n");
        printf("    ./delete_user.sh     - 删除用户\n");
        printf("    ./change_password.sh - 修改密码\n");
        return 1;
    }
    return 0;
}

/**
 * 主函数
 */
int main() {
    char cmd_line[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    
    printf("========================================\n");
    printf("  欢迎使用 Mini Linux Shell\n");
    printf("  输入 'help' 查看可用命令\n");
    printf("  输入 'exit' 退出Shell\n");
    printf("========================================\n\n");
    
    while (1) {
        show_prompt();
        
        // 读取命令
        if (fgets(cmd_line, sizeof(cmd_line), stdin) == NULL) {
            break;
        }
        
        // 移除换行符
        cmd_line[strcspn(cmd_line, "\n")] = 0;
        
        // 空命令，继续
        if (strlen(cmd_line) == 0) {
            continue;
        }
        
        // 追加到历史记录
        append_to_history(cmd_line);
        
        // 解析命令
        int argc = parse_command(cmd_line, args);
        if (argc == 0) {
            continue;
        }
        
        // 处理内置命令
        if (handle_builtin(args)) {
            continue;
        }
        
        // 判断命令类型并执行
        if (is_shell_script(args[0])) {
            // Shell脚本：使用system调用
            execute_shell_script(args);
        } else {
            // C程序或系统命令：使用fork/execvp
            int ret = execute_c_program(args);
            if (ret == -1) {
                printf("command not found: %s\n", args[0]);
            }
        }
    }
    
    return 0;
}


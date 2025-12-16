#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LENGTH 100
#define MAX_ARGS 10

// 命令函数指针类型
typedef int (*command_func)(int argc, char *argv[]);

// 命令结构体
typedef struct {
    char *name;
    command_func func;
    char *description;
} Command;

// 声明函数
void shell_init();
void shell_loop();
Command* find_command(const char *name);

#endif // SHELL_H

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
/**
 * @brief 初始化 Shell
 */
void shell_init();

/**
 * @brief 运行 Shell 主循环
 */
void shell_loop();

/**
 * @brief 查找并返回命令
 * @param name 命令名称
 * @return 找到的命令结构体指针，未找到返回 NULL
 */
Command* find_command(const char *name);

#endif // SHELL_H

#ifndef HISTORY_H
#define HISTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 命令长度常量
#define MAX_CMD_LENGTH 100

#define MAX_HISTORY 100

// 命令历史结构体
typedef struct {
    char command[MAX_CMD_LENGTH];
    int result; // 0表示成功，非0表示失败
    char timestamp[20];
} HistoryItem;

// 声明函数
/**
 * @brief 初始化历史记录
 */
void history_init();

/**
 * @brief 添加历史记录
 * @param command 命令内容
 * @param result 命令执行结果
 */
void history_add(const char *command, int result);

/**
 * @brief 显示历史记录
 */
void history_show();

/**
 * @brief 获取历史记录数量
 * @return 历史记录数量
 */
int history_size();

/**
 * @brief 获取指定索引的历史记录
 * @param index 索引
 * @return 命令内容
 */
const char* history_get_command(int index);

#endif // HISTORY_H

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
void history_init();
void history_add(const char *command, int result);
void history_show();

int history_size();
const char* history_get_command(int index);

#endif // HISTORY_H

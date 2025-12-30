#ifndef HISTORY_INTERNAL_H
#define HISTORY_INTERNAL_H

#include "history.h"
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// 共享的全局变量声明
extern HistoryItem history[MAX_HISTORY];
extern int history_count;

/**
 * @brief 获取历史记录文件路径
 * 
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return int 成功返回1，失败返回0
 */
int get_history_path(char *path, size_t size);

#endif // HISTORY_INTERNAL_H

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 声明辅助函数
/**
 * @brief 输出错误信息
 * @param message 错误内容
 */
void error(const char *message);

/**
 * @brief 输出成功信息
 * @param message 成功内容
 */
void success(const char *message);

/**
 * @brief 输出警告信息
 * @param message 警告内容
 */
void warning(const char *message);

/**
 * @brief 分割命令行为参数数组
 * @param command 命令行字符串
 * @param argv 参数数组存储位置
 * @param max_args 最大参数数量
 * @return 参数个数
 */
int split_command(const char *command, char **argv, int max_args);

/**
 * @brief 去除字符串首尾空格
 * @param str 字符串指针
 */
void trim(char *str);

#endif // UTIL_H

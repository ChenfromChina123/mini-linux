#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

/**
 * @brief 特殊键码定义
 */
#define KEY_UP    -1001
#define KEY_DOWN  -1002
#define KEY_RIGHT -1003
#define KEY_LEFT  -1004
#define KEY_BACKSPACE 127
#define KEY_ENTER 10
#define KEY_ESC 27

/**
 * @brief 启用终端原始模式
 * @param orig 存储原始终端属性
 * @return 成功返回1，失败返回0
 */
int enable_raw_mode(struct termios *orig);

/**
 * @brief 禁用终端原始模式
 * @param orig 原始终端属性
 */
void disable_raw_mode(struct termios *orig);

/**
 * @brief 读取单个键码，支持方向键
 * @return 键码值（特殊键为负数）
 */
int read_key_code();

/**
 * @brief 带行编辑功能的输入读取
 * @param prompt 提示符
 * @param buf 存储输入的缓冲区
 * @param size 缓冲区大小
 * @return 读取的字符数
 */
int read_line_with_edit(const char *prompt, char *buf, size_t size);

#endif // INPUT_H

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 声明辅助函数
void error(const char *message);
void success(const char *message);
void warning(const char *message);
int split_command(const char *command, char **argv, int max_args);
void trim(char *str);

#endif // UTIL_H

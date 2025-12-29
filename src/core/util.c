#include "util.h"
//统一错误信息格式
// 错误信息格式：\033[31mError: %s\033[0m\n
// 成功信息格式：\033[32mSuccess: %s\033[0m\n
// 警告信息格式：\033[33mWarning: %s\033[0m\n
/**
 * @brief 显示错误信息
 * @param message 错误内容
 */
void error(const char *message) {
    printf("\033[31mError: %s\033[0m\n", message);
}

/**
 * @brief 显示成功信息
 * @param message 成功内容
 */
void success(const char *message) {
    printf("\033[32mSuccess: %s\033[0m\n", message);
}

// 显示警告信息
void warning(const char *message) {
    printf("\033[33mWarning: %s\033[0m\n", message);
}

// 将命令字符串分割成命令和参数
int split_command(const char *command, char **argv, int max_args) {
    int argc = 0;
    char *cmd_copy = strdup(command);
    char *token = strtok(cmd_copy, " ");
    
    while (token != NULL && argc < max_args) {
        argv[argc++] = strdup(token);
        //strdup 函数用于复制字符串。
        //token 是当前分割出的参数。
        //argv[argc++] 是参数数组的当前位置，用于存储复制后的参数。
        //argc++ 用于增加参数计数器，指向下一个参数位置。
        token = strtok(NULL, " ");
        //strtok(NULL, " ") 用于继续分割字符串，从当前位置开始。
        //token 会更新为下一个参数，直到没有更多参数或达到最大参数数。
    }
    
    free(cmd_copy);
    return argc;
}

/**
 * @brief 去除字符串两端空白字符
 * @param str 目标字符串
 */
void trim(char *str) {
    int start = 0;
    int end = strlen(str) - 1;
    
    // 去除开头的空格和换行符
    while (start <= end && (str[start] == ' ' || str[start] == '\n' || str[start] == '\t')) {
        start++;
    }
    
    // 去除结尾的空格和换行符
    while (end >= start && (str[end] == ' ' || str[end] == '\n' || str[end] == '\t')) {
        end--;
    }
    
    // 重新构建字符串
    for (int i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[end - start + 1] = '\0';
}

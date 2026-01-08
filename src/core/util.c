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

// 将命令字符串分割成命令和参数（使用标准库函数 strspn/strcspn，最安全且代码最简洁）
int split_command(const char *command, char **argv, int max_args) {
    int argc = 0;
    const char *delimiters = " \t\n\r"; // 定义所有可能的空白分隔符
    const char *ptr = command;

    while (argc < max_args) {
        // 1. 跳过开头的空白字符（返回 ptr 中第一个不属于 delimiters 的字符索引）
        ptr += strspn(ptr, delimiters);
        
        // 如果已经到达字符串末尾，则退出
        if (*ptr == '\0') break;

        // 2. 计算当前参数的长度（返回 ptr 中第一个属于 delimiters 的字符索引）
        size_t len = strcspn(ptr, delimiters);

        // 3. 分配内存并拷贝参数
        argv[argc] = (char *)malloc(len + 1);
        if (argv[argc]) {
            memcpy(argv[argc], ptr, len);
            argv[argc][len] = '\0';
            argc++;
        }

        // 4. 将指针移动到当前参数的末尾，准备处理下一个
        ptr += len;
    }

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

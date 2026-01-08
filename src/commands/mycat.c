/*
 * mycat.c - 显示文件内容命令
 * 功能：显示指定文本文件的内容，支持重定向功能
 * 使用：mycat <filename> [> output] [>> output]
 */

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 显示文件内容
 * @param filename 输入文件路径
 * @param output 输出流
 * @return 成功返回0，失败返回-1
 */
int cat_file(const char *filename, FILE *output) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("打开文件失败");
        return -1;
    }
    
    char buffer[4096];
    //定义4K的缓存大小
    size_t bytes_read;  
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        fwrite(buffer, 1, bytes_read, output);
        //fwrite用法
    }
    //循环读取文件内容
    
    fclose(fp);
    return 0;
}

/**
 * @brief mycat 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_mycat(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: mycat <文件名> [文件名...]");
        return 1;
    }
    
    int rc = 0;
    for (int i = 1; i < argc; i++) {
        if (cat_file(argv[i], stdout) != 0) {
            rc = 1;
        }
    }
    
    return rc;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 mycat）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_mycat(argc, argv);
}
#endif

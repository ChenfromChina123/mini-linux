/*
 * mycat.c - 显示文件内容命令
 * 功能：显示指定文本文件的内容，支持重定向功能
 * 使用：mycat <filename> [> output] [>> output]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 显示文件内容
 */
int cat_file(const char *filename, FILE *output) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("打开文件失败");
        return -1;
    }
    
    char buffer[4096];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        fwrite(buffer, 1, bytes_read, output);
    }
    
    fclose(fp);
    return 0;
}

/**
 * 主函数
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <filename> [> output] [>> output]\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    FILE *output = stdout;
    
    // 检查是否有重定向
    if (argc >= 4) {
        if (strcmp(argv[2], ">") == 0) {
            // 覆盖输出
            output = fopen(argv[3], "w");
            if (!output) {
                perror("打开输出文件失败");
                return 1;
            }
        } else if (strcmp(argv[2], ">>") == 0) {
            // 追加输出
            output = fopen(argv[3], "a");
            if (!output) {
                perror("打开输出文件失败");
                return 1;
            }
        }
    }
    
    int ret = cat_file(input_file, output);
    
    if (output != stdout) {
        fclose(output);
        if (ret == 0) {
            printf("内容已输出到 '%s'\n", argv[3]);
        }
    }
    
    return ret;
}

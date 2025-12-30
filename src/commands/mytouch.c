/*
 * mytouch.c - 创建文件命令
 * 功能：创建新文件，判断是否有重名文件，提供覆盖或重命名选项
 * 使用：mytouch <filename>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * 检查文件是否存在
 */
int file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

/**
 * 创建新文件
 */
int create_file(const char *filename) {
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("创建文件失败");
        return -1;
    }
    close(fd);
    return 0;
}

/**
 * 主函数
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <filename>\n", argv[0]);
        return 1;
    }
    
    const char *filename = argv[1];
    
    // 检查文件是否已存在
    if (file_exists(filename)) {
        char choice;
        printf("文件 '%s' 已存在。是否覆盖？(y/n): ", filename);
        scanf(" %c", &choice);
        
        if (choice == 'y' || choice == 'Y') {
            // 覆盖文件
            if (create_file(filename) == 0) {
                printf("文件 '%s' 已覆盖。\n", filename);
            }
        } else {
            // 要求输入新文件名
            char new_filename[256];
            printf("请输入新文件名: ");
            scanf("%s", new_filename);
            
            if (create_file(new_filename) == 0) {
                printf("文件 '%s' 创建成功。\n", new_filename);
            }
        }
    } else {
        // 文件不存在，直接创建
        if (create_file(filename) == 0) {
            printf("文件 '%s' 创建成功。\n", filename);
        }
    }
    
    return 0;
}

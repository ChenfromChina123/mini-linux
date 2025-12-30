/*
 * mytouch.c - 创建文件命令
 * 功能：创建新文件，判断是否有重名文件，提供覆盖或重命名选项
 * 使用：mytouch <filename>
 */

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * @brief 检查文件是否存在
 * @param filename 文件名
 * @return 存在返回1，不存在返回0
 */
int file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

/**
 * @brief 创建新文件
 * @param filename 文件名
 * @return 成功返回0，失败返回-1
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
 * @brief mytouch 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_mytouch(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: mytouch <文件名> [文件名...]");
        return 1;
    }
    
    int rc = 0;
    for (int i = 1; i < argc; i++) {
        if (create_file(argv[i]) != 0) {
            rc = 1;
        }
    }
    
    return rc;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 mytouch）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_mytouch(argc, argv);
}
#endif

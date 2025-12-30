/*
 * mycp.c - 文件复制命令
 * 功能：复制文件
 * 使用：mycp <source> <destination>
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
 * @brief 复制文件
 * @param src 源文件路径
 * @param dst 目标文件路径
 * @return 成功返回0，失败返回-1
 */
int copy_file(const char *src, const char *dst) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("打开源文件失败");
        return -1;
    }
    
    // 获取源文件权限
    struct stat st;
    if (fstat(src_fd, &st) == -1) {
        perror("获取文件信息失败");
        close(src_fd);
        return -1;
    }
    
    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (dst_fd == -1) {
        perror("打开目标文件失败");
        close(src_fd);
        return -1;
    }
    
    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("写入文件失败");
            close(src_fd);
            close(dst_fd);
            return -1;
        }
    }
    
    if (bytes_read == -1) {
        perror("读取文件失败");
        close(src_fd);
        close(dst_fd);
        return -1;
    }
    
    close(src_fd);
    close(dst_fd);
    return 0;
}

/**
 * @brief mycp 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_mycp(int argc, char *argv[]) {
    if (argc < 3) {
        error("使用方法: mycp <源文件> <目标文件>");
        return 1;
    }
    
    const char *src = argv[1];
    const char *dst = argv[2];
    
    if (copy_file(src, dst) == 0) {
        printf("已复制文件: %s -> %s\n", src, dst);
        return 0;
    }
    
    return 1;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 mycp）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_mycp(argc, argv);
}
#endif

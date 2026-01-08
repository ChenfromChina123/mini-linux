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
#include <dirent.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief 复制文件
 * @param src 源文件路径
 * @param dst 目标文件路径
 * @return 成功返回0，失败返回-1
 */
int copy_file(const char *src, const char *dst) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "无法打开源文件 %s: %s\n", src, strerror(errno));
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
        fprintf(stderr, "无法创建目标文件 %s: %s\n", dst, strerror(errno));
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
 * @brief 递归复制文件夹
 * @param src 源文件夹路径
 * @param dst 目标文件夹路径
 * @return 成功返回0，失败返回-1
 */
int copy_directory(const char *src, const char *dst) {
    DIR *dir = opendir(src);
    if (!dir) {
        fprintf(stderr, "无法打开源目录 %s: %s\n", src, strerror(errno));
        return -1;
    }

    // 获取源目录权限
    struct stat st;
    if (stat(src, &st) == -1) {
        perror("获取目录信息失败");
        closedir(dir);
        return -1;
    }

    // 创建目标目录
    if (mkdir(dst, st.st_mode) == -1 && errno != EEXIST) {
        fprintf(stderr, "无法创建目标目录 %s: %s\n", dst, strerror(errno));
        closedir(dir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[PATH_MAX];
        char dst_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        struct stat entry_st;
        if (stat(src_path, &entry_st) == -1) {
            continue;
        }

        if (S_ISDIR(entry_st.st_mode)) {
            // 递归复制子目录
            copy_directory(src_path, dst_path);
        } else {
            // 复制文件
            copy_file(src_path, dst_path);
        }
    }

    closedir(dir);
    return 0;
}

/**
 * @brief mycp 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_mycp(int argc, char *argv[]) {
    int recursive = 0;
    int arg_idx = 1;

    if (argc < 3) {
        error("使用方法: mycp [-r] <源路径> <目标路径>");
        return 1;
    }

    // 简单的参数解析
    if (strcmp(argv[1], "-r") == 0) {
        recursive = 1;
        arg_idx = 2;
        if (argc < 4) {
            error("使用方法: mycp [-r] <源路径> <目标路径>");
            return 1;
        }
    }

    const char *src = argv[arg_idx];
    const char *dst = argv[arg_idx + 1];

    struct stat st;
    if (stat(src, &st) == -1) {
        error("源路径不存在");
        return 1;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!recursive) {
            error("mycp: 略过目录 (使用 -r 以递归复制)");
            return 1;
        }
        if (copy_directory(src, dst) == 0) {
            success("目录复制成功");
            return 0;
        }
    } else {
        if (copy_file(src, dst) == 0) {
            success("文件复制成功");
            return 0;
        }
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

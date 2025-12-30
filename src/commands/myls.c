/*
 * myls.c - 目录列表命令
 * 功能：显示目录内容，支持长格式显示
 * 使用：myls [-l] [directory]
 */

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

/**
 * @brief 显示文件权限字符串
 * @param mode 文件模式
 */
void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

/**
 * @brief 显示文件大小（人类可读格式）
 * @param size 文件大小
 */
void print_size(off_t size) {
    if (size < 1024) {
        printf("%5ld ", (long)size);
    } else if (size < 1024 * 1024) {
        printf("%4ldK ", (long)(size / 1024));
    } else if (size < 1024 * 1024 * 1024) {
        printf("%4ldM ", (long)(size / (1024 * 1024)));
    } else {
        printf("%4ldG ", (long)(size / (1024 * 1024 * 1024)));
    }
}

/**
 * @brief 长格式显示目录内容
 * @param dirname 目录名
 * @return 成功返回0，失败返回-1
 */
int list_directory_long(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("打开目录失败");
        return -1;
    }
    
    struct dirent *entry;
    char filepath[1024];
    
    printf("总计:\n");
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == -1) {
            continue;
        }
        
        // 权限
        print_permissions(st.st_mode);
        printf(" ");
        
        // 硬链接数
        printf("%3ld ", (long)st.st_nlink);
        
        // 所有者
        struct passwd *pw = getpwuid(st.st_uid);
        if (pw) {
            printf("%-8s ", pw->pw_name);
        } else {
            printf("%-8d ", st.st_uid);
        }
        
        // 组
        struct group *gr = getgrgid(st.st_gid);
        if (gr) {
            printf("%-8s ", gr->gr_name);
        } else {
            printf("%-8d ", st.st_gid);
        }
        
        // 大小
        print_size(st.st_size);
        
        // 修改时间
        char time_str[80];
        struct tm *tm_info = localtime(&st.st_mtime);
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
        printf("%s ", time_str);
        
        // 文件名（目录用蓝色显示）
        if (S_ISDIR(st.st_mode)) {
            printf("\033[1;34m%s\033[0m\n", entry->d_name);
        } else if (st.st_mode & S_IXUSR) {
            // 可执行文件用绿色显示
            printf("\033[1;32m%s\033[0m\n", entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dir);
    return 0;
}

/**
 * @brief 简短格式显示目录内容
 * @param dirname 目录名
 * @return 成功返回0，失败返回-1
 */
int list_directory_short(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("打开目录失败");
        return -1;
    }
    
    struct dirent *entry;
    char filepath[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == -1) {
            printf("%s  ", entry->d_name);
            continue;
        }
        
        // 目录用蓝色显示
        if (S_ISDIR(st.st_mode)) {
            printf("\033[1;34m%s\033[0m  ", entry->d_name);
        } else if (st.st_mode & S_IXUSR) {
            // 可执行文件用绿色显示
            printf("\033[1;32m%s\033[0m  ", entry->d_name);
        } else {
            printf("%s  ", entry->d_name);
        }
    }
    printf("\n");
    
    closedir(dir);
    return 0;
}

/**
 * @brief myls 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_myls(int argc, char *argv[]) {
    int long_format = 0;
    const char *dirname = ".";
    
    // 解析参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            long_format = 1;
        } else {
            dirname = argv[i];
        }
    }
    
    if (long_format) {
        return list_directory_long(dirname) == 0 ? 0 : 1;
    } else {
        return list_directory_short(dirname) == 0 ? 0 : 1;
    }
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 myls）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_myls(argc, argv);
}
#endif

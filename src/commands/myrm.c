/*
 * myrm.c - 文件/目录删除命令
 * 功能：删除文件或目录，删除前给出提示
 * 使用：myrm [-i] [-r] <file/directory>
 */

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

/**
 * @brief 递归删除目录
 * @param path 目录路径
 * @param interactive 是否交互确认
 * @return 成功返回0，失败返回-1
 */
int remove_directory(const char *path, int interactive) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("打开目录失败");
        return -1;
    }
    
    struct dirent *entry;
    char filepath[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(filepath, &st) == -1) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            //判断是否为目录
            // 递归删除子目录
            remove_directory(filepath, interactive);
        } else {
            // 删除文件
            if (interactive) {
                char choice;
                printf("删除文件 '%s'? (y/n): ", filepath);
                scanf(" %c", &choice);
                if (choice != 'y' && choice != 'Y') {
                    continue;
                }
            }
            unlink(filepath);
        }
    }
    
    closedir(dir);
    
    // 删除目录本身
    if (interactive) {
        char choice;
        printf("删除目录 '%s'? (y/n): ", path);
        scanf(" %c", &choice);
        if (choice != 'y' && choice != 'Y') {
            return 0;
        }
    }
    
    if (rmdir(path) == -1) {
        perror("删除目录失败");
        return -1;
    }
    
    return 0;
}

/**
 * @brief 删除文件
 * @param path 文件路径
 * @param interactive 是否交互确认
 * @return 成功返回0，失败返回-1
 */
int remove_file(const char *path, int interactive) {
    if (interactive) {
        char choice;
        printf("删除文件 '%s'? (y/n): ", path);
        scanf(" %c", &choice);
        if (choice != 'y' && choice != 'Y') {
            printf("已取消删除。\n");
            return 0;
        }
    }
    
    if (unlink(path) == -1) {
        perror("删除文件失败");
        return -1;
    }
    
    return 0;
}

/**
 * @brief myrm 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_myrm(int argc, char *argv[]) {
    int interactive = 0;  // -i 选项
    int recursive = 0;    // -r 选项
    const char *path = NULL;
    
    // 解析选项
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            interactive = 1;
        } else if (strcmp(argv[i], "-r") == 0) {
            recursive = 1;
        } else {
            path = argv[i];
        }
    }
    
    if (!path) {
        error("使用方法: myrm [-i] [-r] <文件/目录>");
        return 1;
    }
    
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("获取文件信息失败");
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        // 目录
        if (!recursive) {
            error("目标是目录，请使用 -r 选项递归删除");
            return 1;
        }
        return remove_directory(path, interactive) == 0 ? 0 : 1;
    } else {
        // 文件
        return remove_file(path, interactive) == 0 ? 0 : 1;
    }
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 myrm）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_myrm(argc, argv);
}
#endif

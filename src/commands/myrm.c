/*
 * myrm.c - 文件/目录删除命令
 * 功能：删除文件或目录，删除前给出提示
 * 使用：myrm [-i] [-r] <file/directory>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

/**
 * 递归删除目录
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
 * 删除文件
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
 * 主函数
 */
int main(int argc, char *argv[]) {
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
        fprintf(stderr, "用法: %s [-i] [-r] <file/directory>\n", argv[0]);
        fprintf(stderr, "  -i  交互式删除（删除前确认）\n");
        fprintf(stderr, "  -r  递归删除目录\n");
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
            fprintf(stderr, "错误: '%s' 是一个目录，请使用 -r 选项递归删除。\n", path);
            return 1;
        }
        if (remove_directory(path, interactive) == 0) {
            printf("目录 '%s' 已删除。\n", path);
        }
    } else {
        // 文件
        if (remove_file(path, interactive) == 0) {
            printf("文件 '%s' 已删除。\n", path);
        }
    }
    
    return 0;
}

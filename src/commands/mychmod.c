/*
 * mychmod.c - 文件权限修改/查看命令
 * 功能：查看或修改文件权限
 * 使用：mychmod <file>         - 查看文件权限
 *      mychmod <mode> <file>  - 修改文件权限
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * 显示文件权限
 */
void show_permissions(const char *filename) {
    struct stat st;
    
    if (stat(filename, &st) == -1) {
        perror("获取文件信息失败");
        return;
    }
    
    printf("文件: %s\n", filename);
    printf("权限: ");
    
    // 所有者权限
    printf((st.st_mode & S_IRUSR) ? "r" : "-");
    printf((st.st_mode & S_IWUSR) ? "w" : "-");
    printf((st.st_mode & S_IXUSR) ? "x" : "-");
    
    // 组权限
    printf((st.st_mode & S_IRGRP) ? "r" : "-");
    printf((st.st_mode & S_IWGRP) ? "w" : "-");
    printf((st.st_mode & S_IXGRP) ? "x" : "-");
    
    // 其他用户权限
    printf((st.st_mode & S_IROTH) ? "r" : "-");
    printf((st.st_mode & S_IWOTH) ? "w" : "-");
    printf((st.st_mode & S_IXOTH) ? "x" : "-");
    
    printf(" (%03o)\n", st.st_mode & 0777);
}

/**
 * 修改文件权限
 */
int change_permissions(const char *filename, const char *mode_str) {
    // 将八进制字符串转换为数值
    char *endptr;
    long mode = strtol(mode_str, &endptr, 8);
    
    if (*endptr != '\0' || mode < 0 || mode > 0777) {
        fprintf(stderr, "错误: 无效的权限模式 '%s'\n", mode_str);
        fprintf(stderr, "权限模式应为3位八进制数（例如：755, 644）\n");
        return -1;
    }
    
    if (chmod(filename, (mode_t)mode) == -1) {
        perror("修改文件权限失败");
        return -1;
    }
    
    return 0;
}

/**
 * 主函数
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <file>              - 查看文件权限\n", argv[0]);
        fprintf(stderr, "      %s <mode> <file>      - 修改文件权限\n", argv[0]);
        fprintf(stderr, "示例: %s 755 myfile.txt\n", argv[0]);
        return 1;
    }
    
    if (argc == 2) {
        // 查看权限
        show_permissions(argv[1]);
    } else if (argc == 3) {
        // 修改权限
        const char *mode = argv[1];
        const char *filename = argv[2];
        
        if (change_permissions(filename, mode) == 0) {
            printf("文件 '%s' 权限已修改为 %s\n", filename, mode);
            show_permissions(filename);
        }
    } else {
        fprintf(stderr, "错误: 参数过多\n");
        return 1;
    }
    
    return 0;
}


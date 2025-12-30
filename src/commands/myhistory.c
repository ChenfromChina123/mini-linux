/*
 * myhistory.c - 命令历史记录命令
 * 功能：查看和管理命令历史记录
 * 使用：myhistory [-a] [-n <count>] [-c]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#define HISTORY_FILE ".mini_shell_history"
#define MAX_HISTORY 1000

/**
 * 获取历史文件路径
 */
int get_history_path(char *path, size_t path_size) {
    struct passwd *pw = getpwuid(getuid());
    if (!pw) {
        fprintf(stderr, "无法获取用户信息\n");
        return -1;
    }
    
    snprintf(path, path_size, "%s/%s", pw->pw_dir, HISTORY_FILE);
    return 0;
}

/**
 * 显示所有历史记录（-a选项）
 */
int show_all_history() {
    char history_path[1024];
    if (get_history_path(history_path, sizeof(history_path)) == -1) {
        return -1;
    }
    
    FILE *fp = fopen(history_path, "r");
    if (!fp) {
        printf("暂无历史记录\n");
        return 0;
    }
    
    char line[1024];
    int line_num = 1;
    
    printf("命令历史记录:\n");
    printf("----------------------------------------\n");
    
    while (fgets(line, sizeof(line), fp)) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        printf("%4d  %s\n", line_num++, line);
    }
    
    fclose(fp);
    return 0;
}

/**
 * 显示最近N条历史记录（-n选项）
 */
int show_recent_history(int count) {
    char history_path[1024];
    if (get_history_path(history_path, sizeof(history_path)) == -1) {
        return -1;
    }
    
    FILE *fp = fopen(history_path, "r");
    if (!fp) {
        printf("暂无历史记录\n");
        return 0;
    }
    
    // 读取所有历史记录到数组
    char **lines = malloc(MAX_HISTORY * sizeof(char*));
    if (!lines) {
        fclose(fp);
        return -1;
    }
    
    int total_lines = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), fp) && total_lines < MAX_HISTORY) {
        line[strcspn(line, "\n")] = 0;
        lines[total_lines] = strdup(line);
        total_lines++;
    }
    
    fclose(fp);
    
    // 显示最近N条
    int start = (total_lines > count) ? (total_lines - count) : 0;
    
    printf("最近 %d 条命令历史:\n", total_lines - start);
    printf("----------------------------------------\n");
    
    for (int i = start; i < total_lines; i++) {
        printf("%4d  %s\n", i + 1, lines[i]);
        free(lines[i]);
    }
    
    // 清理前面的记录
    for (int i = 0; i < start; i++) {
        free(lines[i]);
    }
    
    free(lines);
    return 0;
}

/**
 * 清空历史记录（-c选项）
 */
int clear_history() {
    char history_path[1024];
    if (get_history_path(history_path, sizeof(history_path)) == -1) {
        return -1;
    }
    
    char choice;
    printf("确定要清空所有历史记录吗? (y/n): ");
    scanf(" %c", &choice);
    
    if (choice != 'y' && choice != 'Y') {
        printf("已取消操作\n");
        return 0;
    }
    
    FILE *fp = fopen(history_path, "w");
    if (!fp) {
        perror("清空历史记录失败");
        return -1;
    }
    
    fclose(fp);
    printf("历史记录已清空\n");
    return 0;
}

/**
 * 主函数
 */
int main(int argc, char *argv[]) {
    if (argc == 1) {
        // 无参数，默认显示所有历史
        return show_all_history();
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            // 显示所有历史
            return show_all_history();
        } else if (strcmp(argv[i], "-n") == 0) {
            // 显示最近N条
            if (i + 1 >= argc) {
                fprintf(stderr, "错误: -n 选项需要指定数量\n");
                return 1;
            }
            int count = atoi(argv[i + 1]);
            if (count <= 0) {
                fprintf(stderr, "错误: 无效的数量\n");
                return 1;
            }
            return show_recent_history(count);
        } else if (strcmp(argv[i], "-c") == 0) {
            // 清空历史
            return clear_history();
        } else {
            fprintf(stderr, "用法: %s [-a] [-n <count>] [-c]\n", argv[0]);
            fprintf(stderr, "  -a         显示所有历史记录\n");
            fprintf(stderr, "  -n <count> 显示最近N条历史记录\n");
            fprintf(stderr, "  -c         清空历史记录\n");
            return 1;
        }
    }
    
    return 0;
}


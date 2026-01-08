/*
 * mykill.c - 终止进程命令
 * 功能：通过进程名称终止进程
 * 使用：mykill <process_name>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

/**
 * 读取进程名称
 */
int get_process_name(const char *pid, char *name, size_t name_size) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/comm", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    if (fgets(name, name_size, fp)) {
        // 移除换行符
        name[strcspn(name, "\n")] = 0;
        fclose(fp);
        return 0;
    }
    
    fclose(fp);
    return -1;
}

/**
 * 根据进程名称查找并终止进程
 */
int kill_process_by_name(const char *process_name) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("打开/proc失败");
        return -1;
    }
    
    int found = 0;
    int killed = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 检查是否为数字（进程ID）
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }
        
        char name[256];
        if (get_process_name(entry->d_name, name, sizeof(name)) == -1) {
            continue;
        }
        
        // 比较进程名称
        if (strcmp(name, process_name) == 0) {
            found = 1;
            int pid = atoi(entry->d_name);
            
            // 不能终止自己
            if (pid == getpid()) {
                printf("警告: 不能终止当前进程\n");
                continue;
            }
            
            // 确认是否终止
            char choice;
            printf("发现进程: %s (PID: %d)\n", name, pid);
            printf("是否终止此进程? (y/n): ");
            scanf(" %c", &choice);
            
            if (choice == 'y' || choice == 'Y') {
                if (kill(pid, SIGTERM) == 0) {
                    //SIGTERM通常为15，用于优雅退出进程
                    printf("进程 %s (PID: %d) 已终止\n", name, pid);
                    killed++;
                } else {
                    perror("终止进程失败");
                }
            }
        }
    }
    
    closedir(dir);
    
    if (!found) {
        printf("未找到名为 '%s' 的进程\n", process_name);
        return -1;
    }
    
    if (killed > 0) {
        printf("总共终止了 %d 个进程\n", killed);
        return 0;
    }
    
    return -1;
}

/**
 * 主函数
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <process_name>\n", argv[0]);
        return 1;
    }
    
    const char *process_name = argv[1];
    
    return kill_process_by_name(process_name);
}


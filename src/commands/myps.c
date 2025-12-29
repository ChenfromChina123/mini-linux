#include "command.h"
#include "util.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
typedef DWORD pid_t;
#else
#include <sys/types.h>
#endif

// 进程信息结构体
struct ProcessInfo {
#ifdef _WIN32
    DWORD pid;
    DWORD ppid;
#else
    pid_t pid;
    pid_t ppid;
#endif
    char state;
    char comm[256];
    long rss;
    unsigned long vsize;
};

// 读取进程信息
#ifndef _WIN32
int read_process_info(pid_t pid, struct ProcessInfo *info) {
#else
int read_process_info(DWORD pid, struct ProcessInfo *info) {
    return -1; // Windows 下暂不实现
}
#endif
#ifndef _WIN32
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }
    
    char line[256];
    info->pid = pid;
    info->ppid = 0;
    info->state = '?';
    info->comm[0] = '\0';
    info->rss = 0;
    info->vsize = 0;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "Name:", 5) == 0) {
            sscanf(line + 5, "%255s", info->comm);
        } else if (strncmp(line, "State:", 6) == 0) {
            sscanf(line + 6, " %c", &info->state);
        } else if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 5, "%d", &info->ppid);
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &info->rss);
        } else if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%lu", &info->vsize);
        }
    }
    
    fclose(file);
    return 0;
}
#endif

// 显示进程列表
void display_process_list(int show_all) {
#ifdef _WIN32
    printf("抱歉，myps 命令目前仅在 Linux/WSL 环境下支持查看进程信息。\n");
    return;
#else
    // 打印表头
    printf("%-6s %-6s %-2s %-15s %-10s %-10s\n", 
           "PID", "PPID", "S", "COMMAND", "RSS (KB)", "VSIZE (KB)");
    printf("%-6s %-6s %-2s %-15s %-10s %-10s\n", 
           "------", "------", "--", "---------------", "----------", "----------");
    
    // 打开/proc目录
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        error("无法打开/proc目录");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 只处理数字目录（进程PID）
        if (entry->d_type == DT_DIR) {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0) {
                struct ProcessInfo info;
                if (read_process_info(pid, &info) == 0) {
                    // 显示进程信息
                    printf("%-6lu %-6lu %-2c %-15s %-10ld %-10lu\n", 
                           (unsigned long)info.pid, (unsigned long)info.ppid, info.state, info.comm, 
                           info.rss, info.vsize / 1024);
                }
            }
        }
    }
    
    closedir(dir);
#endif
}

// myps命令实现：显示进程信息
int cmd_myps(int argc, char *argv[]) {
    int show_all = 0;
    pid_t target_pid = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            show_all = 1;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            target_pid = (pid_t)atoi(argv[i + 1]);
            i++;
        } else {
            error("无效的命令行参数");
            printf("使用方法: myps [-a] [-p <pid>]");
            return 1;
        }
    }
    
    if (target_pid > 0) {
        // 显示指定进程信息
        struct ProcessInfo info;
        if (read_process_info(target_pid, &info) == 0) {
            printf("%-6s %-6s %-2s %-15s %-10s %-10s\n", 
                   "PID", "PPID", "S", "COMMAND", "RSS (KB)", "VSIZE (KB)");
            printf("%-6s %-6s %-2s %-15s %-10s %-10s\n", 
                   "------", "------", "--", "---------------", "----------", "----------");
            printf("%-6lu %-6lu %-2c %-15s %-10ld %-10lu\n", 
                   (unsigned long)info.pid, (unsigned long)info.ppid, info.state, info.comm, 
                   info.rss, info.vsize / 1024);
        } else {
            error("无法获取进程信息");
            return 1;
        }
    } else {
        // 显示所有进程信息
        display_process_list(show_all);
    }
    
    return 0;
}

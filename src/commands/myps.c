/*
 * myps.c - 进程信息显示命令
 * 功能：显示系统中进程信息
 * 使用：myps [-a] [-u]
 */

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

/**
 * @brief 读取进程状态信息
 * @param pid 进程号字符串
 * @param name 进程名输出缓冲区
 * @param state 进程状态输出
 * @param utime 用户态时间输出
 * @param stime 内核态时间输出
 * @param priority 优先级输出
 * @param nice nice值输出
 * @return 成功返回0，失败返回-1
 */
int read_proc_stat(const char *pid, char *name, char *state, unsigned long *utime, 
                   unsigned long *stime, long *priority, long *nice) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    int pid_num, ppid;
    fscanf(fp, "%d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %ld %ld",
           &pid_num, name, state, &ppid, utime, stime, priority, nice);
    
    fclose(fp);
    
    // 移除括号
    int len = strlen(name);
    if (len > 2 && name[0] == '(' && name[len-1] == ')') {
        memmove(name, name+1, len-2);
        name[len-2] = '\0';
    }
    
    return 0;
}

/**
 * @brief 读取进程内存信息
 * @param pid 进程号字符串
 * @param vmsize 虚拟内存大小输出（kB）
 * @param uid 用户ID输出
 * @return 成功返回0，失败返回-1
 */
int read_proc_status(const char *pid, unsigned long *vmsize, int *uid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    char line[256];
    *vmsize = 0;
    *uid = -1;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%lu", vmsize);
        } else if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line + 4, "%d", uid);
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * 显示进程信息（-a选项：显示所有进程）
 */
int show_all_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("打开/proc失败");
        return -1;
    }
    
    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", 
           "USER", "PID", "CPU%", "MEM", "COMMAND", "STATE");
    printf("----------------------------------------------------------------\n");
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 检查是否为数字（进程ID）
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }
        
        char name[256];
        char state;
        unsigned long utime, stime, vmsize;
        long priority, nice;
        int uid;
        
        if (read_proc_stat(entry->d_name, name, &state, &utime, &stime, 
                          &priority, &nice) == -1) {
            continue;
        }
        
        if (read_proc_status(entry->d_name, &vmsize, &uid) == -1) {
            continue;
        }
        
        // 获取用户名
        struct passwd *pw = getpwuid(uid);
        const char *username = pw ? pw->pw_name : "?";
        
        // 计算CPU使用率（简化版）
        double cpu_percent = (utime + stime) / 100.0;
        if (cpu_percent > 100.0) cpu_percent = 99.9;
        
        // 显示状态字符
        char state_str[8];
        snprintf(state_str, sizeof(state_str), "%c", state);
        
        printf("%-8s %-8s %5.1f %5luM %-20s %-5s\n",
               username, entry->d_name, cpu_percent, vmsize / 1024,
               name, state_str);
    }
    
    closedir(dir);
    return 0;
}

/**
 * 显示当前用户的进程（-u选项：仅显示当前用户的进程）
 */
int show_user_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("打开/proc失败");
        return -1;
    }
    
    int current_uid = getuid();
    
    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", 
           "USER", "PID", "CPU%", "MEM", "COMMAND", "STATE");
    printf("----------------------------------------------------------------\n");
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 检查是否为数字（进程ID）
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }
        
        char name[256];
        char state;
        unsigned long utime, stime, vmsize;
        long priority, nice;
        int uid;
        
        if (read_proc_stat(entry->d_name, name, &state, &utime, &stime, 
                          &priority, &nice) == -1) {
            continue;
        }
        
        if (read_proc_status(entry->d_name, &vmsize, &uid) == -1) {
            continue;
        }
        
        // 仅显示当前用户的进程
        if (uid != current_uid) {
            continue;
        }
        
        // 获取用户名
        struct passwd *pw = getpwuid(uid);
        const char *username = pw ? pw->pw_name : "?";
        
        // 计算CPU使用率（简化版）
        double cpu_percent = (utime + stime) / 100.0;
        if (cpu_percent > 100.0) cpu_percent = 99.9;
        
        // 显示状态字符
        char state_str[8];
        snprintf(state_str, sizeof(state_str), "%c", state);
        
        printf("%-8s %-8s %5.1f %5luM %-20s %-5s\n",
               username, entry->d_name, cpu_percent, vmsize / 1024,
               name, state_str);
    }
    
    closedir(dir);
    return 0;
}

/**
 * @brief 显示指定PID的进程信息（-p 选项）
 * @param pid 进程号字符串
 * @return 成功返回0，失败返回-1
 */
int show_pid_process(const char *pid) {
    char name[256];
    char state;
    unsigned long utime, stime, vmsize;
    long priority, nice;
    int uid;

    if (read_proc_stat(pid, name, &state, &utime, &stime, &priority, &nice) != 0) {
        return -1;
    }
    if (read_proc_status(pid, &vmsize, &uid) != 0) {
        return -1;
    }

    struct passwd *pw = getpwuid(uid);
    const char *username = pw ? pw->pw_name : "?";

    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", "USER", "PID", "CPU%", "MEM", "COMMAND", "STATE");
    printf("----------------------------------------------------------------\n");

    double cpu_percent = (utime + stime) / 100.0;
    if (cpu_percent > 100.0) cpu_percent = 99.9;

    char state_str[8];
    snprintf(state_str, sizeof(state_str), "%c", state);

    printf("%-8s %-8s %5.1f %5luM %-20s %-5s\n",
           username, pid, cpu_percent, vmsize / 1024, name, state_str);

    return 0;
}

/**
 * @brief myps 命令实现
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0，失败返回非0
 */
int cmd_myps(int argc, char *argv[]) {
    int show_user = 0;
    const char *pid = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0) {
            show_user = 1;
        } else if (strcmp(argv[i], "-a") == 0) {
            show_user = 0;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                error("使用方法: myps [-a] [-u] [-p <pid>]");
                return 1;
            }
            pid = argv[++i];
        } else {
            error("使用方法: myps [-a] [-u] [-p <pid>]");
            return 1;
        }
    }

    if (pid != NULL) {
        return show_pid_process(pid) == 0 ? 0 : 1;
    }
    if (show_user) {
        return show_user_processes() == 0 ? 0 : 1;
    }
    return show_all_processes() == 0 ? 0 : 1;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 myps）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_myps(argc, argv);
}
#endif

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
 //解析指定 PID 进程的核心运行状态信息
int read_proc_stat(const char *pid, char *name, char *state, unsigned long *utime, 
                   unsigned long *stime, long *priority, long *nice) {
    // 拼接/proc/<pid>/stat文件路径（缓冲区大小256足够，PID最大为4294967295）
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    
    // 打开stat文件（只读）
    FILE *fp = fopen(path, "r");
    if (!fp) {  // 打开失败（如进程已终止），直接返回错误
        return -1;
    }
    
    // 临时变量：存储PID数值、父进程PID（ppid，本函数未使用但需占位解析）
    int pid_num, ppid;
    // fscanf按格式解析stat文件：%*d/%*u表示跳过对应字段（不需要的字段）
    // 解析顺序：PID -> 进程名 -> 状态 -> 父PID -> 跳过若干字段 -> 用户态时间 -> 内核态时间 -> 跳过字段 -> 优先级 -> nice值
    fscanf(fp, "%d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %ld %ld",
           &pid_num, name, state, &ppid, utime, stime, priority, nice);
    
    fclose(fp);  // 关闭文件句柄（必须，避免资源泄漏）
    
    // 处理进程名的括号：stat文件中进程名格式为"(process_name)"，需移除首尾括号
    int len = strlen(name);
    if (len > 2 && name[0] == '(' && name[len-1] == ')') {
        // memmove：将name[1]开始的len-2个字符移动到name[0]（覆盖原括号）
        memmove(name, name+1, len-2);
        name[len-2] = '\0';  // 手动添加字符串结束符，避免乱码
    }
    
    return 0;  // 解析成功
}

/**
 * @brief 读取进程的内存和用户ID信息（从/proc/<pid>/status文件）
 * @param pid 进程号字符串
 * @param vmsize 输出参数：进程占用的虚拟内存大小（单位：KB）
 * @param uid 输出参数：进程所属用户的UID（用户ID）
 * @return 成功返回0，失败返回-1
 * 补充：/proc/<pid>/status文件为键值对格式，每行一个属性，可读性比stat更高
 */
 //从 /proc 伪文件系统中，读取指定 PID 进程的两个关键信息：进程占用的虚拟内存大小；进程所属用户的 UID（用户唯一标识）；
int read_proc_status(const char *pid, unsigned long *vmsize, int *uid) {
    // 拼接/proc/<pid>/status文件路径
    char path[256];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    char line[256];          // 临时缓冲区：存储每行读取的内容
    *vmsize = 0;             // 初始化虚拟内存大小为0
    *uid = -1;               // 初始化UID为-1（无效值）
    
    // 逐行读取status文件，直到文件末尾（fgets返回NULL）
    while (fgets(line, sizeof(line), fp)) {
        // 匹配"VmSize:"行：提取虚拟内存大小
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%lu", vmsize);  // 跳过"VmSize:"，读取数值
        } 
        // 匹配"Uid:"行：提取进程所属UID（第一列即为真实UID）
        else if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line + 4, "%d", uid);      // 跳过"Uid:"，读取UID数值
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * @brief 显示系统中所有进程信息（对应-a选项）
 * @return 成功返回0，失败返回-1（如无法打开/proc目录）
 * 核心逻辑：遍历/proc下所有以数字命名的目录（即PID目录），逐个解析进程信息并打印
 */
 //遍历 Linux 系统 /proc 目录下的所有进程（以数字命名的子目录），读取每个进程的信息
int show_all_processes() {
    // 打开/proc目录（Linux系统的伪文件系统，存储内核和进程的实时信息）
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("打开/proc失败");  // 打印具体错误原因（如权限不足）
        return -1;
    }
    
    // 打印表头：格式化输出，%-8s表示左对齐、占8个字符宽度
    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", 
           "USER",   // 进程所属用户
           "PID",    // 进程ID
           "CPU%",   // CPU使用率（简化计算）
           "MEM",    // 虚拟内存大小（MB）
           "COMMAND",// 进程名
           "STATE"); // 进程状态
    printf("----------------------------------------------------------------\n");  // 分隔线
    
    struct dirent *entry;  // 目录项结构体：存储遍历到的目录/文件信息
    // 遍历/proc目录下的所有项，直到readdir返回NULL（遍历结束）
    while ((entry = readdir(dir)) != NULL) {
        // 过滤非PID目录：仅处理以数字开头的目录（PID为纯数字）
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }
        
        // 定义变量存储当前进程的解析结果
        char name[256];                // 进程名
        char state;                    // 进程状态
        unsigned long utime, stime;    // 用户态/内核态CPU时间
        unsigned long vmsize;          // 虚拟内存大小（KB）
        long priority, nice;           // 优先级/nice值
        int uid;                       // 进程所属UID
        
        // 读取进程stat信息，失败则跳过该进程（如进程已退出）
        if (read_proc_stat(entry->d_name, name, &state, &utime, &stime, 
                          &priority, &nice) == -1) {
            continue;
        }
        
        // 读取进程status信息，失败则跳过
        if (read_proc_status(entry->d_name, &vmsize, &uid) == -1) {
            continue;
        }
        
        // 通过UID获取用户名：getpwuid返回passwd结构体，包含用户名等信息
        struct passwd *pw = getpwuid(uid);
        const char *username = pw ? pw->pw_name : "?";  // 无对应用户则显示?
        
        // 简化版CPU使用率计算：(用户态+内核态时间)/100（jiffies转百分比，真实ps会结合系统总时间计算）
        double cpu_percent = (utime + stime) / 100.0;
        if (cpu_percent > 100.0) cpu_percent = 99.9;  // 限制最大值，避免显示异常
        
        // 进程状态转为字符串（单字符转字符串，方便格式化输出）
        char state_str[8];
        snprintf(state_str, sizeof(state_str), "%c", state);
        
        // 格式化打印进程信息：MEM列转换为MB（KB/1024）
        printf("%-8s %-8s %5.1f %5luM %-20s %-5s\n",
               username,        // 用户名
               entry->d_name,   // PID字符串
               cpu_percent,     // CPU使用率（保留1位小数）
               vmsize / 1024,   // 虚拟内存（MB）
               name,            // 进程名
               state_str);      // 进程状态
    }
    
    closedir(dir);  // 关闭目录句柄（避免资源泄漏）
    return 0;
}

/**
 * @brief 仅显示当前用户的进程信息（对应-u选项）
 * @return 成功返回0，失败返回-1
 * 核心逻辑：与show_all_processes一致，仅增加UID匹配（当前用户UID）
 */
//遍历 /proc 目录下的所有 PID 目录，仅筛选出当前用户（运行 myps 的用户）所属的进程,读取信息,实现类似 ps -u 的核心功能
int show_user_processes() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("打开/proc失败");
        return -1;
    }
    
    // 获取当前用户的UID（运行myps程序的用户）
    int current_uid = getuid();
    
    // 打印表头（与show_all_processes一致）
    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", 
           "USER", "PID", "CPU%", "MEM", "COMMAND", "STATE");
    printf("----------------------------------------------------------------\n");
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 过滤非PID目录
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }
        
        // 定义变量存储进程信息（与show_all_processes一致）
        char name[256];
        char state;
        unsigned long utime, stime, vmsize;
        long priority, nice;
        int uid;
        
        // 读取stat信息，失败则跳过
        if (read_proc_stat(entry->d_name, name, &state, &utime, &stime, 
                          &priority, &nice) == -1) {
            continue;
        }
        
        // 读取status信息，失败则跳过
        if (read_proc_status(entry->d_name, &vmsize, &uid) == -1) {
            continue;
        }
        
        // 核心过滤逻辑：仅显示当前用户的进程（UID匹配）
        if (uid != current_uid) {
            continue;
        }
        
        // 以下逻辑与show_all_processes一致
        struct passwd *pw = getpwuid(uid);
        const char *username = pw ? pw->pw_name : "?";
        
        double cpu_percent = (utime + stime) / 100.0;
        if (cpu_percent > 100.0) cpu_percent = 99.9;
        
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
 * @brief 显示指定PID的进程信息（对应-p <pid>选项）
 * @param pid 进程号字符串（如"1"表示init进程）
 * @return 成功返回0，失败返回-1（如PID不存在）
 * 核心逻辑：直接解析指定PID的信息，无需遍历所有进程
 */
 //查找指定PID进程的信息
int show_pid_process(const char *pid) {
    // 定义变量存储进程信息
    char name[256];
    char state;
    unsigned long utime, stime, vmsize;
    long priority, nice;
    int uid;

    // 读取stat信息，失败则返回错误
    if (read_proc_stat(pid, name, &state, &utime, &stime, &priority, &nice) != 0) {
        return -1;
    }
    // 读取status信息，失败则返回错误
    if (read_proc_status(pid, &vmsize, &uid) != 0) {
        return -1;
    }

    // 获取用户名
    struct passwd *pw = getpwuid(uid);
    const char *username = pw ? pw->pw_name : "?";

    // 打印表头
    printf("%-8s %-8s %-5s %-6s %-20s %-5s\n", "USER", "PID", "CPU%", "MEM", "COMMAND", "STATE");
    printf("----------------------------------------------------------------\n");

    // 计算CPU使用率（简化版）
    double cpu_percent = (utime + stime) / 100.0;
    if (cpu_percent > 100.0) cpu_percent = 99.9;

    // 进程状态转字符串
    char state_str[8];
    snprintf(state_str, sizeof(state_str), "%c", state);

    // 打印指定PID的进程信息
    printf("%-8s %-8s %5.1f %5luM %-20s %-5s\n",
           username, pid, cpu_percent, vmsize / 1024, name, state_str);

    return 0;
}

/**
 * @brief myps命令的核心入口：解析命令行参数，调用对应功能函数
 * @param argc 参数个数（main函数传入）
 * @param argv 参数数组（main函数传入，argv[0]为程序名）
 * @return 成功返回0，失败返回非0（如参数错误）
 * 参数解析逻辑：
 *   -a：显示所有进程（默认）
 *   -u：仅显示当前用户进程
 *   -p <pid>：显示指定PID进程（需跟PID参数）
 */
 //解析命令行参数（argc/argv），判断用户的操作意图（显示所有进程 / 当前用户进程 / 指定 PID 进程）
int cmd_myps(int argc, char *argv[]) {
    int show_user = 0;       // 标记：是否仅显示当前用户进程（0=否，1=是）
    const char *pid = NULL;  // 存储指定的PID（-p选项）

    // 遍历命令行参数（从argv[1]开始，argv[0]是程序名）
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0) {
            show_user = 1;  // 设置为仅显示当前用户进程
        } else if (strcmp(argv[i], "-a") == 0) {
            show_user = 0;  // 重置为显示所有进程（-a优先级高于-u）
        } else if (strcmp(argv[i], "-p") == 0) {
            // 检查-p后是否有PID参数（避免数组越界）
            if (i + 1 >= argc) {
                error("使用方法: myps [-a] [-u] [-p <pid>]");  // 自定义错误打印函数
                return 1;
            }
            pid = argv[++i];  // 读取-p后的PID参数（i自增，跳过已处理的PID）
        } else {
            // 未知参数，打印用法并退出
            error("使用方法: myps [-a] [-u] [-p <pid>]");
            return 1;
        }
    }

    // 优先级：-p > -u/-a
    if (pid != NULL) {
        // 显示指定PID进程，成功返回0，失败返回1
        return show_pid_process(pid) == 0 ? 0 : 1;
    }
    // 无-p时，根据show_user标记选择显示当前用户/所有进程
    if (show_user) {
        return show_user_processes() == 0 ? 0 : 1;
    }
    // 默认显示所有进程
    return show_all_processes() == 0 ? 0 : 1;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（仅当定义MINI_LINUX_STANDALONE时编译）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码（0=成功，非0=失败）
 * 作用：让myps可以单独编译为可执行文件（而非作为命令模块）
 */
int main(int argc, char *argv[]) {
    // 调用cmd_myps处理参数和逻辑
    return cmd_myps(argc, argv);
}
#endif

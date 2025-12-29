#include "history.h"

#include <limits.h>
#include <errno.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// 全局变量定义
HistoryItem history[MAX_HISTORY];
int history_count = 0;

/**
 * @brief 初始化命令历史记录
 * 从用户主目录加载持久化的历史记录
 */
void history_init() {
    history_count = 0;

    // 从用户主目录的 .mini_history 文件加载历史（如果存在）
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return;

    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s/.mini_history", home) >= (int)sizeof(path)) return;

    FILE *f = fopen(path, "r");
    if (f == NULL) return;

    // 读取所有行，保留最后 MAX_HISTORY 条
    char line[1024];
    // 使用临时数组保存读取的条目计数可能超过 MAX_HISTORY
    HistoryItem tmp_items[1024];
    int tmp_count = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        // 每行格式：timestamp\tresult\tcommand\n
        char *p = line;
        // 去除末尾换行
        size_t L = strlen(p);
        if (L > 0 && (p[L-1] == '\n' || p[L-1] == '\r')) p[L-1] = '\0';

        char *ts = strtok(p, "\t");
        char *res = ts ? strtok(NULL, "\t") : NULL;
        char *cmd = res ? strtok(NULL, "\t") : NULL;
        if (ts && res && cmd) {
            if (tmp_count < (int)(sizeof(tmp_items)/sizeof(tmp_items[0]))) {
                strncpy(tmp_items[tmp_count].timestamp, ts, sizeof(tmp_items[tmp_count].timestamp)-1);
                tmp_items[tmp_count].timestamp[sizeof(tmp_items[tmp_count].timestamp)-1] = '\0';
                tmp_items[tmp_count].result = atoi(res);
                strncpy(tmp_items[tmp_count].command, cmd, sizeof(tmp_items[tmp_count].command)-1);
                tmp_items[tmp_count].command[sizeof(tmp_items[tmp_count].command)-1] = '\0';
                tmp_count++;
            }
        }
    }
    fclose(f);

    // 取最后 MAX_HISTORY 条到 history 数组
    int start = tmp_count > MAX_HISTORY ? tmp_count - MAX_HISTORY : 0;
    for (int i = start; i < tmp_count; i++) {
        int idx = i - start;
        history[idx] = tmp_items[i];
    }
    history_count = tmp_count - start;
}

// 添加命令到历史记录
void history_add(const char *command, int result) {
    // 获取当前时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    
    // 添加到历史记录
    if (history_count < MAX_HISTORY) {
        // 历史记录未满，直接添加到末尾
        strcpy(history[history_count].command, command);
        history[history_count].result = result;
        strcpy(history[history_count].timestamp, timestamp);
        history_count++;
    } else {
        // 历史记录已满，移除最旧的记录，将其他记录向前移动
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        // 添加新记录到末尾
        strcpy(history[MAX_HISTORY - 1].command, command);
        history[MAX_HISTORY - 1].result = result;
        strcpy(history[MAX_HISTORY - 1].timestamp, timestamp);
    }

    // 追加到历史文件
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home != NULL) {
        char path[PATH_MAX];
        if (snprintf(path, sizeof(path), "%s/.mini_history", home) < (int)sizeof(path)) {
            FILE *f = fopen(path, "a");
            if (f != NULL) {
                fprintf(f, "%s\t%d\t%s\n", timestamp, result, command);
                fclose(f);
            }
        }
    }
}

/**
 * @brief 显示命令历史记录
 */
void history_show() {
    printf("\033[34m命令历史记录：\033[0m\n");
    printf("%-5s %-20s %-30s %s\n", "序号", "时间", "命令", "结果");
    printf("%s\n", "------------------------------------------------------------------");
    
    for (int i = 0; i < history_count; i++) {
        const char *result_str = history[i].result == 0 ? "成功" : "失败";
        printf("%-5d %-20s %-30s %s\n", i + 1, history[i].timestamp, history[i].command, result_str);
    }
}

int history_size() { return history_count; }
const char* history_get_command(int index) { if (index < 0 || index >= history_count) return NULL; return history[index].command; }

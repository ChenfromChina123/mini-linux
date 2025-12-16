#include "history.h"

// 全局变量定义
HistoryItem history[MAX_HISTORY];
int history_count = 0;

// 初始化命令历史记录
void history_init() {
    history_count = 0;
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
}

// 显示命令历史记录
void history_show() {
    printf("\033[34m命令历史记录：\033[0m\n");
    printf("%-5s %-20s %-30s %s\n", "序号", "时间", "命令", "结果");
    printf("%s\n", "------------------------------------------------------------------");
    
    for (int i = 0; i < history_count; i++) {
        const char *result_str = history[i].result == 0 ? "成功" : "失败";
        printf("%-5d %-20s %-30s %s\n", i + 1, history[i].timestamp, history[i].command, result_str);
    }
}

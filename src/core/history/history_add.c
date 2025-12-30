#include "history_internal.h"

/**
 * @brief 添加命令到历史记录
 * 
 * @param command 命令内容
 * @param result 命令执行结果
 */
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
    char path[PATH_MAX];
    if (get_history_path(path, sizeof(path))) {
        FILE *f = fopen(path, "a");
        if (f != NULL) {
            fprintf(f, "%s\t%d\t%s\n", timestamp, result, command);
            fclose(f);
        }
    }
}

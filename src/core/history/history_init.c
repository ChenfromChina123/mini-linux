#include "history_internal.h"

/**
 * @brief 初始化命令历史记录
 * 从用户主目录加载持久化的历史记录
 */
void history_init() {
    history_count = 0;

    char path[PATH_MAX];
    if (!get_history_path(path, sizeof(path))) return;

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

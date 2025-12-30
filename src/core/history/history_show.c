#include "history_internal.h"

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

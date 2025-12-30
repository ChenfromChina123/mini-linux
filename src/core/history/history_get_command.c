#include "history_internal.h"

/**
 * @brief 根据索引获取历史命令
 * 
 * @param index 历史记录索引
 * @return const char* 命令字符串，如果索引无效则返回 NULL
 */
const char* history_get_command(int index) { 
    if (index < 0 || index >= history_count) return NULL; 
    return history[index].command; 
}

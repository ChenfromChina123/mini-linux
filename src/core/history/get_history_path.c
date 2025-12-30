#include "history_internal.h"

/**
 * @brief 获取历史记录文件路径
 * 
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return int 成功返回1，失败返回0
 */
int get_history_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;

    if (snprintf(path, size, "%s/.mini_history", home) >= (int)size) return 0;
    return 1;
}

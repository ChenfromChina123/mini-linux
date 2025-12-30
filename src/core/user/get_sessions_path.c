#include "user_internal.h"

/**
 * 获取会话跟踪文件路径
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return 成功返回1，失败返回0
 */
int get_sessions_path(char *path, size_t size) {
    const char *home = getenv("HOME");
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_sessions", home) < (int)size;
}

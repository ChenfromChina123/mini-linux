#include "user_internal.h"

/**
 * 获取用户数据库文件路径
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return 成功返回1，失败返回0
 */
int get_user_db_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_users", home) < (int)size;
}

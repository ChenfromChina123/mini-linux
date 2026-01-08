#include "user_internal.h"

/**
 * @brief 重新加载用户数据
 * @details 从用户数据文件重新加载用户列表，用于在外部脚本修改用户后同步内存数据
 */
void user_reload() {
    load_users();
}


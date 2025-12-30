#include "user_internal.h"

/**
 * 初始化用户子系统
 */
void user_init() {
    load_users();
    strcpy(current_user.username, "");
    current_user.is_root = 0;
}

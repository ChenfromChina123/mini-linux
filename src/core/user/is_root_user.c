#include "user_internal.h"

/**
 * 判断当前登录用户是否为管理员
 * @return 是返回1，否则返回0
 */
int is_root_user() {
    return current_user.is_root;
}

#include "user_internal.h"

/**
 * 获取当前登录用户信息
 */
const User* get_current_user() { return &current_user; }

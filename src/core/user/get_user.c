#include "user_internal.h"

/**
 * 获取指定索引的用户信息
 */
const User* get_user(int index) { if (index < 0 || index >= user_count) return NULL; return &users[index]; }

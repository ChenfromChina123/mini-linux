#include "user_internal.h"

/**
 * 用户登录验证
 * @param username 用户名
 * @param password 密码
 * @return 登录成功返回1，失败返回0
 */
int user_login(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            // 匹配成功，更新全局当前用户信息
            strcpy(current_user.username, users[i].username);
            strcpy(current_user.password, users[i].password);
            current_user.is_root = users[i].is_root;
            return 1;
        }
    }
    return 0; // 验证失败
}

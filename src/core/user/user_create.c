#include "user_internal.h"

/**
 * 创建新用户
 * @param username 用户名
 * @param password 密码
 * @param is_root 是否为管理员
 * @return 成功返回1，失败返回0
 */
int user_create(const char *username, const char *password, int is_root) {
    // 检查数组上限
    if (user_count >= MAX_USERS) {
        error("用户数量已达上限");
        return 0;
    }
    
    // 唯一性检查
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            error("用户名已存在");
            return 0;
        }
    }
    
    // 写入全局数组并同步到文件
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].is_root = is_root;
    user_count++;
    save_users();
    
    return 1;
}

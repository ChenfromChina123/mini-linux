#include "user_internal.h"

/**
 * 修改用户密码
 * @param username 用户名
 * @param old_password 旧密码（非强制模式需提供）
 * @param new_password 新密码
 * @param force 是否强制修改（管理员修改他人密码时设为1）
 * @return 成功返回1，失败返回0
 */
int user_change_password(const char *username, const char *old_password, const char *new_password, int force) {
    if (!username || !new_password) { error("参数无效"); return 0; }
    
    // 查找用户
    int idx = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) { idx = i; break; }
    }
    
    if (idx < 0) { error("用户不存在"); return 0; }
    
    // 权限与密码验证
    if (!force) {
        if (!old_password || strcmp(users[idx].password, old_password) != 0) { error("旧密码错误"); return 0; }
    }
    
    // 长度检查
    size_t L = strlen(new_password);
    if (L == 0 || L >= MAX_PASSWORD_LENGTH) { error("新密码长度非法"); return 0; }
    
    // 更新并同步
    strncpy(users[idx].password, new_password, MAX_PASSWORD_LENGTH - 1);
    users[idx].password[MAX_PASSWORD_LENGTH - 1] = '\0';
    save_users();
    success("密码已更新");
    return 1;
}

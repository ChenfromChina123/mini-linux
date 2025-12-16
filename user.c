#include "user.h"
#include "util.h"

// 全局变量定义
User users[MAX_USERS];
int user_count = 0;
User current_user;

// 初始化用户系统，添加默认用户
void user_init() {
    // 添加root用户
    strcpy(users[user_count].username, "root");
    strcpy(users[user_count].password, "root");
    users[user_count].is_root = 1;
    user_count++;
    
    // 添加普通用户
    strcpy(users[user_count].username, "user");
    strcpy(users[user_count].password, "user");
    users[user_count].is_root = 0;
    user_count++;
    
    // 初始化当前用户
    strcpy(current_user.username, "");
    current_user.is_root = 0;
}

// 用户登录
int user_login(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            // 登录成功，更新当前用户
            strcpy(current_user.username, users[i].username);
            strcpy(current_user.password, users[i].password);
            current_user.is_root = users[i].is_root;
            return 1;
        }
    }
    return 0; // 登录失败
}

// 创建用户
int user_create(const char *username, const char *password, int is_root) {
    // 检查是否有足够空间
    if (user_count >= MAX_USERS) {
        error("用户数量已达上限");
        return 0;
    }
    
    // 检查用户名是否已存在
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            error("用户名已存在");
            return 0;
        }
    }
    
    // 创建新用户
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].is_root = is_root;
    user_count++;
    
    return 1;
}

// 删除用户
int user_delete(const char *username) {
    // 不能删除root用户
    if (strcmp(username, "root") == 0) {
        error("不能删除root用户");
        return 0;
    }
    
    // 查找要删除的用户
    int index = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        error("用户不存在");
        return 0;
    }
    
    // 删除用户，后面的用户向前移动
    for (int i = index; i < user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    user_count--;
    
    return 1;
}

// 检查当前用户是否为root用户
int is_root_user() {
    return current_user.is_root;
}

int get_user_count() { return user_count; }
const User* get_user(int index) { if (index < 0 || index >= user_count) return NULL; return &users[index]; }
const User* get_current_user() { return &current_user; }
void user_list_all() {
    printf("\033[34m所有用户：\033[0m\n");
    printf("%-4s %-20s %-6s\n", "序号", "用户名", "类型");
    for (int i = 0; i < user_count; i++) {
        printf("%-4d %-20s %-6s\n", i + 1, users[i].username, users[i].is_root ? "root" : "user");
    }
}
void user_list_active() {
    printf("\033[34m活跃用户：\033[0m\n");
    if (current_user.username[0] == '\0') {
        printf("无\n");
    } else {
        printf("%s\n", current_user.username);
    }
}

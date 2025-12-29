#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 10
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

// 用户结构体
typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int is_root; // 1表示root用户，0表示普通用户
} User;

// 当前登录用户
extern User current_user;

// 声明函数
/**
 * @brief 用户登录
 * @param username 用户名
 * @param password 密码
 * @return 登录成功返回1，失败返回0
 */
int user_login(const char *username, const char *password);

/**
 * @brief 创建用户
 * @param username 用户名
 * @param password 密码
 * @param is_root 是否为管理员
 * @return 创建成功返回1，失败返回0
 */
int user_create(const char *username, const char *password, int is_root);

/**
 * @brief 删除用户
 * @param username 用户名
 * @return 删除成功返回1，失败返回0
 */
int user_delete(const char *username);

/**
 * @brief 检查当前用户是否为管理员
 * @return 是管理员返回1，否则返回0
 */
int is_root_user();

/**
 * @brief 初始化用户管理
 */
void user_init();

/**
 * @brief 获取用户总数
 * @return 用户总数
 */
int get_user_count();

/**
 * @brief 获取指定索引的用户
 * @param index 索引
 * @return 用户结构体指针
 */
const User* get_user(int index);

/**
 * @brief 获取当前登录用户
 * @return 当前用户结构体指针
 */
const User* get_current_user();

/**
 * @brief 列出所有用户
 */
void user_list_all();

/**
 * @brief 列出当前活跃用户
 */
void user_list_active();

/**
 * @brief 注册用户会话
 */
void user_session_register();

/**
 * @brief 注销用户会话
 */
void user_session_unregister();

/**
 * @brief 修改用户密码
 * @param username 用户名
 * @param old_password 旧密码
 * @param new_password 新密码
 * @param force 是否强制修改
 * @return 修改成功返回1，失败返回0
 */
int user_change_password(const char *username, const char *old_password, const char *new_password, int force);

#endif // USER_H

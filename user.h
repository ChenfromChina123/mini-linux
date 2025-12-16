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
int user_login(const char *username, const char *password);
int user_create(const char *username, const char *password, int is_root);
int user_delete(const char *username);
int is_root_user();
void user_init();

int get_user_count();
const User* get_user(int index);
const User* get_current_user();
void user_list_all();
void user_list_active();

#endif // USER_H

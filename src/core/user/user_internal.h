#ifndef USER_INTERNAL_H
#define USER_INTERNAL_H

#include "user.h"
#include "util.h"
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <process.h>
#define GETPID() _getpid()
#else
#define GETPID() getpid()
#endif

// 最大用户数
#define MAX_USERS 1024

// 全局变量声明
extern User users[MAX_USERS];
extern int user_count;
extern User current_user;

// 内部辅助函数声明
int get_user_db_path(char *path, size_t size);
int get_sessions_path(char *path, size_t size);
void save_users();
void load_users();

#endif // USER_INTERNAL_H

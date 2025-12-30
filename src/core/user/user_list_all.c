#include "user_internal.h"

/**
 * 列出系统中所有的注册用户
 */
void user_list_all() {
    printf("\033[34m所有用户：\033[0m\n");
    printf("%-4s %-20s %-6s\n", "序号", "用户名", "类型");
    for (int i = 0; i < user_count; i++) {
        printf("%-4d %-20s %-6s\n", i + 1, users[i].username, users[i].is_root ? "root" : "user");
    }
}

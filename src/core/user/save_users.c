#include "user_internal.h"

/**
 * 将当前内存中的用户列表保存到持久化文件
 */
void save_users() {
    char p[PATH_MAX];
    if (!get_user_db_path(p, sizeof(p))) return;
    FILE *f = fopen(p, "w");
    if (!f) return;
    for (int i = 0; i < user_count; i++) fprintf(f, "%s\t%s\t%d\n", users[i].username, users[i].password, users[i].is_root);
    fclose(f);
}

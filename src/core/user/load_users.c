#include "user_internal.h"

/**
 * 从持久化文件加载用户列表，如果文件不存在则初始化默认用户
 */
void load_users() {
    char p[PATH_MAX];
    if (!get_user_db_path(p, sizeof(p))) {
        user_count = 0;
        // 如果无法获取路径，创建内存默认用户
        strcpy(users[user_count].username, "root");
        strcpy(users[user_count].password, "root");
        users[user_count].is_root = 1;
        user_count++;
        
        strcpy(users[user_count].username, "user");
        strcpy(users[user_count].password, "user");
        users[user_count].is_root = 0;
        user_count++;
        return;
    }
    FILE *f = fopen(p, "r");
    if (!f) {
        // 文件不存在，初始化默认用户并保存
        user_count = 0;
        strcpy(users[user_count].username, "root");
        strcpy(users[user_count].password, "root");
        users[user_count].is_root = 1;
        user_count++;
        strcpy(users[user_count].username, "user");
        strcpy(users[user_count].password, "user");
        users[user_count].is_root = 0;
        user_count++;
        save_users();
        return;
    }
    user_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        size_t L = strlen(line);
        if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
        char *u = strtok(line, "\t");
        char *pw = u ? strtok(NULL, "\t") : NULL;
        char *ir = pw ? strtok(NULL, "\t") : NULL;
        if (u && pw && ir) {
            if (user_count < MAX_USERS) {
                // 复制用户信息到全局数组
                strncpy(users[user_count].username, u, sizeof(users[user_count].username)-1);
                users[user_count].username[sizeof(users[user_count].username)-1] = '\0';
                strncpy(users[user_count].password, pw, sizeof(users[user_count].password)-1);
                users[user_count].password[sizeof(users[user_count].password)-1] = '\0';
                users[user_count].is_root = atoi(ir) ? 1 : 0;
                user_count++;
            }
        }
    }
    fclose(f);
    
    // 如果加载后依然没有用户，则创建
    if (user_count == 0) {
        strcpy(users[user_count].username, "root");
        strcpy(users[user_count].password, "root");
        users[user_count].is_root = 1;
        user_count++;
        strcpy(users[user_count].username, "user");
        strcpy(users[user_count].password, "user");
        users[user_count].is_root = 0;
        user_count++;
        save_users();
    }
}

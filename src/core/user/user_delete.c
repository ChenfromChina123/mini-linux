#include "user_internal.h"

/**
 * 删除用户。
 * 包含对会话记录的同步清理。
 * @param username 要删除的用户名
 * @return 成功返回1，失败返回0
 */
int user_delete(const char *username) {
    // 安全检查：root用户不可删除
    if (strcmp(username, "root") == 0) {
        error("不能删除root用户");
        return 0;
    }
    
    // 1. 查找用户在数组中的位置
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
    
    // 2. 从全局数组中移除（后续元素前移）
    for (int i = index; i < user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    user_count--;
    save_users(); // 同步到用户数据库
    
    // 3. 同步清理该用户的活跃会话记录，防止 ghost 会话
    char p[PATH_MAX];
    if (get_sessions_path(p, sizeof(p))) {
        typedef struct { char u[MAX_USERNAME_LENGTH]; int pid; } E;
        E entries[256]; int cnt = 0;
        FILE *fin = fopen(p, "r");
        if (fin) {
            char line[256];
            while (fgets(line, sizeof(line), fin)) {
                size_t L = strlen(line);
                if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
                char *u = strtok(line, "\t");
                char *pp = u ? strtok(NULL, "\t") : NULL;
                if (u && pp) {
                    // 仅保留非目标用户的会话
                    if (strcmp(u, username) != 0 && cnt < 256) {
                        strncpy(entries[cnt].u, u, sizeof(entries[cnt].u)-1);
                        entries[cnt].u[sizeof(entries[cnt].u)-1] = '\0';
                        entries[cnt].pid = atoi(pp);
                        cnt++;
                    }
                }
            }
            fclose(fin);
            
            // 写回更新后的会话文件
            FILE *fout = fopen(p, "w");
            if (fout) {
                for (int i = 0; i < cnt; i++) fprintf(fout, "%s\t%d\n", entries[i].u, entries[i].pid);
                fclose(fout);
            }
        }
    }
    
    return 1;
}

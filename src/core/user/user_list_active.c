#include "user_internal.h"

/**
 * 列出当前活跃（已登录）的用户。
 * 会校验会话文件中的用户是否依然存在于用户数据库中。
 */
void user_list_active() {
    printf("\033[34m活跃用户：\033[0m\n");
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) { 
        if (current_user.username[0] == '\0') printf("无\n"); else printf("%s\n", current_user.username); 
        return; 
    }
    
    FILE *f = fopen(p, "r");
    if (!f) { 
        if (current_user.username[0] == '\0') printf("无\n"); else printf("%s\n", current_user.username); 
        return; 
    }
    
    char seen[MAX_USERS][MAX_USERNAME_LENGTH]; int sc = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        size_t L = strlen(line);
        if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
        char *u = strtok(line, "\t");
        if (u) {
            // 1. 存在性验证：检查该活跃用户是否依然存在于系统中
            int exists = 0;
            for (int i = 0; i < user_count; i++) {
                if (strcmp(users[i].username, u) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) continue;

            // 2. 去重显示（同一用户可能有多个终端会话）
            int ok = 1; 
            for (int i = 0; i < sc; i++) if (strcmp(seen[i], u) == 0) { ok = 0; break; }
            if (ok && sc < MAX_USERS) { 
                strncpy(seen[sc], u, sizeof(seen[0]) - 1); 
                seen[sc][sizeof(seen[0]) - 1] = '\0'; 
                sc++; 
            }
        }
    }
    fclose(f);
    
    if (sc == 0) printf("无\n"); 
    else for (int i = 0; i < sc; i++) printf("%s\n", seen[i]);
}

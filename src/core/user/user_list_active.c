#include "user_internal.h"
#include <signal.h>

/**
 * 列出当前活跃（已登录）的用户。
 * 会校验会话文件中的用户是否依然存在于用户数据库中，
 * 并且检查对应的进程 PID 是否依然存活。
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
    
    // 用于存储有效的会话，以便后续清理过期会话
    typedef struct { char u[MAX_USERNAME_LENGTH]; int pid; } SessionEntry;
    SessionEntry valid_sessions[256];
    int valid_count = 0;

    while (fgets(line, sizeof(line), f)) {
        size_t L = strlen(line);
        if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
        
        char *u = strtok(line, "\t");
        char *p_str = strtok(NULL, "\t");
        
        if (u && p_str) {
            int pid = atoi(p_str);
            
            // 1. 存在性验证：检查该活跃用户是否依然存在于系统中
            int exists = 0;
            for (int i = 0; i < user_count; i++) {
                if (strcmp(users[i].username, u) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists) continue;

            // 2. 进程存活验证：检查 PID 是否依然运行
            // kill(pid, 0) 如果返回 0 表示进程存在，或者有权限发送信号
            // 如果返回 -1 且 errno 为 ESRCH，表示进程不存在
            if (kill(pid, 0) == -1 && errno == ESRCH) {
                continue;
            }

            // 记录有效会话
            if (valid_count < 256) {
                strncpy(valid_sessions[valid_count].u, u, MAX_USERNAME_LENGTH - 1);
                valid_sessions[valid_count].u[MAX_USERNAME_LENGTH - 1] = '\0';
                valid_sessions[valid_count].pid = pid;
                valid_count++;
            }

            // 3. 去重显示（同一用户可能有多个终端会话）
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

    // 自动清理：将有效的会话写回文件，移除已失效的 PID 记录
    FILE *fout = fopen(p, "w");
    if (fout) {
        for (int i = 0; i < valid_count; i++) {
            fprintf(fout, "%s\t%d\n", valid_sessions[i].u, valid_sessions[i].pid);
        }
        fclose(fout);
    }
    
    if (sc == 0) printf("无\n"); 
    else for (int i = 0; i < sc; i++) printf("%s\n", seen[i]);
}

#include "user_internal.h"

/**
 * 注册当前会话。
 * 将当前登录的用户名及其进程PID记录到会话文件中。
 */
void user_session_register() {
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) return;
    typedef struct { char u[MAX_USERNAME_LENGTH]; int pid; } E;
    E entries[256]; int cnt = 0;
    
    // 1. 读取现有会话
    FILE *fin = fopen(p, "r");
    if (fin) {
        char line[256];
        while (fgets(line, sizeof(line), fin)) {
            size_t L = strlen(line);
            if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
            char *u = strtok(line, "\t");
            char *pp = u ? strtok(NULL, "\t") : NULL;
            if (u && pp) {
                if (cnt < (int)(sizeof(entries)/sizeof(entries[0]))) {
                    strncpy(entries[cnt].u, u, sizeof(entries[cnt].u)-1);
                    entries[cnt].u[sizeof(entries[cnt].u)-1] = '\0';
                    entries[cnt].pid = atoi(pp);
                    cnt++;
                }
            }
        }
        fclose(fin);
    }
    
    // 2. 检查并更新当前PID对应的会话记录（防止重复或旧数据）
    int mypid = GETPID();
    for (int i = 0; i < cnt; i++) {
        if (entries[i].pid == mypid) { 
            entries[i] = entries[cnt-1]; 
            cnt--; 
            i--; 
        }
    }
    
    // 3. 添加新记录
    if (cnt < (int)(sizeof(entries)/sizeof(entries[0]))) {
        strncpy(entries[cnt].u, current_user.username, sizeof(entries[cnt].u)-1);
        entries[cnt].u[sizeof(entries[cnt].u)-1] = '\0';
        entries[cnt].pid = mypid;
        cnt++;
    }
    
    // 4. 写回会话文件
    FILE *fout = fopen(p, "w");
    if (!fout) return;
    for (int i = 0; i < cnt; i++) fprintf(fout, "%s\t%d\n", entries[i].u, entries[i].pid);
    fclose(fout);
}

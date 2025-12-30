#include "user_internal.h"

/**
 * 注销当前会话。
 * 从会话文件中移除当前进程对应的记录。
 */
void user_session_unregister() {
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) return;
    typedef struct { char u[MAX_USERNAME_LENGTH]; int pid; } E;
    E entries[256]; int cnt = 0;
    
    // 1. 读取所有会话
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
    
    // 2. 过滤掉当前进程的记录
    int mypid = GETPID();
    E kept[256]; int kcnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (entries[i].pid != mypid) kept[kcnt++] = entries[i];
    }
    
    // 3. 写回文件
    FILE *fout = fopen(p, "w");
    if (!fout) return;
    for (int i = 0; i < kcnt; i++) fprintf(fout, "%s\t%d\n", kept[i].u, kept[i].pid);
    fclose(fout);
}

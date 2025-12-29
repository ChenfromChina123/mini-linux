#include "user.h"
#include "util.h"
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#ifdef _WIN32
#include <process.h>
#define GETPID() _getpid()
#else
#define GETPID() getpid()
#endif

// 全局变量定义
User users[MAX_USERS];
int user_count = 0;
User current_user;

// 初始化用户系统，添加默认用户
static int get_user_db_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_users", home) < (int)size;
}

static int get_sessions_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_sessions", home) < (int)size;
}

static void save_users() {
    char p[PATH_MAX];
    if (!get_user_db_path(p, sizeof(p))) return;
    FILE *f = fopen(p, "w");
    if (!f) return;
    for (int i = 0; i < user_count; i++) fprintf(f, "%s\t%s\t%d\n", users[i].username, users[i].password, users[i].is_root);
    fclose(f);
}

static void load_users() {
    char p[PATH_MAX];
    if (!get_user_db_path(p, sizeof(p))) {
        user_count = 0;
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

void user_init() {
    load_users();
    strcpy(current_user.username, "");
    current_user.is_root = 0;
}

void user_session_register() {
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) return;
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
    int mypid = GETPID();
    for (int i = 0; i < cnt; i++) if (entries[i].pid == mypid) { entries[i] = entries[cnt-1]; cnt--; i--; }
    if (cnt < (int)(sizeof(entries)/sizeof(entries[0]))) {
        strncpy(entries[cnt].u, current_user.username, sizeof(entries[cnt].u)-1);
        entries[cnt].u[sizeof(entries[cnt].u)-1] = '\0';
        entries[cnt].pid = mypid;
        cnt++;
    }
    FILE *fout = fopen(p, "w");
    if (!fout) return;
    for (int i = 0; i < cnt; i++) fprintf(fout, "%s\t%d\n", entries[i].u, entries[i].pid);
    fclose(fout);
}

void user_session_unregister() {
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) return;
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
    int mypid = GETPID();
    E kept[256]; int kcnt = 0;
    for (int i = 0; i < cnt; i++) if (entries[i].pid != mypid) kept[kcnt++] = entries[i];
    FILE *fout = fopen(p, "w");
    if (!fout) return;
    for (int i = 0; i < kcnt; i++) fprintf(fout, "%s\t%d\n", kept[i].u, kept[i].pid);
    fclose(fout);
}

// 用户登录
int user_login(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            // 登录成功，更新当前用户
            strcpy(current_user.username, users[i].username);
            strcpy(current_user.password, users[i].password);
            current_user.is_root = users[i].is_root;
            return 1;
        }
    }
    return 0; // 登录失败
}

// 创建用户
int user_create(const char *username, const char *password, int is_root) {
    // 检查是否有足够空间
    if (user_count >= MAX_USERS) {
        error("用户数量已达上限");
        return 0;
    }
    
    // 检查用户名是否已存在
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            error("用户名已存在");
            return 0;
        }
    }
    
    // 创建新用户
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].is_root = is_root;
    user_count++;
    save_users();
    
    return 1;
}

// 删除用户
int user_delete(const char *username) {
    // 不能删除root用户
    if (strcmp(username, "root") == 0) {
        error("不能删除root用户");
        return 0;
    }
    
    // 查找要删除的用户
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
    
    // 删除用户，后面的用户向前移动
    for (int i = index; i < user_count - 1; i++) {
        users[i] = users[i + 1];
    }
    user_count--;
    save_users();
    
    return 1;
}

// 检查当前用户是否为root用户
int is_root_user() {
    return current_user.is_root;
}

int get_user_count() { return user_count; }
const User* get_user(int index) { if (index < 0 || index >= user_count) return NULL; return &users[index]; }
const User* get_current_user() { return &current_user; }
void user_list_all() {
    printf("\033[34m所有用户：\033[0m\n");
    printf("%-4s %-20s %-6s\n", "序号", "用户名", "类型");
    for (int i = 0; i < user_count; i++) {
        printf("%-4d %-20s %-6s\n", i + 1, users[i].username, users[i].is_root ? "root" : "user");
    }
}
void user_list_active() {
    printf("\033[34m活跃用户：\033[0m\n");
    char p[PATH_MAX];
    if (!get_sessions_path(p, sizeof(p))) { if (current_user.username[0] == '\0') printf("无\n"); else printf("%s\n", current_user.username); return; }
    FILE *f = fopen(p, "r");
    if (!f) { if (current_user.username[0] == '\0') printf("无\n"); else printf("%s\n", current_user.username); return; }
    char seen[MAX_USERS][MAX_USERNAME_LENGTH]; int sc = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        size_t L = strlen(line);
        if (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = '\0';
        char *u = strtok(line, "\t");
        if (u) {
            int ok = 1; for (int i = 0; i < sc; i++) if (strcmp(seen[i], u) == 0) { ok = 0; break; }
            if (ok && sc < MAX_USERS) { strncpy(seen[sc], u, sizeof(seen[0]) - 1); seen[sc][sizeof(seen[0]) - 1] = '\0'; sc++; }
        }
    }
    fclose(f);
    if (sc == 0) printf("无\n"); else for (int i = 0; i < sc; i++) printf("%s\n", seen[i]);
}

int user_change_password(const char *username, const char *old_password, const char *new_password, int force) {
    if (!username || !new_password) { error("参数无效"); return 0; }
    int idx = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) { idx = i; break; }
    }
    if (idx < 0) { error("用户不存在"); return 0; }
    if (!force) {
        if (!old_password || strcmp(users[idx].password, old_password) != 0) { error("旧密码错误"); return 0; }
    }
    size_t L = strlen(new_password);
    if (L == 0 || L >= MAX_PASSWORD_LENGTH) { error("新密码长度非法"); return 0; }
    strncpy(users[idx].password, new_password, MAX_PASSWORD_LENGTH - 1);
    users[idx].password[MAX_PASSWORD_LENGTH - 1] = '\0';
    save_users();
    success("密码已更新");
    return 1;
}

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
// 最大用户数
#define MAX_USERS 1024
// 用户结构体数组
User users[MAX_USERS];
// 当前用户数
int user_count = 0;
// 当前登录用户 
User current_user;

// 初始化用户系统，添加默认用户
/**
 * 获取用户数据库文件路径
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return 成功返回1，失败返回0
 */
static int get_user_db_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_users", home) < (int)size;
}

/**
 * 获取会话跟踪文件路径
 * @param path 存储路径的缓冲区
 * @param size 缓冲区大小
 * @return 成功返回1，失败返回0
 */
static int get_sessions_path(char *path, size_t size) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) home = getenv("USERPROFILE");
#endif
    if (home == NULL) return 0;
    return snprintf(path, size, "%s/.mini_sessions", home) < (int)size;
}

/**
 * 将当前内存中的用户列表保存到持久化文件
 */
static void save_users() {
    char p[PATH_MAX];
    if (!get_user_db_path(p, sizeof(p))) return;
    FILE *f = fopen(p, "w");
    if (!f) return;
    for (int i = 0; i < user_count; i++) fprintf(f, "%s\t%s\t%d\n", users[i].username, users[i].password, users[i].is_root);
    fclose(f);
}

/**
 * 从持久化文件加载用户列表，如果文件不存在则初始化默认用户
 */
static void load_users() {
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
    
    // 如果加载后依然没有用户，则兜底创建
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

/**
 * 初始化用户子系统
 */
void user_init() {
    load_users();
    strcpy(current_user.username, "");
    current_user.is_root = 0;
}

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

/**
 * 用户登录验证
 * @param username 用户名
 * @param password 密码
 * @return 登录成功返回1，失败返回0
 */
int user_login(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            // 匹配成功，更新全局当前用户信息
            strcpy(current_user.username, users[i].username);
            strcpy(current_user.password, users[i].password);
            current_user.is_root = users[i].is_root;
            return 1;
        }
    }
    return 0; // 验证失败
}

/**
 * 创建新用户
 * @param username 用户名
 * @param password 密码
 * @param is_root 是否为管理员
 * @return 成功返回1，失败返回0
 */
int user_create(const char *username, const char *password, int is_root) {
    // 检查数组上限
    if (user_count >= MAX_USERS) {
        error("用户数量已达上限");
        return 0;
    }
    
    // 唯一性检查
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            error("用户名已存在");
            return 0;
        }
    }
    
    // 写入全局数组并同步到文件
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].is_root = is_root;
    user_count++;
    save_users();
    
    return 1;
}

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

/**
 * 判断当前登录用户是否为管理员
 * @return 是返回1，否则返回0
 */
int is_root_user() {
    return current_user.is_root;
}

/**
 * 获取当前系统总用户数
 */
int get_user_count() { return user_count; }

/**
 * 获取指定索引的用户信息
 */
const User* get_user(int index) { if (index < 0 || index >= user_count) return NULL; return &users[index]; }

/**
 * 获取当前登录用户信息
 */
const User* get_current_user() { return &current_user; }

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

/**
 * 修改用户密码
 * @param username 用户名
 * @param old_password 旧密码（非强制模式需提供）
 * @param new_password 新密码
 * @param force 是否强制修改（管理员修改他人密码时设为1）
 * @return 成功返回1，失败返回0
 */
int user_change_password(const char *username, const char *old_password, const char *new_password, int force) {
    if (!username || !new_password) { error("参数无效"); return 0; }
    
    // 查找用户
    int idx = -1;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) { idx = i; break; }
    }
    
    if (idx < 0) { error("用户不存在"); return 0; }
    
    // 权限与密码验证
    if (!force) {
        if (!old_password || strcmp(users[idx].password, old_password) != 0) { error("旧密码错误"); return 0; }
    }
    
    // 长度检查
    size_t L = strlen(new_password);
    if (L == 0 || L >= MAX_PASSWORD_LENGTH) { error("新密码长度非法"); return 0; }
    
    // 更新并同步
    strncpy(users[idx].password, new_password, MAX_PASSWORD_LENGTH - 1);
    users[idx].password[MAX_PASSWORD_LENGTH - 1] = '\0';
    save_users();
    success("密码已更新");
    return 1;
}

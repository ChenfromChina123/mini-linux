#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#include <process.h>
#include <direct.h>
#define STDIN_FILENO 0
#define getcwd _getcwd
#else
#include <sys/wait.h>
#include <termios.h>
#endif

static void enable_raw(void *orig, int *use_raw) {
    *use_raw = 0;
#ifndef _WIN32
    struct termios *t = (struct termios *)orig;
    if (tcgetattr(STDIN_FILENO, t) == 0) {
        struct termios raw = *t;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        *use_raw = 1;
    }
#endif
}

int cmd_users(int argc, char *argv[]) {
    user_list_all();
    user_list_active();
    return 0;
}

static void disable_raw(void *orig, int use_raw) {
#ifndef _WIN32
    if (use_raw) tcsetattr(STDIN_FILENO, TCSAFLUSH, (struct termios *)orig);
#endif
}

static int read_key() {
    int c = getchar();
    if (c == 27) {
        int c1 = getchar();
        if (c1 == '[') {
            int c2 = getchar();
            if (c2 == 'A') return -1001;
            if (c2 == 'B') return -1002;
            if (c2 == 'C') return -1003;
            if (c2 == 'D') return -1004;
        }
        return 27;
    }
    return c;
}

static int read_line_edit(const char *prompt, char *out, size_t out_size) {
#ifdef _WIN32
    printf("%s", prompt); fflush(stdout);
    if (fgets(out, out_size, stdin) == NULL) return 0;
    trim(out);
    return (int)strlen(out);
#else
    struct termios orig; int use_raw = 0; enable_raw(&orig, &use_raw);
    size_t len = 0; size_t cur = 0; int hist_idx = history_size();
    char saved[1024]; saved[0] = '\0';
    printf("%s", prompt); fflush(stdout);
    while (1) {
        int k = read_key();
        if (k == '\r' || k == '\n') { putchar('\n'); break; }
        else if (k == 127 || k == 8) {
            if (cur > 0) {
                cur--; for (size_t i = cur; i < len - 1; i++) out[i] = out[i + 1]; len--; out[len] = '\0';
                fputs("\x1b[D", stdout); fputs("\x1b[K", stdout); fwrite(out + cur, 1, len - cur, stdout); fputs(" ", stdout);
                size_t back = len - cur + 1; while (back--) fputs("\x1b[D", stdout);
                fflush(stdout);
            }
        } else if (k == -1004) {
            if (cur > 0) { fputs("\x1b[D", stdout); cur--; }
        } else if (k == -1003) {
            if (cur < len) { fputs("\x1b[C", stdout); cur++; }
        } else if (k == -1001 || k == -1002) {
            if (hist_idx == history_size()) strncpy(saved, out, sizeof(saved) - 1), saved[sizeof(saved) - 1] = '\0';
            if (k == -1001 && hist_idx > 0) hist_idx--; else if (k == -1002 && hist_idx < history_size()) hist_idx++;
            const char *h = hist_idx < history_size() ? history_get_command(hist_idx) : saved;
            if (!h) h = "";
            size_t L = strlen(h); if (L >= out_size) L = out_size - 1;
            memcpy(out, h, L); out[L] = '\0'; len = L; cur = L;
            fputs("\r", stdout); fputs("\x1b[K", stdout); printf("%s%s", prompt, out); fflush(stdout);
        } else if (k >= 32 && k < 127) {
            if (len + 1 < out_size) {
                for (size_t i = len; i > cur; i--) out[i] = out[i - 1]; out[cur] = (char)k; len++; out[len] = '\0';
                fputs("\x1b[K", stdout); fwrite(out + cur, 1, len - cur, stdout);
                cur++; size_t back = len - cur; while (back--) fputs("\x1b[D", stdout); fflush(stdout);
            }
        }
    }
    disable_raw(&orig, use_raw);
    return (int)len;
#endif
}

// 定义所有命令
Command commands[] = {
    {"mycat", cmd_mycat, "读取并显示文件内容"},
    {"myrm", cmd_myrm, "删除指定文件"},
    {"myvi", cmd_myvi, "简单的文件编辑器"},
    {"mytouch", cmd_mytouch, "创建空文件"},
    {"myecho", cmd_myecho, "向文件写入内容"},
    {"mycp", cmd_mycp, "复制文件"},
    {"myls", cmd_myls, "显示目录内容"},
    {"mycd", cmd_mycd, "切换当前目录"},
    {"mymkdir", cmd_mymkdir, "创建目录"},
    {"myps", cmd_myps, "显示进程信息"},
    {"users", cmd_users, "显示系统用户与活跃用户"},
    {"passwd", cmd_passwd, "修改密码"},
    {"agent", cmd_agent, "启动小晨AI终端助手"},
    {"exit", cmd_exit, "退出shell"},
    {"clear", cmd_clear, "清屏"},
    {"help", cmd_help, "显示帮助信息"},
    {"history", cmd_history, "显示命令历史记录"},
    {"useradd", cmd_useradd, "创建新用户"},
    {"userdel", cmd_userdel, "删除用户"},
    {NULL, NULL, NULL} // 命令列表结束标记
};

// 查找命令
Command* find_command(const char *name) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

// 退出命令实现
int cmd_exit(int argc, char *argv[]) {
    printf("\033[34m感谢使用Mini Linux Shell！\033[0m\n");
    user_session_unregister();
    exit(0);
    return 0;
}

// 清屏命令实现
int cmd_clear(int argc, char *argv[]) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return 0;
}

// 帮助命令实现
int cmd_help(int argc, char *argv[]) {
    printf("\033[34m可用命令：\033[0m\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        // 默认显示 名称 - 描述
        printf("%s\t- %s", commands[i].name, commands[i].description);

        // 为部分命令添加简短用法说明
        if (strcmp(commands[i].name, "mycat") == 0) {
            printf("，用法: mycat <文件名> [文件名...]");
        } else if (strcmp(commands[i].name, "myls") == 0) {
            printf("，用法: myls [目录] 或 myls -l [目录]");
        } else if (strcmp(commands[i].name, "mycd") == 0) {
            printf("，用法: mycd [目录]（无参数切换到主目录，支持 ~ 展开）");
        } else if (strcmp(commands[i].name, "mymkdir") == 0) {
            printf("，用法: mymkdir [-p] <目录>（-p: 创建父目录）");
        } else if (strcmp(commands[i].name, "myecho") == 0) {
            printf("，用法: myecho <文本> [> 文件]（支持重定向）");
        } else if (strcmp(commands[i].name, "mycp") == 0) {
            printf("，用法: mycp <源文件> <目标文件>");
        } else if (strcmp(commands[i].name, "myrm") == 0) {
            printf("，用法: myrm <文件>");
        } else if (strcmp(commands[i].name, "mytouch") == 0) {
            printf("，用法: mytouch <文件>");
        } else if (strcmp(commands[i].name, "myvi") == 0) {
            printf("，用法: myvi <文件>");
        } else if (strcmp(commands[i].name, "myps") == 0) {
            printf("，用法: myps");
        } else if (strcmp(commands[i].name, "users") == 0) {
            printf("，用法: users");
        } else if (strcmp(commands[i].name, "passwd") == 0) {
            printf("，用法: passwd [用户名]（root可改他人）");
        } else if (strcmp(commands[i].name, "history") == 0) {
            printf("，用法: history");
        } else if (strcmp(commands[i].name, "agent") == 0) {
            printf("，用法: agent [指令...]（无参数启动交互式AI助手）");
        }

        printf("\n");
    }
    return 0;
}

int cmd_passwd(int argc, char *argv[]) {
    if (argc == 1) {
        char oldpw[MAX_PASSWORD_LENGTH];
        char newpw1[MAX_PASSWORD_LENGTH];
        char newpw2[MAX_PASSWORD_LENGTH];
        printf("旧密码: "); fgets(oldpw, sizeof(oldpw), stdin); trim(oldpw);
        printf("新密码: "); fgets(newpw1, sizeof(newpw1), stdin); trim(newpw1);
        printf("再次输入新密码: "); fgets(newpw2, sizeof(newpw2), stdin); trim(newpw2);
        if (strcmp(newpw1, newpw2) != 0) { error("两次输入不一致"); return 1; }
        return user_change_password(current_user.username, oldpw, newpw1, 0) ? 0 : 1;
    } else {
        if (!is_root_user()) { error("只有root可以修改其他用户"); return 1; }
        const char *target = argv[1];
        char newpw1[MAX_PASSWORD_LENGTH];
        char newpw2[MAX_PASSWORD_LENGTH];
        if (argc >= 3) {
            strncpy(newpw1, argv[2], sizeof(newpw1) - 1); newpw1[sizeof(newpw1) - 1] = '\0';
            strncpy(newpw2, newpw1, sizeof(newpw2) - 1); newpw2[sizeof(newpw2) - 1] = '\0';
        } else {
            printf("新密码: "); fgets(newpw1, sizeof(newpw1), stdin); trim(newpw1);
            printf("再次输入新密码: "); fgets(newpw2, sizeof(newpw2), stdin); trim(newpw2);
        }
        if (strcmp(newpw1, newpw2) != 0) { error("两次输入不一致"); return 1; }
        return user_change_password(target, NULL, newpw1, 1) ? 0 : 1;
    }
}

// 历史命令实现
int cmd_history(int argc, char *argv[]) {
    history_show();
    return 0;
}

// 创建用户命令实现
int cmd_useradd(int argc, char *argv[]) {
    if (!is_root_user()) {
        error("只有root用户可以创建用户");
        return 1;
    }
    
    if (argc < 3) {
        error("使用方法: useradd <用户名> <密码> [--root]");
        return 1;
    }
    
    int is_root = 0;
    if (argc >= 4 && strcmp(argv[3], "--root") == 0) {
        is_root = 1;
    }
    
    if (user_create(argv[1], argv[2], is_root)) {
        success("用户创建成功");
        return 0;
    } else {
        return 1;
    }
}

// 删除用户命令实现
int cmd_userdel(int argc, char *argv[]) {
    if (!is_root_user()) {
        error("只有root用户可以删除用户");
        return 1;
    }
    
    if (argc < 2) {
        error("使用方法: userdel <用户名>");
        return 1;
    }
    
    if (user_delete(argv[1])) {
        success("用户删除成功");
        return 0;
    } else {
        return 1;
    }
}

// 初始化shell
void shell_init() {
    // 初始化用户系统
    user_init();
    
    // 初始化命令历史记录
    history_init();
    
    // 显示欢迎信息
    printf("\033[34m欢迎使用Mini Linux Shell！\033[0m\n");
    printf("\033[34m输入 'help' 查看可用命令。\033[0m\n");
    
    // 登录界面
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    while (1) {
        printf("login: ");
        if (fgets(username, sizeof(username), stdin) == NULL) exit(0);
        trim(username);
        if (strlen(username) == 0) continue;
        
        printf("password: ");
        // 简单起见，这里不隐藏密码输入（Windows下实现隐藏输入较复杂）
        if (fgets(password, sizeof(password), stdin) == NULL) exit(0);
        trim(password);
        
        if (user_login(username, password)) {
            user_session_register();
            success("登录成功！");
            break;
        } else {
            error("登录失败，用户名或密码错误。");
        }
    }
}

// 执行命令
int execute_command(int argc, char *argv[]) {
    if (argc == 0) {
        return 0;
    }
    
    // 1. 检查是否为内置命令
    Command *cmd = find_command(argv[0]);
    if (cmd != NULL) {
        // 内置命令，直接执行
        return cmd->func(argc, argv);
    }
    
    // 2. 检查是否为可执行文件（C程序或shell脚本）
    
    // 尝试直接执行
    if (access(argv[0], 0) == 0) {
        // 文件可执行
#ifdef _WIN32
        // Windows 下使用 system 或 _spawn
        // 这里简单起见使用 system，但需要拼接命令
        char cmd_line[1024] = "";
        for (int i = 0; i < argc; i++) {
            strcat(cmd_line, argv[i]);
            if (i < argc - 1) strcat(cmd_line, " ");
        }
        return system(cmd_line);
#else
        // 创建子进程执行命令
        pid_t pid = fork();
        
        if (pid < 0) {
            // fork失败
            error("无法创建子进程");
            return 1;
        } else if (pid == 0) {
            // 子进程
            execvp(argv[0], argv);
            // 如果execvp返回，说明执行失败
            perror("execvp");
            exit(1);
        } else {
            // 父进程，等待子进程结束
            int status;
            waitpid(pid, &status, 0);
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }
#endif
    }
    
    error("未找到命令");
    return 1;
}

// Shell主循环
void shell_loop() {
    char input[MAX_CMD_LENGTH];
    char *argv[MAX_ARGS];
    int argc;
    
    while (1) {
        // 显示提示符
        char prompt[256];
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) == NULL) strcpy(cwd, "?");
        snprintf(prompt, sizeof(prompt), "\033[32m%s@mini-computer\033[0m:\033[34m%s\033[0m$ ", 
                 current_user.username, cwd);
        
        // 读取输入
        if (read_line_edit(prompt, input, sizeof(input)) <= 0) {
            continue;
        }
        
        // 保存历史记录
        history_add(input, 0);
        
        // 解析命令
        argc = split_command(input, argv, MAX_ARGS);
        
        // 执行命令
        execute_command(argc, argv);
    }
}

// 程序入口点
int main(int argc, char *argv[]) {
    // 设置控制台为 UTF-8
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    
    shell_init();
    shell_loop();
    return 0;
}

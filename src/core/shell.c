#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>


/**
 * @brief 启用终端原始模式
 * @param orig 原始终端属性存储
 * @param use_raw 是否成功启用的标志
 */
static void enable_raw(struct termios *orig, int *use_raw) {
    *use_raw = 0;
    if (tcgetattr(STDIN_FILENO, orig) == 0) {
        struct termios raw = *orig;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        *use_raw = 1;
    }
}

/**
 * @brief 显示系统用户与活跃用户
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回0
 */
int cmd_users(int argc, char *argv[]) {
    user_list_all();
    user_list_active();
    return 0;
}

/**
 * @brief 禁用终端原始模式
 * @param orig 原始终端属性
 * @param use_raw 是否处于原始模式
 */
static void disable_raw(struct termios *orig, int use_raw) {
    if (use_raw) tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

// 读取键码
// 读取用户输入的键码。
// 如果是特殊键（如方向键），会返回一个负数。
// 否则，返回键码本身。
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

// 读取用户输入的行编辑
// 读取用户输入的行编辑，支持方向键、删除键、历史命令等功能。
// 1. 打印提示信息 prompt。
// 2. 循环读取键码 k，直到用户按下 Enter 键或换行键。
// 3. 处理特殊键（如方向键、删除键），更新当前输入位置 cur 和输出缓冲区 out。
// 4. 处理普通字符键，将其添加到输出缓冲区 out 中。
// 5. 最后，将输出缓冲区 out 中的内容复制到 out 参数中。
static int read_line_edit(const char *prompt, char *out, size_t out_size) {
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

/**
 * @brief 查找内置命令
 * @param name 命令名称
 * @return 命令结构体指针，未找到返回NULL
 */
Command* find_command(const char *name) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

/**
 * @brief 退出 Shell
 */
int cmd_exit(int argc, char *argv[]) {
    printf("\033[34m感谢使用Mini Linux Shell！\033[0m\n");
    user_session_unregister();
    exit(0);
    return 0;
}

/**
 * @brief 清除屏幕
 */
int cmd_clear(int argc, char *argv[]) {
    system("clear");
    return 0;
}

/**
 * @brief 显示帮助信息
 * 按照功能分类显示命令用法
 */
int cmd_help(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("\n\033[1;34mMini Linux Shell - 可用命令：\033[0m\n");

    printf("\n\033[1;33m内置命令：\033[0m\n");
    printf("  help                       - 显示此帮助信息\n");
    printf("  clear                      - 清除终端屏幕\n");
    printf("  exit                       - 退出 Mini Linux Shell\n");
    printf("  history                    - 显示命令历史记录\n");

    printf("\n\033[1;33m文件操作：\033[0m\n");
    printf("  mycat <文件>               - 读取并显示文件内容\n");
    printf("  mytouch <文件>             - 创建一个空文件\n");
    printf("  myecho <文本> [> 文件]     - 打印文本或重定向到文件\n");
    printf("  mycp <源> <目的>           - 复制文件或目录\n");
    printf("  myrm <文件>                - 删除指定文件\n");
    printf("  myvi <文件>                - 简单的文本编辑器\n");
    printf("  mychmod <权限> <文件>      - 修改文件权限 (如: mychmod 777 test.txt)\n");

    printf("\n\033[1;33m目录操作：\033[0m\n");
    printf("  myls [-l] [目录]           - 列出目录内容 (-l 显示详细信息)\n");
    printf("  mycd [目录]                - 切换当前工作目录 (支持 ~ 展开)\n");
    printf("  mymkdir [-p] <目录>        - 创建新目录 (-p 递归创建)\n");

    printf("\n\033[1;33m进程管理：\033[0m\n");
    printf("  myps                       - 显示当前系统进程快照\n");
    printf("  mykill <PID>               - 终止指定进程\n");

    printf("\n\033[1;33m用户管理：\033[0m\n");
    printf("  users                      - 列出系统中所有用户\n");
    printf("  passwd [用户名]            - 修改用户密码 (root 可修改他人)\n");
    printf("  useradd <名> <密> [--root] - 创建新用户 (仅限 root)\n");
    printf("  userdel <用户名>           - 删除指定用户 (仅限 root)\n");

    printf("\n\033[1;33mAI 助手：\033[0m\n");
    printf("  agent [指令...]            - 启动小晨AI助手 (无参数进入交互模式)\n");

    printf("\n\033[1;32m提示：\033[0m本 Shell 支持执行系统命令 (如 ls, top, ping 等)。\n");
    printf("      如果输入的命令不是内置命令，Shell 将尝试从系统 PATH 中寻找并执行。\n\n");

    return 0;
}

/**
 * @brief 修改用户密码
 */
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


/**
 * @brief 创建新用户
 */
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

// 删除用户命令实现：允许root用户删除已存在的用户。
// 1. 检查是否为root用户：如果不是，提示错误并返回。
// 2. 检查参数数量：如果参数不足，提示使用方法并返回。
// 3. 解析参数：获取要删除的用户名。
// 4. 调用 user_delete 函数删除用户。
// 5. 根据删除结果打印成功或失败消息。
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
}

/**
 * @brief 执行命令
 * @param argc 参数个数
 * @param argv 参数列表
 * @return 执行结果状态码
 */
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
    if (access(argv[0], X_OK) == 0) {
        // 文件可执行
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
            error("命令执行失败");
            exit(1);
        } else {
            // 父进程，等待子进程完成
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                error("命令执行异常");
                return 1;
            }
        }
    }
    
    // 3. 检查是否为C程序源文件
    char c_filename[256];
    sprintf(c_filename, "%s.c", argv[0]);
    if (access(c_filename, R_OK) == 0) {
        error("直接执行C源文件暂不支持，请先编译");
        return 1;
    }
    
    // 4. 检查是否为shell脚本文件
    char sh_filename[256];
    sprintf(sh_filename, "%s.sh", argv[0]);
    if (access(sh_filename, R_OK) == 0) {
        // 使用system函数执行shell脚本
        return system(sh_filename);
    }
    
    // 5. 命令不存在
    error("command not found");
    return 1;
}

/**
 * @brief Shell 主循环
 * 处理登录验证和命令循环执行
 */
void shell_loop() {
    // 用户登录
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int login_success = 0;
    
    printf("\n\033[1;36m========================================\033[0m\n");
    printf("\033[1;36m       欢迎登录 Mini Linux Shell        \033[0m\n");
    printf("\033[1;36m========================================\033[0m\n");
    
    for (int i = 0; i < 3; i++) {
        printf("\n用户名: ");
        fflush(stdout);
        if (fgets(username, sizeof(username), stdin) == NULL) break;
        trim(username);
        
        printf("密码: ");
        fflush(stdout);
        if (fgets(password, sizeof(password), stdin) == NULL) break;
        trim(password);
        
        if (user_login(username, password)) {
            login_success = 1;
            break;
        } else {
            error("用户名或密码错误，请重试");
        }
    }
    
    if (!login_success) {
        printf("\033[31m登录失败次数过多，退出。\033[0m\n");
        exit(1);
    }
    
    user_session_register();
    // 主命令循环
    // 1. 显示命令提示
    // 2. 读取用户输入
    // 3. 解析命令
    // 4. 执行命令
    // 5. 循环执行以上步骤
    char command[MAX_CMD_LENGTH];
    
    while (1) {
        char prompt[128]; snprintf(prompt, sizeof(prompt), "\033[32m%s@mini-linux\033[0m:\033[34m$\033[0m ", current_user.username);
        int L = read_line_edit(prompt, command, sizeof(command));
        if (L <= 0) continue;
        
        if (strlen(command) == 0) {
            continue;
        }
        
        // 解析命令
        char *argv[MAX_ARGS];
        int argc = split_command(command, argv, MAX_ARGS);
        
        // 执行命令并获取结果
        int result = execute_command(argc, argv);
        
        history_add(command, result);
        
        // 释放内存
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
    }
}

 

#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>


// 启用原始模式
// 原始模式下，终端输入的每个字符都会立即被读取，而不会等待用户按下 Enter 键。
// 这使得我们可以实现实时读取用户输入，而不是等待用户按下 Enter 键。
// 原始模式下，终端输入的每个字符都会立即被读取，而不会等待用户按下 Enter 键。
// 这使得我们可以实现实时读取用户输入，而不是等待用户按下 Enter 键。
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

// users命令实现：显示所有用户和活动用户
// users 工作流程：
// 1. 调用 user_list_all 函数显示所有用户。
// 2. 调用 user_list_active 函数显示活动用户。
int cmd_users(int argc, char *argv[]) {
    user_list_all();
    user_list_active();
    return 0;
}

// 禁用原始模式
// 禁用原始模式后，终端输入的每个字符都会等待用户按下 Enter 键后才被读取。
// 这是默认的终端模式，用于处理用户输入。
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

// 查找命令
// 查找命令列表中名称为 name 的命令。
// 如果找到，返回指向该命令的指针；否则，返回 NULL。
Command* find_command(const char *name) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

// 退出命令实现
// 退出 Mini Linux Shell 程序。
// 1. 打印退出提示信息。
// 2. 注销当前用户会话。
// 3. 调用 exit 函数退出程序。
int cmd_exit(int argc, char *argv[]) {
    printf("\033[34m感谢使用Mini Linux Shell！\033[0m\n");
    user_session_unregister();
    exit(0);
    return 0;
}

// 清屏命令实现
// 清屏命令实现：清除终端屏幕。
// 1. 根据操作系统类型（Windows 或类 Unix）调用不同的清屏命令。
// 2. 调用 system 函数执行清屏命令。
int cmd_clear(int argc, char *argv[]) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return 0;
}

// 帮助命令实现
// 帮助命令实现：显示所有可用命令的名称和简短描述。
// 1. 遍历命令列表，打印每个命令的名称和描述。
// 2. 为部分命令添加简短的用法说明。
int cmd_help(int argc, char *argv[]) {
    printf("\033[34m可用命令：\033[0m\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        // 默认显示 名称 - 描述
        printf("%s\t- %s", commands[i].name, commands[i].description);

        // 为部分命令添加简短用法说明
        //strcmp 函数用于比较两个字符串是否相等。
        //commands[i].name 是当前命令的名称。
        //"mycat" 等是要匹配的命令名称。
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

// 密码修改命令实现
// 密码修改命令实现：允许用户修改自己的密码或root用户修改其他用户的密码。
// 1. 检查参数数量：如果没有参数，提示输入旧密码、新密码和确认新密码。
// 2. 如果有参数，检查是否为root用户：
//    a. 如果是root用户，检查是否提供了目标用户名和新密码。
//    b. 如果不是root用户，检查是否提供了新密码。
// 3. 验证旧密码：调用 user_check_password 函数验证旧密码是否正确。
// 4. 验证新密码：检查新密码是否与确认新密码匹配。
// 5. 调用 user_change_password 函数修改密码。
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


// 创建用户命令实现：允许root用户创建新用户。
// 1. 检查是否为root用户：如果不是，提示错误并返回。
// 2. 检查参数数量：如果参数不足，提示使用方法并返回。
// 3. 解析参数：获取用户名、密码和是否为root用户的标志。
// 4. 调用 user_create 函数创建用户。
// 5. 根据创建结果打印成功或失败消息。  
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

// 主循环
// 1. 显示登录提示
// 2. 读取用户名和密码
// 3. 调用 user_login 函数验证登录
// 4. 如果登录成功，注册用户会话
// 5. 进入主命令循环
void shell_loop() {
    // 用户登录
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int login_success = 0;
    
    for (int i = 0; i < 3; i++) {
        printf("\n用户名: ");
        fgets(username, sizeof(username), stdin);
        trim(username);
        
        printf("密码: ");
        fgets(password, sizeof(password), stdin);
        trim(password);
        
        if (user_login(username, password)) {
            login_success = 1;
            break;
        } else {
            error("用户名或密码错误");
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

// 主函数
int main() {
    // 初始化shell
    shell_init();
    
    // 启动主循环
    shell_loop();
    
    return 0;
}

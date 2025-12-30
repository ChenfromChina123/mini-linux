#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include "input.h"
#include <unistd.h>
#include <sys/wait.h>


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
    {"agent", cmd_agent, "启动小晨AI终端助手"},
    {"exit", cmd_exit, "退出shell"},
    {"clear", cmd_clear, "清屏"},
    {"help", cmd_help, "显示帮助信息"},
    {"history", cmd_history, "显示命令历史记录"},
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

    printf("\n\033[1;33m用户管理 (脚本实现)：\033[0m\n");
    printf("  users                      - 列出系统中所有用户\n");
    printf("  mypasswd [用户名]          - 修改用户密码 (root 可修改他人)\n");
    printf("  myuseradd                  - 创建新用户 (支持交互/批量)\n");
    printf("  myuserdel <用户名>         - 删除指定用户\n");

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
    
    // 首先尝试在当前目录找 .sh
    sprintf(sh_filename, "%s.sh", argv[0]);
    if (access(sh_filename, R_OK) == 0) {
        char full_cmd[1024] = {0};
        sprintf(full_cmd, "sh %s", sh_filename);
        for (int i = 1; i < argc; i++) {
            strcat(full_cmd, " ");
            strcat(full_cmd, argv[i]);
        }
        return system(full_cmd);
    }
    
    // 尝试在 scripts/ 目录找 .sh
    sprintf(sh_filename, "scripts/%s.sh", argv[0]);
    if (access(sh_filename, R_OK) == 0) {
        // 构建完整命令行字符串
        char full_cmd[1024] = {0};
        sprintf(full_cmd, "sh %s", sh_filename);
        for (int i = 1; i < argc; i++) {
            strcat(full_cmd, " ");
            strcat(full_cmd, argv[i]);
        }
        return system(full_cmd);
    }

    // 尝试在 bin/ 目录找 .sh (Makefile 复制后的位置)
    sprintf(sh_filename, "bin/%s.sh", argv[0]);
    if (access(sh_filename, R_OK) == 0) {
        char full_cmd[1024] = {0};
        sprintf(full_cmd, "sh %s", sh_filename);
        for (int i = 1; i < argc; i++) {
            strcat(full_cmd, " ");
            strcat(full_cmd, argv[i]);
        }
        return system(full_cmd);
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
    printf("\033[1;36m       输入 'help' 查看可用命令         \033[0m\n");
    printf("\033[1;36m       输入 'exit' 退出 Shell           \033[0m\n");
    printf("\033[1;36m========================================\033[0m\n");
    
    for (int i = 0; i < 3; i++) {
        if (read_line_with_edit("\n用户名: ", username, sizeof(username)) <= 0) break;
        trim(username);
        
        if (read_line_with_edit("密码: ", password, sizeof(password)) <= 0) break;
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
        int L = read_line_with_edit(prompt, command, sizeof(command));
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

 

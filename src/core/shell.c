#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include "input.h"
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>


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
    {"mychmod", cmd_mychmod, "查看或修改文件权限"},
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
    printf("  mychmod <文件>             - 查看文件权限\n");
    printf("  mychmod <权限> <文件>      - 修改文件权限 (如: mychmod 777 test.txt)\n");

    printf("\n\033[1;33m目录操作：\033[0m\n");
    printf("  myls [-l] [目录]           - 列出目录内容 (-l 显示详细信息)\n");
    printf("  mycd [目录]                - 切换当前工作目录 (支持 ~ 展开)\n");
    printf("  mymkdir [-p] <目录>        - 创建新目录 (-p 递归创建)\n");

    printf("\n\033[1;33m用户管理：\033[0m\n");
    printf("  users                      - 显示系统用户与活跃用户\n");
    printf("  myuseradd <用户名>         - 创建新用户 (仅管理员)\n");
    printf("  myuserdel <用户名>         - 删除指定用户 (仅管理员)\n");
    printf("  mypasswd/passwd [用户名]   - 修改用户密码\n");

    printf("\n\033[1;33m系统工具：\033[0m\n");
    printf("  myps                       - 显示当前进程状态\n");
    printf("  mykill <进程名>            - 终止指定名称的进程\n");
    printf("  agent                      - 启动小晨AI终端助手\n");

    printf("\n");

    return 0;
}

/**
 * @brief 启动小晨AI终端助手
 */

// 历史命令实现
int cmd_history(int argc, char *argv[]) {
    history_show();
    return 0;
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

    // 2. 优先检查并执行 Shell 脚本 (scripts/ 目录)
    char sh_filename[256];
    const char *cmd_name = argv[0];
    
    // 别名映射
    if (strcmp(cmd_name, "passwd") == 0) cmd_name = "mypasswd";
    else if (strcmp(cmd_name, "useradd") == 0) cmd_name = "myuseradd";
    else if (strcmp(cmd_name, "userdel") == 0) cmd_name = "myuserdel";

    sprintf(sh_filename, "scripts/%s.sh", cmd_name);
    if (access(sh_filename, R_OK) == 0) {
        char full_cmd[1024] = {0};
        sprintf(full_cmd, "bash %s", sh_filename);
        for (int i = 1; i < argc; i++) {
            strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
            strncat(full_cmd, argv[i], sizeof(full_cmd) - strlen(full_cmd) - 1);
        }
        int result = system(full_cmd);
        
        // 执行用户管理脚本后重新加载用户数据，确保内存数据与文件同步
        if (strcmp(cmd_name, "myuseradd") == 0 || 
            strcmp(cmd_name, "myuserdel") == 0 || 
            strcmp(cmd_name, "mypasswd") == 0) {
            user_reload();
        }
        
        return result;
    }

    // 3. 检查是否为可执行文件（C程序）
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
    
    // 4. 命令不存在
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
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "?");
        }
        
        char prompt[512]; 
        snprintf(prompt, sizeof(prompt), "\033[32m%s@mini-linux\033[0m:\033[34m%s\033[0m\033[34m$\033[0m ", 
                 current_user.username, cwd);
        
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

 

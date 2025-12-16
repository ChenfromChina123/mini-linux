#include "shell.h"
#include "user.h"
#include "command.h"
#include "history.h"
#include "util.h"
#include <unistd.h>
#include <sys/wait.h>

// 定义所有命令
Command commands[] = {
    {"mycat", cmd_mycat, "读取并显示文件内容"},
    {"myrm", cmd_myrm, "删除指定文件"},
    {"myvi", cmd_myvi, "简单的文件编辑器"},
    {"mytouch", cmd_mytouch, "创建空文件"},
    {"myecho", cmd_myecho, "向文件写入内容"},
    {"mycp", cmd_mycp, "复制文件"},
    {"myls", cmd_myls, "显示目录内容"},
    {"myps", cmd_myps, "显示进程信息"},
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
    exit(0);
    return 0;
}

// 清屏命令实现
int cmd_clear(int argc, char *argv[]) {
    system("cls");
    return 0;
}

// 帮助命令实现
int cmd_help(int argc, char *argv[]) {
    printf("\033[34m可用命令：\033[0m\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        printf("%s\t- %s\n", commands[i].name, commands[i].description);
    }
    return 0;
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
    
    // 主命令循环
    char command[MAX_CMD_LENGTH];
    
    while (1) {
        // 显示命令提示符
        printf("\033[32m%s@mini-linux\033[0m:\033[34m$\033[0m ", current_user.username);
        
        // 读取用户输入
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        trim(command);
        
        if (strlen(command) == 0) {
            continue;
        }
        
        // 解析命令
        char *argv[MAX_ARGS];
        int argc = split_command(command, argv, MAX_ARGS);
        
        // 执行命令并获取结果
        int result = execute_command(argc, argv);
        
        // 记录命令历史
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

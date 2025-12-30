#include "command.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * agent命令实现 - 启动小晨AI终端助手
 * 
 * 功能说明：
 * - 无参数：启动交互式AI助手
 * - 带参数：将所有参数作为一条指令发送给AI助手
 * 
 * 使用示例：
 * - agent                    # 启动交互式模式
 * - agent 帮我创建一个文件   # 直接执行指令
 * 
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 命令执行状态（0表示成功，1表示失败）
 */
int cmd_agent(int argc, char *argv[]) {
    // 获取项目根目录路径
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        error("无法获取当前目录");
        return 1;
    }

    // 检查 xiaochen_agent_v2 目录是否存在
    char agent_path[1280];
    snprintf(agent_path, sizeof(agent_path), "%s/xiaochen_agent_v2", cwd);
    
    if (access(agent_path, F_OK) != 0) {
        error("未找到 xiaochen_agent_v2 目录");
        printf("请确保 xiaochen_agent_v2 目录存在于当前项目根目录下\n");
        return 1;
    }

    // 检查 Python 是否可用
    if (system("python --version > nul 2>&1") != 0 && 
        system("python3 --version > nul 2>&1") != 0) {
        error("未找到 Python 解释器");
        printf("请安装 Python 3.x 后再使用 agent 命令\n");
        return 1;
    }

    // 确定使用 python 还是 python3
    char python_cmd[32] = "python";
    if (system("python --version > nul 2>&1") != 0) {
        strcpy(python_cmd, "python3");
    }


    // 检查并安装依赖
    printf("\033[34m正在检查 Python 依赖...\033[0m\n");
    char check_cmd[1280];
    snprintf(check_cmd, sizeof(check_cmd), 
             "cd %s && %s xiaochen_agent_v2/check_deps.py", 
             cwd, python_cmd);
    
    int check_result = system(check_cmd);
    if (check_result != 0) {
        error("依赖检查失败");
        return 1;
    }
    
    printf("\n");

    // 构建命令
    char command[4096];
    
    if (argc == 1) {
        // 无参数：启动交互式模式
        printf("\033[34m正在启动小晨AI终端助手...\033[0m\n");
        printf("\033[34m提示：输入 'exit' 或 'quit' 退出助手\033[0m\n\n");
        
        snprintf(command, sizeof(command), 
                 "cd %s && %s -m xiaochen_agent_v2", 
                 cwd, python_cmd);
    } else {
        // 有参数：将所有参数合并为一条指令
        // 构建参数字符串
        char full_query[3072] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) strcat(full_query, " ");
            strcat(full_query, argv[i]);
        }
        
        printf("\033[34m正在启动小晨AI终端助手...\033[0m\n");
        printf("\033[34m指令: %s\033[0m\n\n", full_query);
        
        // 使用 run_once.py 脚本执行单条指令（更安全，避免字符串转义问题）
        snprintf(command, sizeof(command), 
                 "cd %s && %s xiaochen_agent_v2/run_once.py \"%s\"", 
                 cwd, python_cmd, full_query);
    }

    // 执行命令
    int result = system(command);
    
    if (result != 0) {
        printf("\n\033[31m小晨AI终端助手执行出错\033[0m\n");
        printf("请检查：\n");
        printf("1. Python 是否正确安装\n");
        printf("2. xiaochen_agent_v2 依赖是否安装（requests 等）\n");
        printf("3. 环境变量 VOID_API_KEY 是否已设置\n");
        return 1;
    }
    
    printf("\n\033[34m小晨AI终端助手已退出\033[0m\n");
    return 0;
}


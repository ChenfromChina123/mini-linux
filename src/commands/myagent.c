#include "command.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

/**
 * agent命令实现 - 启动小晨AI终端助手
 * 
 * 功能说明：
 * - 直接调用打包好的xiaochen_agent可执行文件
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
    char agent_executable[2048];
    char cwd[1024];
    
    // 获取当前目录
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        error("无法获取当前目录");
        return 1;
    }

    // 查找xiaochen_agent可执行文件
    // 优先级：./bin/xiaochen_agent -> ../bin/xiaochen_agent
    if (access("./bin/xiaochen_agent", X_OK) == 0) {
        snprintf(agent_executable, sizeof(agent_executable), "./bin/xiaochen_agent");
    } else if (access("../bin/xiaochen_agent", X_OK) == 0) {
        snprintf(agent_executable, sizeof(agent_executable), "../bin/xiaochen_agent");
    } else if (access("bin/xiaochen_agent", X_OK) == 0) {
        snprintf(agent_executable, sizeof(agent_executable), "bin/xiaochen_agent");
    } else {
        error("未找到 xiaochen_agent 可执行文件");
        printf("请先运行 'make agent' 构建Agent可执行文件\n");
        printf("或确保 bin/xiaochen_agent 存在且可执行\n");
        return 1;
    }

    // 构建命令
    char command[4096];
    
    if (argc == 1) {
        // 无参数：启动交互式模式
        printf("\033[34m正在启动小晨AI终端助手...\033[0m\n");
        printf("\033[34m提示：输入 'exit' 或 'quit' 退出助手\033[0m\n\n");
        
        snprintf(command, sizeof(command), "%s", agent_executable);
    } else {
        // 有参数：将所有参数合并为一条指令
        char full_query[3072] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) strcat(full_query, " ");
            strcat(full_query, argv[i]);
        }
        
        printf("\033[34m正在启动小晨AI终端助手...\033[0m\n");
        printf("\033[34m指令: %s\033[0m\n\n", full_query);
        
        // 传递参数给可执行文件
        snprintf(command, sizeof(command), 
                 "%s \"%s\"", 
                 agent_executable, full_query);
    }

    // 执行命令
    int result = system(command);
    
    if (result != 0) {
        printf("\n\033[31m小晨AI终端助手执行出错\033[0m\n");
        printf("请检查：\n");
        printf("1. 是否已运行 'make agent' 构建可执行文件\n");
        printf("2. 环境变量 VOID_API_KEY 是否已设置\n");
        printf("3. 可执行文件是否有执行权限\n");
        return 1;
    }
    
    printf("\n\033[34m小晨AI终端助手已退出\033[0m\n");
    return 0;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 myagent）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_agent(argc, argv);
}
#endif


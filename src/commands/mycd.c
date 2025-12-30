#include "command.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// mycd命令实现：切换当前工作目录
// 用法：mycd [目录]
// - 无参数：切换到用户主目录（环境变量 HOME 或 USERPROFILE）
// - 支持以 ~ 开头的路径（仅 ~ 或 ~/...）
int cmd_mycd(int argc, char *argv[]) {

    // 处理参数
    const char *target = NULL;
    if (argc < 2) {
        // 切换到 HOME
        target = getenv("HOME");
        if (target == NULL) {
            error("无法找到用户主目录");
            return 1;
        }
    } else {
        target = argv[1];
        // 处理 ~ 展开
        if (target[0] == '~') {
            const char *home = getenv("HOME");
            if (home == NULL) {
                error("无法展开 ~：找不到 HOME");
                return 1;
            }
            if (target[1] == '\0') {
                target = home;
            } else if (target[1] == '/' || target[1] == '\\') {
                // 需要构造新的路径字符串
                size_t len = strlen(home) + strlen(target) + 1;
                char *buf = (char *)malloc(len);
                if (buf == NULL) {
                    error("内存分配失败");
                    return 1;
                }
                // home + (target+1)
                strcpy(buf, home);
                strcat(buf, target + 1);
                int rc = chdir(buf);
                free(buf);
                if (rc != 0) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "切换目录失败: %s", strerror(errno));
                    error(msg);
                    return 1;
                }
                return 0;
            } else {
                // ~username 不支持
                error("不支持的路径格式");
                return 1;
            }
        }
    }

    // 直接 chdir
    //chdir(target) 用于切换当前工作目录到指定路径。
    //!= 0 检查是否切换成功。   
    if (chdir(target) != 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "切换目录失败: %s", strerror(errno));
        error(msg);
        return 1;
    }

    return 0;
}

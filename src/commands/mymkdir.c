#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


//函数说明：递归创建路径中的所有父目录。
//参数：
//  - path：要创建的目录路径
//返回值：
//  - 成功：0
//  - 失败：-1
static int make_parents(const char *path) {
    if (path == NULL || *path == '\0') return -1;

    char *tmp = strdup(path);
    if (tmp == NULL) return -1;

    size_t len = strlen(tmp);
    // 去除尾部多余的 '/'
    while (len > 0 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
        tmp[len - 1] = '\0';
        len--;
    }

    for (size_t i = 1; i < len; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            tmp[i] = '\0';
            // 跳过根目录位点
            if (strlen(tmp) > 0) {
                if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                    free(tmp);
                    return -1;
                }
            }
            tmp[i] = '/';
        }
    }

    // 最后创建完整路径
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        free(tmp);
        return -1;
    }

    free(tmp);
    return 0;
}

// mymkdir 命令实现
// 用法: mymkdir [-p] <目录>
// -p：递归创建父目录（类似 mkdir -p）
// mymkdir工作流程：
// 1. 检查参数：确保至少有一个目录参数。
// 2. 解析选项：检查是否包含 -p 选项。
// 3. 递归创建父目录（如果 -p 选项存在）。
// 4. 创建目录：根据路径创建目录。
// 5. 打印成功消息：告知用户目录已成功创建。
int cmd_mymkdir(int argc, char *argv[]) {
    int make_parents_flag = 0;
    const char *path = NULL;

    if (argc < 2) {
        error("使用方法: mymkdir [-p] <目录>");
        return 1;
    }

    int idx = 1;
    if (argc >= 2 && strcmp(argv[1], "-p") == 0) {
        make_parents_flag = 1;
        idx = 2;
    }

    if (idx >= argc) {
        error("使用方法: mymkdir [-p] <目录>");
        return 1;
    }

    path = argv[idx];

    if (make_parents_flag) {
        if (make_parents(path) != 0) {
            char msg[256];
            snprintf(msg, sizeof(msg), "创建目录失败: %s", strerror(errno));
            error(msg);
            return 1;
        }
        success("目录创建成功");
        return 0;
    } else {
        if (mkdir(path, 0755) != 0) {
            char msg[256];
            snprintf(msg, sizeof(msg), "创建目录失败: %s", strerror(errno));
            error(msg);
            return 1;
        }
        success("目录创建成功");
        return 0;
    }
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 mymkdir）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_mymkdir(argc, argv);
}
#endif

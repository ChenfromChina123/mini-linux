#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// 递归创建父目录（类似 mkdir -p）
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
            // 跳过根目录位点（例如 Windows 的 C:）
            if (strlen(tmp) > 0) {
#ifdef _WIN32
                if (mkdir(tmp, 0) != 0 && errno != EEXIST) {
                    free(tmp);
                    return -1;
                }
#else
                if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                    free(tmp);
                    return -1;
                }
#endif
            }
            tmp[i] = '/';
        }
    }

    // 最后创建完整路径
#ifdef _WIN32
    if (mkdir(tmp, 0) != 0 && errno != EEXIST) {
        free(tmp);
        return -1;
    }
#else
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        free(tmp);
        return -1;
    }
#endif

    free(tmp);
    return 0;
}

// mymkdir 命令实现
// 用法: mymkdir [-p] <目录>
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
#ifdef _WIN32
        if (mkdir(path, 0) != 0) {
#else
        if (mkdir(path, 0755) != 0) {
#endif
            char msg[256];
            snprintf(msg, sizeof(msg), "创建目录失败: %s", strerror(errno));
            error(msg);
            return 1;
        }
        success("目录创建成功");
        return 0;
    }
}

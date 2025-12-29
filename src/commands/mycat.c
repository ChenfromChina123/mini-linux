#include "command.h"
#include "util.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
//mycat工作流程：
// 1. 解析路径：支持 ~ 展开，并把相对路径转换为绝对路径。
// 2. 打开文件：使用 fopen 打开文件，检查是否成功。
// 3. 读取文件内容：使用 fgets 逐行读取文件内容，避免直接输出到标准输出。
// 4. 输出文件内容：使用 printf 输出文件内容。
// 5. 关闭文件：使用 fclose 关闭文件。
// 返回 0 成功，非 0 失败。
/**
 * @brief 解析并规范化文件路径
 * @param path 输入路径（支持 ~）
 * @param out 输出的绝对路径缓冲区
 * @param outlen 缓冲区大小
 * @return 成功返回0，失败返回-1
 */
static int resolve_path(const char *path, char *out, size_t outlen) {
    if (path == NULL || out == NULL) return -1;

    char tmp[PATH_MAX];

    // 展开 ~ 到用户主目录
    if (path[0] == '~') {
        const char *home = getenv("HOME");
#ifdef _WIN32
        if (home == NULL) home = getenv("USERPROFILE");
#endif
        if (home == NULL) return -1;
        if (path[1] == '\0' || path[1] == '/' || path[1] == '\\') {//家目录打开请求操作
            // ~ 或 ~/something
            snprintf(tmp, sizeof(tmp), "%s%s", home, path + 1);
        } else {
            // ~username 的情形不处理
            return -1;
        }
    } else {
        // 不是 ~ 开头，直接拷贝
        strncpy(tmp, path, sizeof(tmp) - 1);//采用strncpy控制拷贝，防止溢出
        tmp[sizeof(tmp) - 1] = '\0';
    }

#ifdef _WIN32// windows系统下，采用_fullpath转换相对路径为绝对路径
    // _fullpath 会把相对路径转换为绝对路径
    if (_fullpath(out, tmp, outlen) == NULL) return -1;//tmp输入路径，out输出路径，outlen输出路径缓冲区大小
#else
    // 在 POSIX 下，realpath 对于不存在的文件会失败，因此先尝试 realpath，失败则手动拼接 cwd
    if (realpath(tmp, out) == NULL) {
        if (tmp[0] == '/') {
            // 绝对路径但 realpath 失败（可能文件不存在），直接返回拷贝
            if (strlen(tmp) >= outlen) return -1;
            strncpy(out, tmp, outlen - 1);
            out[outlen - 1] = '\0';
        } else {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) == NULL) return -1;
            if ((size_t)snprintf(out, outlen, "%s/%s", cwd, tmp) >= outlen) return -1;
        }
    }
#endif

    return 0;
}

/**
 * @brief 报告带有路径信息的错误
 * @param prefix 错误前缀
 * @param path 涉及的文件路径
 */
static void report_error_with_path(const char *prefix, const char *path) {
    if (prefix == NULL) {
        if (path) error(path);
        return;
    }
    if (path == NULL) {
        error(prefix);
        return;
    }
    size_t plen = strlen(prefix);
    size_t slen = strlen(path);
    size_t len = plen + slen + 1;
    char *msg = (char *)malloc(len);
    if (msg == NULL) {
        // allocation failed, fallback to prefix only
        error(prefix);
        return;
    }
    memcpy(msg, prefix, plen);
    memcpy(msg + plen, path, slen);
    msg[plen + slen] = '\0';
    error(msg);
    free(msg);
}

/**
 * @brief mycat 命令实现
 * 读取并显示文件内容
 */
int cmd_mycat(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: mycat <文件名> [文件名...]");
        return 1;
    }
    
    // 处理每个文件，支持 ~ 展开以及相对/绝对路径解析
    for (int i = 1; i < argc; i++) {
        char resolved_path[PATH_MAX];
        // 解析并规范化路径（展开 ~，将相对路径转换为绝对路径）
        if (resolve_path(argv[i], resolved_path, sizeof(resolved_path)) != 0) {
            report_error_with_path("无法解析路径: ", argv[i]);
            continue;
        }

        FILE *file = fopen(resolved_path, "r");
        if (file == NULL) {
            report_error_with_path("无法打开文件: ", resolved_path);
            continue;
        }

        // 读取文件内容并输出
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);//使用buffer输出文件内容，避免直接输出到标准输出
        }

        fclose(file);
    }
    
    return 0;
}

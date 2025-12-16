#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// mycp命令实现：复制文件
int cmd_mycp(int argc, char *argv[]) {
    if (argc < 3) {
        error("使用方法: mycp <源文件> <目标文件>");
        return 1;
    }

    const char *src_file = argv[1];
    const char *dest_file = argv[2];

    // 打开源文件进行读取
    FILE *src = fopen(src_file, "r");
    if (src == NULL) {
        char msg[256];
        snprintf(msg, sizeof(msg), "无法打开源文件: %s", strerror(errno));
        error(msg);
        return 1;
    }

    // 如果目标是目录（或以斜杠结尾），则把源文件名附加到目标目录
    char final_dest[PATH_MAX];
    struct stat st;
    int dest_is_dir = 0;

    // 目标以 '/' 或 '\\' 结尾，认为是目录
    size_t dlen = strlen(dest_file);
    if (dlen > 0 && (dest_file[dlen - 1] == '/' || dest_file[dlen - 1] == '\\')) {
        dest_is_dir = 1;
    } else if (stat(dest_file, &st) == 0) {
        if (S_ISDIR(st.st_mode)) dest_is_dir = 1;
    }

    if (dest_is_dir) {
        // 确认目录存在
        if (stat(dest_file, &st) != 0 || !S_ISDIR(st.st_mode)) {
            error("目标目录不存在");
            fclose(src);
            return 1;
        }

        // 取源文件名的 basename
        const char *p = src_file;
        const char *base = p + strlen(p);
        while (base > p && *(base - 1) != '/' && *(base - 1) != '\\') base--;

        if ((size_t)snprintf(final_dest, sizeof(final_dest), "%s/%s", dest_file, base) >= sizeof(final_dest)) {
            error("目标路径过长");
            fclose(src);
            return 1;
        }
    } else {
        // 目标不是目录，直接使用
        if (strlen(dest_file) >= sizeof(final_dest)) {
            error("目标路径过长");
            fclose(src);
            return 1;
        }
        strcpy(final_dest, dest_file);
    }

    // 打开目标文件进行写入（如果父目录不存在，fopen 会失败）
    FILE *dest = fopen(final_dest, "w");
    if (dest == NULL) {
        char msg[256];
        snprintf(msg, sizeof(msg), "无法打开目标文件: %s", strerror(errno));
        error(msg);
        fclose(src);
        return 1;
    }

    // 复制文件内容
    char buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            error("写入目标文件失败");
            fclose(src);
            fclose(dest);
            return 1;
        }
    }

    // 检查源文件读取是否出错
    if (ferror(src)) {
        error("读取源文件失败");
        fclose(src);
        fclose(dest);
        return 1;
    }

    fclose(src);
    fclose(dest);

    printf("已复制文件: %s -> %s\n", src_file, final_dest);
    return 0;
}

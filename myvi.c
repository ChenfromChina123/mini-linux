#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 一个简单的行编辑器：将文件读入内存（行数组），提供基本命令来查看/插入/删除/替换行，
// 支持命令：
// :q      - 不保存退出
// :w      - 保存
// :wq     - 保存并退出
// :p      - 打印全部
// :p N    - 打印第 N 行（从1开始）
// :i N    - 在第 N 行之前插入（不带 N 则在末尾插入），随后输入多行，单独一行 "." 结束输入
// :a N    - 在第 N 行之后插入（不带 N 则在末尾插入）
// :d N    - 删除第 N 行
// :s N txt- 将第 N 行替换为 txt（txt 可包含空格）

static void print_help() {
    printf("可用命令：\n");
    printf(":q         不保存退出\n");
    printf(":w         保存\n");
    printf(":wq        保存并退出\n");
    printf(":p         打印所有行\n");
    printf(":p N       打印第 N 行\n");
    printf(":i N       在第 N 行之前插入（输入多行，单独一行 '.' 结束）\n");
    printf(":a N       在第 N 行之后插入（输入多行，单独一行 '.' 结束）\n");
    printf(":d N       删除第 N 行\n");
    printf(":s N text  将第 N 行替换为 text\n");
}

int cmd_myvi(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: myvi <文件名>");
        return 1;
    }

    const char *filename = argv[1];

    // 动态行数组
    size_t cap = 128;
    size_t line_count = 0;
    char **lines = (char **)malloc(sizeof(char *) * cap);
    if (lines == NULL) {
        error("内存分配失败");
        return 1;
    }

    // 读取文件到内存
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), f) != NULL) {
            // 移除结尾换行
            size_t L = strlen(buf);
            if (L > 0 && (buf[L-1] == '\n' || buf[L-1] == '\r')) buf[--L] = '\0';
            // 扩容
            if (line_count >= cap) {
                cap *= 2;
                char **tmp = realloc(lines, sizeof(char *) * cap);
                if (tmp == NULL) break;
                lines = tmp;
            }
            lines[line_count] = strdup(buf);
            if (lines[line_count] == NULL) break;
            line_count++;
        }
        fclose(f);
        printf("\033[34m已载入文件：%s（%zu 行）\033[0m\n", filename, line_count);
    } else {
        printf("\033[34m创建新文件：%s\033[0m\n", filename);
    }

    // 交互式命令循环
    char cmd[4096];
    int modified = 0;
    while (1) {
        printf("myvi> ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) break;
        // trim newline
        size_t L = strlen(cmd);
        if (L > 0 && (cmd[L-1] == '\n' || cmd[L-1] == '\r')) cmd[--L] = '\0';

        if (L == 0) continue;

        if (cmd[0] != ':') {
            printf("请输入以 ':' 开头的命令，输入 ':q' 退出或 ':w' 保存。输入 ':h' 查看帮助。\n");
            continue;
        }

        // 解析命令
        if (strcmp(cmd, ":q") == 0) {
            // 退出不保存
            break;
        } else if (strcmp(cmd, ":w") == 0) {
            // 保存
            FILE *out = fopen(filename, "w");
            if (out == NULL) {
                error("无法打开文件写入");
            } else {
                for (size_t i = 0; i < line_count; i++) fprintf(out, "%s\n", lines[i]);
                fclose(out);
                modified = 0;
                success("文件已保存");
            }
        } else if (strcmp(cmd, ":wq") == 0) {
            FILE *out = fopen(filename, "w");
            if (out == NULL) {
                error("无法打开文件写入");
            } else {
                for (size_t i = 0; i < line_count; i++) fprintf(out, "%s\n", lines[i]);
                fclose(out);
                success("文件已保存");
                break;
            }
        } else if (strcmp(cmd, ":p") == 0) {
            for (size_t i = 0; i < line_count; i++) {
                printf("%6zu: %s\n", i + 1, lines[i]);
            }
        } else if (strncmp(cmd, ":p ", 3) == 0) {
            int n = atoi(cmd + 3);
            if (n >= 1 && (size_t)n <= line_count) printf("%6d: %s\n", n, lines[n-1]);
            else printf("行号超出范围\n");
        } else if (strncmp(cmd, ":d ", 3) == 0) {
            int n = atoi(cmd + 3);
            if (n >= 1 && (size_t)n <= line_count) {
                free(lines[n-1]);
                for (size_t i = n-1; i < line_count - 1; i++) lines[i] = lines[i+1];
                line_count--;
                modified = 1;
                success("已删除行");
            } else {
                printf("行号超出范围\n");
            }
        } else if (strncmp(cmd, ":s ", 3) == 0) {
            // :s N text
            char *p = cmd + 3;
            while (*p == ' ') p++;
            int n = atoi(p);
            while (*p && *p != ' ') p++;
            while (*p == ' ') p++;
            if (n >= 1 && (size_t)n <= line_count) {
                free(lines[n-1]);
                lines[n-1] = strdup(p);
                if (lines[n-1] == NULL) { error("内存分配失败"); break; }
                modified = 1;
                success("已替换行");
            } else {
                printf("行号超出范围\n");
            }
        } else if (strncmp(cmd, ":i", 2) == 0 || strncmp(cmd, ":a", 2) == 0) {
            int insert_before = (cmd[1] == 'i');
            char *p = cmd + 2;
            while (*p == ' ') p++;
            int n = 0;
            if (*p != '\0') n = atoi(p);
            size_t pos;
            if (n <= 0) pos = line_count; // 末尾
            else if ((size_t)n > line_count) pos = line_count; // append
            else pos = (insert_before ? (size_t)(n - 1) : (size_t)n);

            printf("输入多行（单独一行 '.' 结束）：\n");
            char buf[4096];
            size_t ins_count = 0;
            char **ins_lines = NULL;
            while (1) {
                if (fgets(buf, sizeof(buf), stdin) == NULL) break;
                size_t LL = strlen(buf);
                if (LL > 0 && (buf[LL-1] == '\n' || buf[LL-1] == '\r')) buf[--LL] = '\0';
                if (strcmp(buf, ".") == 0) break;
                char *ln = strdup(buf);
                if (ln == NULL) { error("内存分配失败"); break; }
                char **tmp = realloc(ins_lines, sizeof(char *) * (ins_count + 1));
                if (tmp == NULL) { free(ln); error("内存分配失败"); break; }
                ins_lines = tmp;
                ins_lines[ins_count++] = ln;
            }
            if (ins_count > 0) {
                // 扩展主数组
                while (line_count + ins_count > cap) {
                    cap *= 2;
                    char **tmp = realloc(lines, sizeof(char *) * cap);
                    if (tmp == NULL) { error("内存分配失败"); break; }
                    lines = tmp;
                }
                // 移动后半部分
                for (size_t i = line_count; i > pos; i--) lines[i + ins_count - 1] = lines[i - 1];
                // 插入
                for (size_t i = 0; i < ins_count; i++) lines[pos + i] = ins_lines[i];
                line_count += ins_count;
                modified = 1;
                success("已插入行");
            }
            free(ins_lines);
        } else if (strcmp(cmd, ":h") == 0 || strcmp(cmd, ":help") == 0) {
            print_help();
        } else {
            printf("未知命令。输入 ':h' 查看帮助。\n");
        }
    }

    // 释放内存
    for (size_t i = 0; i < line_count; i++) free(lines[i]);
    free(lines);

    if (modified) printf("注意：已修改但未保存。使用 ':w' 保存。\n");

    return 0;
}

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

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

// 进入插入模式的 helper 函数
static void enter_insert_mode(size_t position, int before, char ***lines_ptr, size_t *line_count_ptr, size_t *cap_ptr) {
    struct termios orig, raw;
    int use_raw = 0;

    char **lines = *lines_ptr;
    size_t line_count = *line_count_ptr;
    size_t cap = *cap_ptr;

    if (tcgetattr(STDIN_FILENO, &orig) == 0) {
        raw = orig;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        use_raw = 1;
    }

    printf("-- INSERT -- (按 ESC 退出插入模式)\n");

    char curbuf[8192];
    size_t cur_len = 0;
    int ch;

    while (1) {
        ch = getchar();
        if (ch == 27) { // ESC
            break;
        } else if (ch == '\n' || ch == '\r') {
            curbuf[cur_len] = '\0';
            // 插入一行到 position 或 position+1
            if (before) {
                while (line_count + 1 > cap) {
                    cap *= 2;
                    char **tmp = realloc(lines, sizeof(char *) * cap);
                    if (tmp == NULL) { error("内存分配失败"); break; }
                    lines = tmp;
                }
                for (size_t i = line_count; i > position; i--) lines[i] = lines[i-1];
                lines[position] = strdup(curbuf);
                line_count++;
                position++;
            } else {
                while (line_count + 1 > cap) {
                    cap *= 2;
                    char **tmp = realloc(lines, sizeof(char *) * cap);
                    if (tmp == NULL) { error("内存分配失败"); break; }
                    lines = tmp;
                }
                for (size_t i = line_count; i > position + 1; i--) lines[i] = lines[i-1];
                lines[position+1] = strdup(curbuf);
                line_count++;
                position++;
            }
            cur_len = 0;
            memset(curbuf, 0, sizeof(curbuf));
        } else if (ch == 127 || ch == 8) { // backspace
            if (cur_len > 0) cur_len--;
        } else {
            if (cur_len + 1 < sizeof(curbuf)) curbuf[cur_len++] = (char)ch;
        }
    }

    if (use_raw) tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);

    *lines_ptr = lines;
    *line_count_ptr = line_count;
    *cap_ptr = cap;
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

    // vi-like 模式：命令模式 + 插入模式（支持 Esc 退出插入）

    size_t cur = 0; // 当前行
    int modified_flag = 0;

    char cmdline[4096];
    while (1) {
        // 显示状态行
        printf("-- COMMAND -- 行 %zu/%zu\n", cur + 1, line_count);
        if (cur < line_count) printf("%s\n", lines[cur]); else printf("~\n");
        printf(":");
        if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) break;
        size_t CL = strlen(cmdline);
        if (CL > 0 && (cmdline[CL-1] == '\n' || cmdline[CL-1] == '\r')) cmdline[--CL] = '\0';

        if (CL == 0) continue;

        // 单字符命令常用快捷键
        if (strcmp(cmdline, "i") == 0) {
            enter_insert_mode(cur, 1, &lines, &line_count, &cap);
            modified_flag = 1;
            continue;
        } else if (strcmp(cmdline, "a") == 0) {
            if (cur < line_count) enter_insert_mode(cur, 0, &lines, &line_count, &cap);
            else enter_insert_mode(line_count, 0, &lines, &line_count, &cap);
            modified_flag = 1;
            continue;
        } else if (strcmp(cmdline, "o") == 0) {
            // 在当前行之后开新行并进入插入
            enter_insert_mode(cur, 0, &lines, &line_count, &cap);
            modified_flag = 1;
            continue;
        } else if (strcmp(cmdline, "j") == 0) {
            if (cur + 1 < line_count) cur++; continue;
        } else if (strcmp(cmdline, "k") == 0) {
            if (cur > 0) cur--; continue;
        } else if (strcmp(cmdline, "dd") == 0) {
            if (cur < line_count) {
                free(lines[cur]);
                for (size_t i = cur; i < line_count - 1; i++) lines[i] = lines[i+1];
                line_count--;
                if (cur >= line_count && line_count > 0) cur = line_count - 1;
                modified_flag = 1;
            }
            continue;
        }

        // 冒号命令（:w, :q, :wq, :p, :h, :s N text, :d N, :i N, :a N）
        if (cmdline[0] == ':') {
            if (strcmp(cmdline, ":q") == 0) {
                break;
            } else if (strcmp(cmdline, ":w") == 0) {
                FILE *out = fopen(filename, "w");
                if (out == NULL) { error("无法打开文件写入"); }
                else {
                    for (size_t i = 0; i < line_count; i++) fprintf(out, "%s\n", lines[i]);
                    fclose(out); modified_flag = 0; success("文件已保存");
                }
            } else if (strcmp(cmdline, ":wq") == 0) {
                FILE *out = fopen(filename, "w");
                if (out == NULL) { error("无法打开文件写入"); }
                else {
                    for (size_t i = 0; i < line_count; i++) fprintf(out, "%s\n", lines[i]);
                    fclose(out);
                    modified_flag = 0;
                    success("文件已保存");
                    break;
                }
            } else if (strcmp(cmdline, ":p") == 0) {
                for (size_t i = 0; i < line_count; i++) printf("%6zu: %s\n", i+1, lines[i]);
            } else if (strncmp(cmdline, ":d ", 3) == 0) {
                int n = atoi(cmdline + 3);
                if (n>=1 && (size_t)n<=line_count) {
                    free(lines[n-1]); for (size_t i=n-1;i<line_count-1;i++) lines[i]=lines[i+1]; line_count--; if (cur>=line_count && line_count>0) cur=line_count-1; modified_flag=1;
                } else printf("行号超出范围\n");
            } else if (strncmp(cmdline, ":s ", 3) == 0) {
                char *p = cmdline + 3; while (*p==' ') p++; int n = atoi(p); while (*p && *p!=' ') p++; while (*p==' ') p++; if (n>=1 && (size_t)n<=line_count) { free(lines[n-1]); lines[n-1]=strdup(p); modified_flag=1; success("已替换行"); } else printf("行号超出范围\n");
            } else if (strncmp(cmdline, ":i ", 3) == 0) {
                int n = atoi(cmdline+3); size_t pos = n<=0?line_count: (size_t)(n-1); enter_insert_mode(pos,1,&lines,&line_count,&cap); modified_flag=1;
            } else if (strncmp(cmdline, ":a ", 3) == 0) {
                int n = atoi(cmdline+3); size_t pos = n<=0?line_count:(size_t)(n-1); enter_insert_mode(pos,0,&lines,&line_count,&cap); modified_flag=1;
            } else if (strcmp(cmdline, ":h") == 0 || strcmp(cmdline, ":help") == 0) {
                print_help();
            } else {
                printf("未知命令\n");
            }
            continue;
        }
    }

    // 释放内存
    for (size_t i = 0; i < line_count; i++) free(lines[i]);
    free(lines);

    if (modified_flag) printf("注意：已修改但未保存。使用 ':w' 保存。\n");

    return 0;
}

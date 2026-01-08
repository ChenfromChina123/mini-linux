#include "command.h"
#include "util.h"

// myecho命令实现：向文件写入内容
// myecho工作流程：
// 1. 检查参数：确保至少有一个内容参数和一个重定向符号。
// 2. 查找重定向符号：确定是覆盖写入还是追加写入。
// 3. 打开文件：根据重定向符号打开指定的文件。
// 4. 写入内容：将参数中的内容写入文件，每个参数之间用空格分隔。
// 5. 关闭文件：写入完成后关闭文件。
// 6. 打印成功消息：告知用户内容已成功写入文件。
int cmd_myecho(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: myecho <内容> > <文件名> 或 myecho <内容> >> <文件名>");
        return 1;
    }
    
    // 查找重定向符号
    int redirect_pos = -1;
    int append_mode = 0; // 0表示覆盖写入，1表示追加写入
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">>") == 0) {
            redirect_pos = i;
            append_mode = 1;
            break;
        } else if (strcmp(argv[i], ">") == 0) {
            redirect_pos = i;
            append_mode = 0;
            break;
        }
    }
    
    if (redirect_pos == -1) {
        // 如果没有重定向符号，则输出到标准输出 (stdout)
        for (int i = 1; i < argc; i++) {
            printf("%s", argv[i]);
            if (i < argc - 1) printf(" ");
        }
        printf("\n");
        return 0;
    }

    if (redirect_pos == argc - 1) {
        error("错误: 重定向符号后缺少文件名");
        return 1;
    }
    
    const char *filename = argv[redirect_pos + 1];
    
    // 打开文件
    FILE *file = NULL;
    if (append_mode) {
        file = fopen(filename, "a");
    } else {
        file = fopen(filename, "w");
    }
    
    if (file == NULL) {
        error("无法打开文件进行写入");
        return 1;
    }
    
    // 写入内容
    for (int i = 1; i < redirect_pos; i++) {
        fprintf(file, "%s", argv[i]);
        if (i < redirect_pos - 1) {
            fprintf(file, " ");
        }
    }
    fprintf(file, "\n");
    
    fclose(file);
    printf("内容已写入文件: %s\n", filename);
    
    return 0;
}

#ifdef MINI_LINUX_STANDALONE
/**
 * @brief 独立可执行程序入口（用于单独编译 myecho）
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 进程退出码
 */
int main(int argc, char *argv[]) {
    return cmd_myecho(argc, argv);
}
#endif

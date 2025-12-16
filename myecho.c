#include "command.h"
#include "util.h"

// myecho命令实现：向文件写入内容
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
    
    if (redirect_pos == -1 || redirect_pos == argc - 1) {
        error("使用方法: myecho <内容> > <文件名> 或 myecho <内容> >> <文件名>");
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

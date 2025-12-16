#include "command.h"
#include "util.h"

// myvi命令实现：简单的文件编辑器
int cmd_myvi(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: myvi <文件名>");
        return 1;
    }
    
    const char *filename = argv[1];
    
    // 显示当前文件内容（如果存在）
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        printf("\033[34m当前文件内容：\033[0m\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }
        fclose(file);
    } else {
        printf("\033[34m创建新文件：%s\033[0m\n", filename);
    }
    
    // 提示用户输入内容
    printf("\033[34m请输入内容（单独一行输入 '.' 结束）：\033[0m\n");
    
    // 打开文件进行写入
    file = fopen(filename, "w");
    if (file == NULL) {
        error("无法打开文件进行写入");
        return 1;
    }
    
    // 读取用户输入
    char line[1024];
    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        
        // 去除换行符
        trim(line);
        
        // 检查是否结束编辑
        if (strcmp(line, ".") == 0) {
            break;
        }
        
        // 写入文件
        fprintf(file, "%s\n", line);
    }
    
    fclose(file);
    printf("\033[32m文件已保存：%s\033[0m\n", filename);
    
    return 0;
}

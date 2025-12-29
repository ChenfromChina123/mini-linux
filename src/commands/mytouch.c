#include "command.h"
#include "util.h"

// mytouch命令实现：创建空文件
// mytouch工作流程：
// 1. 检查参数：确保至少有一个文件名参数。
// 2. 处理每个文件：
//主要函数：fopen
//    - 以追加模式打开文件，如果文件不存在则创建。
//    - 关闭文件。
//    - 打印成功消息：告知用户文件已成功创建。
int cmd_mytouch(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: mytouch <文件名> [文件名...]");
        return 1;
    }
    
    // 处理每个文件
    for (int i = 1; i < argc; i++) {
        // 以追加模式打开文件，如果文件不存在则创建
        FILE *file = fopen(argv[i], "a");
        if (file == NULL) {
            error("无法创建文件");
            continue;
        }
        fclose(file);
        printf("已创建文件: %s\n", argv[i]);
    }
    
    return 0;
}

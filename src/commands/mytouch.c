#include "command.h"
#include "util.h"

// mytouch命令实现：创建空文件
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

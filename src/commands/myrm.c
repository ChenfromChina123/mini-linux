#include "command.h"
#include "util.h"

// myrm命令实现：删除指定文件
int cmd_myrm(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: myrm <文件名> [文件名...]");
        return 1;
    }
    
    // 处理每个文件
    for (int i = 1; i < argc; i++) {
        if (remove(argv[i]) != 0) {
            error("删除文件失败");
            continue;
        }
        printf("已删除文件: %s\n", argv[i]);
    }
    
    return 0;
}

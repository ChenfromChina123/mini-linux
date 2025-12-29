#include "command.h"
#include "util.h"

// myrm命令实现：删除指定文件，不能删除目录
//myrm工作流程：
// 1. 检查参数：确保至少有一个文件名作为参数。
// 2. 循环处理每个文件：
//    a. 调用 remove 函数删除文件。
//    b. 如果删除成功，打印成功消息；如果失败，打印失败消息并继续处理下一个文件。
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

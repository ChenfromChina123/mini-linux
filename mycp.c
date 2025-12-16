#include "command.h"
#include "util.h"

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
        error("无法打开源文件");
        return 1;
    }
    
    // 打开目标文件进行写入
    FILE *dest = fopen(dest_file, "w");
    if (dest == NULL) {
        error("无法打开目标文件");
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
    
    printf("已复制文件: %s -> %s\n", src_file, dest_file);
    return 0;
}

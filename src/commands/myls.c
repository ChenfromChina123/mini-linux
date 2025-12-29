#include "command.h"
#include "util.h"
#include <dirent.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#endif
#include <time.h>

// 显示文件权限
void print_permissions(mode_t mode) {
    // 文件类型
    if (S_ISDIR(mode)) printf("d");
#ifdef S_ISLNK
    else if (S_ISLNK(mode)) printf("l");
#endif
    else if (S_ISREG(mode)) printf("-");
#ifdef S_ISBLK
    else if (S_ISBLK(mode)) printf("b");
#endif
#ifdef S_ISCHR
    else if (S_ISCHR(mode)) printf("c");
#endif
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode)) printf("p");
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode)) printf("s");
#endif
    
    // 用户权限
    printf("%c%c%c",
           (mode & S_IRUSR) ? 'r' : '-',
           (mode & S_IWUSR) ? 'w' : '-',
           (mode & S_IXUSR) ? 'x' : '-');
    
    // 组权限
#ifdef S_IRGRP
    printf("%c%c%c",
           (mode & S_IRGRP) ? 'r' : '-',
           (mode & S_IWGRP) ? 'w' : '-',
           (mode & S_IXGRP) ? 'x' : '-');
#else
    printf("---");
#endif
    
    // 其他用户权限
#ifdef S_IROTH
    printf("%c%c%c",
           (mode & S_IROTH) ? 'r' : '-',
           (mode & S_IWOTH) ? 'w' : '-',
           (mode & S_IXOTH) ? 'x' : '-');
#else
    printf("---");
#endif
}

// 显示目录内容
void list_directory(const char *dir_path, int long_format) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        error("无法打开目录");
        return;
    }
    
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过.和..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (long_format) {
            // 长格式显示
            struct stat file_stat;
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            
            if (stat(full_path, &file_stat) == -1) {
                continue;
            }
            
            // 权限
            print_permissions(file_stat.st_mode);
            
            // 硬链接数
            printf(" %2lu", (unsigned long)file_stat.st_nlink);
            
            // 用户名
#ifndef _WIN32
            struct passwd *pwd = getpwuid(file_stat.st_uid);
            printf(" %-8s", pwd ? pwd->pw_name : "unknown");
#else
            printf(" %-8s", "user");
#endif
            
            // 组名
#ifndef _WIN32
            struct group *grp = getgrgid(file_stat.st_gid);
            printf(" %-8s", grp ? grp->gr_name : "unknown");
#else
            printf(" %-8s", "group");
#endif
            
            // 文件大小
            printf(" %8ld", (long)file_stat.st_size);
            
            // 最后修改时间
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&file_stat.st_mtime));
            printf(" %s", time_str);
            
            // 文件名
            printf(" %s\n", entry->d_name);
        } else {
            // 短格式显示
            printf("%s\t", entry->d_name);
        }
    }
    
    if (!long_format) {
        printf("\n");
    }
    
    closedir(dir);
}

// myls命令实现：显示目录内容
int cmd_myls(int argc, char *argv[]) {
    int long_format = 0;
    const char *dir_path = ".";
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            long_format = 1;
        } else {
            dir_path = argv[i];
        }
    }
    
    // 检查路径是否存在且是目录
    struct stat path_stat;
    if (stat(dir_path, &path_stat) == -1) {
        error("目录不存在");
        return 1;
    }
    
    if (!S_ISDIR(path_stat.st_mode)) {
        // 如果不是目录，直接显示文件名
        printf("%s\n", dir_path);
        return 0;
    }
    
    // 显示目录内容
    list_directory(dir_path, long_format);
    
    return 0;
}

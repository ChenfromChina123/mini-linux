#include "shell.h"

/**
 * @brief Mini Linux Shell 程序入口
 * @return int 进程退出码
 */
int main(void) {
    shell_init();
    shell_loop();
    return 0;
}


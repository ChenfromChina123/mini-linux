#include "input.h"
#ifndef MINI_LINUX_STANDALONE
#include "history.h"
#endif
#include <unistd.h>

/**
 * @brief 启用终端原始模式
 */
int enable_raw_mode(struct termios *orig) {
    if (tcgetattr(STDIN_FILENO, orig) == -1) return 0;
    struct termios raw = *orig;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return 0;
    return 1;
}

/**
 * @brief 禁用终端原始模式
 */
void disable_raw_mode(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

/**
 * @brief 读取单个键码，识别转义序列
 */
int read_key_code() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1) return -1;
    }

    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '3': return 127; // Delete key handled as backspace for simplicity
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return KEY_UP;
                    case 'B': return KEY_DOWN;
                    case 'C': return KEY_RIGHT;
                    case 'D': return KEY_LEFT;
                }
            }
        }
        return '\x1b';
    }
    return c;
}

/**
 * @brief 带行编辑功能的输入读取
 */
int read_line_with_edit(const char *prompt, char *buf, size_t size) {
    struct termios orig;
    int raw_enabled = enable_raw_mode(&orig);
    
    size_t len = 0;
    size_t cur = 0;
#ifndef MINI_LINUX_STANDALONE
    int hist_idx = history_size();
#endif
    char saved_buf[1024] = {0};
    
    printf("%s", prompt);
    fflush(stdout);
    
    while (1) {
        int k = read_key_code();
        
        if (k == KEY_ENTER || k == '\n' || k == '\r') {
            buf[len] = '\0';
            putchar('\n');
            break;
        } else if (k == KEY_BACKSPACE || k == 8) {
            if (cur > 0) {
                cur--;
                for (size_t i = cur; i < len - 1; i++) buf[i] = buf[i+1];
                len--;
                buf[len] = '\0';
                
                // 更新显示：光标左移，清除后面内容，重新打印，光标回到原位
                printf("\b\033[K%s", &buf[cur]);
                for (size_t i = cur; i < len; i++) printf("\b");
                fflush(stdout);
            }
        } else if (k == KEY_LEFT) {
            if (cur > 0) {
                cur--;
                printf("\033[D");
                fflush(stdout);
            }
        } else if (k == KEY_RIGHT) {
            if (cur < len) {
                printf("\033[C");
                cur++;
                fflush(stdout);
            }
        } else if (k == KEY_UP || k == KEY_DOWN) {
#ifndef MINI_LINUX_STANDALONE
            // 历史记录逻辑
            if (hist_idx == history_size()) {
                strncpy(saved_buf, buf, sizeof(saved_buf)-1);
            }
            
            if (k == KEY_UP && hist_idx > 0) hist_idx--;
            else if (k == KEY_DOWN && hist_idx < history_size()) hist_idx++;
            
            const char *h = (hist_idx < history_size()) ? history_get_command(hist_idx) : saved_buf;
            if (h) {
                // 清除当前行
                while (cur > 0) { printf("\b"); cur--; }
                printf("\033[K");
                
                strncpy(buf, h, size - 1);
                buf[size - 1] = '\0';
                len = strlen(buf);
                cur = len;
                printf("%s", buf);
                fflush(stdout);
            }
#endif
        } else if (k >= 32 && k < 127) {
            if (len < size - 1) {
                for (size_t i = len; i > cur; i--) buf[i] = buf[i-1];
                buf[cur] = (char)k;
                len++;
                buf[len] = '\0';
                
                // 打印当前字符及后续字符，然后光标移回
                printf("%s", &buf[cur]);
                cur++;
                for (size_t i = cur; i < len; i++) printf("\b");
                fflush(stdout);
            }
        }
    }
    
    if (raw_enabled) disable_raw_mode(&orig);
    return (int)len;
}

#include "command.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef enum { MODE_NORMAL, MODE_INSERT, MODE_COMMAND, MODE_SEARCH } Mode;

enum { KEY_NONE = 0, KEY_UP = -1001, KEY_DOWN = -1002, KEY_LEFT = -1003, KEY_RIGHT = -1004 };

static int screen_rows = 24;
static int screen_cols = 80;

static void get_window_size() {
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_row > 0) screen_rows = ws.ws_row;
        if (ws.ws_col > 0) screen_cols = ws.ws_col;
    }
}

static void enable_raw(struct termios *orig, int *use_raw) {
    *use_raw = 0;
    if (tcgetattr(STDIN_FILENO, orig) == 0) {
        struct termios raw = *orig;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        *use_raw = 1;
    }
}

static void disable_raw(struct termios *orig, int use_raw) {
    if (use_raw) tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

static int read_key() {
    int c = getchar();
    if (c == 27) {
        int c1 = getchar();
        if (c1 == '[' || c1 == 'O') {
            int c2 = getchar();
            if (c2 >= '0' && c2 <= '9') {
                for (;;) {
                    int cx = getchar();
                    if (cx == 'A' || cx == 'B' || cx == 'C' || cx == 'D' || cx == '~') { c2 = cx; break; }
                }
            }
            if (c2 == 'A') return KEY_UP;
            if (c2 == 'B') return KEY_DOWN;
            if (c2 == 'C') return KEY_RIGHT;
            if (c2 == 'D') return KEY_LEFT;
        }
        return 27;
    }
    return c;
}

static void render(char **lines, size_t line_count, size_t top, size_t row, size_t col, int show_numbers, Mode mode, const char *status) {
    printf("\x1b[2J");
    printf("\x1b[H");
    size_t avail = screen_rows > 2 ? (size_t)(screen_rows - 2) : 0;
    for (size_t i = 0; i < avail; i++) {
        size_t li = top + i;
        if (li < line_count) {
            if (show_numbers) printf("%6zu ", li + 1);
            printf("%s", lines[li]);
        } else {
            if (show_numbers) printf("      ");
            printf("~");
        }
        printf("\r\n");
    }
    char mode_str[32];
    if (mode == MODE_INSERT) strcpy(mode_str, "INSERT"); else if (mode == MODE_COMMAND) strcpy(mode_str, ":"); else if (mode == MODE_SEARCH) strcpy(mode_str, "/"); else strcpy(mode_str, "NORMAL");
    printf("%s  %zu/%zu  col %zu  %s\r\n", mode_str, row + 1, line_count, col + 1, status ? status : "");
    size_t cy = row - top;
    size_t cx = (show_numbers ? 7 : 0) + col + 1;
    if (cy >= avail) cy = avail ? avail - 1 : 0;
    printf("\x1b[%zu;%zuH", cy + 1, cx);
    fflush(stdout);
}

static void ensure_cap(char ***lines_ptr, size_t *cap_ptr, size_t need) {
    char **lines = *lines_ptr;
    size_t cap = *cap_ptr;
    while (need > cap) {
        size_t ncap = cap ? cap * 2 : 128;
        char **tmp = realloc(lines, sizeof(char *) * ncap);
        if (!tmp) return;
        lines = tmp;
        cap = ncap;
    }
    *lines_ptr = lines;
    *cap_ptr = cap;
}

static void insert_line(char ***lines_ptr, size_t *line_count_ptr, size_t *cap_ptr, size_t pos, const char *text) {
    char **lines = *lines_ptr;
    size_t line_count = *line_count_ptr;
    size_t cap = *cap_ptr;
    ensure_cap(&lines, &cap, line_count + 1);
    for (size_t i = line_count; i > pos; i--) lines[i] = lines[i - 1];
    lines[pos] = strdup(text ? text : "");
    *lines_ptr = lines;
    *line_count_ptr = line_count + 1;
    *cap_ptr = cap;
}

static void delete_line(char ***lines_ptr, size_t *line_count_ptr, size_t pos, char **yank) {
    char **lines = *lines_ptr;
    size_t line_count = *line_count_ptr;
    if (pos >= line_count) return;
    if (yank) {
        if (*yank) free(*yank);
        *yank = strdup(lines[pos]);
    }
    free(lines[pos]);
    for (size_t i = pos; i + 1 < line_count; i++) lines[i] = lines[i + 1];
    *line_count_ptr = line_count - 1;
}

static void paste_line(char ***lines_ptr, size_t *line_count_ptr, size_t *cap_ptr, size_t pos, const char *yank) {
    if (!yank) return;
    insert_line(lines_ptr, line_count_ptr, cap_ptr, pos, yank);
}

static void insert_char(char **line_ptr, size_t col, char ch) {
    char *s = *line_ptr;
    size_t len = strlen(s);
    if (col > len) col = len;
    char *ns = (char *)malloc(len + 2);
    memcpy(ns, s, col);
    ns[col] = ch;
    memcpy(ns + col + 1, s + col, len - col + 1);
    free(s);
    *line_ptr = ns;
}

static void delete_char(char **line_ptr, size_t col) {
    char *s = *line_ptr;
    size_t len = strlen(s);
    if (col >= len) return;
    char *ns = (char *)malloc(len);
    memcpy(ns, s, col);
    memcpy(ns + col, s + col + 1, len - col);
    ns[len - 1] = '\0';
    free(s);
    *line_ptr = ns;
}

static void split_line(char ***lines_ptr, size_t *line_count_ptr, size_t *cap_ptr, size_t row, size_t col) {
    char **lines = *lines_ptr;
    size_t line_count = *line_count_ptr;
    size_t cap = *cap_ptr;
    char *s = lines[row];
    size_t len = strlen(s);
    if (col > len) col = len;
    char *left = (char *)malloc(col + 1);
    memcpy(left, s, col);
    left[col] = '\0';
    char *right = strdup(s + col);
    free(lines[row]);
    lines[row] = left;
    ensure_cap(&lines, &cap, line_count + 1);
    for (size_t i = line_count; i > row + 1; i--) lines[i] = lines[i - 1];
    lines[row + 1] = right;
    *lines_ptr = lines;
    *line_count_ptr = line_count + 1;
    *cap_ptr = cap;
}

static void join_with_prev(char ***lines_ptr, size_t *line_count_ptr, size_t row, size_t *col_ptr) {
    if (row == 0) return;
    char **lines = *lines_ptr;
    size_t line_count = *line_count_ptr;
    size_t lenp = strlen(lines[row - 1]);
    size_t lenc = strlen(lines[row]);
    char *ns = (char *)malloc(lenp + lenc + 1);
    memcpy(ns, lines[row - 1], lenp);
    memcpy(ns + lenp, lines[row], lenc + 1);
    free(lines[row - 1]);
    free(lines[row]);
    lines[row - 1] = ns;
    for (size_t i = row; i + 1 < line_count; i++) lines[i] = lines[i + 1];
    *lines_ptr = lines;
    *line_count_ptr = line_count - 1;
    *col_ptr = lenp;
}

static void save_file(const char *filename, char **lines, size_t line_count) {
    FILE *out = fopen(filename, "w");
    if (out == NULL) { error("无法打开文件写入"); return; }
    for (size_t i = 0; i < line_count; i++) fprintf(out, "%s\n", lines[i]);
    fclose(out);
}

int cmd_myvi(int argc, char *argv[]) {
    if (argc < 2) {
        error("使用方法: myvi <文件名>");
        return 1;
    }
    const char *filename = argv[1];
    size_t cap = 128;
    size_t line_count = 0;
    char **lines = (char **)malloc(sizeof(char *) * cap);
    if (!lines) { error("内存分配失败"); return 1; }
    FILE *f = fopen(filename, "r");
    if (f) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), f)) {
            size_t L = strlen(buf);
            if (L > 0 && (buf[L - 1] == '\n' || buf[L - 1] == '\r')) buf[--L] = '\0';
            ensure_cap(&lines, &cap, line_count + 1);
            lines[line_count++] = strdup(buf);
        }
        fclose(f);
    } else {
        ensure_cap(&lines, &cap, 1);
        lines[line_count++] = strdup("");
    }
    struct termios orig;
    int use_raw = 0;
    enable_raw(&orig, &use_raw);
    get_window_size();
    size_t row = 0;
    size_t col = 0;
    size_t top = 0;
    int show_numbers = 0;
    Mode mode = MODE_NORMAL;
    char status[256];
    status[0] = '\0';
    char cmd[256];
    size_t cmd_len = 0;
    char search_pat[256];
    search_pat[0] = '\0';
    char *yank = NULL;
    int dirty = 0;
    int pending_d = 0;
    int pending_g = 0;
    int count = 0;
    while (1) {
        render(lines, line_count, top, row, col, show_numbers, mode, status);
        int k = read_key();
        if (mode == MODE_NORMAL) {
            if (k >= '0' && k <= '9') {
                if (k == '0' && count == 0) { col = 0; continue; }
                count = count * 10 + (k - '0');
                continue;
            }
            if (k == KEY_LEFT || k == 'h') { if (col > 0) col--; }
            else if (k == KEY_RIGHT || k == 'l') { size_t len = strlen(lines[row]); if (col < len) col++; }
            else if (k == KEY_DOWN || k == 'j') {
                int n = count ? count : 1; count = 0;
                while (n-- && row + 1 < line_count) { row++; size_t len = strlen(lines[row]); if (col > len) col = len; }
            } else if (k == KEY_UP || k == 'k') {
                int n = count ? count : 1; count = 0;
                while (n-- && row > 0) { row--; size_t len = strlen(lines[row]); if (col > len) col = len; }
            } else if (k == '$') { col = strlen(lines[row]); }
            else if (k == 'G') { row = line_count ? line_count - 1 : 0; size_t len = strlen(lines[row]); if (col > len) col = len; }
            else if (k == 'g') { if (pending_g) { row = 0; pending_g = 0; } else { pending_g = 1; } }
            else if (k == 'i') { mode = MODE_INSERT; }
            else if (k == 'a') { size_t len = strlen(lines[row]); if (col < len) col++; mode = MODE_INSERT; }
            else if (k == 'I') { col = 0; mode = MODE_INSERT; }
            else if (k == 'A') { col = strlen(lines[row]); mode = MODE_INSERT; }
            else if (k == 'o') { split_line(&lines, &line_count, &cap, row, strlen(lines[row])); row++; col = 0; mode = MODE_INSERT; dirty = 1; }
            else if (k == 'O') { split_line(&lines, &line_count, &cap, row, 0); col = 0; mode = MODE_INSERT; dirty = 1; }
            else if (k == 'x') { delete_char(&lines[row], col); dirty = 1; }
            else if (k == 'd') { if (pending_d) { int n = count ? count : 1; count = 0; while (n-- && line_count) { delete_line(&lines, &line_count, row, &yank); if (row >= line_count && line_count) row = line_count - 1; } pending_d = 0; dirty = 1; } else { pending_d = 1; } }
            else if (k == 'p') { paste_line(&lines, &line_count, &cap, row + 1, yank); dirty = 1; }
            else if (k == '/') { mode = MODE_SEARCH; cmd_len = 0; }
            else if (k == ':') { mode = MODE_COMMAND; cmd_len = 0; }
            else if (k == 'n') {
                if (search_pat[0]) {
                    size_t r = row; size_t c = col + 1; int found = 0;
                    for (; r < line_count; r++, c = 0) {
                        char *p = strstr(lines[r] + c, search_pat);
                        if (p) { row = r; col = (size_t)(p - lines[r]); found = 1; break; }
                    }
                    if (!found) strcpy(status, "未找到"); else status[0] = '\0';
                }
            } else if (k == 'N') {
                if (search_pat[0]) {
                    size_t r = row; size_t c = col ? col - 1 : 0; int found = 0;
                    for (;; ) {
                        if (r >= line_count) break;
                        char *last = NULL; char *s = lines[r]; size_t sl = strlen(s);
                        for (char *q = strstr(s, search_pat); q; q = strstr(q + 1, search_pat)) { if ((size_t)(q - s) < c) last = q; }
                        if (last) { row = r; col = (size_t)(last - s); found = 1; break; }
                        if (r == 0) break; r--; c = sl;
                    }
                    if (!found) strcpy(status, "未找到"); else status[0] = '\0';
                }
            } else if (k == 27) { pending_d = 0; pending_g = 0; count = 0; }
            size_t avail = screen_rows > 2 ? (size_t)(screen_rows - 2) : 0;
            if (row < top) top = row;
            if (row >= top + avail) top = row - (avail ? avail - 1 : 0);
        } else if (mode == MODE_INSERT) {
            if (k == 27) { mode = MODE_NORMAL; }
            else if (k == KEY_LEFT) { if (col > 0) col--; }
            else if (k == KEY_RIGHT) { size_t len = strlen(lines[row]); if (col < len) col++; }
            else if (k == KEY_DOWN) { if (row + 1 < line_count) { row++; size_t len = strlen(lines[row]); if (col > len) col = len; } }
            else if (k == KEY_UP) { if (row > 0) { row--; size_t len = strlen(lines[row]); if (col > len) col = len; } }
            else if (k == '\r' || k == '\n') { split_line(&lines, &line_count, &cap, row, col); row++; col = 0; dirty = 1; }
            else if (k == 127 || k == 8) { if (col > 0) { delete_char(&lines[row], col - 1); col--; dirty = 1; } else if (row > 0) { join_with_prev(&lines, &line_count, row, &col); row--; dirty = 1; } }
            else { insert_char(&lines[row], col, (char)k); col++; dirty = 1; }
            size_t avail = screen_rows > 2 ? (size_t)(screen_rows - 2) : 0;
            if (row < top) top = row;
            if (row >= top + avail) top = row - (avail ? avail - 1 : 0);
        } else if (mode == MODE_COMMAND) {
            if (k == 27) { mode = MODE_NORMAL; status[0] = '\0'; }
            else if (k == '\r' || k == '\n') {
                cmd[cmd_len] = '\0';
                if (strcmp(cmd, "w") == 0) { save_file(filename, lines, line_count); dirty = 0; success("文件已保存"); }
                else if (strcmp(cmd, "q") == 0) { if (dirty) { strcpy(status, "有未保存修改"); } else { disable_raw(&orig, use_raw); for (size_t i = 0; i < line_count; i++) free(lines[i]); free(lines); return 0; } }
                else if (strcmp(cmd, "q!") == 0) { disable_raw(&orig, use_raw); for (size_t i = 0; i < line_count; i++) free(lines[i]); free(lines); return 0; }
                else if (strcmp(cmd, "wq") == 0) { save_file(filename, lines, line_count); dirty = 0; success("文件已保存"); disable_raw(&orig, use_raw); for (size_t i = 0; i < line_count; i++) free(lines[i]); free(lines); return 0; }
                else if (strcmp(cmd, "set number") == 0) { show_numbers = 1; status[0] = '\0'; }
                else if (strcmp(cmd, "set nonumber") == 0) { show_numbers = 0; status[0] = '\0'; }
                else if (strcmp(cmd, "help") == 0) { strcpy(status, "h/j/k/l i a o dd x p :w :q :wq / n N"); }
                else { strcpy(status, "未知命令"); }
                mode = MODE_NORMAL; cmd_len = 0;
            } else if (k == 127 || k == 8) { if (cmd_len > 0) cmd[--cmd_len] = '\0'; }
            else { if (cmd_len + 1 < sizeof(cmd)) cmd[cmd_len++] = (char)k; }
        } else if (mode == MODE_SEARCH) {
            if (k == 27) { mode = MODE_NORMAL; status[0] = '\0'; cmd_len = 0; }
            else if (k == '\r' || k == '\n') {
                cmd[cmd_len] = '\0';
                strncpy(search_pat, cmd, sizeof(search_pat) - 1); search_pat[sizeof(search_pat) - 1] = '\0';
                size_t r = row; size_t c = col + 1; int found = 0;
                for (; r < line_count; r++, c = 0) {
                    char *p = strstr(lines[r] + c, search_pat);
                    if (p) { row = r; col = (size_t)(p - lines[r]); found = 1; break; }
                }
                if (!found) strcpy(status, "未找到"); else status[0] = '\0';
                mode = MODE_NORMAL; cmd_len = 0;
            } else if (k == 127 || k == 8) { if (cmd_len > 0) cmd[--cmd_len] = '\0'; }
            else { if (cmd_len + 1 < sizeof(cmd)) cmd[cmd_len++] = (char)k; }
        }
    }
    disable_raw(&orig, use_raw);
    for (size_t i = 0; i < line_count; i++) free(lines[i]);
    free(lines);
    return 0;
}

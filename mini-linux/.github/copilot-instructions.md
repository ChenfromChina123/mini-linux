# Copilot / Agent 指南 — mini-linux

目标：使 AI 代码代理快速、可靠地在本仓库中贡献 C 代码。

项目概览
- 本仓库实现一个小型交互式 shell，入口在 `shell.c`，各个命令由独立的 `my*.c` 文件实现（例如 `mycat.c`, `mycp.c`, `myvi.c`）。
- 设计模式：每个命令导出 `int cmd_<name>(int argc, char *argv[])`，并在 `shell.c` 的命令表中注册。

关键位置与约定
- 构建：`Makefile` 管理构建。编辑 `C_SRCS` 增减命令源码后需重新 `make`。
- 命令注册：`shell.c`（查找命令表）——添加命令需同时在 `command.h` 声明并在此注册。
- 通用工具：`util.h`/`util.c` 提供 `error()`, `success()`, `warning()`（带颜色）——请优先使用以保持一致输出风格。
- 历史：`history.c` 将历史持久化到 `~/.mini_history`（格式：`timestamp<TAB>result<TAB>command`），请勿随意更改格式。

新增/修改命令步骤（必须遵守）
1. 新文件 `my<name>.c`，实现 `int cmd_my<name>(int argc, char *argv[])`。
2. 在 `command.h` 添加函数原型声明。
3. 在 `Makefile` 的 `C_SRCS` 中添加源文件名。
4. 在 `shell.c` 的命令表中注册条目：`{"myname", cmd_myname, "简短说明"}`，并在 `help` 中提供用法示例。
5. 使用 `util` 中的输出函数报告错误/成功；在涉及系统调用失败时，输出 `strerror(errno)` 以提供上下文。
6. 运行 `make`，再运行 `./mini_linux_shell` 进行交互式手工测试。

代码风格与注意点（对代理重要）
- 内存：项目使用 `malloc`/`realloc`/`strdup`，修改时必须保证没有内存泄漏（尤其是 `myvi.c` 的行数组）。
- 平台差异：某些文件使用 POSIX API（`termios`, `realpath`），在 Windows 原生环境可能不可用。优先在 Linux/WSL 上运行与测试。
- 错误处理：在报错时包含操作的目标（路径、文件名）和 errno 信息；避免使用容易截断的 `snprintf` 模式，若需构造字符串可直接打印或分配足够缓冲。
- 不引入外部依赖：本仓库仅用 C 标准库与 POSIX 调用；新增第三方库需先征询维护者。

常见实现样板（示例参考）
- 路径解析（mycat/mycp）：实现 `resolve_path`，支持 `~` 展开、相对转绝对并在 Windows/POSIX 分支处理 `_fullpath`/`realpath`。
- 复制到目录（mycp）：先 `stat` 目标是否为目录，若是则拼接源文件 basename。
- 编辑器（myvi）：读入文件为 `char** lines`，对插入/删除/替换操作及时维护 `line_count` 与内存分配；插入模式可用 `termios` 切换原始模式读取按键。

构建与调试技巧
- 强制重建：`make -B`；清理：`make clean`（Makefile 内根据平台选择 `rm -f` 或 `del`）。
- 在本仓库进行更改后，请务必运行 `make` 并在 `./mini_linux_shell` 中用交互式命令验收。若修改涉及 `history` 或文件写入，请用临时文件验证格式。

对 AI 代理的行为约束
- 不要修改 `history` 文件格式或 `cmd_*` 公共 API，除非同时更新相关注册/加载逻辑并说明兼容性影响。
- 提交的 PR/补丁应包含：变更摘要（中文/英文均可）、为何需要修改、如何手动验证（最小复现步骤）。

检查点与验收测试（建议在 PR 中包含）
- 新增命令能被 `make` 编译并出现在 `help` 输出中。
- 基本操作无崩溃：打开/保存/读取测试文件（对 `myvi`、`mycat`、`mycp` 做烟雾测试）。

如果内容不完整或需补充：请指出你希望智能代理在 PR 中自动包含的更多信息（如单元测试、注释模板或更严格的错误消息）。

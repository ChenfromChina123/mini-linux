`mycd` 命令用于切换当前 shell 的工作目录。

以下是其核心实现逻辑的详细说明：

**1. 目录切换核心**
`mycd` 的核心是调用系统调用 `chdir(target)`。

**2. 特殊路径处理**
[cmd_mycd](file:///d:/Users/Administrator/Mini_computer/src/commands/mycd.c#L13-L74) 函数实现了对特殊路径的支持：

- **主目录支持 (`~`)**：
  - 如果参数为空，默认尝试从环境变量 `HOME` 获取主目录路径。
  - 如果路径以 `~` 开头，程序会手动将其展开。例如 `~/docs` 会被转换为 `/home/user/docs`（取决于 `HOME` 变量的值）。
- **内存管理**：在展开 `~` 路径时，程序使用 `malloc` 分配临时缓冲区，并在 `chdir` 调用后通过 `free` 释放，防止内存泄漏。

**3. 错误处理**
- **权限与路径检查**：如果 `chdir` 返回非零值，程序会通过 `strerror(errno)` 获取具体的系统错误信息（如“目录不存在”或“权限不足”），并使用 `error()` 函数打印。

**4. 关键函数参考**

- **[chdir](file:///d:/Users/Administrator/Mini_computer/src/commands/mycd.c#L66)**：改变进程当前工作目录的系统调用。
- **[getenv](file:///d:/Users/Administrator/Mini_computer/src/commands/mycd.c#L19)**：用于获取环境变量（如 `HOME`）。

**总结**
`mycd` 不仅仅是一个简单的 `chdir` 封装，它还通过环境变量实现了对 Linux 风格主目录快捷方式（`~`）的支持。

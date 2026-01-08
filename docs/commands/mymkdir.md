
          
`mymkdir` 命令的实现主要分布在 [mymkdir.c](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c) 文件中。它模仿了 Linux 系统中 `mkdir` 命令的功能，支持创建单个目录以及通过 `-p` 选项递归创建多级目录。

以下是其核心实现逻辑的详细说明：

**1. 递归创建逻辑**
递归创建功能由静态辅助函数 [make_parents](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c#L19-L54) 实现。

- **路径解析**：它首先通过 `strdup` 复制输入路径，以便在不修改原字符串的情况下进行操作。
- **逐级遍历**：程序遍历路径字符串，每当遇到路径分隔符（`/` 或 `\`）时，将其临时替换为 `\0`（字符串结束符）。
- **尝试创建**：利用系统调用 `mkdir(tmp, 0755)` 尝试创建当前截断的路径，权限为 `0755`。
  - 如果目录已存在（`errno == EEXIST`），则忽略错误继续。
  - 如果创建失败且不是因为已存在，则返回错误。
- **恢复与继续**：创建完一级目录后，将 `\0` 还原回分隔符，继续向后遍历直到完成整个路径。

**2. 命令入口逻辑**
主入口函数 [cmd_mymkdir](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c#L65-L106) 负责处理用户输入：

- **参数解析**：
  - 检查参数数量，如果少于 2 个则显示用法说明。
  - 检查第一个参数是否为 `-p`，如果是，则设置 `make_parents_flag` 标志。
- **执行创建**：
  - 如果设置了 `-p`，调用 `make_parents(path)`。
  - 如果没有设置 `-p`，直接调用标准库的 `mkdir(path, 0755)`。
- **反馈与输出**：
  - 使用 [util.h](file:///d:/Users/Administrator/Mini_computer/include/util.h) 中定义的 `success()` 或 `error()` 函数向控制台打印操作结果。
    
**3. 关键函数参考**

- **[make_parents](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c#L19)**：处理 `-p` 递归逻辑的核心函数。
- **[cmd_mymkdir](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c#L65)**：命令的主逻辑，负责参数解析和分发任务。
- **[mkdir](file:///d:/Users/Administrator/Mini_computer/src/commands/mymkdir.c#L97)**：系统级调用，用于在文件系统中实际创建目录。

**总结**
`mymkdir` 的实现非常简洁且健壮，通过手动拆解路径字符串实现了对 `-p` 选项的支持，并利用 `errno` 处理了目录已存在等常见边缘情况。
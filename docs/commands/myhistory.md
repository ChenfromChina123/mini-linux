`myhistory` 命令用于记录、查看和管理用户在 shell 中执行过的历史命令。

以下是其核心实现逻辑的详细说明：

**1. 历史文件管理**
- **存储位置**：历史记录保存在用户主目录下的 `.mini_shell_history` 文件中。
- **路径获取**：通过 [get_history_path](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L19-L28) 函数，结合 `getpwuid(getuid())` 获取当前用户的主目录并拼接文件名。

**2. 核心功能实现**
- **显示全部 (-a)**：[show_all_history](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L33-L59) 逐行读取历史文件并带行号输出。
- **显示最近 N 条 (-n)**：[show_recent_history](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L64-L112) 将文件内容读入内存数组，计算起始位置，仅输出最后 N 条记录。
- **清空历史 (-c)**：[clear_history](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L117-L141) 以写入模式 (`w`) 重新打开文件，从而清空内容。在执行前会要求用户进行确认。

**3. 内存与资源处理**
- **动态分配**：在读取最近记录时使用 `malloc` 和 `strdup` 管理字符串数组，并在输出后使用 `free` 释放，防止内存溢出。
- **最大限制**：系统硬编码了 `MAX_HISTORY`（1000条）作为单次读取的上限。

**4. 关键函数参考**

- **[getpwuid/getuid](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L20)**：获取系统用户信息。
- **[fgets](file:///d:/Users/Administrator/Mini_computer/src/commands/myhistory.c#L51)**：安全地逐行读取文件内容。

**总结**
`myhistory` 实现了类似标准 Bash history 的持久化存储和管理功能，通过简单的文件读写和内存管理，为用户提供了命令回溯的能力。

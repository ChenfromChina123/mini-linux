`mychmod` 命令用于查看或修改文件/目录的权限。

以下是其核心实现逻辑的详细说明：

**1. 查看权限逻辑**
由 [show_permissions](file:///d:/Users/Administrator/Mini_computer/src/commands/mychmod.c#L17-L44) 函数实现：

- **获取状态**：使用 `stat(filename, &st)` 获取文件的详细信息。
- **位运算解析**：通过按位与运算（`&`）和标准权限掩码（如 `S_IRUSR`, `S_IWGRP` 等）来解析权限。
- **格式化输出**：将权限位转换为传统的 `rwxrwxrwx` 字符串格式，并显示八进制数值（如 `755`）。

**2. 修改权限逻辑**
由 [change_permissions](file:///d:/Users/Administrator/Mini_computer/src/commands/mychmod.c#L49-L66) 函数实现：

- **模式转换**：使用 `strtol(mode_str, &endptr, 8)` 将用户输入的八进制字符串转换为长整型数值。
- **合法性检查**：确保输入是有效的 3 位八进制数（0-777）。
- **执行修改**：调用 `chmod(filename, (mode_t)mode)` 系统调用应用新权限。

**3. 命令入口逻辑**
[main](file:///d:/Users/Administrator/Mini_computer/src/commands/mychmod.c#L71-L97) 函数根据参数个数决定执行的操作：
- **2 个参数**：调用 `show_permissions` 查看权限。
- **3 个参数**：调用 `change_permissions` 修改权限并显示修改后的结果。

**4. 关键函数参考**

- **[stat](file:///d:/Users/Administrator/Mini_computer/src/commands/mychmod.c#L20)**：获取文件元数据。
- **[chmod](file:///d:/Users/Administrator/Mini_computer/src/commands/mychmod.c#L60)**：更改文件访问权限的系统调用。

**总结**
`mychmod` 提供了直观的权限查看和修改功能，支持标准的八进制数字模式，并能清晰地反馈当前文件的权限状态。

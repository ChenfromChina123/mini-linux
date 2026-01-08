`myls` 命令用于列出目录中的文件和子目录，并支持详细信息的展示。

以下是其核心实现逻辑的详细说明：

**1. 列表显示模式**
`myls` 支持两种显示模式，由 [cmd_myls](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L181-L199) 根据 `-l` 选项分发：

- **短格式 (Short Format)**：由 [list_directory_short](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L135-L173) 实现，仅列出文件名。
myls
- **长格式 (Long Format)**：由 [list_directory_long](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L58-L128) 实现，显示权限、链接数、所有者、组、大小和修改时间。
myls -l

**2. 核心功能点**
- **权限解析**：[print_permissions](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L24-L35) 函数将 `st_mode` 位掩码转换为可读的 `rwxrwxrwx` 字符串。
- **彩色输出**：
  - **蓝色**：代表目录。
  - **绿色**：代表可执行文件。
  - 通过 ANSI 转义序列（如 `\033[1;34m`）实现。
- **人类可读的大小**：[print_size](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L41-L51) 自动将字节单位转换为 K, M, G 等更直观的单位。
- **元数据获取**：利用 `getpwuid` 和 `getgrgid` 将 UID/GID 转换为真实的用户名和组名。

**3. 关键函数参考**

- **[opendir/readdir](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L59)**：打开并读取目录流。
- **[stat](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L78)**：获取文件详细状态。
- **[strftime](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c#L112)**：格式化文件的修改时间。

**总结**
`myls` 通过结合目录流操作和文件状态查询，提供了一个功能丰富且带有视觉反馈的目录查看工具。

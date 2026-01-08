`mycp` 命令用于在文件系统中复制文件或目录。

以下是其核心实现逻辑的详细说明：

**1. 文件复制逻辑**
核心功能实现在 [copy_file](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c#L23-L68) 函数中：

- **打开源文件**：使用 `open(src, O_RDONLY)` 打开源文件。
- **保留权限**：通过 `fstat` 获取源文件的权限模式（`st_mode`），确保目标文件在创建时具有相同的权限。
- **创建目标文件**：使用 `open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode)` 创建或清空目标文件。
- **数据传输**：
  - 使用 4KB 的缓冲区进行读写操作。
  - 在循环中调用 `read` 和 `write`，直到源文件读取完毕。
- **资源关闭**：使用 `close` 关闭文件描述符，确保系统资源被释放。

**2. 递归目录复制逻辑**
当指定 `-r` 选项时，通过 [copy_directory](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c#L77-L127) 函数处理：

- **目录遍历**：使用 `opendir` 和 `readdir` 遍历源目录下的所有项。
- **属性检测**：使用 `stat` 判断每一项是文件还是子目录。
- **递归处理**：
  - 如果是子目录，递归调用 `copy_directory`。
  - 如果是文件，调用 `copy_file` 进行复制。
- **跳过特殊项**：自动跳过 `.`（当前目录）和 `..`（父目录）以防止无限循环。

**3. 命令解析与分发**
[cmd_mycp](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c#L134-L181) 函数负责处理用户输入：

- **参数解析**：支持 `-r` 选项，用于开启递归模式。
- **类型判断**：使用 `S_ISDIR` 宏检查源路径是否为目录。
- **错误处理**：如果源路径是目录但未提供 `-r` 参数，会提示用户。

**4. 关键函数参考**

- **[open/read/write/close](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c)**：底层的 UNIX 文件 I/O 系统调用。
- **[fstat/stat](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c)**：用于获取文件的元数据。
- **[opendir/readdir/closedir](file:///d:/Users/Administrator/Mini_computer/src/commands/mycp.c)**：用于目录遍历。

**总结**
`mycp` 不仅支持基础的文件复制，还通过递归算法实现了对整个目录树的完整克隆，并能够自动保持文件权限，功能与 Linux 原生 `cp` 命令高度一致。

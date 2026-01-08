`mykill` 命令允许用户通过进程名称来终止运行中的进程，而不仅仅是 PID。

以下是其核心实现逻辑的详细说明：

**1. 进程识别逻辑**
- **遍历 /proc**：[kill_process_by_name](file:///d:/Users/Administrator/Mini_computer/src/commands/mykill.c#L40-L103) 遍历 Linux 的 `/proc` 目录，筛选出所有以数字命名的 PID 文件夹。
- **读取进程名**：对于每个 PID，通过 [get_process_name](file:///d:/Users/Administrator/Mini_computer/src/commands/mykill.c#L17-L35) 读取 `/proc/[PID]/comm` 文件来获取该进程的可执行文件名。

**2. 终止执行流程**
- **名称匹配**：将读取到的进程名与用户输入的名称进行比对。
- **安全检查**：程序会自动检查目标 PID 是否为当前 shell 进程，防止意外自杀。
- **交互式确认**：在发送信号前，程序会提示用户确认是否终止发现的进程。
- **信号发送**：使用 `kill(pid, SIGTERM)` 向目标进程发送终止信号。

**3. 关键函数参考**

- **[opendir/readdir](file:///d:/Users/Administrator/Mini_computer/src/commands/mykill.c#L41)**：用于遍历系统的虚拟文件系统目录。
- **[kill](file:///d:/Users/Administrator/Mini_computer/src/commands/mykill.c#L80)**：发送信号给进程的核心系统调用。

**总结**
`mykill` 结合了 `/proc` 文件系统解析和信号机制，提供了一种比传统 `kill` 更人性化的进程管理方式。

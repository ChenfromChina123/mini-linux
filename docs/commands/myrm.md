`myrm` 命令的执行过程可以分为 **参数解析**、**目标检测** 和 **删除执行** 三个主要阶段。

以下是详细的步骤解析：

### **1. 第一阶段：参数解析**
在 [cmd_myrm](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L114) 函数中，程序首先遍历 `argv` 数组：
*   **`-i` (Interactive)**：设置 `interactive = 1`，表示在删除前需要用户输入 `y/n` 确认。
*   **`-r` (Recursive)**：设置 `recursive = 1`，表示支持递归删除目录及其内容。
*   **`path`**：记录要删除的目标路径。如果没有提供路径，程序会报错并退出。

### **2. 第二阶段：目标检测**
程序使用 [stat()](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L136) 函数获取目标的元数据：
*   **检查是否存在**：如果 `stat()` 返回 -1，说明文件或目录不存在，程序报错。
*   **判断类型**：通过 `S_ISDIR(st.st_mode)` 判断目标是**文件**还是**目录**。
    *   如果是目录且没有加 `-r` 参数，程序会提示“请使用 -r 选项”并终止。

### **3. 第三阶段：删除执行**

#### **A. 如果目标是普通文件**
调用 [remove_file](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L89) 函数：
1.  如果开启了 `-i`，打印 `删除文件 'xxx'? (y/n)` 并等待用户输入。
2.  调用底层的 [unlink()](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L100) 系统调用来从磁盘上移除文件。

#### **B. 如果目标是目录（必须配合 -r）**
调用 [remove_directory](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L23) 函数，这是一个**递归过程**：
1.  **打开目录**：使用 `opendir()`。
2.  **遍历条目**：使用 `readdir()` 读取目录下的每一个文件和子目录。
    *   跳过 `.` 和 `..`（当前目录和父目录），防止陷入无限死循环。
3.  **递归处理**：
    *   如果是子目录，再次调用 `remove_directory()`。
    *   如果是文件，直接调用 `unlink()` 删除。
4.  **清理空目录**：当目录下的所有内容都删完后，最后调用 [rmdir()](file:///d:/Users/Administrator/Mini_computer/src/commands/myrm.c#L75) 删除已经变为空的目录本身。

### **流程图解：**
```text
[输入 myrm -r test]
      |
      v
[解析参数: recursive=1, path="test"]
      |
      v
[stat("test") -> 发现是目录]
      |
      v
[进入 remove_directory("test")]
      |-- 遍历 test 内部...
      |-- 发现子文件 a.txt -> unlink("test/a.txt")
      |-- 发现子目录 sub  -> 递归进入 remove_directory("test/sub")
      |-- ...
      v
[所有内容删完 -> rmdir("test")]
```

### **关键技术点：**
*   **`unlink()`**：这是 Linux 删除文件的真正操作，它会减少文件的链接数。
*   **`rmdir()`**：只能删除**空目录**。这就是为什么删除目录必须先递归清空其内容的原因。
*   **`lstat` vs `stat`**：在处理符号链接时，通常使用 `lstat` 以避免顺着链接去删掉原始文件，但在你的简化版实现中目前主要使用的是 `stat`。
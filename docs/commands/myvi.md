`myvi` 是一个功能完备的交互式终端文本编辑器，其实现涵盖了终端 I/O 控制、动态内存管理、屏幕渲染引擎以及状态机模型。

以下是对其实现细节的深度解析：

### **1. 核心架构与数据结构**

`myvi` 的设计基于 **行缓冲模型 (Line-based Buffer)**。

- **内存模型**：
  - [lines](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L202)：一个二级指针 `char **`，指向动态分配的字符串数组。每一行都是一个独立的堆内存块。
  - [cap](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L200) 与 [line_count](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L201)：分别记录数组的当前容量和实际行数。
  - [ensure_cap](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L51-L63)：当行数超过容量时，通过 `realloc` 实现倍增扩容。

- **状态管理**：
  - [Mode](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L11)：定义了四种模式：`NORMAL` (普通)、`INSERT` (插入)、`COMMAND` (命令) 和 `SEARCH` (搜索)。

### **2. 终端底层控制 (Raw Mode)**

为了实现单键响应（按下一个键立即生效，无需按回车），`myvi` 切换了终端模式：

- **Raw Mode**：通过 [enable_raw_mode](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L221) 禁用终端的行缓冲和回显功能。
- **窗口自适应**：[get_window_size](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L14-L20) 使用 `ioctl` 系统调用获取当前终端的行数和列数，确保编辑器能在不同大小的窗口中正确渲染。

### **3. 渲染引擎 (Rendering)**

[render](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L22-L48) 函数是 UI 的核心：

- **ANSI 转义序列**：
  - `\x1b[2J`：清除全屏。
  - `\x1b[H`：光标复位到左上角。
  - `\x1b[row;colH`：精确定位光标位置。
- **滚动支持**：通过变量 `top` 记录当前屏幕显示的第一行，实现长文件的垂直滚动。
- **状态栏**：在屏幕底部实时刷新模式名称、坐标、文件名和操作状态（如“未找到”）。

### **4. 编辑操作实现逻辑**

- **字符插入/删除**：
  - [insert_char](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L100-L110) 和 [delete_char](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L113-L123)：通过 `malloc` 分配新空间，并使用 `memcpy` 搬运数据，实现行内字符的移动。
- **行操作**：
  - [split_line](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L126-L146)：当在行中按回车时，将当前行拆分为两行。
  - [join_with_prev](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L149-L165)：当在行首按退格时，将当前行合并到上一行末尾。
- **剪贴板 (Yank)**：
  - 使用静态变量 [yank](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L234) 存储最后一次被删除（`dd`）的行内容，通过 `p` 键调用 [paste_line](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L94-L97) 实现粘贴。

### **5. 搜索与指令系统**

- **搜索模式**：
  - 用户按下 `/` 后进入搜索模式，输入关键字并回车。
  - 程序使用 `strstr` 在缓冲区中查找匹配项，并自动跳转光标。
  - `n` 和 `N` 键分别实现向后和向前查找下一个匹配点。
- **冒号命令**：
  - 支持 `:w` (保存)、`:q` (退出)、`:wq` (保存退出) 以及 `:set number` (开启行号) 等经典指令。

### **6. 关键函数汇总**

| 函数名 | 作用 |
| :--- | :--- |
| [cmd_myvi](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L194) | 命令主入口，包含核心事件循环。 |
| [render](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L22) | 负责终端屏幕的实时刷新。 |
| [read_key_code](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L252) | 从底层读取按键（包括转义序列，如方向键）。 |
| [save_file](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c#L168) | 将内存中的行缓冲同步到磁盘文件。 |

**总结**
`myvi` 的实现展示了如何从零开始构建一个复杂的交互式系统：从最底层的字节流读取，到中间层的内存模型维护，再到最顶层的状态机逻辑处理，每一层都紧密耦合，是 C 语言系统级编程的典型范例。

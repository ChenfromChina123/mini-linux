# C 成员功能实现与 Agent 工作流程详细说明

本仓库实现了一个类 Linux 的微型系统环境，结合了底层 C 语言开发的系统命令（C 成员）与上层基于大模型的 AI 代理（Agent）。本文档详细说明了 C 成员的具体实现以及 Agent 的自动化工作流程。

---

## **1. C 成员：系统内核命令实现**

C 成员负责提供系统的基础操作能力，所有命令均位于 `src/commands` 目录下，并通过核心 Shell (`src/core/shell.c`) 进行调度。

### **核心命令详解**

#### **文本编辑器 ([myvi.c](file:///d:/Users/Administrator/Mini_computer/src/commands/myvi.c))**
*   **功能**：实现基础的文本编辑、行插入/删除、文件保存。
*   **关键技术**：
    *   **终端 Raw 模式**：通过 `termios.h` 禁用终端的规范模式和回显，实现按键的实时捕获。
    *   **动态行管理**：使用 `char **lines` 动态数组存储文本行，支持无限长度（受内存限制）的文件编辑。
    *   **ANSI 转义序列**：利用 `\x1b[2J` 等指令控制光标位置和屏幕刷新。

#### **文件查看 ([mycat.c](file:///d:/Users/Administrator/Mini_computer/src/commands/mycat.c))**
*   **功能**：读取并显示文件内容。
*   **关键技术**：
    *   **路径解析**：调用 `resolve_path` 处理 `~` 和相对路径。
    *   **流式读取**：使用 `fopen` 和 `fgets` 逐行读取文件，降低内存占用。

#### **目录管理 ([myls.c](file:///d:/Users/Administrator/Mini_computer/src/commands/myls.c))**
*   **功能**：列出目录内容，支持 `-l` 长格式。
*   **关键技术**：
    *   **目录遍历**：使用 `opendir` 和 `readdir` 获取目录项。
    *   **状态获取**：调用 `stat()` 获取文件大小、权限和修改时间。
    *   **权限转换**：通过位运算解析 `mode_t` 权限位并转换为字符串。

---

## **2. Agent 工作流程**

Agent 是系统的“大脑”，负责解析用户意图并调用 C 成员执行任务。其核心逻辑位于 `xiaochen_agent_v2/core/agent.py`。

### **工作循环 (The Loop)**

1.  **环境感知 (Perception)**：
    *   Agent 启动时会获取当前工作目录、系统状态以及历史命令记录。
2.  **推理决策 (Reasoning)**：
    *   将用户输入发送给大模型（LLM）。
    *   模型根据系统提示词（System Prompt）判断是否需要调用工具。
3.  **标签解析 (Tag Parsing)**：
    *   Agent 解析模型输出中的特定标签（如 `<thought>`、`<call:run_command>`）。
4.  **用户授权 (Authorization)**：
    *   对于敏感操作，Agent 会暂停并等待用户点击“允许”或“拒绝”。
5.  **顺序执行 (Execution)**：
    *   Agent 通过 `subprocess` 调用编译好的 C 程序（如 `./mycat test.txt`）。
6.  **观察反馈 (Observation)**：
    *   捕获 C 程序的标准输出和错误流，将其作为新的上下文反馈给 LLM。
7.  **任务完成 (Completion)**：
    *   重复上述过程，直到 LLM 认为任务已完成并给出最终回答。

---

## **3. 协同交互流程示例**

当用户输入 "帮我看看 test.txt 的内容并修改第三行为 'Hello World'" 时：

1.  **Agent 推理**：判断需要先查看内容，生成调用 `mycat test.txt` 的指令。
2.  **C 成员执行**：`mycat` 读取磁盘文件，将内容输出到终端。
3.  **Agent 获取结果**：获取到文件内容。
4.  **Agent 再次推理**：根据获取的内容，判断需要使用编辑器修改，生成调用 `myvi test.txt` 或类似的内部编辑逻辑。
5.  **C 成员执行**：`myvi` 修改文件并保存。
6.  **Agent 确认**：告知用户修改已完成。

---

## **4. 开发建议**

*   **新增命令**：在 `src/commands` 下创建新 `.c` 文件，实现 `main` 函数，并在 `Makefile` 中添加编译规则。
*   **错误处理**：C 命令应通过 `stderr` 输出错误信息，Agent 会自动捕获这些信息以进行故障排除。
*   **编码规范**：所有 C 代码应遵循函数级注释规范，确保控制台输出使用 UTF-8 编码以适配 Agent 的解析。

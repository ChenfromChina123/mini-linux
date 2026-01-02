`myps` 的核心原理是利用 Linux 系统中的 **`/proc` 虚拟文件系统**。

在 Linux 中，`/proc` 并不存在于硬盘上，而是内核在内存中维护的一个“窗口”，通过它你可以实时查看内核的运行状态。

以下是 `myps` 实现的三个关键步骤：

### **1. 遍历 `/proc` 目录**
在 [myps.c:L90](file:///d:/Users/Administrator/Mini_computer/src/commands/myps.c#L90) 中，程序使用 `opendir("/proc")` 打开该目录，并遍历其中的所有条目。
*   内核为每个运行中的进程在 `/proc` 下创建一个以 **PID（进程ID）** 命名的文件夹（例如 `/proc/1`）。
*   程序会检查条目名称是否全为数字，从而识别出哪些是进程文件夹。

### **2. 解析进程数据文件**
对于每一个 PID 文件夹，`myps` 主要读取两个文件：

#### **A. `/proc/[pid]/stat` (读取基础状态)**
在 [read_proc_stat](file:///d:/Users/Administrator/Mini_computer/src/commands/myps.c#L28) 函数中实现：
*   **用途**：获取进程名、状态（S/R/Z）、用户态时间 (`utime`) 和内核态时间 (`stime`)。
*   **实现**：使用 `fscanf` 按照固定格式解析该文件的内容。

#### **B. `/proc/[pid]/status` (读取内存和用户ID)**
在 [read_proc_status](file:///d:/Users/Administrator/Mini_computer/src/commands/myps.c#L62) 函数中实现：
*   **用途**：获取进程的内存占用 (`VmSize`) 和运行该进程的用户 ID (`Uid`)。
*   **实现**：逐行读取文件，寻找以 `VmSize:` 和 `Uid:` 开头的行。

### **3. 转换与格式化输出**
最后，程序将获取到的原始数据进行处理：
*   **用户名转换**：通过 `getpwuid(uid)` 将数字形式的 `Uid` 转换为可读的用户名（如 `root`）。
*   **CPU 计算**：将 `utime` 和 `stime` 相加，计算出一个简化的 CPU 使用率。
*   **单位转换**：将内存从 KB 转换为 MB。

### **总结流程图**
```text
[myps 启动] 
    |
    v
打开 /proc 目录 
    |
    +-- 找到数字命名的文件夹 (PID)
    |       |
    |       +-- 读取 /proc/[PID]/stat   --> 获取进程名、状态、时间
    |       +-- 读取 /proc/[PID]/status --> 获取内存(VmSize)、用户(Uid)
    |       |
    |       +-- 转换 Uid 为用户名
    |
    v
格式化打印到屏幕
```

这种“读取文件”的方式正是 Linux “一切皆文件”思想的体现，`myps` 实际上就是一个**解析特定系统文件的文本处理工具**。
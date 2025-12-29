# Mini Linux Shell

一个轻量级的自定义Linux shell，使用C语言编写，具有基本的命令执行、文件操作、目录管理、进程监控和用户认证功能。

## 目录

- [功能特性](#功能特性)
- [文件结构](#文件结构)
- [编译说明](#编译说明)
- [使用指南](#使用指南)
  - [登录](#登录)
  - [基本命令](#基本命令)
  - [文件操作](#文件操作)
  - [目录操作](#目录操作)
  - [进程管理](#进程管理)
  - [用户管理](#用户管理)
  - [命令历史](#命令历史)
- [实现细节](#实现细节)
- [未来增强](#未来增强)
- [许可证](#许可证)

## 功能特性

### 核心功能
- **命令行界面**：动态提示符显示当前用户名
- **行编辑器**：支持方向键左右移动、上下浏览历史、行内插入/删除
- **命令解析**：将输入标记化为命令和参数
- **多进程执行**：使用 `fork()` 和 `waitpid()` 执行命令
- **命令历史**：记录并持久化，包含时间戳和执行结果
- **用户认证**：支持root和普通用户权限，用户/会话持久化

### 文件操作
- **mycat**：显示文件内容
- **myrm**：删除文件
- **myvi**：vi风格文本编辑器（NORMAL/INSERT/COMMAND/SEARCH）
- **mytouch**：创建空文件
- **myecho**：向文件写入内容
- **mycp**：复制文件

### 目录操作
- **myls**：列出目录内容，支持 `-l` 长格式选项

### 进程管理
- **myps**：显示进程信息，支持多种选项

### 用户管理
- **useradd**：创建新用户（仅root权限）
- **userdel**：删除用户（仅root权限）
- **users**：列出所有用户与活跃用户（跨终端会话）
- **passwd**：修改密码（本人需验证旧密码；root可修改他人）

### AI助手工具
- **agent**：启动小晨AI终端助手（xiaochen_agent_v2）
  - 支持交互式对话模式
  - 支持单条命令快速执行
  - 可执行文件操作、代码编写、项目管理等任务
  - 基于大语言模型的智能终端助手
  - 自动检查和安装 Python 依赖

## 文件结构

```
Mini_computer/
├── shell.h          # 主shell头文件
├── shell.c          # 主shell实现
├── user.h           # 用户系统头文件
├── user.c           # 用户系统实现
├── history.h        # 命令历史头文件
├── history.c        # 命令历史实现
├── command.h        # 命令声明
├── util.h           # 工具函数头文件
├── util.c           # 工具函数实现
├── mycat.c          # mycat命令实现
├── myrm.c           # myrm命令实现
├── myvi.c           # myvi命令实现
├── mytouch.c        # mytouch命令实现
├── myecho.c         # myecho命令实现
├── mycp.c           # mycp命令实现
├── myls.c           # myls命令实现
├── myps.c           # myps命令实现
├── myagent.c        # agent命令实现（AI助手）
├── xiaochen_agent_v2/ # 小晨AI终端助手（Python）
│   ├── check_deps.py  # 依赖检查脚本
│   ├── run_once.py    # 单条命令执行脚本
│   └── ...            # 其他 Agent 模块
├── Makefile         # 编译规则
├── build.bat        # Windows 编译脚本
├── start_agent.bat  # Agent 独立启动脚本（批处理）
├── start_agent.ps1  # Agent 独立启动脚本（PowerShell）
├── WINDOWS_BUILD.md # Windows 平台编译说明
└── README.md        # 此文档
```

## 编译说明

### 前置条件

- **Linux 系统或兼容环境**（WSL、Cygwin、MSYS2）
- GCC编译器
- Make 工具
- Python 3.x（用于 `agent` AI助手命令，可选）
- Python依赖包（首次运行 `agent` 命令时会自动安装）：
  - requests>=2.28.0
  - urllib3>=1.26.0
  - colorama>=0.4.6

> **Windows 用户请注意**：本项目使用了 Linux 特定的系统调用，建议使用 WSL (Windows Subsystem for Linux) 编译运行。详见 [WINDOWS_BUILD.md](WINDOWS_BUILD.md)。

### 编译步骤

#### Linux / WSL / Mac

1. **克隆或导航到项目目录**
   ```bash
   cd mini-linux
   ```

2. **使用Makefile编译**
   ```bash
   make
   ```

   这将生成主可执行文件 `mini_linux_shell`。

3. **清理编译产物**（可选）
   ```bash
   make clean
   ```

#### Windows 原生环境

Windows 原生环境下无法编译完整的 Shell，但可以单独使用 AI 助手功能：

1. **安装 Python 依赖（可选，首次运行会自动安装）**
   ```powershell
   pip install -r requirements.txt
   ```

2. **使用启动脚本**
   ```powershell
   # 方式 1：批处理脚本
   .\start_agent.bat
   
   # 方式 2：PowerShell 脚本
   .\start_agent.ps1
   
   # 方式 3：直接运行
   python -m xiaochen_agent_v2
   ```

详细的 Windows 编译说明请参考 [WINDOWS_BUILD.md](WINDOWS_BUILD.md)。

## 使用指南

### 登录

运行shell可执行文件并输入您的凭证：

```bash
./mini_linux_shell

欢迎使用Mini Linux Shell！
输入 'help' 查看可用命令。

用户名: root
密码: root

root@mini-linux:$ 
```

**默认用户**:
- `root` / `root` (超级用户)
- `user` / `user` (普通用户)

### 基本命令

- **help**: 显示可用命令
  ```bash
  root@mini-linux:$ help
  ```

- **exit**: 退出shell
  ```bash
  root@mini-linux:$ exit
  ```

- **clear**: 清屏
  ```bash
  root@mini-linux:$ clear
  ```

### 文件操作

- **mycat**: 显示文件内容
  ```bash
  root@mini-linux:$ mycat file.txt
  ```

- **myrm**: 删除文件
  ```bash
  root@mini-linux:$ myrm file.txt
  ```

- **myvi**: 编辑文件（vi风格）
  ```bash
  root@mini-linux:$ myvi file.txt
  ```
  - NORMAL：`h/j/k/l` 或方向键移动，`dd` 删除行，`p` 粘贴，`x` 删除字符
  - INSERT：`i/a/I/A/o/O` 进入插入，`ESC` 返回 NORMAL，`Enter` 断行
  - COMMAND：输入 `:`，支持 `w`/`q`/`q!`/`wq`、`set number`/`set nonumber`
  - SEARCH：输入 `/pattern`，`n` 下一个，`N` 上一个

- **mytouch**: 创建空文件
  ```bash
  root@mini-linux:$ mytouch newfile.txt
  ```

- **myecho**: 向文件写入内容
  ```bash
  # 覆盖文件内容
  root@mini-linux:$ myecho Hello World > file.txt
  
  # 追加内容到文件
  root@mini-linux:$ myecho More content >> file.txt
  ```

- **mycp**: 复制文件
  ```bash
  root@mini-linux:$ mycp source.txt destination.txt
  ```

### 目录操作

- **myls**: 列出目录内容
  ```bash
  # 短格式
  root@mini-linux:$ myls
  
  # 长格式（权限、大小等）
  root@mini-linux:$ myls -l
  
  # 列出指定目录
  root@mini-linux:$ myls /home
  ```

### 进程管理

- **myps**: 显示进程信息
  ```bash
  # 显示所有进程
  root@mini-linux:$ myps
  
  # 显示指定PID的进程
  root@mini-linux:$ myps -p 1234
  ```

### 用户管理

> **注意**：这些命令需要root权限

- **useradd**: 创建新用户
  ```bash
  # 创建普通用户
  root@mini-linux:$ useradd newuser password
  
  # 创建root用户（不推荐）
  root@mini-linux:$ useradd admin password --root
  ```

- **userdel**: 删除用户
  ```bash
  root@mini-linux:$ userdel newuser
  ```

- **users**: 显示所有用户与活跃用户
  ```bash
  root@mini-linux:$ users
  ```

- **passwd**: 修改密码
  ```bash
  # 修改本人密码（需旧密码）
  user@mini-linux:$ passwd
  # 以root修改他人密码
  root@mini-linux:$ passwd otheruser
  # 以root直接指定新密码
  root@mini-linux:$ passwd otheruser newpass
  ```

### 命令历史

- **history**: 显示命令历史，包含时间戳和结果
  ```bash
  root@mini-linux:$ history
  ```

### AI助手工具

- **agent**: 启动小晨AI终端助手
  ```bash
  # 启动交互式AI助手（需要先设置环境变量）
  root@mini-linux:$ agent
  
  # 直接执行单条指令
  root@mini-linux:$ agent 帮我创建一个测试文件
  root@mini-linux:$ agent 显示当前目录结构
  ```

  **环境配置**：
  
  使用前需要设置以下环境变量（Windows PowerShell）：
  ```powershell
  $env:VOID_API_KEY = "你的API密钥"
  $env:VOID_BASE_URL = "https://api.deepseek.com"  # 可选，默认为DeepSeek
  $env:VOID_MODEL = "deepseek-chat"  # 可选，默认为deepseek-chat
  ```

  或在Linux/Mac终端：
  ```bash
  export VOID_API_KEY="你的API密钥"
  export VOID_BASE_URL="https://api.deepseek.com"  # 可选
  export VOID_MODEL="deepseek-chat"  # 可选
  ```

  **功能特性**：
  - 🤖 智能对话：理解自然语言指令
  - 📁 文件操作：读取、编写、搜索文件
  - 🔍 代码搜索：在项目中查找代码片段
  - 🛠️ 命令执行：自动执行终端命令（需要确认）
  - 📝 任务管理：跟踪和管理多个任务
  - 💾 会话管理：保存和加载对话历史
  - ⚡ 中断控制：支持 Ctrl+C 中断执行
  - 🔄 操作回滚：支持撤销文件修改
  - 🔧 自动安装依赖：首次运行自动检查并安装所需 Python 包

  **实现架构**：
  - `myagent.c`: C 语言接口，负责从 shell 调用 Python Agent
  - `check_deps.py`: 自动检查并安装 Python 依赖包
  - `run_once.py`: 单条命令执行脚本，避免字符串转义问题
  - `xiaochen_agent_v2/`: 完整的 Python AI Agent 系统
  
  详细使用说明请参考 `xiaochen_agent_v2/README.md`

## 实现细节

### 多进程架构

Shell采用经典的Unix风格多进程架构：

1. **父进程**: 读取用户输入，解析命令，创建子进程
2. **子进程**: 执行命令，返回退出状态
3. **命令执行**: 使用 `fork()` 创建子进程，`waitpid()` 等待完成

### 命令解析

1. **内置命令**: 在shell进程中直接执行
2. **外部命令**: 在子进程中使用 `execvp()` 执行
3. **路径解析**: 检查命令是否为有效的可执行文件

### 用户认证

- 用户持久化：`~/.mini_users`（用户名、密码、是否root）
- 会话跟踪：`~/.mini_sessions`（用户名、PID），跨终端显示活跃用户
- 用户管理操作需要root权限（除本人修改密码）

### 命令历史

- 持久化文件：`~/.mini_history`
- 启动时加载最近记录，执行时追加
- 记录时间戳、命令字符串和退出状态（最多显示100条）

## 未来增强

### 文件操作
- [ ] **mychmod**: 修改文件权限
- [ ] **myrm**: 添加确认提示（-i选项）
- [ ] **mytouch**: 检查文件是否存在，提供覆盖或重命名选项

### 目录操作
- [ ] **mkdir**: 创建目录
- [ ] **rmdir**: 删除目录
- [ ] **cd**: 更改当前目录
- [ ] **pwd**: 打印当前目录

### 进程管理
- [ ] **kill**: 通过名称或PID终止进程
- [ ] **top**: 实时进程监控

### 命令特性
- [ ] **管道支持**: 支持使用 `|` 连接命令
- [ ] **重定向增强**: 支持 `<` 输入重定向
- [ ] **后台执行**: 支持使用 `&` 在后台运行命令

### 用户系统
- [ ] 持久化用户存储
- [ ] 密码哈希
- [ ] 组管理

## 许可证

此项目采用 Apache-2 许可证开源。

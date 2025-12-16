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
- **命令解析**：将输入标记化为命令和参数
- **多进程执行**：使用 `fork()` 和 `waitpid()` 执行命令
- **命令历史**：记录所有命令，包含时间戳和执行结果
- **用户认证**：基本登录系统，支持root和普通用户权限

### 文件操作
- **mycat**：显示文件内容
- **myrm**：删除文件
- **myvi**：简单文本文件编辑器
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
├── Makefile         # 编译规则
└── README.md        # 此文档
```

## 编译说明

### 前置条件

- GCC编译器
- Linux系统（`myps` 需要访问 `/proc` 文件系统）

### 编译步骤

1. **克隆或导航到项目目录**
   ```bash
   cd Mini_computer
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

- **myvi**: 编辑文件（简单文本编辑器）
  ```bash
  root@mini-linux:$ myvi file.txt
  ```
  > 逐行输入内容，在新行输入 `.` 保存并退出

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

### 命令历史

- **history**: 显示命令历史，包含时间戳和结果
  ```bash
  root@mini-linux:$ history
  ```

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

- 简单的无文件认证系统
- 用户凭证存储在内存中
- 用户管理操作需要root权限

### 命令历史

- 最多存储100条命令
- 记录时间戳、命令字符串和退出状态
- 实现为循环缓冲区

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

此项目采用 MIT 许可证开源。
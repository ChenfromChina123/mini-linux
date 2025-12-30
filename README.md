# Mini Linux Shell

一个基于Linux的自定义Shell程序，使用C语言编写核心功能，Shell脚本实现用户管理。
本项目是Linux操作系统综合作业，实现了完整的Shell交互功能。

## 目录

- [功能特性](#功能特性)
- [系统架构](#系统架构)
- [文件结构](#文件结构)
- [编译说明](#编译说明)
- [使用指南](#使用指南)
- [实现细节](#实现细节)
- [开发文档](#开发文档)

## 功能特性

### 核心Shell功能
- **动态命令提示符**：显示用户名@主机名:当前目录格式
- **命令解析**：支持命令和参数的解析
- **命令执行**：
  - C程序：使用fork()创建子进程，execvp()执行命令，waitpid()等待结束
  - Shell脚本：使用system()函数调用
- **高级输入处理**：
  - 支持方向键（上下左右）进行光标移动和历史命令回溯
  - 支持 Backspace 删除和行内编辑功能
  - 统一的输入处理模块（input.c/input.h）
- **内置命令**：exit, cd, pwd, help
- **命令历史**：自动记录到~/.mini_shell_history

### 文件操作命令（C语言实现）
- **mytouch**：创建文件，检查重名并提供覆盖/重命名选项
- **mycat**：显示文件内容，支持 `>` 和 `>>` 重定向
- **mycp**：复制文件
- **myrm**：删除文件/目录，支持 `-i`（交互确认）和 `-r`（递归删除）选项
- **mychmod**：查看/修改文件权限

### 目录操作命令（C语言实现）
- **myls**：列出目录内容，支持 `-l` 长格式显示

### 进程管理命令（C语言实现）
- **myps**：显示进程信息，支持 `-a`（所有进程）和 `-u`（当前用户进程）选项
- **mykill**：通过进程名称终止进程

### 历史记录命令（C语言实现）
- **myhistory**：管理命令历史
  - `-a`：显示所有历史记录
  - `-n <count>`：显示最近N条记录
  - `-c`：清空历史记录

### 用户管理命令（Shell脚本实现）
- **myuseradd.sh**：创建用户
  - 交互式模式：逐步输入用户信息
  - 批量模式：从文件读取用户列表批量创建
- **myuserdel.sh**：删除用户，提供交互确认
- **mypasswd.sh**：修改密码
  - 普通用户：只能修改自己的密码（需验证旧密码）
  - root用户：可以修改任何用户的密码

## 系统架构

### 多进程架构

```
┌─────────────────────────────────────┐
│     主Shell进程 (mini_linux_shell)  │
│  - 显示提示符                        │
│  - 读取用户输入                      │
│  - 解析命令                          │
│  - 判断命令类型                      │
└──────────┬──────────────────────────┘
           │
           ├─── 内置命令 ────> 直接在主进程执行
           │                   (help/exit/users/passwd/useradd/userdel/history 等)
           │
           ├─── 内建命令 ────> 直接在主进程执行（C函数）
           │                   (mytouch/mycat/mycp/myrm/myls/myps/mycd/mymkdir/myecho/myvi/agent)
           │
           └─── Shell脚本 ──> system()调用
                               (create_user.sh, delete_user.sh...)
```

### 命令执行流程

1. **显示提示符**：`用户名@主机名:当前目录$`
2. **读取命令**：接收用户输入
3. **解析命令**：分割成命令和参数
4. **判断类型**：
   - exit → 退出Shell
   - 内置命令 → 直接执行
   - Shell脚本（以#!/bin/bash开头的可执行文件）→ system()调用
   - C程序/系统命令 → fork() + execvp()
   - 无效命令 → 显示"command not found"
5. **记录历史**：追加到历史文件
6. **循环**：返回步骤1

## 文件结构

```
Mini_computer/
├── src/
│   ├── app/
│   │   └── main.c                # 主程序入口
│   ├── core/                     # Shell核心与公共模块
│   │   ├── shell.c               # Shell交互与命令分发
│   │   ├── util.c                # 工具函数
│   │   ├── history/              # 历史记录模块
│   │   └── user/                 # 用户模块
│   └── commands/                 # 内建命令实现（也可单独编译为独立程序）
│       ├── mytouch.c             # 创建文件
│       ├── mycat.c               # 显示文件内容
│       ├── mycp.c                # 复制文件
│       ├── myrm.c                # 删除文件/目录
│       ├── myls.c                # 列出目录
│       ├── myps.c                # 显示进程信息
│       ├── mycd.c                # 切换目录
│       ├── mymkdir.c             # 创建目录
│       ├── myecho.c              # 写入文件（重定向）
│       ├── myvi.c                # 简易编辑器
│       └── myagent.c             # AI终端助手
├── scripts/                      # Shell脚本（用户管理）
│   ├── create_user.sh            # 创建用户
│   ├── delete_user.sh            # 删除用户
│   └── change_password.sh        # 修改密码
├── bin/                          # 独立命令与脚本输出目录
│   ├── mytouch                   # 各命令可执行文件（可选）
│   ├── mycat
│   └── ...
├── docs/                         # 文档目录
│   ├── 任务说明.md               # 作业要求
│   └── 分工说明.md               # 团队分工
├── Makefile                      # 编译脚本
└── README.md                     # 本文档
```

## 编译说明

### 前置条件

- **操作系统**：Linux（推荐Ubuntu 20.04+）
- **编译器**：GCC
- **工具**：Make
- **权限**：部分功能需要root权限（用户管理脚本）

### 编译步骤

1. **克隆或进入项目目录**
   ```bash
   cd Mini_computer
   ```

2. **编译所有程序**
   ```bash
   make
   ```
   
   这将：
   - 编译主程序到 `./mini_linux_shell`
   - （可选）编译独立命令到 `bin/` 目录
   - 复制Shell脚本到 `bin/` 目录并添加执行权限

3. **查看编译结果**
   ```bash
   ls -lh .
   ls -lh bin/
   ```

4. **清理编译产物**（可选）
   ```bash
   make clean
   ```

5. **重新编译**
   ```bash
   make rebuild
   ```

6. **运行Shell**
   ```bash
   make run
   # 或直接运行
   ./mini_linux_shell
   ```

### Makefile目标

- `make` 或 `make all` - 编译所有程序
- `make clean` - 清理编译产物
- `make rebuild` - 重新编译
- `make run` - 编译并运行
- `make install` - 安装到系统（/usr/local/bin）
- `make uninstall` - 从系统卸载
- `make test-compile` - 测试编译
- `make help` - 显示帮助信息

## 使用指南

### 启动Shell

```bash
./bin/mini_shell
```

启动后会显示：
```
========================================
  欢迎使用 Mini Linux Shell
  输入 'help' 查看可用命令
  输入 'exit' 退出Shell
========================================

user@hostname:~$
```

### 内置命令

- **help** - 显示帮助信息
  ```bash
  user@hostname:~$ help
  ```

- **exit** - 退出Shell
  ```bash
  user@hostname:~$ exit
  ```

- **cd** - 切换目录
  ```bash
  user@hostname:~$ cd /tmp
  user@hostname:/tmp$ cd
  user@hostname:~$
  ```

- **pwd** - 显示当前目录
  ```bash
  user@hostname:~$ pwd
  /home/user
  ```

### 文件操作

- **创建文件**
  ```bash
  user@hostname:~$ mytouch test.txt
  ```

- **显示文件内容**
  ```bash
  user@hostname:~$ mycat test.txt
  ```

- **重定向输出**
  ```bash
  user@hostname:~$ mycat file1.txt > file2.txt
  user@hostname:~$ mycat file1.txt >> file2.txt
  ```

- **复制文件**
  ```bash
  user@hostname:~$ mycp source.txt dest.txt
  ```

- **删除文件**
  ```bash
  # 交互式删除
  user@hostname:~$ myrm -i file.txt
  # 递归删除目录
  user@hostname:~$ myrm -r mydir
  ```

- **查看文件权限**
  ```bash
  user@hostname:~$ mychmod test.txt
  ```

- **修改文件权限**
  ```bash
  user@hostname:~$ mychmod 755 test.txt
  ```

### 目录操作

- **列出目录内容**
  ```bash
  # 简短格式
  user@hostname:~$ myls
  # 长格式
  user@hostname:~$ myls -l
  # 指定目录
  user@hostname:~$ myls /home
  ```

### 进程管理

- **显示进程信息**
  ```bash
  # 显示所有进程
  user@hostname:~$ myps -a
  # 显示当前用户进程
  user@hostname:~$ myps -u
  ```

- **终止进程**
  ```bash
  user@hostname:~$ mykill process_name
  ```

### 命令历史

- **显示所有历史**
  ```bash
  user@hostname:~$ myhistory
  # 或
  user@hostname:~$ myhistory -a
  ```

- **显示最近N条**
  ```bash
  user@hostname:~$ myhistory -n 10
  ```

- **清空历史**
  ```bash
  user@hostname:~$ myhistory -c
  ```

### 用户管理（需要root权限）

- **创建用户（交互式）**
  ```bash
  user@hostname:~$ myuseradd
  ```

- **批量创建用户**
  ```bash
  # 准备用户列表文件 users.txt
  # 格式：username:password（每行一个）
  user@hostname:~$ myuseradd --batch users.txt
  ```

- **删除用户**
  ```bash
  user@hostname:~$ myuserdel username
  ```

- **修改密码**
  ```bash
  # 普通用户修改自己的密码
  user@hostname:~$ mypasswd
  
  # root用户修改他人密码
  user@hostname:~$ mypasswd username
  ```

## 实现细节

### 命令解析

使用 `strtok()` 函数将输入字符串按空格和制表符分割成参数数组。

### 命令类型判断

1. 检查是否为内置命令（exit, cd, pwd, help）
2. 检查是否为Shell脚本：
   - 文件可执行
   - 第一行包含 `#!/bin/bash` 或 `#!/bin/sh`
3. 否则视为C程序或系统命令

### C程序执行

```c
pid_t pid = fork();
if (pid == 0) {
    // 子进程
    execvp(args[0], args);
} else {
    // 父进程
    int status;
    waitpid(pid, &status, 0);
}
```

### Shell脚本执行

```c
system(script_path);
```

### 历史记录

- 存储位置：`~/.mini_shell_history`
- 每次执行命令后自动追加
- 使用链表或文件读写实现历史管理

### 动态提示符

- 获取当前用户名：`getpwuid(getuid())`
- 获取主机名：`gethostname()`
- 获取当前目录：`getcwd()`
- 颜色输出：使用ANSI转义序列

## 开发文档

### 项目要求

详细的项目要求请参考：
- [任务说明文档](docs/任务说明.md)

### 设计原则

1. **每个命令一个独立文件**：不使用函数库形式
2. **C程序与Shell脚本分离**：文件操作用C，用户管理用Shell
3. **标准POSIX接口**：使用标准系统调用
4. **错误处理**：所有命令都有完善的错误处理和提示
5. **边界条件**：考虑文件不存在、权限不足等情况

### 技术栈

- **编程语言**：C语言（C11标准）、Bash Shell
- **系统调用**：fork, execvp, waitpid, open, read, write, stat等
- **编译工具**：GCC, Make

### 扩展功能

- ✅ 动态提示符
- ✅ 交互式删除确认
- ✅ 文件重定向支持
- ✅ 批量用户创建
- ✅ 颜色输出（目录、可执行文件）

## 许可证

本项目采用 Apache-2.0 许可证开源。

---

**开发团队**：详见 [分工说明](docs/分工说明.md)

**最后更新**：2025年12月

# Windows 平台编译说明

本项目现在已支持在 Windows 环境下使用 MinGW 直接编译运行。

## 编译方法 (推荐)

### 1. 安装 MinGW-w64
确保你的系统已安装 MinGW-w64 并且 `gcc` 和 `mingw32-make` 已加入系统环境变量 PATH 中。

### 2. 执行编译
在项目根目录下打开 PowerShell 或 CMD，运行：
```powershell
mingw32-make clean
mingw32-make
```

### 3. 运行
```powershell
.\mini_linux_shell.exe
```

## 注意事项

- **控制台编码**：程序会自动尝试执行 `chcp 65001` 以支持 UTF-8 显示。
- **功能限制**：
  - `myps`：由于 Windows 不支持 `/proc` 文件系统，该命令在 Windows 下仅显示提示信息。
  - `myls -l`：用户名和组名在 Windows 下显示为固定占位符。
  - `myvi`：在 Windows 下支持基本的编辑功能。

## 备选方案 (如果需要完整 Linux 体验)

### 方案一：使用 WSL (Windows Subsystem for Linux) - 推荐

WSL 提供了完整的 Linux 环境，是在 Windows 上运行 Linux 程序的最佳方案。

1. **安装 WSL**
   ```powershell
   # 在 PowerShell（管理员权限）中运行
   wsl --install
   ```

2. **在 WSL 中编译运行**
   ```bash
   # 在 WSL 终端中
   cd /mnt/c/Users/Administrator/Desktop/v2/mini-linux
   make
   ./mini_linux_shell
   ```

### 方案二：使用 Cygwin

Cygwin 提供了 POSIX 兼容层，可以运行 Linux 程序。

1. 下载并安装 [Cygwin](https://www.cygwin.com/)
2. 在安装时选择 gcc、make、binutils 等开发工具
3. 在 Cygwin 终端中编译运行

### 方案三：使用 MinGW + MSYS2

MSYS2 提供了 Unix 工具链和编译环境。

1. 下载并安装 [MSYS2](https://www.msys2.org/)
2. 安装开发工具：
   ```bash
   pacman -S mingw-w64-x86_64-gcc make
   ```
3. 在 MSYS2 终端中编译运行

## Agent 命令的特殊说明

`agent` 命令（小晨AI终端助手）依赖 Python 环境，在 Windows 上可以直接使用：

### 前置条件

1. 安装 Python 3.x
2. 依赖包会在**首次运行时自动安装**，无需手动操作

### 自动依赖安装

项目内置了智能依赖检查和安装功能：

- ✅ 首次运行时自动检测缺失的 Python 包
- ✅ 自动调用 pip 安装所需依赖（requests、urllib3、colorama）
- ✅ 显示友好的安装进度和结果提示
- ✅ 安装失败时提供手动安装指导

如需手动安装依赖，可执行：
```powershell
pip install -r requirements.txt
```

### 设置环境变量

在 PowerShell 中：
```powershell
$env:VOID_API_KEY = "你的API密钥"
$env:VOID_BASE_URL = "https://api.deepseek.com"
$env:VOID_MODEL = "deepseek-chat"
```

### 独立使用 Agent（不依赖 Shell）

如果只想使用 AI 助手功能，可以直接运行：

```powershell
# 进入项目目录
cd C:\Users\Administrator\Desktop\v2\mini-linux

# 直接运行 Python 模块
python -m xiaochen_agent_v2
```

或者创建一个快捷启动脚本 `start_agent.bat`：
```batch
@echo off
chcp 65001 >nul
python -m xiaochen_agent_v2
pause
```

## 总结

- **完整 Shell 功能**：请使用 WSL、Cygwin 或 MSYS2
- **仅使用 Agent**：可以在原生 Windows 上通过 Python 直接运行
- **最佳实践**：推荐使用 WSL，获得完整的 Linux 开发体验

## 常见问题

**Q: 为什么不能在 Windows 上编译？**

A: 项目使用了大量 Linux 特定的系统调用（如 fork、exec、/proc 文件系统等），这些在 Windows 上没有直接对应的实现。

**Q: 可以移植到 Windows 吗？**

A: 理论上可以，但需要：
- 替换所有 POSIX 特定的函数调用
- 使用 Windows API 重新实现进程管理
- 修改文件系统操作以适配 Windows
- 这是一个较大的工程，建议使用 WSL 更为实际

**Q: Agent 命令需要编译吗？**

A: 不需要。Agent 是 Python 实现的，只要有 Python 环境和必要的依赖包即可运行。


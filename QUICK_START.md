# Agent 命令快速上手指南

## 🚀 快速开始

### 第一步：设置 API 密钥

在使用 `agent` 命令前，需要设置环境变量。

#### Linux / WSL / Mac
```bash
export VOID_API_KEY="你的API密钥"
export VOID_BASE_URL="https://api.deepseek.com"  # 可选
export VOID_MODEL="deepseek-chat"  # 可选
```

#### Windows PowerShell
```powershell
$env:VOID_API_KEY = "你的API密钥"
$env:VOID_BASE_URL = "https://api.deepseek.com"  # 可选
$env:VOID_MODEL = "deepseek-chat"  # 可选
```

#### Windows CMD
```cmd
set VOID_API_KEY=你的API密钥
set VOID_BASE_URL=https://api.deepseek.com
set VOID_MODEL=deepseek-chat
```

### 第二步：运行 Agent

#### 方式一：在 Mini Linux Shell 中使用（Linux/WSL）

```bash
# 编译并运行 shell
make
./mini_linux_shell

# 登录后使用 agent 命令
root@mini-linux:$ agent                    # 交互式模式
root@mini-linux:$ agent 帮我创建一个文件    # 单次执行
```

#### 方式二：独立运行（所有平台）

**Windows PowerShell:**
```powershell
.\start_agent.ps1
```

**Windows CMD:**
```cmd
start_agent.bat
```

**Linux / WSL / Mac:**
```bash
python3 -m xiaochen_agent_v2
```

## ✨ 首次运行

首次运行时，系统会：

1. ✅ 自动检测 Python 环境
2. ✅ 检查必需的依赖包（requests, urllib3, colorama）
3. ✅ 自动安装缺失的依赖
4. ✅ 启动 AI 助手

你会看到类似这样的输出：

```
====================================
检测到缺失的 Python 依赖包
====================================
缺失的包: colorama>=0.4.6

正在自动安装依赖...
====================================

Collecting colorama>=0.4.6
  Downloading colorama-0.4.6-py2.py3-none-any.whl (25 kB)
Installing collected packages: colorama
Successfully installed colorama-0.4.6

====================================
✓ 依赖安装成功！
====================================

=== 小晨终端助手 (XIAOCHEN_TERMINAL) ===
```

## 💡 使用示例

### 交互式对话
```bash
User: 帮我创建一个名为 test.txt 的文件，内容是 Hello World

[小晨终端助手]: 好的，我来帮你创建文件...
```

### 文件操作
```bash
User: 显示当前目录的所有 .c 文件

User: 帮我在 myagent.c 中添加一个注释

User: 搜索项目中所有包含 "agent" 的文件
```

### 代码编写
```bash
User: 帮我写一个 Python 脚本，用于批量重命名文件

User: 优化 myagent.c 中的错误处理逻辑
```

### 项目管理
```bash
User: 分析当前项目的文件结构

User: 检查代码中是否有潜在的 bug

User: 生成项目的 .gitignore 文件
```

## 🔧 常见问题

### Q1: 依赖安装失败怎么办？

**A:** 手动安装依赖：
```bash
pip install -r requirements.txt
```

或单独安装：
```bash
pip install requests urllib3 colorama
```

### Q2: 提示 "No module named 'pip'" 怎么办？

**A:** 升级或重新安装 pip：
```bash
# Linux/Mac
python3 -m ensurepip --upgrade

# Windows
python -m ensurepip --upgrade
```

### Q3: 没有设置 API Key 会怎样？

**A:** 首次运行时会提示你选择模型并输入 API Key：
```
=== 小晨终端助手 (XIAOCHEN_TERMINAL) ===
1. DeepSeek (Default)
2. Doubao (Volcano Ark)

Select model (default 1): 1
Enter API Key for deepseek-chat: 
```

### Q4: 如何退出 Agent？

**A:** 在交互模式中输入以下任一命令：
- `exit`
- `quit`
- 或按 `Ctrl+C`

### Q5: Agent 支持哪些操作？

**A:** Agent 支持：
- 📁 读写文件
- 🔍 搜索文件和代码
- ✏️ 编辑代码
- 🖥️ 执行终端命令（需确认）
- 📋 任务管理
- 💾 操作历史与回滚

## 📚 更多信息

- 完整文档：[README.md](README.md)
- Windows 编译说明：[WINDOWS_BUILD.md](WINDOWS_BUILD.md)
- 项目分工：[分工说明.md](分工说明.md)

## 🎯 获取 API Key

### DeepSeek
1. 访问 https://platform.deepseek.com/
2. 注册并登录
3. 在 API Keys 页面创建新密钥

### 其他模型提供商
- OpenAI: https://platform.openai.com/
- 豆包（Doubao）: https://console.volcengine.com/ark

## 💬 反馈与支持

如遇到问题或有建议，欢迎：
- 提交 GitHub Issue
- 查看项目文档
- 联系项目维护者

---

**开始你的 AI 编程之旅吧！** 🚀


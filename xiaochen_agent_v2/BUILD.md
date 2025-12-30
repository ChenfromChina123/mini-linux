# 小晨Agent打包说明

本文档说明如何将小晨Agent打包为Linux可执行文件，无需额外依赖。

## 打包方法

提供两种打包方法，推荐使用方法二（更快更小）。

### 方法一：使用PyInstaller（完全独立）

**优点**：
- 完全独立，不需要系统Python环境
- 打包后的文件可以在任何Linux系统运行

**缺点**：
- 文件较大（约30-50MB）
- 打包时间较长（3-5分钟）

**步骤**：

```bash
cd xiaochen_agent_v2

# 1. 安装PyInstaller
pip3 install pyinstaller

# 2. 运行打包脚本
chmod +x build_package.sh
./build_package.sh

# 3. 生成的文件在 dist/ 目录
ls -lh dist/xiaochen_agent
```

### 方法二：使用shiv（推荐）

**优点**：
- 文件小（约5-10MB）
- 打包速度快（30秒内）
- 启动速度快

**缺点**：
- 需要目标系统有Python 3.7+环境

**步骤**：

```bash
cd xiaochen_agent_v2

# 1. 安装shiv
pip3 install shiv

# 2. 运行打包脚本
chmod +x build_simple.sh
./build_simple.sh

# 3. 生成的文件在 dist/ 目录
ls -lh dist/xiaochen_agent
```

### 方法三：从主项目构建

在主项目目录直接构建：

```bash
# 在Mini_computer目录
chmod +x scripts/build_agent.sh
./scripts/build_agent.sh

# 可执行文件会复制到 bin/xiaochen_agent
```

## 使用打包后的程序

### 1. 设置环境变量

```bash
export VOID_API_KEY="your_api_key_here"
export VOID_BASE_URL="https://api.deepseek.com"  # 可选
export VOID_MODEL="deepseek-chat"  # 可选
```

### 2. 运行程序

```bash
# 交互式模式
./dist/xiaochen_agent

# 单条命令模式
./dist/xiaochen_agent "帮我创建一个测试文件"

# 显示帮助
./dist/xiaochen_agent --help
```

### 3. 安装到系统（可选）

```bash
# 复制到系统路径
sudo cp dist/xiaochen_agent /usr/local/bin/

# 然后可以直接使用
xiaochen_agent
```

## 发布包

运行 `build_package.sh` 会自动创建发布包：

```
xiaochen_agent_linux_20251230.tar.gz
├── xiaochen_agent          # 可执行文件
├── config.json.example     # 配置示例
└── README.txt              # 使用说明
```

可以直接分发这个tar.gz文件。

## 配置文件

首次运行时，程序会在以下位置创建配置：

```
~/.config/xiaochen_agent/
├── config.json             # 配置文件
└── logs/                   # 日志目录
    ├── sessions/           # 会话记录
    └── void_usage_history.jsonl  # 使用历史
```

## 故障排除

### 1. 权限错误

```bash
chmod +x dist/xiaochen_agent
```

### 2. 找不到Python（仅shiv方式）

确保系统安装了Python 3.7+：

```bash
python3 --version
```

### 3. API密钥未设置

确保设置了环境变量：

```bash
echo $VOID_API_KEY
```

### 4. 依赖缺失（PyInstaller方式不应出现）

如果使用shiv方式，确保安装了依赖：

```bash
pip3 install -r requirements.txt
```

## 技术细节

### PyInstaller打包原理

1. 分析Python程序的所有依赖
2. 将Python解释器、依赖库、程序代码打包到一个文件
3. 创建启动器，运行时解压到临时目录执行

### shiv打包原理

1. 创建一个包含所有依赖的zip文件
2. 在文件头部添加shebang（#!/usr/bin/env python3）
3. 运行时使用系统Python解释器执行

### 文件大小对比

| 方法 | 文件大小 | 打包时间 | 启动时间 |
|------|---------|---------|---------|
| PyInstaller | 30-50MB | 3-5分钟 | 1-2秒 |
| shiv | 5-10MB | 30秒 | <1秒 |

## 更新Makefile

已更新主项目的Makefile，添加了agent构建目标：

```bash
# 编译Shell和命令
make

# 构建Agent可执行文件
make agent

# 完整构建（包括Agent）
make all-with-agent
```

## 注意事项

1. **环境变量**：API密钥必须通过环境变量设置，不要硬编码
2. **权限**：某些操作可能需要sudo权限
3. **网络**：需要网络连接访问API
4. **Python版本**：shiv方式需要目标系统有Python 3.7+
5. **系统兼容性**：在相同或更新的Linux发行版上运行

## 推荐配置

对于不同场景的推荐：

- **开发环境**：直接使用Python运行（`python3 run.py`）
- **测试环境**：使用shiv打包（快速迭代）
- **生产环境**：使用PyInstaller打包（完全独立）
- **分发给用户**：使用PyInstaller打包（无需Python环境）

## 示例：完整构建流程

```bash
# 1. 进入项目目录
cd Mini_computer

# 2. 构建Shell系统
make

# 3. 构建Agent
./scripts/build_agent.sh

# 4. 测试
export VOID_API_KEY="sk-xxx"
./bin/xiaochen_agent --help

# 5. 运行Shell
./bin/mini_shell

# 6. 在Shell中使用Agent
user@host:~$ agent "显示当前目录"
```

## 相关文档

- [README.md](README.md) - Agent使用说明
- [../README.md](../README.md) - 项目总体说明
- [../docs/任务实现说明.md](../docs/任务实现说明.md) - 实现细节


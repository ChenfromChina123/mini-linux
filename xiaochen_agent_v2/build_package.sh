#!/bin/bash
#
# build_package.sh - 打包xiaochen_agent为Linux可执行文件
# 功能：使用PyInstaller将Python程序打包为单个可执行文件
# 使用：./build_package.sh
#

set -e  # 遇到错误立即退出

echo "========================================"
echo "  小晨Agent Linux打包工具"
echo "========================================"
echo

# 检查Python版本
echo "[1/6] 检查Python环境..."
if ! command -v python3 &> /dev/null; then
    echo "错误: 未找到python3"
    exit 1
fi

PYTHON_VERSION=$(python3 --version | awk '{print $2}')
echo "✓ Python版本: $PYTHON_VERSION"
echo

# 检查并安装依赖
echo "[2/6] 检查依赖包..."
if ! python3 -c "import PyInstaller" &> /dev/null; then
    echo "PyInstaller未安装，正在安装..."
    pip3 install pyinstaller
else
    echo "✓ PyInstaller已安装"
fi

# 安装项目依赖
echo "安装项目依赖..."
pip3 install -r requirements.txt
echo "✓ 依赖包安装完成"
echo

# 清理旧的构建文件
echo "[3/6] 清理旧的构建文件..."
rm -rf build dist __pycache__
rm -f xiaochen_agent.spec
echo "✓ 清理完成"
echo

# 使用PyInstaller打包
echo "[4/6] 开始打包..."
echo "这可能需要几分钟时间，请耐心等待..."
echo

if [ -f "build_linux.spec" ]; then
    # 使用自定义spec文件
    pyinstaller build_linux.spec
else
    # 使用默认配置
    pyinstaller --onefile \
                --name xiaochen_agent \
                --add-data "config.json.example:." \
                --add-data "README.md:." \
                --hidden-import=xiaochen_agent_v2 \
                --hidden-import=xiaochen_agent_v2.core \
                --hidden-import=xiaochen_agent_v2.ui \
                --hidden-import=xiaochen_agent_v2.utils \
                --console \
                run.py
fi

echo
echo "✓ 打包完成"
echo

# 检查生成的文件
echo "[5/6] 验证打包结果..."
if [ -f "dist/xiaochen_agent" ]; then
    chmod +x dist/xiaochen_agent
    FILE_SIZE=$(du -h dist/xiaochen_agent | cut -f1)
    echo "✓ 可执行文件已生成: dist/xiaochen_agent"
    echo "  文件大小: $FILE_SIZE"
else
    echo "✗ 打包失败：未找到可执行文件"
    exit 1
fi
echo

# 创建发布包
echo "[6/6] 创建发布包..."
RELEASE_DIR="xiaochen_agent_linux_$(date +%Y%m%d)"
mkdir -p "$RELEASE_DIR"

# 复制可执行文件
cp dist/xiaochen_agent "$RELEASE_DIR/"

# 复制配置示例
cp config.json.example "$RELEASE_DIR/"

# 创建README
cat > "$RELEASE_DIR/README.txt" << 'EOF'
小晨AI终端助手 - Linux版本
============================

使用方法：

1. 设置环境变量（必需）：
   export VOID_API_KEY="你的API密钥"
   export VOID_BASE_URL="https://api.deepseek.com"  # 可选
   export VOID_MODEL="deepseek-chat"  # 可选

2. 运行程序：
   ./xiaochen_agent

3. 或者直接执行命令：
   ./xiaochen_agent "帮我创建一个测试文件"

配置文件：
- 首次运行会在 ~/.config/xiaochen_agent/ 目录创建配置文件
- 可以复制 config.json.example 到该目录并重命名为 config.json 进行自定义配置

注意事项：
- 确保文件有执行权限：chmod +x xiaochen_agent
- 如果遇到权限问题，请使用 sudo 运行
- 日志文件保存在 ~/.config/xiaochen_agent/logs/

更多信息请访问项目主页。
EOF

# 打包为tar.gz
tar -czf "${RELEASE_DIR}.tar.gz" "$RELEASE_DIR"
echo "✓ 发布包已创建: ${RELEASE_DIR}.tar.gz"
echo

# 显示总结
echo "========================================"
echo "  打包完成！"
echo "========================================"
echo
echo "生成的文件："
echo "  1. 可执行文件: dist/xiaochen_agent"
echo "  2. 发布包: ${RELEASE_DIR}.tar.gz"
echo
echo "测试运行："
echo "  cd dist"
echo "  export VOID_API_KEY=\"your_api_key\""
echo "  ./xiaochen_agent --help"
echo
echo "安装到系统："
echo "  sudo cp dist/xiaochen_agent /usr/local/bin/"
echo
echo "========================================"


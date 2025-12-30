#!/bin/bash
#
# build_simple.sh - 简化版打包脚本
# 使用shiv创建单文件Python应用（更小更快）
#

set -e

echo "========================================"
echo "  小晨Agent 简化打包工具 (使用shiv)"
echo "========================================"
echo

# 检查shiv
echo "[1/4] 检查shiv..."
if ! command -v shiv &> /dev/null; then
    echo "shiv未安装，正在安装..."
    pip3 install shiv
fi
echo "✓ shiv已安装"
echo

# 安装依赖
echo "[2/4] 安装依赖..."
pip3 install -r requirements.txt
echo "✓ 依赖安装完成"
echo

# 创建打包目录
echo "[3/4] 打包..."
rm -rf dist
mkdir -p dist

# 使用shiv打包
shiv -c xiaochen_agent \
     -o dist/xiaochen_agent \
     --python '/usr/bin/env python3' \
     -e xiaochen_agent_v2.ui.cli:run_cli \
     .

chmod +x dist/xiaochen_agent
echo "✓ 打包完成"
echo

# 显示结果
echo "[4/4] 完成！"
FILE_SIZE=$(du -h dist/xiaochen_agent | cut -f1)
echo "可执行文件: dist/xiaochen_agent"
echo "文件大小: $FILE_SIZE"
echo
echo "测试运行："
echo "  export VOID_API_KEY=\"your_api_key\""
echo "  ./dist/xiaochen_agent --help"
echo


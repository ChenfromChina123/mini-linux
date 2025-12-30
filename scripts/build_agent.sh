#!/bin/bash
#
# build_agent.sh - 在主项目目录构建Agent可执行文件
# 功能：将xiaochen_agent_v2打包并复制到bin目录
#

set -e

echo "========================================"
echo "  构建小晨Agent可执行文件"
echo "========================================"
echo

# 进入agent目录
cd xiaochen_agent_v2

# 检查打包工具
if command -v pyinstaller &> /dev/null; then
    echo "使用PyInstaller打包..."
    ./build_package.sh
elif command -v shiv &> /dev/null; then
    echo "使用shiv打包..."
    ./build_simple.sh
else
    echo "错误: 未找到打包工具 (pyinstaller或shiv)"
    echo "请安装其中一个："
    echo "  pip3 install pyinstaller"
    echo "  或"
    echo "  pip3 install shiv"
    exit 1
fi

# 返回主目录
cd ..

# 复制到bin目录
echo
echo "复制到bin目录..."
mkdir -p bin
cp xiaochen_agent_v2/dist/xiaochen_agent bin/
chmod +x bin/xiaochen_agent

echo
echo "✓ 构建完成！"
echo "可执行文件位置: bin/xiaochen_agent"
echo
echo "使用方法："
echo "  export VOID_API_KEY=\"your_api_key\""
echo "  ./bin/xiaochen_agent"
echo


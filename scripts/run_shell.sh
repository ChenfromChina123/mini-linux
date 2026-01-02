#!/bin/bash
#
# run_shell.sh - 启动Mini Linux Shell并设置环境
#

# 获取脚本所在目录的父目录（项目根目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BIN_DIR="$PROJECT_ROOT/bin"

# 将bin目录添加到PATH
export PATH="$BIN_DIR:$PATH"

# 自动配置Agent智能助手
if [ -f "$BIN_DIR/xiaochen_terminal" ]; then
    if [ ! -f "$BIN_DIR/xiaochen_agent" ] || [ "$BIN_DIR/xiaochen_terminal" -nt "$BIN_DIR/xiaochen_agent" ]; then
        echo "正在配置Agent智能助手..."
        cp "$BIN_DIR/xiaochen_terminal" "$BIN_DIR/xiaochen_agent"
        chmod +x "$BIN_DIR/xiaochen_agent"
        echo "✓ Agent已配置完成"
        echo
    fi
elif [ -f "$PROJECT_ROOT/xiaochen_terminal" ]; then
    echo "正在配置Agent智能助手..."
    cp "$PROJECT_ROOT/xiaochen_terminal" "$BIN_DIR/xiaochen_agent"
    chmod +x "$BIN_DIR/xiaochen_agent"
    echo "✓ Agent已配置完成"
    echo
fi

# 启动Shell
echo "启动 Mini Linux Shell..."
echo "bin目录已添加到PATH: $BIN_DIR"
echo

exec "$BIN_DIR/mini_shell"


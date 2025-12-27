import sys
import os

# 在导入其他模块前，先检查并安装依赖
try:
    # 尝试导入依赖检查模块（不依赖其他第三方库）
    from .check_deps import check_and_install_dependencies
    
    # 检查并安装依赖
    if not check_and_install_dependencies():
        print("\n依赖安装失败，无法启动小晨终端助手")
        sys.exit(1)
except ImportError:
    # 如果 check_deps.py 不存在，跳过检查
    pass

# 依赖检查完成后，导入主程序
from .cli import run_cli

if __name__ == "__main__":
    run_cli()

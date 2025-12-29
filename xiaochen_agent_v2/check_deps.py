#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
依赖检查和自动安装脚本
用于检查 xiaochen_agent_v2 所需的 Python 依赖包
"""

import sys
import subprocess
from pathlib import Path


def check_and_install_dependencies():
    """
    检查并安装 requirements.txt 中列出的依赖包
    
    Returns:
        bool: 所有依赖已安装返回 True，否则返回 False
    """
    # 获取 requirements.txt 路径
    script_dir = Path(__file__).parent
    requirements_file = script_dir / "requirements.txt"
    
    if not requirements_file.exists():
        print(f"❌ 未找到 requirements.txt 文件: {requirements_file}")
        return False
    
    # 读取依赖列表
    try:
        with open(requirements_file, 'r', encoding='utf-8') as f:
            dependencies = [
                line.strip() 
                for line in f 
                if line.strip() and not line.strip().startswith('#')
            ]
    except Exception as e:
        print(f"❌ 读取 requirements.txt 失败: {e}")
        return False
    
    if not dependencies:
        print("✓ 无需安装依赖")
        return True
    
    # 检查每个依赖包
    missing_packages = []
    for dep in dependencies:
        package_name = dep.split('>=')[0].split('==')[0].strip()
        try:
            __import__(package_name)
        except ImportError:
            missing_packages.append(dep)
    
    if not missing_packages:
        print("✓ 所有依赖已安装")
        return True
    
    # 提示需要安装的包
    print(f"⚠️  检测到 {len(missing_packages)} 个缺失的依赖包:")
    for pkg in missing_packages:
        print(f"   - {pkg}")
    
    # 询问是否自动安装
    try:
        response = input("\n是否自动安装缺失的依赖? (y/n, 默认 y): ").strip().lower()
        if not response:
            response = 'y'
    except (EOFError, KeyboardInterrupt):
        response = 'y'
        print()
    
    if response != 'y':
        print("❌ 用户取消安装")
        return False
    
    # 自动安装依赖
    print(f"\n正在安装依赖包...")
    try:
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", "-q", "--upgrade", "pip"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", "-q", "-r", str(requirements_file)]
        )
        
        print("✓ 依赖安装成功")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ 依赖安装失败: {e}")
        print(f"\n请手动运行以下命令安装依赖:")
        print(f"  pip install -r {requirements_file}")
        return False
    except Exception as e:
        print(f"❌ 安装过程出错: {e}")
        return False


def main():
    """
    主函数：检查并安装依赖
    
    Returns:
        int: 成功返回 0，失败返回 1
    """
    print("=" * 50)
    print("小晨 AI 终端助手 - 依赖检查")
    print("=" * 50)
    
    success = check_and_install_dependencies()
    
    if success:
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())


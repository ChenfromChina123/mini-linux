#!/usr/bin/env python3
"""
依赖检查和自动安装脚本

功能：
1. 检查必需的 Python 包是否已安装
2. 如果缺失，自动安装依赖
3. 提供友好的错误提示
"""

import sys
import subprocess
import os


def check_and_install_dependencies():
    """
    检查并安装项目所需的依赖包
    
    Returns:
        bool: 所有依赖是否成功安装
    """
    # 必需的依赖包
    required_packages = {
        'requests': 'requests>=2.28.0',
        'urllib3': 'urllib3>=1.26.0',
        'colorama': 'colorama>=0.4.6',
    }
    
    missing_packages = []
    
    # 检查每个包是否已安装
    for package_name, package_spec in required_packages.items():
        try:
            __import__(package_name)
        except ImportError:
            missing_packages.append(package_spec)
    
    # 如果没有缺失的包，直接返回
    if not missing_packages:
        return True
    
    # 提示用户需要安装依赖
    print("\n" + "="*60)
    print("检测到缺失的 Python 依赖包")
    print("="*60)
    print(f"缺失的包: {', '.join(missing_packages)}")
    print("\n正在自动安装依赖...")
    print("="*60 + "\n")
    
    # 尝试自动安装
    try:
        # 获取 requirements.txt 路径
        script_dir = os.path.dirname(os.path.abspath(__file__))
        requirements_file = os.path.join(script_dir, 'requirements.txt')
        
        if os.path.exists(requirements_file):
            # 使用 requirements.txt 安装
            cmd = [sys.executable, '-m', 'pip', 'install', '-r', requirements_file]
        else:
            # 直接安装缺失的包
            cmd = [sys.executable, '-m', 'pip', 'install'] + missing_packages
        
        # 执行安装命令
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        if result.returncode == 0:
            print("\n" + "="*60)
            print("✓ 依赖安装成功！")
            print("="*60 + "\n")
            return True
        else:
            print("\n" + "="*60)
            print("✗ 依赖安装失败")
            print("="*60)
            print(f"错误信息: {result.stderr}")
            print("\n请手动安装依赖：")
            print(f"  pip install {' '.join(missing_packages)}")
            print("="*60 + "\n")
            return False
            
    except Exception as e:
        print("\n" + "="*60)
        print("✗ 自动安装依赖时出错")
        print("="*60)
        print(f"错误: {str(e)}")
        print("\n请手动安装依赖：")
        print(f"  pip install {' '.join(missing_packages)}")
        print("="*60 + "\n")
        return False


if __name__ == "__main__":
    success = check_and_install_dependencies()
    sys.exit(0 if success else 1)


#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
单次命令执行脚本
用于从 mini-Linux shell 中执行单条 AI 指令
"""

import sys
import os


def run_single_command(command: str):
    """
    执行单条 AI 命令
    
    Args:
        command: 用户输入的命令字符串
    """
    # 确保控制台编码为 UTF-8
    if sys.platform == "win32":
        import io
        sys.stdin = io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8')
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
        os.system('chcp 65001 > nul')
    
    # 导入 Agent
    try:
        from xiaochen_agent_v2.core.agent import VoidAgent
        from xiaochen_agent_v2.core.config import Config
        from xiaochen_agent_v2.core.config_manager import ConfigManager
        from xiaochen_agent_v2.utils.files import get_repo_root
    except ImportError as e:
        print(f"错误：无法导入必要模块: {e}")
        print("请确保已安装所有依赖: pip install -r requirements.txt")
        sys.exit(1)
    
    # 初始化配置管理器
    config_file = os.path.join(get_repo_root(), "config.json")
    config_manager = ConfigManager(config_file=config_file)
    saved_config = config_manager.load_config() if config_manager else {}
    
    # 优先级: 环境变量 > 配置文件 > 错误退出
    api_key = os.environ.get("VOID_API_KEY") or saved_config.get("api_key", "")
    base_url = os.environ.get("VOID_BASE_URL") or saved_config.get("base_url", "https://api.deepseek.com")
    model_name = os.environ.get("VOID_MODEL") or saved_config.get("model_name", "deepseek-chat")
    verify_ssl = saved_config.get("verify_ssl", True)
    
    if not api_key:
        print("错误：未设置 API Key")
        print("请设置环境变量 VOID_API_KEY 或在 config.json 中配置")
        sys.exit(1)
    
    # 创建 Agent 配置
    config = Config(
        apiKey=api_key,
        baseUrl=base_url,
        modelName=model_name,
        verifySsl=verify_ssl
    )
    
    # 应用白名单配置
    whitelisted_tools = saved_config.get("whitelisted_tools")
    whitelisted_commands = saved_config.get("whitelisted_commands")
    if isinstance(whitelisted_tools, list):
        config.whitelistedTools = whitelisted_tools
    if isinstance(whitelisted_commands, list):
        config.whitelistedCommands = whitelisted_commands
    
    # 创建 Agent 实例
    agent = VoidAgent(config)
    
    # 应用其他配置
    read_indent_mode = saved_config.get("read_indent_mode", "smart")
    python_validate_ruff = saved_config.get("python_validate_ruff", "auto")
    token_threshold = saved_config.get("token_threshold", 30000)
    
    agent.readIndentMode = str(read_indent_mode or "smart")
    agent.pythonValidateRuff = str(python_validate_ruff or "auto")
    
    # 执行命令
    try:
        agent.chat(command)
    except KeyboardInterrupt:
        print("\n\n用户中断执行")
        sys.exit(130)
    except Exception as e:
        print(f"\n\n执行出错: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


def main():
    """
    主函数：解析命令行参数并执行
    """
    if len(sys.argv) < 2:
        print("用法: python run_once.py <命令>")
        print("示例: python run_once.py '帮我创建一个测试文件'")
        sys.exit(1)
    
    # 将所有参数合并为一条命令
    command = " ".join(sys.argv[1:])
    
    if not command.strip():
        print("错误：命令不能为空")
        sys.exit(1)
    
    run_single_command(command)


if __name__ == "__main__":
    main()


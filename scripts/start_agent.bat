@echo off
REM 小晨AI终端助手启动脚本 (Windows)

REM 切换到项目根目录
cd /d "%~dp0.."

echo ====================================
echo   小晨AI终端助手 (XIAOCHEN_TERMINAL)
echo ====================================
echo.

REM 检查环境变量
if "%VOID_API_KEY%"=="" (
    echo [警告] 未设置环境变量 VOID_API_KEY
    echo.
    echo 请先设置环境变量：
    echo   set VOID_API_KEY=你的API密钥
    echo   set VOID_BASE_URL=https://api.deepseek.com
    echo   set VOID_MODEL=deepseek-chat
    echo.
    echo 或在 PowerShell 中：
    echo   $env:VOID_API_KEY = "你的API密钥"
    echo   $env:VOID_BASE_URL = "https://api.deepseek.com"
    echo   $env:VOID_MODEL = "deepseek-chat"
    echo.
)

REM 设置控制台编码为 UTF-8
chcp 65001 >nul

REM 检查并安装依赖
echo [信息] 正在检查 Python 依赖...
python xiaochen_agent_v2\check_deps.py
if %errorlevel% neq 0 (
    echo.
    echo [错误] 依赖检查失败
    pause
    exit /b 1
)

REM 启动 Agent
python -m xiaochen_agent_v2

if %errorlevel% neq 0 (
    echo.
    echo [错误] 启动失败，请检查：
    echo 1. Python 是否已安装
    echo 2. 依赖包是否已安装：pip install requests urllib3
    echo 3. 环境变量是否已正确设置
    echo.
    pause
)


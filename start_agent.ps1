# 小晨AI终端助手启动脚本 (PowerShell)
# 
# 使用说明：
# 1. 首先安装 Python 3.x
# 2. 安装依赖：pip install requests urllib3
# 3. 设置环境变量（在脚本中或系统环境变量）

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  小晨AI终端助手 (XIAOCHEN_TERMINAL)" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# 检查环境变量
if (-not $env:VOID_API_KEY) {
    Write-Host "[警告] 未设置环境变量 VOID_API_KEY" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "请先设置环境变量：" -ForegroundColor Yellow
    Write-Host '  $env:VOID_API_KEY = "你的API密钥"' -ForegroundColor White
    Write-Host '  $env:VOID_BASE_URL = "https://api.deepseek.com"' -ForegroundColor White
    Write-Host '  $env:VOID_MODEL = "deepseek-chat"' -ForegroundColor White
    Write-Host ""
    
    # 可选：在脚本中临时设置（取消下面的注释并填入你的密钥）
    # $env:VOID_API_KEY = "sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    # $env:VOID_BASE_URL = "https://api.deepseek.com"
    # $env:VOID_MODEL = "deepseek-chat"
}

# 检查 Python
try {
    $pythonVersion = python --version 2>&1
    Write-Host "[信息] 检测到 Python: $pythonVersion" -ForegroundColor Green
} catch {
    Write-Host "[错误] 未找到 Python，请先安装 Python 3.x" -ForegroundColor Red
    Read-Host "按任意键退出"
    exit 1
}

# 设置控制台编码为 UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$env:PYTHONIOENCODING = "utf-8"

# 检查并安装依赖
Write-Host "[信息] 正在检查 Python 依赖..." -ForegroundColor Green
$checkResult = & python xiaochen_agent_v2\check_deps.py
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[错误] 依赖检查失败" -ForegroundColor Red
    Read-Host "按任意键退出"
    exit 1
}

# 启动 Agent
Write-Host "[信息] 正在启动小晨AI终端助手..." -ForegroundColor Green
Write-Host ""

python -m xiaochen_agent_v2

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[错误] 启动失败，请检查：" -ForegroundColor Red
    Write-Host "1. Python 是否已安装" -ForegroundColor Yellow
    Write-Host "2. 依赖包是否已安装：pip install requests urllib3" -ForegroundColor Yellow
    Write-Host "3. 环境变量是否已正确设置" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "按任意键退出"
}


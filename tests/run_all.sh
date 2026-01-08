#!/bin/bash

# Mini Linux Shell 全面自动化测试脚本
# 运行方式: ./tests/run_all.sh

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 统计变量
PASSED=0
FAILED=0

log_success() { echo -e "${GREEN}[PASS]${NC} $1"; ((PASSED++)); }
log_fail() { echo -e "${RED}[FAIL]${NC} $1"; ((FAILED++)); }
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }

log_info "开始全面功能测试..."

# 1. 检查二进制文件是否存在
log_info "正在检查 bin/ 目录下的命令..."
CMDS=("mini_shell" "mycat" "mycp" "myls" "myrm" "mytouch" "mychmod" "mymkdir" "myps" "mycd" "mypasswd" "passwd" "myuseradd" "useradd")
for cmd in "${CMDS[@]}"; do
    if [ -f "bin/$cmd" ]; then
        log_success "命令 $cmd 已存在"
    else
        log_fail "命令 $cmd 缺失"
    fi
done

# 2. 检查换行符修复 (CRLF)
log_info "正在检查 Shell 脚本换行符..."
for script in bin/*.sh bin/mypasswd bin/myuseradd bin/myuserdel; do
    if [ -f "$script" ]; then
        if grep -q $'\r' "$script"; then
            log_fail "脚本 $script 包含 CRLF 字符 (\r)"
        else
            log_success "脚本 $script 格式正确 (Unix)"
        fi
    fi
done

# 3. 测试文件与目录操作链路 (Standalone)
log_info "正在测试文件操作链路 (Standalone模式)..."
TEST_DIR="test_workspace_$(date +%s)"
./bin/mymkdir -p "$TEST_DIR" && [ -d "$TEST_DIR" ] || log_fail "mymkdir 失败"
cd "$TEST_DIR" || exit

../bin/mytouch test.txt && [ -f "test.txt" ] || log_fail "mytouch 失败"
../bin/myecho "Hello Mini Linux" > test.txt
CONTENT=$(../bin/mycat test.txt)
if [ "$CONTENT" == "Hello Mini Linux" ]; then
    log_success "myecho/mycat 功能正常"
else
    log_fail "myecho/mycat 内容校验失败: $CONTENT"
fi

# 测试 mychmod
../bin/mychmod 777 test.txt
PERM=$(ls -l test.txt | cut -c 1-10)
if [[ "$PERM" == "-rwxrwxrwx" ]]; then
    log_success "mychmod 修改权限成功"
else
    log_fail "mychmod 权限校验失败: $PERM"
fi

# 清理
cd ..
./bin/myrm -r "$TEST_DIR"
if [ ! -d "$TEST_DIR" ]; then
    log_success "myrm 递归删除成功"
else
    log_fail "myrm 删除失败"
fi

# 4. 测试 Shell 内部集成功能 (Pipe模式)
log_info "正在测试 Shell 内部集成与别名..."
# 模拟 root 登录并执行 help 和 users
output=$(echo -e "root\nroot\nhelp\nusers\nexit" | ./bin/mini_shell 2>/dev/null)

if echo "$output" | grep -q "mypasswd/passwd"; then
    log_success "Shell 内部 passwd 别名提示已生效"
else
    log_fail "Shell 内部 passwd 别名提示缺失"
fi

if echo "$output" | grep -q "root"; then
    log_success "Shell 内部 users 命令执行成功"
else
    log_fail "Shell 内部 users 命令输出异常"
fi

# 5. 测试密码修改 (mypasswd 脚本别名)
log_info "正在测试 mypasswd 别名脚本..."
if [ -x "./bin/passwd" ]; then
    # 尝试运行 passwd --help (如果有的话) 或简单检查是否报错
    ./bin/passwd --help >/dev/null 2>&1
    log_success "passwd 脚本别名可执行"
else
    log_fail "passwd 脚本别名不可执行"
fi

echo -e "\n${BLUE}========================================${NC}"
echo -e "测试完成: ${GREEN}通过 $PASSED${NC}, ${RED}失败 $FAILED${NC}"
echo -e "${BLUE}========================================${NC}"

if [ $FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi

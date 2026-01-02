#!/bin/bash
#
# myuseradd.sh - 创建用户脚本 (Mini Shell 内部用户系统)
# 功能：创建新用户，支持交互式和批量创建
# 使用：myuseradd [--batch <file>]
#

# 更新 Mini Shell 的 .mini_users 用户管理文件
update_mini_users() {
    local username="$1"
    local password="$2"
    local is_root="${3:-0}"
    local db_file="$HOME/.mini_users"
    
    # 确保用户管理文件存在
    touch "$db_file"
    
    # 检查是否已存在于 .mini_users
    if grep -q "^$username	" "$db_file"; then
        # 更新密码和 root 状态
        sed -i "s/^$username	.*/$username	$password	$is_root/" "$db_file"
    else
        # 追加新用户
        echo -e "$username\t$password\t$is_root" >> "$db_file"
        #-e用于识别转义字符，如\t
    fi
}

# 检查用户是否已在用户管理文件中存在
check_user_exists() {
    local username="$1"
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        grep -q "^$username	" "$db_file"
        return $?
    fi
    return 1
}

# 交互式创建单个用户
create_user_interactive() {
    echo "========================================"
    echo "  交互式创建 Mini Shell 用户"
    echo "========================================"
    
    # 输入用户名
    read -p "请输入用户名: " username
    
    if [ -z "$username" ]; then
    #-z作用：检查字符串是否为空
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    # 检查用户是否已存在
    if check_user_exists "$username"; then
        echo "错误: 用户 '$username' 已存在"
        return 1
    fi
    
    # 输入密码
    read -s -p "请输入密码: " password
    echo
    read -s -p "请再次输入密码: " password2
    echo
    
    if [ "$password" != "$password2" ]; then
        echo "错误: 两次输入的密码不一致"
        return 1
    fi
    
    if [ -z "$password" ]; then
        echo "错误: 密码不能为空"
        return 1
    fi
    
    # 同步到 Mini Shell 用户管理文件
    update_mini_users "$username" "$password" 0
    
    echo "成功: Mini Shell 用户 '$username' 创建成功"
    return 0
}

# 批量创建用户
create_users_batch() {
    local user_file="$1"
    
    if [ ! -f "$user_file" ]; then
        echo "错误: 文件 '$user_file' 不存在"
        return 1
    fi
    
    echo "========================================"
    echo "  批量创建 Mini Shell 用户"
    echo "========================================"
    echo "从文件读取用户列表: $user_file"
    echo
    
    local success_count=0
    local fail_count=0
    
    # 读取文件，每行格式：username:password
    while IFS=: read -r username password || [ -n "$username" ]; do
        # 跳过空行和注释
        if [ -z "$username" ] || [[ "$username" =~ ^#.* ]]; then
            continue
        fi
        
        # 去除空格
        username=$(echo "$username" | tr -d ' ')
        password=$(echo "$password" | tr -d ' ')
        
        echo -n "创建用户 '$username'... "
        
        # 检查用户是否已存在
    if check_user_exists "$username"; then
            echo "跳过（已存在）"
            ((fail_count++))
            continue
        fi
        
        # 如果没有提供密码，设置默认密码
        if [ -z "$password" ]; then
            password="123456"
            echo -n "(使用默认密码) "
        fi
        
        # 同步到 Mini Shell 用户管理文件
        update_mini_users "$username" "$password" 0
        
        echo "成功"
        ((success_count++))
    done < "$user_file"
    
    echo
    echo "批量创建完成:"
    echo "  - 成功: $success_count 个用户"
    echo "  - 失败: $fail_count 个用户"
    
    return 0
}

# 主函数
main() {
    if [ "$1" = "--batch" ]; then
        if [ -z "$2" ]; then
            echo "错误: 请指定用户列表文件"
            echo "用法: myuseradd --batch <user_file>"
            exit 1
        fi
        create_users_batch "$2"
    else
        create_user_interactive
    fi
}

main "$@"

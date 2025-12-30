#!/bin/bash
#
# create_user.sh - 创建用户脚本
# 功能：创建新用户，支持交互式和批量创建
# 使用：./create_user.sh [--batch <file>]
#

# 检查是否为root用户
check_root() {
    if [ "$EUID" -ne 0 ]; then
        echo "错误: 此脚本需要root权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 更新 Mini Shell 的 .mini_users 数据库
update_mini_users() {
    local username="$1"
    local password="$2"
    local is_root="${3:-0}"
    local db_file="$HOME/.mini_users"
    
    # 确保数据库文件存在
    touch "$db_file"
    
    # 检查是否已存在于 .mini_users
    if grep -q "^$username	" "$db_file"; then
        # 更新密码和 root 状态
        sed -i "s/^$username	.*/$username	$password	$is_root/" "$db_file"
    else
        # 追加新用户
        echo -e "$username\t$password\t$is_root" >> "$db_file"
    fi
}

# 交互式创建单个用户
create_user_interactive() {
    echo "========================================"
    echo "  交互式创建用户"
    echo "========================================"
    
    # 输入用户名
    read -p "请输入用户名: " username
    
    if [ -z "$username" ]; then
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    # 检查用户是否已存在
    if id "$username" &>/dev/null; then
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
    
    # 创建用户
    useradd -m -s /bin/bash "$username"
    if [ $? -ne 0 ]; then
        echo "错误: 创建用户失败"
        return 1
    fi
    
    # 设置密码
    echo "$username:$password" | chpasswd
    if [ $? -ne 0 ]; then
        echo "错误: 设置密码失败"
        userdel -r "$username"
        return 1
    fi
    
    # 同步到 Mini Shell 数据库
    update_mini_users "$username" "$password" 0
    
    echo "成功: 用户 '$username' 创建成功"
    echo "  - 主目录: /home/$username"
    echo "  - Shell: /bin/bash"
    
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
    echo "  批量创建用户"
    echo "========================================"
    echo "从文件读取用户列表: $user_file"
    echo
    
    local success_count=0
    local fail_count=0
    
    # 读取文件，每行格式：username password
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
        if id "$username" &>/dev/null; then
            echo "跳过（已存在）"
            ((fail_count++))
            continue
        fi
        
        # 创建用户
        useradd -m -s /bin/bash "$username" &>/dev/null
        if [ $? -ne 0 ]; then
            echo "失败（创建用户）"
            ((fail_count++))
            continue
        fi
        
        # 设置密码
        if [ -n "$password" ]; then
            echo "$username:$password" | chpasswd &>/dev/null
            if [ $? -ne 0 ]; then
                echo "失败（设置密码）"
                userdel -r "$username" &>/dev/null
                ((fail_count++))
                continue
            fi
        else
            # 如果没有提供密码，设置默认密码
            password="123456"
            echo "$username:$password" | chpasswd &>/dev/null
            echo "成功（使用默认密码: 123456）"
        fi
        
        # 同步到 Mini Shell 数据库
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
    check_root
    
    if [ "$1" = "--batch" ]; then
        if [ -z "$2" ]; then
            echo "错误: 请指定用户列表文件"
            echo "用法: $0 --batch <user_file>"
            echo
            echo "文件格式（每行）："
            echo "  username:password"
            echo "  或"
            echo "  username（使用默认密码123456）"
            exit 1
        fi
        create_users_batch "$2"
    else
        create_user_interactive
    fi
}

main "$@"


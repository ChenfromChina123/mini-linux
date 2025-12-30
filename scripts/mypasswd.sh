#!/bin/bash
#
# change_password.sh - 修改用户密码脚本
# 功能：修改用户密码，区分普通用户和root用户权限
# 使用：./change_password.sh [username]
#

# 更新 Mini Shell 数据库中的密码
update_mini_users_password() {
    local username="$1"
    local new_password="$2"
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        # 获取原有的 is_root 状态
        local is_root=$(grep "^$username	" "$db_file" | cut -f3)
        if [ -n "$is_root" ]; then
            sed -i "s/^$username	.*/$username	$new_password	$is_root/" "$db_file"
        fi
    fi
}

# 普通用户修改自己的密码
change_own_password() {
    local username=$(whoami)
    
    echo "========================================"
    echo "  修改密码"
    echo "========================================"
    echo "当前用户: $username"
    echo
    
    # 验证旧密码（简化版，实际应该验证系统密码）
    read -s -p "请输入当前密码: " old_password
    echo
    
    # 注意：在实际系统中应该验证旧密码
    # 这里简化处理，假设已通过系统认证
    
    # 输入新密码
    read -s -p "请输入新密码: " new_password
    echo
    read -s -p "请再次输入新密码: " new_password2
    echo
    
    if [ "$new_password" != "$new_password2" ]; then
        echo "错误: 两次输入的密码不一致"
        return 1
    fi
    
    if [ -z "$new_password" ]; then
        echo "错误: 密码不能为空"
        return 1
    fi
    
    # 检查密码强度（至少6个字符）
    if [ ${#new_password} -lt 6 ]; then
        echo "错误: 密码长度至少为6个字符"
        return 1
    fi
    
    # 修改密码
    echo "$username:$new_password" | sudo chpasswd
    if [ $? -eq 0 ]; then
        update_mini_users_password "$username" "$new_password"
        echo "成功: 密码已修改"
    else
        echo "错误: 修改密码失败"
        return 1
    fi
    
    return 0
}

# root用户修改他人密码
change_other_password() {
    local target_user="$1"
    
    # 检查用户是否存在
    if ! id "$target_user" &>/dev/null; then
        echo "错误: 用户 '$target_user' 不存在"
        return 1
    fi
    
    echo "========================================"
    echo "  修改用户密码（root权限）"
    echo "========================================"
    echo "目标用户: $target_user"
    echo
    
    # 输入新密码
    read -s -p "请输入新密码: " new_password
    echo
    read -s -p "请再次输入新密码: " new_password2
    echo
    
    if [ "$new_password" != "$new_password2" ]; then
        echo "错误: 两次输入的密码不一致"
        return 1
    fi
    
    if [ -z "$new_password" ]; then
        echo "错误: 密码不能为空"
        return 1
    fi
    
    # 检查密码强度（至少6个字符）
    if [ ${#new_password} -lt 6 ]; then
        echo "错误: 密码长度至少为6个字符"
        return 1
    fi
    
    # 修改密码
    echo "$target_user:$new_password" | chpasswd
    if [ $? -eq 0 ]; then
        update_mini_users_password "$target_user" "$new_password"
        echo "成功: 用户 '$target_user' 的密码已修改"
    else
        echo "错误: 修改密码失败"
        return 1
    fi
    
    return 0
}

# root用户交互式修改密码
root_interactive_mode() {
    echo "========================================"
    echo "  修改用户密码（root权限）"
    echo "========================================"
    
    # 显示系统用户列表
    echo "当前系统用户列表:"
    echo "----------------------------------------"
    awk -F: '$3 >= 1000 && $3 < 65534 { printf "  - %s (UID: %d)\n", $1, $3 }' /etc/passwd
    echo "  - root (UID: 0)"
    echo "----------------------------------------"
    echo
    
    # 输入用户名
    read -p "请输入要修改密码的用户名: " target_user
    
    if [ -z "$target_user" ]; then
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    change_other_password "$target_user"
}

# 主函数
main() {
    # 检查是否为root用户
    if [ "$EUID" -eq 0 ]; then
        # root用户可以修改任何用户的密码
        if [ -z "$1" ]; then
            # 无参数，交互式选择用户
            root_interactive_mode
        else
            # 有参数，直接修改指定用户密码
            change_other_password "$1"
        fi
    else
        # 普通用户只能修改自己的密码
        if [ -z "$1" ] || [ "$1" = "$(whoami)" ]; then
            change_own_password
        else
            echo "错误: 普通用户只能修改自己的密码"
            echo "如需修改其他用户密码，请使用root权限运行此脚本"
            exit 1
        fi
    fi
}

main "$@"


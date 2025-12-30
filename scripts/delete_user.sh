#!/bin/bash
#
# delete_user.sh - 删除用户脚本
# 功能：删除指定用户，若用户不存在则给出提示
# 使用：./delete_user.sh [username]
#

# 检查是否为root用户
check_root() {
    if [ "$EUID" -ne 0 ]; then
        echo "错误: 此脚本需要root权限运行"
        echo "请使用: sudo $0"
        exit 1
    fi
}

# 删除用户
delete_user() {
    local username="$1"
    
    # 检查用户是否存在
    if ! id "$username" &>/dev/null; then
        echo "错误: 用户 '$username' 不存在"
        return 1
    fi
    
    # 检查是否为当前登录用户
    if [ "$username" = "$SUDO_USER" ] || [ "$username" = "$(whoami)" ]; then
        echo "错误: 不能删除当前登录的用户"
        return 1
    fi
    
    # 显示用户信息
    echo "========================================"
    echo "  用户信息"
    echo "========================================"
    local user_info=$(getent passwd "$username")
    local home_dir=$(echo "$user_info" | cut -d: -f6)
    local shell=$(echo "$user_info" | cut -d: -f7)
    
    echo "用户名: $username"
    echo "主目录: $home_dir"
    echo "Shell: $shell"
    echo
    
    # 确认删除
    read -p "确定要删除用户 '$username' 吗? (y/n): " choice
    
    if [ "$choice" != "y" ] && [ "$choice" != "Y" ]; then
        echo "已取消删除操作"
        return 0
    fi
    
    # 询问是否删除主目录
    read -p "是否同时删除主目录和邮件文件? (y/n): " delete_home
    
    if [ "$delete_home" = "y" ] || [ "$delete_home" = "Y" ]; then
        # 删除用户及其主目录
        userdel -r "$username"
        if [ $? -eq 0 ]; then
            echo "成功: 用户 '$username' 及其主目录已删除"
        else
            echo "错误: 删除用户失败"
            return 1
        fi
    else
        # 仅删除用户，保留主目录
        userdel "$username"
        if [ $? -eq 0 ]; then
            echo "成功: 用户 '$username' 已删除（主目录保留在 $home_dir）"
        else
            echo "错误: 删除用户失败"
            return 1
        fi
    fi
    
    return 0
}

# 交互式删除用户
delete_user_interactive() {
    echo "========================================"
    echo "  删除用户"
    echo "========================================"
    
    # 显示当前系统用户列表（UID >= 1000的普通用户）
    echo "当前系统用户列表:"
    echo "----------------------------------------"
    awk -F: '$3 >= 1000 && $3 < 65534 { printf "  - %s (UID: %d)\n", $1, $3 }' /etc/passwd
    echo "----------------------------------------"
    echo
    
    # 输入用户名
    read -p "请输入要删除的用户名: " username
    
    if [ -z "$username" ]; then
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    delete_user "$username"
}

# 主函数
main() {
    check_root
    
    if [ -z "$1" ]; then
        # 无参数，交互式模式
        delete_user_interactive
    else
        # 有参数，直接删除指定用户
        delete_user "$1"
    fi
}

main "$@"


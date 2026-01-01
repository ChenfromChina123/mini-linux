#!/bin/bash
#
# myuserdel.sh - 删除用户脚本 (Mini Shell 内部用户系统)
# 功能：删除指定用户，若用户不存在则给出提示
# 使用：myuserdel [username]
#

# 从 Mini Shell 用户管理文件删除
delete_from_mini_users() {
    local username=$1
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        sed -i "/^${username}\t/d" "$db_file"
    fi
}

# 检查用户是否在用户管理文件中
check_user_in_mini_users() {
    local username=$1
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        grep -q "^${username}\t" "$db_file"
        return $?
    fi
    return 1
}

# 删除用户函数
delete_user() {
    local username=$1
    
    # 检查用户是否在用户管理文件中存在
    if ! check_user_in_mini_users "$username"; then
        echo "错误: 用户 $username 不存在于 Mini Shell 管理文件中。"
        return 1
    fi
    
    # root 用户保护
    if [ "$username" == "root" ]; then
        echo "错误: 不能删除 root 用户。"
        return 1
    fi
    
    # 交互式确认
    read -p "确定要删除用户 $username 吗? (y/n): " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "操作已取消。"
        return 0
    fi
    
    # 从用户管理文件删除
    delete_from_mini_users "$username"
    echo "成功: Mini Shell 用户 '$username' 已删除"
    
    return 0
}

# 交互式删除用户
delete_user_interactive() {
    echo "========================================"
    echo "  删除 Mini Shell 用户"
    echo "========================================"
    
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        echo "当前 Mini Shell 用户列表:"
        echo "----------------------------------------"
        cut -f1 "$db_file" | while read u; do echo "  - $u"; done
        echo "----------------------------------------"
        echo
    fi
    
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
    if [ -z "$1" ]; then
        # 无参数，交互式模式
        delete_user_interactive
    else
        # 有参数，直接删除指定用户
        delete_user "$1"
    fi
}

main "$@"

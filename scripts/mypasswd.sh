#!/bin/bash
#
# mypasswd.sh - 修改 Mini Shell 用户密码脚本
# 功能：修改 Mini Shell 内部用户管理文件中的用户密码
# 使用：mypasswd [username]
#

# 更新 Mini Shell 用户管理文件中的密码
# 参数1: 用户名
# 参数2: 新密码
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

# 检查用户是否已在用户管理文件中存在
# 参数1: 用户名
check_user_exists() {
    local username="$1"
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        # 使用 grep 确保精确匹配用户名
        grep -q "^$username	" "$db_file"
        return $?
    fi
    return 1
}

# 修改用户密码
# 参数1: 目标用户名
change_password() {
    local target_user="$1"
    
    # 去除可能存在的回车符（Windows换行符处理）
    target_user=$(echo "$target_user" | tr -d '\r')
    
    # 检查用户是否存在
    if ! check_user_exists "$target_user"; then
        echo "错误: Mini Shell 用户 '$target_user' 不存在"
        return 1
    fi
    
    echo "========================================"
    echo "  修改 Mini Shell 用户密码"
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
    
    # 修改用户管理文件中的密码
    update_mini_users_password "$target_user" "$new_password"
    echo "成功: 用户 '$target_user' 的密码已修改"
    
    return 0
}

# 交互式模式，显示用户列表并选择
interactive_mode() {
    local db_file="$HOME/.mini_users"
    echo "========================================"
    echo "  修改 Mini Shell 用户密码"
    echo "========================================"
    
    if [ ! -f "$db_file" ]; then
        echo "错误: 用户管理文件不存在 ($db_file)"
        return 1
    fi
    
    # 显示 Mini Shell 用户列表
    echo "当前 Mini Shell 用户列表:"
    echo "----------------------------------------"
    awk -F'\t' '{ printf "  - %s (%s)\n", $1, ($3 == 1 ? "管理员" : "普通用户") }' "$db_file"
    echo "----------------------------------------"
    echo
    
    # 输入用户名
    read -p "请输入要修改密码的用户名: " target_user
    
    if [ -z "$target_user" ]; then
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    change_password "$target_user"
}

# 主函数
main() {
    # 严格过滤参数，去掉回车符和空格
    local first_arg=$(echo "$1" | tr -d ' \r\n')
    
    # 增加防御性判断：如果参数为空，或者参数名等于脚本名本身，则进入交互模式
    if [ -z "$first_arg" ] || [ "$first_arg" = "mypasswd" ] || [ "$first_arg" = "./mypasswd" ]; then
        # 无参数或错误参数，交互式选择用户
        interactive_mode
    else
        # 有参数，直接修改指定用户密码
        change_password "$first_arg"
    fi
}

main "$@"

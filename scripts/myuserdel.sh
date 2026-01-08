#!/bin/bash
#
# myuserdel.sh - 删除用户脚本 (Mini Shell 内部用户系统)
# 功能：删除指定用户，若用户不存在则给出提示
# 使用：myuserdel [username]
#

# 从 Mini Shell 用户管理文件删除
delete_from_mini_users() {
    local username="$1"
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        # 使用更健壮的 sed 匹配，支持制表符或空格
        sed -i "/^${username}[[:space:]]/d" "$db_file"
    fi
}

# 检查用户是否在用户管理文件中
check_user_in_mini_users() {
    local username="$1"
    local db_file="$HOME/.mini_users"
    if [ -f "$db_file" ]; then
        grep -q "^${username}	" "$db_file"
        return $?
    fi
    return 1
}

# 删除用户函数
delete_user() {
    local username="$1"
    
    # 去除可能存在的回车符
    username=$(echo "$username" | tr -d '\r')
    
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
    confirm=$(echo "$confirm" | tr -d '\r')
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
        # 使用 awk 处理制表符分隔的文件
        awk -F'\t' '{ print "  - " $1 }' "$db_file"
        echo "----------------------------------------"
        echo
    fi
    
    # 输入用户名
    read -p "请输入要删除的用户名: " username
    username=$(echo "$username" | tr -d '\r')
    
    if [ -z "$username" ]; then
        echo "错误: 用户名不能为空"
        return 1
    fi
    
    delete_user "$username"
}

# 主函数
main() {
    # 严格过滤参数，去掉回车符、换行符和首尾空格
    local first_arg=$(echo "$1" | tr -d ' \r\n')
    
    # 增加防御性判断：如果参数为空，或者参数名等于脚本名本身，则进入交互模式
    if [ -z "$first_arg" ] || [ "$first_arg" = "myuserdel" ] || [ "$first_arg" = "./myuserdel" ] || [ "$first_arg" = "userdel" ]; then
        # 无参数或错误参数，交互式模式
        delete_user_interactive
    else
        # 有参数，直接删除指定用户
        delete_user "$first_arg"
    fi
}

main "$@"

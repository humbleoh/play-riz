#!/bin/bash

# MQTT用户动态管理工具
# 支持添加、删除用户并即时生效

PASSWORD_FILE="mosquitto_passwd"
BROKER_PID_FILE="/tmp/mosquitto.pid"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印帮助信息
show_help() {
    echo -e "${BLUE}MQTT用户动态管理工具${NC}"
    echo ""
    echo "用法: $0 [选项] <用户名> [密码]"
    echo ""
    echo "选项:"
    echo "  -a, --add <用户名> [密码]    添加新用户（如果不提供密码，将提示输入）"
    echo "  -d, --delete <用户名>        删除用户"
    echo "  -l, --list                   列出所有用户"
    echo "  -r, --reload                 手动重载broker配置"
    echo "  -s, --status                 检查broker状态"
    echo "  -h, --help                   显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 -a newuser newpass        # 添加用户newuser，密码为newpass"
    echo "  $0 -a newuser                # 添加用户newuser，交互式输入密码"
    echo "  $0 -d olduser                # 删除用户olduser"
    echo "  $0 -l                        # 列出所有用户"
    echo ""
}

# 检查mosquitto_passwd工具是否可用
check_mosquitto_passwd() {
    if ! command -v mosquitto_passwd &> /dev/null; then
        echo -e "${RED}错误: mosquitto_passwd工具未找到${NC}"
        echo "请确保已安装Mosquitto并且mosquitto_passwd在PATH中"
        exit 1
    fi
}

# 获取broker进程ID
get_broker_pid() {
    # 尝试从多个可能的位置获取PID
    local pid=""
    
    # 方法1: 从PID文件读取
    if [ -f "$BROKER_PID_FILE" ]; then
        pid=$(cat "$BROKER_PID_FILE" 2>/dev/null)
    fi
    
    # 方法2: 通过进程名查找
    if [ -z "$pid" ] || ! kill -0 "$pid" 2>/dev/null; then
        pid=$(pgrep -f "mosquitto.*mosquitto_auth.conf" | head -1)
    fi
    
    # 方法3: 查找任何mosquitto进程
    if [ -z "$pid" ]; then
        pid=$(pgrep mosquitto | head -1)
    fi
    
    echo "$pid"
}

# 重载broker配置
reload_broker() {
    local pid=$(get_broker_pid)
    
    if [ -z "$pid" ]; then
        echo -e "${RED}错误: 未找到运行中的Mosquitto broker进程${NC}"
        echo "请确保broker正在运行"
        return 1
    fi
    
    echo -e "${YELLOW}正在重载broker配置 (PID: $pid)...${NC}"
    
    if kill -HUP "$pid" 2>/dev/null; then
        echo -e "${GREEN}✓ 配置重载成功${NC}"
        return 0
    else
        echo -e "${RED}✗ 配置重载失败${NC}"
        return 1
    fi
}

# 添加用户
add_user() {
    local username="$1"
    local password="$2"
    
    if [ -z "$username" ]; then
        echo -e "${RED}错误: 用户名不能为空${NC}"
        return 1
    fi
    
    echo -e "${BLUE}添加用户: $username${NC}"
    
    # 检查用户是否已存在
    if [ -f "$PASSWORD_FILE" ] && grep -q "^$username:" "$PASSWORD_FILE"; then
        echo -e "${YELLOW}警告: 用户 '$username' 已存在，将更新密码${NC}"
    fi
    
    # 添加用户到密码文件
    if [ -n "$password" ]; then
        # 使用提供的密码
        mosquitto_passwd -b "$PASSWORD_FILE" "$username" "$password"
    else
        # 交互式输入密码
        mosquitto_passwd "$PASSWORD_FILE" "$username"
    fi
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 用户添加成功${NC}"
        
        # 重载broker配置
        if reload_broker; then
            echo -e "${GREEN}✓ 用户 '$username' 已生效，可以立即使用${NC}"
        else
            echo -e "${YELLOW}⚠ 用户已添加到密码文件，但配置重载失败${NC}"
            echo -e "${YELLOW}  请手动重启broker或运行: $0 -r${NC}"
        fi
    else
        echo -e "${RED}✗ 用户添加失败${NC}"
        return 1
    fi
}

# 删除用户
delete_user() {
    local username="$1"
    
    if [ -z "$username" ]; then
        echo -e "${RED}错误: 用户名不能为空${NC}"
        return 1
    fi
    
    if [ ! -f "$PASSWORD_FILE" ]; then
        echo -e "${RED}错误: 密码文件不存在${NC}"
        return 1
    fi
    
    # 检查用户是否存在
    if ! grep -q "^$username:" "$PASSWORD_FILE"; then
        echo -e "${RED}错误: 用户 '$username' 不存在${NC}"
        return 1
    fi
    
    echo -e "${BLUE}删除用户: $username${NC}"
    
    # 删除用户
    mosquitto_passwd -D "$PASSWORD_FILE" "$username"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 用户删除成功${NC}"
        
        # 重载broker配置
        if reload_broker; then
            echo -e "${GREEN}✓ 用户 '$username' 已被禁用，立即生效${NC}"
        else
            echo -e "${YELLOW}⚠ 用户已从密码文件删除，但配置重载失败${NC}"
            echo -e "${YELLOW}  请手动重启broker或运行: $0 -r${NC}"
        fi
    else
        echo -e "${RED}✗ 用户删除失败${NC}"
        return 1
    fi
}

# 列出所有用户
list_users() {
    if [ ! -f "$PASSWORD_FILE" ]; then
        echo -e "${YELLOW}密码文件不存在或为空${NC}"
        return 0
    fi
    
    echo -e "${BLUE}当前用户列表:${NC}"
    echo "=================="
    
    local count=0
    while IFS=':' read -r username hash; do
        if [ -n "$username" ]; then
            count=$((count + 1))
            echo -e "${GREEN}$count. $username${NC}"
        fi
    done < "$PASSWORD_FILE"
    
    if [ $count -eq 0 ]; then
        echo -e "${YELLOW}没有找到用户${NC}"
    else
        echo "=================="
        echo -e "${BLUE}总计: $count 个用户${NC}"
    fi
}

# 检查broker状态
check_status() {
    local pid=$(get_broker_pid)
    
    echo -e "${BLUE}Broker状态检查:${NC}"
    echo "=================="
    
    if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
        echo -e "${GREEN}✓ Mosquitto broker正在运行 (PID: $pid)${NC}"
        
        # 检查配置文件
        if [ -f "mosquitto_auth.conf" ]; then
            echo -e "${GREEN}✓ 认证配置文件存在${NC}"
        else
            echo -e "${YELLOW}⚠ 认证配置文件不存在${NC}"
        fi
        
        # 检查密码文件
        if [ -f "$PASSWORD_FILE" ]; then
            local user_count=$(grep -c '^[^#].*:' "$PASSWORD_FILE" 2>/dev/null || echo "0")
            echo -e "${GREEN}✓ 密码文件存在 ($user_count 个用户)${NC}"
        else
            echo -e "${YELLOW}⚠ 密码文件不存在${NC}"
        fi
    else
        echo -e "${RED}✗ Mosquitto broker未运行${NC}"
        echo -e "${YELLOW}  请先启动broker: ./start_auth_broker.sh${NC}"
    fi
}

# 主函数
main() {
    # 检查mosquitto_passwd工具
    check_mosquitto_passwd
    
    # 解析命令行参数
    case "$1" in
        -a|--add)
            add_user "$2" "$3"
            ;;
        -d|--delete)
            delete_user "$2"
            ;;
        -l|--list)
            list_users
            ;;
        -r|--reload)
            reload_broker
            ;;
        -s|--status)
            check_status
            ;;
        -h|--help|"")
            show_help
            ;;
        *)
            echo -e "${RED}错误: 未知选项 '$1'${NC}"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
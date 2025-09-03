#!/bin/bash

# MQTT设备监控框架演示启动脚本
# 此脚本将启动MQTT broker、服务端和一个模拟设备

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# 检查是否已编译
check_build() {
    if [ ! -f "build/server" ] || [ ! -f "build/device" ]; then
        print_error "项目尚未编译，请先运行 ./build.sh"
        exit 1
    fi
}

# 检查MQTT broker
check_mosquitto() {
    if ! command -v mosquitto &> /dev/null; then
        print_error "mosquitto未安装，请先安装mosquitto"
        print_info "macOS: brew install mosquitto"
        print_info "Ubuntu: sudo apt-get install mosquitto"
        exit 1
    fi
}

# 检查端口是否被占用
check_port() {
    local port=$1
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1; then
        return 0  # 端口被占用
    else
        return 1  # 端口空闲
    fi
}

# 启动MQTT broker
start_mosquitto() {
    print_step "启动MQTT Broker..."
    
    if check_port 1883; then
        print_warning "端口1883已被占用，假设MQTT broker已在运行"
    else
        print_info "在后台启动mosquitto..."
        mosquitto -d -p 1883
        sleep 2
        
        if check_port 1883; then
            print_info "MQTT Broker启动成功 (端口: 1883)"
        else
            print_error "MQTT Broker启动失败"
            exit 1
        fi
    fi
}

# 启动服务端
start_server() {
    print_step "启动服务端..."
    print_info "服务端将在新终端窗口中启动"
    
    # 根据操作系统选择终端命令
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)' && ./build/server --host localhost --port 1883 --server-id demo_server"'
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        if command -v gnome-terminal &> /dev/null; then
            gnome-terminal -- bash -c "cd $(pwd) && ./build/server --host localhost --port 1883 --server-id demo_server; exec bash"
        elif command -v xterm &> /dev/null; then
            xterm -e "cd $(pwd) && ./build/server --host localhost --port 1883 --server-id demo_server; exec bash" &
        else
            print_warning "未找到合适的终端，请手动在新终端中运行:"
            print_info "./build/server --host localhost --port 1883 --server-id demo_server"
        fi
    else
        print_warning "未识别的操作系统，请手动在新终端中运行:"
        print_info "./build/server --host localhost --port 1883 --server-id demo_server"
    fi
    
    sleep 2
}

# 启动设备端
start_device() {
    print_step "启动模拟设备..."
    print_info "设备端将在新终端窗口中启动"
    
    # 根据操作系统选择终端命令
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        osascript -e 'tell application "Terminal" to do script "cd '$(pwd)' && ./build/device --id demo_sensor --type temperature_sensor --host localhost --port 1883 --simulate"'
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        if command -v gnome-terminal &> /dev/null; then
            gnome-terminal -- bash -c "cd $(pwd) && ./build/device --id demo_sensor --type temperature_sensor --host localhost --port 1883 --simulate; exec bash"
        elif command -v xterm &> /dev/null; then
            xterm -e "cd $(pwd) && ./build/device --id demo_sensor --type temperature_sensor --host localhost --port 1883 --simulate; exec bash" &
        else
            print_warning "未找到合适的终端，请手动在新终端中运行:"
            print_info "./build/device --id demo_sensor --type temperature_sensor --host localhost --port 1883 --simulate"
        fi
    else
        print_warning "未识别的操作系统，请手动在新终端中运行:"
        print_info "./build/device --id demo_sensor --type temperature_sensor --host localhost --port 1883 --simulate"
    fi
    
    sleep 2
}

# 显示使用说明
show_instructions() {
    echo ""
    print_info "=== 演示启动完成 ==="
    echo ""
    print_info "现在你可以:"
    echo "  1. 在服务端终端中输入 'list' 查看设备列表"
    echo "  2. 输入 'status demo_sensor' 查看设备状态"
    echo "  3. 输入 'command demo_sensor get_temperature' 发送命令"
    echo "  4. 在设备端终端中输入 'status' 查看设备信息"
    echo "  5. 输入 'set temperature 25.5' 设置设备属性"
    echo ""
    print_info "停止演示:"
    echo "  1. 在各个终端中输入 'quit' 退出程序"
    echo "  2. 运行 'pkill mosquitto' 停止MQTT broker"
    echo ""
    print_info "查看详细文档: cat README.md"
}

# 清理函数
cleanup() {
    print_info "正在清理..."
    # 这里可以添加清理逻辑，比如停止后台进程
}

# 设置信号处理
trap cleanup EXIT

# 显示欢迎信息
echo "======================================"
echo "   MQTT设备监控框架 - 快速演示"
echo "======================================"
echo ""

# 主执行流程
check_build
check_mosquitto
start_mosquitto
start_server
start_device
show_instructions

print_info "演示环境已启动！按Ctrl+C退出此脚本。"

# 保持脚本运行
while true; do
    sleep 1
done
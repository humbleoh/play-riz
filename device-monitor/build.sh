#!/bin/bash

# MQTT设备监控框架构建脚本
# 使用方法: ./build.sh [clean|install]

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

# 检查依赖
check_dependencies() {
    print_info "检查依赖库..."
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake未安装，请先安装CMake"
        exit 1
    fi
    
    # 检查pkg-config
    if ! command -v pkg-config &> /dev/null; then
        print_warning "pkg-config未安装，可能影响依赖库检测"
    fi
    
    # 检查libmosquitto
    if ! pkg-config --exists libmosquitto 2>/dev/null; then
        print_warning "libmosquitto未找到，请确保已安装mosquitto开发库"
        print_info "macOS: brew install mosquitto"
        print_info "Ubuntu: sudo apt-get install libmosquitto-dev"
    fi
    
    # 检查jsoncpp
    if ! pkg-config --exists jsoncpp 2>/dev/null; then
        print_warning "jsoncpp未找到，请确保已安装jsoncpp开发库"
        print_info "macOS: brew install jsoncpp"
        print_info "Ubuntu: sudo apt-get install libjsoncpp-dev"
    fi
}

# 清理构建目录
clean_build() {
    print_info "清理构建目录..."
    if [ -d "build" ]; then
        rm -rf build
        print_info "构建目录已清理"
    fi
}

# 创建构建目录
create_build_dir() {
    if [ ! -d "build" ]; then
        print_info "创建构建目录..."
        mkdir build
    fi
}

# 配置项目
configure_project() {
    print_info "配置CMake项目..."
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cd ..
}

# 编译项目
build_project() {
    print_info "编译项目..."
    cd build
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cd ..
    print_info "编译完成！"
}

# 安装项目
install_project() {
    print_info "安装项目..."
    cd build
    sudo make install
    cd ..
    print_info "安装完成！"
}

# 显示使用说明
show_usage() {
    echo "MQTT设备监控框架构建脚本"
    echo "使用方法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  clean     清理构建目录"
    echo "  install   编译并安装项目"
    echo "  help      显示此帮助信息"
    echo ""
    echo "默认行为: 检查依赖、配置、编译项目"
}

# 显示构建结果
show_result() {
    print_info "构建结果:"
    if [ -f "build/server" ]; then
        echo "  ✓ 服务端程序: build/server"
    else
        echo "  ✗ 服务端程序构建失败"
    fi
    
    if [ -f "build/device" ]; then
        echo "  ✓ 设备端程序: build/device"
    else
        echo "  ✗ 设备端程序构建失败"
    fi
    
    echo ""
    print_info "使用方法:"
    echo "  启动服务端: ./build/server"
    echo "  启动设备端: ./build/device --id device001"
    echo "  查看详细说明: cat README.md"
}

# 主函数
main() {
    case "$1" in
        "clean")
            clean_build
            ;;
        "install")
            check_dependencies
            create_build_dir
            configure_project
            build_project
            install_project
            ;;
        "help")
            show_usage
            exit 0
            ;;
        "")
            check_dependencies
            create_build_dir
            configure_project
            build_project
            show_result
            ;;
        *)
            print_error "未知选项: $1"
            show_usage
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"
#!/bin/bash

# OpusLib GDExtension 构建脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    print_info "检查构建依赖..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake 未安装，请先安装 CMake"
        exit 1
    fi
    
    if ! command -v git &> /dev/null; then
        print_error "Git 未安装，请先安装 Git"
        exit 1
    fi
    
    print_success "依赖检查完成"
}

# 初始化子模块
init_submodules() {
    print_info "初始化 Git 子模块..."
    
    if [ ! -d "godot-cpp/.git" ]; then
        git submodule update --init --recursive godot-cpp
    fi
    
    if [ ! -d "third/opus/.git" ]; then
        git submodule update --init --recursive third/opus
    fi
    
    print_success "子模块初始化完成"
}

# 构建项目
build_project() {
    local build_type=${1:-Release}
    local build_dir="build"
    
    print_info "开始构建项目 (${build_type} 模式)..."
    
    # 创建构建目录（不删除现有文件）
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # 检查是否需要重新配置
    local need_reconfigure=false
    if [ ! -f "CMakeCache.txt" ]; then
        need_reconfigure=true
    else
        # 检查构建类型是否改变
        local current_build_type=$(grep "CMAKE_BUILD_TYPE:STRING=" CMakeCache.txt 2>/dev/null | cut -d'=' -f2 || echo "")
        if [ "$current_build_type" != "$build_type" ]; then
            need_reconfigure=true
            print_warning "构建类型从 $current_build_type 改变为 $build_type，重新配置..."
        fi
    fi
    
    # 配置项目（仅在需要时）
    if [ "$need_reconfigure" = true ]; then
        print_info "配置 CMake..."
        cmake .. -DCMAKE_BUILD_TYPE="$build_type" \
                 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    fi
    
    # 构建项目
    print_info "编译项目..."
    cmake --build . --config "$build_type" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    cd ..
    print_success "构建完成"
}

# 清理构建文件
clean() {
    print_info "清理构建文件..."
    rm -rf build/
    rm -rf bin/
    rm -rf lib/
    print_success "清理完成"
}

# 显示帮助信息
show_help() {
    echo "OpusLib GDExtension 构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  build [debug|release]  构建项目 (默认: release)"
    echo "  clean                  清理构建文件"
    echo "  init                   初始化子模块"
    echo "  help                   显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 build              # 构建 Release 版本"
    echo "  $0 build debug        # 构建 Debug 版本"
    echo "  $0 clean              # 清理构建文件"
}

# 主函数
main() {
    case "${1:-build}" in
        "build")
            check_dependencies
            init_submodules
            build_project "${2:-Release}"
            ;;
        "clean")
            clean
            ;;
        "init")
            check_dependencies
            init_submodules
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@" 
cmake_minimum_required(VERSION 3.16)

project(OpusLib VERSION 1.0.0 LANGUAGES CXX C)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 平台特定设置
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(BUILD_TYPE_LOWER STREQUAL "debug")
    set(BUILD_SUFFIX "debug")
else()
    set(BUILD_SUFFIX "release")
endif()

if(WIN32)
    set(TARGET_PATH "win64")
    set(TARGET_NAME "libp3opus.windows.template_${BUILD_SUFFIX}.x86_64.dll")
elseif(APPLE)
    set(TARGET_PATH "macos")
    set(TARGET_NAME "libp3opus.macos.template_${BUILD_SUFFIX}.universal.dylib")
elseif(UNIX)
    set(TARGET_PATH "linux")
    set(TARGET_NAME "libp3opus.linux.template_${BUILD_SUFFIX}.x86_64.so")
endif()

# 根据构建类型设置godot-cpp目标
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(BUILD_TYPE_LOWER STREQUAL "debug")
    set(GODOTCPP_TARGET "template_debug" CACHE STRING "Godot-cpp target type" FORCE)
else()
    set(GODOTCPP_TARGET "template_release" CACHE STRING "Godot-cpp target type" FORCE)
endif()

# 添加godot-cpp子目录
add_subdirectory(godot-cpp)

# 构建Opus库
set(OPUS_BUILD_SHARED_LIBRARY ON CACHE BOOL "Build shared library")
set(OPUS_BUILD_TESTING OFF CACHE BOOL "Build tests")
set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "Build programs")
set(OPUS_INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "Install pkg-config module")
set(OPUS_INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "Install CMake config module")

# 修复ARM Mac上的NEON检测问题
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(OPUS_MAY_HAVE_NEON ON CACHE BOOL "Enable NEON support")
    set(OPUS_PRESUME_NEON ON CACHE BOOL "Presume NEON support")
endif()

add_subdirectory(third/opus)

# 收集源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE HEADERS 
    "src/*.h"
    "src/*.hpp"
)

# 创建共享库
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    OUTPUT_NAME "p3opus"
    PREFIX "lib"
)

# 根据平台设置后缀
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".windows.template_${BUILD_SUFFIX}.x86_64.dll"
    )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".macos.template_${BUILD_SUFFIX}.universal.dylib"
    )
elseif(UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".linux.template_${BUILD_SUFFIX}.x86_64.so"
    )
endif()

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    third/opus/include/
    godot-cpp/include/
    godot-cpp/gen/include/
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp
    opus
)

# 编译器特定设置
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Wno-unused-parameter
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()

# 添加预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GDEXTENSION
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 创建extension配置文件的模板
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extension.gdextension.in")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/extension.gdextension.in"
        "${CMAKE_CURRENT_BINARY_DIR}/extension.gdextension"
        @ONLY
    )
else()
    message(WARNING "extension.gdextension.in not found, skipping configuration file generation")
endif()

# 打印构建信息
message(STATUS "Building OpusLib GDExtension")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target platform: ${TARGET_PATH}")
message(STATUS "Output name: ${TARGET_NAME}") 